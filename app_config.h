#ifndef APP_CONFIG_H
#define APP_CONFIG_H

#include <QString>
#include <QJsonObject>

// 反演物理模型参数(替代 inversion::init 中的硬编码)
struct PhysicalModelParams {
    // m_mp[8] - 模型参数
    double mp0 = 2.2000E-14;
    double mp1 = 2.5700E-4;
    double mp2 = 1.4967E+3;
    double mp3 = 4.1785E+3;
    double mp4 = 9.0300E-4;
    double mp5 = 9.9704E+2;
    double mp6 = 2.9800E+2;
    double mp7 = 5.9520E-1;

    // m_pp[7] - 物理参数
    double pp0 = 8.0600E-2;
    double pp1 = 4.8000E-6;
    double pp2 = 5.2600E+3;
    double pp3 = 8.2900E+2;
    double pp4 = 3.0000E+10;
    double pp5 = 2.7150E+3;
    double pp6 = 6.8200E+1;

    // m_ip[6] - 反演参数
    double ip0 = 1.0000E-6;
    double ip1 = 1.0000E-4;
    double ip2 = 100.00;
    double ip3 = 25.00;
    double ip4 = 20.00;
    double ip5 = 1.0000E+0;

    // m_cp[2] - 修正系数(通常从0.txt动态读取)
    double cp0 = 1.0000E+0;
    double cp1 = 0.0000E+0;

    // 反演控制参数
    double cv = 0.01;         // 体积浓度
    double gammaDefault = 20.0; // 默认正则化参数

    QJsonObject toJson() const;
    static PhysicalModelParams fromJson(const QJsonObject& obj);
};

// 检测参数默认值
struct DetectionDefaults {
    double sampleWidthMM = 10.0;   // 默认样品池宽度(mm)
    double specFmin = 2.0;         // 默认衰减谱频率下限(MHz)
    double specFmax = 40.0;        // 默认衰减谱频率上限(MHz)
    double samplingRateMHz = 100.0;// FFT采样率(MHz)
    int detectionIntervalSec = 10; // 默认检测间隔(秒)
    int emaSmoothingWindow = 10;   // EMA平滑窗口

    QJsonObject toJson() const;
    static DetectionDefaults fromJson(const QJsonObject& obj);
};

// 应用配置聚合
struct AppConfig {
    PhysicalModelParams physicalModel;
    DetectionDefaults detection;

    bool loadFromFile(const QString& filePath);
    bool saveToFile(const QString& filePath) const;
    static AppConfig defaults();

    QJsonObject toJson() const;
    static AppConfig fromJson(const QJsonObject& obj);
};

#endif // APP_CONFIG_H
