#ifndef INVERSION_H
#define INVERSION_H
#include <QFile>
#include <QTextStream>
#include <QDebug>
#include <QString>
#include <cmath>
#include <QObject>
#define MaxSize 200
#define MaxSize2 40000

struct PhysicalModelParams;

extern "C" {
    #include "nnls.h" // 包含 C 语言头文件
    //#include "labwindows_api.h"
    #include "export.h"
}

class inversion : public QObject
{
public:
    inversion(QString path);
    void init();
    void initWithConfig(const PhysicalModelParams& cfg);
    void check();
    void MyIniRead();
    void LesOrtJc(double* Spec,double* F,int NumOfWl,double LB,double UB,int NumOfDiameter,
        int SplineNum,double Gamma,double L,double* Diameter,double* Distribution,
        double* D21,double* D32,double*D43);
    // 进行依赖关系修正
    void CorrectDependent();
    void init0();
    void org(int nb,int nr,double lt,double coe[MaxSize][MaxSize],double gmeas[],
          double* r,double* distr,int nc,double gm);
    void normal(double* xin,int n);
    void ParticleInfo2(double Diameter[],double Distribution[],int NumOfDiameter,
    double* D21, double* D32,double* D43);
    void iniort(int nc,int nr,double r[]);
    double Bspline(int i, int nc, double t);
    double B3(int i, double t);
    double B2(int i,double t);
    double B1(int i,double t);
    void brmul(double a[][MaxSize],double b[][MaxSize],int m,int n,int k,double c[][MaxSize]);
    double RR(double rin, double rbar, double k);
    void Uniform1(double *InputArray, double *OutputArray, int NumOfArray);
    void ParticleInfo(double Diameter[], double Distribution[], int NumOfDiameter,
                      double* D21, double* D32, double* D43, double* D50, double* D90);

private:
    double m_pp[7],m_mp[8],m_ip[6],m_cp[2];
    double m_psi[MaxSize][MaxSize];
    double m_psit[MaxSize][MaxSize];
    double m_H[MaxSize][MaxSize];
    double m_HT[MaxSize][MaxSize];
    double m_MA[MaxSize][MaxSize];
    double m_MAT[MaxSize][MaxSize];
    double m_MB[MaxSize][MaxSize];
    double m_ML[MaxSize][MaxSize];
    double m_MLT[MaxSize][MaxSize];
    double m_MC[MaxSize][MaxSize];
    double m_Tn[MaxSize];
    double m_Spec[MaxSize];
    double m_F[MaxSize];
    int m_NumOfWl;
    double m_LB,m_UB;
    int m_NumOfDiameter;
    int m_SplineNum;
    double m_Gamma1,m_L;
    double m_Diameter[MaxSize];
    double m_Distribution[MaxSize];	 //数目频度分布
    double m_VDistribution[MaxSize];   //体积频度分布   zhangshiwei20230428
    double m_CumuDistribution[MaxSize];    //数目累积分布
    double m_CumuVDistribution[MaxSize];   //体积累积分布 zhangshiwei20230428
    double m_D21;
    double m_D32;
    double m_D43;
    double m_D50;
    double m_D90;
    QString m_path;
};

#endif // INVERSION_H
