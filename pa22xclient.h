#ifndef PA22XCLIENT_H
#define PA22XCLIENT_H
#include <memory>
#include <map>
#include <QObject>
#include <QByteArray>
#include <QString>
#include <QMutex>
#include <chrono>
#include <QStringListModel>
#include <QTcpSocket>
#include <QVector>
#include <QStringList>
#include <stdio.h>
#include <QTimer>
#include "mtld/mtld.h"
#include "evaluator.h"
#include <QMessageBox>
//----------------------------------------------------------------------------
 extern double Mywave[];  // 2023.1.3 sumingxu
 extern double MywaveLeft[];
 extern double MywaveRight[];
const int kFixedPEChannelCount = 2;
//-----------------------------------------------------------------------------
//
typedef enum _DeviceType {
    kDeviceTypeUnknow,
    kDeviceTypePE,
    kDeviceTypePA
} DeviceType;
//------------------------------------------------------------------------------
//
#pragma pack(push, 1)
struct X22_DataHeader {
    uint32_t _device;		// 0 ~ 3
    union {					// 4 ~ 7
        uint32_t _group;
        uint32_t _channel;
    };
    uint8_t  rev[120];		// 8 ~ 127
};
#pragma pack(pop)

#pragma pack(push, 1)
struct X22_Encoder {
    uint32_t fwd;
    uint32_t rvs;
};
#pragma pack(pop)

#pragma pack(push, 1)
struct X22_Waveform {
    uint8_t  waveP[448];  // 0 ~ 447
    uint16_t path0;       // 448 449 闸门A声程
    uint8_t  amp0;        // 450 闸门A波高
    uint8_t  rev0;        // 451 保留
    uint16_t path1;       // 452 453
    uint8_t  amp1;        // 454
    uint8_t  rev1;        // 455
    uint16_t path2;       // 456 457
    uint8_t  amp2;        // 458
    uint8_t  rev2;        // 459
    uint16_t path3;       // 460 461
    uint8_t  amp3;        // 462
    uint8_t  rev3;        // 463
    uint8_t  ch;          // 464
    uint8_t  rev4[47];    // 465 ~ 511
    uint8_t  waveN[448];  // 512 ~ 959
    X22_Encoder enc[2];      // 960 ~ 975
    uint8_t  rev5[48];    // 976 ~ 1023
};
#pragma pack(pop)
//------------------------------------------------------------------------------
//
typedef enum _DualMode {
    kDualModeSingle = 0,
    kDualModeDual = 1,
} DualMode;
//------------------------------------------------------------------------------
//
typedef enum _Filter {
    kFilter0 = 0,
    kFilter1 = 1,
    kFilter2 = 2
} Filter;
//------------------------------------------------------------------------------
//
typedef enum _PAScanType {
    kPAScanTypeS = 0,
    kPAScanTypeL = 1,
    kPAScanTypeCustom = 2
} PAScanType;
//------------------------------------------------------------------------------
//
typedef enum _DampingMode {
    kDampingMode0 = 80,
    kDampingMode1 = 400,
} DampingMode;
//------------------------------------------------------------------------------
//
typedef enum _PECompresserMode {
    kPECompresserModeUnknow = 0,
    kPECompresserModeIgnored,
    kPECompresserModeAverage,
    kPECompresserModePeak
} PECompresserMode;
//------------------------------------------------------------------------------
//
typedef enum _RectificationMode {
    kRectificationFull = 0,
    kRectificationPositive,
    kRectificationNegative,
    kRectificationRF
} RectificationMode;
//------------------------------------------------------------------------------
//
typedef enum _GateMeasuresMode {
    kGateMeasuresModeEdge = 0,
    kGateMeasuresModePeak = 1,
} GateMeasuresMode;
//------------------------------------------------------------------------------
//
typedef struct _ScanRuleElement {
    int beamIndex;
    float angle;
    float x;
}  ScanRuleElement;
//------------------------------------------------------------------------------
//
typedef struct _ImageScale {
    float start;
    float end;
} ImageScale;

//------------------------------------------------------------------------------
//
class PA22XClient;

class UTShadowDevice {

public:
    const static int _PABeamsPerGroup = 128;
    const static int _PAMaxGroup = 4;
    const static int _PAImageWidth = 512;
    const static int _PAImageHeight = 400;
    const static int _PEWaveLength = 448;

    UTShadowDevice(const QString& identifier,
                   const QString& deviceNumber,
                   PA22XClient* commandDelegate);
    virtual ~UTShadowDevice();
    const QString& getIdentifier() const;
    const QString& getDeviceNumber() const;
    virtual void init() = 0;
    virtual QString parameterChanged(QString parameterString) = 0;
    virtual QString generateScanrule() = 0;
    virtual void copyImageData(int index, const unsigned char* data, size_t length) = 0;
    virtual void copyAWaveData(int index, const unsigned char* data, size_t length) = 0;
    virtual X22_Waveform* getAWaveData(int index) = 0;
    virtual unsigned char* getImageData(int index) = 0;
    virtual void appendParameters(QMap<QString, QString>& parameters);
    const QString getSelectedChannel();
    QString setSelectedChannel(QString channel);
    const QString getSelectedGroup();
    void setSelectedGroup(QString group);
    virtual QVector<QMap<QString, QString>>& getParameters();
    virtual QMap<QString, QString>& getSelectedParameter() = 0;
    virtual ScanRuleElement* getScanRule(int index) = 0;
    virtual ImageScale* getImageScales(int index) = 0;
    virtual QString setPRF(QString PRF);
    const QString getPRF();
    virtual QString setHighVoltage(QString highVoltage);
    const QString getHighVoltage();
    virtual int identifierToIndex(QString& identifier);
    void setTypeDescriptor(QString typeDescriptor);
    QString getTypeDescriptor();
    DeviceType getDeviceType();
    virtual unsigned int getScanruleBeamCount(int index);
    PA22XClient* getCommandDelegate();
    void lock();
    void unlock();
    virtual void logWaveData() = 0;
    virtual void logWaData() = 0;
    virtual void logSTData() = 0;
    virtual void WaveDataClose() = 0;
    virtual void DataSpecAnaly() = 0;

    void setChannelNumberCount(int count);
    int getChannelNumberCount() const;
protected:

    QMutex _lock;
    QString _selectedGroup;
    QString _selectedChannel;
    PA22XClient* _commandDelegate;
    QString _identifier;
    QString _deviceNumber;
    QString _typeDescriptor;
    QString _PRF;
    QString _highVoltage;
    QVector<QMap<QString, QString>> _parameters;
    FILE* _fp_log;

    bool _logWaveData;
    bool _logWave;
    bool _logWaveST;
    bool* _channelLogStates;
    int _channelNumberCount;
    QVector<int> stonewave;
    QVector<int> mywave;
};
//------------------------------------------------------------------------------
//
class UTShadowPADevice : public UTShadowDevice {
public:
    UTShadowPADevice(const QString& identifier,
                     const QString& deviceNumbe,
                     PA22XClient* commandDelegate);
    virtual ~UTShadowPADevice();
    virtual void init();
    virtual QString parameterChanged(QString parameterString);
    virtual QString generateScanrule();
    virtual void copyImageData(int index, const unsigned char* data, size_t length);
    virtual void copyAWaveData(int index, const unsigned char* data, size_t length);
    virtual ScanRuleElement* getScanRule(int index);
    virtual ImageScale* getImageScales(int index);
    virtual X22_Waveform* getAWaveData(int index);
    virtual unsigned char* getImageData(int index);
    QMap<QString, QString>& getSelectedParameter();
    virtual unsigned int getScanruleBeamCount(int index);
    virtual void logWaveData();
    virtual void logWaData();
    virtual void logSTData();
    virtual void WaveDataClose();
    virtual void DataSpecAnaly();

private:
    unsigned char _image[_PAImageHeight*_PAImageWidth];
    X22_Waveform _awaves[_PAMaxGroup*_PABeamsPerGroup];
    ScanRuleElement _scanRuleElements[_PAMaxGroup*_PABeamsPerGroup];
    unsigned int _scanRuleBeamCounts[_PAMaxGroup] {128, 128, 128, 128};
    ImageScale _imageScales[_PAMaxGroup];
};
//------------------------------------------------------------------------------
//
class UTShadowPEDevice : public UTShadowDevice {
public:
    UTShadowPEDevice(const QString& identifier,
                     const QString& deviceNumbe,
                     PA22XClient* commandDelegate);
    virtual ~UTShadowPEDevice();
    virtual int channels();
    virtual void init();
    virtual QString parameterChanged(QString parameterString);
    virtual QString generateScanrule();
    virtual void copyImageData(int index, const unsigned char* data, size_t length);
    virtual void copyAWaveData(int index, const unsigned char* data, size_t length);
    virtual ScanRuleElement* getScanRule(int index);
    virtual ImageScale* getImageScales(int index);
    virtual X22_Waveform* getAWaveData(int index);
    virtual unsigned char* getImageData(int index);
    QMap<QString, QString>& getSelectedParameter();
    virtual void logWaveData();
    virtual void logWaData();
    virtual void logSTData();
    virtual void WaveDataClose();
    virtual void DataSpecAnaly();
    virtual void logAWaveDataToFile(int index, struct X22_Waveform* waveform);
    virtual void logAWaDataToFile(int index, struct X22_Waveform* waveform);
    virtual void logASTDataToFile(int index, struct X22_Waveform* waveform);

protected:
    QVector<X22_Waveform*> _awaves;
};
//------------------------------------------------------------------------------
//
class UTShadowPE2Device : public UTShadowPEDevice {
public:
    const static int _maxChannel = 2;
    UTShadowPE2Device(const QString& identifier,
                      const QString& deviceNumbe,
                      PA22XClient* commandDelegate);
    virtual ~UTShadowPE2Device();
    virtual int channels();
};
//------------------------------------------------------------------------------
//
class UTShadowPE8Device : public UTShadowPEDevice {
public:
    const static int _maxChannel = 8;
    UTShadowPE8Device(const QString& identifier,
                      const QString& deviceNumber,
                      PA22XClient* commandDelegate);
    virtual ~UTShadowPE8Device();
    virtual int channels();
};
//------------------------------------------------------------------------------
//

//#define RUN_EVALUATOR
#define DATA_UDP_SOCKET
//#define DATA_TCP_SOCKET

class PA22XClient;

//-----------------------------------------------------------------------------
//
class UdpDataThread : public QThread
{
    Q_OBJECT

public:
    UdpDataThread(QObject *parent,
                  QHostAddress& address,
                  quint16 port);
    virtual ~UdpDataThread();
    virtual void run() Q_DECL_OVERRIDE;
    void SetClient(PA22XClient* client);
private slots:
    void processDatagram(QUdpSocket& socket);
private:
    PA22XClient* _client;
    mutable QMutex _clientMutex;
protected:
    QHostAddress _serverAddress;
    quint16 _port;
};
//-----------------------------------------------------------------------------
//
class PA22XClient : public QObject
{
    Q_OBJECT

public:
    PA22XClient(QObject* parent = nullptr);
    ~PA22XClient();
    bool connectToServer(const QString& serverIP);
    void disconnectFromServer();
    QString sendCommandSync(const QString& commandString);
    bool createShadowDevices();
    void deleteShadowDevices();
    QVector<UTShadowDevice*>& getShadowDevices();
    QString selectShadowDevice(const QString& identifier);
    UTShadowDevice* getSelectedShadowDevice();
    QString getPEPRF();
    QString setPEPRF(const QString& PRF);
    QString getPEMaxPRF();
    const QString updateTransmitSequenceMaxPRF();
    QString setPEChannelNumberCount(int count);
    QString lastErrorString() const;
#ifdef RUN_EVALUATOR
    QStringList getEvaluatorInfo();
    void clearEvaluatorInfo();
    NDTEvaluator* getEvaluator();
#endif

protected:
#ifdef RUN_EVALUATOR
    NDTEvaluator* _evaluator;
#endif
    QHostAddress* _serverAddress;
    QTcpSocket* _commandSocket;

#ifdef DATA_TCP_SOCKET
    QTcpSocket* _dataSocket;
    IMTLDReadWrite* _dataSource;
#endif

#ifdef DATA_UDP_SOCKET
    UdpDataThread* _dataThread;
#endif

    QVector<UTShadowDevice*> _shadowDevices;
    QString _selectedIdentifier;
    QString _PEPRF;
    QString _max_PEPRF;
    QString _lastError;

public slots:

#ifdef DATA_TCP_SOCKET
    bool readFromTCPDataSocket();
#endif

#ifdef DATA_UDP_SOCKET
    bool readFromUDPDataSocket();
#endif

private:
    bool handlePacket(MTLDPacket& packet);


};


class MockPA22XClient : public PA22XClient
{
    Q_OBJECT

public:
    explicit MockPA22XClient(QObject* parent = nullptr);
    ~MockPA22XClient();
    bool connectToServer(const QString& serverIP);
    void disconnectFromServer();
    QString sendCommandSync(const QString& commandString);
    bool createShadowDevices();
    QString setPEPRF(const QString& PRF);
    QString getPEMaxPRF();
    const QString updateTransmitSequenceMaxPRF();
    QString setPEChannelNumberCount(int count);

private slots:
    void generateSimulatedData();

private:
    QMap<QString, QString> createMockParameterMap(int channel) const;

    QTimer* _simulationTimer = nullptr;
    int _activeChannelCount = kFixedPEChannelCount;
    int _simulationTick = 0;
};


#endif // PA22XCLIENT_H
