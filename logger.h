#pragma once
#include <QObject>
#include <QFile>
#include <QTextStream>
#include <QDate>
#include <QMutex>
#include <QStringList>

class Logger final : public QObject {
    Q_OBJECT
public:
    // retentionDays: 日志保留天数；alsoConsole: 是否同时打印到控制台/调试器
    static void init(int retentionDays = 7, bool alsoConsole = true);
    static void shutdown();
    static void qtMessageHandler(QtMsgType type, const QMessageLogContext &ctx, const QString &msg);
    static bool isVerboseEnabled();
    static QString currentLogFilePath();
    static QString currentTraceFilePath();
    static void appendTrace(const QString& scope, const QString& message);
    static void appendTraceBlock(const QString& scope, const QStringList& lines);

private:
    explicit Logger(QObject* parent=nullptr);
    ~Logger();

    void write(QtMsgType type, const QMessageLogContext& ctx, const QString& msg);
    void writeTrace(const QString& scope, const QStringList& lines);
    void rotateIfNeeded();
    void purgeOldLogs();
    QString levelToString(QtMsgType type) const;
    QString logDir() const;
    QString traceFilePathForDate(const QDate& date) const;

    QFile m_file;
    QTextStream m_stream;
    QDate m_currentDate;
    QString m_dir;
    int m_retentionDays = 7;
    bool m_alsoConsole = true;
    bool m_verboseDebug = false;
    QMutex m_mutex;
    QString m_currentFilePath;
    QString m_currentTraceFilePath;

    static Logger* s_instance;
};
