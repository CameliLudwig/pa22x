#include "logger.h"
#include <QStandardPaths>
#include <QDir>
#include <QCoreApplication>
#include <QDateTime>
#include <QThread>
#include <QTextCodec>
#include <QFileInfo>
#include <QRegularExpression>
#include <QDebug>

#ifdef Q_OS_WIN
  #include <windows.h>
  static void fsyncFile(QFile& f) { /* Windows 无直接 fsync，略过 */ }
#else
  #include <unistd.h>
  #include <sys/types.h>
  #include <sys/stat.h>
  static void fsyncFile(QFile& f) { if (f.handle() != -1) ::fsync(f.handle()); }
#endif

Logger* Logger::s_instance = nullptr;

namespace {

bool verboseLogRequested()
{
    return QCoreApplication::arguments().contains(QStringLiteral("--verbose-log"))
            || qEnvironmentVariableIntValue("PA22X_VERBOSE_LOG") == 1;
}

}

Logger::Logger(QObject* parent) : QObject(parent) {
    m_stream.setCodec("UTF-8");
}

Logger::~Logger() {
    if (m_file.isOpen()) m_file.close();
}

QString Logger::logDir() const {
//    QString base = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
//    QString appName = QCoreApplication::applicationName().isEmpty()
//                      ? QStringLiteral("pa22x")
//                      : QCoreApplication::applicationName();
//    QString dir = QDir(base).filePath(QStringLiteral("logs"));
    QString dir = QCoreApplication::applicationDirPath();

    QDir().mkpath(dir);
    return dir;
}

QString Logger::traceFilePathForDate(const QDate& date) const
{
    return QDir(m_dir)
        .filePath(QStringLiteral("pa22x_trace_%1.log").arg(date.toString("yyyy-MM-dd")));
}

void Logger::init(int retentionDays, bool alsoConsole) {
    if (s_instance) return;
    s_instance = new Logger();
    s_instance->m_retentionDays = retentionDays;
    s_instance->m_alsoConsole = alsoConsole;
    s_instance->m_verboseDebug = verboseLogRequested();
    s_instance->m_dir = s_instance->logDir();
    s_instance->m_currentDate = QDate::currentDate();

    const QString filePath = QDir(s_instance->m_dir)
        .filePath(QStringLiteral("pa22x_%1.log").arg(s_instance->m_currentDate.toString("yyyy-MM-dd")));
    s_instance->m_currentFilePath = filePath;
    s_instance->m_currentTraceFilePath = s_instance->traceFilePathForDate(s_instance->m_currentDate);
    const bool newLogFile = !QFileInfo::exists(filePath) || QFileInfo(filePath).size() == 0;

    s_instance->m_file.setFileName(filePath);
    s_instance->m_file.open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text);
    s_instance->m_stream.setDevice(&s_instance->m_file);
    s_instance->m_stream.setGenerateByteOrderMark(newLogFile);
    s_instance->purgeOldLogs();
    qInstallMessageHandler(Logger::qtMessageHandler);
    qInfo().noquote() << QStringLiteral("[LOG] 初始化完成 | 文件=%1 | 程序目录=%2 | 启动目录=%3 | 详细调试=%4")
                         .arg(QDir::toNativeSeparators(filePath),
                              QDir::toNativeSeparators(QCoreApplication::applicationDirPath()),
                              QDir::toNativeSeparators(QDir::currentPath()),
                              s_instance->m_verboseDebug ? QStringLiteral("开启")
                                                         : QStringLiteral("关闭"));
}

void Logger::shutdown() {
    if (!s_instance) return;
    qInstallMessageHandler(nullptr);
    delete s_instance;
    s_instance = nullptr;
}

QString Logger::levelToString(QtMsgType type) const {
    switch (type) {
        case QtDebugMsg:    return "DEBUG";
        case QtInfoMsg:     return "INFO";
        case QtWarningMsg:  return "WARN";
        case QtCriticalMsg: return "ERROR";
        case QtFatalMsg:    return "FATAL";
    }
    return "?";
}

void Logger::rotateIfNeeded() {
    QDate today = QDate::currentDate();
    if (today == m_currentDate) return;

    m_currentDate = today;
    if (m_file.isOpen()) m_file.close();

    const QString filePath = QDir(m_dir)
        .filePath(QStringLiteral("pa22x_%1.log").arg(today.toString("yyyy-MM-dd")));
    m_currentFilePath = filePath;
    m_currentTraceFilePath = traceFilePathForDate(today);
    const bool newLogFile = !QFileInfo::exists(filePath) || QFileInfo(filePath).size() == 0;
    m_file.setFileName(filePath);
    m_file.open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text);
    m_stream.setDevice(&m_file);
    m_stream.setGenerateByteOrderMark(newLogFile);
    purgeOldLogs();
}

void Logger::purgeOldLogs() {
    QDir dir(m_dir);
    const QDate keepAfter = QDate::currentDate().addDays(-m_retentionDays);
    const QList<QPair<QString, QRegularExpression>> logPatterns = {
        qMakePair(QStringLiteral("pa22x_*.log"),
                  QRegularExpression(QStringLiteral("^pa22x_(\\d{4}-\\d{2}-\\d{2})\\.log$"))),
        qMakePair(QStringLiteral("pa22x_trace_*.log"),
                  QRegularExpression(QStringLiteral("^pa22x_trace_(\\d{4}-\\d{2}-\\d{2})\\.log$")))
    };

    for (const auto& entry : logPatterns) {
        const QStringList files = dir.entryList(QStringList() << entry.first, QDir::Files, QDir::Name);
        for (const QString& fn : files) {
            const auto m = entry.second.match(fn);
            if (!m.hasMatch())
                continue;

            const QDate d = QDate::fromString(m.captured(1), "yyyy-MM-dd");
            if (d.isValid() && d < keepAfter)
                dir.remove(fn);
        }
    }
}

void Logger::write(QtMsgType type, const QMessageLogContext& ctx, const QString& msg) {
    QMutexLocker locker(&m_mutex);
    rotateIfNeeded();

    const auto now = QDateTime::currentDateTime();
    const QString ts = now.toString("yyyy-MM-dd hh:mm:ss.zzz");
    const auto tid = reinterpret_cast<quintptr>(QThread::currentThreadId());

    // 清理消息中的换行，避免破坏一行一条
    QString clean = msg;
    clean.replace('\n', ' ');

    m_stream << ts << " | " << levelToString(type)
             << " | T:" << QString::number(tid, 16)
             << " | " << (ctx.file ? ctx.file : "?") << ":" << (ctx.line)
             << " | " << (ctx.function ? ctx.function : "?")
             << " | " << clean << '\n';
    m_stream.flush();

    if (type >= QtWarningMsg) {
        fsyncFile(m_file);
    }

    if (m_alsoConsole && (type != QtDebugMsg || m_verboseDebug)) {
#if QT_VERSION >= QT_VERSION_CHECK(5, 5, 0)
        fprintf(stderr, "%s | %s | T:%s | %s:%d | %s | %s\n",
                ts.toLocal8Bit().constData(),
                levelToString(type).toLocal8Bit().constData(),
                QString::number(tid, 16).toLocal8Bit().constData(),
                ctx.file ? ctx.file : "?", ctx.line,
                ctx.function ? ctx.function : "?",
                clean.toLocal8Bit().constData());
        fflush(stderr);
#endif
    }
}

void Logger::qtMessageHandler(QtMsgType type, const QMessageLogContext &ctx, const QString &msg) {
    if (!s_instance) return;
    s_instance->write(type, ctx, msg);
}

void Logger::writeTrace(const QString& scope, const QStringList& lines)
{
    QMutexLocker locker(&m_mutex);
    rotateIfNeeded();

    const auto now = QDateTime::currentDateTime();
    const QString ts = now.toString("yyyy-MM-dd hh:mm:ss.zzz");
    const auto tid = reinterpret_cast<quintptr>(QThread::currentThreadId());
    const QString traceFilePath = traceFilePathForDate(m_currentDate);
    m_currentTraceFilePath = traceFilePath;
    const bool newLogFile = !QFileInfo::exists(traceFilePath) || QFileInfo(traceFilePath).size() == 0;

    QFile traceFile(traceFilePath);
    if (!traceFile.open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text))
        return;

    QTextStream traceStream(&traceFile);
    traceStream.setCodec("UTF-8");
    traceStream.setGenerateByteOrderMark(newLogFile);
    traceStream << "==== " << ts
                << " | T:" << QString::number(tid, 16)
                << " | " << scope
                << " ====\n";

    for (const QString& line : lines)
        traceStream << line << '\n';

    traceStream << '\n';
    traceStream.flush();
    if (m_alsoConsole && m_verboseDebug) {
        fprintf(stderr, "%s | TRACE | T:%s | %s | lines=%d\n",
                ts.toLocal8Bit().constData(),
                QString::number(tid, 16).toLocal8Bit().constData(),
                scope.toLocal8Bit().constData(),
                lines.size());
        fflush(stderr);
    }
}

bool Logger::isVerboseEnabled()
{
    return s_instance ? s_instance->m_verboseDebug : verboseLogRequested();
}

QString Logger::currentLogFilePath()
{
    return s_instance ? s_instance->m_currentFilePath : QString();
}

QString Logger::currentTraceFilePath()
{
    return s_instance ? s_instance->m_currentTraceFilePath : QString();
}

void Logger::appendTrace(const QString& scope, const QString& message)
{
    if (!s_instance)
        return;

    s_instance->writeTrace(scope, message.split(QLatin1Char('\n')));
}

void Logger::appendTraceBlock(const QString& scope, const QStringList& lines)
{
    if (!s_instance)
        return;

    s_instance->writeTrace(scope, lines);
}
