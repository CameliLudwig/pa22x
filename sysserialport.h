#ifndef SYSSERIALPORT_H
#define SYSSERIALPORT_H
#include <QSerialPort>
#include <QDebug>
#include <QString>
#include <QObject>
#include <QFile>
#include "DAM3000M.h"
#include <QMutex>
#include "QDir"

class SYSserialport : public QObject
{
    Q_OBJECT
public:
    SYSserialport();
    ~SYSserialport();
    void Check(double outputCurrent);
    void InitSerial();
    void CloseSerial();
    void ClearSeria();

signals:
    void toerror(QString error);
    void UpdateSerialData(QByteArray data);

public slots:
    void SendSerialData(QString data);
    void ReceviceSerialData();
private:
    QSerialPort *mSerial;
    QMutex mSerialMutex;  // 创建一个互斥锁
    //IO处理粘包的帧头和帧长
    QByteArray  m_FrameHead;
    int m_Recv_Length = 0;
    QByteArray  m_rxBuffer;  // 接收缓冲区(替代static局部变量)
};

#endif // SYSSERIALPORT_H
