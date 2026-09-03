#include "xlsxhandle.h"
#include "logger.h"

#include <QCoreApplication>
#include <QDate>
#include <QDateTime>
#include <QDir>
#include <QFileInfo>
#include <QMutexLocker>
#include <QThread>
#include <QDebug>

using namespace QXlsx;

namespace {

constexpr int kMaxRowsPerSheet = 50000;
constexpr int kLogColumnCount = 6;
constexpr int kMaxPendingRows = 500;       // 缓冲区上限，防止内存耗尽
constexpr int kMaxConsecutiveFailures = 5; // 连续失败上限，触发丢弃保护

QString toNativePath(const QString& filePath)
{
    return QDir::toNativeSeparators(QDir::cleanPath(filePath));
}

}

xlsxHandle::xlsxHandle()
{
    m_flushTimer = new QTimer(this);
    m_flushTimer->setSingleShot(false);
    m_flushTimer->setInterval(2000);
    connect(m_flushTimer, &QTimer::timeout, this, [this]() {
        QMutexLocker locker(&m_mutex);
        if (!m_pendingRows.isEmpty()) {
            QString errorMessage;
            flushBufferLocked(&errorMessage);
        }
    });
    m_flushTimer->start();
}

xlsxHandle::~xlsxHandle()
{
    if (m_flushTimer) {
        m_flushTimer->stop();
    }
    flushAndWait();
}

QString xlsxHandle::currentLogFilePath() const
{
    QMutexLocker locker(&m_mutex);
    return m_filePath_log;
}

QString xlsxHandle::buildLogFilePath() const
{
    const QString appDir = QCoreApplication::applicationDirPath();
    return QDir(appDir).filePath(
        QStringLiteral("pa22x_log_%1.xlsx")
            .arg(QDate::currentDate().toString(QStringLiteral("yyyyMMdd"))));
}

bool xlsxHandle::ensureWorkbookReady(QXlsx::Document& xlsx, QString* errorMessage)
{
    QStringList sheetNames = xlsx.sheetNames();
    if (sheetNames.isEmpty()) {
        xlsx.addSheet(QStringLiteral("Sheet1"));
        sheetNames = xlsx.sheetNames();
    }

    if (sheetNames.isEmpty()) {
        if (errorMessage)
            *errorMessage = QStringLiteral("xlsx 工作表初始化失败");
        return false;
    }

    Worksheet* sheet =
        dynamic_cast<Worksheet*>(xlsx.sheet(sheetNames.constLast()));
    if (!sheet) {
        if (errorMessage)
            *errorMessage = QStringLiteral("xlsx 当前工作表不可用");
        return false;
    }

    if (sheet->dimension().lastRow() <= 0) {
        sheet->write(1, 1, QStringLiteral("Time"));
        sheet->write(1, 2, QStringLiteral("Scope"));
        sheet->write(1, 3, QStringLiteral("Code"));
        sheet->write(1, 4, QStringLiteral("Step"));
        sheet->write(1, 5, QStringLiteral("Detail"));
        sheet->write(1, 6, QStringLiteral("CycleId"));
    }

    xlsx.setColumnWidth(1, 24);
    xlsx.setColumnWidth(2, 16);
    xlsx.setColumnWidth(3, 14);
    xlsx.setColumnWidth(4, 28);
    xlsx.setColumnWidth(5, 160);
    xlsx.setColumnWidth(6, 28);
    return true;
}

bool xlsxHandle::createNewExcelFileLocked(QString* errorMessage)
{
    const QString targetFilePath = buildLogFilePath();

    if (QFileInfo::exists(targetFilePath)) {
        QXlsx::Document existing(targetFilePath);
        if (!ensureWorkbookReady(existing, errorMessage)) {
            m_filePath_log.clear();
            return false;
        }
        if (!existing.save()) {
            if (errorMessage) {
                *errorMessage = QStringLiteral("xlsx 日志文件打开失败：%1")
                                    .arg(toNativePath(targetFilePath));
            }
            m_filePath_log.clear();
            return false;
        }

        m_filePath_log = targetFilePath;
        qInfo().noquote() << QStringLiteral("[日志][XLSX-01] xlsx 日志已复用 | %1")
                             .arg(toNativePath(targetFilePath));
        return true;
    }

    QXlsx::Document xlsx;
    if (!ensureWorkbookReady(xlsx, errorMessage))
        return false;

    if (!xlsx.saveAs(targetFilePath)) {
        if (errorMessage) {
            *errorMessage = QStringLiteral("xlsx 日志文件创建失败：%1")
                                .arg(toNativePath(targetFilePath));
        }
        m_filePath_log.clear();
        return false;
    }

    m_filePath_log = targetFilePath;
    qInfo().noquote() << QStringLiteral("[日志][XLSX-01] xlsx 日志已创建 | %1")
                         .arg(toNativePath(targetFilePath));
    return true;
}

void xlsxHandle::checkExcel()
{
    QMutexLocker locker(&m_mutex);

    QString errorMessage;
    const QString todayFilePath = buildLogFilePath();
    if (m_filePath_log != todayFilePath || !QFileInfo(todayFilePath).exists()) {
        m_filePath_log.clear();
        m_cachedRowCount = 0;
        if (!createNewExcelFileLocked(&errorMessage))
            qWarning().noquote() << errorMessage;
        return;
    }

    // 使用缓存的计数避免每次打开xlsx文件进行读取
    const int totalRows = m_cachedRowCount + m_pendingRows.size();
    if (totalRows < kMaxRowsPerSheet)
        return;

    QXlsx::Document xlsx(todayFilePath);
    if (!ensureWorkbookReady(xlsx, &errorMessage)) {
        qWarning().noquote() << QStringLiteral("xlsx 日志文件不可用，尝试重建 | %1 | %2")
                                .arg(toNativePath(todayFilePath), errorMessage);
        if (!createNewExcelFileLocked(&errorMessage))
            qWarning().noquote() << errorMessage;
        return;
    }

    const QStringList sheetNames = xlsx.sheetNames();
    if (sheetNames.size() < 255) {
        const QString nextSheetName =
            QStringLiteral("Sheet%1").arg(sheetNames.size() + 1);
        xlsx.addSheet(nextSheetName);
        if (!ensureWorkbookReady(xlsx, &errorMessage) || !xlsx.save()) {
            qWarning().noquote() << QStringLiteral("xlsx 新工作表创建失败 | %1 | %2")
                                    .arg(toNativePath(todayFilePath), errorMessage);
            return;
        }

        m_cachedRowCount = 1;
        qInfo().noquote() << QStringLiteral("[日志][XLSX-02] xlsx 工作表已轮转 | %1 | %2")
                             .arg(toNativePath(todayFilePath), nextSheetName);
        return;
    }

    qWarning().noquote() << QStringLiteral("xlsx 已达到最大工作表数，将继续写入最后一个工作表 | %1")
                            .arg(toNativePath(todayFilePath));
}

void xlsxHandle::insertData(QString data)
{
    insertRecord(QStringLiteral("系统"),
                 QString(),
                 QStringLiteral("消息"),
                 data,
                 QString());
}

void xlsxHandle::insertRecord(QString scope,
                              QString code,
                              QString step,
                              QString detail,
                              QString cycleId)
{
    QMutexLocker locker(&m_mutex);

    const QString todayFilePath = buildLogFilePath();
    if (m_filePath_log != todayFilePath || !QFileInfo(todayFilePath).exists()) {
        m_filePath_log.clear();
        m_cachedRowCount = 0;
        QString errorMessage;
        if (!createNewExcelFileLocked(&errorMessage))
            qWarning().noquote() << errorMessage;
    } else {
        const int totalRows = m_cachedRowCount + m_pendingRows.size();
        if (totalRows >= kMaxRowsPerSheet) {
            QXlsx::Document xlsx(todayFilePath);
            QString errorMessage;
            if (!ensureWorkbookReady(xlsx, &errorMessage)) {
                qWarning().noquote() << QStringLiteral("xlsx 日志文件不可用，尝试重建 | %1 | %2")
                                        .arg(toNativePath(todayFilePath), errorMessage);
                m_cachedRowCount = 0;
                if (!createNewExcelFileLocked(&errorMessage))
                    qWarning().noquote() << errorMessage;
            } else {
                const QStringList sheetNames = xlsx.sheetNames();
                if (sheetNames.size() < 255) {
                    xlsx.addSheet(QStringLiteral("Sheet%1").arg(sheetNames.size() + 1));
                    if (!ensureWorkbookReady(xlsx, &errorMessage) || !xlsx.save()) {
                        qWarning().noquote() << QStringLiteral("xlsx 新工作表创建失败 | %1 | %2")
                                                .arg(toNativePath(todayFilePath), errorMessage);
                    } else {
                        m_cachedRowCount = 1;
                    }
                }
            }
        }
    }

    const QStringList columns = {
        QDateTime::currentDateTime().toString(QStringLiteral("yyyy-MM-dd HH:mm:ss")),
        scope,
        code,
        step,
        detail,
        cycleId
    };

    m_pendingRows.append(columns);

    // 保护：缓冲区超过上限时丢弃最旧的记录
    while (m_pendingRows.size() > kMaxPendingRows) {
        m_pendingRows.removeFirst();
        qWarning().noquote() << QStringLiteral("xlsx 缓冲区溢出，丢弃最旧记录 | path=%1")
                                .arg(toNativePath(m_filePath_log));
    }

    if (m_pendingRows.size() >= 50) {
        QString errorMessage;
        flushBufferLocked(&errorMessage);
    }
}

bool xlsxHandle::flushBufferLocked(QString* errorMessage)
{
    if (m_pendingRows.isEmpty() || m_filePath_log.isEmpty())
        return true;

    // 若上一次后台保存尚未完成，暂不提交新数据，避免堆积
    if (m_saveRunning.load(std::memory_order_acquire))
        return false;

    // 将待写数据交换到 m_saveInFlight，空出 m_pendingRows 继续接收新记录
    m_saveInFlight.swap(m_pendingRows);
    const QString filePath = m_filePath_log;
    const int currentCached = m_cachedRowCount;
    m_saveRunning.store(true, std::memory_order_release);

    // 后台线程执行 XLSX 的 open→write→save（耗时操作不阻塞 UI）
    QtConcurrent::run([this, filePath, currentCached]() {
        QVector<QStringList> rowsToWrite;
        std::swap(rowsToWrite, m_saveInFlight);

        QXlsx::Document xlsx(filePath);
        QString workbookError;
        if (!ensureWorkbookReady(xlsx, &workbookError)) {
            qWarning().noquote() << QStringLiteral("xlsx 后台保存-打开失败 | %1 | %2")
                                    .arg(toNativePath(filePath), workbookError);
            m_saveRunning.store(false, std::memory_order_release);
            return;
        }

        const QStringList sheetNames = xlsx.sheetNames();
        Worksheet* sheet =
            dynamic_cast<Worksheet*>(xlsx.sheet(sheetNames.constLast()));
        if (!sheet) {
            qWarning().noquote() << QStringLiteral("xlsx 后台保存-工作表为空 | %1")
                                    .arg(toNativePath(filePath));
            m_saveRunning.store(false, std::memory_order_release);
            return;
        }

        int nextRow = qMax(1, sheet->dimension().lastRow()) + 1;
        for (const QStringList& columns : rowsToWrite) {
            const int columnCount = qMin(columns.size(), kLogColumnCount);
            for (int column = 0; column < columnCount; ++column)
                sheet->write(nextRow, column + 1, columns.at(column));
            ++nextRow;
        }

        xlsx.setColumnWidth(1, 24);
        xlsx.setColumnWidth(2, 16);
        xlsx.setColumnWidth(3, 14);
        xlsx.setColumnWidth(4, 28);
        xlsx.setColumnWidth(5, 160);
        xlsx.setColumnWidth(6, 28);

        if (!xlsx.save()) {
            QMutexLocker locker(&m_mutex);
            ++m_consecutiveSaveFailures;
            qWarning().noquote() << QStringLiteral("xlsx 后台保存失败 | %1 | failures=%2")
                                    .arg(toNativePath(filePath))
                                    .arg(m_consecutiveSaveFailures);
            if (m_consecutiveSaveFailures >= kMaxConsecutiveFailures) {
                m_cachedRowCount = 0;
                m_consecutiveSaveFailures = 0;
                m_filePath_log.clear();
            }
        } else {
            QMutexLocker locker(&m_mutex);
            m_consecutiveSaveFailures = 0;
            m_cachedRowCount = nextRow - 1;
        }

        m_saveRunning.store(false, std::memory_order_release);
    });

    return true;
}

void xlsxHandle::scheduleFlush()
{
    if (m_flushTimer && !m_flushTimer->isActive())
        m_flushTimer->start(2000);
}

void xlsxHandle::flushAndWait()
{
    // 1. 等待后台保存完成（最长5秒）
    int waitCount = 0;
    while (m_saveRunning.load(std::memory_order_acquire) && waitCount < 50) {
        QThread::msleep(100);
        ++waitCount;
    }
    if (m_saveRunning.load(std::memory_order_acquire)) {
        qWarning() << "xlsx 后台保存未在5秒内完成，强制退出";
        m_saveRunning.store(false, std::memory_order_release);
    }

    // 2. 如果 m_saveInFlight 还有残留数据（后台线程未执行），合并回 pending
    {
        QMutexLocker locker(&m_mutex);
        if (!m_saveInFlight.isEmpty()) {
            m_pendingRows.append(m_saveInFlight);
            m_saveInFlight.clear();
        }
    }

    // 3. 同步写出所有残留数据（不抛后台，直接写）
    {
        QMutexLocker locker(&m_mutex);
        if (m_pendingRows.isEmpty() || m_filePath_log.isEmpty())
            return;

        QXlsx::Document xlsx(m_filePath_log);
        QString workbookError;
        if (!ensureWorkbookReady(xlsx, &workbookError)) {
            qWarning().noquote() << QStringLiteral("xlsx 退出刷盘-打开失败 | %1 | %2")
                                    .arg(toNativePath(m_filePath_log), workbookError);
            m_pendingRows.clear();
            return;
        }

        const QStringList sheetNames = xlsx.sheetNames();
        Worksheet* sheet =
            dynamic_cast<Worksheet*>(xlsx.sheet(sheetNames.constLast()));
        if (!sheet) {
            qWarning().noquote() << QStringLiteral("xlsx 退出刷盘-工作表为空 | %1")
                                    .arg(toNativePath(m_filePath_log));
            m_pendingRows.clear();
            return;
        }

        int nextRow = qMax(1, sheet->dimension().lastRow()) + 1;
        for (const QStringList& columns : m_pendingRows) {
            const int columnCount = qMin(columns.size(), kLogColumnCount);
            for (int column = 0; column < columnCount; ++column)
                sheet->write(nextRow, column + 1, columns.at(column));
            ++nextRow;
        }

        if (!xlsx.save()) {
            qWarning().noquote() << QStringLiteral("xlsx 退出刷盘-保存失败 | %1 | rows=%2")
                                    .arg(toNativePath(m_filePath_log))
                                    .arg(m_pendingRows.size());
        }

        m_pendingRows.clear();
        m_cachedRowCount = 0;
    }
}

bool xlsxHandle::appendRowLocked(const QStringList& columns, QString* errorMessage)
{
    if (m_filePath_log.isEmpty()) {
        if (errorMessage)
            *errorMessage = QStringLiteral("xlsx 日志文件路径为空，无法写入日志");
        return false;
    }

    QXlsx::Document xlsx(m_filePath_log);
    QString workbookError;
    if (!ensureWorkbookReady(xlsx, &workbookError)) {
        if (errorMessage) {
            *errorMessage = QStringLiteral("xlsx 日志文件打开失败 | %1 | %2")
                    .arg(toNativePath(m_filePath_log), workbookError);
        }
        return false;
    }

    const QStringList sheetNames = xlsx.sheetNames();
    Worksheet* sheet =
        dynamic_cast<Worksheet*>(xlsx.sheet(sheetNames.constLast()));
    if (!sheet) {
        if (errorMessage) {
            *errorMessage = QStringLiteral("xlsx 当前工作表为空，无法写入 | %1")
                    .arg(toNativePath(m_filePath_log));
        }
        return false;
    }

    const int lastRow = qMax(1, sheet->dimension().lastRow());
    const int targetRow = lastRow + 1;
    const int columnCount = qMin(columns.size(), kLogColumnCount);
    for (int column = 0; column < columnCount; ++column)
        sheet->write(targetRow, column + 1, columns.at(column));

    xlsx.setColumnWidth(1, 24);
    xlsx.setColumnWidth(2, 16);
    xlsx.setColumnWidth(3, 14);
    xlsx.setColumnWidth(4, 28);
    xlsx.setColumnWidth(5, 160);
    xlsx.setColumnWidth(6, 28);

    if (!xlsx.save()) {
        if (errorMessage) {
            *errorMessage = QStringLiteral("xlsx 日志写入失败 | %1 | row=%2")
                    .arg(toNativePath(m_filePath_log))
                    .arg(targetRow);
        }
        return false;
    }

    if (Logger::isVerboseEnabled()) {
        qDebug().noquote() << QStringLiteral("xlsx 日志写入完成 | %1 | row=%2")
                              .arg(toNativePath(m_filePath_log))
                              .arg(targetRow);
    }

    return true;
}

void xlsxHandle::createNewExcelFile()
{
    qDebug() << "xlsx" << QThread::currentThreadId();

    QMutexLocker locker(&m_mutex);
    QString errorMessage;
    if (!createNewExcelFileLocked(&errorMessage))
        qWarning().noquote() << errorMessage;
}
