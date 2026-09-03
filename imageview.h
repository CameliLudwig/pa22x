#ifndef IMAGEVIEW_H
#define IMAGEVIEW_H

#include <QWidget>
#include <QString>

typedef enum _ImageViewDrawRotation {
    kImageViewDrawHorizontal,
    kImageViewDrawVertical,
} ImageViewDrawRotation;

class ImageView : public QWidget
{
    Q_OBJECT

public:
    explicit ImageView(QWidget *parent = 0);
    ~ImageView();
    virtual void setData(void* data) = 0;
    virtual void draw(ImageViewDrawRotation rotation = kImageViewDrawHorizontal,
                      QString rectification = QString("")) = 0;
    // 设置门的参数
    virtual void setGate(int start, int width, int height, QString name);
    // 设置射线引导线的起点和终点坐标
    virtual void setBeamGuideLine(int x0, int y0, int x1, int y1);

protected:
    ImageViewDrawRotation _rotation;
    QString _rectification;
    void paintEvent(QPaintEvent *) override;

    struct {
        int start;
        int width;
        int height;
    } gate[4];

    int _beam_line_x0;
    int _beam_line_y0;
    int _beam_line_x1;
    int _beam_line_y1;
};

#endif // PAIMAGEVIEW_H
