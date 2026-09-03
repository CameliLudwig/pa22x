#ifndef FFT_H
#define FFT_H

#include <QObject>
#include <QDebug>
#include <qmath.h>

#define MAX_MATRIX_SIZE    4194304 // 2048*2048
#define PI                 3.141592653
#define MAX_VECTOR_LENGTH  1000

#define Fs  250000000

typedef struct Complex
{
    double rl;   //实部 用这个当y轴画图像就可以
    double im;   //虚部
}Complex;



class fft : public QObject
{
    Q_OBJECT
public:
    explicit fft(QObject *parent = nullptr);
    //傅里叶转换 频域
    bool fft1(QVector<Complex> inVec, int const len, QVector<Complex>& outVec);
    //逆转换
    bool ifft(const QVector<Complex>& inVec, QVector<Complex>& outVec);

    // 计算FFT中进行傅里叶变换计算时所需的层数。
    int get_computation_layers(int num);
    //判断给定的整数是否是2的幂次方。
    bool is_power_of_two(int num);
    //测试
    void test();


};

#endif // FFT_H
