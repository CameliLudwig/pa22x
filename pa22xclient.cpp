#include <QtEndian>
#include <QCoreApplication>
#include <QElapsedTimer>
#include <QEventLoop>
#include <QNetworkProxy>
#include <cmath>
#include <QDebug>
#include <string>
#include <iostream>
#include "pa22xclient.h"
#include <QtConcurrent/QtConcurrent>
//----------------------------------------------------------------------------
 double Mywave[448];  // 2023.1.3 sumingxu
 double MywaveLeft[448];
 double MywaveRight[448];
//-----------------------------------------------------------------------------

namespace {

constexpr double kMockPi = 3.14159265358979323846;

void pumpSocketWaitEvents()
{
    QCoreApplication::processEvents(QEventLoop::ExcludeUserInputEvents, 20);
}

bool waitForSocketConnectedResponsive(QAbstractSocket* socket, int timeoutMs)
{
    if (!socket)
        return false;
    if (socket->state() == QAbstractSocket::ConnectedState)
        return true;

    QElapsedTimer timer;
    timer.start();
    while (socket->state() != QAbstractSocket::ConnectedState && timer.elapsed() < timeoutMs) {
        const int remaining = qMax(1, timeoutMs - static_cast<int>(timer.elapsed()));
        socket->waitForConnected(qMin(100, remaining));
        pumpSocketWaitEvents();
    }
    return socket->state() == QAbstractSocket::ConnectedState;
}

void waitForSocketDisconnectedResponsive(QAbstractSocket* socket, int timeoutMs)
{
    if (!socket || socket->state() == QAbstractSocket::UnconnectedState)
        return;

    QElapsedTimer timer;
    timer.start();
    while (socket->state() != QAbstractSocket::UnconnectedState && timer.elapsed() < timeoutMs) {
        const int remaining = qMax(1, timeoutMs - static_cast<int>(timer.elapsed()));
        socket->waitForDisconnected(qMin(100, remaining));
        pumpSocketWaitEvents();
    }
}

void fillMockWaveform(X22_Waveform& waveform, int channelIndex, int tick)
{
    memset(&waveform, 0, sizeof(X22_Waveform));
    waveform.ch = static_cast<uint8_t>(channelIndex + 1);
    waveform.path0 = static_cast<uint16_t>(80 + channelIndex * 8);
    waveform.path1 = static_cast<uint16_t>(160 + channelIndex * 6);
    waveform.amp0 = static_cast<uint8_t>(100 + channelIndex * 5);
    waveform.amp1 = static_cast<uint8_t>(70 + channelIndex * 4);
    waveform.enc[0].fwd = static_cast<uint32_t>(tick * 25 + channelIndex * 3);
    waveform.enc[0].rvs = static_cast<uint32_t>(tick * 5 + channelIndex);

    for (int i = 0; i < 448; ++i) {
        const double x = static_cast<double>(i) / 448.0;
        const double center1 = 0.18 + 0.025 * channelIndex;
        const double center2 = 0.52 + 0.01 * std::sin((tick + channelIndex * 3) * 0.08);
        const double envelope1 = std::exp(-std::pow((x - center1) / 0.045, 2.0));
        const double envelope2 = std::exp(-std::pow((x - center2) / 0.065, 2.0));
        const double carrier1 = std::sin((18.0 + channelIndex) * x * 2.0 * kMockPi + tick * 0.03);
        const double carrier2 = std::cos((9.0 + channelIndex * 0.7) * x * 2.0 * kMockPi + tick * 0.02);
        const double value = (120.0 - channelIndex * 4.0) * envelope1 * carrier1
                           + (60.0 + channelIndex * 2.0) * envelope2 * carrier2;

        if (value >= 0.0) {
            waveform.waveP[i] = static_cast<uint8_t>(qBound(0, qRound(value), 255));
            waveform.waveN[i] = 0;
        } else {
            waveform.waveP[i] = 0;
            waveform.waveN[i] = static_cast<uint8_t>(qBound(0, qRound(-value), 255));
        }
    }
}

void resetWaveCaches()
{
    memset(Mywave, 0, sizeof(Mywave));
    memset(MywaveLeft, 0, sizeof(MywaveLeft));
    memset(MywaveRight, 0, sizeof(MywaveRight));
}

}

// 将字节数组转换为MTLD数据包
bool ConventBytesToMTLDPacket(QByteArray& byteArray,MTLDPacket& packet)
{
    // 最小长度校验: 4字节标记 + 7字节类型 + 4字节长度(仅用后3字节) + 1字节校验
    if (byteArray.size() < 16)
        return false;

    const unsigned char* bytes = reinterpret_cast<const unsigned char*>(byteArray.constData());

    // 检查字节数组的起始标识
    if (!(bytes[0] == 0x55 && bytes[1] == 0xaa && bytes[2] == 0x55 && bytes[3] == 0xaa))
        return false;

    // 解析数据包类型
    std::string type = std::string(reinterpret_cast<const char*>(&bytes[5]), 7);
    packet.set_type(type);

    // 解析数据包的原始大小 (3字节大端)
    int data_length = (bytes[13] << 16) | (bytes[14] << 8) | bytes[15];
    packet.set_packet_org_size(data_length);

    // 数据长度合法性检查
    if (data_length < 16 || data_length > byteArray.size())
        return false;

    packet.resize(data_length);

    // 复制数据到MTLD数据包
    memcpy(const_cast<unsigned char*>(packet.get_data()), byteArray.constData() + 16,
           static_cast<size_t>(data_length - 16));
    return true;
}

// UDP数据接收线程类
UdpDataThread::UdpDataThread(QObject *parent,
                             QHostAddress& address,
                             quint16 port)
    : QThread(parent),
      _serverAddress(address),
      _port(port)
{

}

// UDP数据接收线程类析构函数
UdpDataThread::~UdpDataThread()
{
    _client = nullptr;
}
void UdpDataThread::processDatagram(QUdpSocket& socket)
{
    PA22XClient* client = nullptr;
    {
        QMutexLocker locker(&_clientMutex);
        client = _client;
    }
    if (!client) {
        return;
    }

    QHostAddress remoteAddr;
    quint16 remotePort;

    QByteArray receive_data;
    receive_data.resize(socket.pendingDatagramSize());

    socket.readDatagram(receive_data.data(), receive_data.size(), &remoteAddr, &remotePort);

    if (client) {
        MTLDPacket packet;
        if (!ConventBytesToMTLDPacket(receive_data, packet)) return;

        X22_DataHeader* header = (X22_DataHeader*)packet.get_data();
        const unsigned char* data = packet.get_data() + sizeof(X22_DataHeader);

        // 原始数据处理逻辑（保持不变）
        if (packet.get_type() == QString("elewave")) {
            qDebug() << "elewave";
        } else {
            for (UTShadowDevice* shadowDevice : client->getShadowDevices()) {
                if (shadowDevice->getDeviceNumber() == QString::number(header->_device)) {
                    if (packet.get_type() == QString("awavepa")) {
                        // 复制awavepa类型数据到影子设备中
                        int size = UTShadowDevice::_PABeamsPerGroup *
                                UTShadowDevice::_PAMaxGroup *
                                sizeof(X22_Waveform);
                        shadowDevice->lock();
                        shadowDevice->copyAWaveData(-1, data, size);
                        shadowDevice->unlock();
                    }
                    else if (packet.get_type() == QString("elewave")) {
                        // 处理elewave类型数据
                        shadowDevice->lock();
                        qDebug() << "elewave";
                        shadowDevice->unlock();
                    }
                    else if (packet.get_type() == QString("simg512")) {
                        // 复制simg512类型数据到影子设备中
                        int size = UTShadowDevice::_PAImageWidth *
                                UTShadowDevice::_PAImageHeight;
                        shadowDevice->lock();
                        shadowDevice->copyImageData(-1, data, size);
                        shadowDevice->unlock();
                    }
                    else if (packet.get_type() == QString("awavepe")) {
                        // 复制awavepe类型数据到影子设备中的特定通道
                        uint32_t channelIndex = header->_channel - 1;
                        int size = sizeof(X22_Waveform);
                        shadowDevice->lock();
                        shadowDevice->copyAWaveData(channelIndex, data, size);
                        shadowDevice->unlock();

                        // 在运行评估器的条件下，通知评估器有新数据到达
#ifdef RUN_EVALUATOR
                        if (client->getEvaluator())
                            client->getEvaluator()->Hit(QString::number(header->_device) + ":" +
                                            QString::number(channelIndex));
#endif
                    }
                }
            }
        }
    }
}

void UdpDataThread::run()
{
    QUdpSocket socket;
    if (!socket.bind(QHostAddress::Any, _port,
                     QUdpSocket::ShareAddress | QUdpSocket::ReuseAddressHint)) {
        qWarning() << "UdpDataThread bind failed:" << socket.errorString() << "port=" << _port;
        return;
    }

    socket.writeDatagram("GIVE_ME_DATA", _serverAddress, _port);

    connect(&socket, &QUdpSocket::readyRead, [&socket, this]() {
        while (socket.hasPendingDatagrams()) {
            if (isInterruptionRequested())
                return;
            processDatagram(socket);
        }
    });

    exec(); // 启动事件循环
}

// 设置客户端对象的函数，将传入的客户端对象指针赋值给成员变量 _client
void UdpDataThread::SetClient(PA22XClient* client)
{
    QMutexLocker locker(&_clientMutex);
    _client = client;
}

// PA22XClient 类的构造函数，初始化对象及其成员变量
PA22XClient::PA22XClient(QObject* parent)
    : QObject(parent),
      _serverAddress(nullptr),    // 初始化服务器地址为 nullptr
      _commandSocket(nullptr)     // 初始化命令套接字为 nullptr
{
    // 如果定义了 DATA_TCP_SOCKET 宏，初始化 TCP 相关成员
#ifdef DATA_TCP_SOCKET
    _dataSocket = nullptr;       // 如果定义了 DATA_TCP_SOCKET，初始化数据套接字为 nullptr
    _dataSource = nullptr;       // 如果定义了 DATA_TCP_SOCKET，初始化数据源为 nullptr
#endif
    // 如果定义了 DATA_UDP_SOCKET 宏，初始化 UDP 线程相关成员
#ifdef DATA_UDP_SOCKET
    _dataThread = nullptr;       // 如果定义了 DATA_UDP_SOCKET，初始化数据线程为 nullptr
#endif
    // 如果定义了 RUN_EVALUATOR 宏，创建并启动评估器
#ifdef RUN_EVALUATOR
    _evaluator = new NDTEvaluator(nullptr);   // 如果定义了 RUN_EVALUATOR，创建并初始化评估器对象
    _evaluator->Run();                        // 如果定义了 RUN_EVALUATOR，启动评估器
#endif
}

// PA22XClient 类的析构函数，释放对象资源
PA22XClient::~PA22XClient()
{
#ifdef RUN_EVALUATOR
    if (_evaluator) {
        _evaluator->Stop();     // 如果定义了 RUN_EVALUATOR，停止评估器
        delete _evaluator;      // 如果定义了 RUN_EVALUATOR，释放评估器对象内存
    }
#endif
    disconnectFromServer();     // 断开与服务器的连接
}
bool PA22XClient::connectToServer(const QString& serverIP)
{
disconnectFromServer();  // 断开当前的服务器连接
    _lastError.clear();
    resetWaveCaches();
    const QString trimmedServerIP = serverIP.trimmed();

    bool commandSocketConnected = true;  // 命令套接字连接状态，默认为连接成功

#ifdef DATA_TCP_SOCKET
    bool dataSocketConnected = true;     // 数据套接字连接状态，默认为连接成功
#endif

    _serverAddress = new QHostAddress();  // 创建服务器地址对象
    if (!_serverAddress->setAddress(trimmedServerIP)) {
        _lastError = QString("invalid ip: %1").arg(trimmedServerIP);
        disconnectFromServer();
        return false;
    }

    // 创建并连接命令套接字
    _commandSocket = new QTcpSocket(this);
    _commandSocket->setProxy(QNetworkProxy::NoProxy);
    _commandSocket->connectToHost(*_serverAddress, 2202);  // 连接服务器的命令端口
    if (!waitForSocketConnectedResponsive(_commandSocket, 2000)) {
        commandSocketConnected = false;  // 如果连接失败，则设置命令套接字连接状态为失败
        _lastError = QString("connect command port failed %1:2202, %2")
                .arg(trimmedServerIP, _commandSocket->errorString());
    }

#ifdef DATA_TCP_SOCKET
    // 创建并连接数据套接字（如果定义了 DATA_TCP_SOCKET）
    _dataSocket = new QTcpSocket(this);
    _dataSocket->setProxy(QNetworkProxy::NoProxy);
    connect(_dataSocket, SIGNAL(readyRead()), this, SLOT(readFromTCPDataSocket()));  // 连接数据套接字的数据就绪信号
    _dataSocket->connectToHost(*_serverAddress, 2201);  // 连接服务器的数据端口
    if (!waitForSocketConnectedResponsive(_dataSocket, 2000)) {
        dataSocketConnected = false;  // 如果连接失败，则设置数据套接字连接状态为失败
        if (_lastError.isEmpty()) {
            _lastError = QString("connect data port failed %1:2201, %2")
                    .arg(trimmedServerIP, _dataSocket->errorString());
        }
    }
    if (dataSocketConnected) {
        _dataSource = new MTLDQTcpSocketReadWrite(_dataSocket, MM_READ);  // 创建数据源对象并开始读取数据
        if (_dataSource)
            _dataSource->begin();  // 开始数据读取
    }
#endif

    // 如果命令套接字和（如果定义了）数据套接字都连接成功
    if (commandSocketConnected
#ifdef DATA_TCP_SOCKET
            && dataSocketConnected
#endif
) {
        createShadowDevices();
        if (_shadowDevices.isEmpty()) {
            if (_lastError.isEmpty())
                _lastError = QStringLiteral("no available device found from sync");
            disconnectFromServer();
            return false;
        }

#ifdef DATA_UDP_SOCKET
        // 创建并启动 UDP 数据线程（如果定义了 DATA_UDP_SOCKET）
        _dataThread = new UdpDataThread(this, *_serverAddress, 2201);
        if (_dataThread) {
            _dataThread->SetClient(this);  // 设置客户端对象
            // 当 UDP 数据线程完成后，自动删除线程对象
            connect(_dataThread, &UdpDataThread::finished, _dataThread, &QObject::deleteLater);
            _dataThread->start();  // 启动 UDP 数据线程
        }
#endif
        return true;  // 返回连接成功
    }

    disconnectFromServer();  // 如果连接失败，则断开服务器连接

    return false;  // 返回连接失败
}


#ifdef DATA_TCP_SOCKET
bool PA22XClient::readFromTCPDataSocket()
{
    MTLDPacket packet;  // 创建一个MTLDPacket对象
    X22_DataHeader *header;  // X22_DataHeader结构体指针
    const unsigned char* data;  // 数据指针

    auto error = _dataSource->read(packet);  // 从数据源中读取数据到packet

    if (error != ME_SUCCESS)  // 如果读取失败，返回false
        return false;

    header = (X22_DataHeader *)packet.get_data();  // 获取数据包头部指针
    data = packet.get_data() + sizeof(X22_DataHeader);  // 获取数据指针，跳过头部

    if (packet.get_type() == QString("elewave")) {  // 如果数据包类型是"elewave"
        qDebug() << "elewave " << packet.get_size();  // 输出调试信息
        FILE* fp = fopen("data.dat", "w+");  // 打开一个文件句柄
        if (fp) {  // 如果文件打开成功
            int size = packet.get_size() - 16;  // 计算数据大小
            int16_t* ele_waveform = (int16_t*)&data[16];  // 获取电波数据
            for (int i = 0; i < size/2; i ++) {  // 遍历数据并写入文件
                fprintf (fp, "%d\n", ele_waveform[i]);
            }
            fclose(fp);  // 关闭文件
        }
    }
    else {  // 如果数据包类型不是"elewave"
        for (UTShadowDevice* shadowDevice : _shadowDevices) {  // 遍历影子设备列表
            if (shadowDevice->getDeviceNumber() == QString::number(header->_device)) {  // 如果设备号匹配
                if (packet.get_type() == QString("awavepa")) {  // 如果数据包类型是"awavepa"
                    int size = UTShadowDevice::_PABeamsPerGroup *
                            UTShadowDevice::_PAMaxGroup *
                            sizeof(X22_Waveform);  // 计算数据大小
                    shadowDevice->lock();  // 锁定设备
                    shadowDevice->copyAWaveData(-1, data, size);  // 复制A波形数据到设备
                    shadowDevice->unlock();  // 解锁设备
                }
                else if (packet.get_type() == QString("simg512")) {  // 如果数据包类型是"simg512"
                    int size = UTShadowDevice::_PAImageWidth *
                            UTShadowDevice::_PAImageHeight;  // 计算数据大小
                    shadowDevice->lock();  // 锁定设备
                    shadowDevice->copyImageData(-1, data, size);  // 复制图像数据到设备
                    shadowDevice->unlock();  // 解锁设备
                }
                else if (packet.get_type() == QString("awavepe")) {  // 如果数据包类型是"awavepe"
                    uint32_t channelIndex = header->_channel - 1;  // 获取通道索引
                    int size = sizeof(X22_Waveform);  // 计算数据大小
                    shadowDevice->lock();  // 锁定设备
                    shadowDevice->copyAWaveData(channelIndex, data, size);  // 复制A波形数据到设备指定通道
                    shadowDevice->unlock();  // 解锁设备

    #ifdef RUN_EVALUATOR
                    if (_evaluator)  // 如果评估器存在
                        _evaluator->Hit(QString::number(header->_device) + ":" +
                                        QString::number(channelIndex));  // 记录命中信息
    #endif
                }
            }
        }
    }

    return true;  // 返回处理成功
}
#endif

#ifdef DATA_UDP_SOCKET
bool PA22XClient::readFromUDPDataSocket()
{
    return false;
}
#endif

#ifdef RUN_EVALUATOR
QStringList PA22XClient::getEvaluatorInfo()
{
    if (_evaluator)  // 如果评估器存在
        return _evaluator->getInfo();  // 返回评估器信息
}
//------------------------------------------------------------------------------
//
void PA22XClient::clearEvaluatorInfo()
{
    if (_evaluator)  // 如果评估器存在
        _evaluator->clearInfo();  // 清除评估器信息
}
//------------------------------------------------------------------------------
//
NDTEvaluator* PA22XClient::getEvaluator()
{
    return _evaluator;  // 返回评估器指针
}
#endif

void PA22XClient::disconnectFromServer()
{
    if (_commandSocket) {  // 如果命令套接字存在
        if (_commandSocket->state() == QAbstractSocket::ConnectedState)  // 如果命令套接字处于连接状态
            _commandSocket->disconnectFromHost();  // 断开连接
        _commandSocket->close();
        _commandSocket->deleteLater();  // 释放命令套接字内存
        _commandSocket = nullptr;
    }

#ifdef DATA_TCP_SOCKET
    if (_dataSource) {  // 如果数据源存在
        _dataSource->end();  // 终止数据源操作
        delete _dataSource;  // 释放数据源内存
        _dataSource = nullptr;
    }

    if (_dataSocket) {  // 如果数据套接字存在
        if (_dataSocket->state() == QAbstractSocket::ConnectedState)  // 如果数据套接字处于连接状态
            _dataSocket->disconnectFromHost();  // 断开连接
        disconnect(_dataSocket,  // 解除数据套接字的信号槽连接
                   SIGNAL(readyRead()),
                   this,
                   SLOT(readFromTCPDataSocket()));
        delete _dataSocket;  // 释放数据套接字内存
        _dataSocket = nullptr;
    }
#endif

#ifdef DATA_UDP_SOCKET
    if (_dataThread) {  // 如果数据线程存在
        _dataThread->SetClient(nullptr);
        _dataThread->requestInterruption();  // 请求中断线程运行
        _dataThread->quit();
        if (!_dataThread->wait(3000)) {
            qWarning() << "UdpDataThread shutdown timeout, forcing terminate";
            _dataThread->terminate();
            _dataThread->wait(500);
        }
        _dataThread->deleteLater();  // 确保线程对象最终被释放
    }
    _dataThread = nullptr;  // 置空数据线程指针
#endif

    if (_serverAddress) {  // 如果服务器地址存在
        delete _serverAddress;  // 释放服务器地址内存
        _serverAddress = nullptr;
    }

    deleteShadowDevices();  // 删除影子设备
}


QString PA22XClient::sendCommandSync(const QString& commandString)
{
    if (!_commandSocket || _commandSocket->state() != QAbstractSocket::ConnectedState) {
        _lastError = QString("命令套接字已断开连接，命令=%1").arg(commandString);
        return _lastError;
    }

    const QByteArray payload = (commandString + "\r\n\r\n").toLatin1();
    _commandSocket->readAll();
    if (_commandSocket->write(payload) == -1 ||
        !_commandSocket->waitForBytesWritten(1000)) {
        _lastError = QString("命令发送失败: %1").arg(commandString);
        return _lastError;
    }

    QString raw;
    while (_commandSocket->waitForReadyRead(100)) {
        raw.append(_commandSocket->readAll());
        if (raw.right(4) == "\r\n\r\n") {
            QStringList rawList = raw.split("\r\n\r\n", QString::SkipEmptyParts);
            QString response = rawList[0].simplified();
            if (response.contains(QRegularExpression("OK\\s+close_dev", QRegularExpression::CaseInsensitiveOption))) {
                _lastError = QString("服务端主动关闭设备连接");
                disconnectFromServer();
                return _lastError;
            }
            _lastError.clear();
            return response;
        }
    }

    _lastError = QString("命令响应超时: %1").arg(commandString);
    return _lastError;
}

bool PA22XClient::createShadowDevices()
{
    deleteShadowDevices();

    auto commandFailed = [](const QString& response) {
        return response.startsWith(QStringLiteral("命令")) ||
               response.startsWith(QStringLiteral("服务端")) ||
               response.startsWith(QStringLiteral("响应")) ||
               response.startsWith(QStringLiteral("IP地址")) ||
               response.startsWith("ERR", Qt::CaseInsensitive);
    };

    QString response;
    response = sendCommandSync("get sync");
    if (commandFailed(response)) {
        if (_lastError.isEmpty())
            _lastError = QString("获取设备同步信息失败: %1").arg(response);
        return false;
    }
    response.remove("ok sync", Qt::CaseInsensitive);

    QJsonParseError parseError;
    QJsonDocument jsonDocument = QJsonDocument::fromJson(response.toUtf8(), &parseError);
    if (parseError.error != QJsonParseError::NoError || !jsonDocument.isObject()) {
        _lastError = QString("同步信息解析失败: %1").arg(response.left(200));
        return false;
    }
    QJsonObject jsonObject = jsonDocument.object();
    QVariantMap clientMap = jsonObject.toVariantMap();

    // 获取通道数量
    int channel_count = 0;
    QRegularExpression re_tsm_seq("\\d{5}");
    QString tsm_seq = sendCommandSync("get tsm_seq");
    if (commandFailed(tsm_seq)) {
        if (_lastError.isEmpty())
            _lastError = QString("获取通道序列失败: %1").arg(tsm_seq);
        return false;
    }
    QRegularExpressionMatchIterator i = re_tsm_seq.globalMatch(tsm_seq);
    while (i.hasNext()) {
        QRegularExpressionMatch match = i.next();
        if (match.hasMatch())
            channel_count ++;
    }
    ////////////////////////////////////////////////////////////////////////////

    QList<QVariant> devices = clientMap["devices"].toList(); // 获取设备列表
    for (QVariant device : devices) {
        UTShadowDevice* shadow = nullptr;

        QVariantMap deviceMap = device.toMap(); // 获取设备信息的VariantMap

        QString identifier = deviceMap["identifier"].toString(); // 获取设备标识符
        QString high_voltage = deviceMap["high_voltage"].toString(); // 获取高压信息

        QString type;
        QString deviceNumber;
        QList<QVariant> groups;
        QList<QVariant> channels;

        QRegularExpression re("(PE2|PE8|PA)Device_(\\d+)"); // 正则表达式匹配设备标识符
        QRegularExpressionMatch match = re.match(identifier);
        if (match.hasMatch()) {
            type = match.captured(1); // 获取设备类型
            deviceNumber = match.captured(2); // 获取设备编号
        }

        if (!type.compare("PE2", Qt::CaseInsensitive) || !type.compare("PE8", Qt::CaseInsensitive)) {
            if (!type.compare("PE2", Qt::CaseInsensitive))
                shadow = new UTShadowPE2Device(identifier, deviceNumber, this); // 创建UTShadowPE2Device实例
            else if (!type.compare("PE8", Qt::CaseInsensitive))
                shadow = new UTShadowPE8Device(identifier, deviceNumber, this); // 创建UTShadowPE8Device实例

            channels = deviceMap["channels"].toList(); // 获取通道列表
            for (int i = 0; i < channels.length(); i ++) {
                QMap<QString, QString> _p;
                QVariantMap parameters = channels[i].toMap(); // 获取通道参数的VariantMap
                QList<QString> keys = parameters.keys();
                for (QString key : keys) {
                    QString value = parameters.value(key).toString();
                    _p.insert(key, value); // 将通道参数插入参数映射中
                }
                shadow->appendParameters(_p); // 将通道参数添加到影子设备中
            }

            QString selected_channel = deviceMap["selected_channel"].toString(); // 获取选定通道
            shadow->setSelectedChannel(selected_channel); // 设置选定通道
            shadow->setChannelNumberCount(channel_count); // 设置通道数量
        }
        else if (!type.compare("PA", Qt::CaseInsensitive)) {
            shadow = new UTShadowPADevice(identifier, deviceNumber, this); // 创建UTShadowPADevice实例

            groups = deviceMap["groups"].toList(); // 获取分组列表
            for (int i = 0; i < groups.length(); i ++) {
                QMap<QString, QString> _p;
                QVariantMap parameters = groups[i].toMap(); // 获取分组参数的VariantMap
                QList<QString> keys = parameters.keys();
                for (QString key : keys) {
                    QString value = parameters.value(key).toString();
                    _p.insert(key, value); // 将分组参数插入参数映射中
                }
                shadow->appendParameters(_p); // 将分组参数添加到影子设备中
            }

            QString selected_group = deviceMap["selected_group"].toString(); // 获取选定分组
            QString PRF = deviceMap["PRF"].toString(); // 获取PRF信息
            QString type_descriptor = deviceMap["type_descriptor"].toString(); // 获取类型描述信息

            shadow->setPRF(PRF); // 设置PRF信息
            shadow->setSelectedGroup(selected_group); // 设置选定分组
            shadow->setTypeDescriptor(type_descriptor); // 设置类型描述信息
        }

        if (shadow) {
            shadow->setHighVoltage(high_voltage); // 设置高压信息
            shadow->init(); // 初始化影子设备
            _shadowDevices.append(shadow); // 将影子设备添加到影子设备列表中
        }
    }

    _PEPRF = clientMap["PEPRF"].toString(); // 获取PEPRF信息
    _selectedIdentifier = clientMap["selected_device"].toString(); // 获取选定设备标识符信息
if (_shadowDevices.isEmpty() && _lastError.isEmpty()) {
        _lastError = QStringLiteral("sync completed but no valid shadow device created");
    }

    return true;
}

QString PA22XClient::lastErrorString() const
{
    return _lastError;
}

QString PA22XClient::getPEPRF()
{
    return _PEPRF;
}

QString PA22XClient::setPEPRF(const QString& PRF)
{
    _PEPRF = PRF; // 设置PEPRF成员变量为传入的PRF值
    return sendCommandSync(QString("set prf %1").arg(_PEPRF)); // 发送设置PRF命令，并返回响应
}

const QString PA22XClient::updateTransmitSequenceMaxPRF()
{
    QString response = sendCommandSync(QString("get pe_max_transmit_sequence_prf")); // 发送获取PE最大传输序列PRF命令
    QRegularExpression re("[0-9]+"); // 匹配数字的正则表达式
    QRegularExpressionMatch match = re.match(response); // 在响应中查找匹配
    if (match.hasMatch())
        _max_PEPRF = match.captured(0); // 如果找到匹配，将匹配到的值保存到_max_PEPRF中
    return response; // 返回响应字符串
}

QString PA22XClient::getPEMaxPRF()
{
    return _max_PEPRF; // 返回保存的PE最大PRF值
}

void PA22XClient::deleteShadowDevices()
{
    qDeleteAll(_shadowDevices); // 删除_shadowDevices列表中所有的影子设备对象，释放内存
    _shadowDevices.clear(); // 清空_shadowDevices列表
}

QVector<UTShadowDevice*>& PA22XClient::getShadowDevices()
{
    return _shadowDevices; // 返回_shadowDevices列表的引用，用于访问所有影子设备对象
}

QString PA22XClient::selectShadowDevice(const QString& identifier)
{
    QString response = ""; // 初始化响应字符串
    _selectedIdentifier = identifier; // 设置选定的设备标识符为传入的标识符
    for (UTShadowDevice* shadow : _shadowDevices) { // 遍历所有影子设备
        if (shadow->getIdentifier() == _selectedIdentifier) { // 如果设备标识符匹配选定的标识符
            sendCommandSync(QString("set dev_select %1").arg(shadow->getDeviceNumber())); // 发送选择设备命令

            if (shadow->getDeviceType() == kDeviceTypePE) { // 如果设备类型为PE
                response += sendCommandSync(QString("set pa_data_stop")); // 发送停止PA数据传输命令
                response += "\n";
                response += sendCommandSync(QString("set pe_data_start")); // 发送启动PE数据传输命令
                response += "\n";
            }
            else if (shadow->getDeviceType() == kDeviceTypePA) { // 如果设备类型为PA
                response += sendCommandSync(QString("set pe_data_stop")); // 发送停止PE数据传输命令
                response += "\n";
                response += sendCommandSync(QString("set pa_data_start")); // 发送启动PA数据传输命令
                response += "\n";
            }
        }
    }
    return response; // 返回所有操作的响应字符串
}

UTShadowDevice* PA22XClient::getSelectedShadowDevice()
{
    for (UTShadowDevice* shadowDevice : _shadowDevices) { // 遍历所有影子设备
        if (shadowDevice->getIdentifier() == _selectedIdentifier) // 如果设备标识符匹配选定的标识符
            return shadowDevice; // 返回选定的影子设备对象指针
    }
    return nullptr; // 如果未找到匹配的影子设备，返回空指针
}

// 设置PE通道数目，并返回执行命令的响应信息
QString PA22XClient::setPEChannelNumberCount(int count)
{
    QString commandString = "set tsm_seq "; // 构造命令字符串
    QString response = ""; // 存储响应信息的字符串

    // 根据通道数目设置命令字符串的不同参数
    switch (count) {
    case 1:
        commandString += "10000";
        break;
    case 2:
        commandString += "10000 20000";
        break;
    case 3:
        commandString += "10000 20000 30000";
        break;
    case 4:
        commandString += "10000 20000 30000 40000";
        break;
    case 5:
        commandString += "10000 20000 30000 40000 50000";
        break;
    case 6:
        commandString += "10000 20000 30000 40000 50000 60000";
        break;
    case 7:
        commandString += "10000 20000 30000 40000 50000 60000 70000";
        break;
    case 8:
        commandString += "10000 20000 30000 40000 50000 60000 70000 80000";
        break;
    default:
        commandString += "10000 20000 30000 40000 50000 60000 70000 80000";
        break;
    }

    // 发送命令并获取响应
    response = sendCommandSync(commandString);
    response += "\n";

    // 获取当前选定的影子设备和其通道标识符
    UTShadowDevice* device = getSelectedShadowDevice();
    QString deviceIdentifier = device->getIdentifier();
    QString channelIdentifier = device->getSelectedChannel();

    // 遍历所有影子设备，对类型为kDeviceTypePE的设备进行配置
    for (UTShadowDevice* device : _shadowDevices) {
        if (device->getDeviceType() == kDeviceTypePE) {
            // 设置设备的通道数目
            device->setChannelNumberCount(count);
            // 选择当前设备并配置其参数
            response += selectShadowDevice(device->getIdentifier());
            for (QMap<QString, QString> parameter : device->getParameters()) {
                QString identifier = parameter["identifier"];
                response += device->setSelectedChannel(identifier);
                response += "\n";
                response += device->parameterChanged("range:100");
                response += "\n";
                response += device->parameterChanged("delay:0");
                response += "\n";
                response += device->parameterChanged("origin:0");
                response += "\n";
            }
        }
    }

    // 恢复原先选定的影子设备和其通道
    response += selectShadowDevice(deviceIdentifier);
    response += device->setSelectedChannel(channelIdentifier);

    // 设置PEPRF为"100"，更新最大传输序列PRF
    response += setPEPRF("100");
    updateTransmitSequenceMaxPRF();

    return response; // 返回执行结果的响应信息
}

// UTShadowDevice类的构造函数，初始化成员变量并打开日志文件
UTShadowDevice::UTShadowDevice(const QString& identifier,
                               const QString& deviceNumber,
                               PA22XClient* commandDelegate)
    : _identifier(identifier),
      _deviceNumber(deviceNumber),
      _commandDelegate(commandDelegate)
{
    _logWave = false;
    _logWaveST = false;
    _logWaveData = false;
    _typeDescriptor = "?"; // 初始化类型描述为问号

    QString logFileName = identifier + ".log"; // 日志文件名为标识符加上.log后缀
    _fp_log = fopen(logFileName.toLatin1().data(), "w"); // 打开日志文件
}

// UTShadowDevice类的析构函数，释放日志文件和通道日志状态数组的资源
UTShadowDevice::~UTShadowDevice()
{
    if (_fp_log)
        fclose(_fp_log); // 关闭日志文件

    if (_channelLogStates)
        free(_channelLogStates); // 释放通道日志状态数组的内存
}

// 获取影子设备的标识符
const QString& UTShadowDevice::getIdentifier() const
{
    return _identifier;
}

// 获取影子设备的设备编号
const QString& UTShadowDevice::getDeviceNumber() const
{
    return _deviceNumber;
}

void UTShadowDevice::appendParameters(QMap<QString, QString>& parameters)
{
    _parameters.append(parameters);  // 将参数追加到成员变量_parameters中
}

const QString UTShadowDevice::getSelectedChannel()
{
    return _selectedChannel;  // 返回当前选定的通道
}

QString UTShadowDevice::setSelectedChannel(QString channel)
{
    //_logWaveData = false;  //关闭数据连续保存功能
    _selectedChannel = channel;  // 设置选定的通道为参数channel
    QString response = "";  // 存储命令响应信息的字符串
    QRegularExpression re("[0-9]+");  // 匹配数字的正则表达式
    QRegularExpressionMatch match = re.match(_selectedChannel);  // 在_selectedChannel中查找匹配项
    if (match.hasMatch()) {
        QString channelNumber = match.captured(0);  // 获取匹配的通道号
        response = _commandDelegate->sendCommandSync(QString("set ch_select %1").arg(channelNumber));  // 发送选择通道的命令并获取响应
    }
    return response;  // 返回命令执行的响应信息
}

const QString UTShadowDevice::getSelectedGroup()
{
    return _selectedGroup;  // 返回当前选定的组
}

void UTShadowDevice::setSelectedGroup(QString group)
{
    _selectedGroup = group;  // 设置选定的组为参数group

    QRegularExpression re("[0-9]+");  // 匹配数字的正则表达式
    QRegularExpressionMatch match = re.match(_selectedGroup);  // 在_selectedGroup中查找匹配项
    if (match.hasMatch()) {
        QString groupNumber = match.captured(0);  // 获取匹配的组号
        _commandDelegate->sendCommandSync(QString("set group_select %1").arg(groupNumber));  // 发送选择组的命令
    }
}

QString UTShadowDevice::setPRF(QString PRF)
{
    _PRF = PRF;  // 设置PRF参数
    return _commandDelegate->sendCommandSync(QString("set prf %1").arg(_PRF));  // 发送设置PRF命令并返回响应
}

QString UTShadowDevice::setHighVoltage(QString highVoltage)
{
    _highVoltage = highVoltage;  // 设置高压参数
    return _commandDelegate->sendCommandSync(QString("set high_voltage %1").arg(_highVoltage));  // 发送设置高压命令并返回响应
}

const QString UTShadowDevice::getPRF()
{
    return _PRF;  // 返回当前PRF参数
}

const QString UTShadowDevice::getHighVoltage()
{
    return _highVoltage;  // 返回当前高压参数
}

QVector<QMap<QString, QString>>& UTShadowDevice::getParameters()
{
    return _parameters;  // 返回存储参数的向量的引用
}

int UTShadowDevice::identifierToIndex(QString& identifier)
{
    int index = 0;  // 初始化索引为0
    QRegularExpression re("(?:PEDevice|PADevice|group|channel)_([0-9]+)");  // 匹配设备、组或通道的标识符的正则表达式
    QRegularExpressionMatch match = re.match(identifier);  // 在identifier中查找匹配项
    if (match.hasMatch())
        index = match.captured(1).toInt() - 1;  // 获取匹配的数字并转换为索引（减1是因为索引从0开始）
    return index;  // 返回计算出的索引值
}

void UTShadowDevice::setTypeDescriptor(QString typeDescriptor)
{
    _typeDescriptor = typeDescriptor;  // 设置设备类型描述符
}

QString UTShadowDevice::getTypeDescriptor()
{
    return _typeDescriptor;  // 返回设备类型描述符
}

DeviceType UTShadowDevice::getDeviceType()
{
    QRegularExpression re("(PE2|PE8|PA)Device_");  // 匹配设备类型的正则表达式
    QRegularExpressionMatch match = re.match(_identifier);  // 在_identifier中查找匹配项
    QString type;

    if (match.hasMatch())
        type = match.captured(1);  // 获取匹配到的设备类型

    if (!type.compare("PE2", Qt::CaseInsensitive) ||
            !type.compare("PE8", Qt::CaseInsensitive))
        return kDeviceTypePE;  // 如果是PE2或PE8设备，返回对应的设备类型枚举值
    else if (!type.compare("PA", Qt::CaseInsensitive))
        return kDeviceTypePA;  // 如果是PA设备，返回对应的设备类型枚举值

    return kDeviceTypeUnknow;  // 如果未匹配到有效设备类型，返回未知设备类型枚举值
}

unsigned int UTShadowDevice::getScanruleBeamCount(int index)
{
    return 0;  // 返回扫描规则中指定索引的束数（此处始终返回0，可能是需要实现的逻辑）
}

PA22XClient* UTShadowDevice::getCommandDelegate()
{
    return _commandDelegate;  // 返回命令委托对象的指针
}

void UTShadowDevice::lock()
{
    _lock.lock();  // 执行锁定操作，用于多线程同步
}

void UTShadowDevice::unlock()
{
    _lock.unlock();  // 执行解锁操作，用于多线程同步
}

void UTShadowDevice::setChannelNumberCount(int count)
{
    _channelNumberCount = count;  // 设置通道数目计数器

    if (_channelLogStates)
        free(_channelLogStates);  // 如果通道状态数组已分配内存，释放其内存空间

    _channelLogStates = (bool*)malloc(sizeof(bool) * _channelNumberCount);  // 根据新的通道数目重新分配通道状态数组的内存空间
    for (int i = 0; i < _channelNumberCount; i++)
        _channelLogStates[i] = false;  // 初始化所有通道状态为未记录
}

int UTShadowDevice::getChannelNumberCount() const
{
    return _channelNumberCount;
}

UTShadowPADevice::UTShadowPADevice(const QString& identifier,
                                   const QString& deviceNumber,
                                   PA22XClient* commandDelegate)
    : UTShadowDevice(identifier, deviceNumber, commandDelegate)
{
    // PA设备的构造函数，调用基类UTShadowDevice的构造函数进行初始化
}

UTShadowPADevice::~UTShadowPADevice()
{
    // PA设备的析构函数，没有具体实现内容
}
void UTShadowPADevice::init()
{
    QString response = "";
    response = _commandDelegate->sendCommandSync(QString("set dev_select %1").arg(_deviceNumber));

    QVector<QMap<QString, QString>>& parameters = getParameters();  // 获取参数列表的引用
    for (QMap<QString, QString>& parameter : parameters) {
        QString groupIdentifier = parameter["identifier"];  // 获取参数的标识符
        int groupNumber = 0;
        QRegularExpression re("group_([0-9]+)");  // 匹配组号的正则表达式
        QRegularExpressionMatch match = re.match(groupIdentifier);
        if (match.hasMatch())
            groupNumber = match.captured(1).toInt();  // 提取匹配到的组号并转换为整数

        int groupIndex = groupNumber - 1;

        response = _commandDelegate->sendCommandSync(QString("set group_select %1").arg(groupNumber));
        response = _commandDelegate->sendCommandSync(QString("get scan_rule"));

        QStringList splited = response.split(' ', QString::SkipEmptyParts);  // 按空格拆分响应字符串
        ScanRuleElement* p = &_scanRuleElements[groupIndex * _PABeamsPerGroup];  // 计算扫描规则元素的起始位置

        _scanRuleBeamCounts[groupIndex] = (splited.count() - 2) / 3;  // 计算组内扫描规则元素的数量
        for (int i = 0; i < _scanRuleBeamCounts[groupIndex]; i++) {
            p[i].beamIndex = splited.value(2 + 3 * i + 0).toInt() - 1;  // 设置束索引
            p[i].angle = splited.value(2 + 3 * i + 1).toDouble();  // 设置角度
            p[i].x = splited.value(2 + 3 * i + 2).toDouble();  // 设置x坐标
        }

        response = _commandDelegate->sendCommandSync(QString("get img_scale"));
        splited = response.split(' ', QString::SkipEmptyParts);  // 再次按空格拆分响应字符串
        _imageScales[groupIndex].start = splited.value(2).toDouble();  // 设置图像缩放起始值
        _imageScales[groupIndex].end = splited.value(3).toDouble();  // 设置图像缩放结束值
    }
}

QString UTShadowPADevice::parameterChanged(QString parameterString)
{
    QString parameterName;
    QString value;
    QRegularExpression re("([a-zA-Z0-9_-]+):([a-zA-Z0-9_.]+)");  // 匹配参数字符串的正则表达式
    QRegularExpressionMatch match = re.match(parameterString);
    if (match.hasMatch()) {
        parameterName = match.captured(1);  // 提取参数名称
        value = match.captured(2);  // 提取参数值
    }

    QVector<QMap<QString, QString>>& parameters = getParameters();  // 获取参数列表的引用
    for (QMap<QString, QString>& parameter : parameters) {
        if (parameter["identifier"] == _selectedGroup) {  // 查找当前选定组的参数
            parameter[parameterName] = value;  // 更新参数值
            break;
        }
    }

    QString commandString = QString("set %1 %2").arg(parameterName).arg(value);  // 构造设置参数的命令字符串

    QString response = _commandDelegate->sendCommandSync(commandString);  // 发送命令并同步获取响应
    return response;  // 返回响应结果
}

QString UTShadowPADevice::generateScanrule()
{
    QString response = _commandDelegate->sendCommandSync(QString("get scan_rule"));

    QString groupIdentifier = getSelectedGroup();
    QVector<QMap<QString, QString>>& parameters = getParameters();
    for (QMap<QString, QString>& parameter : parameters) {
        if (parameter["identifier"] == groupIdentifier) {          

            _commandDelegate->sendCommandSync(QString("set pa_data_stop"));

            _commandDelegate->sendCommandSync(
                        QString("set scan_type %1").
                            arg(parameter["scan_type"])
                    );

            _commandDelegate->sendCommandSync(
                        QString("set lscan_ele %1 %2").
                            arg(parameter["start_element"]).
                            arg(parameter["stop_element"])
                    );

            _commandDelegate->sendCommandSync(
                        QString("set velocity %1 %2").
                            arg(parameter["longitude_velocity"]).
                            arg(parameter["shear_velocity"])
                    );

            qDebug () << parameter["aperture_from"] << " " << parameter["aperture_size"];
            _commandDelegate->sendCommandSync(
                        QString("set aperture %1 %2").
                            arg(parameter["aperture_from"]).
                            arg(parameter["aperture_size"])
                    );

            _commandDelegate->sendCommandSync(
                        QString("set probe %1 %2 %3 %4").
                            arg(parameter["probe_freq"]).
                            arg(parameter["probe_element_count"]).
                            arg(parameter["probe_element_distance"]).
                            arg(parameter["probe_delay"])
                     );

            _commandDelegate->sendCommandSync(
                        QString("set wedge %1 %2 %3 %4 %5").
                            arg(parameter["wedge_used"]).
                            arg(parameter["wedge_shear_wave"]).
                            arg(parameter["wedge_angle"]).
                            arg(parameter["wedge_velocity"]).
                            arg(parameter["wedge_element_height"])
                     );

            _commandDelegate->sendCommandSync(
                        QString("set angle %1 %2").
                            arg(parameter["angle_from"]).
                            arg(parameter["angle_to"])
                      );

            _commandDelegate->sendCommandSync(
                        QString("set focus %1").
                            arg(parameter["focus"])
                      );

            _commandDelegate->sendCommandSync(QString("set scan_rule"));
            _commandDelegate->sendCommandSync(QString("set smooth true"));
            _commandDelegate->sendCommandSync(QString("set pa_data_start"));

            // get scan rule and related

            int groupIndex = 0;
            QRegularExpression re("group_([0-9]+)");
            QRegularExpressionMatch match = re.match(groupIdentifier);
            if (match.hasMatch())
                groupIndex = match.captured(1).toInt()-1;

            QString response = _commandDelegate->sendCommandSync(QString("get scan_rule"));
            QStringList splited  = response.split(' ', QString::SkipEmptyParts);
            ScanRuleElement* p = &_scanRuleElements[groupIndex*_PABeamsPerGroup];

            _scanRuleBeamCounts[groupIndex] = (splited.count() - 2) / 3;
            for (int i = 0; i < _scanRuleBeamCounts[groupIndex]; i++) {
                p[i].beamIndex = splited.value(2 + 3*i + 0).toInt() - 1;
                p[i].angle = splited.value(2 + 3*i + 1).toDouble();
                p[i].x = splited.value(2 + 3*i + 2).toDouble();
            }

            response = _commandDelegate->sendCommandSync(QString("get img_scale"));
            splited = response.split(' ', QString::SkipEmptyParts);
            _imageScales[groupIndex].start = splited.value(2).toDouble();
            _imageScales[groupIndex].end = splited.value(3).toDouble();

            return response;
        }
    }

    return response;
}
unsigned int UTShadowPADevice::getScanruleBeamCount(int index)
{
    return _scanRuleBeamCounts[index];  // 返回指定索引处的扫描规则元素数量
}

void UTShadowPADevice::copyImageData(int index,
                                     const unsigned char* data,
                                     size_t length)
{
    (void)index;  // 防止未使用参数的编译警告
    memset(_image, 0, length);  // 将图像数据区域清零
    memcpy(_image, data, length);  // 将输入数据复制到图像数据区域
}

void UTShadowPADevice::copyAWaveData(int index,
                                     const unsigned char* data,
                                     size_t length)
{
    (void)index;  // 防止未使用参数的编译警告
    memset(_awaves, 0, length);  // 将A波数据区域清零
    memcpy(_awaves, data, length);  // 将输入数据复制到A波数据区域
}

X22_Waveform* UTShadowPADevice::getAWaveData(int index)
{
    return &_awaves[index*_PABeamsPerGroup];  // 返回指定索引处的A波数据起始地址
}

unsigned char* UTShadowPADevice::getImageData(int index)
{
    (void)index;  // 防止未使用参数的编译警告
    return _image;  // 返回图像数据区域的起始地址
}

QMap<QString, QString>& UTShadowPADevice::getSelectedParameter()
{
    for (QMap<QString, QString>& parameter : _parameters) {
        if (parameter["identifier"] == _selectedGroup) {
            return parameter;  // 返回选定组的参数映射
        }
    }
    return _parameters[0];  // 如果未找到选定组的参数，则返回第一个参数映射
}

ScanRuleElement* UTShadowPADevice::getScanRule(int index)
{
    return &_scanRuleElements[index*_PABeamsPerGroup];  // 返回指定索引处的扫描规则元素数组起始地址
}

ImageScale* UTShadowPADevice::getImageScales(int index)
{
    return &_imageScales[index];  // 返回指定索引处的图像缩放信息结构体地址
}

void UTShadowPADevice::logWaveData()
{
    // 记录波形数据的日志（未实现具体功能）
}

void UTShadowPADevice::logWaData()
{
    // 记录W数据的日志（未实现具体功能）
}

void UTShadowPADevice::logSTData()
{
    // 记录ST数据的日志（未实现具体功能）
}

void UTShadowPADevice::WaveDataClose()
{
    // 关闭波形数据（未实现具体功能）
}

UTShadowPEDevice::UTShadowPEDevice(const QString& identifier,
                                   const QString& deviceNumber,
                                   PA22XClient* commandDelegate)
    : UTShadowDevice(identifier, deviceNumber, commandDelegate)
{
}

UTShadowPEDevice::~UTShadowPEDevice()
{
    _logWaveData = 0;
    _logWave = 0;
    _logWaveST = 0;
    qDeleteAll(_awaves);  // 删除所有A波数据对象
    _awaves.clear();  // 清空A波数据数组
}

int UTShadowPEDevice::channels()
{
    return 0;  // 返回通道数为0（未实现具体功能）
}

void UTShadowPEDevice::init()
{
    qDeleteAll(_awaves);
    _awaves.clear();
    for (int i = 0; i < channels(); i ++) {
        X22_Waveform* awave = new X22_Waveform();  // 创建新的A波数据对象
        _awaves.append(awave);  // 将A波数据对象添加到数组中
    }
}


QString UTShadowPEDevice::parameterChanged(QString parameterString)
{
    QString parameterName;  // 参数名称
    QString value;  // 参数值
    QRegularExpression re("([a-zA-Z0-9_-]+):([a-zA-Z0-9_.]+)");
    QRegularExpressionMatch match = re.match(parameterString);  // 使用正则表达式匹配参数字符串
    if (match.hasMatch()) {
        parameterName = match.captured(1);  // 获取参数名称
        value = match.captured(2);  // 获取参数值
    }

    QVector<QMap<QString, QString>>& parameters = getParameters();  // 获取设备参数列表的引用
    for (QMap<QString, QString>& parameter : parameters) {
        if (parameter["identifier"] == _selectedChannel) {  // 根据选定的通道标识符查找匹配的参数
            parameter[parameterName] = value;  // 更新参数值
            break;
        }
    }

    QString commandString = QString("set %1 %2").
            arg(parameterName).
            arg(value);  // 构造设置参数的命令字符串

    QString response = _commandDelegate->sendCommandSync(commandString);  // 发送同步命令并获取响应
    return response;  // 返回命令响应
}

QString UTShadowPEDevice::generateScanrule()
{
    return QString("PEDevice doesn't support' scanrule");  // 返回不支持扫描规则的提示信息
}

void UTShadowPEDevice::copyImageData(int index,
                                     const unsigned char* data,
                                     size_t length)
{
    (void)index;  // 防止未使用参数的编译警告
    (void)data;  // 防止未使用参数的编译警告
    (void)length;  // 防止未使用参数的编译警告
    // 由于该设备不支持图像数据复制操作，因此此处没有实现具体功能
}
void UTShadowPEDevice::copyAWaveData(int index,
                                     const unsigned char* data,
                                     size_t length)
{
    if (index < 0 || index >= _awaves.size())
        return;

    struct X22_Waveform* wave_form = (struct X22_Waveform*)data;

    // 检查是否打开了日志文件并且波形数据的通道号为1
    if (_fp_log && wave_form->ch == 1) {
        // 在这里可以添加日志输出代码，例如：
        // qDebug() <<  wave_form->enc[0].fwd << " " << wave_form->enc[0].rvs;
        // fprintf(_fp_log, "encoder: %d %d, ", wave_form->enc[0].fwd, wave_form->enc[0].rvs);
        // fprintf(_fp_log, "data: ");
        // for (int i = 0; i < 400; i ++)
        //     fprintf (_fp_log, "%d ", wave_form->waveP[i]);
        // fprintf (_fp_log, "\n");
    }

    // 将波形数据复制到内部数组中的特定索引位置
    memcpy(_awaves[index], data, length);

    //背景信号  检查是否需要记录波形数据到文件中，根据日志状态和通道状态进行判断
    if (_logWave && _channelLogStates) {
        if (_channelLogStates[index] == false) {
            logAWaDataToFile(index, wave_form);
            _channelLogStates[index] = true;
        }

        // 检查是否还有未记录的通道，若全部通道均已记录则停止日志记录
        bool continueLogging = false;
        for (int i = 0; i < _channelNumberCount; i++) {
            if (_channelLogStates[i] == false) {
                continueLogging = true;
                break;
            }
        }

        if (!continueLogging)
            _logWave = false;
    }

    // 石灰石信号单次保存逻辑
    if (_logWaveST && _channelLogStates) {
        if (_channelLogStates[index] == false) {
            logASTDataToFile(index, wave_form);
            _channelLogStates[index] = true;
        }

        // 检查是否还有未记录的通道，若全部通道均已记录则停止单次保存
        bool continueLogging = false;
        for (int i = 0; i < _channelNumberCount; i++) {
            if (_channelLogStates[i] == false) {
                continueLogging = true;
                break;
            }
        }

        if (!continueLogging)
            _logWaveST = false;
    }

    // 石灰石信号-连续保存逻辑
    if (_channelLogStates) {
        bool continueLogging = false;
        for (int i = 0; i < channels(); i++) {
            if (_channelLogStates[i] == false)
                continueLogging = true;
        }

        // 若所有通道数据已记录完毕且正在进行连续保存，则停止连续保存
        if (continueLogging == false && _logWaveData) {
            _logWaveData = false;
            qDebug() << "log wave data stop";
        }
    }

    // 如果需要记录波形数据，则调用相关函数进行记录，并标记通道数据已记录
    if (_logWaveData) {
        logAWaveDataToFile(index, wave_form);
        if (_channelLogStates)
            _channelLogStates[index] = true;
    }
}


void UTShadowPEDevice::logAWaDataToFile(int index, struct X22_Waveform* waveform)
{
//    QString currentTimeString = QDateTime::currentDateTime()
//                .toString("yyyyMMdd_hhmmss");

//    QString logFilePath = _identifier + "_"
//            +  "CHANNEL_" + QString::number(index) + "_"
//            + currentTimeString + ".TXT";

    // 获取exe所在目录
    QString appDir = QCoreApplication::applicationDirPath();
    QString filePath = appDir + "/boxing_water.txt";

    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::critical(nullptr, "文件写入失败",
            QString("无法创建文件\n路径: ") + filePath);
        return;
    }

    QTextStream out(&file);
    out.setCodec("UTF-8");  // 设置编码格式

    int length = 448;
    for (int i = 0; i < length; ++i) {
        int data = 0;

        if (waveform->waveP[i] >= waveform->waveN[i]) {
            data = waveform->waveP[i];
            stonewave << data;
        } else {
            data = -waveform->waveN[i];
            stonewave << data;
        }

        out << data << "\n";  // 写入数据并换行
    }

    file.close();
}

void UTShadowPEDevice::logASTDataToFile(int index, struct X22_Waveform* waveform)
{
    Q_UNUSED(index);
    QString currentTimeString = QDateTime::currentDateTime()
            .toString("yyyyMMdd_hhmmss");

    QString logFilePath = QCoreApplication::applicationDirPath()
            + "/WaveRf_" + currentTimeString + ".txt";

    QFile file(logFilePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        qWarning() << "Cannot open stone wave file:" << logFilePath;
        return;
    }

    QTextStream out(&file);
    const int length = 448;
    for (int i = 0; i < length; i++) {
        int data = 0;
        if (waveform->waveP[i] >= waveform->waveN[i]) {
            data = waveform->waveP[i];
        } else {
            data = -waveform->waveN[i];
        }
        stonewave << data;
        out << data << "\n";
    }
    file.close();
}
void UTShadowPEDevice::logAWaveDataToFile(int index, struct X22_Waveform* waveform)
{
    //2023.1.3 sumingxu
    // 对波形数据进行处理，将符合条件的数据保存到Mywave数组中
    for (int i = 0; i < 448; i ++) {
        int data = 0;

        if (waveform->waveP[i] >= waveform->waveN[i]) {
            data = waveform->waveP[i];
        }
        else if (waveform->waveP[i] < waveform->waveN[i]) {
            data = -waveform->waveN[i];
        }
        Mywave[i] = data;
    }

    // 防止stonewave无限增长 (每次只保留最新448个点)
    if (stonewave.size() > 100000)
        stonewave.remove(0, stonewave.size() - 448);
}

X22_Waveform* UTShadowPEDevice::getAWaveData(int index)
{
    if (index < 0 || index >= _awaves.size())
        return nullptr;
    return _awaves[index];
}

unsigned char* UTShadowPEDevice::getImageData(int index)
{
    return nullptr;  // 返回空指针，表示没有图像数据
}

QMap<QString, QString>& UTShadowPEDevice::getSelectedParameter()
{
    // 遍历_parameters数组，根据_selectedChannel找到对应的参数并返回
    for (QMap<QString, QString>& parameter : _parameters) {
        if (parameter["identifier"] == _selectedChannel) {
            return parameter;
        }
    }
    return _parameters[0];  // 如果没有找到对应的参数，则返回_parameters数组的第一个元素的引用
}

ScanRuleElement* UTShadowPEDevice::getScanRule(int index)
{
    return nullptr;  // 返回空指针，表示没有扫描规则数据
}

ImageScale* UTShadowPEDevice::getImageScales(int index)
{
    return nullptr;  // 返回空指针，表示没有图像比例尺数据
}

void UTShadowPEDevice::logWaveData()
{
    _logWaveData = true;  // 开启波形数据记录标志
    if (_channelLogStates) {
        for (int i = 0; i < _channelNumberCount; i ++)
            _channelLogStates[i] = false;  // 关闭所有通道的记录状态
    }
}

void UTShadowPEDevice::logWaData()
{
    if (_channelLogStates) {
        _logWave = true;  // 开启波数据记录标志
        for (int i = 0; i < _channelNumberCount; i ++)
        {
            _channelLogStates[i] = false;
        }
    }
}

void UTShadowPEDevice::logSTData()
{
    if (_channelLogStates) {
        _logWaveST = true;  // 开启ST波数据记录标志
        for (int i = 0; i < _channelNumberCount; i ++)
            _channelLogStates[i] = false;
    }
}


UTShadowPE2Device::UTShadowPE2Device(const QString& identifier,
                                     const QString& deviceNumbe,
                                     PA22XClient* commandDelegate)
    : UTShadowPEDevice(identifier, deviceNumbe, commandDelegate)
{
    init();  // 调用初始化函数
}

UTShadowPE2Device::~UTShadowPE2Device()
{
    // 析构函数
}

int UTShadowPE2Device::channels()
{
    return UTShadowPE2Device::_maxChannel;  // 返回通道数目
}

UTShadowPE8Device::UTShadowPE8Device(const QString& identifier,
                                     const QString& deviceNumbe,
                                     PA22XClient* commandDelegate)
    : UTShadowPEDevice(identifier, deviceNumbe, commandDelegate)
{
    init();  // 调用初始化函数
}

UTShadowPE8Device::~UTShadowPE8Device()
{
    // 析构函数
}

int UTShadowPE8Device::channels()
{
    return UTShadowPE8Device::_maxChannel;  // 返回通道数目
}

void UTShadowPEDevice::WaveDataClose()
{
    _logWaveData = false;  // 关闭波形数据记录标志
}

void UTShadowPADevice::DataSpecAnaly()
{
    // 空函数，未实现功能
}

void UTShadowPEDevice::DataSpecAnaly()
{
    qDebug() << 1122;  // 输出调试信息
}

MockPA22XClient::MockPA22XClient(QObject* parent)
    : PA22XClient(parent)
{
    _PEPRF = QStringLiteral("100");
    _max_PEPRF = QStringLiteral("4000");
}

MockPA22XClient::~MockPA22XClient()
{
    disconnectFromServer();
}

bool MockPA22XClient::connectToServer(const QString& serverIP)
{
    Q_UNUSED(serverIP);
    disconnectFromServer();
    _serverAddress = new QHostAddress(QHostAddress::LocalHost);
    createShadowDevices();

    _simulationTimer = new QTimer(this);
    connect(_simulationTimer, &QTimer::timeout, this, &MockPA22XClient::generateSimulatedData);
    _simulationTimer->start(40);
    generateSimulatedData();
    return true;
}

void MockPA22XClient::disconnectFromServer()
{
    if (_simulationTimer) {
        _simulationTimer->stop();
        _simulationTimer->deleteLater();
        _simulationTimer = nullptr;
    }

    deleteShadowDevices();

    if (_serverAddress) {
        delete _serverAddress;
        _serverAddress = nullptr;
    }

    _selectedIdentifier.clear();
}

QString MockPA22XClient::sendCommandSync(const QString& commandString)
{
    if (commandString == QStringLiteral("get tsm_seq")) {
        QStringList tokens;
        for (int i = 0; i < kFixedPEChannelCount; ++i)
            tokens << QString::number((i + 1) * 10000);
        return tokens.join(" ");
    }

    if (commandString == QStringLiteral("get activable_channels"))
        return QString::number(kFixedPEChannelCount);

    if (commandString == QStringLiteral("get pe_max_transmit_sequence_prf"))
        return _max_PEPRF;

    if (commandString.startsWith(QStringLiteral("set prf "))) {
        _PEPRF = commandString.section(' ', 2, 2).trimmed();
        return QStringLiteral("OK");
    }

    if (commandString == QStringLiteral("set data_stop") ||
        commandString == QStringLiteral("set data_start")) {
        return QStringLiteral("OK");
    }

    if (commandString.startsWith(QStringLiteral("set activable_channels "))) {
        _activeChannelCount = kFixedPEChannelCount;
        for (UTShadowDevice* device : _shadowDevices)
            device->setChannelNumberCount(qMin(kFixedPEChannelCount, device->getParameters().size()));
        return QStringLiteral("OK");
    }

    if (commandString.startsWith(QStringLiteral("set dev_select "))) {
        const QString deviceNumber = commandString.section(' ', 2, 2).trimmed();
        for (UTShadowDevice* device : _shadowDevices) {
            if (device->getDeviceNumber() == deviceNumber) {
                _selectedIdentifier = device->getIdentifier();
                break;
            }
        }
        return QStringLiteral("OK");
    }

    if (commandString.startsWith(QStringLiteral("set ch_select ")) ||
        commandString.startsWith(QStringLiteral("set high_voltage "))) {
        return QStringLiteral("OK");
    }

    return QStringLiteral("OK");
}

bool MockPA22XClient::createShadowDevices()
{
    deleteShadowDevices();

    UTShadowPE8Device* shadow = new UTShadowPE8Device(QStringLiteral("PE8Device_1"),
                                                      QStringLiteral("1"), this);
    for (int channel = 1; channel <= UTShadowPE8Device::_maxChannel; ++channel) {
        QMap<QString, QString> parameter = createMockParameterMap(channel);
        shadow->appendParameters(parameter);
    }

    shadow->setHighVoltage(QStringLiteral("200"));
    shadow->setSelectedChannel(QStringLiteral("channel_1"));
    shadow->setChannelNumberCount(kFixedPEChannelCount);
    shadow->init();

    _shadowDevices.append(shadow);
    _selectedIdentifier = shadow->getIdentifier();
    return true;
}

QString MockPA22XClient::setPEPRF(const QString& PRF)
{
    _PEPRF = PRF;
    return QStringLiteral("OK");
}

QString MockPA22XClient::getPEMaxPRF()
{
    return _max_PEPRF;
}

const QString MockPA22XClient::updateTransmitSequenceMaxPRF()
{
    _max_PEPRF = QStringLiteral("4000");
    return _max_PEPRF;
}

QString MockPA22XClient::setPEChannelNumberCount(int count)
{
    Q_UNUSED(count);
    _activeChannelCount = kFixedPEChannelCount;
    return PA22XClient::setPEChannelNumberCount(_activeChannelCount);
}

void MockPA22XClient::generateSimulatedData()
{
    ++_simulationTick;
    for (UTShadowDevice* device : _shadowDevices) {
        UTShadowPEDevice* peDevice = dynamic_cast<UTShadowPEDevice*>(device);
        if (!peDevice)
            continue;

        for (int channel = 0; channel < _activeChannelCount; ++channel) {
            X22_Waveform waveform;
            fillMockWaveform(waveform, channel, _simulationTick);
            device->lock();
            peDevice->copyAWaveData(channel,
                                    reinterpret_cast<const unsigned char*>(&waveform),
                                    sizeof(X22_Waveform));
            device->unlock();
        }
    }
}

QMap<QString, QString> MockPA22XClient::createMockParameterMap(int channel) const
{
    QMap<QString, QString> parameter;
    parameter.insert(QStringLiteral("identifier"), QStringLiteral("channel_%1").arg(channel));
    parameter.insert(QStringLiteral("gain"), QStringLiteral("38.0"));
    parameter.insert(QStringLiteral("pulse_width"), QStringLiteral("120"));
    parameter.insert(QStringLiteral("range"), QStringLiteral("100"));
    parameter.insert(QStringLiteral("delay"), QStringLiteral("0"));
    parameter.insert(QStringLiteral("origin"), QStringLiteral("0"));
    parameter.insert(QStringLiteral("filter"), QStringLiteral("0"));
    parameter.insert(QStringLiteral("dual_mode"), QStringLiteral("0"));
    parameter.insert(QStringLiteral("damping"), QStringLiteral("80"));
    parameter.insert(QStringLiteral("velocity"), QStringLiteral("5920"));
    parameter.insert(QStringLiteral("compress_rate"), QStringLiteral("1"));
    parameter.insert(QStringLiteral("compress_mode"), QStringLiteral("0"));
    parameter.insert(QStringLiteral("gate_a_position"), QStringLiteral("15"));
    parameter.insert(QStringLiteral("gate_a_width"), QStringLiteral("20"));
    parameter.insert(QStringLiteral("gate_a_threshold"), QStringLiteral("45"));
    parameter.insert(QStringLiteral("gate_a_measure"), QStringLiteral("0"));
    parameter.insert(QStringLiteral("gate_a_tracing"), QStringLiteral("0"));
    parameter.insert(QStringLiteral("gate_a_alarm"), QStringLiteral("0"));
    parameter.insert(QStringLiteral("gate_b_position"), QStringLiteral("35"));
    parameter.insert(QStringLiteral("gate_b_width"), QStringLiteral("20"));
    parameter.insert(QStringLiteral("gate_b_threshold"), QStringLiteral("40"));
    parameter.insert(QStringLiteral("gate_b_measure"), QStringLiteral("0"));
    parameter.insert(QStringLiteral("gate_b_tracing"), QStringLiteral("0"));
    parameter.insert(QStringLiteral("gate_b_alarm"), QStringLiteral("0"));
    parameter.insert(QStringLiteral("gate_c_position"), QStringLiteral("55"));
    parameter.insert(QStringLiteral("gate_c_width"), QStringLiteral("20"));
    parameter.insert(QStringLiteral("gate_c_threshold"), QStringLiteral("35"));
    parameter.insert(QStringLiteral("gate_c_measure"), QStringLiteral("0"));
    parameter.insert(QStringLiteral("gate_c_tracing"), QStringLiteral("0"));
    parameter.insert(QStringLiteral("gate_c_alarm"), QStringLiteral("0"));
    parameter.insert(QStringLiteral("gate_d_position"), QStringLiteral("75"));
    parameter.insert(QStringLiteral("gate_d_width"), QStringLiteral("20"));
    parameter.insert(QStringLiteral("gate_d_threshold"), QStringLiteral("30"));
    parameter.insert(QStringLiteral("gate_d_measure"), QStringLiteral("0"));
    parameter.insert(QStringLiteral("gate_d_tracing"), QStringLiteral("0"));
    parameter.insert(QStringLiteral("gate_d_alarm"), QStringLiteral("0"));
    parameter.insert(QStringLiteral("rectification"), QStringLiteral("0"));
    return parameter;
}


