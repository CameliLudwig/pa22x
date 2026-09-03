#include "fft.h"
#include <math.h>
#include <memory>

fft::fft(QObject *parent) : QObject(parent)
{

}

//傅里叶变换 频域
bool fft::fft1(QVector<Complex> inVec, int const len, QVector<Complex>& outVec)
{
    if ((len <= 0) || inVec.isEmpty() || outVec.isEmpty())
        return false;
    if (!is_power_of_two(len))
        return false;

    // 使用 Qt 容器管理内存
    QVector<Complex> pVec(len);
    QVector<Complex> Weights(len);
    QVector<Complex> X(len);
    QVector<int> pnInvBits(len);

    // 直接拷贝数据
    std::copy(inVec.begin(), inVec.end(), pVec.begin());

    // 计算权重系列
    double fixed_factor = (-2 * M_PI) / len;
    for (int i = 0; i < len / 2; i++) {
        double angle = i * fixed_factor;
        Weights[i].rl = qCos(angle);  // 使用 Qt 数学函数
        Weights[i].im = qSin(angle);
    }

    // 对称权重生成
    for (int i = len / 2; i < len; i++) {
        Weights[i].rl = -Weights[i - len / 2].rl;
        Weights[i].im = -Weights[i - len / 2].im;
    }

    // 计算倒序位码
    int r = get_computation_layers(len);
    for (int i = 0; i < len; i++) {
        int index = 0;
        for (int m = r - 1; m >= 0; m--) {
            index += ((i >> m) & 1) << (r - m - 1);
        }
        pnInvBits[i] = index;
        X[i] = pVec[pnInvBits[i]];
    }

    // FFT 核心计算
    for (int L = 1; L <= r; L++) {
        int distance = 1 << (L - 1);
        int W = 1 << (r - L);
        int B = len >> L;
        int N = len / B;

        for (int b = 0; b < B; b++) {
            int mid = b * N;
            for (int n = 0; n < N / 2; n++) {
                int index = n + mid;
                int dist = index + distance;
                double rl = X[index].rl + (Weights[n * W].rl * X[dist].rl - Weights[n * W].im * X[dist].im);
                double im = X[index].im + (Weights[n * W].im * X[dist].rl + Weights[n * W].rl * X[dist].im);
                pVec[index] = {rl, im};
            }
            for (int n = N / 2; n < N; n++) {
                int index = n + mid;
                int dist = index - distance;
                double rl = X[dist].rl + (Weights[n * W].rl * X[index].rl - Weights[n * W].im * X[index].im);
                double im = X[dist].im + (Weights[n * W].im * X[index].rl + Weights[n * W].rl * X[index].im);
                pVec[index] = {rl, im};
            }
        }
        X = pVec; // Qt 容器直接赋值
    }

    outVec = pVec; // 输出结果
    return true;
}

//逆变换
bool fft::ifft(const QVector<Complex>& inVec, QVector<Complex>& outVec)
{
    if (inVec.isEmpty() || outVec.isEmpty() || !is_power_of_two(inVec.size()))
        return false;

    int len = inVec.size();
    QVector<double> W_rl(len);
    QVector<double> W_im(len);
    QVector<double> X_rl(len);
    QVector<double> X_im(len);
    QVector<double> X2_rl(len);
    QVector<double> X2_im(len);

    double fixed_factor = (-2 * M_PI) / len;
    for (int i = 0; i < len / 2; i++) {
        double angle = i * fixed_factor;
        W_rl[i] = qCos(angle);
        W_im[i] = qSin(angle);
    }
    for (int i = len / 2; i < len; i++) {
        W_rl[i] = -W_rl[i - len / 2];
        W_im[i] = -W_im[i - len / 2];
    }

    // 初始化输入数据
    for (int i = 0; i < len; i++) {
        X_rl[i] = inVec[i].rl;
        X_im[i] = inVec[i].im;
    }

    int r = get_computation_layers(len);
    for (int L = r; L >= 1; L--) {
        int distance = 1 << (L - 1);
        int W = 1 << (r - L);
        int B = len >> L;
        int N = len / B;

        for (int b = 0; b < B; b++) {
            for (int n = 0; n < N / 2; n++) {
                int index = n + b * N;
                X2_rl[index] = (X_rl[index] + X_rl[index + distance]) / 2;
                X2_im[index] = (X_im[index] + X_im[index + distance]) / 2;
            }
            for (int n = N / 2; n < N; n++) {
                int index = n + b * N;
                double re = (X_rl[index] - X_rl[index - distance]) / 2;
                double im = (X_im[index] - X_im[index - distance]) / 2;
                double square = W_rl[n * W] * W_rl[n * W] + W_im[n * W] * W_im[n * W];
                double part1 = re * W_rl[n * W] + im * W_im[n * W];
                double part2 = im * W_rl[n * W] - re * W_im[n * W];
                X2_rl[index] = square > 0 ? part1 / square : 0;
                X2_im[index] = square > 0 ? part2 / square : 0;
            }
        }
        X_rl = X2_rl; // Qt 容器深拷贝
        X_im = X2_im;
    }

    // 位码倒序
    for (int i = 0; i < len; i++) {
        int index = 0;
        for (int m = r - 1; m >= 0; m--) {
            index += ((i >> m) & 1) << (r - m - 1);
        }
        outVec[i] = {X_rl[index], X_im[index]};
    }
    return true;
}

// 计算FFT中进行傅里叶变换计算时所需的层数。
int fft::get_computation_layers(int num)
{
    int nLayers = 0; // 初始化计算层数为0
    int len = num;   // 初始化长度为传入的num值

    if (len == 2)
        return 1;    // 如果长度为2，直接返回1层

    while (true)
    {
        len = len / 2; // 每次循环长度减半
        nLayers++;     // 层数加1

        if (len == 2)
            return nLayers + 1; // 如果长度减半后等于2，返回层数加1

        if (len < 1)
            return -1;   // 如果长度小于1，返回-1（错误状态）
    }
}
//判断给定的整数是否是2的幂次方。
bool fft::is_power_of_two(int num)
{
    int temp = num; // 将传入的num值存入临时变量temp
    int mod = 0;    // 初始化mod为0
    int result = 0; // 初始化result为0

    if (num < 2)
        return false; // 如果num小于2，直接返回false
    if (num == 2)
        return true;  // 如果num等于2，直接返回true

    while (temp > 1)
    {
        result = temp / 2; // temp除以2的结果存入result
        mod = temp % 2;    // temp除以2的余数存入mod

        if (mod)
            return false;  // 如果余数不为0，说明num不是2的幂，返回false

        if (2 == result)
            return true;   // 如果result等于2，说明num是2的幂，返回true

        temp = result;     // 更新temp为result的值，继续循环判断
    }

    return false; // 如果循环结束还未返回true，则返回false
}



void fft::test()
{
    double vec[] = { 15, 32, 9, 222, 118, 151, 5, 7, 56, 233, 56, 121, 235, 89, 98, 111 };
    int len = sizeof(vec) / sizeof(double);

    QVector<Complex> inVec(len);
    QVector<Complex> outVec(len);
    for (int i = 0; i < len; i++) {
        inVec[i].rl = vec[i];
    }

    fft1(inVec, len, outVec);

    qDebug() << "快速傅里叶变换结果为：";
    for (int i = 0; i < len; i++) {
        QString msg = QString("result[%1]: %2 %3 %4i")
            .arg(i+1)
            .arg(outVec[i].rl, 0, 'f', 2)
            .arg(outVec[i].im < 0 ? "-" : "+")
            .arg(qAbs(outVec[i].im), 0, 'f', 2);
        qDebug() << msg;
    }

    QVector<Complex> invert(len);
    QVector<Complex> outVecp = outVec;
    ifft(outVecp, invert);

    qDebug() << "逆变换结果为：";
    for (int i = 0; i < len; i++) {
        QString msg = QString("ifft[%1]: %2").arg(i+1).arg(invert[i].rl, 0, 'f', 2);
        qDebug() << msg;
    }
}
