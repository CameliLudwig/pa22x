#include "peimageview.h"

PEImageView::PEImageView(QWidget *parent) :
    ImageView(parent),
    _wave_form(nullptr)
{
}

PEImageView::~PEImageView()
{
}

void PEImageView::setData(void* data)
{
    _wave_form = (X22_Waveform*)data;
}

void PEImageView::draw(ImageViewDrawRotation rotation,
                       QString rectification)
{
    _rotation = rotation;
    _rectification = rectification;
    update();
}

bool PEImageView::exportCurrentSamples(QVector<double>* samples,
                                       QString* errorMessage) const
{
    if (!_wave_form) {
        if (errorMessage)
            *errorMessage = QStringLiteral("PEAWaveView 当前没有可导出的波形。");
        return false;
    }

    QVector<double> localSamples;
    localSamples.reserve(UTShadowDevice::_PEWaveLength);

    bool hasNonZeroSample = false;
    for (int i = 0; i < UTShadowDevice::_PEWaveLength; ++i) {
        const int sample = _wave_form->waveP[i] >= _wave_form->waveN[i]
                ? _wave_form->waveP[i]
                : -_wave_form->waveN[i];
        localSamples << sample;
        if (sample != 0)
            hasNonZeroSample = true;
    }

    if (!hasNonZeroSample) {
        if (errorMessage)
            *errorMessage = QStringLiteral("PEAWaveView 当前波形全为 0。");
        return false;
    }

    if (samples)
        *samples = localSamples;
    return true;
}

// 设置页面黑白模式，0 为黑色字体，1 为白色字体。
void PEImageView::setMode(int a)
{
    m_mode = a;
}

void PEImageView::paintEvent(QPaintEvent *e)
{
    Q_UNUSED(e);

    if (_wave_form) {
        int width = this->size().width();
        int height = this->size().height();

        QPainter p(this);
        p.setWindow(QRect(0, 0, width, height));

        const QColor frameColor = m_mode
                ? QColor(QStringLiteral("#8fa7b0"))
                : QColor(QStringLiteral("#5f7680"));
        const QColor waveColor = m_mode
                ? QColor(QStringLiteral("#3f6f7e"))
                : QColor(QStringLiteral("#88b4c0"));
        const QColor gateAColor = m_mode
                ? QColor(QStringLiteral("#c86759"))
                : QColor(QStringLiteral("#e1836f"));
        const QColor gateBColor = m_mode
                ? QColor(QStringLiteral("#5b9384"))
                : QColor(QStringLiteral("#79b09f"));
        const QColor gateCColor = m_mode
                ? QColor(QStringLiteral("#4f789d"))
                : QColor(QStringLiteral("#74a2cb"));
        const QColor gateCAltColor = m_mode
                ? QColor(QStringLiteral("#6d88aa"))
                : QColor(QStringLiteral("#93b0cf"));
        const QColor gateDColor = m_mode
                ? QColor(QStringLiteral("#b58a48"))
                : QColor(QStringLiteral("#d6ad66"));

        QPen framePen(frameColor);
        framePen.setWidth(1);
        p.setPen(framePen);

        p.drawLine(0, 0, 0, 255);
        p.drawLine(0, 255, 447, 255);
        p.drawLine(447, 255, 447, 0);
        p.drawLine(447, 0, 0, 0);

        QPen wavePen(waveColor);
        wavePen.setWidth(1);
        p.setPen(wavePen);

        if (_rotation == kImageViewDrawHorizontal) {
            uint8_t P[448];
            uint8_t N[448];

            for (int i = 0; i < 448; i++) {
                P[i] = _wave_form->waveP[i];
                N[i] = _wave_form->waveN[i];
            }

            if (_rectification == "rf") {
                for (int i = 0; i < 448; i++) {
                    P[i] /= 2;
                    N[i] /= 2;
                }

                // 使用 int 中间计算防止 unsigned char 溢出
                auto rfEncode = [](uint8_t pVal, uint8_t nVal) -> unsigned char {
                    if (pVal == 0 && nVal == 0)
                        return 128;
                    if (pVal < nVal)
                        return static_cast<unsigned char>(qBound(0, static_cast<int>(nVal) + 128, 255));
                    return static_cast<unsigned char>(qBound(0, 256 - (static_cast<int>(pVal) + 128), 255));
                };

                for (int i = 1; i < 448; i++) {
                    unsigned char pre = rfEncode(P[i-1], N[i-1]);
                    unsigned char current = rfEncode(P[i], N[i]);

                    unsigned char append = 128;
                    if (P[i] != 0 && N[i] != 0) {
                        if (P[i] > N[i])
                            append = static_cast<unsigned char>(qBound(0, static_cast<int>(N[i]) + 128, 255));
                        else if (P[i] < N[i])
                            append = static_cast<unsigned char>(qBound(0, 256 - (static_cast<int>(P[i]) + 128), 255));
                    }

                    p.drawLine(i - 1, pre, i, current);
                    if (append != 128)
                        p.drawLine(i, 128, i, append);
                }
            }
            else {
                for (int i = 1; i < 448; i++)
                    p.drawLine(i - 1, (height - P[i-1]), i, (height - P[i]));
            }

            p.setPen(gateAColor);
            p.drawLine(gate[0].start, gate[0].height, gate[0].start + gate[0].width, gate[0].height);
            p.setPen(gateBColor);
            p.drawLine(gate[1].start, gate[1].height, gate[1].start + gate[1].width, gate[1].height);
            p.setPen(gateCColor);
            p.drawLine(gate[2].start, gate[2].height, gate[2].start + gate[2].width, gate[2].height);
            p.setPen(gateDColor);
            p.drawLine(gate[3].start, gate[3].height, gate[3].start + gate[3].width, gate[3].height);
        }
        else if (_rotation == kImageViewDrawVertical) {
            for (int i = 1; i < 448; i++)
                p.drawLine((unsigned char)_wave_form->waveP[i-1], i - 1,
                           (unsigned char)_wave_form->waveP[i], i);

            p.setPen(gateAColor);
            p.drawLine(gate[0].height, gate[0].start,
                       gate[0].height, gate[0].start + gate[0].width);
            p.setPen(gateDColor);
            p.drawLine(gate[1].height, gate[1].start,
                       gate[1].height, gate[1].start + gate[1].width);
            p.setPen(gateCAltColor);
            p.drawLine(gate[2].height, gate[2].start,
                       gate[2].height, gate[2].start + gate[2].width);
            p.setPen(gateBColor);
            p.drawLine(gate[3].height, gate[3].start,
                       gate[3].height, gate[3].start + gate[3].width);
        }
    }
}
