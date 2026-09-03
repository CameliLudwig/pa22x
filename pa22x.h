#ifndef PA22X_H
#define PA22X_H
#include <QMainWindow>
#include <iostream>
#include <QFileDialog>
#include "ndtmath.h"
#include "fft.h"
#include <qcustomplot/qcustomplot.h>
#include <QPen>
#include <string>
#include <QString>
#include <QProcess>
#include <QTimer>
#include <QFile>
#include <windows.h>
#include <QDebug>
#include <QFileDialog>
#include <QStringList>

#include <stdio.h>
#include <math.h>
#include "logger.h"

#include <QTranslator>
#include <QDoubleSpinBox>
#include <QSpinBox>
#include <QComboBox>
#include <QCheckBox>
#include <QHBoxLayout>
#include <QResizeEvent>
#include <QShowEvent>
#include <QByteArray>
#include <QSound>
#include "mtld/mtld.h"
#include "pa22xclient.h"
#include "inversion.h"
#include "sysserialport.h"
#include <QtConcurrent/QtConcurrent>
#include <QScopedPointer>
#include <atomic>

QT_BEGIN_NAMESPACE
namespace Ui { class pa22x; }
QT_END_NAMESPACE

class pa22x : public QMainWindow
{
    Q_OBJECT

public:
    pa22x(QWidget *parent = nullptr);
    ~pa22x();
    //设置语言
    void setPA22xClient(PA22XClient* pa22xClient);
    QVector<double> fre;
    QVector<double> fre2;
    QVector<double> myfre;  //有效衰减谱频率
    QVector<double> amp;  //背景幅频曲线幅值
    QVector<double> amp2;  //浆液幅频曲线幅值
    int posl, posh;
    QVector<double> myspectrum;  //有效衰减谱
    //QVector<double> fitspectrum;  //拟合衰减谱
    double disfre;  //幅频曲线横坐标
    bool statime1 = false; //关闭时钟
    int flag1 = 0; //输出标志
    int flag2 = 0; //输出标志2
    int flag3 = 0; //输出标志3
    int count3 = 0; //d90输出计数
    bool specflag = false; //实验衰减谱判定
    QVector<double>rum;
    double avedv90 = 0.0; //dv90平均值
    double avepass = 0.0; //pass平均值
    double curr2 = 0.0; //电流输出
    int coun2 = 0;  //测不出计数，大于20时电流输出4mA
    bool avespecflag = false; //实验衰减谱平均值判定
    QString Parameter_Changed(QString parameter, double value);
    void updateTime(int seconds);


private slots:
    //槽函数
    //连接按钮
    void on_connectButton_clicked(bool checked);
    //保存参数
    void on_PESaveParameters_clicked();
    //加载参数
    void on_PELoadParameters_clicked();
    //启动
    void on_PESaveWave_clicked();
    //"停止"按钮，关闭时钟
    void on_CloseWaveSave_clicked();
    //检测
    void on_SpecAnaly_clicked(bool checked);
    //退出
    void on_Inserve_clicked();
    //背景信号
    void on_WaSaveWave_clicked();
    //保存石灰石浆液信号，单次保存，"信号保存"按钮
    void on_STSaveWave_clicked();
    //历史数据按钮
    void on_HisData_clicked();
    void PEChannelActived(int index);

    void on_PEChannelCountSelection_activated(int index);
    void on_deviceSelection_activated(int index);
    void on_pushButton_mode_clicked(bool checked);
    void on_PEGainInput_valueChanged(double arg1);
    void on_PERangeInput_valueChanged(double arg1);
    void on_PEDelayInput_valueChanged(double arg1);
    void on_PEDualModeSelection_activated(int index);
    void on_simulateDetectButton_clicked();

signals:
      void updatePlotData(double specfmin,double specfmax, double minsp ,double maxsp);
      void updateLabelData(QString name,QString data);
      void error(QString error);
      void check();
private slots:
      void onPlotDataUpdate(double specfmin,double specfmax, double minsp ,double maxsp);
      void onLabelDataUpdate(QString name,QString data);
      void onTimerCheck();
      void onerror(QString error);
      void onserialcheck();
private:
    Ui::pa22x *ui;
    //设置英文版本的初始化
    //void textDisplay();
    //显示函数
    void display();
    //设置初始化
    void inputBinding(QDoubleSpinBox* input,
                      double start,
                      double end,
                      QComboBox* steps,
                      QString formats,
                      int active);
    void inputBinding(QSpinBox* input,
                      int start,
                      int end,
                      QComboBox* steps,
                      QString formats,
                      int active);
    //输入
    void inputUpdate(QDoubleSpinBox* input,
                     QString value);
    void inputUpdate(QSpinBox* input,
                     QString value);
    void inputUpdate(QComboBox* selections,
                     QString value);
    void inputUpdate(QLineEdit* input,
                     QString text);
    void inputUpdate(QCheckBox* checkBox,
                     QString text);
    void timerEvent(QTimerEvent *) override;
    void loadUiSettingsFromDisk();
    void saveUiSettingsToDisk() const;
    void applyStaticTexts();
    void applyVisualTheme(bool lightMode);
    void applyWaveViewStyle();
    void refreshWaveCaptureButtonState();
    bool suspendDisplayRefresh();
    void resumeDisplayRefresh(bool wasRunning);
    void pumpUiEvents(int maxMs = 12) const;
    void initializeResponsiveLayout();
    void applyResponsiveLayout();
    void applyStartupPresentation();
    void toggleEscapePresentation();
    void registerEscapeHotKey();
    void unregisterEscapeHotKey();
    void clearStaleDetectionOutputFiles(bool announce = false);
    void normalizeLegacyParameterEditors();
    void alignParameterGroupRows();
    QSize currentViewportSize() const;
    void applyCompactLayoutIfNeeded();
    QString defaultFileDirectory() const;
    QString fileDialogPath(const QString& fileName = QString()) const;
    void rememberFileDialogPath(const QString& selectedPath);
    bool hasValidBackgroundSignal(QString* errorMessage = nullptr) const;
    bool attachClient(PA22XClient* client, bool simulationMode);
    void disconnectClient(bool showMessage, bool forShutdown = false);
    void refreshPEChannelCountSelectionUi(UTShadowDevice* selectedShadow);
    void refreshPEChannelSelectionUi(UTShadowDevice* selectedShadow);
    void applyLockedPeChannelUi();
    void enforceLockedPeChannelCount(UTShadowDevice* selectedShadow = nullptr, bool pushToDevice = false);
    int currentPeDualMode(UTShadowDevice* selectedShadow = nullptr) const;
    QString forcedPeChannelIdentifier(UTShadowDevice* selectedShadow = nullptr,
                                      int dualMode = -1) const;
    bool collectDisplayedWaveSamples(QVector<double>* samples,
                                     QString* channelIdentifier = nullptr,
                                     QString* errorMessage = nullptr) const;
    bool collectCurrentWaveSamples(QVector<double>* samples, QString* errorMessage = nullptr) const;
    bool collectDetectionWaveSamples(QVector<double>* samples,
                                     QString* sourceName = nullptr,
                                     QString* errorMessage = nullptr) const;
    bool writeBackgroundSignalFile(const QVector<double>& samples,
                                   const QString& channelIdentifier,
                                   QString* errorMessage = nullptr);
    bool captureBackgroundSignalFromDevice(int maxWaitMs, QString* errorMessage = nullptr);
    bool saveCurrentBackgroundSignal(QString* errorMessage = nullptr);
    bool ensureBackgroundSignalReady(int maxWaitMs,
                                     QString* errorMessage = nullptr,
                                     bool forceRefresh = false);
    bool refreshCurrentSpectrumPreview(bool writeSpectrumFile = false, QString* errorMessage = nullptr);
    void markBackgroundDirty(const QString& reason = QString(), bool clearSpectrum = true);
    QString buildCurrentParameterEntry(UTShadowDevice* device) const;
    bool applyParameterText(UTShadowDevice* device, const QString& parameterText, int* channelIndexOut = nullptr);
    bool loadHistoryFile(const QString& fileName);
    void runSimulationWorkflow();
    void finishSimulationWorkflow();
    void runSimulationWorkflowLegacy();
    void finishSimulationWorkflowLegacy();
    void generateSimulationInversionFiles(const QString& appDir) const;
    void setInternalToolsUnlocked(bool unlocked, bool announce = true);
    void resetDetectionSessionState(bool clearDisplayedLabels = false);
    //匹配设备标识符
    //DeviceType deviceTypeByIdentifier(QString identifier);


    PA22XClient* _pa22xClient = nullptr;
    int _displayTimer = 0;
    int _evaluatorTimer = 0;
    //QSound* _bells;
    bool is_start = 0;
    QTimer *m_timer = nullptr;
    bool m_timerActive = false;  // 新增状态标志
    std::atomic_bool m_analysisRunning{false};
    bool m_isSimulationMode = false;
    bool m_internalToolsUnlocked = false;
    bool m_simulationWorkflowRunning = false;
    int m_simulationExpectedCycles = 0;
    bool m_lightThemeActive = false;
    //QThread *m_timerThread = nullptr;
    QDateTime m_currentTime;
    QDateTime m_simulationStartedAt;

    bool m_isconnect = 0;
    bool m_compactLayoutApplied = false;
    qreal m_uiScaleFactor = 1.0;
    bool m_responsiveLayoutApplying = false;
    bool m_startupPresentationApplied = false;
    bool m_escapeHotKeyRegistered = false;
    bool m_connectOperationInProgress = false;
    bool m_shutdownInProgress = false;
    bool m_backgroundDirty = true;
    QString m_lastBackgroundDirtyReason;
    QDateTime m_lastBackgroundDirtyLoggedAt;
    QVector<double> m_displayedWaveSamples;
    QString m_displayedWaveChannelIdentifier;
    QDateTime m_lastDisplayedWaveCapturedAt;
    quint64 m_lastDetectionWaveSignature = 0;
    quint64 m_lastDetectionSpectrumSignature = 0;
    bool m_hasLastDetectionWaveSignature = false;
    bool m_hasLastDetectionSpectrumSignature = false;
    double m_lastValidD50 = 0.0;
    double m_lastValidD90 = 0.0;
    double m_lastValidPassrate = 0.0;
    bool m_hasLastValidSingleResult = false;
    QString m_lastTransientSerialError;
    QDateTime m_lastTransientSerialErrorAt;
    //串口
    SYSserialport * serial = nullptr;
    QPointer<QLineEdit> m_lineEditBiaoding;


protected:
    void closeEvent(QCloseEvent *e) override;
    void showEvent(QShowEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;
#ifdef Q_OS_WIN
    bool nativeEvent(const QByteArray &eventType, void *message, long *result) override;
#endif
private:
    void shutdown();  // 统一的收尾

};
#endif // PA22X_H
