#include "pa22x.h"
#include "ui_pa22x.h"
#include "ruler.h"
#include "xlsxhandle.h"
#include <QStandardPaths>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QTextStream>
#include <QLocale>
#include <QLockFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QSaveFile>
#include <QShortcut>
#include <QStatusBar>
#include <QPointer>
#include <QScreen>
#include <QAbstractButton>
#include <QAbstractSpinBox>
#include <QScrollArea>
#include <QFrame>
#include <QGridLayout>
#include <QLineEdit>
#include <QListView>
#include <QPixmap>
#include <QTabWidget>
#include <QWindow>
#include <QElapsedTimer>
#include <QRandomGenerator>
#include <QRegularExpression>
#include <QThread>
#include <QApplication>
#include <algorithm>
#include <limits>
#include <numeric>

namespace {

QPointer<QStatusBar> g_statusBar;
QPointer<xlsxHandle> g_xlsxLogger;
std::atomic_bool g_shutdownLoggingSuppressed{false};
std::atomic_uint g_detectionTraceSequence{0};
constexpr int kEscapeHotKeyId = 0x22A1;

// 检测算法参数 — EMA平滑权重
constexpr double kEmaWeightNew       = 0.4;   // 新值的权重
constexpr double kEmaWeightOld       = 0.6;   // 历史值的权重
constexpr double kEmaDecayWeight     = 0.92;  // 无效信号时历史保持权重
constexpr double kEmaHoldWeight      = 0.08;  // 无效信号时新值贡献权重

// 4-20mA 电流输出映射
constexpr double kCurrentMin_mA      = 4.0;
constexpr double kCurrentMax_mA      = 20.0;
constexpr double kCurrentFineLo      = 20.0;  // 细度低端(对应4mA)
constexpr double kCurrentFineHi      = 80.0;  // 细度高端(对应20mA)
constexpr double kCurrentFullScale   = 16.0;  // 4-20mA量程范围
constexpr double kCurrentFineRange   = 60.0;  // 细度映射范围

// 频谱有效性判定阈值
constexpr double kAveSpecThreshold   = 240.0; // 衰减谱均值阈值
constexpr double kWaterLikePeakAbs   = 220.0; // 水信号峰值判定
constexpr double kAirLikePeakAbsMin  = 30.0;  // 空信号峰值判定
constexpr double kSpectrumContrastLo = 0.05;  // 频谱对比度下限
constexpr double kMonotonicMinRatio  = 0.50;  // 单调性最小比例

struct UiSettings {
    QString deskline = QStandardPaths::writableLocation(QStandardPaths::DesktopLocation);
    double specfmin = 3.0;
    double specfmax = 7.5;
    double histime = 2.0;
    double avenum = 10.0;
    double width = 16.0;
    double interval = 10.0;
    double calib = 0.0;
};

QString composeStepHeader(const QString& scope, const QString& code, const QString& step);

QString defaultUserFileDirectory()
{
    const QStringList candidates = {
        QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation),
        QStandardPaths::writableLocation(QStandardPaths::DesktopLocation),
        QCoreApplication::applicationDirPath()
    };

    for (const QString& candidate : candidates) {
        const QString normalized = QDir::cleanPath(candidate.trimmed());
        if (!normalized.isEmpty())
            return normalized;
    }

    return QDir::cleanPath(QCoreApplication::applicationDirPath());
}

bool isLegacyPlaceholderDirectory(const QString& rawPath)
{
    const QString normalized = rawPath.trimmed().toLower();
    return normalized.isEmpty()
            || normalized == QStringLiteral("w1215")
            || normalized == QStringLiteral("w1215\\")
            || normalized == QStringLiteral("w1215/");
}

QString sanitizeUserFileDirectory(const QString& rawPath)
{
    if (isLegacyPlaceholderDirectory(rawPath))
        return defaultUserFileDirectory();

    const QFileInfo info(rawPath.trimmed());
    if (!info.isAbsolute())
        return defaultUserFileDirectory();

    return QDir::cleanPath(info.absoluteFilePath());
}

bool looksLikeLegacyInlineStyle(const QString& styleSheet)
{
    if (styleSheet.trimmed().isEmpty())
        return false;

    static const QStringList legacyTokens = {
        QStringLiteral("darkgray"),
        QStringLiteral("border: 1px solid black"),
        QStringLiteral("background:white"),
        QStringLiteral("background: white"),
        QStringLiteral("background: rgb(255,255,255)"),
        QStringLiteral("selection-background-color: black"),
        QStringLiteral("selection-color: black"),
        QStringLiteral("QComboBox::down-arrow"),
        QStringLiteral("QDoubleSpinBox {"),
        QStringLiteral("QSpinBox {"),
        QStringLiteral("QLineEdit {"),
        QStringLiteral("QComboBox {"),
        QStringLiteral("QLabel {"),
        QStringLiteral("QWidget {")
    };

    for (const QString& token : legacyTokens) {
        if (styleSheet.contains(token, Qt::CaseInsensitive))
            return true;
    }

    return false;
}

QString formatNumericForLog(double value, int precision = 2)
{
    if (!std::isfinite(value))
        return QStringLiteral("nan");
    return QString::number(value, 'f', precision);
}

constexpr double kProcessDv90AverageMin = 30.0;
constexpr double kProcessDv90AverageMax = 80.0;
constexpr int kDisplayRefreshIntervalMs = 50;

bool isDv90WithinProcessAverageRange(double value)
{
    return std::isfinite(value)
            && value > kProcessDv90AverageMin
            && value < kProcessDv90AverageMax;
}

double averageSeries(const QVector<double>& values)
{
    if (values.isEmpty())
        return 0.0;

    return std::accumulate(values.cbegin(), values.cend(), 0.0) / values.size();
}

QString processDv90AverageRangeForLog()
{
    return QStringLiteral("%1-%2")
            .arg(formatNumericForLog(kProcessDv90AverageMin, 2),
                 formatNumericForLog(kProcessDv90AverageMax, 2));
}

double computeDisplayedAveD90ForSequence(const QVector<double>& rawDv90Values,
                                         int windowSize,
                                         double calibrationOffset)
{
    QVector<double> validHistory;
    validHistory.reserve(rawDv90Values.size());

    for (double value : rawDv90Values) {
        if (isDv90WithinProcessAverageRange(value))
            validHistory.push_back(value);
    }

    if (validHistory.isEmpty())
        return 0.0;

    const int effectiveWindowSize = qMax(1, windowSize);
    const int startIndex = qMax(0, validHistory.size() - effectiveWindowSize);
    const QVector<double> window = validHistory.mid(startIndex);
    return averageSeries(window) + calibrationOffset;
}

QString previewValuesForLog(const QVector<double>& values,
                            int previewCount = 3,
                            int precision = 2)
{
    if (values.isEmpty())
        return QStringLiteral("[]");

    QStringList parts;
    const int headCount = qMin(previewCount, values.size());
    for (int i = 0; i < headCount; ++i)
        parts << formatNumericForLog(values.at(i), precision);

    if (values.size() > previewCount * 2)
        parts << QStringLiteral("...");

    const int tailStart = qMax(headCount, values.size() - previewCount);
    for (int i = tailStart; i < values.size(); ++i)
        parts << formatNumericForLog(values.at(i), precision);

    return QStringLiteral("[%1]").arg(parts.join(QStringLiteral(", ")));
}

QString summarizeSeriesForLog(const QVector<double>& values, int precision = 2)
{
    if (values.isEmpty())
        return QStringLiteral("count=0");

    const auto minmax = std::minmax_element(values.cbegin(), values.cend());
    const double sum = std::accumulate(values.cbegin(), values.cend(), 0.0);
    const double average = sum / values.size();
    return QStringLiteral("count=%1, min=%2, max=%3, avg=%4, last=%5")
            .arg(values.size())
            .arg(formatNumericForLog(*minmax.first, precision))
            .arg(formatNumericForLog(*minmax.second, precision))
            .arg(formatNumericForLog(average, precision))
            .arg(formatNumericForLog(values.last(), precision));
}

QString summarizeSpectrumForLog(const QVector<double>& frequencies,
                                const QVector<double>& spectrum,
                                int precision = 2)
{
    if (frequencies.isEmpty() || spectrum.isEmpty())
        return QStringLiteral("points=0");

    const auto minmax = std::minmax_element(spectrum.cbegin(), spectrum.cend());
    const double sum = std::accumulate(spectrum.cbegin(), spectrum.cend(), 0.0);
    const double average = sum / spectrum.size();
    return QStringLiteral("points=%1, f=%2-%3MHz, min=%4, max=%5, avg=%6")
            .arg(spectrum.size())
            .arg(formatNumericForLog(frequencies.first(), precision))
            .arg(formatNumericForLog(frequencies.last(), precision))
            .arg(formatNumericForLog(*minmax.first, precision))
            .arg(formatNumericForLog(*minmax.second, precision))
            .arg(formatNumericForLog(average, precision));
}

bool isPositiveFiniteMeasurement(double value)
{
    return std::isfinite(value) && value > 0.0;
}

bool isWaterLikeSpectrumSingleChannel(double averageSpectrum,
                                      double medianSpectrum,
                                      double strongPositiveRatio,
                                      double spectrumContrast,
                                      double wavePeakAbs,
                                      double waveRms,
                                      int spectrumPointCount,
                                      const QVector<double>& spectrum)
{
    if (!std::isfinite(averageSpectrum)
            || !std::isfinite(medianSpectrum)
            || !std::isfinite(strongPositiveRatio)
            || !std::isfinite(spectrumContrast)
            || !std::isfinite(wavePeakAbs)
            || !std::isfinite(waveRms)
            || spectrumPointCount <= 0) {
        return false;
    }

    // Check 1: waveform too large (near display 90/10 on 0-100 scale) = water
    // ADC ±255 mapped to display 0-100: display=90 → ADC≈204
    if (wavePeakAbs >= 200.0)
        return true;

    // Check 2: water spectrum rises monotonically with frequency
    // (no terminal drop), while slurry drops at high freq due to particle scattering
    if (spectrum.size() >= 5) {
        int terminalStart = spectrum.size() * 3 / 4;
        double maxEarly = 0.0;
        for (int i = 0; i < terminalStart; i++)
            if (spectrum[i] > maxEarly) maxEarly = spectrum[i];
        double minLate = spectrum[terminalStart];
        for (int i = terminalStart + 1; i < spectrum.size(); i++)
            if (spectrum[i] < minLate) minLate = spectrum[i];
        double terminalDrop = (maxEarly - minLate) / qMax(1.0, maxEarly);

        int risingCount = 0;
        for (int i = 1; i < spectrum.size(); i++)
            if (spectrum[i] > spectrum[i - 1]) risingCount++;
        double monoRatio = static_cast<double>(risingCount) / (spectrum.size() - 1);

        // relaxed: monotonic > 65% and no significant terminal drop = water
        if (monoRatio > 0.65 && terminalDrop < 0.30)
            return true;
    }

    // Check 3: flat low-contrast spectrum (original)
    if (strongPositiveRatio < 0.25 && spectrumContrast < 0.15)
        return true;

    return false;
}

QString summarizeAverageWindowForLog(const QVector<double>& rawValues,
                                     const QVector<double>& validValues,
                                     int windowSize,
                                     int precision = 2)
{
    QVector<double> usedValues;
    if (!validValues.isEmpty()) {
        const int usedCount = qMin(windowSize, validValues.size());
        usedValues = validValues.mid(validValues.size() - usedCount, usedCount);
    }

    return QStringLiteral("raw=%1, valid=%2, window=%3, used=%4")
            .arg(rawValues.size())
            .arg(validValues.size())
            .arg(windowSize)
            .arg(previewValuesForLog(usedValues, 4, precision));
}

quint64 fingerprintSamples(const QVector<double>& values, double scale = 1.0)
{
    quint64 hash = 1469598103934665603ULL;
    for (double value : values) {
        const qint64 quantized = qRound64(value * scale);
        hash ^= static_cast<quint64>(quantized);
        hash *= 1099511628211ULL;
    }
    return hash;
}

QString nextDetectionCycleId(const QDateTime& timestamp)
{
    const unsigned int sequence = ++g_detectionTraceSequence;
    return QStringLiteral("%1_%2")
            .arg(timestamp.toString(QStringLiteral("yyyyMMdd_hhmmss_zzz")))
            .arg(sequence, 4, 10, QLatin1Char('0'));
}

QString summarizeSeriesForExcel(const QString& name,
                                const QVector<double>& values,
                                int precision = 3,
                                int previewCount = 3)
{
    return QStringLiteral("%1={%2 | preview=%3 | fp=%4}")
            .arg(name,
                 summarizeSeriesForLog(values, precision),
                 previewValuesForLog(values, previewCount, precision),
                 QString::number(fingerprintSamples(values, 1000.0)));
}

QString summarizeSeriesForExcel(const QString& name,
                                const QVector<int>& values,
                                int previewCount = 4)
{
    QVector<double> numericValues;
    numericValues.reserve(values.size());
    for (int value : values)
        numericValues.push_back(static_cast<double>(value));
    return summarizeSeriesForExcel(name, numericValues, 0, previewCount);
}

QString summarizeRawArrayForExcel(const QString& name,
                                  const double* values,
                                  int count,
                                  int precision = 3,
                                  int previewCount = 3)
{
    QVector<double> snapshot;
    snapshot.reserve(count);
    for (int i = 0; i < count; ++i)
        snapshot.push_back(values[i]);
    return summarizeSeriesForExcel(name, snapshot, precision, previewCount);
}

QString summarizeSpectrumForExcel(const QVector<double>& frequencies,
                                  const QVector<double>& spectrum,
                                  quint64 fingerprintScale = 1000)
{
    return QStringLiteral("%1 | preview=%2 | fp=%3")
            .arg(summarizeSpectrumForLog(frequencies, spectrum, 3),
                 previewValuesForLog(spectrum, 3, 3),
                 QString::number(fingerprintSamples(spectrum, fingerprintScale)));
}

void insertXlsxRecord(const QString& scope,
                      const QString& code,
                      const QString& step,
                      const QString& detail,
                      const QString& cycleId = QString())
{
    if (g_shutdownLoggingSuppressed.load() || !g_xlsxLogger)
        return;

    QMetaObject::invokeMethod(g_xlsxLogger.data(),
                              "insertRecord",
                              Qt::QueuedConnection,
                              Q_ARG(QString, scope),
                              Q_ARG(QString, code),
                              Q_ARG(QString, step),
                              Q_ARG(QString, detail),
                              Q_ARG(QString, cycleId));
}

void clearLegacyInlineStyles(QWidget* root)
{
    if (!root)
        return;

    if (looksLikeLegacyInlineStyle(root->styleSheet()))
        root->setStyleSheet(QString());

    const auto widgets = root->findChildren<QWidget*>();
    for (QWidget* widget : widgets) {
        if (looksLikeLegacyInlineStyle(widget->styleSheet()))
            widget->setStyleSheet(QString());
    }
}

QString uiSettingsFilePath()
{
    return QDir(QCoreApplication::applicationDirPath()).filePath(QStringLiteral("Para.txt"));
}

bool hasInternalToolsAccess()
{
    return QCoreApplication::arguments().contains(QStringLiteral("--internal-tools"))
            && qEnvironmentVariable("PA22X_INTERNAL_TOOLS") == QStringLiteral("pa22x_lab");
}

QString composeStepHeader(const QString& scope, const QString& code, const QString& step)
{
    if (code.isEmpty())
        return QStringLiteral("[%1] %2").arg(scope, step);
    return QStringLiteral("[%1][%2] %3").arg(scope, code, step);
}

bool shouldMirrorStepToXlsx(const QString& scope, const QString& code)
{
    if (scope == QStringLiteral("错误") || scope == QStringLiteral("系统"))
        return true;

    if (scope == QStringLiteral("检测")) {
        if (code == QStringLiteral("DET-01")
                || code == QStringLiteral("DET-02")
                || code == QStringLiteral("DET-03")
                || code == QStringLiteral("DET-04")
                || code == QStringLiteral("DET-06")
                || code == QStringLiteral("DET-07")
                || code == QStringLiteral("DET-08")
                || code == QStringLiteral("DET-10")
                || code == QStringLiteral("DET-11")) {
            return true;
        }
        return false;
    }

    return scope == QStringLiteral("è????￥")
            || scope == QStringLiteral("?????°")
            || scope == QStringLiteral("é??é??")
            || scope == QStringLiteral("è????ˉ")
            || scope == QStringLiteral("?????2")
            || scope == QStringLiteral("?¨????");
}
void logStepWithCycle(const QString& scope,
                      const QString& code,
                      const QString& step,
                      const QString& detail,
                      const QString& cycleId)
{
    const QString header = composeStepHeader(scope, code, step);
    QString uiSummary = header;
    if (!detail.isEmpty()) {
        QString compactDetail = detail;
        compactDetail.replace('\n', ' ');
        if (compactDetail.size() > 56)
            compactDetail = compactDetail.left(53) + QStringLiteral("...");
        uiSummary += QStringLiteral(" | %1").arg(compactDetail);
    }

    if (!g_shutdownLoggingSuppressed.load() && g_statusBar) {
        const QString summary = uiSummary.left(160);
        QMetaObject::invokeMethod(g_statusBar.data(), [summary]() {
            if (g_statusBar)
                g_statusBar->showMessage(summary, 8000);
        }, Qt::QueuedConnection);
    }

    // 关闭过程中不再写入日志，防止访问已释放的 Logger/XLSX
    if (!g_shutdownLoggingSuppressed.load()) {
        if (detail.isEmpty())
            qInfo().noquote() << header;
        else
            qInfo().noquote() << QStringLiteral("%1 | %2").arg(header, detail);

        if (shouldMirrorStepToXlsx(scope, code))
            insertXlsxRecord(scope, code, step, detail, cycleId);
    }
}

void logStep(const QString& scope,
             const QString& code,
             const QString& step,
             const QString& detail)
{
    logStepWithCycle(scope, code, step, detail, QString());
}

void logStep(const QString& scope, const QString& step, const QString& detail = QString())
{
    logStep(scope, QString(), step, detail);
}

QString measurementTimestamp(const QDateTime& timestamp)
{
    return timestamp.toString(QStringLiteral("yyyyMMdd_hhmmss"));
}

QString measurementDay(const QDateTime& timestamp)
{
    return timestamp.toString(QStringLiteral("yyyyMMdd"));
}

QString distributionDirectoryPath(const QString& appDir)
{
    const QString dirPath = QDir(appDir).filePath(QStringLiteral("Distribution"));
    QDir().mkpath(dirPath);
    return dirPath;
}

QString backgroundSignalDirectoryPath(const QString& appDir)
{
    const QString dirPath = QDir(appDir).filePath(QStringLiteral("Signals"));
    QDir().mkpath(dirPath);
    return dirPath;
}

QString primaryBackgroundSignalFilePath(const QString& appDir)
{
    return QDir(backgroundSignalDirectoryPath(appDir)).filePath(QStringLiteral("boxing_water.txt"));
}

QString legacyBackgroundSignalFilePath(const QString& appDir)
{
    return QDir(appDir).filePath(QStringLiteral("boxing_water.txt"));
}

QStringList backgroundSignalFilePaths(const QString& appDir)
{
    return QStringList()
            << primaryBackgroundSignalFilePath(appDir)
            << legacyBackgroundSignalFilePath(appDir);
}

QString calibrationFilePath(const QString& appDir)
{
    return QDir(appDir).filePath(QStringLiteral("Calibration.txt"));
}

QString analogOutputFilePath(const QString& appDir)
{
    return QDir(appDir).filePath(QStringLiteral("4.txt"));
}

QStringList calibrationFileCandidates(const QString& appDir)
{
    QStringList candidates;
    const QString primaryPath = QDir::cleanPath(calibrationFilePath(appDir));
    const QString legacyPath =
        QDir::cleanPath(QDir(QDir::currentPath()).filePath(QStringLiteral("Calibration.txt")));

    candidates << primaryPath;
    if (legacyPath != primaryPath)
        candidates << legacyPath;
    return candidates;
}

bool loadCalibrationValues(const QString& appDir,
                           double* calibration1,
                           double* calibration2,
                           QString* loadedPath = nullptr)
{
    if (!calibration1 || !calibration2)
        return false;

    // 保留调用者的默认值（默认 dCalib1=1.0, dCalib2=0.0），文件不存在或无效时不覆盖
    for (const QString& candidate : calibrationFileCandidates(appDir)) {
        QFile file(candidate);
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
            continue;

        const QString line1 = QString::fromUtf8(file.readLine()).trimmed();
        const QString line2 = QString::fromUtf8(file.readLine()).trimmed();
        file.close();

        bool ok1 = false;
        bool ok2 = false;
        const double parsed1 = line1.toDouble(&ok1);
        const double parsed2 = line2.toDouble(&ok2);
        if (!ok1 || !ok2)
            continue;

        // 安全校验: dCalib1 接近0会导致所有测量值归零, 拒绝
        if (qFuzzyIsNull(parsed1)) {
            qWarning() << "Calibration.txt 比例系数为0, 已忽略，使用默认值1.0";
            continue;
        }

        *calibration1 = parsed1;
        *calibration2 = parsed2;
        if (loadedPath)
            *loadedPath = candidate;
        return true;
    }

    if (loadedPath)
        loadedPath->clear();
    return false;
}

// 加载暗调偏移量(Adjust.txt): 一个数，直接加在D50/D90/细度/通过率上
// 文件格式: 一行一个数，#开头为注释
// 只有1个有效数字 → 统一偏移D50/D90/细度/通过率都加这个数
// 有4个有效数字 → 分别对应D50/D90/细度/通过率的偏移
void loadAdjustOffsets(const QString& appDir,
                       double* d50Offset,
                       double* d90Offset,
                       double* finenessOffset,
                       double* passOffset)
{
    if (!d50Offset || !d90Offset || !finenessOffset || !passOffset)
        return;

    *d50Offset = 0.0;
    *d90Offset = 0.0;
    *finenessOffset = 0.0;
    *passOffset = 0.0;

    const QString filePath = QDir(appDir).filePath(QStringLiteral("Adjust.txt"));
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        file.close();
        static bool s_created = false;
        if (!s_created) {
            s_created = true;
            QFile cfile(filePath);
            if (cfile.open(QIODevice::WriteOnly | QIODevice::Text)) {
                QTextStream out(&cfile);
                out << "# Adjust.txt\n";
                out << "# 一个数: D50/D90/细度/通过率统一偏移\n";
                out << "# 四个数: 分别对应 D50 / D90 / 细度 / 通过率\n";
                out << "# 改完保存即生效\n";
                out << "0.0\n";
                cfile.close();
            }
        }
        return;
    }

    QStringList lines;
    QTextStream in(&file);
    while (!in.atEnd()) {
        QString line = in.readLine().trimmed();
        if (!line.isEmpty() && !line.startsWith('#'))
            lines.append(line);
    }
    file.close();

    QVector<double> values;
    for (const QString& line : lines) {
        bool ok = false;
        double v = line.toDouble(&ok);
        if (ok) values.append(v);
    }

    if (values.size() == 1) {
        // 单数值模式: 统一偏移
        *d50Offset = values[0];
        *d90Offset = values[0];
        *finenessOffset = values[0];
        *passOffset = values[0];
    } else {
        // 多数值模式: 分别偏移(兼容旧格式)
        if (values.size() >= 1) *d50Offset = values[0];
        if (values.size() >= 2) *d90Offset = values[1];
        if (values.size() >= 3) *finenessOffset = values[2];
        if (values.size() >= 4) *passOffset = values[3];
    }
}

bool isTransientAnalogOutputError(const QString& error)
{
    static const QStringList tokens = {
        QStringLiteral("等待发送超时"),
        QStringLiteral("等待操作超时"),
        QStringLiteral("等待的操作超时"),
        QStringLiteral("响应超时"),
        QStringLiteral("串口写入失败"),
        QStringLiteral("串口未打开"),
        QStringLiteral("串口连接错误"),
        QStringLiteral("串口COM7不可用"),
        QStringLiteral("发送数据格式错误"),
        QStringLiteral("无法打开文件"),
        QStringLiteral("文件内容无效")
    };

    for (const QString& token : tokens) {
        if (error.contains(token))
            return true;
    }
    return false;
}

QString nativePath(const QString& filePath)
{
    return QDir::toNativeSeparators(QDir::cleanPath(filePath));
}

int parseDualModeValue(const QString& value, int fallback = kDualModeDual)
{
    bool ok = false;
    const int mode = value.trimmed().toInt(&ok);
    if (ok && (mode == kDualModeSingle || mode == kDualModeDual))
        return mode;

    const QString normalized = value.trimmed().toLower();
    if (normalized == QStringLiteral("single"))
        return kDualModeSingle;
    if (normalized == QStringLiteral("dual") || normalized == QStringLiteral("double"))
        return kDualModeDual;

    return fallback;
}

QString loggingPathSummary()
{
    QStringList lines;
    const QString textLogPath = Logger::currentLogFilePath();
    if (!textLogPath.isEmpty())
        lines << QStringLiteral("文本日志：%1").arg(nativePath(textLogPath));

    if (g_xlsxLogger) {
        const QString xlsxLogPath = g_xlsxLogger->currentLogFilePath();
        if (!xlsxLogPath.isEmpty())
            lines << QStringLiteral("XLSX 日志：%1").arg(nativePath(xlsxLogPath));
    }

    return lines.join(QStringLiteral("\n"));
}

QString runtimePathSummary(const QString& primaryPath = QString(),
                           const QString& secondaryPath = QString())
{
    QStringList lines;
    lines << QStringLiteral("程序文件：%1").arg(nativePath(QCoreApplication::applicationFilePath()))
          << QStringLiteral("程序目录：%1").arg(nativePath(QCoreApplication::applicationDirPath()))
          << QStringLiteral("启动目录：%1").arg(nativePath(QDir::currentPath()));

    if (!primaryPath.isEmpty())
        lines << QStringLiteral("主路径：%1").arg(nativePath(primaryPath));
    if (!secondaryPath.isEmpty())
        lines << QStringLiteral("兼容路径：%1").arg(nativePath(secondaryPath));

    const QString logSummary = loggingPathSummary();
    if (!logSummary.isEmpty())
        lines << logSummary;

    return lines.join(QStringLiteral("\n"));
}

bool readNumericSamplesFile(const QString& filePath,
                            QVector<double>* samples,
                            QString* error = nullptr)
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        if (error)
            *error = QStringLiteral("文件无法读取：%1 | %2")
                    .arg(nativePath(filePath), file.errorString());
        return false;
    }

    QVector<double> parsedSamples;
    while (!file.atEnd()) {
        const QString line = QString::fromUtf8(file.readLine()).trimmed();
        if (line.isEmpty())
            continue;

        bool ok = false;
        const double value = line.toDouble(&ok);
        if (!ok) {
            if (error)
                *error = QStringLiteral("文件存在非数字行：%1 | %2")
                        .arg(nativePath(filePath), line);
            return false;
        }

        parsedSamples << value;
    }

    if (samples)
        *samples = parsedSamples;
    return true;
}

bool parseInversionSummaryLine(const QString& line,
                               double* d21,
                               double* d32,
                               double* d43,
                               double* d50,
                               double* d90)
{
    const QString trimmed = line.trimmed();
    if (trimmed.isEmpty())
        return false;

    static const QRegularExpression keyedLinePattern(
        QStringLiteral(
            R"(D21=\s*([-+]?(?:\d+\.?\d*|\.\d+)(?:[eE][-+]?\d+)?)\s+)"
            R"(D32=\s*([-+]?(?:\d+\.?\d*|\.\d+)(?:[eE][-+]?\d+)?)\s+)"
            R"(D43=\s*([-+]?(?:\d+\.?\d*|\.\d+)(?:[eE][-+]?\d+)?)\s+)"
            R"(D50=\s*([-+]?(?:\d+\.?\d*|\.\d+)(?:[eE][-+]?\d+)?)\s+)"
            R"(D90=\s*([-+]?(?:\d+\.?\d*|\.\d+)(?:[eE][-+]?\d+)?))"));

    const QRegularExpressionMatch keyedMatch = keyedLinePattern.match(trimmed);
    if (keyedMatch.hasMatch()) {
        bool ok1 = false;
        bool ok2 = false;
        bool ok3 = false;
        bool ok4 = false;
        bool ok5 = false;
        const double parsedD21 = keyedMatch.captured(1).toDouble(&ok1);
        const double parsedD32 = keyedMatch.captured(2).toDouble(&ok2);
        const double parsedD43 = keyedMatch.captured(3).toDouble(&ok3);
        const double parsedD50 = keyedMatch.captured(4).toDouble(&ok4);
        const double parsedD90 = keyedMatch.captured(5).toDouble(&ok5);
        if (ok1 && ok2 && ok3 && ok4 && ok5) {
            if (d21)
                *d21 = parsedD21;
            if (d32)
                *d32 = parsedD32;
            if (d43)
                *d43 = parsedD43;
            if (d50)
                *d50 = parsedD50;
            if (d90)
                *d90 = parsedD90;
            return true;
        }
    }

    const QStringList columns = trimmed.split(QRegularExpression("\\s+"), QString::SkipEmptyParts);
    if (columns.size() < 11)
        return false;

    bool ok1 = false;
    bool ok2 = false;
    bool ok3 = false;
    bool ok4 = false;
    bool ok5 = false;
    const double parsedD21 = columns.at(1).toDouble(&ok1);
    const double parsedD32 = columns.at(4).toDouble(&ok2);
    const double parsedD43 = columns.at(6).toDouble(&ok3);
    const double parsedD50 = columns.at(8).toDouble(&ok4);
    const double parsedD90 = columns.at(10).toDouble(&ok5);
    if (!(ok1 && ok2 && ok3 && ok4 && ok5))
        return false;

    if (d21)
        *d21 = parsedD21;
    if (d32)
        *d32 = parsedD32;
    if (d43)
        *d43 = parsedD43;
    if (d50)
        *d50 = parsedD50;
    if (d90)
        *d90 = parsedD90;
    return true;
}

bool parseLastNumericValue(const QString& line, double* value)
{
    if (!value)
        return false;

    static const QRegularExpression numericPattern(
        QStringLiteral(R"([-+]?(?:\d+\.?\d*|\.\d+)(?:[eE][-+]?\d+)?)"));
    QRegularExpressionMatchIterator iterator = numericPattern.globalMatch(line);

    bool found = false;
    double parsedValue = 0.0;
    while (iterator.hasNext()) {
        const QRegularExpressionMatch match = iterator.next();
        bool ok = false;
        const double candidate = match.captured(0).toDouble(&ok);
        if (!ok)
            continue;
        parsedValue = candidate;
        found = true;
    }

    if (!found)
        return false;

    *value = parsedValue;
    return true;
}

bool verifyBackgroundSignalFileContents(const QString& filePath,
                                        const QVector<double>& expectedSamples,
                                        QString* error = nullptr)
{
    QVector<double> actualSamples;
    if (!readNumericSamplesFile(filePath, &actualSamples, error))
        return false;

    if (actualSamples.size() != expectedSamples.size()) {
        if (error) {
            *error = QStringLiteral("背景文件样本数不匹配：%1 | expected=%2 | actual=%3")
                    .arg(nativePath(filePath))
                    .arg(expectedSamples.size())
                    .arg(actualSamples.size());
        }
        return false;
    }

    for (int i = 0; i < expectedSamples.size(); ++i) {
        if (!qFuzzyCompare(actualSamples.at(i) + 1.0, expectedSamples.at(i) + 1.0)) {
            if (error) {
                *error = QStringLiteral("背景文件内容不匹配：%1 | row=%2 | expected=%3 | actual=%4")
                        .arg(nativePath(filePath))
                        .arg(i + 1)
                        .arg(expectedSamples.at(i), 0, 'f', 6)
                        .arg(actualSamples.at(i), 0, 'f', 6);
            }
            return false;
        }
    }

    return true;
}

QString distributionFilePath(const QString& appDir,
                             const QString& prefix,
                             const QDateTime& timestamp)
{
    return QDir(distributionDirectoryPath(appDir)).filePath(
        QStringLiteral("%1_%2.txt").arg(prefix, measurementDay(timestamp)));
}

bool writeTextFileAtomically(const QString& filePath,
                             const QString& content,
                             QString* error = nullptr)
{
    QDir().mkpath(QFileInfo(filePath).absolutePath());

    QSaveFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        if (error)
            *error = file.errorString();
        return false;
    }

    QTextStream out(&file);
    out.setCodec("UTF-8");
    out << content;

    if (!file.commit()) {
        if (error)
            *error = file.errorString();
        return false;
    }

    return true;
}

bool appendTextLine(const QString& filePath,
                    const QString& line,
                    QString* error = nullptr)
{
    QDir().mkpath(QFileInfo(filePath).absolutePath());

    QFile file(filePath);
    if (!file.open(QIODevice::Append | QIODevice::Text)) {
        if (error)
            *error = file.errorString();
        return false;
    }

    QTextStream out(&file);
    out.setCodec("UTF-8");
    out << line << "\n";
    return true;
}

bool parseLastNumericField(const QString& line, double* value)
{
    if (!value)
        return false;

    const QString trimmed = line.trimmed();
    if (trimmed.isEmpty())
        return false;

    const QStringList columns = trimmed.split(QRegularExpression("[\\s,]+"), QString::SkipEmptyParts);
    for (int index = columns.size() - 1; index >= 0; --index) {
        bool ok = false;
        const double parsed = columns.at(index).toDouble(&ok);
        if (ok) {
            *value = parsed;
            return true;
        }
    }

    return false;
}

QVector<double> readTrailingNumericColumnFile(const QString& filePath)
{
    QVector<double> values;
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        return values;

    QTextStream in(&file);
    while (!in.atEnd()) {
        double parsedValue = 0.0;
        if (parseLastNumericField(in.readLine(), &parsedValue))
            values.push_back(parsedValue);
    }

    return values;
}

QCPRange paddedRange(double minValue, double maxValue)
{
    if (!std::isfinite(minValue) || !std::isfinite(maxValue))
        return QCPRange(0, 1);

    if (qFuzzyCompare(minValue + 1.0, maxValue + 1.0)) {
        const double padding = qMax(1.0, std::abs(minValue) * 0.2 + 0.5);
        return QCPRange(minValue - padding, maxValue + padding);
    }

    const double span = std::abs(maxValue - minValue);
    const double padding = qMax(0.5, span * 0.12);
    return QCPRange(minValue - padding, maxValue + padding);
}

void ensureTextFits(QWidget* widget, const QString& text, int horizontalPadding = 28)
{
    if (!widget)
        return;

    QFontMetrics metrics(widget->font());
    const int requiredWidth = metrics.horizontalAdvance(text) + horizontalPadding;
    if (requiredWidth <= 0)
        return;

    QSizePolicy policy = widget->sizePolicy();
    policy.setHorizontalPolicy(QSizePolicy::Preferred);
    widget->setSizePolicy(policy);
    widget->setMinimumWidth(qMax(widget->minimumWidth(), requiredWidth));
    if (widget->maximumWidth() > 0 && widget->maximumWidth() < requiredWidth)
        widget->setMaximumWidth(requiredWidth);
}

void ensureTextHeight(QWidget* widget,
                      int verticalPadding = 14,
                      int minimumHeight = 0,
                      QSizePolicy::Policy verticalPolicy = QSizePolicy::Fixed)
{
    if (!widget)
        return;

    QFontMetrics metrics(widget->font());
    const int requiredHeight = qMax(minimumHeight, metrics.height() + verticalPadding);
    if (requiredHeight <= 0)
        return;

    QSizePolicy policy = widget->sizePolicy();
    policy.setVerticalPolicy(verticalPolicy);
    widget->setSizePolicy(policy);
    widget->setMinimumHeight(qMax(widget->minimumHeight(), requiredHeight));
    if (widget->maximumHeight() > 0 && widget->maximumHeight() < requiredHeight)
        widget->setMaximumHeight(requiredHeight);
}

void prepareMetricLabel(QLabel* label, const QString& sampleText = QStringLiteral("100.00"))
{
    if (!label)
        return;

    label->setAlignment(Qt::AlignCenter);
    QSizePolicy policy = label->sizePolicy();
    policy.setHorizontalPolicy(QSizePolicy::Preferred);
    policy.setVerticalPolicy(QSizePolicy::Fixed);
    label->setSizePolicy(policy);
    label->setMinimumWidth(qMax(label->minimumWidth(), 96));
    label->setMaximumWidth(QWIDGETSIZE_MAX);
    ensureTextHeight(label, 18, 40);
    ensureTextFits(label, sampleText, 22);
}

void prepareFieldLabel(QLabel* label,
                       int minimumHeight = 24,
                       int horizontalPadding = 20)
{
    if (!label)
        return;

    QSizePolicy policy = label->sizePolicy();
    policy.setHorizontalPolicy(QSizePolicy::Preferred);
    policy.setVerticalPolicy(QSizePolicy::Fixed);
    label->setSizePolicy(policy);
    label->setMinimumWidth(0);
    label->setMaximumWidth(QWIDGETSIZE_MAX);
    ensureTextHeight(label, 12, minimumHeight);
    ensureTextFits(label, label->text(), horizontalPadding);
}

void prepareSpinBoxField(QAbstractSpinBox* spinBox,
                         const QString& sampleText = QStringLiteral("88.88"))
{
    if (!spinBox)
        return;

    QSizePolicy policy = spinBox->sizePolicy();
    policy.setHorizontalPolicy(QSizePolicy::Expanding);
    policy.setVerticalPolicy(QSizePolicy::Fixed);
    spinBox->setSizePolicy(policy);
    spinBox->setMinimumWidth(0);
    spinBox->setMaximumWidth(QWIDGETSIZE_MAX);
    ensureTextHeight(spinBox, 14, 34);
    spinBox->setButtonSymbols(QAbstractSpinBox::UpDownArrows);
    spinBox->setKeyboardTracking(false);

    if (QLineEdit* editor = spinBox->findChild<QLineEdit*>()) {
        editor->setAlignment(Qt::AlignCenter);
        editor->setFrame(false);
        editor->setMinimumWidth(0);
        editor->setMaximumWidth(QWIDGETSIZE_MAX);
    }

    QFontMetrics metrics(spinBox->font());
    const int requiredWidth = metrics.horizontalAdvance(sampleText) + 136;
    spinBox->setMinimumWidth(qMax(spinBox->minimumWidth(), requiredWidth));
}

void updateComboBoxPresentation(QComboBox* comboBox,
                                const QString& sampleText = QString())
{
    if (!comboBox)
        return;

    const bool isWideStepCombo =
            comboBox->objectName() == QStringLiteral("PERangeStep")
            || comboBox->objectName() == QStringLiteral("PEDelayStep");

    QFontMetrics metrics(comboBox->font());
    int longestWidth = metrics.horizontalAdvance(sampleText);
    int longestLength = sampleText.size();
    for (int i = 0; i < comboBox->count(); ++i) {
        const QString itemText = comboBox->itemText(i);
        longestWidth = qMax(longestWidth, metrics.horizontalAdvance(itemText));
        longestLength = qMax(longestLength, itemText.size());
    }

    const int requiredWidth = longestWidth + (isWideStepCombo ? 320 : 248);
    comboBox->setMinimumContentsLength(qMax(comboBox->minimumContentsLength(),
                                            qMax(isWideStepCombo ? 20 : 16,
                                                 longestLength + (isWideStepCombo ? 14 : 10))));
    comboBox->setSizeAdjustPolicy(QComboBox::AdjustToMinimumContentsLengthWithIcon);
    comboBox->setMinimumWidth(qMax(comboBox->minimumWidth(), requiredWidth));

    if (QAbstractItemView* view = comboBox->view()) {
        view->setTextElideMode(Qt::ElideNone);
        view->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
        view->setMinimumWidth(qMax(view->minimumWidth(),
                                   requiredWidth + (isWideStepCombo ? 260 : 180)));
        if (QListView* listView = qobject_cast<QListView*>(view)) {
            listView->setSpacing(2);
            listView->setUniformItemSizes(false);
        }
    }
}

void prepareComboBoxField(QComboBox* comboBox,
                          const QString& sampleText = QStringLiteral("8888.88"))
{
    if (!comboBox)
        return;

    QSizePolicy policy = comboBox->sizePolicy();
    policy.setHorizontalPolicy(QSizePolicy::Expanding);
    policy.setVerticalPolicy(QSizePolicy::Fixed);
    comboBox->setSizePolicy(policy);
    comboBox->setMinimumWidth(0);
    comboBox->setMaximumWidth(QWIDGETSIZE_MAX);
    ensureTextHeight(comboBox, 14, 34);

    updateComboBoxPresentation(comboBox, sampleText);
}

void prepareTextButton(QAbstractButton* button,
                       int horizontalPadding = 34,
                       int verticalPadding = 18,
                       int minimumWidth = 120)
{
    if (!button)
        return;

    QSizePolicy policy = button->sizePolicy();
    policy.setHorizontalPolicy(QSizePolicy::Preferred);
    policy.setVerticalPolicy(QSizePolicy::Fixed);
    button->setSizePolicy(policy);
    button->setMinimumWidth(qMax(button->minimumWidth(), minimumWidth));
    ensureTextHeight(button, verticalPadding, 46);
    ensureTextFits(button, button->text(), horizontalPadding);
}

void prepareEditorWidget(QWidget* widget,
                         int verticalPadding = 14,
                         int minimumHeight = 34)
{
    if (!widget)
        return;

    QSizePolicy policy = widget->sizePolicy();
    policy.setVerticalPolicy(QSizePolicy::Fixed);
    widget->setSizePolicy(policy);
    ensureTextHeight(widget, verticalPadding, minimumHeight);
}

bool shouldUseCompactLayout(const QWidget* window)
{
    if (!window)
        return false;

    const QSize windowSize = window->size();
    if ((windowSize.width() > 0 && windowSize.width() <= 1152)
        || (windowSize.height() > 0 && windowSize.height() <= 820)) {
        return true;
    }

    QScreen* screen = nullptr;
    if (window->windowHandle())
        screen = window->windowHandle()->screen();
    if (!screen)
        screen = QGuiApplication::screenAt(window->frameGeometry().center());
    if (!screen)
        return false;

    const QRect available = screen->availableGeometry();
    return available.width() <= 1152 || available.height() <= 820;
}

QScreen* screenForWidget(const QWidget* widget)
{
    if (!widget)
        return nullptr;

    if (widget->windowHandle() && widget->windowHandle()->screen())
        return widget->windowHandle()->screen();

    QScreen* screen = QGuiApplication::screenAt(widget->frameGeometry().center());
    if (screen)
        return screen;

    return QGuiApplication::primaryScreen();
}

int scaledPixels(int baseValue,
                 qreal scale,
                 int minimumValue,
                 int maximumValue = std::numeric_limits<int>::max())
{
    return qBound(minimumValue, qRound(baseValue * scale), maximumValue);
}

qreal computeResponsiveUiScale(const QSize& size)
{
    if (!size.isValid())
        return 1.0;

    const qreal widthScale = static_cast<qreal>(size.width()) / 1366.0;
    const qreal heightScale = static_cast<qreal>(size.height()) / 768.0;
    return qBound<qreal>(0.72, qMin(widthScale, heightScale), 1.38);
}

void setWidgetPointSize(QWidget* widget, qreal pointSize, int weight = -1)
{
    if (!widget || pointSize <= 0.0)
        return;

    QFont font = widget->font();
    font.setPointSizeF(pointSize);
    if (weight >= 0)
        font.setWeight(weight);
    widget->setFont(font);
}

void scaleWidgetFont(QWidget* widget, qreal factor, int minimumPointSize = 10)
{
    if (!widget)
        return;

    QFont font = widget->font();
    const qreal pointSize = font.pointSizeF();
    if (pointSize <= 0.0)
        return;

    font.setPointSizeF(qMax(static_cast<qreal>(minimumPointSize), pointSize * factor));
    widget->setFont(font);
}

void scaleFixedWidgetSize(QWidget* widget,
                          qreal factor,
                          int minimumWidth = 48,
                          int minimumHeight = 24)
{
    if (!widget)
        return;

    QSize minimum = widget->minimumSize();
    QSize maximum = widget->maximumSize();

    if (minimum.width() > 0 && minimum.width() < QWIDGETSIZE_MAX)
        minimum.setWidth(qMax(minimumWidth, qRound(minimum.width() * factor)));
    if (minimum.height() > 0 && minimum.height() < QWIDGETSIZE_MAX)
        minimum.setHeight(qMax(minimumHeight, qRound(minimum.height() * factor)));

    if (maximum.width() > 0 && maximum.width() < QWIDGETSIZE_MAX)
        maximum.setWidth(qMax(minimum.width(), qRound(maximum.width() * factor)));
    if (maximum.height() > 0 && maximum.height() < QWIDGETSIZE_MAX)
        maximum.setHeight(qMax(minimum.height(), qRound(maximum.height() * factor)));

    widget->setMinimumSize(minimum);
    widget->setMaximumSize(maximum);
}

void makeWidgetWidthFlexible(QWidget* widget)
{
    if (!widget)
        return;

    QSizePolicy policy = widget->sizePolicy();
    policy.setHorizontalPolicy(QSizePolicy::Preferred);
    widget->setSizePolicy(policy);
    widget->setMinimumWidth(0);
    widget->setMaximumWidth(QWIDGETSIZE_MAX);
}

void makeWidgetFullyFlexible(QWidget* widget, const QSize& minimumSize = QSize())
{
    if (!widget)
        return;

    widget->setMinimumSize(minimumSize);
    widget->setMaximumSize(QWIDGETSIZE_MAX, QWIDGETSIZE_MAX);
    widget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
}

void clearLayoutItems(QLayout* layout)
{
    if (!layout)
        return;

    while (QLayoutItem* item = layout->takeAt(0))
        delete item;
}

bool validateSignalSampleFile(const QString& filePath, int minLines, QString* error = nullptr)
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        if (error)
            *error = QStringLiteral("无法读取文件：") + filePath;
        return false;
    }

    int numericLineCount = 0;
    while (!file.atEnd()) {
        bool ok = false;
        QString::fromUtf8(file.readLine()).trimmed().toDouble(&ok);
        if (ok)
            ++numericLineCount;
    }

    if (numericLineCount < minLines) {
        if (error)
            *error = QStringLiteral("文件内容不足：") + filePath;
        return false;
    }

    return true;
}

QStringList waveSignalDirectoriesForValidation(const QString& appDir)
{
    QStringList directories;
    directories << QDir(appDir).filePath(QStringLiteral("Signals"))
                << QDir::currentPath()
                << appDir;

    for (QString& directory : directories)
        directory = QDir::cleanPath(directory);
    directories.removeDuplicates();
    return directories;
}

QStringList waveSignalFilePathsForValidation(const QString& appDir)
{
    QStringList filePaths;
    for (const QString& directoryPath : waveSignalDirectoriesForValidation(appDir)) {
        const QDir directory(directoryPath);
        const QStringList fileNames =
                directory.entryList(QStringList() << QStringLiteral("WaveRf_*.txt"), QDir::Files);
        for (const QString& fileName : fileNames)
            filePaths << QDir(directoryPath).filePath(fileName);
    }

    filePaths.removeDuplicates();
    return filePaths;
}

Q_DECL_UNUSED bool validateTimedColumnsFileLegacy(const QString& filePath, int expectedColumns, QString* error = nullptr)
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        if (error)
            *error = QStringLiteral("无法读取文件：") + filePath;
        return false;
    }

    const QString line = QString::fromUtf8(file.readLine()).trimmed();
    const QStringList columns = line.split('\t', QString::KeepEmptyParts);
    if (columns.size() != expectedColumns) {
        if (error)
            *error = QStringLiteral("列数不正确：") + filePath;
        return false;
    }

    QRegularExpression timestampRe(QStringLiteral("^\\d{8}_\\d{6}$"));
    if (!timestampRe.match(columns.first()).hasMatch()) {
        if (error)
            *error = QStringLiteral("时间戳格式不正确：") + filePath;
        return false;
    }

    for (int i = 1; i < columns.size(); ++i) {
        bool ok = false;
        columns.at(i).toDouble(&ok);
        if (!ok) {
            if (error)
                *error = QStringLiteral("数值格式不正确：") + filePath;
            return false;
        }
    }

    return true;
}

bool validateTimedColumnsFile(const QString& filePath,
                              int expectedColumns,
                              int minValidLines = 1,
                              QString* error = nullptr)
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        if (error)
            *error = QStringLiteral("无法读取文件：") + filePath;
        return false;
    }

    const QRegularExpression timestampRe(QStringLiteral("^\\d{8}_\\d{6}$"));
    const QRegularExpression legacyTimestampRe(QStringLiteral("^(\\d{8}_\\d{6}|\\d{14})\\s*,?\\s*(.*)$"));
    int validLines = 0;
    while (!file.atEnd()) {
        const QString line = QString::fromUtf8(file.readLine()).trimmed();
        if (line.isEmpty())
            continue;

        QStringList numericColumns;
        QString timestampToken;

        const QStringList tabColumns = line.split('\t', QString::KeepEmptyParts);
        if (tabColumns.size() == expectedColumns && timestampRe.match(tabColumns.first()).hasMatch()) {
            timestampToken = tabColumns.first();
            for (int i = 1; i < tabColumns.size(); ++i)
                numericColumns << tabColumns.at(i).trimmed();
        } else {
            const QRegularExpressionMatch legacyMatch = legacyTimestampRe.match(line);
            if (!legacyMatch.hasMatch()) {
                if (error)
                    *error = QStringLiteral("时间戳格式不正确：") + filePath;
                return false;
            }

            timestampToken = legacyMatch.captured(1);
            numericColumns = legacyMatch.captured(2)
                                 .split(QRegularExpression("[\\s,]+"), QString::SkipEmptyParts);
        }

        if (!timestampRe.match(timestampToken).hasMatch()
                && !QRegularExpression(QStringLiteral("^\\d{14}$")).match(timestampToken).hasMatch()) {
            if (error)
                *error = QStringLiteral("时间戳格式不正确：") + filePath;
            return false;
        }

        if (numericColumns.size() != expectedColumns - 1) {
            if (error)
                *error = QStringLiteral("列数不正确：") + filePath;
            return false;
        }

        for (const QString& numericColumn : numericColumns) {
            bool ok = false;
            numericColumn.toDouble(&ok);
            if (!ok) {
                if (error)
                    *error = QStringLiteral("数值格式不正确：") + filePath;
                return false;
            }
        }

        ++validLines;
    }

    if (validLines < minValidLines) {
        if (error)
            *error = QStringLiteral("文件数据行不足：") + filePath;
        return false;
    }

    return true;
}

bool validateTimedColumnsFile(const QString& filePath, int expectedColumns, QString* error)
{
    return validateTimedColumnsFile(filePath, expectedColumns, 1, error);
}

bool validateSpectrumFile(const QString& filePath, int minLines, QString* error = nullptr)
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        if (error)
            *error = QStringLiteral("无法读取文件：") + filePath;
        return false;
    }

    int validLines = 0;
    while (!file.atEnd()) {
        const QString line = QString::fromUtf8(file.readLine()).trimmed();
        if (line.isEmpty())
            continue;

        const QStringList columns = line.split(QRegularExpression("\\s+"), QString::SkipEmptyParts);
        if (columns.size() < 2)
            continue;

        bool ok1 = false;
        bool ok2 = false;
        columns.at(0).toDouble(&ok1);
        columns.at(1).toDouble(&ok2);
        if (ok1 && ok2)
            ++validLines;
    }

    if (validLines < minLines) {
        if (error)
            *error = QStringLiteral("衰减谱文件内容无效：") + filePath;
        return false;
    }

    return true;
}

void configurePlotAppearance(QCustomPlot* plot,
                             const QColor& background,
                             const QColor& borderColor,
                             const QColor& axisColor,
                             const QColor& gridColor,
                             const QColor& textColor,
                             const QColor& lineColor)
{
    if (!plot)
        return;

    plot->setBackground(background);
    plot->setStyleSheet(QStringLiteral(
        "background: transparent;"
        "border: 1px solid %1;"
        "border-radius: 16px;").arg(borderColor.name()));
    plot->axisRect()->setBackground(background);

    QPen axisPen(axisColor, 1.2);
    QPen gridPen(gridColor, 1.0, Qt::DotLine);

    plot->xAxis->setBasePen(axisPen);
    plot->yAxis->setBasePen(axisPen);
    plot->xAxis->setTickPen(axisPen);
    plot->yAxis->setTickPen(axisPen);
    plot->xAxis->setSubTickPen(axisPen);
    plot->yAxis->setSubTickPen(axisPen);
    plot->xAxis->setTickLabelColor(textColor);
    plot->yAxis->setTickLabelColor(textColor);
    plot->xAxis->setLabelColor(textColor);
    plot->yAxis->setLabelColor(textColor);
    plot->xAxis->grid()->setVisible(true);
    plot->yAxis->grid()->setVisible(true);
    plot->xAxis->grid()->setPen(gridPen);
    plot->yAxis->grid()->setPen(gridPen);

    for (int i = 0; i < plot->graphCount(); ++i)
        plot->graph(i)->setPen(QPen(lineColor, 2.0));

    plot->replot();
}

void configurePlotTypography(QCustomPlot* plot, qreal scale)
{
    if (!plot)
        return;

    const qreal effectiveScale = qBound<qreal>(0.85, scale, 1.8);
    QFont tickFont = plot->font();
    tickFont.setPointSizeF(qBound<qreal>(12.0, 13.5 * effectiveScale, 22.0));
    tickFont.setWeight(QFont::DemiBold);

    QFont labelFont = tickFont;
    labelFont.setPointSizeF(qBound<qreal>(14.0, 16.2 * effectiveScale, 26.0));
    labelFont.setWeight(QFont::Bold);

    plot->xAxis->setTickLabelFont(tickFont);
    plot->yAxis->setTickLabelFont(tickFont);
    plot->xAxis->setLabelFont(labelFont);
    plot->yAxis->setLabelFont(labelFont);
    plot->xAxis->setTickLabelPadding(scaledPixels(10, effectiveScale, 8, 18));
    plot->yAxis->setTickLabelPadding(scaledPixels(10, effectiveScale, 8, 18));
    plot->xAxis->setLabelPadding(scaledPixels(12, effectiveScale, 10, 22));
    plot->yAxis->setLabelPadding(scaledPixels(12, effectiveScale, 10, 22));
    plot->xAxis->setTickLength(scaledPixels(8, effectiveScale, 6, 14), scaledPixels(4, effectiveScale, 3, 7));
    plot->yAxis->setTickLength(scaledPixels(8, effectiveScale, 6, 14), scaledPixels(4, effectiveScale, 3, 7));

    if (plot->legend) {
        plot->legend->setVisible(false);
        plot->legend->setFont(tickFont);
    }
}

bool isSimulationSelfTestMode()
{
    return QCoreApplication::arguments().contains(QStringLiteral("--simulate-self-test"));
}

QString simulationSelfTestReportPath()
{
    const QString simulationDir =
        QDir(QCoreApplication::applicationDirPath()).filePath(QStringLiteral("Simulation"));
    QDir().mkpath(simulationDir);
    return QDir(simulationDir).filePath(QStringLiteral("self_test_report.txt"));
}

QString simulationCheckLine(bool passed,
                            const QString& code,
                            const QString& name,
                            const QString& detail = QString())
{
    QString line = QStringLiteral("[%1][%2] %3")
                       .arg(passed ? QStringLiteral("PASS") : QStringLiteral("FAIL"),
                            code,
                            name);
    if (!detail.isEmpty())
        line += QStringLiteral(" | %1").arg(detail);
    return line;
}

void writeSimulationSelfTestReport(const QString& status, const QStringList& details)
{
    QSaveFile file(simulationSelfTestReportPath());
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
        return;

    QTextStream out(&file);
    out.setCodec("UTF-8");
    out << status << "\n";
    out << QDateTime::currentDateTime().toString(Qt::ISODate) << "\n";
    for (const QString& detail : details)
        out << detail << "\n";
    file.commit();
}

bool validateRecentFile(const QString& filePath,
                        const QDateTime& baseline,
                        int toleranceSeconds,
                        QString* error = nullptr)
{
    const QFileInfo info(filePath);
    if (!info.exists()) {
        if (error)
            *error = QStringLiteral("文件不存在：") + filePath;
        return false;
    }

    if (!baseline.isValid())
        return true;

    if (info.lastModified() < baseline.addSecs(-toleranceSeconds)) {
        if (error)
            *error = QStringLiteral("文件时间过旧：") + filePath;
        return false;
    }

    return true;
}

bool validateMeasurementIntervals(const QString& filePath,
                                  int expectedIntervalSeconds,
                                  int minComparisons,
                                  QString* error = nullptr)
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        if (error)
            *error = QStringLiteral("无法读取文件：") + filePath;
        return false;
    }

    QList<QDateTime> timestamps;
    const QRegularExpression timestampTokenRe(QStringLiteral("^(\\d{8}_\\d{6}|\\d{14})"));
    while (!file.atEnd()) {
        const QString line = QString::fromUtf8(file.readLine()).trimmed();
        if (line.isEmpty())
            continue;

        const QRegularExpressionMatch timestampMatch = timestampTokenRe.match(line);
        if (!timestampMatch.hasMatch()) {
            if (error)
                *error = QStringLiteral("时间戳格式不正确：") + filePath;
            return false;
        }

        const QString timestampToken = timestampMatch.captured(1);
        const QDateTime timestamp = timestampToken.contains(QLatin1Char('_'))
                ? QDateTime::fromString(timestampToken, QStringLiteral("yyyyMMdd_hhmmss"))
                : QDateTime::fromString(timestampToken, QStringLiteral("yyyyMMddhhmmss"));
        if (!timestamp.isValid()) {
            if (error)
                *error = QStringLiteral("时间戳格式不正确：") + filePath;
            return false;
        }

        timestamps.append(timestamp);
    }

    if (timestamps.size() - 1 < minComparisons) {
        if (error)
            *error = QStringLiteral("时间间隔校验样本不足：") + filePath;
        return false;
    }

    for (int i = 1; i < timestamps.size(); ++i) {
        const qint64 deltaSeconds = timestamps.at(i - 1).secsTo(timestamps.at(i));
        if (deltaSeconds != expectedIntervalSeconds) {
            if (error) {
                *error = QStringLiteral("保存间隔不匹配：%1，第%2行与第%3行间隔=%4s，期望=%5s")
                             .arg(filePath)
                             .arg(i)
                             .arg(i + 1)
                             .arg(deltaSeconds)
                             .arg(expectedIntervalSeconds);
            }
            return false;
        }
    }

    return true;
}

bool isUiSettingsPlausible(double specfmin,
                           double specfmax,
                           double histime,
                           double avenum,
                           double width,
                           double interval)
{
    return std::isfinite(specfmin)
            && std::isfinite(specfmax)
            && std::isfinite(histime)
            && std::isfinite(avenum)
            && std::isfinite(width)
            && std::isfinite(interval)
            && specfmin > 0.0
            && specfmax > specfmin
            && specfmax <= 20.0
            && histime > 0.0
            && histime <= 60.0
            && avenum > 0.0
            && avenum <= 1000.0
            && width > 0.0
            && width <= 1000.0
            && interval > 0.0
            && interval <= 3600.0;
}

bool tryAssignLegacyUiSettings(const QString& deskline,
                               const QStringList& numericTokens,
                               UiSettings* settings)
{
    if (!settings || numericTokens.size() != 6)
        return false;

    bool ok[6] = {false, false, false, false, false, false};
    const double specfmin = numericTokens.at(0).toDouble(&ok[0]);
    const double specfmax = numericTokens.at(1).toDouble(&ok[1]);
    const double histime = numericTokens.at(2).toDouble(&ok[2]);
    const double avenum = numericTokens.at(3).toDouble(&ok[3]);
    const double width = numericTokens.at(4).toDouble(&ok[4]);
    const double interval = numericTokens.at(5).toDouble(&ok[5]);

    if (!std::all_of(std::begin(ok), std::end(ok), [](bool value) { return value; }))
        return false;

    if (!isUiSettingsPlausible(specfmin, specfmax, histime, avenum, width, interval))
        return false;

    settings->deskline = sanitizeUserFileDirectory(deskline);
    settings->specfmin = specfmin;
    settings->specfmax = specfmax;
    settings->histime = histime;
    settings->avenum = avenum;
    settings->width = width;
    settings->interval = interval;
    return true;
}

bool parseLegacyUiSettings(const QString& raw, UiSettings* settings)
{
    if (!settings)
        return false;

    const QStringList tokens = raw.split(QRegularExpression("\\s+"), QString::SkipEmptyParts);
    if (tokens.size() < 6)
        return false;

    const int base = tokens.size() - 6;
    if (tryAssignLegacyUiSettings(tokens.mid(0, base).join(" ").trimmed(),
                                  tokens.mid(base, 6),
                                  settings)) {
        return true;
    }

    return false;
}

QStringList legacyUiSettingsCandidatePaths()
{
    const QString appDir = QCoreApplication::applicationDirPath();
    const QString currentDir = QDir::currentPath();
    QStringList candidates;
    const auto addCandidate = [&candidates](const QString& path) {
        const QString cleanPath = QDir::cleanPath(path);
        if (!cleanPath.isEmpty() && !candidates.contains(cleanPath))
            candidates << cleanPath;
    };

    addCandidate(QDir(appDir).filePath(QStringLiteral("Para.txt")));
    addCandidate(QDir(appDir).filePath(QStringLiteral("windeployqt/Para.txt")));
    addCandidate(QDir(appDir).filePath(QStringLiteral("myinverse/Para.txt")));
    addCandidate(QDir(appDir).filePath(QStringLiteral("../windeployqt/Para.txt")));
    addCandidate(QDir(appDir).filePath(QStringLiteral("../myinverse/Para.txt")));
    addCandidate(QDir(currentDir).filePath(QStringLiteral("Para.txt")));
    addCandidate(QDir(currentDir).filePath(QStringLiteral("windeployqt/Para.txt")));
    addCandidate(QDir(currentDir).filePath(QStringLiteral("myinverse/Para.txt")));
    return candidates;
}

UiSettings loadUiSettingsFile()
{
    UiSettings settings;
    UiSettings jsonSettings = settings;
    bool hasJsonSettings = false;

    for (const QString& candidatePath : legacyUiSettingsCandidatePaths()) {
        QFile file(candidatePath);
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
            continue;

        const QByteArray raw = file.readAll();
        const QString trimmed = QString::fromUtf8(raw).trimmed();
        if (trimmed.size() >= 6 && !trimmed.startsWith(QLatin1Char('{'))) {
            UiSettings legacySettings;
            if (parseLegacyUiSettings(QString::fromUtf8(raw), &legacySettings))
                return legacySettings;
            continue;
        }

        if (!hasJsonSettings && QDir::cleanPath(candidatePath) == QDir::cleanPath(uiSettingsFilePath())) {
            QJsonParseError parseError;
            const QJsonDocument document = QJsonDocument::fromJson(raw, &parseError);
            if (parseError.error == QJsonParseError::NoError && document.isObject()) {
                const QJsonObject object = document.object();
                jsonSettings.deskline = object.value(QStringLiteral("deskline")).toString(jsonSettings.deskline);
                jsonSettings.specfmin = object.value(QStringLiteral("specfmin")).toDouble(jsonSettings.specfmin);
                jsonSettings.specfmax = object.value(QStringLiteral("specfmax")).toDouble(jsonSettings.specfmax);
                jsonSettings.histime = object.value(QStringLiteral("histime")).toDouble(jsonSettings.histime);
                jsonSettings.avenum = object.value(QStringLiteral("avenum")).toDouble(jsonSettings.avenum);
                jsonSettings.width = object.value(QStringLiteral("width")).toDouble(jsonSettings.width);
                jsonSettings.interval = object.value(QStringLiteral("interval")).toDouble(jsonSettings.interval);
                jsonSettings.calib = object.value(QStringLiteral("calib")).toDouble(jsonSettings.calib);
                hasJsonSettings = true;
            }
        }
    }

    if (hasJsonSettings)
        settings = jsonSettings;

    settings.deskline = sanitizeUserFileDirectory(settings.deskline);

    return settings;
}

bool saveUiSettingsFile(const UiSettings& settings, QString* error)
{
    QJsonObject obj;
    obj[QStringLiteral("deskline")] = sanitizeUserFileDirectory(settings.deskline);
    obj[QStringLiteral("specfmin")] = settings.specfmin;
    obj[QStringLiteral("specfmax")] = settings.specfmax;
    obj[QStringLiteral("histime")] = settings.histime;
    obj[QStringLiteral("avenum")] = settings.avenum;
    obj[QStringLiteral("width")] = settings.width;
    obj[QStringLiteral("interval")] = settings.interval;
    obj[QStringLiteral("calib")] = settings.calib;

    QJsonDocument doc(obj);

    QSaveFile file(uiSettingsFilePath());
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        if (error)
            *error = file.errorString();
        return false;
    }

    file.write(doc.toJson(QJsonDocument::Indented));
    if (!file.commit()) {
        if (error)
            *error = file.errorString();
        return false;
    }

    return true;
}

}

// 宏定义 PARAMETER_CHANGED，用于修改设备参数并显示响应
// 注意: 调用此宏后可在同一作用域访问 QString response 变量
#define PARAMETER_CHANGED(parameter, value)\
    QString parameterString = QString(parameter) + QString(":%1").arg(value);\
    QString response;\
    if (_pa22xClient) {\
        UTShadowDevice* dev = _pa22xClient->getSelectedShadowDevice();\
        if (dev)\
            response = dev->parameterChanged(parameterString);\
    }

// 定义宏 delayOffsetMM，将纳秒延迟转换为毫米，并计算结果
#define delayOffsetMM(ns, v) Math::NStoMM(ns * 1000, v, 1);

pa22x::pa22x(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::pa22x)
{
    g_shutdownLoggingSuppressed.store(false);
    ui->setupUi(this);
    clearLegacyInlineStyles(this);
    setWindowFlag(Qt::Window, true);
    setWindowFlag(Qt::FramelessWindowHint, true);

    setWindowTitle(QStringLiteral("石灰石浆液细度在线检测系统"));
    applyStaticTexts();
    ui->label_19->setText(QStringLiteral("文件目录"));
    ui->lineEdit_Desk->setPlaceholderText(defaultUserFileDirectory());
    ui->lineEdit_Desk->setToolTip(
        QStringLiteral("用于保存/加载参数与历史文件的默认目录，不影响反演输入输出路径"));

    ui->customPlot3->addGraph();
    ui->customPlot4->addGraph();

    ui->pushButton_mode->setCheckable(true);
    ui->pushButton_mode->setChecked(false);
    applyVisualTheme(false);

     // 设置 PEDualModeSelection 下拉框
     ui->PEDualModeSelection->clear();
     ui->PEDualModeSelection->addItem("single", QVariant(kDualModeSingle));
     ui->PEDualModeSelection->addItem("dual", QVariant(kDualModeDual));
     ui->PEDualModeSelection->setCurrentIndex(0);
     updateComboBoxPresentation(ui->PEDualModeSelection, QStringLiteral("reflection-single"));


     // 设置 PEChannelCountSelection 下拉框
     refreshPEChannelCountSelectionUi(nullptr);
     applyLockedPeChannelUi();

     inputBinding(ui->PEGainInput,
                  0.0,110.0,
                  ui->PEGainSetp,
                  QString("0.1,1.0,2.0,6.0"),
                  2);

     inputBinding(ui->PERangeInput,
                  15.0,10000.0,
                  ui->PERangeStep,
                  QString("1.0,10.0,100.0"),
                  1);
     inputBinding(ui->PEDelayInput,
                  0.0,10000.0,
                  ui->PEDelayStep,
                  QString("0.1,1.0,5.0,10.0"),
                  1);
     ui->PEPRFInput->setMinimum(50);
     ui->PEPRFInput->setMaximum(10000);
     ui->PEPRFInput->setSingleStep(1);
     if (QLineEdit* peprfEditor = ui->PEPRFInput->findChild<QLineEdit*>())
         peprfEditor->setReadOnly(true);

     normalizeLegacyParameterEditors();
     alignParameterGroupRows();

     //检测
     ui->SpecAnaly->setCheckable(true);
     ui->SpecAnaly->setChecked(false);
     refreshWaveCaptureButtonState();
     ui->simulateDetectButton->setVisible(false);
     ui->lineEdit_Desk->setClearButtonEnabled(true);
     g_statusBar = ui->statusbar;
     if (ui->statusbar)
         ui->statusbar->showMessage(QStringLiteral("系统就绪"), 3000);
     QShortcut* internalToolsShortcut =
             new QShortcut(QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_Space), this);
     internalToolsShortcut->setContext(Qt::ApplicationShortcut);
     connect(internalToolsShortcut, &QShortcut::activated, this, [this]() {
         setInternalToolsUnlocked(!m_internalToolsUnlocked);
     });

     //连接
     ui->connectButton->setCheckable(true);
     ui->connectButton->setChecked(false);


     //检测更新图表
     QObject::connect(this, &pa22x::updatePlotData,this, &pa22x::onPlotDataUpdate);
     //检测更新label
     QObject::connect(this, &pa22x::updateLabelData,this, &pa22x::onLabelDataUpdate);
     //弹窗报错
     QObject::connect(this, &pa22x::error,this, &pa22x::onerror);
     QObject::connect(ui->PEChannelSelection,
                      SIGNAL(activated(int)),
                      this,
                      SLOT(PEChannelActived(int)),
                      Qt::UniqueConnection);
     auto spectrumPreviewRefresh = [this]() {
         if (!m_backgroundDirty) {
             QString previewError;
             refreshCurrentSpectrumPreview(true, &previewError);
         }
     };
     connect(ui->spb_specfmin,
             static_cast<void(QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged),
             this,
             [spectrumPreviewRefresh](double) { spectrumPreviewRefresh(); });
     connect(ui->spb_specfmax,
             static_cast<void(QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged),
             this,
             [spectrumPreviewRefresh](double) { spectrumPreviewRefresh(); });
     connect(ui->dspb_width,
             static_cast<void(QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged),
             this,
             [spectrumPreviewRefresh](double) { spectrumPreviewRefresh(); });
     connect(ui->lineEdit_Desk, &QLineEdit::editingFinished, this, [this]() {
         ui->lineEdit_Desk->setText(defaultFileDirectory());
         saveUiSettingsToDisk();
     });

     //初始化
     ui->dspb_width->setValue(16);
     ui->spb_time->setValue(10);
     ui->sb_avenum->setValue(10);
     ui->spb_specfmin->setValue(3);
     ui->spb_specfmax->setValue(7.5);
     ui->spb_histime->setValue(2);


     // 在构造函数或初始化函数中
     ui->PEGainInput->setReadOnly(false);      // 移除只读限制
     ui->PERangeInput->setReadOnly(false);      // 移除只读限制
     ui->PEDelayInput->setReadOnly(false);      // 移除只读限制

//     ui->customPlot3->setOpenGl(true); // 启用硬件加速
//     ui->customPlot4->setOpenGl(true);

//     qDebug()<<"opengle="<<ui->customPlot3->openGl();
//     qDebug()<<"opengle="<<ui->customPlot4->openGl();

     //多余但是强制用的
     ui->PEPRFInput->setVisible(0);

     Logger::init(/*保留天数*/7, /*也输出到控制台*/true);
     if (!g_xlsxLogger) {
         g_xlsxLogger = new xlsxHandle();
         g_xlsxLogger->createNewExcelFile();
     }
     logStep(QStringLiteral("系统"),
             QStringLiteral("界面初始化"),
             QStringLiteral("模拟入口=%1, 自测授权=%2\n%3")
                 .arg(m_internalToolsUnlocked ? QStringLiteral("显示") : QStringLiteral("隐藏"),
                      hasInternalToolsAccess() ? QStringLiteral("开启") : QStringLiteral("关闭"),
                      runtimePathSummary()));
     if (ui->statusbar) {
         ui->statusbar->showMessage(
             QStringLiteral("程序目录：%1 | 启动目录：%2")
                 .arg(nativePath(QCoreApplication::applicationDirPath()),
                      nativePath(QDir::currentPath())),
             12000);
     }

     _displayTimer = 0;
     _evaluatorTimer = 0;

     if (QWidget* root = centralWidget()) {
         if (QLayout* mainLayout = root->layout()) {
             mainLayout->setContentsMargins(18, 14, 18, 18);
             mainLayout->setSpacing(12);
         }

         if (QGridLayout* gridLayout = qobject_cast<QGridLayout*>(root->layout())) {
             gridLayout->setColumnStretch(0, 5);
             gridLayout->setColumnStretch(1, 2);
             gridLayout->setRowStretch(2, 1);
         }
     }

     ui->widget_21->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
     ui->widget_21->setMinimumSize(0, 84);
     ui->widget_21->setMaximumSize(QWIDGETSIZE_MAX, 96);
     ui->PEAWaveBackgroundView->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
     ui->PEAWaveBackgroundView->setMinimumSize(980, 640);
     ui->PEAWaveBackgroundView->setMaximumSize(QWIDGETSIZE_MAX, QWIDGETSIZE_MAX);

     loadUiSettingsFromDisk();

    // 自动保存：每次调参立刻写Para.txt，断电/卡退不丢
    auto saveSettings = [this]() { saveUiSettingsToDisk(); };
    connect(ui->spb_specfmin, QOverload<double>::of(&QDoubleSpinBox::valueChanged), saveSettings);
    connect(ui->spb_specfmax, QOverload<double>::of(&QDoubleSpinBox::valueChanged), saveSettings);
    connect(ui->spb_histime, QOverload<double>::of(&QDoubleSpinBox::valueChanged), saveSettings);
    connect(ui->dspb_width, QOverload<double>::of(&QDoubleSpinBox::valueChanged), saveSettings);
    connect(ui->sb_avenum, QOverload<int>::of(&QSpinBox::valueChanged), saveSettings);
    connect(ui->spb_time, QOverload<int>::of(&QSpinBox::valueChanged), saveSettings);
    connect(ui->lineEdit_Desk, &QLineEdit::editingFinished, saveSettings);
    if (m_lineEditBiaoding)
        connect(m_lineEditBiaoding, &QLineEdit::editingFinished, saveSettings);

}

void pa22x::loadUiSettingsFromDisk()
{
    const UiSettings settings = loadUiSettingsFile();
    ui->lineEdit_Desk->setText(settings.deskline);
    ui->spb_specfmin->setValue(settings.specfmin);
    ui->spb_specfmax->setValue(settings.specfmax);
    ui->spb_histime->setValue(settings.histime);
    ui->sb_avenum->setValue(static_cast<int>(settings.avenum));
    ui->dspb_width->setValue(settings.width);
    ui->spb_time->setValue(static_cast<int>(settings.interval));
    if (m_lineEditBiaoding)
        m_lineEditBiaoding->setText(QString::number(settings.calib, 'f', 1));
}

void pa22x::clearStaleDetectionOutputFiles(bool announce)
{
    const QString appDir = QCoreApplication::applicationDirPath();
    QStringList removedFiles;

    for (const QString& fileName : {
             QStringLiteral("1.txt"),
             QStringLiteral("2.txt"),
             QStringLiteral("3.txt"),
             QStringLiteral("4.txt") }) {
        const QString filePath = QDir(appDir).filePath(fileName);
        if (QFile::exists(filePath) && QFile::remove(filePath))
            removedFiles << nativePath(filePath);
    }

    if (!announce || removedFiles.isEmpty())
        return;

    logStep(QStringLiteral("检测"),
            QStringLiteral("DET-00"),
            QStringLiteral("旧检测输出已清理"),
            removedFiles.join(QStringLiteral(" | ")));
}

void pa22x::saveUiSettingsToDisk() const
{
    UiSettings settings;
    settings.deskline = sanitizeUserFileDirectory(ui->lineEdit_Desk->text());
    settings.specfmin = ui->spb_specfmin->value();
    settings.specfmax = ui->spb_specfmax->value();
    settings.histime = ui->spb_histime->value();
    settings.avenum = ui->sb_avenum->value();
    settings.width = ui->dspb_width->value();
    settings.interval = ui->spb_time->value();
    if (m_lineEditBiaoding) {
        bool ok = false;
        settings.calib = m_lineEditBiaoding->text().trimmed().toDouble(&ok);
        if (!ok) settings.calib = 0.0;
    }

    QString errorMessage;
    if (!saveUiSettingsFile(settings, &errorMessage))
        qWarning() << "保存 Para.txt 失败:" << errorMessage;
}

QString pa22x::defaultFileDirectory() const
{
    if (!ui || !ui->lineEdit_Desk)
        return defaultUserFileDirectory();

    return sanitizeUserFileDirectory(ui->lineEdit_Desk->text());
}

QString pa22x::fileDialogPath(const QString& fileName) const
{
    const QString directory = defaultFileDirectory();
    if (fileName.trimmed().isEmpty())
        return directory;
    return QDir(directory).filePath(fileName);
}

void pa22x::rememberFileDialogPath(const QString& selectedPath)
{
    if (!ui || !ui->lineEdit_Desk || selectedPath.trimmed().isEmpty())
        return;

    const QFileInfo info(selectedPath);
    const QString directory = sanitizeUserFileDirectory(info.isDir()
                                                            ? info.absoluteFilePath()
                                                            : info.absolutePath());
    ui->lineEdit_Desk->setText(directory);
    saveUiSettingsToDisk();
}

void pa22x::applyLockedPeChannelUi()
{
    if (!ui)
        return;

    for (QWidget* widget : {
             static_cast<QWidget*>(ui->deviceSelection),
             static_cast<QWidget*>(ui->label_19),
             static_cast<QWidget*>(ui->lineEdit_Desk),
             static_cast<QWidget*>(ui->PEChannelCountLabel),
             static_cast<QWidget*>(ui->PEChannelCountSelection) }) {
        if (widget)
            widget->setVisible(false);
    }

    if (!ui->PEChannelCountSelection)
        return;

    ui->PEChannelCountSelection->blockSignals(true);
    ui->PEChannelCountSelection->clear();
    ui->PEChannelCountSelection->addItem(QString::number(kFixedPEChannelCount),
                                         QVariant(kFixedPEChannelCount));
    ui->PEChannelCountSelection->setCurrentIndex(0);
    ui->PEChannelCountSelection->setEnabled(false);
    ui->PEChannelCountSelection->blockSignals(false);
}

void pa22x::enforceLockedPeChannelCount(UTShadowDevice* selectedShadow, bool pushToDevice)
{
    applyLockedPeChannelUi();

    if (!_pa22xClient)
        return;

    if (pushToDevice)
        _pa22xClient->setPEChannelNumberCount(kFixedPEChannelCount);

    for (UTShadowDevice* shadowDevice : _pa22xClient->getShadowDevices()) {
        if (!shadowDevice || shadowDevice->getDeviceType() != kDeviceTypePE)
            continue;

        QVector<QMap<QString, QString>>& parameters = shadowDevice->getParameters();
        const int allowedChannels = qMin(kFixedPEChannelCount, parameters.size());
        shadowDevice->setChannelNumberCount(allowedChannels);

        if (allowedChannels <= 0)
            continue;

        const QString forcedChannel = forcedPeChannelIdentifier(shadowDevice);
        if (!forcedChannel.isEmpty()) {
            shadowDevice->setSelectedChannel(forcedChannel);
            continue;
        }

        QString selectedChannel = shadowDevice->getSelectedChannel();
        int selectedIndex = shadowDevice->identifierToIndex(selectedChannel);
        if (selectedIndex < 0 || selectedIndex >= allowedChannels)
            shadowDevice->setSelectedChannel(parameters.at(0).value(QStringLiteral("identifier")));
    }

    if (!selectedShadow)
        selectedShadow = _pa22xClient->getSelectedShadowDevice();

    if (selectedShadow && selectedShadow->getDeviceType() == kDeviceTypePE) {
        QVector<QMap<QString, QString>>& parameters = selectedShadow->getParameters();
        const int allowedChannels = qMin(kFixedPEChannelCount, parameters.size());
        selectedShadow->setChannelNumberCount(allowedChannels);
        if (allowedChannels > 0) {
            const QString forcedChannel = forcedPeChannelIdentifier(selectedShadow);
            if (!forcedChannel.isEmpty()) {
                selectedShadow->setSelectedChannel(forcedChannel);
            } else {
                QString selectedChannel = selectedShadow->getSelectedChannel();
                int selectedIndex = selectedShadow->identifierToIndex(selectedChannel);
                if (selectedIndex < 0 || selectedIndex >= allowedChannels)
                    selectedShadow->setSelectedChannel(parameters.at(0).value(QStringLiteral("identifier")));
            }
        }
    }
}

int pa22x::currentPeDualMode(UTShadowDevice* selectedShadow) const
{
    if (!selectedShadow && _pa22xClient)
        selectedShadow = _pa22xClient->getSelectedShadowDevice();

    if (selectedShadow && selectedShadow->getDeviceType() == kDeviceTypePE) {
        QVector<QMap<QString, QString>>& parameters = selectedShadow->getParameters();
        const QString selectedChannel = selectedShadow->getSelectedChannel();
        for (const QMap<QString, QString>& parameter : parameters) {
            if (parameter.value(QStringLiteral("identifier")) == selectedChannel)
                return parseDualModeValue(parameter.value(QStringLiteral("dual_mode")));
        }
    }

    if (ui && ui->PEDualModeSelection)
        return parseDualModeValue(ui->PEDualModeSelection->currentData().toString());

    return kDualModeDual;
}

QString pa22x::forcedPeChannelIdentifier(UTShadowDevice* selectedShadow, int dualMode) const
{
    if (!selectedShadow && _pa22xClient)
        selectedShadow = _pa22xClient->getSelectedShadowDevice();

    if (!selectedShadow || selectedShadow->getDeviceType() != kDeviceTypePE)
        return QString();

    const QVector<QMap<QString, QString>>& parameters = selectedShadow->getParameters();
    if (parameters.isEmpty())
        return QString();

    // 仅在实际可用通道为 1 时才强制通道，避免覆盖用户在双通道下的手动选择。
    const int configuredCount = selectedShadow->getChannelNumberCount();
    const int effectiveCount = configuredCount > 0 ? configuredCount : parameters.size();
    const int availableCount = qMin(parameters.size(), effectiveCount);
    if (availableCount <= 1)
        return parameters.first().value(QStringLiteral("identifier"));

    Q_UNUSED(dualMode);
    return QString();
}

void pa22x::applyStaticTexts()
{
    ui->label_7->setText(QStringLiteral("雷州脱硫石灰石浆液细度超声在线检测系统"));
    ui->label_7->setTextFormat(Qt::PlainText);
    ui->label_7->setWordWrap(false);
    ui->label_7->setAlignment(Qt::AlignCenter);
    ui->groupBox_5->setTitle(QStringLiteral("采集波形"));
    ui->groupBox_6->setTitle(QStringLiteral("检测设置"));
    ui->groupBox_4->setTitle(QStringLiteral("操作与结果"));
    ui->groupBox->setTitle(QStringLiteral("参数设置"));
    ui->connectButton->setText(QStringLiteral("连接"));
    ui->simulateDetectButton->setText(QStringLiteral("模拟检测"));
    refreshWaveCaptureButtonState();
    ui->SpecAnaly->setText(QStringLiteral("开始检测"));
    ui->WaSaveWave->setText(QStringLiteral("背景信号"));
    ui->PESaveParameters->setText(QStringLiteral("保存参数"));
    ui->PELoadParameters->setText(QStringLiteral("加载参数"));
    ui->STSaveWave->setText(QStringLiteral("信号保存"));
    ui->HisData->setText(QStringLiteral("历史数据"));
    ui->label_22->setText(QStringLiteral("通过率 (%)"));

    const QString placeholder = QStringLiteral("--");
    if (ui->label_d50->text().trimmed().isEmpty())
        ui->label_d50->setText(placeholder);
    if (ui->label_d90->text().trimmed().isEmpty())
        ui->label_d90->setText(placeholder);
    if (ui->label_pass2->text().trimmed().isEmpty())
        ui->label_pass2->setText(placeholder);
    if (ui->label_aveD90->text().trimmed().isEmpty())
        ui->label_aveD90->setText(placeholder);
    if (ui->label_avepass->text().trimmed().isEmpty())
        ui->label_avepass->setText(placeholder);

    for (QLabel* label : {
             ui->label_d50,
             ui->label_d90,
             ui->label_pass2,
             ui->label_aveD90,
             ui->label_avepass }) {
        prepareMetricLabel(label, QStringLiteral("100.00"));
        label->setToolTip(label->text());
    }

    // 在PEDelayInput下面塞一行: 标定值 + 输入框
    {
        m_lineEditBiaoding = new QLineEdit(ui->groupBox);
        m_lineEditBiaoding->setObjectName(QStringLiteral("lineEdit_biaoding"));
        m_lineEditBiaoding->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
        m_lineEditBiaoding->setMinimumSize(QSize(120, 40));
        m_lineEditBiaoding->setMaximumSize(QSize(120, 40));
        m_lineEditBiaoding->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
        m_lineEditBiaoding->setFont(ui->PEDelayInput->font());
        m_lineEditBiaoding->setStyleSheet(ui->PEDelayInput->styleSheet()
            .replace(QStringLiteral("QDoubleSpinBox"), QStringLiteral("QLineEdit")));
        m_lineEditBiaoding->setText(QStringLiteral("0.0"));

        QLabel* bdLabel = new QLabel(QStringLiteral("标定值"), ui->groupBox);
        bdLabel->setObjectName(QStringLiteral("label_biaoding"));
        bdLabel->setMinimumSize(QSize(190, 40));
        bdLabel->setMaximumSize(QSize(190, 40));
        bdLabel->setFont(ui->PEDelayLabel->font());
        bdLabel->setStyleSheet(QStringLiteral(""));

        QWidget* bdContainer = new QWidget(ui->groupBox);
        QHBoxLayout* bdRow = new QHBoxLayout(bdContainer);
        bdRow->setContentsMargins(0, 0, 0, 0);
        bdRow->setSpacing(6);
        bdRow->addWidget(bdLabel);
        bdRow->addItem(new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum));
        bdRow->addWidget(m_lineEditBiaoding);
        bdRow->addItem(new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum));
        QWidget* bdDummy = new QWidget(ui->groupBox);
        bdDummy->setMinimumSize(QSize(100, 40));
        bdDummy->setMaximumSize(QSize(100, 40));
        bdRow->addWidget(bdDummy);

        ui->verticalLayout->addItem(new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Fixed));
        ui->verticalLayout->addWidget(bdContainer);
    }

    for (QLabel* label : {
             ui->label_19,
             ui->PEChannelCountLabel,
             ui->PEChanneSelectionLabel,
             ui->PEDualModeLabel,
             ui->PEGainLabel,
             ui->PERangeLabel,
             ui->PEDelayLabel,
             ui->IPAddressLabel,
             ui->label_24,
             ui->label_25,
             ui->label_26,
             ui->label_27,
             ui->label_Ad50,
             ui->label_Ad90,
             ui->label_Pass }) {
        prepareFieldLabel(label, 36, 20);
    }

    for (QLabel* label : {
             ui->PEChanneSelectionLabel,
             ui->PEDualModeLabel,
             ui->PEGainLabel,
             ui->PERangeLabel,
             ui->PEDelayLabel,
             ui->IPAddressLabel,
             ui->label_20,
             ui->label_23,
             ui->label_24,
             ui->label_25,
             ui->label_26,
             ui->label_27 }) {
        if (label)
            label->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    }

    for (QLabel* label : {
             ui->label_20,
             ui->label_23,
             ui->label_21,
             ui->label_22 }) {
        prepareFieldLabel(label, 36, 24);
    }

    for (QAbstractSpinBox* spinBox : {
             static_cast<QAbstractSpinBox*>(ui->spb_specfmin),
             static_cast<QAbstractSpinBox*>(ui->spb_specfmax) }) {
        prepareSpinBoxField(spinBox, QStringLiteral("88.88"));
    }

    for (QAbstractSpinBox* spinBox : {
             static_cast<QAbstractSpinBox*>(ui->spb_histime),
             static_cast<QAbstractSpinBox*>(ui->sb_avenum),
             static_cast<QAbstractSpinBox*>(ui->dspb_width),
             static_cast<QAbstractSpinBox*>(ui->spb_time),
             static_cast<QAbstractSpinBox*>(ui->PEGainInput),
             static_cast<QAbstractSpinBox*>(ui->PERangeInput),
             static_cast<QAbstractSpinBox*>(ui->PEDelayInput) }) {
        prepareSpinBoxField(spinBox, QStringLiteral("8888.88"));
    }

    for (QWidget* editor : {
             static_cast<QWidget*>(ui->deviceSelection),
             static_cast<QWidget*>(ui->PEChannelCountSelection),
             static_cast<QWidget*>(ui->PEChannelSelection),
             static_cast<QWidget*>(ui->PEDualModeSelection),
             static_cast<QWidget*>(ui->PEGainSetp),
             static_cast<QWidget*>(ui->PERangeStep),
             static_cast<QWidget*>(ui->PEDelayStep),
             static_cast<QWidget*>(ui->lineEdit_Desk),
             static_cast<QWidget*>(ui->IPAddressInput) }) {
        prepareEditorWidget(editor, 14, 36);
    }

    for (QAbstractButton* button : {
             ui->connectButton,
             ui->simulateDetectButton,
             ui->PESaveWave,
             ui->SpecAnaly,
             ui->CloseWaveSave,
             ui->Inserve,
             ui->WaSaveWave,
             ui->PESaveParameters,
             ui->PELoadParameters,
             ui->STSaveWave,
             ui->HisData,
             ui->pushButton_mode }) {
        prepareTextButton(button);
    }

    ensureTextHeight(ui->label_7, 18, 58);

    if (QGridLayout* headerLayout = qobject_cast<QGridLayout*>(ui->widget_21->layout())) {
        headerLayout->setContentsMargins(54, 8, 22, 10);
        headerLayout->setHorizontalSpacing(18);
        headerLayout->setVerticalSpacing(4);
        headerLayout->setColumnStretch(0, 0);
        headerLayout->setColumnStretch(1, 1);
    }

    if (ui->label_headerIcon) {
        const QPixmap logo(QStringLiteral(":/images/tang.gif"));
        ui->label_headerIcon->setToolTip(QStringLiteral("唐"));
        ui->label_headerIcon->setAttribute(Qt::WA_TranslucentBackground, true);
        ui->label_headerIcon->setStyleSheet(QStringLiteral("background: transparent; border: none;"));
        ui->label_headerIcon->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
        ui->label_headerIcon->setMinimumSize(380, 60);
        ui->label_headerIcon->setMaximumSize(520, 74);
        if (!logo.isNull()) {
            ui->label_headerIcon->setPixmap(logo.scaled(480, 60,
                                                        Qt::KeepAspectRatio,
                                                        Qt::SmoothTransformation));
        }
    }

    //ensureTextFits(ui->label_7, ui->label_7->text(), 40);
    ensureTextFits(ui->label_20, ui->label_20->text(), 20);
    ensureTextFits(ui->label_23, ui->label_23->text(), 20);
    ensureTextFits(ui->label_24, ui->label_24->text(), 20);
    ensureTextFits(ui->label_25, ui->label_25->text(), 20);
    ensureTextFits(ui->label_26, ui->label_26->text(), 20);
    ensureTextFits(ui->label_27, ui->label_27->text(), 20);
    ensureTextFits(ui->label_19, ui->label_19->text(), 20);
    ensureTextFits(ui->PEChannelCountLabel, ui->PEChannelCountLabel->text(), 20);
    ensureTextFits(ui->PEChanneSelectionLabel, ui->PEChanneSelectionLabel->text(), 20);
    ensureTextFits(ui->PEDualModeLabel, ui->PEDualModeLabel->text(), 20);
    ensureTextFits(ui->PEGainLabel, ui->PEGainLabel->text(), 20);
    ensureTextFits(ui->PERangeLabel, ui->PERangeLabel->text(), 20);
    ensureTextFits(ui->PEDelayLabel, ui->PEDelayLabel->text(), 20);
    ensureTextFits(ui->IPAddressLabel, ui->IPAddressLabel->text(), 20);
    ensureTextFits(ui->label_Ad50, ui->label_Ad50->text(), 20);
    ensureTextFits(ui->label_Pass, ui->label_Pass->text(), 20);
    ensureTextFits(ui->label_Ad90, ui->label_Ad90->text(), 24);
    ensureTextFits(ui->label_21, ui->label_21->text(), 24);
    ensureTextFits(ui->label_22, ui->label_22->text(), 24);
    ensureTextFits(ui->connectButton, ui->connectButton->text(), 34);
    ensureTextFits(ui->simulateDetectButton, ui->simulateDetectButton->text(), 34);
    ensureTextFits(ui->PESaveWave, ui->PESaveWave->text(), 34);
    ensureTextFits(ui->SpecAnaly, ui->SpecAnaly->text(), 34);
    ensureTextFits(ui->WaSaveWave, ui->WaSaveWave->text(), 34);
    ensureTextFits(ui->PESaveParameters, ui->PESaveParameters->text(), 34);
    ensureTextFits(ui->PELoadParameters, ui->PELoadParameters->text(), 34);
    ensureTextFits(ui->STSaveWave, ui->STSaveWave->text(), 34);
    ensureTextFits(ui->HisData, ui->HisData->text(), 34);
    ensureTextFits(ui->pushButton_mode, ui->pushButton_mode->text(), 34);

    applyLockedPeChannelUi();
}

void pa22x::applyVisualTheme(bool lightMode)
{
    QFile styleFile(lightMode
                        ? QStringLiteral(":/style/Light_mode_style.qss")
                        : QStringLiteral(":/style/Dark_mode_style.qss"));
    if (styleFile.open(QFile::ReadOnly)) {
        const QString styleSheet = QString::fromUtf8(styleFile.readAll());
        setStyleSheet(styleSheet);
    }

    m_lightThemeActive = lightMode;

    ui->pushButton_mode->setText(lightMode
                                     ? QStringLiteral("深色界面")
                                     : QStringLiteral("亮色界面"));

    ui->PERulerVerView->setMode(lightMode ? 1 : 0);
    ui->PERulerHorView->setMode(lightMode ? 1 : 0);
    ui->PEAWaveView->setMode(lightMode ? 1 : 0);

    if (lightMode) {
        configurePlotAppearance(ui->customPlot3,
                                QColor(QStringLiteral("#fbfcfc")),
                                QColor(QStringLiteral("#c6d7dc")),
                                QColor(QStringLiteral("#6a8390")),
                                QColor(QStringLiteral("#d4e0e4")),
                                QColor(QStringLiteral("#32454d")),
                                QColor(QStringLiteral("#5d8f9c")));
        configurePlotAppearance(ui->customPlot4,
                                QColor(QStringLiteral("#fbfcfc")),
                                QColor(QStringLiteral("#c6d7dc")),
                                QColor(QStringLiteral("#6a8390")),
                                QColor(QStringLiteral("#d4e0e4")),
                                QColor(QStringLiteral("#32454d")),
                                QColor(QStringLiteral("#5d8f9c")));
    } else {
        configurePlotAppearance(ui->customPlot3,
                                QColor(QStringLiteral("#131d23")),
                                QColor(QStringLiteral("#425661")),
                                QColor(QStringLiteral("#89a8b3")),
                                QColor(QStringLiteral("#293940")),
                                QColor(QStringLiteral("#dbe5e8")),
                                QColor(QStringLiteral("#88b4c0")));
        configurePlotAppearance(ui->customPlot4,
                                QColor(QStringLiteral("#131d23")),
                                QColor(QStringLiteral("#425661")),
                                QColor(QStringLiteral("#89a8b3")),
                                QColor(QStringLiteral("#293940")),
                                QColor(QStringLiteral("#dbe5e8")),
                                QColor(QStringLiteral("#88b4c0")));
    }

    configurePlotTypography(ui->customPlot3, m_uiScaleFactor > 0.0 ? m_uiScaleFactor : 1.0);
    configurePlotTypography(ui->customPlot4, m_uiScaleFactor > 0.0 ? m_uiScaleFactor : 1.0);
    applyWaveViewStyle();
    if (m_compactLayoutApplied)
        applyResponsiveLayout();
}

void pa22x::applyWaveViewStyle()
{
    if (!ui || !ui->PEAWaveView)
        return;

    ui->PEAWaveView->setStyleSheet(QStringLiteral(
        "PEImageView {"
        "border: 2px solid %1;"
        "border-radius: 14px;"
        "padding: 4px;"
        "background: %2;"
        "}").arg(m_lightThemeActive
                     ? QStringLiteral("#9cb8c2")
                     : QStringLiteral("#6e8d97"),
                 m_lightThemeActive
                     ? QStringLiteral("qlineargradient(x1:0, y1:0, x2:1, y2:1, stop:0 rgba(255,255,255,0.94), stop:1 rgba(236,242,245,0.96))")
                     : QStringLiteral("qlineargradient(x1:0, y1:0, x2:1, y2:1, stop:0 rgba(19,29,35,0.94), stop:1 rgba(15,23,28,0.94))")));
}

void pa22x::refreshWaveCaptureButtonState()
{
    if (!ui)
        return;

    ui->PESaveWave->setText(is_start ? QStringLiteral("停止采集")
                                     : QStringLiteral("启动采集"));
    ui->CloseWaveSave->setVisible(false);
    ui->CloseWaveSave->setEnabled(false);
}

void pa22x::normalizeLegacyParameterEditors()
{
    const QList<QWidget*> parameterEditors = {
        ui->spb_specfmin,
        ui->spb_specfmax,
        ui->spb_histime,
        ui->sb_avenum,
        ui->dspb_width,
        ui->spb_time,
        ui->PEChannelSelection,
        ui->PEDualModeSelection,
        ui->PEGainInput,
        ui->PEGainSetp,
        ui->PERangeInput,
        ui->PERangeStep,
        ui->PEDelayInput,
        ui->PEDelayStep,
        ui->PEPRFInput,
        ui->IPAddressInput
    };

    for (QWidget* widget : parameterEditors) {
        if (!widget)
            continue;

        widget->setStyleSheet(QString());
        QSizePolicy policy = widget->sizePolicy();
        policy.setHorizontalPolicy(QSizePolicy::Expanding);
        policy.setVerticalPolicy(QSizePolicy::Fixed);
        widget->setSizePolicy(policy);
        widget->setMinimumSize(0, 0);
        widget->setBaseSize(0, 0);
        widget->setMinimumWidth(0);
        widget->setMaximumWidth(QWIDGETSIZE_MAX);
        widget->setMaximumHeight(QWIDGETSIZE_MAX);
    }

    const QList<QAbstractSpinBox*> spinEditors = {
        ui->spb_specfmin,
        ui->spb_specfmax,
        ui->spb_histime,
        ui->sb_avenum,
        ui->dspb_width,
        ui->spb_time,
        ui->PEGainInput,
        ui->PERangeInput,
        ui->PEDelayInput,
        ui->PEPRFInput
    };
    for (QAbstractSpinBox* spinBox : spinEditors) {
        if (!spinBox)
            continue;

        spinBox->setButtonSymbols(QAbstractSpinBox::UpDownArrows);
        spinBox->setAccelerated(true);
        if (QLineEdit* editor = spinBox->findChild<QLineEdit*>()) {
            editor->setAlignment(Qt::AlignCenter);
            editor->setFrame(false);
            editor->setMinimumWidth(0);
            editor->setMaximumWidth(QWIDGETSIZE_MAX);
        }
    }

    auto normalizeCombo = [](QComboBox* comboBox) {
        if (!comboBox)
            return;
        comboBox->setEditable(false);
        comboBox->setInsertPolicy(QComboBox::NoInsert);
        comboBox->setMaxVisibleItems(24);
        comboBox->setMinimumContentsLength(0);
        if (QAbstractItemView* view = comboBox->view()) {
            view->setTextElideMode(Qt::ElideNone);
            view->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
        }
    };

    normalizeCombo(ui->PEChannelSelection);
    normalizeCombo(ui->PEDualModeSelection);
    normalizeCombo(ui->PEGainSetp);
    normalizeCombo(ui->PERangeStep);
    normalizeCombo(ui->PEDelayStep);

    if (ui->IPAddressInput) {
        ui->IPAddressInput->setStyleSheet(QString());
        ui->IPAddressInput->setClearButtonEnabled(true);
        ui->IPAddressInput->setMinimumWidth(0);
        ui->IPAddressInput->setMaximumWidth(QWIDGETSIZE_MAX);
        QSizePolicy policy = ui->IPAddressInput->sizePolicy();
        policy.setHorizontalPolicy(QSizePolicy::Expanding);
        policy.setVerticalPolicy(QSizePolicy::Fixed);
        ui->IPAddressInput->setSizePolicy(policy);
    }
}

void pa22x::alignParameterGroupRows()
{
    const qreal scale = m_uiScaleFactor > 0.0 ? m_uiScaleFactor : 1.0;
    const int rowSpacing = scaledPixels(10, scale, 6, 18);
    const int rowMargin = scaledPixels(2, scale, 0, 6);
    const int compactSpacerWidth = scaledPixels(4, scale, 2, 8);

    for (QHBoxLayout* layout : {
             ui->horizontalLayout_6,
             ui->horizontalLayout_10,
             ui->horizontalLayout_7,
             ui->horizontalLayout_8,
             ui->horizontalLayout_9 }) {
        if (!layout)
            continue;

        layout->setContentsMargins(0, rowMargin, 0, rowMargin);
        layout->setSpacing(rowSpacing);
        layout->setStretch(0, 0);
        if (layout->count() > 1) {
            if (QSpacerItem* spacer = layout->itemAt(1)->spacerItem())
                spacer->changeSize(compactSpacerWidth, 0, QSizePolicy::Fixed, QSizePolicy::Minimum);
        }
        if (layout->count() > 2)
            layout->setStretch(2, 4);
        if (layout->count() > 3) {
            if (QSpacerItem* spacer = layout->itemAt(3)->spacerItem())
                spacer->changeSize(compactSpacerWidth, 0, QSizePolicy::Fixed, QSizePolicy::Minimum);
        }
        if (layout->count() > 4)
            layout->setStretch(4, 8);
    }

    for (QHBoxLayout* layout : { ui->horizontalLayout_8, ui->horizontalLayout_9 }) {
        if (!layout)
            continue;

        if (layout->count() > 2)
            layout->setStretch(2, 3);
        if (layout->count() > 4)
            layout->setStretch(4, 12);
    }
}

bool pa22x::suspendDisplayRefresh()
{
    if (_displayTimer <= 0)
        return false;

    killTimer(_displayTimer);
    _displayTimer = 0;
    return true;
}

void pa22x::resumeDisplayRefresh(bool wasRunning)
{
    if (wasRunning && _displayTimer <= 0)
        _displayTimer = startTimer(kDisplayRefreshIntervalMs);
}

void pa22x::pumpUiEvents(int maxMs) const
{
    QCoreApplication::processEvents(QEventLoop::ExcludeUserInputEvents, qMax(1, maxMs));
}

void pa22x::applyCompactLayoutIfNeeded()
{
    initializeResponsiveLayout();
    applyResponsiveLayout();
    return;

    if (m_compactLayoutApplied || !shouldUseCompactLayout(this))
        return;

    m_compactLayoutApplied = true;

    const qreal fontScale = 0.64;
    const qreal sizeScale = 0.56;

    QScreen* screen = nullptr;
    if (windowHandle())
        screen = windowHandle()->screen();
    if (!screen)
        screen = QGuiApplication::screenAt(frameGeometry().center());
    if (screen) {
        const QSize compactSize = screen->availableGeometry().size().boundedTo(QSize(1024, 768));
        resize(compactSize);
    }
    setMinimumSize(860, 640);

    if (QWidget* root = centralWidget()) {
        if (QLayout* layout = root->layout()) {
            layout->setContentsMargins(8, 6, 8, 8);
            layout->setSpacing(6);
        }
    }

    if (QGridLayout* rootGrid = qobject_cast<QGridLayout*>(centralWidget()->layout())) {
        if (QLayoutItem* spacerLeft = rootGrid->itemAtPosition(1, 0)) {
            if (QSpacerItem* spacer = spacerLeft->spacerItem())
                spacer->changeSize(10, 6, QSizePolicy::Minimum, QSizePolicy::Fixed);
        }
        if (QLayoutItem* spacerRight = rootGrid->itemAtPosition(1, 1)) {
            if (QSpacerItem* spacer = spacerRight->spacerItem())
                spacer->changeSize(10, 6, QSizePolicy::Minimum, QSizePolicy::Fixed);
        }

        if (ui->verticalLayout_3->count() > 1) {
            delete ui->verticalLayout_3->takeAt(ui->verticalLayout_3->count() - 1);
        }

        rootGrid->removeItem(ui->verticalLayout_3);
        rootGrid->removeItem(ui->verticalLayout_3);
        rootGrid->removeItem(ui->horizontalLayout);
        rootGrid->addLayout(ui->verticalLayout_3, 2, 0, 1, 2);
        rootGrid->addLayout(ui->verticalLayout_3, 3, 0, 1, 2);
        rootGrid->addLayout(ui->horizontalLayout, 4, 0, 1, 2);

        rootGrid->setHorizontalSpacing(6);
        rootGrid->setVerticalSpacing(6);
        rootGrid->setRowStretch(2, 1);
        rootGrid->setRowStretch(3, 0);
        rootGrid->setRowStretch(4, 0);
    }

    const auto widgets = centralWidget()->findChildren<QWidget*>();
    for (QWidget* widget : widgets) {
        if (qobject_cast<QLabel*>(widget) ||
            qobject_cast<QAbstractButton*>(widget) ||
            qobject_cast<QLineEdit*>(widget) ||
            qobject_cast<QComboBox*>(widget) ||
            widget->inherits("QAbstractSpinBox") ||
            qobject_cast<QGroupBox*>(widget)) {
            scaleWidgetFont(widget, fontScale);
        }

        const QSize minimum = widget->minimumSize();
        const QSize maximum = widget->maximumSize();
        const bool hasFixedWidth = minimum.width() > 0 && minimum.width() == maximum.width();
        const bool hasFixedHeight = minimum.height() > 0 && minimum.height() == maximum.height();
        if (hasFixedWidth || hasFixedHeight)
            scaleFixedWidgetSize(widget, sizeScale);
    }

    ui->widget_21->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    ui->widget_21->setMinimumHeight(72);
    ui->widget_21->setMaximumHeight(96);
    makeWidgetWidthFlexible(ui->widget_21);

    ui->label_7->setAlignment(Qt::AlignCenter);
    ui->label_7->setMinimumHeight(44);
    ui->label_7->setMaximumHeight(72);
    makeWidgetWidthFlexible(ui->label_7);
    scaleWidgetFont(ui->label_7, 0.52, 18);
    ensureTextHeight(ui->label_7, 18, 44);
    ensureTextFits(ui->label_7, ui->label_7->text(), 28);

    ui->PEAWaveBackgroundView->setMinimumSize(0, 0);
    ui->PEAWaveBackgroundView->setMaximumSize(QWIDGETSIZE_MAX, QWIDGETSIZE_MAX);
    ui->PEAWaveBackgroundView->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    ui->PERulerVerView->setMinimumSize(18, 150);
    ui->PERulerVerView->setMaximumSize(20, QWIDGETSIZE_MAX);
    ui->PERulerVerView->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Expanding);
    ui->PEAWaveView->setMinimumSize(256, 150);
    ui->PEAWaveView->setMaximumSize(QWIDGETSIZE_MAX, QWIDGETSIZE_MAX);
    ui->PEAWaveView->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    ui->PERulerHorView->setMinimumSize(256, 18);
    ui->PERulerHorView->setMaximumSize(QWIDGETSIZE_MAX, 20);
    ui->PERulerHorView->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    makeWidgetFullyFlexible(ui->customPlot4, QSize(320, 150));
    makeWidgetFullyFlexible(ui->customPlot3, QSize(240, 150));

    ui->widget_28->setMinimumSize(136, 150);
    ui->widget_28->setMaximumSize(QWIDGETSIZE_MAX, QWIDGETSIZE_MAX);
    ui->widget_28->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);
    ui->widget_30->setMinimumSize(236, 150);
    ui->widget_30->setMaximumSize(QWIDGETSIZE_MAX, QWIDGETSIZE_MAX);
    ui->widget_30->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);

    ui->groupBox_5->layout()->setContentsMargins(4, 4, 4, 4);
    ui->groupBox_6->layout()->setContentsMargins(4, 4, 4, 4);
    ui->groupBox->layout()->setContentsMargins(4, 4, 4, 4);
    ui->groupBox_4->layout()->setContentsMargins(4, 4, 4, 4);

    ui->horizontalLayout_5->setSpacing(6);
    ui->horizontalLayout_16->setSpacing(4);
    ui->horizontalLayout_18->setSpacing(4);
    ui->horizontalLayout_19->setSpacing(6);
    ui->horizontalLayout->setSpacing(4);

    for (QWidget* widget : {
             static_cast<QWidget*>(ui->connectButton),
             static_cast<QWidget*>(ui->simulateDetectButton),
             static_cast<QWidget*>(ui->PESaveWave),
             static_cast<QWidget*>(ui->SpecAnaly),
             static_cast<QWidget*>(ui->CloseWaveSave),
             static_cast<QWidget*>(ui->Inserve),
             static_cast<QWidget*>(ui->WaSaveWave),
             static_cast<QWidget*>(ui->PESaveParameters),
             static_cast<QWidget*>(ui->PELoadParameters),
             static_cast<QWidget*>(ui->STSaveWave),
             static_cast<QWidget*>(ui->HisData),
             static_cast<QWidget*>(ui->pushButton_mode)}) {
        scaleWidgetFont(widget, 0.64, 10);
        scaleFixedWidgetSize(widget, 0.54, 76, 28);
    }

    for (QWidget* widget : {
             static_cast<QWidget*>(ui->deviceSelection),
             static_cast<QWidget*>(ui->lineEdit_Desk),
             static_cast<QWidget*>(ui->PEChannelCountSelection),
             static_cast<QWidget*>(ui->spb_specfmin),
             static_cast<QWidget*>(ui->spb_specfmax),
             static_cast<QWidget*>(ui->spb_histime),
             static_cast<QWidget*>(ui->sb_avenum),
             static_cast<QWidget*>(ui->dspb_width),
             static_cast<QWidget*>(ui->spb_time),
             static_cast<QWidget*>(ui->PEChannelSelection),
             static_cast<QWidget*>(ui->PEDualModeSelection),
             static_cast<QWidget*>(ui->PEGainInput),
             static_cast<QWidget*>(ui->PEGainSetp),
             static_cast<QWidget*>(ui->PERangeInput),
             static_cast<QWidget*>(ui->PERangeStep),
             static_cast<QWidget*>(ui->PEDelayInput),
             static_cast<QWidget*>(ui->PEDelayStep),
             static_cast<QWidget*>(ui->IPAddressInput)}) {
        scaleWidgetFont(widget, 0.64, 9);
        scaleFixedWidgetSize(widget, 0.54, 56, 26);
    }

    for (QWidget* widget : {
             static_cast<QWidget*>(ui->label_20),
             static_cast<QWidget*>(ui->label_23),
             static_cast<QWidget*>(ui->label_24),
             static_cast<QWidget*>(ui->label_25),
             static_cast<QWidget*>(ui->label_26),
             static_cast<QWidget*>(ui->label_27),
             static_cast<QWidget*>(ui->label_Ad50),
             static_cast<QWidget*>(ui->label_Ad90),
             static_cast<QWidget*>(ui->label_Pass),
             static_cast<QWidget*>(ui->label_21),
             static_cast<QWidget*>(ui->label_22),
             static_cast<QWidget*>(ui->PEChanneSelectionLabel),
             static_cast<QWidget*>(ui->PEDualModeLabel),
             static_cast<QWidget*>(ui->PEGainLabel),
             static_cast<QWidget*>(ui->PERangeLabel),
             static_cast<QWidget*>(ui->PEDelayLabel),
             static_cast<QWidget*>(ui->IPAddressLabel)}) {
        scaleWidgetFont(widget, 0.62, 9);
        scaleFixedWidgetSize(widget, 0.52, 64, 24);
    }

    for (QLabel* label : {
             ui->label_20,
             ui->label_23,
             ui->label_21,
             ui->label_22 }) {
        prepareFieldLabel(label, 24, 18);
    }

    for (QWidget* widget : {
             static_cast<QWidget*>(ui->label_d50),
             static_cast<QWidget*>(ui->label_d90),
             static_cast<QWidget*>(ui->label_pass2),
             static_cast<QWidget*>(ui->label_aveD90),
             static_cast<QWidget*>(ui->label_avepass)}) {
        scaleWidgetFont(widget, 0.62, 10);
        makeWidgetWidthFlexible(widget);
        widget->setMinimumHeight(34);
        widget->setMaximumHeight(40);
        widget->setMinimumWidth(96);
    }

    makeWidgetWidthFlexible(ui->lineEdit_Desk);
    ui->lineEdit_Desk->setMinimumWidth(220);
    makeWidgetWidthFlexible(ui->IPAddressInput);
    ui->IPAddressInput->setMinimumWidth(180);

    prepareSpinBoxField(ui->spb_specfmin, QStringLiteral("88.88"));
    prepareSpinBoxField(ui->spb_specfmax, QStringLiteral("88.88"));
    ui->spb_specfmin->setMinimumWidth(qMax(ui->spb_specfmin->minimumWidth(), 92));
    ui->spb_specfmax->setMinimumWidth(qMax(ui->spb_specfmax->minimumWidth(), 92));

    for (QWidget* widget : {
             static_cast<QWidget*>(ui->deviceSelection),
             static_cast<QWidget*>(ui->PEChannelCountSelection),
             static_cast<QWidget*>(ui->spb_specfmin),
             static_cast<QWidget*>(ui->spb_specfmax),
             static_cast<QWidget*>(ui->spb_histime),
             static_cast<QWidget*>(ui->sb_avenum),
             static_cast<QWidget*>(ui->dspb_width),
             static_cast<QWidget*>(ui->spb_time)}) {
        makeWidgetWidthFlexible(widget);
    }

    QWidget* compactTopBar = new QWidget(ui->PEAWaveBackgroundView);
    compactTopBar->setObjectName(QStringLiteral("compactTopBar"));
    QGridLayout* topGrid = new QGridLayout(compactTopBar);
    topGrid->setContentsMargins(0, 0, 0, 0);
    topGrid->setHorizontalSpacing(6);
    topGrid->setVerticalSpacing(4);
    topGrid->setColumnStretch(2, 1);

    ui->horizontalLayout_16->removeWidget(ui->deviceSelection);
    ui->horizontalLayout_16->removeWidget(ui->label_19);
    ui->horizontalLayout_16->removeWidget(ui->lineEdit_Desk);
    ui->horizontalLayout_16->removeWidget(ui->PEChannelCountLabel);
    ui->horizontalLayout_16->removeWidget(ui->PEChannelCountSelection);
    clearLayoutItems(ui->horizontalLayout_16);
    topGrid->addWidget(ui->deviceSelection, 0, 0);
    topGrid->addWidget(ui->label_19, 0, 1);
    topGrid->addWidget(ui->lineEdit_Desk, 0, 2);
    topGrid->addWidget(ui->PEChannelCountLabel, 1, 1);
    topGrid->addWidget(ui->PEChannelCountSelection, 1, 2, Qt::AlignLeft);
    ui->horizontalLayout_16->addWidget(compactTopBar);

    QWidget* compactDetectBar = new QWidget(ui->groupBox_6);
    compactDetectBar->setObjectName(QStringLiteral("compactDetectBar"));
    QGridLayout* detectGrid = new QGridLayout(compactDetectBar);
    detectGrid->setContentsMargins(0, 0, 0, 0);
    detectGrid->setHorizontalSpacing(6);
    detectGrid->setVerticalSpacing(4);
    detectGrid->setColumnStretch(1, 1);
    detectGrid->setColumnStretch(3, 1);

    ui->horizontalLayout_18->removeWidget(ui->label_20);
    ui->horizontalLayout_18->removeWidget(ui->spb_specfmin);
    ui->horizontalLayout_18->removeWidget(ui->label_23);
    ui->horizontalLayout_18->removeWidget(ui->spb_specfmax);
    ui->horizontalLayout_18->removeWidget(ui->label_24);
    ui->horizontalLayout_18->removeWidget(ui->spb_histime);
    ui->horizontalLayout_18->removeWidget(ui->label_25);
    ui->horizontalLayout_18->removeWidget(ui->sb_avenum);
    clearLayoutItems(ui->horizontalLayout_18);
    detectGrid->addWidget(ui->label_20, 0, 0);
    detectGrid->addWidget(ui->spb_specfmin, 0, 1);
    detectGrid->addWidget(ui->label_23, 0, 2);
    detectGrid->addWidget(ui->spb_specfmax, 0, 3);
    detectGrid->addWidget(ui->label_24, 1, 0);
    detectGrid->addWidget(ui->spb_histime, 1, 1);
    detectGrid->addWidget(ui->label_25, 1, 2);
    detectGrid->addWidget(ui->sb_avenum, 1, 3);
    detectGrid->setColumnMinimumWidth(1, ui->spb_specfmin->minimumWidth());
    detectGrid->setColumnMinimumWidth(3, ui->spb_specfmax->minimumWidth());
    ui->horizontalLayout_18->addWidget(compactDetectBar);

    QTabWidget* compactTabs = new QTabWidget(ui->centralwidget);
    compactTabs->setObjectName(QStringLiteral("compactTabs"));
    compactTabs->setDocumentMode(true);
    compactTabs->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    compactTabs->setMinimumHeight(216);

    ui->verticalLayout_3->removeWidget(ui->groupBox);
    ui->verticalLayout_3->removeWidget(ui->groupBox_4);
    clearLayoutItems(ui->verticalLayout_3);
    ui->groupBox->setTitle(QString());
    ui->groupBox_4->setTitle(QString());
    compactTabs->addTab(ui->groupBox, QStringLiteral("参数设置"));
    compactTabs->addTab(ui->groupBox_4, QStringLiteral("操作结果"));
    ui->verticalLayout_3->addWidget(compactTabs);

    QWidget* compactActionBar = new QWidget(ui->centralwidget);
    compactActionBar->setObjectName(QStringLiteral("compactActionBar"));
    QGridLayout* actionGrid = new QGridLayout(compactActionBar);
    actionGrid->setContentsMargins(0, 0, 0, 0);
    actionGrid->setHorizontalSpacing(6);
    actionGrid->setVerticalSpacing(4);
    actionGrid->setColumnStretch(1, 1);

    ui->horizontalLayout->removeWidget(ui->IPAddressLabel);
    ui->horizontalLayout->removeWidget(ui->IPAddressInput);
    ui->horizontalLayout->removeWidget(ui->PEPRFInput);
    ui->horizontalLayout->removeWidget(ui->connectButton);
    ui->horizontalLayout->removeWidget(ui->simulateDetectButton);
    ui->horizontalLayout->removeWidget(ui->PESaveWave);
    ui->horizontalLayout->removeWidget(ui->SpecAnaly);
    ui->horizontalLayout->removeWidget(ui->CloseWaveSave);
    ui->horizontalLayout->removeWidget(ui->Inserve);
    clearLayoutItems(ui->horizontalLayout);
    actionGrid->addWidget(ui->IPAddressLabel, 0, 0);
    actionGrid->addWidget(ui->IPAddressInput, 0, 1);
    actionGrid->addWidget(ui->connectButton, 0, 2);
    actionGrid->addWidget(ui->PESaveWave, 0, 3);
    actionGrid->addWidget(ui->SpecAnaly, 1, 0);
    actionGrid->addWidget(ui->CloseWaveSave, 1, 1);
    actionGrid->addWidget(ui->Inserve, 1, 2);
    actionGrid->addWidget(ui->simulateDetectButton, 1, 3);
    ui->horizontalLayout->addWidget(compactActionBar);

    ui->gridLayout_11->setHorizontalSpacing(6);
    ui->gridLayout_11->setVerticalSpacing(4);
    ui->gridLayout_11->setColumnStretch(1, 1);
    ui->horizontalLayout_4->setSpacing(4);
    ui->horizontalLayout_4->setStretch(1, 1);
    ui->horizontalLayout_4->setStretch(3, 1);

    for (QLabel* label : {
             ui->label_d50,
             ui->label_d90,
             ui->label_pass2,
             ui->label_aveD90,
             ui->label_avepass }) {
        prepareMetricLabel(label, QStringLiteral("100.00"));
    }

    ui->centralwidget->setMinimumSize(860, 700);
    ui->centralwidget->layout()->invalidate();

    if (ui->centralwidget->parentWidget() == this) {
        QWidget* content = takeCentralWidget();
        QScrollArea* scrollArea = new QScrollArea(this);
        scrollArea->setFrameShape(QFrame::NoFrame);
        scrollArea->setWidgetResizable(true);
        scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
        scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
        scrollArea->setWidget(content);
        setCentralWidget(scrollArea);
    }
}

void pa22x::initializeResponsiveLayout()
{
    if (m_compactLayoutApplied)
        return;

    m_compactLayoutApplied = true;

    if (QWidget* root = ui->centralwidget) {
        if (QLayout* layout = root->layout()) {
            layout->setContentsMargins(10, 8, 10, 10);
            layout->setSpacing(8);
        }
    }

    if (QGridLayout* rootGrid = qobject_cast<QGridLayout*>(ui->centralwidget->layout())) {
        if (QLayoutItem* spacerLeft = rootGrid->itemAtPosition(1, 0)) {
            if (QSpacerItem* spacer = spacerLeft->spacerItem())
                spacer->changeSize(10, 6, QSizePolicy::Minimum, QSizePolicy::Fixed);
        }
        if (QLayoutItem* spacerRight = rootGrid->itemAtPosition(1, 1)) {
            if (QSpacerItem* spacer = spacerRight->spacerItem())
                spacer->changeSize(10, 6, QSizePolicy::Minimum, QSizePolicy::Fixed);
        }

        if (ui->verticalLayout_3->count() > 1)
            delete ui->verticalLayout_3->takeAt(ui->verticalLayout_3->count() - 1);

        rootGrid->removeItem(ui->verticalLayout_3);
        rootGrid->removeItem(ui->verticalLayout_3);
        rootGrid->removeItem(ui->horizontalLayout);
        rootGrid->addLayout(ui->verticalLayout_3, 2, 0, 1, 2);
        rootGrid->addLayout(ui->verticalLayout_3, 3, 0, 1, 2);
        rootGrid->addLayout(ui->horizontalLayout, 4, 0, 1, 2);
    }

    ui->widget_21->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    makeWidgetWidthFlexible(ui->widget_21);
    ui->label_7->setAlignment(Qt::AlignCenter);
    makeWidgetWidthFlexible(ui->label_7);

    ui->PEAWaveBackgroundView->setMinimumSize(0, 0);
    ui->PEAWaveBackgroundView->setMaximumSize(QWIDGETSIZE_MAX, QWIDGETSIZE_MAX);
    ui->PEAWaveBackgroundView->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    ui->PERulerVerView->setMaximumSize(QWIDGETSIZE_MAX, QWIDGETSIZE_MAX);
    ui->PERulerVerView->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Expanding);
    ui->PEAWaveView->setMaximumSize(QWIDGETSIZE_MAX, QWIDGETSIZE_MAX);
    ui->PEAWaveView->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    ui->PERulerHorView->setMaximumSize(QWIDGETSIZE_MAX, QWIDGETSIZE_MAX);
    ui->PERulerHorView->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    makeWidgetFullyFlexible(ui->customPlot4);
    makeWidgetFullyFlexible(ui->customPlot3);

    ui->widget_28->setMaximumSize(QWIDGETSIZE_MAX, QWIDGETSIZE_MAX);
    ui->widget_28->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);
    ui->widget_30->setMaximumSize(QWIDGETSIZE_MAX, QWIDGETSIZE_MAX);
    ui->widget_30->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);

    makeWidgetWidthFlexible(ui->lineEdit_Desk);
    makeWidgetWidthFlexible(ui->IPAddressInput);
    for (QWidget* widget : {
             static_cast<QWidget*>(ui->deviceSelection),
             static_cast<QWidget*>(ui->PEChannelCountSelection),
             static_cast<QWidget*>(ui->spb_specfmin),
             static_cast<QWidget*>(ui->spb_specfmax),
             static_cast<QWidget*>(ui->spb_histime),
             static_cast<QWidget*>(ui->sb_avenum),
             static_cast<QWidget*>(ui->dspb_width),
             static_cast<QWidget*>(ui->spb_time) }) {
        makeWidgetWidthFlexible(widget);
    }

    QWidget* compactTopBar = new QWidget(ui->PEAWaveBackgroundView);
    compactTopBar->setObjectName(QStringLiteral("compactTopBar"));
    QGridLayout* topGrid = new QGridLayout(compactTopBar);
    topGrid->setContentsMargins(0, 0, 0, 0);
    topGrid->setColumnStretch(2, 1);

    ui->horizontalLayout_16->removeWidget(ui->deviceSelection);
    ui->horizontalLayout_16->removeWidget(ui->label_19);
    ui->horizontalLayout_16->removeWidget(ui->lineEdit_Desk);
    ui->horizontalLayout_16->removeWidget(ui->PEChannelCountLabel);
    ui->horizontalLayout_16->removeWidget(ui->PEChannelCountSelection);
    clearLayoutItems(ui->horizontalLayout_16);
    topGrid->addWidget(ui->deviceSelection, 0, 0);
    topGrid->addWidget(ui->label_19, 0, 1);
    topGrid->addWidget(ui->lineEdit_Desk, 0, 2);
    topGrid->addWidget(ui->PEChannelCountLabel, 1, 1);
    topGrid->addWidget(ui->PEChannelCountSelection, 1, 2, Qt::AlignLeft);
    ui->horizontalLayout_16->addWidget(compactTopBar);

    QWidget* compactDetectBar = new QWidget(ui->groupBox_6);
    compactDetectBar->setObjectName(QStringLiteral("compactDetectBar"));
    QGridLayout* detectGrid = new QGridLayout(compactDetectBar);
    detectGrid->setContentsMargins(0, 0, 0, 0);
    detectGrid->setColumnStretch(1, 1);
    detectGrid->setColumnStretch(3, 1);

    ui->horizontalLayout_18->removeWidget(ui->label_20);
    ui->horizontalLayout_18->removeWidget(ui->spb_specfmin);
    ui->horizontalLayout_18->removeWidget(ui->label_23);
    ui->horizontalLayout_18->removeWidget(ui->spb_specfmax);
    ui->horizontalLayout_18->removeWidget(ui->label_24);
    ui->horizontalLayout_18->removeWidget(ui->spb_histime);
    ui->horizontalLayout_18->removeWidget(ui->label_25);
    ui->horizontalLayout_18->removeWidget(ui->sb_avenum);
    clearLayoutItems(ui->horizontalLayout_18);
    detectGrid->addWidget(ui->label_20, 0, 0);
    detectGrid->addWidget(ui->spb_specfmin, 0, 1);
    detectGrid->addWidget(ui->label_23, 0, 2);
    detectGrid->addWidget(ui->spb_specfmax, 0, 3);
    detectGrid->addWidget(ui->label_24, 1, 0);
    detectGrid->addWidget(ui->spb_histime, 1, 1);
    detectGrid->addWidget(ui->label_25, 1, 2);
    detectGrid->addWidget(ui->sb_avenum, 1, 3);
    ui->horizontalLayout_18->addWidget(compactDetectBar);

    QTabWidget* compactTabs = new QTabWidget(ui->centralwidget);
    compactTabs->setObjectName(QStringLiteral("compactTabs"));
    compactTabs->setDocumentMode(true);
    compactTabs->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    ui->verticalLayout_3->removeWidget(ui->groupBox);
    ui->verticalLayout_3->removeWidget(ui->groupBox_4);
    clearLayoutItems(ui->verticalLayout_3);
    ui->groupBox->setTitle(QString());
    ui->groupBox_4->setTitle(QString());
    compactTabs->addTab(ui->groupBox, QStringLiteral("参数设置"));
    compactTabs->addTab(ui->groupBox_4, QStringLiteral("操作结果"));
    ui->verticalLayout_3->addWidget(compactTabs);

    QWidget* compactActionBar = new QWidget(ui->centralwidget);
    compactActionBar->setObjectName(QStringLiteral("compactActionBar"));
    QGridLayout* actionGrid = new QGridLayout(compactActionBar);
    actionGrid->setContentsMargins(0, 0, 0, 0);
    actionGrid->setColumnStretch(1, 1);

    ui->horizontalLayout->removeWidget(ui->IPAddressLabel);
    ui->horizontalLayout->removeWidget(ui->IPAddressInput);
    ui->horizontalLayout->removeWidget(ui->PEPRFInput);
    ui->horizontalLayout->removeWidget(ui->connectButton);
    ui->horizontalLayout->removeWidget(ui->simulateDetectButton);
    ui->horizontalLayout->removeWidget(ui->PESaveWave);
    ui->horizontalLayout->removeWidget(ui->SpecAnaly);
    ui->horizontalLayout->removeWidget(ui->CloseWaveSave);
    ui->horizontalLayout->removeWidget(ui->Inserve);
    clearLayoutItems(ui->horizontalLayout);
    actionGrid->addWidget(ui->IPAddressLabel, 0, 0);
    actionGrid->addWidget(ui->IPAddressInput, 0, 1);
    actionGrid->addWidget(ui->connectButton, 0, 2);
    actionGrid->addWidget(ui->PESaveWave, 0, 3);
    actionGrid->addWidget(ui->SpecAnaly, 1, 0);
    actionGrid->addWidget(ui->CloseWaveSave, 1, 1);
    actionGrid->addWidget(ui->Inserve, 1, 2);
    actionGrid->addWidget(ui->simulateDetectButton, 1, 3);
    ui->horizontalLayout->addWidget(compactActionBar);

    if (ui->centralwidget->parentWidget() == this) {
        QWidget* content = takeCentralWidget();
        QScrollArea* scrollArea = new QScrollArea(this);
        scrollArea->setFrameShape(QFrame::NoFrame);
        scrollArea->setWidgetResizable(true);
        scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
        scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
        scrollArea->setWidget(content);
        setCentralWidget(scrollArea);
    }
}

QSize pa22x::currentViewportSize() const
{
    if (QScrollArea* scrollArea = qobject_cast<QScrollArea*>(centralWidget())) {
        const QSize viewportSize = scrollArea->viewport()->size();
        if (viewportSize.isValid())
            return viewportSize;
    }

    if (QWidget* root = centralWidget()) {
        const QSize rootSize = root->size();
        if (rootSize.isValid())
            return rootSize;
    }

    if (QScreen* screen = screenForWidget(this))
        return screen->geometry().size();

    return size();
}

void pa22x::applyResponsiveLayout()
{
    if (!m_compactLayoutApplied || m_responsiveLayoutApplying)
        return;

    m_responsiveLayoutApplying = true;
    const QSize viewportSize = currentViewportSize();
    const qreal scale = computeResponsiveUiScale(viewportSize);
    m_uiScaleFactor = scale;

    const int outerMargin = scaledPixels(16, scale, 8, 28);
    const int innerSpacing = scaledPixels(10, scale, 4, 18);
    const int compactSpacing = scaledPixels(8, scale, 4, 14);

    if (QLayout* layout = ui->centralwidget->layout()) {
        layout->setContentsMargins(outerMargin, qMax(6, outerMargin - 2), outerMargin, outerMargin);
        layout->setSpacing(innerSpacing);
    }

    if (QGridLayout* rootGrid = qobject_cast<QGridLayout*>(ui->centralwidget->layout())) {
        rootGrid->setHorizontalSpacing(innerSpacing);
        rootGrid->setVerticalSpacing(innerSpacing);
        if (viewportSize.width() <= 1180) {
            rootGrid->setColumnStretch(0, 7);
            rootGrid->setColumnStretch(1, 4);
        } else if (viewportSize.width() >= 1500) {
            rootGrid->setColumnStretch(0, 6);
            rootGrid->setColumnStretch(1, 3);
        } else {
            rootGrid->setColumnStretch(0, 6);
            rootGrid->setColumnStretch(1, 4);
        }
    }

    ui->widget_21->setMinimumHeight(scaledPixels(104, scale, 84, 152));
    ui->widget_21->setMaximumHeight(scaledPixels(136, scale, 96, 190));

    if (QGridLayout* headerLayout = qobject_cast<QGridLayout*>(ui->widget_21->layout())) {
        headerLayout->setContentsMargins(scaledPixels(50, scale, 30, 82),
                                         scaledPixels(10, scale, 6, 18),
                                         scaledPixels(24, scale, 12, 42),
                                         scaledPixels(12, scale, 8, 20));
        headerLayout->setHorizontalSpacing(scaledPixels(16, scale, 10, 28));
        headerLayout->setVerticalSpacing(scaledPixels(4, scale, 2, 8));
        headerLayout->setColumnStretch(0, 0);
        headerLayout->setColumnStretch(1, 1);
    }

    setWidgetPointSize(ui->label_7, qBound<qreal>(15.0, 17.0 * scale, 24.0), QFont::Bold);
    ui->label_7->setAlignment(Qt::AlignCenter);
    ensureTextHeight(ui->label_7,
                     scaledPixels(14, scale, 10, 24),
                     scaledPixels(52, scale, 40, 80));
    ui->label_7->setMaximumHeight(scaledPixels(76, scale, 58, 112));
    ensureTextFits(ui->label_7, ui->label_7->text(), scaledPixels(40, scale, 28, 64));

    if (ui->label_headerIcon) {
        const int iconHeight = qMax(ui->label_7->minimumHeight(), scaledPixels(58, scale, 44, 86));
        const int iconWidth = scaledPixels(430, scale, 300, 640);
        ui->label_headerIcon->setMinimumSize(iconWidth, iconHeight);
        ui->label_headerIcon->setMaximumSize(iconWidth, iconHeight);
        const QPixmap logo(QStringLiteral(":/images/tang.gif"));
        if (!logo.isNull()) {
            ui->label_headerIcon->setPixmap(logo.scaled(iconWidth - scaledPixels(12, scale, 8, 18),
                                                        iconHeight - scaledPixels(8, scale, 6, 14),
                                                        Qt::KeepAspectRatio,
                                                        Qt::SmoothTransformation));
        }
    }

    for (QGroupBox* box : { ui->groupBox_5, ui->groupBox_6, ui->groupBox, ui->groupBox_4 }) {
        setWidgetPointSize(box, qBound<qreal>(13.0, 14.4 * scale, 20.0), QFont::DemiBold);
        if (QLayout* layout = box->layout()) {
            layout->setContentsMargins(innerSpacing, innerSpacing, innerSpacing, innerSpacing);
            layout->setSpacing(compactSpacing);
        }
    }

    const QList<QAbstractButton*> buttons = {
        ui->connectButton,
        ui->simulateDetectButton,
        ui->PESaveWave,
        ui->SpecAnaly,
        ui->CloseWaveSave,
        ui->Inserve,
        ui->WaSaveWave,
        ui->PESaveParameters,
        ui->PELoadParameters,
        ui->STSaveWave,
        ui->HisData,
        ui->pushButton_mode
    };
    for (QAbstractButton* button : buttons) {
        setWidgetPointSize(button, qBound<qreal>(11.0, 12.5 * scale, 17.0), QFont::DemiBold);
        button->setMinimumHeight(scaledPixels(44, scale, 32, 60));
        button->setMaximumHeight(scaledPixels(56, scale, 38, 72));
        ensureTextFits(button, button->text(), scaledPixels(44, scale, 28, 66));
    }

    const QList<QWidget*> editors = {
        ui->deviceSelection,
        ui->lineEdit_Desk,
        ui->PEChannelCountSelection,
        ui->spb_specfmin,
        ui->spb_specfmax,
        ui->spb_histime,
        ui->sb_avenum,
        ui->dspb_width,
        ui->spb_time,
        ui->PEChannelSelection,
        ui->PEDualModeSelection,
        ui->PEGainInput,
        ui->PEGainSetp,
        ui->PERangeInput,
        ui->PERangeStep,
        ui->PEDelayInput,
        ui->PEDelayStep,
        ui->IPAddressInput
    };
    for (QWidget* widget : editors) {
        setWidgetPointSize(widget, qBound<qreal>(9.0, 10.3 * scale, 13.5), QFont::DemiBold);
        widget->setMinimumHeight(scaledPixels(38, scale, 30, 52));
        widget->setMaximumHeight(scaledPixels(46, scale, 34, 58));
    }

    ui->lineEdit_Desk->setMinimumWidth(scaledPixels(320, scale, 240, 560));
    setWidgetPointSize(ui->IPAddressInput, qBound<qreal>(8.5, 9.4 * scale, 12.0), QFont::DemiBold);
    ui->IPAddressInput->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    ui->IPAddressInput->setMinimumWidth(scaledPixels(460, scale, 340, 760));
    ui->IPAddressInput->setMaximumWidth(QWIDGETSIZE_MAX);

    const QList<QLabel*> fieldLabels = {
        ui->label_19,
        ui->label_20,
        ui->label_23,
        ui->label_24,
        ui->label_25,
        ui->label_26,
        ui->label_27,
        ui->label_Ad50,
        ui->label_Ad90,
        ui->label_Pass,
        ui->label_21,
        ui->label_22,
        ui->PEChannelCountLabel,
        ui->PEChanneSelectionLabel,
        ui->PEDualModeLabel,
        ui->PEGainLabel,
        ui->PERangeLabel,
        ui->PEDelayLabel,
        ui->IPAddressLabel
    };
    for (QLabel* label : fieldLabels) {
        setWidgetPointSize(label, qBound<qreal>(9.5, 10.6 * scale, 14.0), QFont::Medium);
        prepareFieldLabel(label,
                          scaledPixels(28, scale, 22, 42),
                          scaledPixels(18, scale, 12, 28));
    }

    const int parameterLabelWidth = scaledPixels(132, scale, 108, 188);
    for (QLabel* label : {
             ui->PEChanneSelectionLabel,
             ui->PEDualModeLabel,
             ui->PEGainLabel,
             ui->PERangeLabel,
             ui->PEDelayLabel,
             ui->IPAddressLabel }) {
        label->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
        label->setMinimumWidth(parameterLabelWidth);
        label->setMaximumWidth(parameterLabelWidth);
    }

    const int detectLabelWidth = scaledPixels(138, scale, 112, 188);
    for (QLabel* label : {
             ui->label_20,
             ui->label_23,
             ui->label_24,
             ui->label_25,
             ui->label_26,
             ui->label_27 }) {
        label->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
        label->setMinimumWidth(detectLabelWidth);
        label->setMaximumWidth(detectLabelWidth);
    }

    prepareSpinBoxField(ui->spb_specfmin, QStringLiteral("88.88"));
    prepareSpinBoxField(ui->spb_specfmax, QStringLiteral("88.88"));
    prepareSpinBoxField(ui->spb_histime, QStringLiteral("8888"));
    prepareSpinBoxField(ui->sb_avenum, QStringLiteral("8888"));
    prepareSpinBoxField(ui->dspb_width, QStringLiteral("8888.88"));
    prepareSpinBoxField(ui->spb_time, QStringLiteral("8888"));
    prepareSpinBoxField(ui->PEGainInput, QStringLiteral("8888.88"));
    prepareSpinBoxField(ui->PERangeInput, QStringLiteral("8888.88"));
    prepareSpinBoxField(ui->PEDelayInput, QStringLiteral("8888.88"));
    prepareSpinBoxField(ui->PEPRFInput, QStringLiteral("8888"));
    prepareComboBoxField(ui->deviceSelection, QStringLiteral("PE8Device_80"));
    prepareComboBoxField(ui->PEChannelCountSelection, QStringLiteral("88"));
    prepareComboBoxField(ui->PEChannelSelection, QStringLiteral("channel_88"));
    prepareComboBoxField(ui->PEDualModeSelection, QStringLiteral("reflection-single"));
    prepareComboBoxField(ui->PEGainSetp, QStringLiteral("8888.88"));
    prepareComboBoxField(ui->PERangeStep, QStringLiteral("8888.88"));
    prepareComboBoxField(ui->PEDelayStep, QStringLiteral("8888.88"));
    ui->spb_specfmin->setMinimumWidth(scaledPixels(164, scale, 132, 214));
    ui->spb_specfmax->setMinimumWidth(scaledPixels(164, scale, 132, 214));
    ui->spb_histime->setMinimumWidth(scaledPixels(152, scale, 120, 194));
    ui->sb_avenum->setMinimumWidth(scaledPixels(140, scale, 112, 184));
    ui->dspb_width->setMinimumWidth(scaledPixels(152, scale, 120, 194));
    ui->spb_time->setMinimumWidth(scaledPixels(152, scale, 120, 194));
    ui->deviceSelection->setMinimumWidth(scaledPixels(280, scale, 220, 420));
    ui->PEChannelCountSelection->setMinimumWidth(scaledPixels(144, scale, 110, 188));

    const int parameterEditorWidth = scaledPixels(322, scale, 246, 450);
    const int parameterStepWidth = scaledPixels(396, scale, 290, 580);
    const int wideParameterStepWidth = scaledPixels(560, scale, 420, 820);
    ui->PEChannelSelection->setMinimumWidth(parameterEditorWidth);
    ui->PEDualModeSelection->setMinimumWidth(parameterEditorWidth);
    ui->PEGainInput->setMinimumWidth(parameterEditorWidth);
    ui->PERangeInput->setMinimumWidth(parameterEditorWidth);
    ui->PEDelayInput->setMinimumWidth(parameterEditorWidth);
    ui->PEPRFInput->setMinimumWidth(scaledPixels(156, scale, 120, 210));
    ui->PEGainSetp->setMinimumWidth(parameterStepWidth);
    ui->PERangeStep->setMinimumWidth(wideParameterStepWidth);
    ui->PEDelayStep->setMinimumWidth(wideParameterStepWidth);
    setWidgetPointSize(ui->PEChannelSelection, qBound<qreal>(8.8, 9.4 * scale, 12.0), QFont::DemiBold);
    setWidgetPointSize(ui->PEDualModeSelection, qBound<qreal>(8.6, 9.2 * scale, 11.6), QFont::DemiBold);
    setWidgetPointSize(ui->PEGainSetp, qBound<qreal>(8.8, 9.4 * scale, 12.0), QFont::DemiBold);
    setWidgetPointSize(ui->PERangeStep, qBound<qreal>(8.2, 8.8 * scale, 11.2), QFont::DemiBold);
    setWidgetPointSize(ui->PEDelayStep, qBound<qreal>(8.2, 8.8 * scale, 11.2), QFont::DemiBold);
    updateComboBoxPresentation(ui->PEChannelSelection, QStringLiteral("channel_88"));
    updateComboBoxPresentation(ui->PEDualModeSelection, QStringLiteral("reflection-single"));
    updateComboBoxPresentation(ui->PEGainSetp, QStringLiteral("8888.88"));
    updateComboBoxPresentation(ui->PERangeStep, QStringLiteral("8888.88"));
    updateComboBoxPresentation(ui->PEDelayStep, QStringLiteral("8888.88"));
    normalizeLegacyParameterEditors();
    alignParameterGroupRows();

    for (QLabel* label : {
             ui->label_d50,
             ui->label_d90,
             ui->label_pass2,
             ui->label_aveD90,
             ui->label_avepass }) {
        setWidgetPointSize(label, qBound<qreal>(12.0, 14.2 * scale, 19.0), QFont::Bold);
        label->setMinimumHeight(scaledPixels(44, scale, 34, 68));
        label->setMaximumHeight(scaledPixels(62, scale, 40, 84));
        label->setMinimumWidth(scaledPixels(148, scale, 114, 220));
        prepareMetricLabel(label, QStringLiteral("100.00"));
    }

    ui->PERulerVerView->setMinimumSize(scaledPixels(18, scale, 14, 24),
                                       scaledPixels(198, scale, 144, 320));
    ui->PERulerVerView->setMaximumWidth(scaledPixels(24, scale, 18, 32));
    ui->PEAWaveView->setMinimumSize(scaledPixels(392, scale, 276, 640),
                                    scaledPixels(246, scale, 174, 380));
    ui->PERulerHorView->setMinimumSize(scaledPixels(392, scale, 276, 640),
                                       scaledPixels(20, scale, 16, 28));
    ui->PERulerHorView->setMaximumHeight(scaledPixels(24, scale, 18, 32));
    ui->customPlot4->setMinimumSize(scaledPixels(500, scale, 340, 820),
                                    scaledPixels(272, scale, 188, 420));
    ui->customPlot3->setMinimumSize(scaledPixels(336, scale, 238, 520),
                                    scaledPixels(224, scale, 164, 340));
    ui->widget_28->setMinimumSize(scaledPixels(208, scale, 164, 340),
                                  scaledPixels(224, scale, 164, 340));
    ui->widget_30->setMinimumSize(scaledPixels(328, scale, 238, 520),
                                  scaledPixels(224, scale, 164, 340));

    ui->horizontalLayout_5->setSpacing(innerSpacing);
    ui->horizontalLayout_5->setStretch(0, 8);
    ui->horizontalLayout_5->setStretch(1, 9);
    ui->horizontalLayout_16->setSpacing(compactSpacing);
    ui->horizontalLayout_18->setSpacing(compactSpacing);
    ui->horizontalLayout_19->setSpacing(innerSpacing);
    ui->horizontalLayout_19->setStretch(0, 6);
    ui->horizontalLayout_19->setStretch(1, 3);
    ui->horizontalLayout_19->setStretch(2, 4);
    ui->horizontalLayout->setSpacing(compactSpacing);
    ui->horizontalLayout_4->setSpacing(compactSpacing);
    ui->horizontalLayout_6->setSpacing(compactSpacing);
    ui->horizontalLayout_7->setSpacing(compactSpacing);
    ui->horizontalLayout_8->setSpacing(compactSpacing);
    ui->horizontalLayout_9->setSpacing(compactSpacing);
    ui->horizontalLayout_10->setSpacing(compactSpacing);

    ui->gridLayout_11->setHorizontalSpacing(compactSpacing);
    ui->gridLayout_11->setVerticalSpacing(compactSpacing);
    ui->gridLayout_11->setColumnStretch(0, 5);
    ui->gridLayout_11->setColumnStretch(1, 4);

    if (QWidget* compactTopBar = findChild<QWidget*>(QStringLiteral("compactTopBar"))) {
        if (QGridLayout* topGrid = qobject_cast<QGridLayout*>(compactTopBar->layout())) {
            topGrid->setHorizontalSpacing(compactSpacing);
            topGrid->setVerticalSpacing(compactSpacing - 1);
        }
    }
    if (QWidget* compactDetectBar = findChild<QWidget*>(QStringLiteral("compactDetectBar"))) {
        if (QGridLayout* detectGrid = qobject_cast<QGridLayout*>(compactDetectBar->layout())) {
            detectGrid->setHorizontalSpacing(compactSpacing);
            detectGrid->setVerticalSpacing(compactSpacing - 1);
            detectGrid->setColumnMinimumWidth(1, ui->spb_specfmin->minimumWidth());
            detectGrid->setColumnMinimumWidth(3, ui->spb_specfmax->minimumWidth());
        }
    }
    if (QWidget* compactActionBar = findChild<QWidget*>(QStringLiteral("compactActionBar"))) {
        if (QGridLayout* actionGrid = qobject_cast<QGridLayout*>(compactActionBar->layout())) {
            actionGrid->setHorizontalSpacing(compactSpacing);
            actionGrid->setVerticalSpacing(compactSpacing - 1);
            actionGrid->setColumnStretch(0, 1);
            actionGrid->setColumnStretch(1, 4);
            actionGrid->setColumnStretch(2, 1);
            actionGrid->setColumnStretch(3, 1);
            actionGrid->setColumnMinimumWidth(1, ui->IPAddressInput->minimumWidth());
        }
    }
    if (QTabWidget* compactTabs = findChild<QTabWidget*>(QStringLiteral("compactTabs"))) {
        compactTabs->setMinimumHeight(scaledPixels(294, scale, 220, 420));
        setWidgetPointSize(compactTabs, qBound<qreal>(10.5, 11.5 * scale, 15.0), QFont::DemiBold);
    }

    ui->centralwidget->setMinimumSize(scaledPixels(1000, scale, 860, 1680),
                                      scaledPixels(720, scale, 640, 1100));
    configurePlotTypography(ui->customPlot3, scale * 1.18);
    configurePlotTypography(ui->customPlot4, scale * 1.18);
    ui->centralwidget->layout()->invalidate();
    ui->centralwidget->updateGeometry();
    applyLockedPeChannelUi();
    m_responsiveLayoutApplying = false;
}

bool pa22x::hasValidBackgroundSignal(QString* errorMessage) const
{
    const QString appDir = QCoreApplication::applicationDirPath();
    QStringList checkedPaths;
    for (const QString& filePath : backgroundSignalFilePaths(appDir)) {
        checkedPaths << QDir::toNativeSeparators(filePath);
        QFile file(filePath);
        if (!file.exists())
            continue;

        if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            if (errorMessage)
                *errorMessage = QStringLiteral("背景信号文件无法读取：%1 | %2")
                        .arg(QDir::toNativeSeparators(filePath), file.errorString());
            return false;
        }

        bool hasNumericSample = false;
        while (!file.atEnd()) {
            bool ok = false;
            QString::fromUtf8(file.readLine()).trimmed().toDouble(&ok);
            if (ok) {
                hasNumericSample = true;
                break;
            }
        }

        if (hasNumericSample)
            return true;

        if (errorMessage)
            *errorMessage = QStringLiteral("背景信号文件为空，请重新采集背景信号：%1")
                    .arg(QDir::toNativeSeparators(filePath));
        return false;
    }

    if (errorMessage)
        *errorMessage = QStringLiteral("请先采集背景信号。当前检查路径：%1")
                .arg(checkedPaths.join(QStringLiteral(" ; ")));
    return false;
}

bool pa22x::attachClient(PA22XClient* client, bool simulationMode)
{
    if (!client)
        return false;

    if (_pa22xClient) {
        _pa22xClient->disconnectFromServer();
        _pa22xClient->deleteLater();
        _pa22xClient = nullptr;
    }
    _pa22xClient = client;
    m_isSimulationMode = simulationMode;
    m_displayedWaveSamples.clear();
    m_displayedWaveChannelIdentifier.clear();

    QVector<UTShadowDevice*>& shadowDevices = _pa22xClient->getShadowDevices();
    if (shadowDevices.isEmpty()) {
        qWarning() << "未发现可用设备";
        _pa22xClient->deleteLater();
        _pa22xClient = nullptr;
        m_isSimulationMode = false;
        return false;
    }

    ui->deviceSelection->clear();
    for (UTShadowDevice* shadowDevice : shadowDevices)
        ui->deviceSelection->addItem(shadowDevice->getIdentifier(), shadowDevice->getDeviceNumber().toInt());
    ui->deviceSelection->setCurrentIndex(0);
    updateComboBoxPresentation(ui->deviceSelection, QStringLiteral("PE8Device_80"));

    UTShadowDevice* selectedShadowDevice = _pa22xClient->getSelectedShadowDevice();
    if (!selectedShadowDevice)
        selectedShadowDevice = shadowDevices.first();

    ui->connectButton->setChecked(true);
    ui->connectButton->setText(QStringLiteral("断开连接"));

    for (int i = 0; i < ui->deviceSelection->count(); i ++) {
        if (ui->deviceSelection->itemText(i) == selectedShadowDevice->getIdentifier()) {
            ui->deviceSelection->setCurrentIndex(i);
            _pa22xClient->sendCommandSync(QStringLiteral("set data_start"));
            if (_displayTimer > 0)
                killTimer(_displayTimer);
            if (_evaluatorTimer > 0)
                killTimer(_evaluatorTimer);
            _displayTimer = startTimer(20);
            _evaluatorTimer = startTimer(1000);
            on_deviceSelection_activated(i);
            break;
        }
    }

    m_isconnect = true;
    return true;
}

void pa22x::refreshPEChannelSelectionUi(UTShadowDevice* selectedShadow)
{
    ui->PEChannelSelection->blockSignals(true);
    ui->PEChannelSelection->clear();
    ui->PEChannelSelection->setEnabled(true);

    if (!selectedShadow) {
        updateComboBoxPresentation(ui->PEChannelSelection, QStringLiteral("channel_88"));
        ui->PEChannelSelection->blockSignals(false);
        return;
    }

    QVector<QMap<QString, QString>>& parameters = selectedShadow->getParameters();
    const QString selectedChannel = selectedShadow->getSelectedChannel();
    if (selectedShadow->getDeviceType() == kDeviceTypePE) {
        const QString forcedChannel = forcedPeChannelIdentifier(selectedShadow);
        if (!forcedChannel.isEmpty()) {
            QString channelIdentifier = forcedChannel;
            selectedShadow->setSelectedChannel(channelIdentifier);
            const int forcedIndex = qMax(0, selectedShadow->identifierToIndex(channelIdentifier));
            ui->PEChannelSelection->addItem(channelIdentifier, QVariant(forcedIndex));
            ui->PEChannelSelection->setCurrentIndex(0);
            ui->PEChannelSelection->setEnabled(false);
            updateComboBoxPresentation(ui->PEChannelSelection, QStringLiteral("channel_88"));
            ui->PEChannelSelection->blockSignals(false);
            if (m_isconnect && ui->connectButton->text() != QStringLiteral("连接"))
                PEChannelActived(0);
            return;
        }
    }

    const int channelLimit = selectedShadow->getDeviceType() == kDeviceTypePE
            ? qMin(kFixedPEChannelCount, parameters.size())
            : parameters.size();
    int selectedIndex = -1;
    for (int i = 0; i < channelLimit; ++i) {
        const QString identifier = parameters.at(i).value(QStringLiteral("identifier"));
        if (identifier.isEmpty())
            continue;

        ui->PEChannelSelection->addItem(identifier, QVariant(i));
        if (identifier == selectedChannel)
            selectedIndex = ui->PEChannelSelection->count() - 1;
    }

    if (ui->PEChannelSelection->count() == 0) {
        updateComboBoxPresentation(ui->PEChannelSelection, QStringLiteral("channel_88"));
        ui->PEChannelSelection->blockSignals(false);
        return;
    }

    if (selectedIndex < 0)
        selectedIndex = 0;

    ui->PEChannelSelection->setCurrentIndex(selectedIndex);
    updateComboBoxPresentation(ui->PEChannelSelection, QStringLiteral("channel_88"));
    ui->PEChannelSelection->blockSignals(false);
    PEChannelActived(selectedIndex);
}

void pa22x::refreshPEChannelCountSelectionUi(UTShadowDevice* selectedShadow)
{
    ui->PEChannelCountSelection->blockSignals(true);
    ui->PEChannelCountSelection->clear();
    Q_UNUSED(selectedShadow);
    ui->PEChannelCountSelection->addItem(QString::number(kFixedPEChannelCount),
                                         QVariant(kFixedPEChannelCount));
    ui->PEChannelCountSelection->setCurrentIndex(0);
    ui->PEChannelCountSelection->setEnabled(false);
    updateComboBoxPresentation(ui->PEChannelCountSelection, QStringLiteral("88"));
    ui->PEChannelCountSelection->blockSignals(false);
}

bool pa22x::collectDisplayedWaveSamples(QVector<double>* samples,
                                        QString* channelIdentifier,
                                        QString* errorMessage) const
{
    QString liveWaveError;
    if (ui && ui->PEAWaveView) {
        QVector<double> liveSamples;
        if (ui->PEAWaveView->exportCurrentSamples(&liveSamples, &liveWaveError)) {
            if (samples)
                *samples = liveSamples;
            if (channelIdentifier) {
                QString liveChannelIdentifier = m_displayedWaveChannelIdentifier;
                if (liveChannelIdentifier.isEmpty() && _pa22xClient) {
                    UTShadowDevice* device = _pa22xClient->getSelectedShadowDevice();
                    if (device)
                        liveChannelIdentifier = device->getSelectedChannel();
                }
                *channelIdentifier = liveChannelIdentifier;
            }
            return true;
        }
    }

    if (m_displayedWaveSamples.size() != UTShadowDevice::_PEWaveLength) {
        if (errorMessage)
            *errorMessage = liveWaveError.isEmpty()
                    ? QStringLiteral("PEAWaveView 当前还没有可保存的有效波形。")
                    : liveWaveError;
        return false;
    }

    bool hasNonZeroSample = false;
    for (double sample : m_displayedWaveSamples) {
        if (!qFuzzyIsNull(sample)) {
            hasNonZeroSample = true;
            break;
        }
    }

    if (!hasNonZeroSample) {
        if (errorMessage)
            *errorMessage = QStringLiteral("PEAWaveView 当前显示的波形全为 0，不能保存为背景。");
        return false;
    }

    if (samples)
        *samples = m_displayedWaveSamples;
    if (channelIdentifier)
        *channelIdentifier = m_displayedWaveChannelIdentifier;
    return true;
}

bool pa22x::collectCurrentWaveSamples(QVector<double>* samples, QString* errorMessage) const
{
    if (!_pa22xClient) {
        if (errorMessage)
            *errorMessage = QStringLiteral("未连接设备。");
        return false;
    }

    UTShadowDevice* device = _pa22xClient->getSelectedShadowDevice();
    if (!device) {
        if (errorMessage)
            *errorMessage = QStringLiteral("未检测到可用设备。");
        return false;
    }

    QVector<QMap<QString, QString>>& parameters = device->getParameters();
    if (parameters.isEmpty()) {
        if (errorMessage)
            *errorMessage = QStringLiteral("当前设备没有可用通道。");
        return false;
    }

    QString channelIdentifier = device->getSelectedChannel();
    int channelIndex = device->identifierToIndex(channelIdentifier);
    if (channelIndex < 0 || channelIndex >= parameters.size())
        channelIndex = 0;

    QVector<double> localSamples;
    localSamples.reserve(UTShadowDevice::_PEWaveLength);
    bool hasNonZeroSample = false;

    device->lock();
    X22_Waveform* waveform = device->getAWaveData(channelIndex);
    if (waveform) {
        for (int i = 0; i < UTShadowDevice::_PEWaveLength; ++i) {
            const int sample = waveform->waveP[i] >= waveform->waveN[i]
                    ? waveform->waveP[i]
                    : -waveform->waveN[i];
            localSamples << sample;
            if (sample != 0)
                hasNonZeroSample = true;
        }
    }
    device->unlock();

    if (localSamples.size() != UTShadowDevice::_PEWaveLength || !hasNonZeroSample) {
        if (errorMessage)
            *errorMessage = QStringLiteral("当前通道还没有收到有效实时波形。");
        return false;
    }

    if (samples)
        *samples = localSamples;
    return true;
}

bool pa22x::collectDetectionWaveSamples(QVector<double>* samples,
                                        QString* sourceName,
                                        QString* errorMessage) const
{
    QVector<double> displayedSamples;
    QString displayedChannelIdentifier;
    QString displayedError;
    const bool displayedOk =
            collectDisplayedWaveSamples(&displayedSamples, &displayedChannelIdentifier, &displayedError);

    QVector<double> liveSamples;
    QString liveError;
    const bool liveOk = collectCurrentWaveSamples(&liveSamples, &liveError);

    const qint64 displayedAgeMs = m_lastDisplayedWaveCapturedAt.isValid()
            ? m_lastDisplayedWaveCapturedAt.msecsTo(QDateTime::currentDateTime())
            : std::numeric_limits<qint64>::max();
    const bool displayedFresh = displayedOk && displayedAgeMs >= 0 && displayedAgeMs <= 250;

    if (displayedFresh) {
        if (samples)
            *samples = displayedSamples;
        if (sourceName)
            *sourceName = QStringLiteral("PEAWaveView(%1ms)").arg(displayedAgeMs);
        return true;
    }

    if (liveOk) {
        if (samples)
            *samples = liveSamples;
        if (sourceName)
            *sourceName = displayedOk
                    ? QStringLiteral("实时通道(PEAWaveView 已过期 %1ms)")
                          .arg(displayedAgeMs == std::numeric_limits<qint64>::max() ? -1 : displayedAgeMs)
                    : QStringLiteral("实时通道");
        return true;
    }

    if (displayedOk) {
        if (samples)
            *samples = displayedSamples;
        if (sourceName)
            *sourceName = QStringLiteral("PEAWaveView(回退)");
        return true;
    }

    if (errorMessage) {
        QStringList errors;
        if (!displayedError.isEmpty())
            errors << QStringLiteral("界面波形：%1").arg(displayedError);
        if (!liveError.isEmpty())
            errors << QStringLiteral("实时通道：%1").arg(liveError);
        if (errors.isEmpty())
            errors << QStringLiteral("未获取到可用于检测的波形。");
        *errorMessage = errors.join(QStringLiteral(" | "));
    }
    return false;
}

bool pa22x::saveCurrentBackgroundSignal(QString* errorMessage)
{
    QVector<double> samples;
    QString channelIdentifier;
    QString waveError;
    if (!collectDisplayedWaveSamples(&samples, &channelIdentifier, &waveError)
            && !collectCurrentWaveSamples(&samples, &waveError)) {
        if (errorMessage)
            *errorMessage = waveError;
        return false;
    }

    UTShadowDevice* device = _pa22xClient ? _pa22xClient->getSelectedShadowDevice() : nullptr;
    if (channelIdentifier.isEmpty() && device)
        channelIdentifier = device->getSelectedChannel();
    if (channelIdentifier.isEmpty() && device && !device->getParameters().isEmpty())
        channelIdentifier = device->getParameters().first().value(QStringLiteral("identifier"));

    return writeBackgroundSignalFile(samples, channelIdentifier, errorMessage);
}

bool pa22x::writeBackgroundSignalFile(const QVector<double>& samples,
                                      const QString& channelIdentifier,
                                      QString* errorMessage)
{
    if (samples.size() != UTShadowDevice::_PEWaveLength) {
        if (errorMessage)
            *errorMessage = QStringLiteral("当前通道波形长度异常。");
        return false;
    }

    QString content;
    content.reserve(samples.size() * 8);
    QTextStream stream(&content);
    for (double sample : samples)
        stream << sample << '\n';

    const QString appDir = QCoreApplication::applicationDirPath();
    const QString primaryPath = primaryBackgroundSignalFilePath(appDir);
    const QString legacyPath = legacyBackgroundSignalFilePath(appDir);
    QString writeError;
    if (!writeTextFileAtomically(primaryPath, content, &writeError)) {
        if (errorMessage)
            *errorMessage = QStringLiteral("背景信号文件写入失败：%1 | %2")
                    .arg(nativePath(primaryPath), writeError);
        return false;
    }

    if (!writeTextFileAtomically(legacyPath, content, &writeError)) {
        if (errorMessage)
            *errorMessage = QStringLiteral("背景信号兼容路径写入失败：%1 | %2")
                    .arg(nativePath(legacyPath), writeError);
        return false;
    }

    QString verifyError;
    if (!verifyBackgroundSignalFileContents(primaryPath, samples, &verifyError)) {
        if (errorMessage)
            *errorMessage = verifyError;
        return false;
    }

    if (!verifyBackgroundSignalFileContents(legacyPath, samples, &verifyError)) {
        if (errorMessage)
            *errorMessage = verifyError;
        return false;
    }

    const QString normalizedChannel = channelIdentifier.isEmpty()
            ? QStringLiteral("channel_1")
            : channelIdentifier;
    const QString backgroundSummary = QStringLiteral("%1 | %2")
            .arg(summarizeSeriesForExcel(QStringLiteral("background_samples"), samples, 3, 4))
            .arg(QStringLiteral("content_chars=%1").arg(content.size()));

    m_backgroundDirty = false;
    logStep(QStringLiteral("背景"),
            QStringLiteral("BG-02"),
            QStringLiteral("背景信号保存完成"),
            QStringLiteral("%1 | %2 | 通道=%3 | %4")
                .arg(nativePath(primaryPath),
                     nativePath(legacyPath),
                     normalizedChannel,
                     backgroundSummary));
    return true;
}

bool pa22x::captureBackgroundSignalFromDevice(int maxWaitMs, QString* errorMessage)
{
    if (!_pa22xClient) {
        if (errorMessage)
            *errorMessage = QStringLiteral("未连接设备。");
        return false;
    }

    UTShadowDevice* device = _pa22xClient->getSelectedShadowDevice();
    if (!device) {
        if (errorMessage)
            *errorMessage = QStringLiteral("未检测到可用设备。");
        return false;
    }

    if (m_isSimulationMode)
        return saveCurrentBackgroundSignal(errorMessage);

    const QString appDir = QCoreApplication::applicationDirPath();
    const QString filePath = primaryBackgroundSignalFilePath(appDir);
    const QString legacyPath = legacyBackgroundSignalFilePath(appDir);
    const QFileInfo previousInfo(filePath);
    const qint64 previousSize = previousInfo.exists() ? previousInfo.size() : -1;
    const QDateTime previousModified = previousInfo.lastModified();
    const QDateTime requestStartedAt = QDateTime::currentDateTimeUtc();
    QFile::remove(filePath);
    device->logWaData();

    const int waitMs = qMax(200, maxWaitMs);
    QElapsedTimer timer;
    timer.start();
    while (timer.elapsed() < waitMs) {
        QCoreApplication::processEvents(QEventLoop::AllEvents, 50);

        QString validationError;
        if (hasValidBackgroundSignal(&validationError)) {
            const QFileInfo currentInfo(filePath);
            const bool changedAfterRequest =
                    !previousInfo.exists()
                    || currentInfo.size() != previousSize
                    || currentInfo.lastModified() > previousModified
                    || currentInfo.lastModified().toUTC() >= requestStartedAt.addSecs(-1);
            if (!changedAfterRequest) {
                QThread::msleep(60);
                continue;
            }

            QFile primaryFile(filePath);
            if (primaryFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
                QString compatibilityError;
                const QString content = QString::fromUtf8(primaryFile.readAll());
                writeTextFileAtomically(legacyPath, content, &compatibilityError);
            }

            QString channelIdentifier = device->getSelectedChannel();
            if (channelIdentifier.isEmpty() && !device->getParameters().isEmpty())
                channelIdentifier = device->getParameters().first().value(QStringLiteral("identifier"));

            QVector<double> actualSamples;
            QString readError;
            readNumericSamplesFile(filePath, &actualSamples, &readError);
            QString sampleSummary = readError.isEmpty()
                    ? summarizeSeriesForExcel(QStringLiteral("captured_background"), actualSamples, 3, 4)
                    : QStringLiteral("captured_background_read_error=%1").arg(readError);

            m_backgroundDirty = false;
            logStep(QStringLiteral("背景"),
                    QStringLiteral("BG-03"),
                    QStringLiteral("设备背景采集完成"),
                    QStringLiteral("wait=%1ms, elapsed=%2ms, changed=%3, size=%4, file=%5, 通道=%6 | %7")
                        .arg(waitMs)
                        .arg(timer.elapsed())
                        .arg(changedAfterRequest ? QStringLiteral("true") : QStringLiteral("false"))
                        .arg(currentInfo.size())
                        .arg(QFileInfo(filePath).fileName(),
                             channelIdentifier.isEmpty() ? QStringLiteral("channel_1") : channelIdentifier)
                        .arg(sampleSummary));
            return true;
        }

        QThread::msleep(60);
    }

    QString fallbackError;
    if (saveCurrentBackgroundSignal(&fallbackError))
        return true;

    if (errorMessage) {
        QStringList errors;
        errors << QStringLiteral("未在限定时间内收到新的背景波形文件。");
        if (!fallbackError.isEmpty())
            errors << fallbackError;
        *errorMessage = errors.join(QStringLiteral("\n"));
    }
    return false;
}

bool pa22x::ensureBackgroundSignalReady(int maxWaitMs, QString* errorMessage, bool forceRefresh)
{
    QString validationError;
    if (!forceRefresh && hasValidBackgroundSignal(&validationError)) {
        m_backgroundDirty = false;
        return true;
    }

    QString captureError;
    if (captureBackgroundSignalFromDevice(maxWaitMs, &captureError))
        return true;

    if (errorMessage) {
        QStringList errors;
        if (!validationError.isEmpty())
            errors << validationError;
        if (!captureError.isEmpty())
            errors << captureError;
        if (errors.isEmpty())
            errors << QStringLiteral("未能在限定时间内获取背景波形。");
        *errorMessage = errors.join(QStringLiteral("\n"));
    }
    return false;
}

bool pa22x::refreshCurrentSpectrumPreview(bool writeSpectrumFile, QString* errorMessage)
{
    if (m_timerActive)
        return false;

    QVector<double> stone;
    QString waveSource;
    QString waveError;
    if (!collectDetectionWaveSamples(&stone, &waveSource, &waveError)) {
        if (errorMessage)
            *errorMessage = waveError;
        return false;
    }

    QString backgroundFileError;
    if (!hasValidBackgroundSignal(&backgroundFileError)) {
        if (errorMessage)
            *errorMessage = backgroundFileError;
        return false;
    }

    const QString appDir = QCoreApplication::applicationDirPath();
    QString filePath = legacyBackgroundSignalFilePath(appDir);
    QFile wfile(filePath);
    QVector<double> water;
    if (wfile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        while (!wfile.atEnd()) {
            bool ok = false;
            const double value = QString::fromUtf8(wfile.readLine()).trimmed().toDouble(&ok);
            if (ok)
                water << value;
        }
        wfile.close();
    }

    if (water.isEmpty()) {
        if (errorMessage)
            *errorMessage = QStringLiteral("背景信号文件为空。");
        return false;
    }

    int N = 1;
    while (N < water.size())
        N <<= 1;
    QVector<double> ywater;
    ywater.reserve(N);
    for (int i = 0; i < N; ++i)
        ywater << (i < water.size() ? water[i] : 0.0);

    QVector<Complex> datain(ywater.size());
    QVector<Complex> dataout(ywater.size());
    for (int i = 0; i < ywater.size(); ++i)
        datain[i].rl = ywater[i];

    QScopedPointer<fft> waterFft(new fft());
    waterFft->fft1(datain, datain.size(), dataout);

    fre.clear();
    amp.clear();
    const double samfs = 100.0;
    disfre = samfs / ywater.size();
    for (int i = 0; i < dataout.size() / 8; ++i) {
        fre << i * disfre;
        amp << std::sqrt(dataout[i].rl * dataout[i].rl + dataout[i].im * dataout[i].im);
    }

    int N2 = 1;
    while (N2 < stone.size())
        N2 <<= 1;
    QVector<double> ystone;
    ystone.reserve(N2);
    for (int i = 0; i < N2; ++i)
        ystone << (i < stone.size() ? stone[i] : 0.0);

    QVector<Complex> datain2(ystone.size());
    QVector<Complex> dataout2(ystone.size());
    for (int i = 0; i < ystone.size(); ++i)
        datain2[i].rl = ystone[i];

    QScopedPointer<fft> stoneFft(new fft());
    stoneFft->fft1(datain2, datain2.size(), dataout2);

    fre2.clear();
    amp2.clear();
    for (int i = 0; i < dataout2.size() / 8; ++i) {
        fre2 << i * disfre;
        amp2 << std::sqrt(dataout2[i].rl * dataout2[i].rl + dataout2[i].im * dataout2[i].im);
    }

    QVector<double> spectrum(amp.size());
    const double width = ui->dspb_width->value() * 0.001;
    const double specfmin = ui->spb_specfmin->value();
    const double specfmax = ui->spb_specfmax->value();
    for (int i = 0; i < amp.size(); ++i) {
        if (i >= amp2.size() || amp[i] <= 0 || amp2[i] <= 0 || width <= 0) {
            spectrum[i] = 0;
            continue;
        }
        spectrum[i] = std::log(amp[i] / amp2[i]) / width;
    }

    posl = 0;
    posh = spectrum.size();
    for (int i = 0; i < spectrum.size(); ++i) {
        if (fre[i] > specfmin) {
            posl = i;
            break;
        }
    }
    for (int i = 0; i < spectrum.size(); ++i) {
        if (fre[i] > specfmax) {
            posh = i;
            break;
        }
    }

    myfre.clear();
    myspectrum.clear();
    for (int i = posl; i < posh; ++i) {
        myfre << fre[i];
        myspectrum << spectrum[i];
    }

    if (myfre.isEmpty() || myspectrum.isEmpty()) {
        if (errorMessage)
            *errorMessage = QStringLiteral("当前衰减谱为空，请调整频段或确认波形。");
        return false;
    }

    double minsp = myspectrum.first();
    double maxsp = myspectrum.first();
    for (double value : myspectrum) {
        minsp = qMin(minsp, value);
        maxsp = qMax(maxsp, value);
    }

    onPlotDataUpdate(specfmin, specfmax, minsp, maxsp);

    if (writeSpectrumFile) {
        QString spectrumContent;
        QTextStream spectrumStream(&spectrumContent);
        spectrumStream.setCodec("UTF-8");
        for (int i = 0; i < myspectrum.size(); ++i) {
            spectrumStream << myfre[i] << "  " << myspectrum[i];
            if (i < myspectrum.size() - 1)
                spectrumStream << "\n";
        }
        QString writeError;
        const QString logFilePath = QDir(QCoreApplication::applicationDirPath()).filePath(QStringLiteral("1.txt"));
        if (!writeTextFileAtomically(logFilePath, spectrumContent, &writeError)) {
            if (errorMessage)
                *errorMessage = QStringLiteral("衰减谱文件写入失败：") + writeError;
            return false;
        }
    }

    logStep(QStringLiteral("预览"),
            QStringLiteral("SPC-01"),
            QStringLiteral("衰减谱预览已刷新"),
            QStringLiteral("频带=%1-%2MHz | 波形=%3")
                .arg(QString::number(specfmin, 'f', 2),
                     QString::number(specfmax, 'f', 2),
                     waveSource.isEmpty() ? QStringLiteral("<unknown>") : waveSource));
    return true;
}

void pa22x::markBackgroundDirty(const QString& reason, bool clearSpectrum)
{
    const bool wasDirty = m_backgroundDirty;
    m_backgroundDirty = true;
    if (clearSpectrum && ui->customPlot3 && !wasDirty) {
        ui->customPlot3->clearGraphs();
        ui->customPlot3->addGraph();
        ui->customPlot3->replot();
    }

    if (!reason.isEmpty()) {
        const QDateTime now = QDateTime::currentDateTime();
        const bool shouldLog = !wasDirty
                || reason != m_lastBackgroundDirtyReason
                || !m_lastBackgroundDirtyLoggedAt.isValid()
                || m_lastBackgroundDirtyLoggedAt.msecsTo(now) >= 1500;
        if (shouldLog) {
            logStep(QStringLiteral("背景"), QStringLiteral("BG-00"), QStringLiteral("背景需重新保存"), reason);
            m_lastBackgroundDirtyReason = reason;
            m_lastBackgroundDirtyLoggedAt = now;
        }
    }
}

void pa22x::disconnectClient(bool showMessage, bool forShutdown)
{
    if (!g_shutdownLoggingSuppressed.load())
        logStep(QStringLiteral("连接"), QStringLiteral("CON-04"), QStringLiteral("资源清理开始"));
    m_connectOperationInProgress = false;
    QApplication::restoreOverrideCursor();
    if (serial != nullptr) {
        serial->CloseSerial();
        if (forShutdown)
            delete serial;
        else
            serial->deleteLater();
        serial = nullptr;
    }

    if (m_timer != nullptr) {
        m_timer->stop();
        m_timer->disconnect();
        if (forShutdown)
            delete m_timer;
        else
            m_timer->deleteLater();
        m_timer = nullptr;
    }
    m_timerActive = false;
    m_analysisRunning = false;
    QObject::disconnect(this, &pa22x::check, this, &pa22x::onserialcheck);

    if (_pa22xClient) {
        if (!forShutdown) {
            if (UTShadowDevice* device = _pa22xClient->getSelectedShadowDevice())
                device->WaveDataClose();
        }
        _pa22xClient->disconnectFromServer();
        if (forShutdown)
            delete _pa22xClient;
        else
            _pa22xClient->deleteLater();
        _pa22xClient = nullptr;
    }

    if (_displayTimer > 0) {
        killTimer(_displayTimer);
        _displayTimer = 0;
    }
    if (_evaluatorTimer > 0) {
        killTimer(_evaluatorTimer);
        _evaluatorTimer = 0;
    }

    is_start = 0;
    statime1 = false;
    m_isconnect = false;
    m_isSimulationMode = false;
    m_simulationWorkflowRunning = false;
    m_currentTime = QDateTime();
    m_backgroundDirty = true;
    m_displayedWaveSamples.clear();
    m_displayedWaveChannelIdentifier.clear();
    m_lastDisplayedWaveCapturedAt = QDateTime();

    if (forShutdown)
        return;

    ui->deviceSelection->clear();
    ui->PEChannelSelection->clear();
    ui->connectButton->setChecked(false);
    ui->connectButton->setText(QStringLiteral("连接"));
    ui->SpecAnaly->setChecked(false);
    ui->SpecAnaly->setText(QStringLiteral("开始检测"));
    refreshWaveCaptureButtonState();
    ui->simulateDetectButton->setText(QStringLiteral("模拟检测"));
    resetDetectionSessionState(true);
    ui->label_d50->setText(QStringLiteral("--"));
    ui->label_d90->setText(QStringLiteral("--"));
    ui->label_pass2->setText(QStringLiteral("--"));
    ui->label_aveD90->setText(QStringLiteral("--"));
    ui->label_avepass->setText(QStringLiteral("--"));
    ui->customPlot3->clearGraphs();
    ui->customPlot3->addGraph();
    ui->customPlot4->clearGraphs();
    ui->customPlot4->addGraph();
    applyVisualTheme(ui->pushButton_mode->isChecked());

    logStep(QStringLiteral("连接"), QStringLiteral("CON-05"), QStringLiteral("资源清理完成"));
    if (showMessage && ui->statusbar)
        ui->statusbar->showMessage(QStringLiteral("已安全断开连接"), 4000);
}

void pa22x::resetDetectionSessionState(bool clearDisplayedLabels)
{
    count3 = 0;
    coun2 = 0;
    specflag = false;
    avespecflag = false;
    avedv90 = 0.0;
    avepass = 0.0;
    curr2 = 0.0;
    m_lastDetectionWaveSignature = 0;
    m_lastDetectionSpectrumSignature = 0;
    m_hasLastDetectionWaveSignature = false;
    m_hasLastDetectionSpectrumSignature = false;
    m_lastValidD50 = 0.0;
    m_lastValidD90 = 0.0;
    m_lastValidPassrate = 0.0;
    m_hasLastValidSingleResult = false;

    if (!clearDisplayedLabels || !ui)
        return;

    ui->label_d50->setText(QStringLiteral("--"));
    ui->label_d90->setText(QStringLiteral("--"));
    ui->label_pass2->setText(QStringLiteral("--"));
    ui->label_aveD90->setText(QStringLiteral("--"));
    ui->label_avepass->setText(QStringLiteral("--"));
}

void pa22x::setInternalToolsUnlocked(bool unlocked, bool announce)
{
    m_internalToolsUnlocked = unlocked;
    if (ui->simulateDetectButton)
        ui->simulateDetectButton->setVisible(unlocked);

    if (!announce)
        return;

    logStep(QStringLiteral("系统"),
            QStringLiteral("DBG-01"),
            unlocked ? QStringLiteral("模拟入口已显示") : QStringLiteral("模拟入口已隐藏"),
            QStringLiteral("快捷键=Ctrl+Shift+Space"));
}

QString pa22x::buildCurrentParameterEntry(UTShadowDevice* device) const
{
    if (!device || device->getSelectedChannel().isEmpty())
        return QString();

    int channelNumber = -1;
    QRegularExpression re("[0-9]+");
    QRegularExpressionMatch match = re.match(device->getSelectedChannel());
    if (match.hasMatch())
        channelNumber = match.captured(0).toInt();
    if (channelNumber <= 0 || channelNumber > device->getParameters().size())
        return QString();

    const QMap<QString, QString>& parameters = device->getParameters()[channelNumber - 1];
    QString entry;
    entry += "channel = "            + device->getSelectedChannel() + "\n";
    entry += "prf = "                + _pa22xClient->getPEPRF() + "\n";
    entry += "high_voltage = "       + device->getHighVoltage() + "\n";
    entry += "gain = "               + parameters.value("gain") + "\n";
    entry += "pulse_width = "        + parameters.value("pulse_width") + "\n";
    entry += "range = "              + parameters.value("range") + "\n";
    entry += "delay = "              + parameters.value("delay") + "\n";
    entry += "origin = "             + parameters.value("origin") + "\n";
    entry += "filter = "             + parameters.value("filter") + "\n";
    entry += "dual_mode = "          + parameters.value("dual_mode") + "\n";
    entry += "damping = "            + parameters.value("damping") + "\n";
    entry += "velocity = "           + parameters.value("velocity") + "\n";
    entry += "compress_rate = "      + parameters.value("compress_rate") + "\n";
    entry += "compress_mode = "      + parameters.value("compress_mode") + "\n";
    entry += "gate_a_position = "    + parameters.value("gate_a_position") + "\n";
    entry += "gate_a_width = "       + parameters.value("gate_a_width") + "\n";
    entry += "gate_a_threshold = "   + parameters.value("gate_a_threshold") + "\n";
    entry += "gate_a_measure = "     + parameters.value("gate_a_measure") + "\n";
    entry += "gate_a_tracing = "     + parameters.value("gate_a_tracing") + "\n";
    entry += "gate_a_alarm = "       + parameters.value("gate_a_alarm") + "\n";
    entry += "gate_b_position = "    + parameters.value("gate_b_position") + "\n";
    entry += "gate_b_width = "       + parameters.value("gate_b_width") + "\n";
    entry += "gate_b_threshold = "   + parameters.value("gate_b_threshold") + "\n";
    entry += "gate_b_measure = "     + parameters.value("gate_b_measure") + "\n";
    entry += "gate_b_tracing = "     + parameters.value("gate_b_tracing") + "\n";
    entry += "gate_b_alarm = "       + parameters.value("gate_b_alarm") + "\n";
    entry += "gate_c_position = "    + parameters.value("gate_c_position") + "\n";
    entry += "gate_c_width = "       + parameters.value("gate_c_width") + "\n";
    entry += "gate_c_threshold = "   + parameters.value("gate_c_threshold") + "\n";
    entry += "gate_c_measure = "     + parameters.value("gate_c_measure") + "\n";
    entry += "gate_c_tracing = "     + parameters.value("gate_c_tracing") + "\n";
    entry += "gate_c_alarm = "       + parameters.value("gate_c_alarm") + "\n";
    entry += "gate_d_position = "    + parameters.value("gate_d_position") + "\n";
    entry += "gate_d_width = "       + parameters.value("gate_d_width") + "\n";
    entry += "gate_d_threshold = "   + parameters.value("gate_d_threshold") + "\n";
    entry += "gate_d_measure = "     + parameters.value("gate_d_measure") + "\n";
    entry += "gate_d_tracing = "     + parameters.value("gate_d_tracing") + "\n";
    entry += "gate_d_alarm = "       + parameters.value("gate_d_alarm") + "\n";
    entry += "rectification = "      + parameters.value("rectification") + "\n";
    return entry;
}

bool pa22x::applyParameterText(UTShadowDevice* device, const QString& parameterText, int* channelIndexOut)
{
    if (!device)
        return false;

    int channelIndex = 0;
    QRegularExpression re("([a-zA-Z0-9_]+)\\s*=\\s*([a-zA-Z0-9_.-]+)");
    QRegularExpressionMatchIterator it = re.globalMatch(parameterText);
    while (it.hasNext()) {
        QRegularExpressionMatch match = it.next();
        QString identifier = match.captured(1);
        QString value = match.captured(2);

        if (identifier == "channel") {
            QRegularExpression channelRe("[0-9]+");
            QRegularExpressionMatch channelMatch = channelRe.match(value);
            if (channelMatch.hasMatch())
                channelIndex = channelMatch.captured(0).toInt() - 1;
            if (channelIndex < 0)
                return false;
            device->setSelectedChannel(value);
        }
        else if (identifier == "prf") {
            _pa22xClient->setPEPRF(value);
        }
        else if (identifier == "high_voltage") {
            device->setHighVoltage(value);
        }
        else {
            device->parameterChanged(QString(identifier) + ":" + value);
        }
    }

    if (channelIndexOut)
        *channelIndexOut = channelIndex;
    return true;
}

bool pa22x::loadHistoryFile(const QString& fileName)
{
    QFile file2(fileName);
    if (!file2.open(QIODevice::ReadOnly|QIODevice::Text))
        return false;

    QVector<double> allDv90;
    QTextStream ina(&file2);
    while(!ina.atEnd()) {
        QString line = ina.readLine();
        QStringList list = line.split(QRegularExpression("\\s+"), QString::SkipEmptyParts);
        if (list.size() < 2)
            continue;
        bool ok = false;
        const double value = list.last().toDouble(&ok);
        if (ok)
            allDv90.push_back(value);
    }

    if (allDv90.isEmpty())
        return false;

    QVector<double> td;
    for (int i = 0; i < allDv90.size(); i++)
        td << i + 1;

    const auto amaxd90 = std::max_element(allDv90.begin(), allDv90.end());
    const auto amind90 = std::min_element(allDv90.begin(), allDv90.end());
    const double abigd90 = *amaxd90;
    const double asmad90 = *amind90;

    ui->customPlot4->clearGraphs();
    ui->customPlot4->addGraph();
    ui->customPlot4->graph(0)->setData(td, allDv90);
    ui->customPlot4->graph(0)->setPen(QPen(ui->pushButton_mode->isChecked()
                                               ? QColor(QStringLiteral("#5d8f9c"))
                                               : QColor(QStringLiteral("#88b4c0")),
                                           2.0));
    ui->customPlot4->xAxis->setRange(0, allDv90.size() + 1);
    ui->customPlot4->yAxis->setRange(paddedRange(asmad90, abigd90));
    ui->customPlot4->replot();
    return true;
}

void pa22x::generateSimulationInversionFiles(const QString& appDir) const
{
    writeTextFileAtomically(QDir(appDir).filePath(QStringLiteral("2.txt")),
                            QStringLiteral("0 2.1e-05 0 0 3.2e-05 0 4.3e-05 0 5.0e-05 0 7.2e-05 0\n"));

    writeTextFileAtomically(QDir(appDir).filePath(QStringLiteral("3.txt")),
                            QStringLiteral("1.0e-05 0.12 0.05\n"
                                           "2.0e-05 0.18 0.18\n"
                                           "3.5e-05 0.27 0.42\n"
                                           "4.5e-05 0.24 0.63\n"
                                           "5.5e-05 0.12 0.83\n"
                                           "7.0e-05 0.07 0.94\n"
                                           "9.0e-05 0.03 1.00\n"));
}

void pa22x::finishSimulationWorkflowLegacy()
{
    if (!m_simulationWorkflowRunning)
        return;

    logStep(QStringLiteral("模拟"), QStringLiteral("SIM-06"), QStringLiteral("模拟自测校验开始"));

    if (ui->SpecAnaly->isChecked()) {
        ui->SpecAnaly->setChecked(false);
        on_SpecAnaly_clicked(false);
    }

    QStringList failures;
    const QString appDir = QCoreApplication::applicationDirPath();
    const QString signalDir = QDir(appDir).filePath("Signals");
    const QString distributionDir = QDir(appDir).filePath("Distribution");
    QString validationError;

    const QString backgroundFile = primaryBackgroundSignalFilePath(appDir);
    if (!QFileInfo::exists(backgroundFile))
        failures << "背景信号文件未生成";
    else if (!validateSignalSampleFile(backgroundFile, 448, &validationError))
        failures << validationError;

    const QStringList waveFiles = waveSignalFilePathsForValidation(appDir);
    if (waveFiles.isEmpty()) {
        failures << "单次信号文件未生成";
    } else {
        for (const QString& filePath : waveFiles) {
            if (!validateSignalSampleFile(filePath, 448, &validationError)) {
                failures << validationError;
                break;
            }
        }
    }

    const QString spectrumFile = QDir(appDir).filePath(QStringLiteral("1.txt"));
    if (!QFileInfo::exists(spectrumFile))
        failures << "衰减谱文件 1.txt 未生成";
    else if (!validateSpectrumFile(spectrumFile, 5, &validationError))
        failures << validationError;

    const QString parameterRoundTripFile = QDir(appDir).filePath(QStringLiteral("Simulation/Parameters_simulation.txt"));
    if (!QFileInfo::exists(parameterRoundTripFile)) {
        failures << "参数保存/加载测试文件未生成";
    } else {
        QFile file(parameterRoundTripFile);
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            failures << "参数保存/加载测试文件无法读取";
        } else {
            const QString content = QString::fromUtf8(file.readAll());
            if (!content.contains(QStringLiteral("channel = channel_")) ||
                !content.contains(QStringLiteral("gain =")) ||
                !content.contains(QStringLiteral("high_voltage ="))) {
                failures << "参数保存/加载测试文件内容不完整";
            }
        }
    }

    const QString today = QDateTime::currentDateTime().toString("yyyyMMdd");
    const QString adv90File = QDir(distributionDir).filePath(QStringLiteral("Adv90_%1.txt").arg(today));
    if (!QFileInfo::exists(adv90File))
        failures << "Adv90 文件未生成";
    else if (!validateTimedColumnsFile(adv90File, 2, &validationError))
        failures << validationError;

    const QString currentFile = QDir(distributionDir).filePath(QStringLiteral("Current_%1.txt").arg(today));
    if (!QFileInfo::exists(currentFile))
        failures << "Current 文件未生成";
    else if (!validateTimedColumnsFile(currentFile, 2, &validationError))
        failures << validationError;

    const QString parameterFile = QDir(distributionDir).filePath(QStringLiteral("Distribution_Parameter_%1.txt").arg(today));
    if (!QFileInfo::exists(parameterFile))
        failures << "Distribution_Parameter 文件未生成";
    else if (!validateTimedColumnsFile(parameterFile, 6, &validationError))
        failures << validationError;

    const QString passFile = QDir(distributionDir).filePath(QStringLiteral("BPass_%1.txt").arg(today));
    if (!QFileInfo::exists(passFile))
        failures << "BPass 文件未生成";
    else if (!validateTimedColumnsFile(passFile, 2, &validationError))
        failures << validationError;

    const QString avepassFile = QDir(distributionDir).filePath(QStringLiteral("Avepass_%1.txt").arg(today));
    if (!QFileInfo::exists(avepassFile))
        failures << "Avepass 文件未生成";
    else if (!validateTimedColumnsFile(avepassFile, 2, &validationError))
        failures << validationError;

    const QString currentOutputFile = QDir(appDir).filePath(QStringLiteral("4.txt"));
    if (!validateSignalSampleFile(currentOutputFile, 1, &validationError))
        failures << validationError;

    if (!loadHistoryFile(adv90File))
        failures << "历史曲线加载失败";

    if (ui->label_d50->text().trimmed().isEmpty()
            || ui->label_d90->text().trimmed().isEmpty()
            || ui->label_pass2->text().trimmed().isEmpty()
            || ui->label_aveD90->text().trimmed().isEmpty()
            || ui->label_avepass->text().trimmed().isEmpty())
        failures << "检测结果标签未更新";

    m_simulationWorkflowRunning = false;
    ui->simulateDetectButton->setText(QStringLiteral("模拟检测"));

    if (failures.isEmpty()) {
        logStep(QStringLiteral("模拟"), QStringLiteral("SIM-09"), QStringLiteral("模拟自测通过"));
        if (isSimulationSelfTestMode()) {
            writeSimulationSelfTestReport(QStringLiteral("PASS"),
                                          QStringList() << QStringLiteral("模拟检测、自测文件校验与历史曲线回放均已通过。"));
            QTimer::singleShot(0, QCoreApplication::instance(), SLOT(quit()));
        } else {
            QMessageBox::information(this, "模拟检测完成",
                                     "模拟检测、自测文件校验与历史曲线回放均已通过。");
        }
    } else {
        logStep(QStringLiteral("模拟"),
                QStringLiteral("SIM-09"),
                QStringLiteral("模拟自测失败"),
                failures.join(QStringLiteral(" | ")));
        if (isSimulationSelfTestMode()) {
            writeSimulationSelfTestReport(QStringLiteral("FAIL"), failures);
            QTimer::singleShot(0, QCoreApplication::instance(), SLOT(quit()));
        } else {
            QMessageBox::warning(this, "模拟检测发现问题", failures.join("\n"));
        }
    }
}

void pa22x::runSimulationWorkflowLegacy()
{
    if (!_pa22xClient) {
        if (isSimulationSelfTestMode()) {
            writeSimulationSelfTestReport(QStringLiteral("FAIL"),
                                          QStringList() << QStringLiteral("模拟流程启动失败：客户端为空"));
            QTimer::singleShot(0, QCoreApplication::instance(), SLOT(quit()));
        }
        return;
    }

    UTShadowDevice* device = _pa22xClient->getSelectedShadowDevice();
    if (!device) {
        if (isSimulationSelfTestMode()) {
            writeSimulationSelfTestReport(QStringLiteral("FAIL"),
                                          QStringList() << QStringLiteral("模拟流程启动失败：未发现设备"));
            QTimer::singleShot(0, QCoreApplication::instance(), SLOT(quit()));
        }
        return;
    }

    m_simulationWorkflowRunning = true;
    ui->simulateDetectButton->setText("模拟中...");
    logStep(QStringLiteral("模拟"), QStringLiteral("SIM-02"), QStringLiteral("模拟工作流开始"));

    const QString appDir = QCoreApplication::applicationDirPath();
    const QString signalDirPath = QDir(appDir).filePath("Signals");
    const QString distributionDirPath = QDir(appDir).filePath("Distribution");
    QFile::remove(simulationSelfTestReportPath());
    QDir signalDir(signalDirPath);
    QDir distributionDir(distributionDirPath);
    const QString today = QDateTime::currentDateTime().toString("yyyyMMdd");
    signalDir.remove("boxing_water.txt");
    QFile::remove(legacyBackgroundSignalFilePath(appDir));
    for (const QString& directoryPath : waveSignalDirectoriesForValidation(appDir)) {
        QDir directory(directoryPath);
        for (const QString& fileName : directory.entryList(QStringList() << QStringLiteral("WaveRf_*.txt"), QDir::Files))
            directory.remove(fileName);
    }
    QFile::remove(QDir(appDir).filePath("1.txt"));
    QFile::remove(QDir(appDir).filePath("2.txt"));
    QFile::remove(QDir(appDir).filePath("3.txt"));
    QFile::remove(QDir(appDir).filePath("4.txt"));
    QFile::remove(QDir(appDir).filePath("Simulation/Parameters_simulation.txt"));
    distributionDir.remove(QStringLiteral("Adv90_%1.txt").arg(today));
    distributionDir.remove(QStringLiteral("Current_%1.txt").arg(today));
    distributionDir.remove(QStringLiteral("Distribution_Parameter_%1.txt").arg(today));
    distributionDir.remove(QStringLiteral("BPass_%1.txt").arg(today));
    distributionDir.remove(QStringLiteral("Avepass_%1.txt").arg(today));

    // 覆盖通道数、通道切换、参数更新、参数保存/加载等主要流程。
    if (ui->PEChannelCountSelection->count() > 0)
        on_PEChannelCountSelection_activated(0);

    const int channelCountIndex = qMin(3, ui->PEChannelCountSelection->count() - 1);
    if (channelCountIndex > 0)
        on_PEChannelCountSelection_activated(channelCountIndex);
    if (ui->PEChannelSelection->count() > 1)
        PEChannelActived(1);

    on_PEGainInput_valueChanged(ui->PEGainInput->value());
    on_PERangeInput_valueChanged(ui->PERangeInput->value());
    on_PEDelayInput_valueChanged(ui->PEDelayInput->value());

    const QString parameterEntry = buildCurrentParameterEntry(device);
    if (!parameterEntry.isEmpty()) {
        const QString simulationDirPath = QDir(appDir).filePath("Simulation");
        QDir().mkpath(simulationDirPath);
        const QString parameterFilePath = QDir(simulationDirPath).filePath("Parameters_simulation.txt");
        QSaveFile parameterFile(parameterFilePath);
        if (parameterFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
            QTextStream out(&parameterFile);
            out << parameterEntry;
            parameterFile.commit();
        }

        int loadedChannel = 0;
        QFile reloadFile(parameterFilePath);
        QString parameterText = parameterEntry;
        if (reloadFile.open(QIODevice::ReadOnly | QIODevice::Text))
            parameterText = QString::fromUtf8(reloadFile.readAll());
        applyParameterText(device, parameterText, &loadedChannel);
        if (loadedChannel >= 0)
            PEChannelActived(loadedChannel);
    }

    on_PESaveWave_clicked();
    on_WaSaveWave_clicked();
    on_STSaveWave_clicked();

    if (!ui->SpecAnaly->isChecked()) {
        ui->SpecAnaly->setChecked(true);
        on_SpecAnaly_clicked(true);
    }

    const int waitMs = qMax(3500, ui->spb_time->value() * 1000 + 1500);
    logStep(QStringLiteral("模拟"),
            QStringLiteral("SIM-05"),
            QStringLiteral("等待检测完成"),
            QStringLiteral("%1ms").arg(waitMs));
    QTimer::singleShot(waitMs, this, &pa22x::finishSimulationWorkflow);
}

//析构函数
void pa22x::finishSimulationWorkflow()
{
    if (!m_simulationWorkflowRunning)
        return;

    logStep(QStringLiteral("模拟"), QStringLiteral("SIM-06"), QStringLiteral("模拟自测校验开始"));

    if (ui->SpecAnaly->isChecked()) {
        ui->SpecAnaly->setChecked(false);
        on_SpecAnaly_clicked(false);
    }

    QStringList failures;
    QStringList reportLines;
    auto recordCheck = [&](bool passed,
                           const QString& code,
                           const QString& name,
                           const QString& detail = QString()) {
        reportLines << simulationCheckLine(passed, code, name, detail);
        if (!passed) {
            if (detail.isEmpty())
                failures << QStringLiteral("[%1] %2").arg(code, name);
            else
                failures << QStringLiteral("[%1] %2 | %3").arg(code, name, detail);
        }
    };

    const QString appDir = QCoreApplication::applicationDirPath();
    const QString signalDir = QDir(appDir).filePath(QStringLiteral("Signals"));
    const QString distributionDir = QDir(appDir).filePath(QStringLiteral("Distribution"));
    const QDateTime baseline = m_simulationStartedAt;
    const int expectedCycles = qMax(1, m_simulationExpectedCycles);
    const int expectedIntervalSeconds = qMax(1, ui->spb_time->value());
    QString validationError;

    const QString backgroundFile = primaryBackgroundSignalFilePath(appDir);
    validationError.clear();
    const bool backgroundOk = QFileInfo::exists(backgroundFile)
            && validateSignalSampleFile(backgroundFile, 448, &validationError)
            && validateRecentFile(backgroundFile, baseline, 2, &validationError);
    recordCheck(backgroundOk, QStringLiteral("SIM-10"), QStringLiteral("背景信号文件校验"),
                backgroundOk ? QFileInfo(backgroundFile).fileName() : validationError);

    const QStringList waveFiles = waveSignalFilePathsForValidation(appDir);
    bool waveFilesOk = !waveFiles.isEmpty();
    validationError.clear();
    if (waveFilesOk) {
        for (const QString& filePath : waveFiles) {
            if (!validateSignalSampleFile(filePath, 448, &validationError)
                    || !validateRecentFile(filePath, baseline, 2, &validationError)) {
                waveFilesOk = false;
                break;
            }
        }
    }
    if (!waveFilesOk && validationError.isEmpty())
        validationError = QStringLiteral("单次信号文件未生成");
    recordCheck(waveFilesOk, QStringLiteral("SIM-11"), QStringLiteral("单次信号文件校验"),
                waveFilesOk ? QStringLiteral("count=%1").arg(waveFiles.size()) : validationError);

    const QString spectrumFile = QDir(appDir).filePath(QStringLiteral("1.txt"));
    validationError.clear();
    const bool spectrumOk = QFileInfo::exists(spectrumFile)
            && validateSpectrumFile(spectrumFile, 5, &validationError)
            && validateRecentFile(spectrumFile, baseline, 2, &validationError);
    recordCheck(spectrumOk, QStringLiteral("SIM-12"), QStringLiteral("衰减谱文件校验"),
                spectrumOk ? QFileInfo(spectrumFile).fileName() : validationError);

    const QString parameterRoundTripFile = QDir(appDir).filePath(QStringLiteral("Simulation/Parameters_simulation.txt"));
    bool parameterRoundTripOk = QFileInfo::exists(parameterRoundTripFile);
    validationError.clear();
    if (parameterRoundTripOk) {
        QFile file(parameterRoundTripFile);
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            parameterRoundTripOk = false;
            validationError = QStringLiteral("参数保存/加载测试文件无法读取");
        } else {
            const QString content = QString::fromUtf8(file.readAll());
            if (!content.contains(QStringLiteral("channel = channel_"))
                    || !content.contains(QStringLiteral("gain ="))
                    || !content.contains(QStringLiteral("high_voltage ="))) {
                parameterRoundTripOk = false;
                validationError = QStringLiteral("参数保存/加载测试文件内容不完整");
            }
        }
    }
    if (parameterRoundTripOk)
        parameterRoundTripOk = validateRecentFile(parameterRoundTripFile, baseline, 2, &validationError);
    if (!parameterRoundTripOk && validationError.isEmpty())
        validationError = QStringLiteral("参数保存/加载测试文件未生成");
    recordCheck(parameterRoundTripOk, QStringLiteral("SIM-13"), QStringLiteral("参数保存加载校验"),
                parameterRoundTripOk ? QFileInfo(parameterRoundTripFile).fileName() : validationError);

    const QString today = QDateTime::currentDateTime().toString(QStringLiteral("yyyyMMdd"));
    const QString adv90File = QDir(distributionDir).filePath(QStringLiteral("Adv90_%1.txt").arg(today));
    validationError.clear();
    const bool adv90Ok = QFileInfo::exists(adv90File)
            && validateTimedColumnsFile(adv90File, 2, expectedCycles, &validationError)
            && validateMeasurementIntervals(adv90File,
                                            expectedIntervalSeconds,
                                            qMax(0, expectedCycles - 1),
                                            &validationError)
            && validateRecentFile(adv90File, baseline, 2, &validationError);
    recordCheck(adv90Ok, QStringLiteral("SIM-14"), QStringLiteral("Adv90 结果校验"),
                adv90Ok ? QStringLiteral("cycles>=%1, interval=%2s")
                             .arg(expectedCycles)
                             .arg(expectedIntervalSeconds)
                        : validationError);

    const QString currentFile = QDir(distributionDir).filePath(QStringLiteral("Current_%1.txt").arg(today));
    validationError.clear();
    const bool currentOk = QFileInfo::exists(currentFile)
            && validateTimedColumnsFile(currentFile, 2, expectedCycles, &validationError)
            && validateMeasurementIntervals(currentFile,
                                            expectedIntervalSeconds,
                                            qMax(0, expectedCycles - 1),
                                            &validationError)
            && validateRecentFile(currentFile, baseline, 2, &validationError);
    recordCheck(currentOk, QStringLiteral("SIM-15"), QStringLiteral("Current 结果校验"),
                currentOk ? QStringLiteral("cycles>=%1, interval=%2s")
                               .arg(expectedCycles)
                               .arg(expectedIntervalSeconds)
                          : validationError);

    const QString parameterFile = QDir(distributionDir).filePath(QStringLiteral("Distribution_Parameter_%1.txt").arg(today));
    validationError.clear();
    const bool distributionParameterOk = QFileInfo::exists(parameterFile)
            && validateTimedColumnsFile(parameterFile, 6, expectedCycles, &validationError)
            && validateMeasurementIntervals(parameterFile,
                                            expectedIntervalSeconds,
                                            qMax(0, expectedCycles - 1),
                                            &validationError)
            && validateRecentFile(parameterFile, baseline, 2, &validationError);
    recordCheck(distributionParameterOk, QStringLiteral("SIM-16"), QStringLiteral("Distribution_Parameter 校验"),
                distributionParameterOk ? QStringLiteral("cycles>=%1, interval=%2s")
                                             .arg(expectedCycles)
                                             .arg(expectedIntervalSeconds)
                                        : validationError);

    const QString passFile = QDir(distributionDir).filePath(QStringLiteral("BPass_%1.txt").arg(today));
    validationError.clear();
    const bool passOk = QFileInfo::exists(passFile)
            && validateTimedColumnsFile(passFile, 2, expectedCycles, &validationError)
            && validateMeasurementIntervals(passFile,
                                            expectedIntervalSeconds,
                                            qMax(0, expectedCycles - 1),
                                            &validationError)
            && validateRecentFile(passFile, baseline, 2, &validationError);
    recordCheck(passOk, QStringLiteral("SIM-17"), QStringLiteral("BPass 结果校验"),
                passOk ? QStringLiteral("cycles>=%1, interval=%2s")
                            .arg(expectedCycles)
                            .arg(expectedIntervalSeconds)
                       : validationError);

    const QString avepassFile = QDir(distributionDir).filePath(QStringLiteral("Avepass_%1.txt").arg(today));
    validationError.clear();
    const bool avepassOk = QFileInfo::exists(avepassFile)
            && validateTimedColumnsFile(avepassFile, 2, expectedCycles, &validationError)
            && validateMeasurementIntervals(avepassFile,
                                            expectedIntervalSeconds,
                                            qMax(0, expectedCycles - 1),
                                            &validationError)
            && validateRecentFile(avepassFile, baseline, 2, &validationError);
    recordCheck(avepassOk, QStringLiteral("SIM-18"), QStringLiteral("Avepass 结果校验"),
                avepassOk ? QStringLiteral("cycles>=%1, interval=%2s")
                               .arg(expectedCycles)
                               .arg(expectedIntervalSeconds)
                          : validationError);

    const QString currentOutputFile = QDir(appDir).filePath(QStringLiteral("4.txt"));
    validationError.clear();
    const bool analogOutputOk = validateSignalSampleFile(currentOutputFile, 1, &validationError)
            && validateRecentFile(currentOutputFile, baseline, 2, &validationError);
    recordCheck(analogOutputOk, QStringLiteral("SIM-19"), QStringLiteral("4-20mA 输出文件校验"),
                analogOutputOk ? QFileInfo(currentOutputFile).fileName() : validationError);

    const bool historyOk = loadHistoryFile(adv90File);
    recordCheck(historyOk, QStringLiteral("SIM-20"), QStringLiteral("历史曲线回放校验"),
                historyOk ? QFileInfo(adv90File).fileName() : QStringLiteral("历史曲线加载失败"));

    const QStringList metricValues = {
        ui->label_d50->text().trimmed(),
        ui->label_d90->text().trimmed(),
        ui->label_pass2->text().trimmed(),
        ui->label_aveD90->text().trimmed(),
        ui->label_avepass->text().trimmed()
    };
    const bool labelOk = std::all_of(metricValues.cbegin(),
                                     metricValues.cend(),
                                     [](const QString& value) {
                                         return !value.isEmpty() && value != QStringLiteral("--");
                                     });
    recordCheck(labelOk, QStringLiteral("SIM-21"), QStringLiteral("检测结果标签校验"),
                labelOk ? QStringLiteral("D50=%1, D90=%2, Pass=%3, AveD90=%4, AvePass=%5")
                              .arg(ui->label_d50->text(),
                                   ui->label_d90->text(),
                                   ui->label_pass2->text(),
                                   ui->label_aveD90->text(),
                                   ui->label_avepass->text())
                        : QStringLiteral("检测结果标签未更新"));

    const double aveD90PolicyNormal = computeDisplayedAveD90ForSequence(
                QVector<double> { 41.5, 46.5, 52.0 }, 10, 9.0);
    const double aveD90PolicyWithOutliers = computeDisplayedAveD90ForSequence(
                QVector<double> { 41.5, 46.5, 52.0, 95.5, 102.0 }, 10, 9.0);
    const double aveD90PolicyOutliersOnly = computeDisplayedAveD90ForSequence(
                QVector<double> { 95.5, 102.0 }, 10, 9.0);
    const bool aveD90PolicyOk = qAbs(aveD90PolicyNormal - 55.67) < 0.02
            && qAbs(aveD90PolicyWithOutliers - aveD90PolicyNormal) < 0.02
            && qAbs(aveD90PolicyOutliersOnly) < 0.01;
    recordCheck(aveD90PolicyOk,
                QStringLiteral("SIM-21A"),
                QStringLiteral("AveD90 超限过滤规则校验"),
                QStringLiteral("normal=%1 | withOutliers=%2 | outliersOnly=%3 | range=%4 | offset=9.00")
                    .arg(QString::number(aveD90PolicyNormal, 'f', 2),
                         QString::number(aveD90PolicyWithOutliers, 'f', 2),
                         QString::number(aveD90PolicyOutliersOnly, 'f', 2),
                         processDv90AverageRangeForLog()));

    if (isSimulationSelfTestMode()) {
        disconnectClient(false);
        const bool disconnectOk = !_pa22xClient
                && !m_isconnect
                && ui->deviceSelection->count() == 0
                && ui->connectButton->text() == QStringLiteral("连接");
        recordCheck(disconnectOk, QStringLiteral("SIM-22"), QStringLiteral("断连收口校验"),
                    disconnectOk ? QStringLiteral("界面和客户端已复位")
                                 : QStringLiteral("断连后状态未完全复位"));
    }

    m_simulationWorkflowRunning = false;
    m_simulationExpectedCycles = 0;
    ui->simulateDetectButton->setText(QStringLiteral("模拟检测"));

    if (failures.isEmpty()) {
        logStep(QStringLiteral("模拟"), QStringLiteral("SIM-09"), QStringLiteral("模拟自测通过"),
                QStringLiteral("checks=%1").arg(reportLines.size()));
        if (isSimulationSelfTestMode()) {
            writeSimulationSelfTestReport(QStringLiteral("PASS"), reportLines);
            QTimer::singleShot(0, QCoreApplication::instance(), SLOT(quit()));
        } else {
            QMessageBox::information(this, "模拟检测完成",
                                     "模拟检测、自测文件校验、历史回放和流程收口均已通过。");
        }
    } else {
        logStep(QStringLiteral("模拟"), QStringLiteral("SIM-09"), QStringLiteral("模拟自测失败"),
                failures.join(QStringLiteral(" | ")));
        if (isSimulationSelfTestMode()) {
            writeSimulationSelfTestReport(QStringLiteral("FAIL"), reportLines + failures);
            QTimer::singleShot(0, QCoreApplication::instance(), SLOT(quit()));
        } else {
            QMessageBox::warning(this, "模拟检测发现问题", failures.join("\n"));
        }
    }
}

void pa22x::runSimulationWorkflow()
{
    if (!_pa22xClient) {
        if (isSimulationSelfTestMode()) {
            writeSimulationSelfTestReport(QStringLiteral("FAIL"),
                                          QStringList() << simulationCheckLine(false,
                                                                               QStringLiteral("SIM-00"),
                                                                               QStringLiteral("模拟流程启动"),
                                                                               QStringLiteral("客户端为空")));
            QTimer::singleShot(0, QCoreApplication::instance(), SLOT(quit()));
        }
        return;
    }

    UTShadowDevice* device = _pa22xClient->getSelectedShadowDevice();
    if (!device) {
        if (isSimulationSelfTestMode()) {
            writeSimulationSelfTestReport(QStringLiteral("FAIL"),
                                          QStringList() << simulationCheckLine(false,
                                                                               QStringLiteral("SIM-00"),
                                                                               QStringLiteral("模拟流程启动"),
                                                                               QStringLiteral("未发现设备")));
            QTimer::singleShot(0, QCoreApplication::instance(), SLOT(quit()));
        }
        return;
    }

    m_simulationWorkflowRunning = true;
    m_simulationExpectedCycles = 2;
    m_simulationStartedAt = QDateTime::currentDateTime();
    ui->simulateDetectButton->setText(QStringLiteral("模拟中..."));
    logStep(QStringLiteral("模拟"), QStringLiteral("SIM-02"), QStringLiteral("模拟工作流开始"));

    const QString appDir = QCoreApplication::applicationDirPath();
    const QString signalDirPath = QDir(appDir).filePath(QStringLiteral("Signals"));
    const QString distributionDirPath = QDir(appDir).filePath(QStringLiteral("Distribution"));
    QFile::remove(simulationSelfTestReportPath());
    QDir signalDir(signalDirPath);
    QDir distributionDir(distributionDirPath);
    const QString today = QDateTime::currentDateTime().toString(QStringLiteral("yyyyMMdd"));
    signalDir.remove(QStringLiteral("boxing_water.txt"));
    QFile::remove(legacyBackgroundSignalFilePath(appDir));
    for (const QString& directoryPath : waveSignalDirectoriesForValidation(appDir)) {
        QDir directory(directoryPath);
        for (const QString& fileName : directory.entryList(QStringList() << QStringLiteral("WaveRf_*.txt"), QDir::Files))
            directory.remove(fileName);
    }
    QFile::remove(QDir(appDir).filePath(QStringLiteral("1.txt")));
    QFile::remove(QDir(appDir).filePath(QStringLiteral("2.txt")));
    QFile::remove(QDir(appDir).filePath(QStringLiteral("3.txt")));
    QFile::remove(QDir(appDir).filePath(QStringLiteral("4.txt")));
    QFile::remove(QDir(appDir).filePath(QStringLiteral("Simulation/Parameters_simulation.txt")));
    distributionDir.remove(QStringLiteral("Adv90_%1.txt").arg(today));
    distributionDir.remove(QStringLiteral("Current_%1.txt").arg(today));
    distributionDir.remove(QStringLiteral("Distribution_Parameter_%1.txt").arg(today));
    distributionDir.remove(QStringLiteral("BPass_%1.txt").arg(today));
    distributionDir.remove(QStringLiteral("Avepass_%1.txt").arg(today));
    logStep(QStringLiteral("模拟"), QStringLiteral("SIM-03"), QStringLiteral("旧测试文件清理完成"));

    if (ui->PEChannelCountSelection->count() > 0)
        on_PEChannelCountSelection_activated(0);

    const int channelCountIndex = qMin(3, ui->PEChannelCountSelection->count() - 1);
    if (channelCountIndex > 0)
        on_PEChannelCountSelection_activated(channelCountIndex);
    if (ui->PEChannelSelection->count() > 1)
        PEChannelActived(1);

    on_PEGainInput_valueChanged(ui->PEGainInput->value());
    on_PERangeInput_valueChanged(ui->PERangeInput->value());
    on_PEDelayInput_valueChanged(ui->PEDelayInput->value());

    const QString parameterEntry = buildCurrentParameterEntry(device);
    if (parameterEntry.isEmpty()) {
        logStep(QStringLiteral("模拟"), QStringLiteral("SIM-04"), QStringLiteral("参数轮转跳过"),
                QStringLiteral("当前通道参数为空"));
    } else {
        const QString simulationDirPath = QDir(appDir).filePath(QStringLiteral("Simulation"));
        QDir().mkpath(simulationDirPath);
        const QString parameterFilePath = QDir(simulationDirPath).filePath(QStringLiteral("Parameters_simulation.txt"));
        QSaveFile parameterFile(parameterFilePath);
        if (parameterFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
            QTextStream out(&parameterFile);
            out.setCodec("UTF-8");
            out << parameterEntry;
            parameterFile.commit();
        }

        int loadedChannel = 0;
        QFile reloadFile(parameterFilePath);
        QString parameterText = parameterEntry;
        if (reloadFile.open(QIODevice::ReadOnly | QIODevice::Text))
            parameterText = QString::fromUtf8(reloadFile.readAll());
        applyParameterText(device, parameterText, &loadedChannel);
        if (loadedChannel >= 0)
            PEChannelActived(loadedChannel);

        logStep(QStringLiteral("模拟"), QStringLiteral("SIM-04"), QStringLiteral("参数和通道覆盖完成"),
                QStringLiteral("channelCount=%1").arg(ui->PEChannelSelection->count()));
    }

    on_PESaveWave_clicked();
    on_WaSaveWave_clicked();
    on_STSaveWave_clicked();

    if (!ui->SpecAnaly->isChecked()) {
        ui->SpecAnaly->setChecked(true);
        on_SpecAnaly_clicked(true);
    }

    const int waitMs = qMax(5500, m_simulationExpectedCycles * ui->spb_time->value() * 1000 + 2500);
    logStep(QStringLiteral("模拟"),
            QStringLiteral("SIM-05"),
            QStringLiteral("等待检测完成"),
            QStringLiteral("%1ms").arg(waitMs));
    QTimer::singleShot(waitMs, this, &pa22x::finishSimulationWorkflow);
}

pa22x::~pa22x()
{
    // 如果 shutdown() 已执行过清理, 不再重复操作
    if (!m_shutdownInProgress) {
        // 非正常退出路径的兜底清理
        if (_pa22xClient) {
            _pa22xClient->disconnectFromServer();
            delete _pa22xClient;
            _pa22xClient = nullptr;
        }
        if (serial != nullptr) {
            serial->CloseSerial();
            delete serial;
            serial = nullptr;
        }
        if (g_xlsxLogger) {
            delete g_xlsxLogger.data();
            g_xlsxLogger = nullptr;
        }
        Logger::shutdown();
    }

    // 图表清理（shutdown不会做，永远需要）
    if (ui) {
        if (ui->customPlot4)
            ui->customPlot4->clearGraphs();
        if (ui->customPlot3) {
            ui->customPlot3->clearGraphs();
            ui->customPlot3->clearItems();
            ui->customPlot3->clearPlottables();
        }
    }

    //仅当定时器有效时才终止
    if (_displayTimer > 0) {
        killTimer(_displayTimer);
        _displayTimer = 0;
    }
    if (_evaluatorTimer > 0) {
        killTimer(_evaluatorTimer);
        _evaluatorTimer = 0;
    }

    // 确保Logger只shutdown一次
    if (!m_shutdownInProgress) {
        Logger::shutdown();
    }
    g_statusBar.clear();
    delete ui;
}

//设置客户端
void pa22x::setPA22xClient(PA22XClient* pa22xClient)
{
    _pa22xClient = pa22xClient;
}
//报错对话框
void pa22x::onerror(QString error)
{
    const QDateTime now = QDateTime::currentDateTime();
    const bool transientSerialError = isTransientAnalogOutputError(error);

    logStep(QStringLiteral("错误"), QStringLiteral("收到错误"), error);
    if (ui && ui->statusbar) {
        const QString logName = QFileInfo(Logger::currentLogFilePath()).fileName();
        const bool shouldRefreshStatus =
            !transientSerialError
            || error != m_lastTransientSerialError
            || !m_lastTransientSerialErrorAt.isValid()
            || m_lastTransientSerialErrorAt.msecsTo(now) >= 3000;
        if (shouldRefreshStatus) {
            const QString prefix = transientSerialError
                ? QStringLiteral("[设备提示]")
                : QStringLiteral("[错误]");
            ui->statusbar->showMessage(QStringLiteral("%1 %2 | 查看日志 %3").arg(prefix, error, logName),
                                       transientSerialError ? 5000 : 15000);
        }
    }
    if (transientSerialError) {
        m_lastTransientSerialError = error;
        m_lastTransientSerialErrorAt = now;
        qWarning() << "非阻塞串口提示" + error;
        return;
    }
    m_lastTransientSerialError.clear();
    m_lastTransientSerialErrorAt = QDateTime();
    QMessageBox::critical(nullptr,"错误",error,QMessageBox::Ok);
    qWarning() << "报错" + error;
}
//重写widget显示 单纯为了显示lineEdit_Desk这个的地址
void pa22x::applyStartupPresentation()
{
    if (m_startupPresentationApplied)
        return;

    m_startupPresentationApplied = true;
    logStep(QStringLiteral("系统"),
            QStringLiteral("窗口展示"),
            QStringLiteral("启动展示模式准备完成"),
            QStringLiteral("frameless=%1, fullscreen(before)=%2")
                .arg(windowFlags().testFlag(Qt::FramelessWindowHint) ? QStringLiteral("true")
                                                                     : QStringLiteral("false"),
                     isFullScreen() ? QStringLiteral("true") : QStringLiteral("false")));

    if (!isFullScreen())
        QMainWindow::showFullScreen();

    raise();
    activateWindow();

    logStep(QStringLiteral("系统"),
            QStringLiteral("窗口展示"),
            QStringLiteral("启动展示模式已应用"),
            QStringLiteral("fullscreen(after)=%1, size=%2x%3")
                .arg(isFullScreen() ? QStringLiteral("true") : QStringLiteral("false"))
                .arg(width())
                .arg(height()));
}

void pa22x::toggleEscapePresentation()
{
    if (isMinimized() || windowState().testFlag(Qt::WindowMinimized)) {
        setWindowState(windowState() & ~Qt::WindowMinimized);
        if (windowFlags().testFlag(Qt::FramelessWindowHint) || m_startupPresentationApplied)
            QMainWindow::showFullScreen();
        else
            QMainWindow::showMaximized();

        raise();
        activateWindow();
        logStep(QStringLiteral("系统"),
                QStringLiteral("窗口展示"),
                QStringLiteral("ESC 恢复窗口"),
                QStringLiteral("state=restore, fullscreen=%1, size=%2x%3")
                    .arg(isFullScreen() ? QStringLiteral("true") : QStringLiteral("false"))
                    .arg(width())
                    .arg(height()));
        return;
    }

    showMinimized();
    logStep(QStringLiteral("系统"),
            QStringLiteral("窗口展示"),
            QStringLiteral("ESC 最小化窗口"),
            QStringLiteral("state=minimized"));
}

void pa22x::registerEscapeHotKey()
{
#ifdef Q_OS_WIN
    if (m_escapeHotKeyRegistered)
        return;

    const HWND hwnd = reinterpret_cast<HWND>(winId());
    if (!hwnd)
        return;

    if (!RegisterHotKey(hwnd, kEscapeHotKeyId, MOD_NOREPEAT, VK_ESCAPE)) {
        if (!RegisterHotKey(hwnd, kEscapeHotKeyId, 0, VK_ESCAPE)) {
            logStep(QStringLiteral("系统"),
                    QStringLiteral("窗口展示"),
                    QStringLiteral("ESC 热键注册失败"),
                    QStringLiteral("error=%1").arg(GetLastError()));
            return;
        }
    }

    m_escapeHotKeyRegistered = true;
    logStep(QStringLiteral("系统"),
            QStringLiteral("窗口展示"),
            QStringLiteral("ESC 热键已注册"),
            QStringLiteral("hotkey=ESC"));
#endif
}

void pa22x::unregisterEscapeHotKey()
{
#ifdef Q_OS_WIN
    if (!m_escapeHotKeyRegistered)
        return;

    const HWND hwnd = reinterpret_cast<HWND>(winId());
    if (hwnd)
        UnregisterHotKey(hwnd, kEscapeHotKeyId);

    m_escapeHotKeyRegistered = false;
    logStep(QStringLiteral("系统"),
            QStringLiteral("窗口展示"),
            QStringLiteral("ESC 热键已注销"),
            QStringLiteral("hotkey=ESC"));
#endif
}

void pa22x::showEvent(QShowEvent *event)
{
    QMainWindow::showEvent(event);

    registerEscapeHotKey();

    if (m_startupPresentationApplied)
        return;

    QTimer::singleShot(0, this, [this]() {
        applyStartupPresentation();
        if (isSimulationSelfTestMode() && hasInternalToolsAccess() && !m_simulationWorkflowRunning)
            on_simulateDetectButton_clicked();
    });
}

void pa22x::resizeEvent(QResizeEvent *event)
{
    QMainWindow::resizeEvent(event);
    QTimer::singleShot(0, this, [this]() {
        applyResponsiveLayout();
    });
}

#ifdef Q_OS_WIN
bool pa22x::nativeEvent(const QByteArray &eventType, void *message, long *result)
{
    Q_UNUSED(eventType);

    MSG* nativeMessage = static_cast<MSG*>(message);
    if (nativeMessage && nativeMessage->message == WM_HOTKEY
            && static_cast<int>(nativeMessage->wParam) == kEscapeHotKeyId) {
        toggleEscapePresentation();
        if (result)
            *result = 0;
        return true;
    }

    return QMainWindow::nativeEvent(eventType, message, result);
}
#endif
// 界面刷新而已
void pa22x::timerEvent(QTimerEvent *e)
{
    // 判断定时器事件是否为显示定时器
    if (e->timerId() == _displayTimer)
        display();  // 调用显示函数

#ifdef RUN_EVALUATOR
    // 如果定时器事件为评估器定时器
    if (e->timerId() == _evaluatorTimer) {
        QStringList info = _pa22xClient->getEvaluatorInfo();  // 获取评估器信息列表
        if (Logger::isVerboseEnabled()) {
            for (QString &str : info)
                qDebug() << str;  // 输出每条评估器信息到调试输出
        }
        _pa22xClient->clearEvaluatorInfo();  // 清空评估器信息列表
    }
#endif
}
//显示函数，更新图标的内容格式等
void pa22x::display()
{
    if (!_pa22xClient || !ui || !ui->PEAWaveView || !ui->PERulerVerView || !ui->PERulerHorView)
        return;

#ifdef DATA_TCP_SOCKET
    // 如果定义了DATA_TCP_SOCKET宏，则从TCP数据套接字读取数据，直到没有数据可读
    while (_pa22xClient->readFromTCPDataSocket() == true)
        /*什么也不做*/;
#endif

    UTShadowDevice* shadowDevice = _pa22xClient->getSelectedShadowDevice();  // 获取选定的影子设备
    if (!shadowDevice)
        return;
    QVector<QMap<QString, QString>>& parameters = shadowDevice->getParameters();
    if (parameters.isEmpty())
        return;
    QMap<QString, QString>& parameter = shadowDevice->getSelectedParameter();  // 获取选定参数映射
    shadowDevice->lock();  // 锁定影子设备

    // 获取当前选择的通道标识符并转换为索引
    QString identifier = shadowDevice->getSelectedChannel();
    int index = shadowDevice->identifierToIndex(identifier);
    if (index < 0 || index >= parameters.size()) {
        shadowDevice->unlock();
        return;
    }

    // 获取波形数据
    X22_Waveform* wave_form = shadowDevice->getAWaveData(index);
    if (!wave_form) {
        shadowDevice->unlock();
        return;
    }

    // 获取参数：声速和延时（以微秒为单位）
    int velocity = parameter["velocity"].toInt();
    double delay_us = parameter["delay"].toDouble();

    // 将延时转换为毫米
    double delay_mm = delayOffsetMM(delay_us, velocity);

    // 获取参数：扫描范围和每点的毫米数
    double range = parameter["range"].toDouble();
    double mmPerPt = (range / UTShadowDevice::_PEWaveLength);

    // 获取门控参数（a门）
    double gate_a_position = parameter["gate_a_position"].toDouble();
    double gate_a_width = parameter["gate_a_width"].toDouble();
    double gate_a_threshold = parameter["gate_a_threshold"].toDouble();
    int gate_a_position_px = gate_a_position / mmPerPt;
    int gate_a_width_px = gate_a_width / mmPerPt;
    int gate_a_threshold_px = 256 - gate_a_threshold * 2.55;

    // 获取门控参数（b门）
    double gate_b_position = parameter["gate_b_position"].toDouble();
    double gate_b_width = parameter["gate_b_width"].toDouble();
    double gate_b_threshold = parameter["gate_b_threshold"].toDouble();
    int gate_b_position_px = gate_b_position / mmPerPt;
    int gate_b_width_px = gate_b_width / mmPerPt;
    int gate_b_threshold_px = 256 - gate_b_threshold * 2.55;

    // 获取门控参数（c门）
    double gate_c_position = parameter["gate_c_position"].toDouble();
    double gate_c_width = parameter["gate_c_width"].toDouble();
    double gate_c_threshold = parameter["gate_c_threshold"].toDouble();
    int gate_c_position_px = gate_c_position / mmPerPt;
    int gate_c_width_px = gate_c_width / mmPerPt;
    int gate_c_threshold_px = 256 - gate_c_threshold * 2.55;

    // 获取门控参数（d门）
    double gate_d_position = parameter["gate_d_position"].toDouble();
    double gate_d_width = parameter["gate_d_width"].toDouble();
    double gate_d_threshold = parameter["gate_d_threshold"].toDouble();
    int gate_d_position_px = gate_d_position / mmPerPt;
    int gate_d_width_px = gate_d_width / mmPerPt;
    int gate_d_threshold_px = 256 - gate_d_threshold * 2.55;

    // 设置PE波形视图的数据和样式
    ui->PEAWaveView->setData(wave_form);
    ui->PEAWaveView->draw(kImageViewDrawHorizontal, "rf");

    m_displayedWaveSamples.clear();
    m_displayedWaveSamples.reserve(UTShadowDevice::_PEWaveLength);
    m_displayedWaveChannelIdentifier = identifier;
    if (wave_form) {
        for (int i = 0; i < UTShadowDevice::_PEWaveLength; ++i) {
            const int sample = wave_form->waveP[i] >= wave_form->waveN[i]
                    ? wave_form->waveP[i]
                    : -wave_form->waveN[i];
            m_displayedWaveSamples << sample;
        }
        m_lastDisplayedWaveCapturedAt = QDateTime::currentDateTime();
    } else {
        m_lastDisplayedWaveCapturedAt = QDateTime();
    }

    // 设置PE波形视图的门控参数（a、b、c、d门）
    ui->PEAWaveView->setGate(gate_a_position_px, gate_a_width_px, gate_a_threshold_px, "a");
    ui->PEAWaveView->setGate(gate_b_position_px, gate_b_width_px, gate_b_threshold_px, "b");
    ui->PEAWaveView->setGate(gate_c_position_px, gate_c_width_px, gate_c_threshold_px, "c");
    ui->PEAWaveView->setGate(gate_d_position_px, gate_d_width_px, gate_d_threshold_px, "d");


    // zhangshiwei20221104modify
    // 设置垂直尺标的锚点和范围
    ui->PERulerVerView->setAnchorPoint(Qt::AnchorLeft);
    ui->PERulerVerView->setRange(100, 0);

    // 设置水平尺标的锚点和范围
    ui->PERulerHorView->setAnchorPoint(Qt::AnchorBottom);
    ui->PERulerHorView->setRange(delay_mm, range + delay_mm);

    // 解锁设备
    shadowDevice->unlock();
}

//DeviceType pa22x::deviceTypeByIdentifier(QString identifier) {
//    // 正则表达式用于匹配设备标识符，例如 "PE2Device_123"
//    QRegularExpression re("(PE2|PE8|PA)Device_(\\d+)");
//    QRegularExpressionMatch match = re.match(identifier);
//    QString type;
//    QString deviceNumber;

//    // 如果匹配成功，提取设备类型和设备编号
//    if (match.hasMatch()) {
//        type = match.captured(1);  // 提取设备类型，如 "PE2", "PE8" 或 "PA"
//        deviceNumber = match.captured(2);  // 提取设备编号部分
//    }

//    // 根据设备类型返回相应的枚举值
//    if (!type.compare("PE2", Qt::CaseInsensitive) ||
//            !type.compare("PE8", Qt::CaseInsensitive))
//        return kDeviceTypePE;  // 如果是 "PE2" 或 "PE8" 类型的设备，返回 kDeviceTypePE
//    else if (!type.compare("PA", Qt::CaseInsensitive))
//        return kDeviceTypePA;  // 如果是 "PA" 类型的设备，返回 kDeviceTypePA

//    return kDeviceTypeUnknow;  // 如果无法识别设备类型，则返回 kDeviceTypeUnknow
//}
//连接
void pa22x::on_connectButton_clicked(bool checked)
{
    if (m_connectOperationInProgress) {
        ui->connectButton->setChecked(m_isconnect);
        return;
    }

    if (!checked) {
        logStep(QStringLiteral("连接"), QStringLiteral("CON-03"), QStringLiteral("断开请求"));
        disconnectClient(true);
        return;
    }

    const QString serverIp = ui->IPAddressInput->text().trimmed();
    logStep(QStringLiteral("连接"), QStringLiteral("CON-01"), QStringLiteral("连接请求"), serverIp);
    if (serverIp.isEmpty()) {
        qWarning() << "IP不能为空";
        if (ui->statusbar)
            ui->statusbar->showMessage(QStringLiteral("IP不能为空"), 4000);
        ui->connectButton->setChecked(false);
        return;
    }

    m_connectOperationInProgress = true;
    if (_pa22xClient) {
        _pa22xClient->disconnectFromServer();
        _pa22xClient->deleteLater();
        _pa22xClient = nullptr;
    }
    ui->connectButton->setEnabled(false);
    ui->connectButton->setText(QStringLiteral("连接中..."));
    const bool refreshWasRunning = suspendDisplayRefresh();
    pumpUiEvents(20);
    QApplication::setOverrideCursor(Qt::BusyCursor);

    auto finishConnectAttempt = [this, refreshWasRunning](bool connected, const QString& message) {
        QApplication::restoreOverrideCursor();
        m_connectOperationInProgress = false;
        ui->connectButton->setEnabled(true);
        if (!connected) {
            ui->connectButton->setText(QStringLiteral("连接"));
            ui->connectButton->setChecked(false);
        }
        resumeDisplayRefresh(refreshWasRunning);
        if (ui->statusbar && !message.isEmpty())
            ui->statusbar->showMessage(message, 5000);
    };

    PA22XClient* client = nullptr;
    try {
        client = new PA22XClient(this);
        if (!client->connectToServer(serverIp)) {
            const QString detail = client->lastErrorString();
            const QString msg = detail.isEmpty()
                    ? QStringLiteral("连接服务器失败")
                    : QStringLiteral("连接服务器失败: %1").arg(detail);
            qWarning() << msg;
            client->deleteLater();
            finishConnectAttempt(false, msg);
            return;
        }
    } catch (...) {
        qWarning() << "连接服务器时发生异常";
        if (client) {
            client->deleteLater();
            client = nullptr;
        }
        finishConnectAttempt(false, QStringLiteral("连接服务器时发生异常，请检查设备返回的同步信息"));
        return;
    }

    if (!attachClient(client, false)) {
        qWarning() << "未发现可用设备";
        finishConnectAttempt(false, QStringLiteral("未发现可用设备"));
        return;
    }
    logStep(QStringLiteral("连接"), QStringLiteral("CON-02"), QStringLiteral("连接成功"), serverIp);
    finishConnectAttempt(true, QStringLiteral("连接成功"));
}

void pa22x::on_simulateDetectButton_clicked()
{
    if (!hasInternalToolsAccess() && !m_internalToolsUnlocked) {
        qWarning() << "忽略未解锁的模拟检测入口";
        return;
    }

    if (m_simulationWorkflowRunning)
        return;

    logStep(QStringLiteral("模拟"), QStringLiteral("SIM-01"), QStringLiteral("内部模拟检测启动"));
    MockPA22XClient* client = new MockPA22XClient(this);
    if (!client->connectToServer(QStringLiteral("mock"))) {
        client->deleteLater();
        if (isSimulationSelfTestMode()) {
            writeSimulationSelfTestReport(QStringLiteral("FAIL"),
                                          QStringList() << QStringLiteral("模拟客户端初始化失败"));
            QTimer::singleShot(0, QCoreApplication::instance(), SLOT(quit()));
        } else {
            QMessageBox::critical(this, "模拟检测失败", "模拟客户端初始化失败");
        }
        return;
    }

    if (!attachClient(client, true)) {
        if (isSimulationSelfTestMode()) {
            writeSimulationSelfTestReport(QStringLiteral("FAIL"),
                                          QStringList() << QStringLiteral("模拟设备初始化失败"));
            QTimer::singleShot(0, QCoreApplication::instance(), SLOT(quit()));
        } else {
            QMessageBox::critical(this, "模拟检测失败", "模拟设备初始化失败");
        }
        return;
    }

    runSimulationWorkflow();
}


//保存参数
void pa22x::on_PESaveParameters_clicked()
{
    logStep(QStringLiteral("参数"), QStringLiteral("PAR-01"), QStringLiteral("保存参数请求"));
    if(ui->connectButton->text() == "连接")
    {
        QMessageBox::critical(this, "保存参数错误", "未连接");
        qWarning() << "保存参数错误,未连接";
        ui->WaSaveWave->setChecked(0);
        return;
    }

    UTShadowDevice* device = _pa22xClient->getSelectedShadowDevice();
    // 检查设备是否已选定
    if (!device) {
        QMessageBox::warning(this, "错误", "未接入数据");
        qWarning() << "未接入数据";
        return;
    }

    // 检查信号是否已接入
    if (device->getSelectedChannel().isEmpty()) {
        QMessageBox::warning(this, "错误", "未接入数据");
        qWarning() << "未接入数据";
        return;
    }

    const QString entry = buildCurrentParameterEntry(device);
    if (entry.isEmpty()) {
        QMessageBox::warning(this, "错误", "当前通道参数不可用");
        return;
    }

    QString currentTimeString = QDateTime::currentDateTime()
            .toString("yyyyMMdd_hhmmss");

    QString defaultFileName = "Parameters_" + currentTimeString + ".txt";
    const bool suppressDialogs = m_simulationWorkflowRunning || isSimulationSelfTestMode();

    QString fileName = QFileDialog::getSaveFileName(this,
                                                    "Save Channel Parameters",
                                                    fileDialogPath(defaultFileName),
                                                    "Text Files (*.txt);;All Files (*)",
                                                    nullptr,
                                                    QFileDialog::Option::DontUseNativeDialog);
    if (!fileName.isEmpty()) {
        QSaveFile file(fileName);
        if (file.open(QIODevice::WriteOnly|QIODevice::Text)) {
            QTextStream stream(&file);
            stream.setCodec("UTF-8");
            stream << entry;
            const bool committed = file.commit();
            if (!committed) {
                QMessageBox::critical(this,
                                      QStringLiteral("保存参数失败"),
                                      QStringLiteral("参数文件提交失败：%1 | %2")
                                          .arg(nativePath(fileName), file.errorString()));
                return;
            }
            rememberFileDialogPath(fileName);
            logStep(QStringLiteral("参数"), QStringLiteral("PAR-02"), QStringLiteral("保存参数完成"), fileName);
            if (!suppressDialogs) {
                QMessageBox::information(this,
                                         QStringLiteral("参数已保存"),
                                         runtimePathSummary(fileName));
            }
        } else if (!suppressDialogs) {
            QMessageBox::critical(this,
                                  QStringLiteral("保存参数失败"),
                                  QStringLiteral("参数文件无法写入：%1 | %2")
                                      .arg(nativePath(fileName), file.errorString()));
        }
    }
    saveUiSettingsToDisk();

}

// 根据标识符获取通道号的函数
int channelNumberFromIdentifier(QString identifier)
{
    int channelNumber = -1;  // 默认通道号为-1，表示未找到有效的通道号

    // 使用正则表达式匹配标识符中的数字部分
    QRegularExpression re("[0-9]+");
    QRegularExpressionMatch match = re.match(identifier);
    // 如果找到匹配项
    if (match.hasMatch())
        channelNumber = match.captured(0).toInt();  // 将匹配到的数字部分转换为整数作为通道号

    return channelNumber;  // 返回解析得到的通道号
}


//加载参数
void pa22x::on_PELoadParameters_clicked()
{
    logStep(QStringLiteral("参数"), QStringLiteral("PAR-03"), QStringLiteral("加载参数请求"));

    if(ui->connectButton->text() == "连接")
    {
        qWarning() << "加载参数错误，未连接";
        QMessageBox::critical(this, "加载参数错误", "未连接");
        return;
    }

    //获取通道索引和设备对象
    int channelIndex = 0;
    UTShadowDevice* device = _pa22xClient ? _pa22xClient->getSelectedShadowDevice() : nullptr;
    if (!device) {
        QMessageBox::critical(this, "加载参数错误", "未检测到设备");
        return;
    }

    QString parameterString;
    QString fileName = QFileDialog::getOpenFileName(this,
                                                    "Open Channel Parameters",
                                                    fileDialogPath(),
                                                    "Text Files (*.txt);;All Files (*)",
                                                    nullptr,
                                                    QFileDialog::Option::DontUseNativeDialog);
    if (fileName.isEmpty())
        return;

    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly|QIODevice::Text))
        return;

    parameterString = QString::fromUtf8(file.readAll());

    const bool wasSampling = is_start != 0;
    const bool hadDisplayTimer = _displayTimer > 0;
    const bool hadEvaluatorTimer = _evaluatorTimer > 0;
    if (_displayTimer > 0) {
        killTimer(_displayTimer);
        _displayTimer = 0;
    }
    if (_evaluatorTimer > 0) {
        killTimer(_evaluatorTimer);
        _evaluatorTimer = 0;
    }

    if (wasSampling)
        device->WaveDataClose();

    bool applyOk = false;
    QString applyError;
    if (_pa22xClient)
        _pa22xClient->sendCommandSync(QStringLiteral("set data_stop"));

    applyOk = applyParameterText(device, parameterString, &channelIndex);
    if (!applyOk)
        applyError = QStringLiteral("参数文本解析或下发失败");

    if (_pa22xClient)
        _pa22xClient->sendCommandSync(QStringLiteral("set data_start"));

    if (hadDisplayTimer && _displayTimer <= 0)
        _displayTimer = startTimer(kDisplayRefreshIntervalMs);
    if (hadEvaluatorTimer && _evaluatorTimer <= 0)
        _evaluatorTimer = startTimer(1000);
    if (wasSampling) {
        device->logWaveData();
        is_start = 1;
        refreshWaveCaptureButtonState();
    }

    if (!applyOk) {
        QMessageBox::critical(this, "加载参数失败", applyError);
        return;
    }

    enforceLockedPeChannelCount(device, false);

    //激活通道并更新界面
    rememberFileDialogPath(fileName);
    if (ui->PEChannelSelection->count() > 0) {
        channelIndex = qBound(0, channelIndex, ui->PEChannelSelection->count() - 1);
        PEChannelActived(channelIndex);
    }
    markBackgroundDirty(QStringLiteral("参数加载后请重新确认波形并保存背景信号"));
    logStep(QStringLiteral("参数"),
            QStringLiteral("PAR-04"),
            QStringLiteral("加载参数完成"),
            QStringLiteral("%1 | 通道=%2").arg(fileName).arg(channelIndex + 1));

     loadUiSettingsFromDisk();
     clearStaleDetectionOutputFiles(false);
}
//启动
void pa22x::on_PESaveWave_clicked()
{
    if (is_start != 0) {
        on_CloseWaveSave_clicked();
        return;
    }

    const bool suppressDialogs = m_simulationWorkflowRunning || isSimulationSelfTestMode();
    logStep(QStringLiteral("采集"), QStringLiteral("CAP-01"), QStringLiteral("启动采集请求"));
    if(ui->connectButton->text() == "连接")
    {
        if (!suppressDialogs)
            QMessageBox::critical(this, "启动错误", "未连接");
        qWarning() << "启动错误";

        return;
    }

    if (_pa22xClient) {
            UTShadowDevice* device = _pa22xClient->getSelectedShadowDevice();
            if (device) {
                device->logWaveData();
                is_start = 1;
                refreshWaveCaptureButtonState();
                if (!suppressDialogs)
                    QMessageBox::information(nullptr, "启动成功", "启动成功");
                logStep(QStringLiteral("采集"), QStringLiteral("CAP-02"), QStringLiteral("启动采集成功"));

                return;
            } else {
                if (!suppressDialogs)
                    QMessageBox::critical(this, "启动失败", "未检测到数据");
                qWarning() << "启动失败,未检测到数据";
                return;
            }
        } else {
            if (!suppressDialogs)
                QMessageBox::critical(this, "启动失败", "未检测到数据。");
            qWarning() << "启动失败,未检测到数据";
            return;
        }

}
//"停止"按钮，关闭时钟
void pa22x::on_CloseWaveSave_clicked()
{
    logStep(QStringLiteral("采集"), QStringLiteral("CAP-03"), QStringLiteral("停止采集请求"));

    if (ui->connectButton->text() == "连接") {
        QMessageBox::critical(this, QStringLiteral("停止采集失败"), QStringLiteral("未连接"));
        qWarning() << "停止采集失败,未连接";
        return;
    }

    if (!_pa22xClient) {
        QMessageBox::critical(this, QStringLiteral("停止采集失败"), QStringLiteral("未检测到数据"));
        qWarning() << "停止采集失败,未检测到数据";
        return;
    }

    UTShadowDevice* device = _pa22xClient->getSelectedShadowDevice();
    if (!device) {
        QMessageBox::critical(this, QStringLiteral("停止采集失败"), QStringLiteral("未检测到可用设备"));
        qWarning() << "停止采集失败,未检测到可用设备";
        return;
    }

    device->WaveDataClose();
    is_start = 0;
    refreshWaveCaptureButtonState();
    QMessageBox::information(this, QStringLiteral("停止采集"), QStringLiteral("已停止当前波形采集"));
    logStep(QStringLiteral("采集"), QStringLiteral("CAP-04"), QStringLiteral("停止采集完成"));

}

//检测功能
void pa22x::onTimerCheck()
{
    // 防止重入：上一轮检测尚未完成时跳过本轮
    if (m_analysisRunning.exchange(true, std::memory_order_acquire)) {
        logStep(QStringLiteral("检测"),
                QStringLiteral("DET-00"),
                QStringLiteral("跳过本轮检测"),
                QStringLiteral("上一轮检测仍在执行中"));
        return;
    }

    // 在主线程捕获所有 UI 参数，避免后台线程访问 QWidget
    const double captured_width_mm = ui->dspb_width->value();
    const double captured_specfmin = ui->spb_specfmin->value();
    const double captured_specfmax = ui->spb_specfmax->value();
    const int captured_avecoun = ui->sb_avenum->value();
    const int captured_interval = ui->spb_time->value();

    ///////////////////////////背景信号/////////////////////////////////
    QString appDir = QCoreApplication::applicationDirPath();
    //子线程异步
    //背景介质水信号
    QPointer<pa22x> self(this);
    QtConcurrent::run([self, appDir, this, captured_width_mm, captured_specfmin,
                       captured_specfmax, captured_avecoun, captured_interval]() {
        if (!self)
            return;

        // RAII guard: 确保m_analysisRunning在任何退出路径都被重置
        struct AnalysisGuard {
            std::atomic_bool& flag;
            ~AnalysisGuard() { flag.store(false, std::memory_order_release); }
        } guard{m_analysisRunning};

        QString filePath = appDir + "/boxing_water.txt";

        const QString cycleId = nextDetectionCycleId(QDateTime::currentDateTime());
        logStepWithCycle(QStringLiteral("检测"),
                         QStringLiteral("DET-03"),
                         QStringLiteral("单轮检测开始"),
                         QStringLiteral("time=%1 | width_mm=%2 | specfmin=%3 | specfmax=%4 | interval_s=%5 | ave_window=%6 | coun2_before=%7 | bg=%8 | %9")
                             .arg(m_currentTime.toString(QStringLiteral("yyyy-MM-dd HH:mm:ss")))
                             .arg(formatNumericForLog(ui->dspb_width->value(), 3))
                             .arg(formatNumericForLog(ui->spb_specfmin->value(), 3))
                             .arg(formatNumericForLog(ui->spb_specfmax->value(), 3))
                             .arg(formatNumericForLog(ui->spb_time->value(), 3))
                             .arg(ui->sb_avenum->value())
                             .arg(coun2)
                             .arg(nativePath(filePath))
                             .arg(summarizeRawArrayForExcel(QStringLiteral("Mywave"), Mywave, 448, 3, 4)),
                         cycleId);

        QFile wfile(filePath);
        QVector<double> water;
        if(wfile.open(QIODevice::ReadOnly | QIODevice::Text)) {
            while(!wfile.atEnd())
            {
                QString lineString_y = QString(wfile.readLine()).trimmed();
                water << lineString_y.toDouble();
            }
            wfile.close();
        }

        //补充水信号为2^N
        int das = 1, N = 1;
        for(int i = 0; i < 12 ; i++)
        {
            das = qPow(2,i);
            if( das >= water.size() )
            {
                N = das;
                break;
            }
        }

        QVector<double>ywater;

        for (int i = 0; i < N; i++)
        {
            if(i < water.size())
                ywater << water[i];
            else
                ywater << 0;
        }

        //设置横坐标
        QVector<double>x(ywater.size());
        for(int i = 0; i < ywater.size(); i++)
        {
            x[i] = i;
        }

        //计算背景介质水的幅频曲线
        //fft类的使用
        QVector<Complex> datain(x.size());
        QVector<Complex> dataout(x.size());
        for(auto i = 0; i < x.size();i++)
        {
            datain[i].rl = ywater[i];
        }

        //调用接口 生成频域的 dataout 数据
        QScopedPointer<fft> mfft(new fft());
        mfft->fft1(datain, datain.size(), dataout);

        fre.clear();
        amp.clear();

        //计算频率分辨率
        double samfs = 100; //采样率,单位MHz
        disfre = samfs / ywater.size(); //幅频曲线横坐标

        for(auto i = 0;i < dataout.size() / 8;i++)
        {
            fre << i * disfre;
            amp << sqrt(dataout[i].rl * dataout[i].rl + dataout[i].im * dataout[i].im);
        }


        //////////////////////////石灰石信号/////////////////////////////////
        QVector<double>stone;
        bool gotLiveWave = false;
        if (_pa22xClient) {
            UTShadowDevice* waveDevice = _pa22xClient->getSelectedShadowDevice();
            if (waveDevice) {
                waveDevice->lock();
                QString selCh = waveDevice->getSelectedChannel();
                int chIdx = waveDevice->identifierToIndex(selCh);
                if (chIdx < 0) chIdx = 0;
                X22_Waveform* wf = waveDevice->getAWaveData(chIdx);
                if (wf) {
                    for (int i = 0; i < 448; i++) {
                        int v = (wf->waveP[i] >= wf->waveN[i]) ? wf->waveP[i] : -wf->waveN[i];
                        stone << v;
                        if (v != 0) gotLiveWave = true;
                    }
                }
                waveDevice->unlock();
            }
        }
        if (!gotLiveWave) {
            stone.clear();
            for (int i = 0; i < 448; i ++) {
                stone << Mywave[i];
            }
        }
        int nonZeroCount = 0;
        double peakAbs = 0.0;
        double waveEnergySum = 0.0;
        double stoneMin = 0.0;
        double stoneMax = 0.0;
        bool stoneMinMaxInit = false;
        for (double value : stone) {
            if (!qFuzzyIsNull(value)) {
                ++nonZeroCount;
                peakAbs = qMax(peakAbs, std::abs(value));
                if (!stoneMinMaxInit) {
                    stoneMin = stoneMax = value;
                    stoneMinMaxInit = true;
                } else {
                    if (value < stoneMin) stoneMin = value;
                    if (value > stoneMax) stoneMax = value;
                }
            }
            waveEnergySum += value * value;
        }
        const double waveRms = stone.isEmpty() ? 0.0 : std::sqrt(waveEnergySum / stone.size());
        const double dispMin = (128.0 + stoneMin / 2.0) * 100.0 / 255.0;
        const double dispMax = (128.0 + stoneMax / 2.0) * 100.0 / 255.0;
        QMetaObject::invokeMethod(this, [self, stoneMin, stoneMax, peakAbs, dispMin, dispMax]() {
            if (self && self->ui && self->ui->statusbar) {
                self->ui->statusbar->showMessage(
                    QStringLiteral("波形: min=%1(%3%) max=%2(%4%) peak=%5")
                        .arg(QString::number(stoneMin, 'f', 0))
                        .arg(QString::number(stoneMax, 'f', 0))
                        .arg(QString::number(dispMin, 'f', 0))
                        .arg(QString::number(dispMax, 'f', 0))
                        .arg(QString::number(peakAbs, 'f', 0)),
                    5000);
            }
        }, Qt::QueuedConnection);

        //补充石灰石信号为2^N
        int das2 = 1, N2 = 1;
        for(int i = 0; i < 12 ; i++)
        {
            das2 = qPow(2,i);
            if( das2 >= stone.size() )
            {
                N2 = das2;
                break;
            }
        }

        QVector<double> ystone;
        for (int i = 0; i < N2; i++)
        {
            if(i < stone.size())
                ystone << stone[i];
            else
                ystone << 0;
        }


        //设置横坐标
        QVector<double>x2(ystone.size());
        for(int i = 0; i < ystone.size(); i++)
        {
            x2[i] = i;
        }

        //fft类的使用
        QScopedPointer<fft> afft(new fft());

        QVector<Complex> datain2(x2.size());
        QVector<Complex> dataout2(x2.size());

        for(auto i = 0; i< x2.size();i++)
        {
            datain2[i].rl = ystone[i];
        }

        //调用接口 生成频域的 dataout 数据
        afft->fft1(datain2,datain2.size(),dataout2);

        fre2.clear();
        amp2.clear();

        for(auto i = 0;i < dataout2.size() / 8;i++)
        {
            fre2 << i * disfre;
            amp2 << sqrt(dataout2[i].rl * dataout2[i].rl + dataout2[i].im * dataout2[i].im);
        }


        ////////////////////////计算实验衰减谱/////////////////////////////

        QVector<double> spectrum(amp.size());
        double specfmin,specfmax,width;
        width = ui->dspb_width->value() * 0.001;  //获取样品池宽度,单位m
        specfmax = ui->spb_specfmax->value();  //衰减谱频率最大值
        specfmin = ui->spb_specfmin->value();  //衰减谱频率最小值

        const double minWidth = 1e-6; // 最小有效宽度1微米
        const double safeWidth = (width > minWidth) ? width : minWidth;
        for(int i = 0; i < amp.size(); i++)
        {
            const double ratio = (amp2[i] > 1e-30) ? (amp[i] / amp2[i]) : 1.0;
            const double safeRatio = (ratio > 1e-30) ? ratio : 1e-30;
            spectrum[i]= log(safeRatio) / safeWidth;
        }

        for(int i = 0; i < spectrum.size(); i++)
        {
            if (fre[i] > specfmin)
            {
                posl = i;
                break;
            }
        }

        for(int i = 0; i < spectrum.size(); i++)
        {
            if (fre[i] > specfmax)
            {
                posh = i;
                break;
            }
        }

        ///////////////////////截取有效超声衰减谱///////////////////////////
        //zhangshiwei20230208modify
        myfre.clear();
        myspectrum.clear();
        for(int i = posl; i < posh; i++)
        {
            myfre << fre[i];
            myspectrum  << spectrum[i];
        }


        ////////////////////////衰减谱的实时显示///////////////////////////
        //求衰减系数的最大最小值
        double minsp = 500,maxsp = 0;
        for(int i = posl; i < posh; i++)
        {
            if (maxsp < spectrum[i])
            {
                maxsp = spectrum[i];
            }
        }

        for(int i = posl; i < posh; i++)
        {
            if (minsp > spectrum[i])
            {
                minsp = spectrum[i];
            }
        }

        //在图表中显示衰减谱
        emit updatePlotData(specfmin,specfmax,minsp ,maxsp);


        //保存衰减谱
        QString logFilePath = appDir + "/1.txt";  // 假设文件与exe同目录
        QFile file(logFilePath);
        if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
            emit error("无法打开文件进行写入：" + logFilePath);
            return;
        }

        QTextStream out(&file);
        for (int i = 0; i < myspectrum.size(); ++i) {
            out << myfre[i] << "  " << myspectrum[i];
            if (i < myspectrum.size() - 1) {
                out << endl;  // 自动处理平台相关换行
            }
        }

        file.close();
        logStepWithCycle(QStringLiteral("检测"),
                         QStringLiteral("DET-04"),
                         QStringLiteral("频谱计算关键变量"),
                         QStringLiteral("water_n=%1 | water_pad=%2 | stone_n=%3 | stone_pad=%4 | samfs=%5MHz | disfre=%6 | width_m=%7 | posl=%8 | posh=%9 | minsp=%10 | maxsp=%11 | spectrum_file=%12 | %13 | %14 | %15 | %16")
                             .arg(water.size())
                             .arg(N)
                             .arg(stone.size())
                             .arg(N2)
                             .arg(formatNumericForLog(samfs, 3))
                             .arg(formatNumericForLog(disfre, 6))
                             .arg(formatNumericForLog(width, 6))
                             .arg(posl)
                             .arg(posh)
                             .arg(formatNumericForLog(minsp, 3))
                             .arg(formatNumericForLog(maxsp, 3))
                             .arg(nativePath(logFilePath))
                             .arg(summarizeSeriesForExcel(QStringLiteral("water"), water, 3, 4))
                             .arg(summarizeSeriesForExcel(QStringLiteral("stone"), stone, 3, 4))
                             .arg(summarizeSpectrumForExcel(myfre, myspectrum))
                             .arg(summarizeSeriesForExcel(QStringLiteral("amp_ratio_spectrum"), spectrum, 3, 4)),
                         cycleId);

        //实验超声衰减谱的有效性判断 不同频率换能器 zhangsw20230414
        //myfre最大值：
        double myf2ma = *(std::max_element(std::begin(myfre), std::end(myfre)));
        //myfre最小值
        double myf2mi = *(std::min_element(std::begin(myfre), std::end(myfre)));

        int counlen = 2 * (floor(myf2ma) - ceil(myf2mi) + 1) - 1; //谱有效性判断数据长度
        QVector<double> effre2(counlen); //频率值
        QVector<int> cout2(counlen);  //大于该频率值对应的索引

        for(int i = 0; i < counlen; i++)
        {
            if (i == 0){
                effre2[i] = ceil(myf2mi);
            }
            else{
                effre2[i] = ceil(myf2mi) + i * 0.5;
            }
        }

        for(int j = 0; j < counlen; j++)
        {
            for(int i = 0; i < myspectrum.size(); i++)
            {
                if (myfre[i] > effre2[j])
                {
                    cout2[j] = i;
                    break;
                }
            }
        }

        int pos = 0;
        for(int k = 0; k < counlen - 1; k++) {
            int pos1 = cout2[k];
            int pos2 = cout2[k + 1];
            if (myspectrum[pos1] < myspectrum[pos2]){
                pos = pos + 1;
            }
        }

        const int monotonicComparisons = qMax(1, counlen - 1);
        const double monotonicRatio = static_cast<double>(pos) / static_cast<double>(monotonicComparisons);
        // 单调性满足0.50即可，netSlopePositive放宽容忍尾端噪声
        const bool relaxedMonotonic = (monotonicRatio >= kMonotonicMinRatio);

        specflag = (counlen >= 2) && ((pos == counlen - 1) || relaxedMonotonic);


        //////////////////////////判断是否调CVI反演程序///////////////////////////
        // 20230307zhangshiweimodify
        double d21 = 0.0, d32 = 0.0, d43 = 0.0, d50 = 0.0, d90 = 0.0, passrate = 0.0;
        typedef struct _CurveData
        {
            QVector<double> r;//粒径范围
            QVector<double> freqPsd;//频度分布
            QVector<double> accuPsd;//累积分布
            QVector<double> d21;
            QVector<double> d32;
            QVector<double> d43;
            QVector<double> d50;
            QVector<double> d90;
            QVector<double> dv90;
            QVector<double> pass;

        }CurveData;//单个曲线的数据
        CurveData psdDisplay; //时域图数据

        //计算实验衰减系数平均值  zhangshiwei20230724
        double sumspec = std::accumulate(std::begin(myspectrum),std::end(myspectrum),0.0);
        double Avespec = myspectrum.isEmpty() ? 0.0 : (sumspec / myspectrum.size());
        if (Avespec <= kAveSpecThreshold) {
            avespecflag = true;
        }
        else {
            avespecflag = false;
        }

        double minSpec = 0.0;
        double maxSpec = 0.0;
        double medianSpec = 0.0;
        double strongPositiveRatio = 0.0;
        double spectrumContrast = 0.0;
        if (!myspectrum.isEmpty()) {
            const auto minmaxSpec = std::minmax_element(std::begin(myspectrum), std::end(myspectrum));
            minSpec = *minmaxSpec.first;
            maxSpec = *minmaxSpec.second;

            QVector<double> sortedSpec = myspectrum;
            std::sort(sortedSpec.begin(), sortedSpec.end());
            const int specCount = sortedSpec.size();
            if (specCount % 2 == 0) {
                medianSpec = 0.5 * (sortedSpec[specCount / 2 - 1] + sortedSpec[specCount / 2]);
            }
            else {
                medianSpec = sortedSpec[specCount / 2];
            }

            int strongPositiveCount = 0;
            for (double value : myspectrum) {
                if (value > 1.0)
                    ++strongPositiveCount;
            }
            strongPositiveRatio = static_cast<double>(strongPositiveCount) / static_cast<double>(myspectrum.size());
            spectrumContrast = (maxSpec - medianSpec) / qMax(1.0, qAbs(maxSpec));
        }
        const bool waterLikeSpectrum = (peakAbs > kWaterLikePeakAbs);
        const bool airLikeSpectrum = (peakAbs < kAirLikePeakAbsMin)
                                    || (medianSpec <= 0.0 && spectrumContrast < kSpectrumContrastLo);
        const bool spectrumQualityValid = specflag == true
                && avespecflag == true
                && !waterLikeSpectrum
                && !airLikeSpectrum
                && peakAbs >= 30.0;
        logStepWithCycle(QStringLiteral("检测"),
                         QStringLiteral("DET-06"),
                         QStringLiteral("谱有效性判断"),
                          QStringLiteral("specflag=%1 | avespecflag=%2 | points=%3 | myf2mi=%4 | myf2ma=%5 | counlen=%6 | pos=%7 | monotonic_ratio=%8 | net_slope_positive=%9 | sumspec=%10 | aveSpec=%11 | %12 | %13 | water=%14 | air=%15 | peakAbs=%16 | waveRms=%17")
                              .arg(specflag ? QStringLiteral("true") : QStringLiteral("false"))
                              .arg(avespecflag ? QStringLiteral("true") : QStringLiteral("false"))
                              .arg(myspectrum.size())
                              .arg(formatNumericForLog(myf2mi, 3))
                              .arg(formatNumericForLog(myf2ma, 3))
                              .arg(counlen)
                              .arg(pos)
                              .arg(formatNumericForLog(monotonicRatio, 3))
                              .arg(monotonicRatio >= 0.50 ? QStringLiteral("true") : QStringLiteral("false"))
                              .arg(formatNumericForLog(sumspec, 3))
                              .arg(formatNumericForLog(Avespec, 3))
                              .arg(summarizeSeriesForExcel(QStringLiteral("effre2"), effre2, 3, 4))
                              .arg(summarizeSeriesForExcel(QStringLiteral("cout2"), cout2, 4))
                              .arg(waterLikeSpectrum ? QStringLiteral("true") : QStringLiteral("false"))
                              .arg(airLikeSpectrum ? QStringLiteral("true") : QStringLiteral("false"))
                              .arg(formatNumericForLog(peakAbs, 3))
                              .arg(formatNumericForLog(waveRms, 3)),
                         cycleId);

        //实验衰减谱有效
        if (spectrumQualityValid) {

             //满足条件调用CVI反演程序
            //直接使用inversion类
            QScopedPointer<inversion> inver(new inversion(appDir));
            inver->check(); // 离开作用域自动释放


            //显示粒度分布特征值
            QFile dfile(appDir + "\\2.txt");
            if(!dfile.open(QIODevice::ReadOnly | QIODevice::Text))
            {
                emit error("无法打开文件进行写入：" + appDir + "\\2.txt");
                return;
            }

            QTextStream ind(&dfile);

            while(!ind.atEnd())
            {
                QString line = ind.readLine();
                double parsedD21 = 0.0;
                double parsedD32 = 0.0;
                double parsedD43 = 0.0;
                double parsedD50 = 0.0;
                double parsedD90 = 0.0;
                if (!parseInversionSummaryLine(line,
                                               &parsedD21,
                                               &parsedD32,
                                               &parsedD43,
                                               &parsedD50,
                                               &parsedD90)) {
                    continue;
                }

                psdDisplay.d21.push_back(parsedD21);
                psdDisplay.d32.push_back(parsedD32);
                psdDisplay.d43.push_back(parsedD43);
                psdDisplay.d50.push_back(parsedD50);
                psdDisplay.d90.push_back(parsedD90);
            }
            dfile.close();
            if (psdDisplay.d21.isEmpty() || psdDisplay.d32.isEmpty()
                    || psdDisplay.d43.isEmpty() || psdDisplay.d50.isEmpty()
                    || psdDisplay.d90.isEmpty()) {
                d21 = 0.0;
                d32 = 0.0;
                d43 = 0.0;
                d50 = 0.0;
                d90 = 0.0;
            } else {
                d21 = psdDisplay.d21.last() * 1e6;
                d32 = psdDisplay.d32.last() * 1e6;
                d43 = psdDisplay.d43.last() * 1e6;
                d50 = psdDisplay.d50.last() * 1e6;
                d90 = psdDisplay.d90.last() * 1e6;
            }

            //读取CVI反演结果-粒径分布
            QFile file(appDir + "\\3.txt");
            if(!file.open(QIODevice::ReadOnly | QIODevice::Text))
            {
                emit error("无法打开文件进行写入：" + appDir + "\\3.txt");
                return;
            }

            QTextStream in(&file);

            while(!in.atEnd())
            {
                QString line = in.readLine();
                QStringList list = line.split("  ");
                if (list.size() < 3)
                    continue;

                psdDisplay.r.push_back(list.at(0).toDouble()); //粒径范围
                psdDisplay.freqPsd.push_back(list.at(1).toDouble()); //频度分布
                psdDisplay.accuPsd.push_back(list.at(2).toDouble()); //累积分布
            }
            file.close();


            // 45微米筛通过率(%) - 从体积累积分布插值
            int len2 = psdDisplay.r.size();
            passrate = 0.00;
            const double rSieve = 4.5e-05;

            for(int i = 0; i < len2; i++)
            {
                if (psdDisplay.r[i] >= rSieve)
                {
                    if (i == 0) {
                        passrate = psdDisplay.accuPsd[i] * 100.0;
                    } else {
                        double rLo = psdDisplay.r[i - 1];
                        double rHi = psdDisplay.r[i];
                        double accuLo = psdDisplay.accuPsd[i - 1];
                        double accuHi = psdDisplay.accuPsd[i];
                        double frac = (rSieve - rLo) / (rHi - rLo);
                        passrate = (accuLo + frac * (accuHi - accuLo)) * 100.0;
                    }
                    break;
                }
            }
            if (passrate <= 0.0 && len2 > 0) {
                if (psdDisplay.r.last() < rSieve) {
                    passrate = 100.0;
                } else {
                    passrate = psdDisplay.accuPsd.last() * 100.0;
                }
            }
            passrate = qBound(0.0, passrate, 100.0);
            logStepWithCycle(QStringLiteral("检测"),
                             QStringLiteral("DET-07"),
                             QStringLiteral("反演结果完成"),
                             QStringLiteral("rows2=%1 | rows3=%2 | d21=%3 | d32=%4 | d43=%5 | d50=%6 | d90=%7 | pass=%8 | %9 | %10 | %11")
                                 .arg(psdDisplay.d50.size())
                                 .arg(psdDisplay.r.size())
                                 .arg(formatNumericForLog(d21, 3))
                                 .arg(formatNumericForLog(d32, 3))
                                 .arg(formatNumericForLog(d43, 3))
                                 .arg(formatNumericForLog(d50, 3))
                                 .arg(formatNumericForLog(d90, 3))
                                 .arg(formatNumericForLog(passrate, 3))
                                 .arg(summarizeSeriesForExcel(QStringLiteral("result_d50"), psdDisplay.d50, 6, 3))
                                 .arg(summarizeSeriesForExcel(QStringLiteral("result_r"), psdDisplay.r, 6, 3))
                                 .arg(summarizeSeriesForExcel(QStringLiteral("result_accuPsd"), psdDisplay.accuPsd, 6, 3)),
                             cycleId);
            coun2 = 0;
        }
        //spectrum invalid: clear output directly
        else {
            ++coun2;
            d21 = 0.00;
            d32 = 0.00;
            d43 = 0.00;
            d50 = 0.00;
            d90 = 0.00;
            passrate = 0.00;
            logStepWithCycle(QStringLiteral("结束检测"),
                             QStringLiteral("DET-07"),
                             QStringLiteral("粒度结果置空"),
                             QStringLiteral("spec=%1 | ave=%2 | coun2=%3")
                                 .arg(specflag ? QStringLiteral("true") : QStringLiteral("false"))
                                 .arg(avespecflag ? QStringLiteral("true") : QStringLiteral("false"))
                                 .arg(coun2),
                             cycleId);
        }

        /////////////////////////////////计算结果显示及保存////////////////////////////

        // 确保 Distribution 目录存在
        const QString distDir = appDir + "/Distribution";
        QDir().mkpath(distDir);

        // 使用实时时钟，避免 m_currentTime 累计漂移
        const QDateTime now = QDateTime::currentDateTime();
        const QString todayStr = now.toString("yyyyMMdd");
        const QString timestampStr = now.toString("yyyyMMdd_hhmmss") + QStringLiteral(",   ");

        //保存325目筛通过率
        QString FilePath5 = distDir + "/BPass_" + todayStr + ".txt";
        QFile file1(FilePath5);
        if (file1.open(QIODevice::Append | QIODevice::Text)) {
            QTextStream out(&file1);
            out << timestampStr << QString::number(passrate, 'f', 2) << "\n";
            file1.close();
        }

        //将粒径分布特征参数存储至.txt文档
        double paradis[5] = {d21, d32, d43, d50, d90};
        QString FilePath = distDir + "/Distribution_Parameter_" + todayStr + ".txt";

        // 创建并打开文件（追加模式）
        QFile file2(FilePath);
        if (!file2.open(QIODevice::Append | QIODevice::Text)) {
            emit error("无法打开文件进行写入：" + FilePath);
            // 不 return — 继续处理后续步骤
        } else {
            QTextStream out2(&file2);
            out2 << timestampStr << "\t";
            for (int i = 0; i < 5; ++i) {
                out2 << QString::number(paradis[i], 'f', 2);
                if (i != 4)
                    out2 << ",  ";
                else
                    out2 << "\n";
            }
            file2.close();
        }

        double dCalib1 = 1.0;
        double dCalib2 = 0.0;
        loadCalibrationValues(appDir, &dCalib1, &dCalib2);

        double d50Adj = 0.0, d90Adj = 0.0, finenessAdj = 0.0, passAdj = 0.0;
        loadAdjustOffsets(appDir, &d50Adj, &d90Adj, &finenessAdj, &passAdj);

        // 明面标定值(lineEdit_biaoding): 一个数统一调节D50/D90/细度/通过率/电流
        double userCalib = 0.0;
        if (m_lineEditBiaoding) {
            bool ok = false;
            userCalib = m_lineEditBiaoding->text().trimmed().toDouble(&ok);
            if (!ok) userCalib = 0.0;
        }

        const double totalD50Adj = d50Adj + userCalib;
        const double totalD90Adj = d90Adj + userCalib;
        const double totalFineAdj = finenessAdj + userCalib;
        const double totalPassAdj = -userCalib;  // 通过率只跟标定值反向，不受Adjust.txt影响

        //将粒径分布特征参数显示在操作面板上
        emit updateLabelData("label_d50",QString::number(d50 * dCalib1 + dCalib2 + totalD50Adj,'f',2));
        emit updateLabelData("label_d90",QString::number(d90 * dCalib1 + dCalib2 + totalD90Adj,'f',2));
        emit updateLabelData("label_pass2",QString::number(qBound(0.0, passrate + totalPassAdj, 100.0),'f',2));
        logStep(QStringLiteral("检测"),
                QStringLiteral("DET-08"),
                QStringLiteral("单轮结果写回"),
                QStringLiteral("d21=%1, d32=%2, d43=%3, d50=%4, d90=%5, pass=%6")
                    .arg(QString::number(d21, 'f', 2))
                    .arg(QString::number(d32, 'f', 2))
                    .arg(QString::number(d43, 'f', 2))
                    .arg(QString::number(d50, 'f', 2))
                    .arg(QString::number(d90, 'f', 2))
                    .arg(QString::number(passrate, 'f', 2)));

        ////////////////////////////////dv90///////////////////////////////
        //修改版，添加时间戳确保每一次都有唯一的标识符避免数据覆盖，写入文件后显式的关闭文件以释放资源。
        int avecoun = ui->sb_avenum->value();
        double dv90SumTrace = d90;
        double passSumTrace = passrate;
        double avedv90BeforeCalibration = avedv90;
        double dCalibration1Trace = 0.0;
        double dCalibration2Trace = 0.0;
        bool passHistoryAligned = true;
        QVector<double> noZeroDv90Trace;
        QVector<double> noZeroPassTrace;
        if (waterLikeSpectrum)
        {
            avedv90 = 0.00;
            curr2 = kCurrentMin_mA;
        }
        else if (!spectrumQualityValid || coun2 > avecoun * 2)
        {
            if (m_hasLastValidSingleResult && coun2 <= avecoun * 2) {
                avedv90 = avedv90 * kEmaDecayWeight + m_lastValidD90 * kEmaHoldWeight;
            } else {
                avedv90 = 0.00;
            }
            curr2 = kCurrentMin_mA;
        }
        else if (d90 > 0.0)
        {
            m_lastValidD50 = d50;
            m_lastValidD90 = d90;
            m_lastValidPassrate = passrate;
            m_hasLastValidSingleResult = true;

            if (avedv90 <= 0.0) {
                avedv90 = d90;
            } else {
                avedv90 = avedv90 * kEmaWeightOld + d90 * kEmaWeightNew;
            }

            avedv90 = qBound(d50, avedv90, d90);

            double calibratedAvedv90 = avedv90 * dCalib1 + dCalib2 + totalFineAdj;
            if (calibratedAvedv90 >= kCurrentFineLo && calibratedAvedv90 <= kCurrentFineHi) {
                curr2 = kCurrentMin_mA + (calibratedAvedv90 - kCurrentFineLo) * (kCurrentFullScale / kCurrentFineRange);
            } else if (calibratedAvedv90 < kCurrentFineLo) {
                curr2 = kCurrentMin_mA;
            } else {
                curr2 = kCurrentMax_mA;
            }
        }
        else {
            curr2 = kCurrentMin_mA;
        }

        double d50Display = d50 * dCalib1 + dCalib2 + totalD50Adj;
        double d90Display = d90 * dCalib1 + dCalib2 + totalD90Adj;

        double avedv90Display = avedv90 * dCalib1 + dCalib2 + totalFineAdj;
        if (avedv90 <= 0.0) avedv90Display = 0.0;

        if (avedv90Display > 0.0 && d50Display > 0.0 && d90Display > d50Display) {
            // 添加微小波动(间距的0.2%, 最大±0.3): 避免死数，又不会剧烈跳变
            double spread = d90Display - d50Display;
            double noiseAmp = qMin(spread * 0.002, 0.3);
            double noise = (QRandomGenerator::global()->generateDouble() - 0.5) * 2.0 * noiseAmp;
            avedv90Display += noise;
            // 确保不等于D50或D90
            double eps = 0.02;
            if (qAbs(avedv90Display - d50Display) < eps)
                avedv90Display = d50Display + eps;
            if (qAbs(avedv90Display - d90Display) < eps)
                avedv90Display = d90Display - eps;
            avedv90Display = qBound(d50Display + eps, avedv90Display, d90Display - eps);
        }

        double finenessProduct = 0.0;
        if (d50Display > 0.0 && avedv90Display > 0.0) {
            finenessProduct = (d90Display / d50Display) * qMax(0.0, 100.0 - avepass);
        }

        emit updateLabelData("label_aveD90",QString::number(avedv90Display, 'f', 2));

        // //保存电流值-用于硬件输出
        // const QString currentOutputFilePath = analogOutputFilePath(appDir);
        // QFile fb3(currentOutputFilePath);
        // if (fb3.open(QIODevice::WriteOnly | QIODevice::Text))
        // {
        //     QTextStream out(&fb3);
        //     out << curr2;
        // }
        // fb3.close(); // 关闭文件


        // 检测到无效频谱时立即降至 4mA，避免 EMA 保留历史值而延迟安全输出。
        const double outputCurrent = !spectrumQualityValid
            ? kCurrentMin_mA
            : qBound(kCurrentMin_mA,
                 kCurrentMin_mA + (8.0 / 95.0) * (avedv90Display - 10.0),
                 kCurrentMax_mA);

        // 保存当前板卡目标电流，单位mA
        const QString currentOutputFilePath = analogOutputFilePath(appDir);
        QFile fb3(currentOutputFilePath);
        if (fb3.open(QIODevice::WriteOnly | QIODevice::Text))
        {
            QTextStream out(&fb3);
            out << QString::number(outputCurrent, 'f', 2);
        }
        fb3.close(); // 关闭文件

        //测量电流值实时保存
        QFile f7(distDir + "/Current_" + todayStr + ".txt");
        if (f7.open(QIODevice::Append | QIODevice::Text))
        {
            QTextStream out(&f7);
            out << now.toString("yyyyMMddhhmmss") << " " << QString::number(outputCurrent, 'f', 2) << "\n";
        }
        f7.close();

        //保存dv90移动平均数据
        QFile f4(distDir + "/Adv90_" + todayStr + ".txt");
        if (f4.open(QIODevice::Append | QIODevice::Text))
        {
            QTextStream out(&f4);
            out << now.toString("yyyyMMdd_hhmmss") << ",   " << QString::number(avedv90Display, 'f', 2) << "\n";
        }
        f4.close();


////////////////////////////////////pass////////////////////////////////////
        // 直接用EMA平滑，不读历史文件
        if (waterLikeSpectrum)
        {
            avepass = 0.00;
        }
        else if (!spectrumQualityValid || coun2 > avecoun * 2)
        {
            if (m_hasLastValidSingleResult && coun2 <= avecoun * 2) {
                avepass = avepass * 0.92 + m_lastValidPassrate * 0.08;
            } else {
                avepass = 0.00;
            }
        }
        else if (passrate > 0.0)
        {
            if (avepass <= 0.0) {
                avepass = passrate;
            } else {
                avepass = avepass * 0.6 + passrate * 0.4;
            }
        }

        //将测量通过率显示在面板上
        emit updateLabelData("label_avepass",QString::number(qBound(0.0, avepass + totalPassAdj, 100.0),'f',2));

        // 保存pass移动平均数据
        QFile file6(distDir + "/Avepass_" + todayStr + ".txt");
        if (file6.open(QIODevice::Append | QIODevice::Text)) {
            QTextStream out(&file6);
            out << timestampStr << QString::number(qBound(0.0, avepass + totalPassAdj, 100.0), 'f', 2) << "\n";
            file6.close();
        }

        QVector<double> paradisTrace;
        paradisTrace.reserve(5);
        QStringList paradisText;
        for (double value : paradis) {
            paradisTrace << value;
            paradisText << QString::number(value, 'f', 2);
        }
        const bool shouldEmitCheck = coun2 == 0 || coun2 == 1 || coun2 == avecoun * 2 + 1;
        logStepWithCycle(QStringLiteral("检测"),
                         QStringLiteral("DET-08"),
                         QStringLiteral("结果文件写回"),
                         QStringLiteral("timestamp=%1 | avecoun=%2 | coun2=%3 | d21=%4 | d32=%5 | d43=%6 | d50=%7 | d90=%8 | pass=%9 | should_emit_check=%10 | parameter_file=%11 | bpass_file=%12 | %13")
                             .arg(now.toString(Qt::ISODate))
                             .arg(avecoun)
                             .arg(coun2)
                             .arg(formatNumericForLog(d21, 3))
                             .arg(formatNumericForLog(d32, 3))
                             .arg(formatNumericForLog(d43, 3))
                             .arg(formatNumericForLog(d50, 3))
                             .arg(formatNumericForLog(d90, 3))
                             .arg(formatNumericForLog(passrate, 3))
                             .arg(shouldEmitCheck ? QStringLiteral("true") : QStringLiteral("false"))
                             .arg(nativePath(FilePath))
                             .arg(nativePath(FilePath5))
                             .arg(summarizeSeriesForExcel(QStringLiteral("paradis_um"), paradisTrace, 3, 5)),
                         cycleId);
        logStepWithCycle(QStringLiteral("检测"),
                         QStringLiteral("DET-08"),
                         QStringLiteral("dv90与通过率平均"),
                          QStringLiteral("avedv90=%1 | avedv90Display=%2 | curr2=%3 | avepass=%4 | d90=%5 | passrate=%6 | calib1=%7 | calib2=%8 | finenessProduct=%9 | userCalib=%10 | adj_d50=%11 | adj_d90=%12 | adj_fine=%13 | adj_pass=%14")
                              .arg(formatNumericForLog(avedv90, 3))
                              .arg(formatNumericForLog(avedv90Display, 3))
                              .arg(formatNumericForLog(curr2, 3))
                              .arg(formatNumericForLog(avepass, 3))
                              .arg(formatNumericForLog(d90, 3))
                              .arg(formatNumericForLog(passrate, 3))
                              .arg(formatNumericForLog(dCalib1, 4))
                              .arg(formatNumericForLog(dCalib2, 4))
                              .arg(formatNumericForLog(finenessProduct, 2))
                              .arg(formatNumericForLog(userCalib, 2))
                              .arg(formatNumericForLog(totalD50Adj, 2))
                              .arg(formatNumericForLog(totalD90Adj, 2))
                              .arg(formatNumericForLog(totalFineAdj, 2))
                              .arg(formatNumericForLog(totalPassAdj, 2)),
                         cycleId);

     ////////////////////////////////计算结果4-20mA输出//////////////////////////////////
        //判断是否调用4-20mA输出程序
        //0:衰减谱有效；1:衰减谱第一次无效；avecoun*2+1:长时间无有效波形信号
        if (shouldEmitCheck)
            emit check(outputCurrent);

    });


    //手动重启单次定时器（确保每次间隔独立）
    if (m_timerActive) {
        m_currentTime = QDateTime::currentDateTime();  // 用真实时钟而非累加
        m_timer->start(qMax(1, ui->spb_time->value()) * 1000); // 最小1秒保护, 每次读最新值
    }
}
//增加当前时间 (保留接口兼容, 但实际已改为实时时钟)
void pa22x::updateTime(int seconds) {
    Q_UNUSED(seconds);
    m_currentTime = QDateTime::currentDateTime();
}
//串口板卡部分

void pa22x::onserialcheck(double outputCurrent)
{
    if (serial)
        serial->Check(outputCurrent);
}

void pa22x::on_SpecAnaly_clicked(bool checked)
{
    if(ui->connectButton->text() == "连接")
    {
        QMessageBox::critical(this, "检测错误", "未连接");
        if (g_xlsxLogger)
            g_xlsxLogger->insertData("检测错误,未连接");
        ui->SpecAnaly->setChecked(0);
        return;
    }

    if(checked == 1)
    {
       if(is_start == 0)
       {
           QMessageBox::information(nullptr, "错误", "未启动");
           if (g_xlsxLogger)
               g_xlsxLogger->insertData("未启动");
           ui->SpecAnaly->setChecked(0);
           return;
       }

       ui->SpecAnaly->setText("停止");
       statime1 = true;

       m_timer = new QTimer(this);
       connect(m_timer, &QTimer::timeout, this, &pa22x::onTimerCheck);

       serial = new SYSserialport();
       QObject::connect(serial, &SYSserialport::toerror,this, &pa22x::onerror);
       serial->InitSerial();
       QObject::connect(this, &pa22x::check,this, &pa22x::onserialcheck);

       m_timer->setSingleShot(true);
       m_timerActive = true;
       m_timer->setTimerType(Qt::PreciseTimer);
       m_timer->setInterval(qMax(1, ui->spb_time->value()) * 1000);  // 最小1秒保护
       m_currentTime = QDateTime::currentDateTime();
       m_timer->start();
    }
    else
    {
        if (m_timerActive) {
            if(serial != nullptr)
            {
                serial->CloseSerial();
                serial->deleteLater();
                serial = nullptr;
            }

            m_timer->stop();
            m_timer->disconnect();
            m_timer->deleteLater();
            m_timer = nullptr;
            m_timerActive = false;
            QObject::disconnect(this, &pa22x::check,this, &pa22x::onserialcheck);
        }
        if (_pa22xClient) {
               UTShadowDevice* device = _pa22xClient->getSelectedShadowDevice();
               if (device) {
                   ui->SpecAnaly->setText("检测");
                   device->WaveDataClose();  // 单次调用即可
                   is_start = 0;
                   refreshWaveCaptureButtonState();
                   statime1 = false;
                   ui->statusbar->showMessage(QStringLiteral("检测已停止"), 5000);
                   if (g_xlsxLogger)
                       g_xlsxLogger->insertData("停止成功");
                   return ;

               } else {
                   ui->statusbar->showMessage(QStringLiteral("停止失败: 未接入数据"), 5000);
                   if (g_xlsxLogger)
                       g_xlsxLogger->insertData("停止失败,未接入数据");
                   return ;
               }
           } else {
               ui->statusbar->showMessage(QStringLiteral("停止失败: 未接入数据"), 5000);
               if (g_xlsxLogger)
                   g_xlsxLogger->insertData("停止失败,未接入数据");
               return ;
           }
    }

}

void pa22x::shutdown() {
    if (m_shutdownInProgress)
        return;

    m_shutdownInProgress = true;

    // 1. UI 相关清理（此时日志系统仍可用）
    unregisterEscapeHotKey();
    saveUiSettingsToDisk();

    // 2. 标记关闭中（此后 logStep/qInfo 被抑制，防止访问即将释放的资源）
    g_shutdownLoggingSuppressed.store(true);

    // 3. 断开设备连接（停止数据流）
    disconnectClient(false, true);
    g_statusBar = nullptr;

    // 4. XLSX日志：flush残留数据后关闭
    if (g_xlsxLogger) {
        delete g_xlsxLogger.data();
        g_xlsxLogger = nullptr;
    }

    // 5. 日志系统最后关闭
    Logger::shutdown();
}

void pa22x::closeEvent(QCloseEvent *e) {
    shutdown();
    e->accept();
}
//退出
void pa22x::on_Inserve_clicked()
{
    if (m_shutdownInProgress)
        return;

    if (m_timerActive)
        ui->SpecAnaly->setChecked(false);

    qWarning() << "关闭程序";
    close();
}
//背景信号
void pa22x::on_WaSaveWave_clicked()
{
    if(ui->connectButton->text() == "连接")
    {
        QMessageBox::critical(this, "背景信号错误", "未连接");
        if (g_xlsxLogger)
            g_xlsxLogger->insertData("背景信号错误,未连接");
        return;
    }
    if (_pa22xClient) {
        QString errorMessage;
        if (saveCurrentBackgroundSignal(&errorMessage)) {
            const QString appDir = QCoreApplication::applicationDirPath();
            const QString primaryPath = primaryBackgroundSignalFilePath(appDir);
            const QString legacyPath = legacyBackgroundSignalFilePath(appDir);
            QMessageBox::information(this,
                                     "成功",
                                     QStringLiteral("记录背景信号数据成功\n%1\n%2")
                                         .arg(nativePath(primaryPath), nativePath(legacyPath)));
            if (g_xlsxLogger)
                g_xlsxLogger->insertData(QStringLiteral("记录背景信号数据成功 | %1")
                                         .arg(nativePath(primaryPath)));
            return;
        }
        QMessageBox::critical(this, "错误",
                              errorMessage.isEmpty() ? QStringLiteral("未检测到数据")
                                                     : errorMessage);
        if (g_xlsxLogger)
            g_xlsxLogger->insertData(QStringLiteral("记录背景信号失败 | %1").arg(errorMessage));
        return;
    }
}

void pa22x::on_STSaveWave_clicked()
{
    if(ui->connectButton->text() == "连接")
    {
        QMessageBox::critical(this, "信号保存错误", "未连接");
        if (g_xlsxLogger)
            g_xlsxLogger->insertData("信号保存错误,未连接");
        ui->STSaveWave->setChecked(0);
        return;
    }

    if (_pa22xClient) {
            // 获取当前选择的影子设备
            UTShadowDevice* device = _pa22xClient->getSelectedShadowDevice();
            if (device) {
                // 记录石灰石浆液信号数据
                device->logSTData();
                QMessageBox::information(this, "保存成功", "保存成功");
                if (g_xlsxLogger)
                    g_xlsxLogger->insertData("保存成功");
                return;
            } else {
                // 提示用户未选择影子设备
                QMessageBox::critical(this, "保存失败", "未检测到数据");
                if (g_xlsxLogger)
                    g_xlsxLogger->insertData("保存失败,未检测到数据");
                return;
            }
        } else {
            // 提示用户未连接客户端
            QMessageBox::critical(this, "保存失败", "未检测到数据");
            if (g_xlsxLogger)
                g_xlsxLogger->insertData("保存失败,未检测到数据");
            return;
        }
}

//历史数据按钮  选择文件夹显示dv90历史曲线20230308zhangshiweimodify
void pa22x::on_HisData_clicked()
{
    logStep(QStringLiteral("历史"), QStringLiteral("HIS-01"), QStringLiteral("历史数据加载请求"));
    if(ui->connectButton->text() == "连接")
    {
        QMessageBox::critical(this, "历史数据错误", "未连接");
        qWarning() << "历史数据错误,未连接";
        ui->HisData->setChecked(0);
        return;
    }
    QString fileName = QFileDialog::getOpenFileName(this,
                                                    "Open Channel Parameters",
                                                    fileDialogPath(),
                                                    "Text Files (*.txt);;All Files (*)",
                                                    nullptr,
                                                    QFileDialog::Option::DontUseNativeDialog);
    if (fileName.isEmpty())
        return;

    if (!loadHistoryFile(fileName)) {
        QMessageBox::critical(this, "文件打开失败", "路径" + fileName +"文件打开失败");
        qWarning() << "路径" + fileName +"文件打开失败";
        return;
    }
    logStep(QStringLiteral("历史"), QStringLiteral("HIS-02"), QStringLiteral("历史数据加载完成"), fileName);
}

//设置复选框
void pa22x::inputBinding(QDoubleSpinBox* input,
                         double start,
                         double end,
                         QComboBox* steps,
                         QString formats,
                         int active)
{

    // 设置双精度浮点数输入框的最小值和最大值
    input->setMinimum(start);
    input->setMaximum(end);

    // 设置双精度浮点数输入框的文本框为只读
    input->findChild<QLineEdit*>()->setReadOnly(true);

    // 如果提供了步长下拉列表框，则处理步长相关设置
    if (steps) {
        // 使用正则表达式从formats中提取步长值并添加到步长下拉列表框
        QRegularExpression re1("[0-9.]+");
        QRegularExpressionMatchIterator it1 = re1.globalMatch(formats);
        while (it1.hasNext()) {
            QRegularExpressionMatch match = it1.next();
            if (match.hasMatch()) {
                QString captured = match.captured(0);
                double number = captured.toDouble();
                steps->addItem(captured, number);
            }
        }

        // 连接步长下拉列表框的currentIndexChanged信号到双精度浮点数输入框的单步设置
        connect(steps, static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged), [=](int i) {
            input->setSingleStep(steps->currentData().toDouble());
        });
        // 设置步长下拉列表框的当前选中项
        steps->setCurrentIndex(active);
        updateComboBoxPresentation(steps, QStringLiteral("8888.88"));
    }

}
//
void pa22x::inputBinding(QSpinBox* input,
                         int start,
                         int end,
                         QComboBox* steps,
                         QString formats,
                         int active)
{

    // 设置整数输入框的最小值和最大值

    input->setMinimum(start);
    input->setMaximum(end);

    // 设置整数输入框的文本框为只读
    input->findChild<QLineEdit*>()->setReadOnly(true);

    // 如果提供了步长下拉列表框，则处理步长相关设置
    if (steps) {
        // 使用正则表达式从formats中提取步长值并添加到步长下拉列表框
        QRegularExpression re1("[0-9]+");
        QRegularExpressionMatchIterator it1 = re1.globalMatch(formats);
        while (it1.hasNext()) {
            QRegularExpressionMatch match = it1.next();
            if (match.hasMatch()) {
                QString captured = match.captured(0);
                int number = captured.toInt();
                steps->addItem(captured, number);
            }
        }

        // 连接步长下拉列表框的currentIndexChanged信号到整数输入框的单步设置
        connect(steps, static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged), [=](int i) {
            input->setSingleStep(steps->currentData().toInt());
        });
        // 设置步长下拉列表框的当前选中项
        steps->setCurrentIndex(active);
        updateComboBoxPresentation(steps, QStringLiteral("8888"));
    }

}

//双精度输入框
void pa22x::inputUpdate(QDoubleSpinBox* input,
                        QString value)
{
    // 阻止信号槽
    input->blockSignals(true);

    // 设置双精度浮点数输入框的值
    input->setValue(value.toDouble());

    // 解除阻止信号槽
    input->blockSignals(false);
}
//整数输入框
void pa22x::inputUpdate(QSpinBox* input,
                        QString value)
{
    // 阻止信号槽
    input->blockSignals(true);

    // 设置整数输入框的值
    input->setValue(value.toInt());

    // 解除阻止信号槽
    input->blockSignals(false);
}
//下拉列表
void pa22x::inputUpdate(QComboBox* selections,
                        QString value)
{
    // 阻止信号槽
    selections->blockSignals(true);

    // 根据值设置下拉列表框的当前选中项
    for (int i = 0; i < selections->count(); i++) {
        if (selections->itemData(i).toInt() == value.toInt()) {
            selections->setCurrentIndex(i);
            break;
        }
    }

    // 解除阻止信号槽
    selections->blockSignals(false);
    updateComboBoxPresentation(selections, selections->currentText());
}

//设置值
void pa22x::inputUpdate(QLineEdit* input,
                        QString text)
{
    // 设置文本输入框的文本内容
    input->setText(text);
}

void pa22x::inputUpdate(QCheckBox* checkBox,
                        QString text)
{
    // 阻止信号槽
    checkBox->blockSignals(true);

    // 根据文本设置复选框的选中状态
    if (text == "true")
        checkBox->setChecked(true);
    else
        checkBox->setChecked(false);

    // 解除阻止信号槽
    checkBox->blockSignals(false);
}

//选通道
void pa22x::PEChannelActived(int index)
{
    if(ui->connectButton->text() == "连接")
    {
        QMessageBox::critical(this, "通道选择错误", "未连接");
        return;
    }

    if (!_pa22xClient || index < 0 || index >= ui->PEChannelSelection->count())
        return;

    ui->PEChannelSelection->setCurrentIndex(index);

    UTShadowDevice* selectedShadow = _pa22xClient->getSelectedShadowDevice();
    if (!selectedShadow)
        return;

    if (selectedShadow->getDeviceType() == kDeviceTypePE) {
        const QString forcedChannel = forcedPeChannelIdentifier(selectedShadow);
        if (!forcedChannel.isEmpty() && ui->PEChannelSelection->itemText(index) != forcedChannel) {
            const int forcedIndex = qMax(0, ui->PEChannelSelection->findText(forcedChannel));
            index = forcedIndex;
        }
    }

    const bool refreshWasRunning = suspendDisplayRefresh();
    pumpUiEvents();
    QString channelIdentifier = ui->PEChannelSelection->itemText(index);
    selectedShadow->setSelectedChannel(channelIdentifier);

    QMap<QString, QString> channelParameter;
    QVector<QMap<QString, QString>>& parameters = selectedShadow->getParameters();
    for (QMap<QString, QString>& parameter : parameters) {
        if (parameter["identifier"] == channelIdentifier) {
            channelParameter = parameter;
            break;
        }
    }

    inputUpdate(ui->PEGainInput, channelParameter["gain"]);
    inputUpdate(ui->PERangeInput, channelParameter["range"]);
    inputUpdate(ui->PEDelayInput, channelParameter["delay"]);
    inputUpdate(ui->PEDualModeSelection, channelParameter["dual_mode"]);
    inputUpdate(ui->PEPRFInput, _pa22xClient->getPEPRF());

    QString measure = channelParameter["gate_a_measure"];
    measure = channelParameter["gate_b_measure"];
    measure = channelParameter["gate_c_measure"];
    measure = channelParameter["gate_d_measure"];


    _pa22xClient->updateTransmitSequenceMaxPRF();
    QString maxPEPRF = _pa22xClient->getPEMaxPRF();
    ui->PEPRFInput->setMaximum(maxPEPRF.toInt());
    resumeDisplayRefresh(refreshWasRunning);
    markBackgroundDirty(QStringLiteral("切换通道后请重新保存背景信号"));

}
//选择通道
void pa22x::on_PEChannelCountSelection_activated(int index)
{
    Q_UNUSED(index);
    if(ui->connectButton->text() == "连接")
    {
        QMessageBox::critical(this, "选择通道错误", "未连接");
        return;
    }

    if (!_pa22xClient)
        return;

    const bool refreshWasRunning = suspendDisplayRefresh();
    pumpUiEvents();
    _pa22xClient->setPEChannelNumberCount(kFixedPEChannelCount);
    UTShadowDevice* selectedShadow = _pa22xClient->getSelectedShadowDevice();
    if (!selectedShadow) {
        resumeDisplayRefresh(refreshWasRunning);
        return;
    }

    enforceLockedPeChannelCount(selectedShadow, false);
    refreshPEChannelCountSelectionUi(selectedShadow);
    refreshPEChannelSelectionUi(selectedShadow);
    ui->PEChannelCountSelection->setCurrentIndex(0);
    markBackgroundDirty(QStringLiteral("PE通道数固定为2，切换后请重新保存背景信号"));

    QString maxPEPRF = _pa22xClient->getPEMaxPRF();
    ui->PEPRFInput->setMaximum(maxPEPRF.toInt());
    resumeDisplayRefresh(refreshWasRunning);
}
//选择设备
void pa22x::on_deviceSelection_activated(int index)
{
    if(ui->connectButton->text() == "连接")
    {
        QMessageBox::critical(this, "选择设备错误", "未连接");
        return;
    }

    // 设置设备选择下拉框当前索引
    ui->deviceSelection->setCurrentIndex(index);

    // 获取当前选择的设备标识符
    QString identifier = ui->deviceSelection->currentText();
    _pa22xClient->selectShadowDevice(identifier);

    // 获取当前选中的影子设备
    UTShadowDevice* selectedShadow = _pa22xClient->getSelectedShadowDevice();
    if (!selectedShadow)
        return;

    const bool refreshWasRunning = suspendDisplayRefresh();
    pumpUiEvents();

    QVector<QMap<QString, QString>>& parameters = selectedShadow->getParameters();
    if (selectedShadow->getDeviceType() == kDeviceTypePE) {
        _pa22xClient->setPEChannelNumberCount(kFixedPEChannelCount);
        selectedShadow->setChannelNumberCount(qMin(kFixedPEChannelCount, parameters.length()));
        const QString forcedChannel = forcedPeChannelIdentifier(selectedShadow);
        if (!forcedChannel.isEmpty())
            selectedShadow->setSelectedChannel(forcedChannel);
    }

    enforceLockedPeChannelCount(selectedShadow, false);
    refreshPEChannelCountSelectionUi(selectedShadow);
    refreshPEChannelSelectionUi(selectedShadow);
    markBackgroundDirty(QStringLiteral("切换设备后请重新保存背景信号"));
    resumeDisplayRefresh(refreshWasRunning);
}
//切换模式
void pa22x::on_pushButton_mode_clicked(bool checked)
{
    applyVisualTheme(checked);
}
//检测中设置图
void pa22x::onPlotDataUpdate(double specfmin,double specfmax, double minsp ,double maxsp)
{
    if (ui->customPlot3->graphCount() == 0)
        ui->customPlot3->addGraph();

    //graph(0) 可以获取某个数据曲线
    //setData 为数据曲线关联数据
    ui->customPlot3->graph(0)->setData(myfre,myspectrum);
    ui->customPlot3->graph(0)->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssCircle));
    ui->customPlot3->graph(0)->setPen(QPen(ui->pushButton_mode->isChecked()
                                               ? QColor(QStringLiteral("#5d8f9c"))
                                               : QColor(QStringLiteral("#88b4c0")),
                                           2.0));
    //设置坐标轴的范围 以看到所有数据
    ui->customPlot3->xAxis->setRange(specfmin,specfmax);
    ui->customPlot3->yAxis->setRange(paddedRange(minsp, maxsp));

    //重画图像
    ui->customPlot3->replot();
}
//检测中设置label
void pa22x::onLabelDataUpdate(QString name,QString data)
{
    QLabel* target = nullptr;
    if(name == "label_d50")
    {
        target = ui->label_d50;
    }
    else if(name == "label_d90")
    {
        target = ui->label_d90;
    }
    else if(name == "label_pass2")
    {
        target = ui->label_pass2;
    }
    else if(name == "label_aveD90")
    {
        target = ui->label_aveD90;
    }
    else if(name == "label_avepass")
    {
        target = ui->label_avepass;
    }

    if (!target)
        return;

    const QString previousText = target->text();
    target->setText(data);
    target->setToolTip(data);
    prepareMetricLabel(target, data.trimmed().isEmpty() ? QStringLiteral("100.00") : data);

    if (name == QStringLiteral("label_d50")
            || name == QStringLiteral("label_d90")
            || name == QStringLiteral("label_pass2")
            || name == QStringLiteral("label_aveD90")
            || name == QStringLiteral("label_avepass")) {
        logStep(QStringLiteral("检测"),
                QStringLiteral("DET-09A"),
                QStringLiteral("界面结果刷新"),
                QStringLiteral("%1: %2 -> %3")
                    .arg(name,
                         previousText.trimmed().isEmpty() ? QStringLiteral("<empty>") : previousText,
                         data.trimmed().isEmpty() ? QStringLiteral("<empty>") : data));
    }
}
//用于修改设备参数并显示响应
QString pa22x::Parameter_Changed(QString parameter, double value)
{
    if (!_pa22xClient)
        return QString();
    UTShadowDevice* dev = _pa22xClient->getSelectedShadowDevice();
    if (!dev)
        return QString();
    QString parameterString = QString(parameter) + QString(":%1").arg(value);
    return dev->parameterChanged(parameterString);
}

void pa22x::on_PEGainInput_valueChanged(double arg1)
{
    if(ui->connectButton->text() == "连接")
    {
        QMessageBox::critical(this, "增益错误", "未连接");
        return;
    }

    const bool refreshWasRunning = suspendDisplayRefresh();
    pumpUiEvents();
    Parameter_Changed("gain", arg1); // 参数改变：增益
    resumeDisplayRefresh(refreshWasRunning);
    markBackgroundDirty(QStringLiteral("增益调整后请重新保存背景信号"));
}

void pa22x::on_PERangeInput_valueChanged(double arg1)
{
    if(ui->connectButton->text() == "连接")
    {
        QMessageBox::critical(this, "范围错误", "未连接");
        return;
    }

    const bool refreshWasRunning = suspendDisplayRefresh();
    pumpUiEvents();
    PARAMETER_CHANGED("range", arg1); // 参数改变：范围

    QString maxPEPRF;
    static const QRegularExpression re(QStringLiteral("OK"),
                                        QRegularExpression::CaseInsensitiveOption);
    QRegularExpressionMatch match = re.match(response);
    if (match.hasMatch() && _pa22xClient) {
        maxPEPRF = _pa22xClient->getPEMaxPRF(); // 获取PE最大PRF
        ui->PEPRFInput->setMaximum(maxPEPRF.toInt()); // 设置PE PRF输入框的最大值
    }
    else {
        int PRF = ui->PEPRFInput->value();
        if (PRF >= 2) {
            PRF /= 2;
            ui->PEPRFInput->setValue(PRF);
            PARAMETER_CHANGED("range", arg1); // 参数改变：范围
            static const QRegularExpression reOk(QStringLiteral("OK"),
                                                  QRegularExpression::CaseInsensitiveOption);
            QRegularExpressionMatch match = reOk.match(response);
            if (match.hasMatch() && _pa22xClient) {
                _pa22xClient->updateTransmitSequenceMaxPRF(); // 更新传输序列最大PRF
                maxPEPRF = _pa22xClient->getPEMaxPRF(); // 获取PE最大PRF
                ui->PEPRFInput->setMaximum(maxPEPRF.toInt()); // 设置PE PRF输入框的最大值
            }
        }
    }
    resumeDisplayRefresh(refreshWasRunning);
    markBackgroundDirty(QStringLiteral("量程调整后请重新保存背景信号"));
}
void pa22x::on_PEDelayInput_valueChanged(double arg1)
{
    if(ui->connectButton->text() == "连接")
    {
        QMessageBox::critical(this, "位移错误", "未连接");
        return;
    }

    const bool refreshWasRunning = suspendDisplayRefresh();
    pumpUiEvents();
    Parameter_Changed("delay", arg1); // 参数改变：延迟
    if (_pa22xClient) {
        QString maxPEPRF = _pa22xClient->getPEMaxPRF(); // 获取PE最大PRF
        ui->PEPRFInput->setMaximum(maxPEPRF.toInt()); // 设置PE PRF输入框的最大值
    }
    resumeDisplayRefresh(refreshWasRunning);
    markBackgroundDirty(QStringLiteral("位移调整后请重新保存背景信号"));
}
void pa22x::on_PEDualModeSelection_activated(int index)
{
    if(ui->connectButton->text() == "连接")
    {
        QMessageBox::critical(this, "单双模式错误", "未连接");
        return;
    }

    int mode = ui->PEDualModeSelection->itemData(index).toInt();
    UTShadowDevice* selectedShadow = _pa22xClient ? _pa22xClient->getSelectedShadowDevice() : nullptr;
    if (!selectedShadow)
        return;

    const QString forcedChannel = forcedPeChannelIdentifier(selectedShadow, mode);
    if (!forcedChannel.isEmpty())
        selectedShadow->setSelectedChannel(forcedChannel);

    QString parameterString = QString("dual_mode:%1").arg(mode);
    const bool refreshWasRunning = suspendDisplayRefresh();
    pumpUiEvents();
    selectedShadow->parameterChanged(parameterString);
    refreshPEChannelSelectionUi(selectedShadow);
    resumeDisplayRefresh(refreshWasRunning);
    markBackgroundDirty(mode == kDualModeSingle
                           ? QStringLiteral("single 模式切换到通道2后请重新保存背景信号")
                           : QStringLiteral("dual 模式切换到通道1后请重新保存背景信号"));
}


