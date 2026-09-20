#include "sysserialport.h"
#include <QCoreApplication>
#include <QFileInfo>
#include <QRegularExpression>
#include <QSerialPortInfo>
SYSserialport::SYSserialport()
{
    mSerial = new QSerialPort;
    m_FrameHead = "0110";
    m_Recv_Length = 16;
}
//crc校验和（前面所有的数的校验和）
uint16_t CRC_Check(const uint8_t *CRC_Ptr, uint8_t LEN)
{
    uint16_t CRC_Value = 0xFFFF;  // 初始值为 0xFFFF
    uint8_t i, j;

    for (i = 0; i < LEN; i++) {
        CRC_Value ^= CRC_Ptr[i];  // 数据异或
        for (j = 0; j < 8; j++) {
            if (CRC_Value & 0x0001) {  // 判断最低位是否为 1
                CRC_Value = (CRC_Value >> 1) ^ 0xA001;  // 如果为 1，右移并异或 0xA001
            } else {
                CRC_Value >>= 1;  // 如果为 0，直接右移
            }
        }
    }

    // 交换高低字节
    CRC_Value = (CRC_Value >> 8) | (CRC_Value << 8);

    return CRC_Value;
}
//CRC校验和
QString calculateCRC(const QString &inputString)
{
    // 去掉输入字符串中的所有空格
    QString modifiedInput = inputString;
    modifiedInput.replace(" ", "");
    // 将修改后的字符串转换为 QByteArray
    QByteArray byteArray = QByteArray::fromHex(modifiedInput.toUtf8());
    // 计算 CRC 校验
    uint16_t crcResult = CRC_Check(reinterpret_cast<const uint8_t*>(byteArray.data()), byteArray.size());

    // 格式化为四位十六进制字符串（大写，补前导0）
    QString hexStr = QString("%1").arg(crcResult, 4, 16, QChar('0')).toUpper();
    // 插入空格（05 39）
    return QString("%1 %2").arg(hexStr.left(2), hexStr.right(2));
}
void  SYSserialport::InitSerial()
{
    mSerial->setPortName("COM7");
    const bool portAvailable = []() -> bool {
        const auto ports = QSerialPortInfo::availablePorts();
        for (const auto& p : ports) {
            if (p.portName() == QStringLiteral("COM7"))
                return true;
        }
        return false;
    }();
    if (!portAvailable) {
        emit toerror("串口COM7不可用");
        return;
    }
    if(!mSerial->open(QIODevice::ReadWrite))
    {
        emit toerror("串口连接错误");
        return;
    }
    mSerial->setBaudRate(QSerialPort::Baud9600);
    mSerial->setDataBits(QSerialPort::Data8);
    mSerial->setParity(QSerialPort::NoParity);
    mSerial->setStopBits(QSerialPort::OneStop);
    mSerial->setFlowControl(QSerialPort::NoFlowControl); // 流控制
    connect(mSerial,&QSerialPort::readyRead,this,&SYSserialport::ReceviceSerialData);

    mSerial->flush();
}

SYSserialport::~SYSserialport()
{
    mSerial->clear();
    mSerial->close();
    mSerial->deleteLater();
    mSerial = nullptr;
}

void SYSserialport::CloseSerial()
{
    if(mSerial->isOpen())
    {
        mSerial->clear();
        mSerial->close();
    }
}
void SYSserialport::ReceviceSerialData()
{
    m_rxBuffer.append(mSerial->readAll());
    QString hexString = m_rxBuffer.toHex().toUpper();
    if (hexString == "00") {
        m_rxBuffer.clear();
        return;
    }
    if(hexString.length() <= 6)
    {
        return;
    }
    //找帧头
    int headerIndex = hexString.indexOf(m_FrameHead);
    if (headerIndex == -1) {
        // 没有找到帧头，清空缓冲区
        m_rxBuffer.clear();
        return;
    }
    // 检查从帧头开始到结尾的数据长度是否至少为 m_Recv_Length
    if (hexString.size() < headerIndex + m_Recv_Length) {
        // 数据不足，等待更多数据
        return;
    }

    const QString frame = hexString.mid(headerIndex , m_Recv_Length);
    const QString payload = frame.left(frame.length() - 4);
    const QString calculatedCRC = calculateCRC(payload).remove(' ');
    const QString receivedCRC = frame.right(4);

    if (calculatedCRC == receivedCRC) {
        //qDebug() << "接收到完整帧，CRC 校验通过！";
        mSerial->clear();
        m_rxBuffer.clear();
    } else {
       m_rxBuffer.clear();
    }

}

void SYSserialport::SendSerialData(QString hexData)
{
    QMutexLocker locker(&mSerialMutex);

    if (!mSerial->isOpen()) {
        emit toerror("串口未打开");
        return;
    }

    // 输入合法性校验
    QString modifiedInput = hexData.replace(" ", "");
    if (modifiedInput.length() % 2 != 0 ||
        modifiedInput.contains(QRegularExpression("[^0-9A-Fa-f]"))) {
        emit toerror("发送数据格式错误");
        return;
    }

    QByteArray data = QByteArray::fromHex(hexData.toUtf8());
    qint64 bytesWritten = mSerial->write(data);
    mSerial->flush();
    if (bytesWritten == -1) {
        emit toerror("串口写入失败: " + mSerial->errorString());
        return;
    }
    mSerial->setReadBufferSize(10);
    if (!mSerial->waitForBytesWritten(500)) { // 等待最多500ms，避免UI卡顿
        emit toerror("等待发送超时: " + mSerial->errorString());
    }

}

void SYSserialport::ClearSeria()
{
    mSerial->clear();
}

void SYSserialport::Check(double value)
{
    // 使用本轮检测直接传入的目标电流，避免读取 4.txt 时序不一致。
    const QString appDir = QCoreApplication::applicationDirPath();

    // 标定使用的文件（缺失或格式异常时回退默认值，不弹阻塞错误）
    double dCalibration1 = 0.0;
    double dCalibration2 = 0.0;
    QString calibrationFilePath = QDir(appDir).filePath(QStringLiteral("Calibration.txt"));
    if (!QFileInfo::exists(calibrationFilePath)) {
        const QString legacyPath = QDir(QDir::currentPath()).filePath(QStringLiteral("Calibration.txt"));
        if (QFileInfo::exists(legacyPath))
            calibrationFilePath = legacyPath;
    }
    if (QFileInfo::exists(calibrationFilePath)) {
        QFile file_Calibration(calibrationFilePath);
        if (file_Calibration.open(QIODevice::ReadOnly)) {
            QTextStream in_Calibration(&file_Calibration);
            const QString line1 = file_Calibration.readLine().trimmed();
            const QString line2 = file_Calibration.readLine().trimmed();
            bool ok1 = false;
            bool ok2 = false;
            const double parsedCalibration1 = line1.toDouble(&ok1);
            const double parsedCalibration2 = line2.toDouble(&ok2);
            file_Calibration.close();

            if (ok1 && ok2) {
                dCalibration1 = parsedCalibration1;
                dCalibration2 = parsedCalibration2;
            } else {
                qWarning() << "Calibration.txt 格式无效，使用默认标定值0" << calibrationFilePath;
            }
        } else {
            qWarning() << "Calibration.txt 无法读取，使用默认标定值0" << calibrationFilePath
                       << file_Calibration.errorString();
        }
    }

    // 0-20mA输出计算
//    ULONG lDALSB = static_cast<ULONG>(value * 0xFFF / 20 + 0.5);
//    USHORT uDALSB = static_cast<USHORT>(lDALSB);

//    if(value >= 0 && value <= 4)
//    {
//        value = 4;
//    }
    // //新公式
    // const double outputCurrent = qBound(4.0, static_cast<double>(value) + dCalibration1, 20.0);
    // USHORT Uvalue = static_cast<USHORT>(outputCurrent * 4095 / 20.0);

    // //2026.7.10
    // const double outputCurrent = qBound(4.0,
    //                                 4.0 + (8.0 / 95.0) * (static_cast<double>(value) - 10.0),
    //                                 20.0);
    // USHORT Uvalue = static_cast<USHORT>(outputCurrent * 4095 / 20.0 + 0.5);
    //2026.7.13
    // // 4.txt 存储的是 12 位 DAC 原始码值（0~4095），直接下发。
    // const double rawDac = qBound(0.0, static_cast<double>(value), 4095.0);
    // const USHORT Uvalue = static_cast<USHORT>(rawDac + 0.5);

    // outputCurrent 是板卡目标电流，单位 mA。
    const double outputCurrent = qBound(4.0, value, 20.0) - 4.0;

    // 模块配置为0-20mA量程时，将电流转换成12位DAC码
    const USHORT Uvalue =
        static_cast<USHORT>(
                outputCurrent * 4095.0 / 16.0);


    QString hexStr = QString("%1").arg(Uvalue, 4, 16, QChar('0')).toUpper(); // 转 HEX 并补零
    QString formattedHex = hexStr.left(2) + " " + hexStr.mid(2); // 插入空格

    //01 10 00 05  00 01 02 05 39  + crc
    SendSerialData("01 10 00 05 00 01 02 " + formattedHex + " " + calculateCRC("01 10 00 05 00 01 02 " + formattedHex));

}
