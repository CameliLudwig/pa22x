#ifndef PEIMAGEVIEW_H
#define PEIMAGEVIEW_H
#include <QPainter>
#include "imageview.h"
#include "pa22xclient.h"
class PEImageView : public ImageView
{
    Q_OBJECT

public:
    explicit PEImageView(QWidget *parent = 0);
    ~PEImageView();
    virtual void setData(void* data) override;
    virtual void draw(ImageViewDrawRotation rotation = kImageViewDrawHorizontal,
                      QString rectification = QString("")) override;
    bool exportCurrentSamples(QVector<double>* samples,
                              QString* errorMessage = nullptr) const;
    void setMode(int a);

protected:
    void paintEvent(QPaintEvent *) override;

private:
    X22_Waveform* _wave_form;
    bool m_mode = false;
};

#endif // PEIMAGEVIEW_H
