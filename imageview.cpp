#include "imageview.h"

ImageView::ImageView(QWidget *parent) :
    QWidget(parent),
    _rotation(kImageViewDrawHorizontal), // 初始化旋转方向为水平
    _beam_line_x0(-1), // 初始化射线引导线的起点 x 坐标
    _beam_line_y0(-1), // 初始化射线引导线的起点 y 坐标
    _beam_line_x1(-1), // 初始化射线引导线的终点 x 坐标
    _beam_line_y1(-1)  // 初始化射线引导线的终点 y 坐标
{
    // 初始化四个门的参数
    for (int i = 0; i < 4; i ++) {
        gate[i].start = 0;    // 起始位置
        gate[i].width = 0;    // 宽度
        gate[i].height = 0;   // 高度
    }
}

ImageView::~ImageView()
{
}

void ImageView::paintEvent(QPaintEvent *)
{
    // 在这里实现绘图逻辑，用来绘制图像及其相关元素
}

void ImageView::setGate(int start, int width, int height, QString name)
{
    // 设置门的参数
    if (name == "a") {
        gate[0].start = start;
        gate[0].width = width;
        gate[0].height = height;
    }
    else if (name == "b") {
        gate[1].start = start;
        gate[1].width = width;
        gate[1].height = height;
    }
    else if (name == "c") {
        gate[2].start = start;
        gate[2].width = width;
        gate[2].height = height;
    }
    else if (name == "d") {
        gate[3].start = start;
        gate[3].width = width;
        gate[3].height = height;
    }
}

void ImageView::setBeamGuideLine(int x0, int y0, int x1, int y1)
{
    // 设置射线引导线的起点和终点坐标
    _beam_line_x0 = x0;
    _beam_line_y0 = y0;
    _beam_line_x1 = x1;
    _beam_line_y1 = y1;
}
