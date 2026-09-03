#ifndef XLSXHANDLE_H
#define XLSXHANDLE_H
#include <QMutex>
#include <QObject>
#include <atomic>
#include "xlsxdocument.h"
#include "xlsxchartsheet.h"
#include "xlsxcellrange.h"
#include "xlsxchart.h"
#include "xlsxrichstring.h"
#include "xlsxworkbook.h"
#include <QtConcurrent/QtConcurrent>
#include <QTimer>
#include <QVector>
class xlsxHandle : public QObject
{
    Q_OBJECT
public:
    explicit xlsxHandle();
    ~xlsxHandle() override;
    QString currentLogFilePath() const;
public slots:
    void checkExcel();
    void insertData(QString data);
    void insertRecord(QString scope,
                      QString code,
                      QString step,
                      QString detail,
                      QString cycleId = QString());
    void createNewExcelFile();
    void flushAndWait();

private:
    bool appendRowLocked(const QStringList& columns, QString* errorMessage = nullptr);
    bool flushBufferLocked(QString* errorMessage = nullptr);
    bool ensureWorkbookReady(QXlsx::Document& xlsx, QString* errorMessage = nullptr);
    bool createNewExcelFileLocked(QString* errorMessage = nullptr);
    QString buildLogFilePath() const;
    void scheduleFlush();
    //文件地址
    QString m_filePath_log;
    mutable QMutex m_mutex;  //加锁
    QVector<QStringList> m_pendingRows;
    int m_cachedRowCount = 0;
    int m_consecutiveSaveFailures = 0;
    QTimer* m_flushTimer = nullptr;
    QVector<QStringList> m_saveInFlight;    // 正在后台保存的数据副本
    std::atomic_bool m_saveRunning{false};  // 后台保存进行中标志
};

#endif // XLSXHANDLE_H
