#pragma once

#include <QWidget>
#include <QRect>
#include <QPainter>

//void DrawSmartRuler(QPainter& p, const QRect& r, double beginValue, double endValue, Qt::AnchorPoint anchor);

class Ruler: public QWidget
{
    Q_OBJECT

private:
    Qt::AnchorPoint anchor;
    double begin {0.0};
    double end {100.0};
    bool m_mode = false;

public:
    Ruler(QWidget* parent);
    //获取点
    Qt::AnchorPoint getAnchorPoint() const;
    //设置点
    void setAnchorPoint(Qt::AnchorPoint);
    //设置起始区间
    void setRange(double begin, double end);
    void setMode(int a);

protected:
    void paintEvent(QPaintEvent *) override;
};
