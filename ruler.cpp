#include "ruler.h"
#include "ruler.h"
#include <cmath>
#include <algorithm>
#include <QRect>
#include <QPainter>
#include <QDebug>


const double kMark[] =
{
    1.0,   // 标尺刻度的基本倍数1.0
    2.0,   // 标尺刻度的基本倍数2.0
    5.0    // 标尺刻度的基本倍数5.0
};

double FirstMark(double beginValue, double interval)
{
    double lf = beginValue / interval;
    if (lf > 0)
    {
        lf += 1.0;  // 如果lf大于0，则加上1.0
    }
    return static_cast<int>(lf) * interval;  // 返回lf的整数乘以间隔
}

double LastMark(double endValue, double interval)
{
    double lf = endValue / interval;
    if (lf < 0)
    {
        lf -= 1.0;  // 如果lf小于0，则减去1.0
    }
    return static_cast<int>(lf) * interval;  // 返回lf的整数乘以间隔
}

struct SmartRulerOptions
{
    double firstMark;   // 第一个刻度标记
    double lastMark;    // 最后一个刻度标记
    double interval;    // 刻度间隔
    double pixelValue;  // 像素值，表示每个像素对应的数值

    SmartRulerOptions(QPainter& p, int length, double beginValue, double endValue)
    {
        double range = endValue - beginValue;  // 计算数值范围
        pixelValue = range / length;           // 计算每个像素对应的数值

        if (pixelValue == 0.0)
            pixelValue = 0.0001;  // 防止除以0，设置一个极小的值

        double ratio = 1.0;
        double val = std::abs(pixelValue);

        // 根据像素值的大小调整倍率，使其落在0.1到1.0之间
        while (val < 0.1)
        {
            val *= 10.0;
            ratio /= 10.0;
        }
        while (val > 1.0)
        {
            val /= 10.0;
            ratio *= 10.0;
        }

        int x = 1;
        bool found = false;

        interval = 1.0;
        // 寻找合适的刻度间隔
        while (!found)
        {
            for (int i = 0; i < (int)_countof(kMark); i++)
            {
                interval = kMark[i] * x;
                firstMark = FirstMark(beginValue, interval * ratio);  // 计算第一个刻度标记
                lastMark = LastMark(endValue, interval * ratio);      // 计算最后一个刻度标记
                // 计算第一个和最后一个刻度标记的宽度
                int firstW = p.fontMetrics().horizontalAdvance(QString("%0").arg(firstMark + interval * ratio));
                int lastW = p.fontMetrics().horizontalAdvance(QString("%0").arg(lastMark + interval * ratio));
                int width = std::max(firstW, lastW);  // 取较大的宽度
                // 如果刻度间隔除以倍率大于宽度的1.5倍，则认为找到合适的间隔
                if (interval / val > width * 1.5)
                {
                    found = true;
                    break;
                }
            }
            x *= 10;
        }
        interval *= ratio;  // 调整刻度间隔乘以倍率
        if (beginValue > endValue)
            interval = -interval;  // 如果起始值大于结束值，则刻度间隔取反
    }
};

void DrawTopRuler(QPainter& p, int length, double beginValue, double endValue)
{
    int h = p.fontMetrics().height() / 2;  // 字体高度的一半

    const SmartRulerOptions opt(p, length, beginValue, endValue);  // 根据绘制选项初始化智能标尺选项

    for (double pos = opt.firstMark; ; pos += opt.interval)
    {
        int px = int((pos - beginValue) / opt.pixelValue + 0.5);  // 计算当前位置的像素坐标

        if (px > length)
        {
            break;  // 如果超出指定长度，结束绘制
        }

        p.drawLine(px, 0, px, -h);  // 绘制刻度线

        QString txt;
        txt.setNum(pos);  // 将数值转换为字符串
        QRect txtrt = p.fontMetrics().boundingRect(txt);  // 获取文本的边界矩形
        txtrt.translate(QPoint(px - txtrt.width() / 2, -h - txtrt.bottom()));  // 根据位置调整文本矩形的位置

        if (txtrt.left() < 0)
            txtrt.translate(-txtrt.left(), 0);  // 如果文本左侧超出边界，移动到边界内

        if (txtrt.right() > length)
            txtrt.translate(length - txtrt.right(), 0);  // 如果文本右侧超出边界，移动到边界内

        p.drawText(txtrt, txt);  // 绘制文本
    }
}

void DrawBottomRuler(QPainter& p, int length, double beginValue, double endValue)
{
    int h = p.fontMetrics().height() / 2;  // 字体高度的一半

    const SmartRulerOptions opt(p, length, beginValue, endValue);  // 根据绘制选项初始化智能标尺选项

    for (double pos = opt.firstMark; ; pos += opt.interval)
    {
        int px = int((pos - beginValue) / opt.pixelValue + 0.5);  // 计算当前位置的像素坐标

        if (px > length)
        {
            break;  // 如果超出指定长度，结束绘制
        }

        p.drawLine(px, 0, px, h);  // 绘制刻度线

        QString txt;
        txt.setNum(pos);  // 将数值转换为字符串
        QRect txtrt = p.fontMetrics().boundingRect(txt);  // 获取文本的边界矩形
        txtrt.translate(QPoint(px - txtrt.width() / 2, h - txtrt.top()));  // 根据位置调整文本矩形的位置

        if (txtrt.left() < 0)
            txtrt.translate(-txtrt.left(), 0);  // 如果文本左侧超出边界，移动到边界内

        if (txtrt.right() > length)
            txtrt.translate(length - txtrt.right(), 0);  // 如果文本右侧超出边界，移动到边界内

        p.drawText(txtrt, txt);  // 绘制文本
    }
}
void DrawSmartRuler(QPainter& p, const QRect& r, double beginValue, double endValue, Qt::AnchorPoint anchor)
{
    switch (anchor)
    {
    case Qt::AnchorTop:
        p.save();
        p.translate(r.bottomLeft());  // 将坐标原点移动到矩形的左下角
        DrawTopRuler(p, r.width() - 1, beginValue, endValue);  // 绘制顶部标尺
        p.restore();
        break;
    case Qt::AnchorRight:
        p.save();
        p.translate(r.topLeft());  // 将坐标原点移动到矩形的左上角
        p.rotate(90);  // 旋转90度，使得绘制的标尺变为垂直方向
        DrawTopRuler(p, r.height() - 1, beginValue, endValue);  // 绘制右侧标尺
        p.restore();
        break;
    case Qt::AnchorBottom:
        p.save();
        p.translate(r.topLeft());  // 将坐标原点移动到矩形的左上角
        DrawBottomRuler(p, r.width() - 1, beginValue, endValue);  // 绘制底部标尺
        p.restore();
        break;
    case Qt::AnchorLeft:
        p.save();
        p.translate(r.topRight());  // 将坐标原点移动到矩形的右上角
        p.rotate(90);  // 旋转90度，使得绘制的标尺变为垂直方向
        DrawBottomRuler(p, r.height() - 1, beginValue, endValue);  // 绘制左侧标尺
        p.restore();
        break;
    default:
        break;
    }
}


Ruler::Ruler(QWidget *parent)
    : QWidget(parent)
{

}
//获取点
Qt::AnchorPoint Ruler::getAnchorPoint() const
{
    return anchor;
}
//设置点
void Ruler::setAnchorPoint(Qt::AnchorPoint anchor)
{
    this->anchor = anchor;
    update();
}
//设置起始区间
void Ruler::setRange(double begin, double end)
{
    this->begin = begin;
    this->end = end;
    update();
}
//设置页面黑白模式，0是黑色字体，1是白色字体
void Ruler::setMode(int a)
{
    m_mode = a;
}
//重绘画
void Ruler::paintEvent(QPaintEvent *)
{
    QPainter p(this);
    QFont f;
    f.setPixelSize(9);
    p.setFont(f);

    if(m_mode == false)
    {
        // 设置画笔颜色为白色
        p.setPen(Qt::white);
    }
    else
    {
        p.setPen(Qt::black);
    }

    DrawSmartRuler(p, rect(), begin, end, anchor);
}
