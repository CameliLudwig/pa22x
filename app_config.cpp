#include "app_config.h"
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QVector>
#include <QDebug>

// --- PhysicalModelParams ---

QJsonObject PhysicalModelParams::toJson() const
{
    QJsonObject obj;
    obj["mp"] = QJsonArray() << mp0 << mp1 << mp2 << mp3 << mp4 << mp5 << mp6 << mp7;
    obj["pp"] = QJsonArray() << pp0 << pp1 << pp2 << pp3 << pp4 << pp5 << pp6;
    obj["ip"] = QJsonArray() << ip0 << ip1 << ip2 << ip3 << ip4 << ip5;
    obj["cp"] = QJsonArray() << cp0 << cp1;
    obj["cv"] = cv;
    obj["gamma_default"] = gammaDefault;
    return obj;
}

PhysicalModelParams PhysicalModelParams::fromJson(const QJsonObject& obj)
{
    PhysicalModelParams p;
    auto readDoubleArray = [&](const char* key, QVector<double>& out) {
        const QJsonArray arr = obj[key].toArray();
        out.resize(arr.size());
        for (int i = 0; i < arr.size(); ++i)
            out[i] = arr[i].toDouble();
    };

    QVector<double> mpVals; readDoubleArray("mp", mpVals);
    if (mpVals.size() >= 8) {
        p.mp0 = mpVals[0]; p.mp1 = mpVals[1]; p.mp2 = mpVals[2]; p.mp3 = mpVals[3];
        p.mp4 = mpVals[4]; p.mp5 = mpVals[5]; p.mp6 = mpVals[6]; p.mp7 = mpVals[7];
    }

    QVector<double> ppVals; readDoubleArray("pp", ppVals);
    if (ppVals.size() >= 7) {
        p.pp0 = ppVals[0]; p.pp1 = ppVals[1]; p.pp2 = ppVals[2]; p.pp3 = ppVals[3];
        p.pp4 = ppVals[4]; p.pp5 = ppVals[5]; p.pp6 = ppVals[6];
    }

    QVector<double> ipVals; readDoubleArray("ip", ipVals);
    if (ipVals.size() >= 6) {
        p.ip0 = ipVals[0]; p.ip1 = ipVals[1]; p.ip2 = ipVals[2]; p.ip3 = ipVals[3];
        p.ip4 = ipVals[4]; p.ip5 = ipVals[5];
    }

    QVector<double> cpVals; readDoubleArray("cp", cpVals);
    if (cpVals.size() >= 2) {
        p.cp0 = cpVals[0]; p.cp1 = cpVals[1];
    }

    if (obj.contains("cv"))
        p.cv = obj["cv"].toDouble(p.cv);
    if (obj.contains("gamma_default"))
        p.gammaDefault = obj["gamma_default"].toDouble(p.gammaDefault);
    return p;
}

// --- DetectionDefaults ---

QJsonObject DetectionDefaults::toJson() const
{
    QJsonObject obj;
    obj["sample_width_mm"] = sampleWidthMM;
    obj["spec_fmin"] = specFmin;
    obj["spec_fmax"] = specFmax;
    obj["sampling_rate_mhz"] = samplingRateMHz;
    obj["detection_interval_sec"] = detectionIntervalSec;
    obj["ema_smoothing_window"] = emaSmoothingWindow;
    return obj;
}

DetectionDefaults DetectionDefaults::fromJson(const QJsonObject& obj)
{
    DetectionDefaults d;
    if (obj.contains("sample_width_mm"))
        d.sampleWidthMM = obj["sample_width_mm"].toDouble(d.sampleWidthMM);
    if (obj.contains("spec_fmin"))
        d.specFmin = obj["spec_fmin"].toDouble(d.specFmin);
    if (obj.contains("spec_fmax"))
        d.specFmax = obj["spec_fmax"].toDouble(d.specFmax);
    if (obj.contains("sampling_rate_mhz"))
        d.samplingRateMHz = obj["sampling_rate_mhz"].toDouble(d.samplingRateMHz);
    if (obj.contains("detection_interval_sec"))
        d.detectionIntervalSec = obj["detection_interval_sec"].toInt(d.detectionIntervalSec);
    if (obj.contains("ema_smoothing_window"))
        d.emaSmoothingWindow = obj["ema_smoothing_window"].toInt(d.emaSmoothingWindow);
    return d;
}

// --- AppConfig ---

QJsonObject AppConfig::toJson() const
{
    QJsonObject obj;
    obj["physical_model"] = physicalModel.toJson();
    obj["detection"] = detection.toJson();
    return obj;
}

AppConfig AppConfig::fromJson(const QJsonObject& obj)
{
    AppConfig cfg;
    if (obj.contains("physical_model"))
        cfg.physicalModel = PhysicalModelParams::fromJson(obj["physical_model"].toObject());
    if (obj.contains("detection"))
        cfg.detection = DetectionDefaults::fromJson(obj["detection"].toObject());
    return cfg;
}

AppConfig AppConfig::defaults()
{
    return AppConfig();
}

bool AppConfig::loadFromFile(const QString& filePath)
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        qWarning() << "Cannot open config file:" << filePath << ", using defaults";
        return false;
    }

    QJsonParseError error;
    QJsonDocument doc = QJsonDocument::fromJson(file.readAll(), &error);
    if (error.error != QJsonParseError::NoError) {
        qWarning() << "Config JSON parse error:" << error.errorString() << ", using defaults";
        return false;
    }

    *this = fromJson(doc.object());
    qInfo() << "Loaded config from:" << filePath;
    return true;
}

bool AppConfig::saveToFile(const QString& filePath) const
{
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly)) {
        qWarning() << "Cannot write config file:" << filePath;
        return false;
    }

    QJsonDocument doc(toJson());
    file.write(doc.toJson(QJsonDocument::Indented));
    return true;
}
