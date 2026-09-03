#include "inversion.h"
#include "app_config.h"

//缺少cvidef.h
//advanlys.h
#include <algorithm>
#include <memory>
#include <limits>
#include <QDir>
#include <QVector>
#include <qmath.h>

// 最大值/最小值查找
void qt_MaxMin1D(const double x[], qintptr n, double *max,
                 qintptr *imax, double *min, qintptr *imin) {
    if(n <= 0 || !x) return;

    double curMax = x[0];
    double curMin = x[0];
    qintptr idxMax = 0;
    qintptr idxMin = 0;

    for(qintptr i = 1; i < n; ++i) {
        if(x[i] > curMax) {
            curMax = x[i];
            idxMax = i;
        }
        if(x[i] < curMin) {
            curMin = x[i];
            idxMin = i;
        }
    }

    if(max) *max = curMax;
    if(imax) *imax = idxMax;
    if(min) *min = curMin;
    if(imin) *imin = idxMin;
}

// 数组求和
void qt_Sum1D(const double x[], qintptr n, double *sum) {
    if(!sum || n <= 0 || !x) return;

    *sum = 0;
    for(qintptr i = 0; i < n; ++i) {
        *sum += x[i];
    }
}

// 线性变换 y = a*x + b
void qt_LinEv1D(const double x[], qintptr n, double a,
                double b, double y[]) {
    if(n <= 0 || !x || !y) return;

    for(qintptr i = 0; i < n; ++i) {
        y[i] = a * x[i] + b;
    }
}

// 数组赋值
void qt_Set1D(double x[], qintptr n, double a) {
    if(n <= 0 || !x) return;

    for(qintptr i = 0; i < n; ++i) {
        x[i] = a;
    }
}

// 数组减法 z = x - y
void qt_Sub1D(const double x[], const double y[], qintptr n, double z[]) {
    if(n <= 0 || !x || !y || !z) return;

    for(qintptr i = 0; i < n; ++i) {
        z[i] = x[i] - y[i];
    }
}

// 绝对值计算
void qt_Abs1D(const double x[], qintptr n, double y[]) {
    if(n <= 0 || !x || !y) return;

    for(qintptr i = 0; i < n; ++i) {
        y[i] = qAbs(x[i]);
    }
}

// 测试函数
double qt_test_isok(double a) {
    return qAbs(a);
}

//反演程序
inversion::inversion(QString path)
{
    m_path = QDir::fromNativeSeparators(path);
    if (!m_path.isEmpty() && !m_path.endsWith(QLatin1Char('/')))
        m_path.append(QLatin1Char('/'));
//    double a = 25.5;
//    qDebug() << a;
//    a = qt_test_isok(a);
//    qDebug() << a;
}
//初始化
void inversion::init()
{
    PhysicalModelParams defaults;
    initWithConfig(defaults);
}

void inversion::initWithConfig(const PhysicalModelParams& cfg)
{
    m_mp[0] = cfg.mp0;
    m_mp[1] = cfg.mp1;
    m_mp[2] = cfg.mp2;
    m_mp[3] = cfg.mp3;
    m_mp[4] = cfg.mp4;
    m_mp[5] = cfg.mp5;
    m_mp[6] = cfg.mp6;
    m_mp[7] = cfg.mp7;

    m_pp[0] = cfg.pp0;
    m_pp[1] = cfg.pp1;
    m_pp[2] = cfg.pp2;
    m_pp[3] = cfg.pp3;
    m_pp[4] = cfg.pp4;
    m_pp[5] = cfg.pp5;
    m_pp[6] = cfg.pp6;

    m_ip[0] = cfg.ip0;
    m_ip[1] = cfg.ip1;
    m_ip[2] = cfg.ip2;
    m_ip[3] = cfg.ip3;
    m_ip[4] = cfg.ip4;
    m_ip[5] = cfg.ip5;

    m_cp[0] = cfg.cp0;
    m_cp[1] = cfg.cp1;
}
//最后直接执行的功能
void inversion::check()
{
    //之前的直接执行的功能
//    MyGetVal();
//    ini(mp,pp);
//    MyIniRead();	  //zhanghsiwei20230428
    init();
    //export.h文件，内含cvidef.h文件
    ini(m_mp,m_pp);
    MyIniRead();

}

void inversion::LesOrtJc(double* Spec,double* F,int NumOfWl,double LB,double UB,int NumOfDiameter,
    int SplineNum,double Gamma,double L,double* Diameter,double* Distribution,
    double* D21,double* D32,double*D43)
{

   int i,j;
   int iSplineNum;
   qintptr iMaxDis,iMinDis;
   double MaxDis,MinDis;
   double Cv,csu,extv,exts;
   typedef double Row[MaxSize];
   std::unique_ptr<Row[]> Coe(new Row[MaxSize]);

   // 初始化Coe为0
   for (i = 0; i < MaxSize; ++i) {
       for (j = 0; j < MaxSize; ++j) {
           Coe[i][j] = 0.0;
       }
   }

   Cv = 0.01;  //zhangshiwei20230407由0.05修改为0.01
   for(j = 0;j < NumOfDiameter;j++)
      {
         Diameter[j]=LB+(UB-LB)/fabs(NumOfDiameter - 1.0) * j;
         Distribution[j]=0;
      }
       //计算系数矩阵
      for(i = 0;i < NumOfWl;i++){
        for(j = 0;j < NumOfDiameter;j++){
           //export.h
           mymfunction(Diameter[j]/2.0,Cv,F[i],&extv, &csu);
           Coe[i][j] = extv; //总衰减系数
        }
      }

      //全局变量置零
      init0();
      //调用反演函数
      for (iSplineNum=SplineNum;iSplineNum>=SplineNum;iSplineNum--)
      {
          org(NumOfWl,NumOfDiameter,1.0,Coe.get(),Spec,Diameter,Distribution,SplineNum,Gamma);
          //归一化
          normal(Distribution,NumOfDiameter);
          //求特征尺寸
          ParticleInfo2(Diameter,Distribution,NumOfDiameter,D21,D32,D43);
          //labwindows自带，advanlys.h
          qt_MaxMin1D(Distribution,NumOfDiameter,&MaxDis,&iMaxDis,&MinDis,&iMinDis);
      }
      if (*D32 <= 0.0 || !std::isfinite(*D32))
      {
         *D32 = 10.0; // D32为负数/零/NaN无意义，作修正
      }
}

void inversion::ParticleInfo2(double Diameter[],double Distribution[],int NumOfDiameter,
double* D21, double* D32,double* D43)
{
   int i;
   double tem1,tem2,tem3,tem4;
   tem1 = 0;
   tem2 = 0;
   tem3 = 0;
   tem4 = 0;

   for(i = 0;i < NumOfDiameter;i++)
   {
        tem1 = tem1 + Distribution[i] * Diameter[i];
        tem2 = tem2 + Distribution[i] * pow(Diameter[i],2);
        tem3 = tem3 + Distribution[i] * pow(Diameter[i],3);
        tem4 = tem4 + Distribution[i] * pow(Diameter[i],4);
   }
   *D21 = (tem1 > 0.0) ? (tem2 / tem1) : 0.0;
   *D32 = (tem2 > 0.0) ? (tem3 / tem2) : 0.0;
   *D43 = (tem3 > 0.0) ? (tem4 / tem3) : 0.0;
}
void inversion::init0()
{ //初始化为零
    int i,j;
    for(i = 0;i < MaxSize;i++)
    {
       m_Tn[i] = 0;
       for(j = 0;j < MaxSize;j++)
       {
          m_psi[i][j]=0;
          m_psit[i][j]=0;
          m_H[i][j]=0;
          m_HT[i][j]=0;
          m_MA[i][j]=0;
          m_MAT[i][j]=0;
          m_MB[i][j]=0;
          m_ML[i][j]=0;
          m_MLT[i][j]=0;
          m_MC[i][j]=0;
       }
    }
}
//初始化Tn，样条数nc
void inversion::iniort(int nc,int nr,double r[])
{
    int i;
    if (nc <= 1 || nr <= 0) return;
    const double denom = static_cast<double>(nc - 1);
    if (qFuzzyIsNull(denom)) return;
    for(i=0;i<nc;i++)
    {
        m_Tn[i]=r[0]+(r[nr-1]-r[0])/denom*i;
    }
}
double inversion::Bspline(int i, int nc, double t)
//从B1-B3,BSpline计算B样条节点
{
    double b4;
    if(i>=nc-4)
    {
        b4=0.0;return(b4);
    }
    const double denom1 = m_Tn[i+3]-m_Tn[i];
    const double denom2 = m_Tn[i+4]-m_Tn[i+1];
    if (qFuzzyIsNull(denom1) && qFuzzyIsNull(denom2))
        return 0.0;
    b4 = 0.0;
    if (!qFuzzyIsNull(denom1))
        b4 += (t-m_Tn[i])/denom1*B3(i,t);
    if (!qFuzzyIsNull(denom2))
        b4 += (m_Tn[i+4]-t)/denom2*B3(i+1,t);
    return b4;
}
double inversion::B1(int i,double t)
{
    double b1;
    b1=0;
    if((t>= m_Tn[i]) && (t<m_Tn[i+1]))
    {
        b1=1;
    }
    return(b1);
}
double inversion::B2(int i,double t)
{
    double b2 = 0.0;
    const double denom1 = m_Tn[i+1]-m_Tn[i];
    const double denom2 = m_Tn[i+2]-m_Tn[i+1];
    if (!qFuzzyIsNull(denom1))
        b2 += (t-m_Tn[i])/denom1*B1(i,t);
    if (!qFuzzyIsNull(denom2))
        b2 += (m_Tn[i+2]-t)/denom2*B1(i+1,t);
    return(b2);
}
double inversion::B3(int i, double t)
{
    double b3 = 0.0;
    const double denom1 = m_Tn[i+2]-m_Tn[i];
    const double denom2 = m_Tn[i+3]-m_Tn[i+1];
    if (!qFuzzyIsNull(denom1))
        b3 += (t-m_Tn[i])/denom1*B2(i,t);
    if (!qFuzzyIsNull(denom2))
        b3 += (m_Tn[i+3]-t)/denom2*B2(i+1,t);
    return(b3);
}
//---------------------------------------------------------------------------
void inversion::brmul(double a[][MaxSize],double b[][MaxSize],int m,int n,int k,double c[][MaxSize])
//矩阵乘
{
    int i,j,ij;
    for (i=0; i<=m-1; i++)
    {
        for (j=0; j<=k-1; j++)
        {
           c[i][j]=0;
           for(ij=0;ij<n;ij++)
           {
               c[i][j]=c[i][j]+a[i][ij]*b[ij][j];
           }
        }
    }
}


void inversion::org(int nb,int nr,double lt,double coe[MaxSize][MaxSize],double gmeas[],
      double* r,double* distr,int nc,double gm)
{
    //最优正则化计算
    int i,j,k;
    double fai;

    double cai[MaxSize] = {0.0};
    QVector<double> a(MaxSize2, 0.0);

    double rnorm;
    double b[MaxSize] = {0.0};
    double w[MaxSize] = {0.0};
    double zz[MaxSize] = {0.0};
    int indx[MaxSize] = {0};
    int mode;

    for(i = 0;i < MaxSize;i++)
    {
       for(j = 0;j < nc;j++){
          m_psi[i][j] = 0;
          m_psit[i][j] = 0;
       }
    }

    //将原问题转变为样条的线性问题
    iniort(nc,nr,r);
    for(i=0;i<nb;i++){
       for(j=0;j<nc;j++){
          for(k=0;k<nr-1;k++){
             fai = Bspline(j,nc,r[k]);  // using  Bspline base
             m_psi[i][j] += coe[i][k]*fai;
             m_psit[j][i]= m_psi[i][j];
          }
       }
    }

    brmul(m_psit,m_psi,nc,nb,nc,m_MB); //即A'*A

    for(i = 0;i < MaxSize;i++)
    {
       for(j = 0;j < MaxSize;j++)
       {
          m_H[i][j] = 0.0;
          m_HT[i][j] = 0.0;
       }
    }
    for(i=0;i<nc-1;i++)
    {
        for(j=0;j<nc;j++)
        {
         m_H[i][j]=0;
         if(i==j) m_H[i][j]=1; //?
         if(j==i-1) m_H[i][j]=-1; //?MATLAB
         m_HT[j][i] = m_H[i][j];
       }
    }
    brmul(m_HT,m_H,nc,nc-1,nc,m_MC); //即H'*H

    for(i=0;i<nc;i++){
       for(j=0;j<nc;j++){
       a[j*nc+i]=m_MB[i][j]+m_MC[i][j]*gm; //即[A'*A+gamma*H'*H]
       }
    }
    //注：nnls中a是按列将其转换为一维数组
    for(i=0;i<nc;i++)
    {
       for(j=0;j<nb;j++){
          b[i]=b[i]+m_psit[i][j]*gmeas[j];
       }
    }

    nnls(a.data(), nc, nc, nc, b, cai, &rnorm, w, zz, indx, &mode);

    //计算样条将结果转换为分布
    for(j=0;j<nr;j++){
       distr[j]=0.0;
       for(i=0;i<nc;i++) distr[j]+=cai[i]*Bspline(i,nc,r[j]);
       }
}

void inversion::normal(double* xin,int n)
{
    //作归一化
    int i;
    double tot;
    tot = 0;
    for(i=0;i < n;i++)
    {
       tot = tot + xin[i];
    }
    if(tot != 0)
    {
      for(i = 0;i < n;i++)
      {
          xin[i] = xin[i] / tot;
      }
    }
}

double inversion::RR(double rin, double rbar, double k)
{
    double arg1, arg2, arg3, distr;
    // 计算 rin/rbar 的 (k-1) 次方
    arg1 = pow((rin / rbar), (k - 1));
    // 计算 e 的 -(rin/rbar) 的 k 次方
    arg2 = exp(-pow((rin / rbar), k));
    // 计算 k 除以 rbar
    arg3 = k / rbar;
    // 计算最终的分布值
    distr = arg1 * arg2 * arg3;
    return distr; // 返回计算得到的分布值
}

void inversion::Uniform1(double *InputArray, double *OutputArray, int NumOfArray)
{
    double Total = 0.0;

    //advanlys.h里自带
    // 计算输入数组的总和
    qt_Sum1D(InputArray, NumOfArray, &Total);
    // 将输入数组归一化到输出数组
    if (qFuzzyIsNull(Total)) {
        qt_Set1D(OutputArray, NumOfArray, 0.0);
        return;
    }
    qt_LinEv1D(InputArray, NumOfArray, 1.0 / Total, 0, OutputArray);
}

void inversion::ParticleInfo(double Diameter[], double Distribution[], int NumOfDiameter,
                  double* D21, double* D32, double* D43, double* D50, double* D90)
{
   int i;
   double tem1, tem2, tem3, tem4;
   tem1 = 0;
   tem2 = 0;
   tem3 = 0;
   tem4 = 0;
   *D50 = 0;
   *D90 = 0;

   // 计算加权直径的矩
   for(i = 0; i < NumOfDiameter; i++)
   {
        tem1 += Distribution[i] * Diameter[i];
        tem2 += Distribution[i] * pow(Diameter[i], 2);
        tem3 += Distribution[i] * pow(Diameter[i], 3);
        tem4 += Distribution[i] * pow(Diameter[i], 4);
   }
   *D21 = (tem1 > 0.0) ? (tem2 / tem1) : 0.0;  // 计算 D21
   *D32 = (tem2 > 0.0) ? (tem3 / tem2) : 0.0;  // 计算 D32
   *D43 = (tem3 > 0.0) ? (tem4 / tem3) : 0.0;  // 计算 D43

   //advanlys.h里自带
   // 初始化累积分布和体积分布数组
   qt_Set1D(m_CumuDistribution, m_NumOfDiameter, 0);
   qt_Set1D(m_CumuVDistribution, m_NumOfDiameter, 0);

   // 将数目分布转换为体积分布
   for(i = 0; i < NumOfDiameter; i++)
   {
      m_VDistribution[i] = Distribution[i] * pow(Diameter[i], 3);
   }
   normal(m_VDistribution, NumOfDiameter);  // 对体积分布进行归一化处理

   // 计算累积分布并确定 D50 和 D90
   for (i = 1; i < NumOfDiameter; i++)
   {
        m_CumuDistribution[i] = m_CumuDistribution[i - 1] + Distribution[i];
        m_CumuVDistribution[i] = m_CumuVDistribution[i - 1] + m_VDistribution[i];
        if ((m_CumuVDistribution[i - 1] < 0.5) && (m_CumuVDistribution[i] >= 0.5)) *D50 = (Diameter[i - 1] + Diameter[i]) / 2.0;
        if ((m_CumuVDistribution[i - 1] < 0.9) && (m_CumuVDistribution[i] >= 0.9)) *D90 = (Diameter[i - 1] + Diameter[i]) / 2.0;
   }
}


void inversion::CorrectDependent()
{
    QFile file(m_path + "0.txt");  // 打开文件 "0.txt"
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning("Cannot open file 0.txt");
        return;
    }

    QTextStream in(&file);
    double cp3 = 0.0, cp4 = 0.0;
    in >> m_cp[0] >> m_cp[1] >> cp3 >> cp4;  // 读取修正系数
    if (in.status() != QTextStream::Ok) {
        cp3 = 0.0; cp4 = 0.0;
    }
    file.close();

    double SelD, SelK;
    double Total, Total0;
    double MyDistribution[MaxSize];

    Total0 = 1e10;  // 初始化总误差为一个较大的数值

    // 计算 SelD 的适应度
    SelD = m_Diameter[1];  // SelD 的初始值为 Diameter 数组中的第二个元素
    for (int i = 1; i <= m_NumOfDiameter; i++)
    {
        for (int j = 0; j < m_NumOfDiameter; j++)
        {
            // 计算分布 MyDistribution
            MyDistribution[j] = RR(m_Diameter[j], m_Diameter[i], 6);  // 使用函数 RR 计算分布
        }
        Uniform1(MyDistribution, MyDistribution, m_NumOfDiameter);  // 归一化 MyDistribution
        //advanlys.h库
        qt_Sub1D(MyDistribution, m_Distribution, m_NumOfDiameter, MyDistribution);  // 计算 MyDistribution 和 Distribution 的差值
        qt_Abs1D(MyDistribution, m_NumOfDiameter, MyDistribution);  // 计算 MyDistribution 的绝对值
        qt_Sum1D(MyDistribution, m_NumOfDiameter, &Total);  // 计算绝对值的总和
        if (Total < Total0)
        {
            Total0 = Total;  // 更新最小误差
            SelD = m_Diameter[i];  // 更新 SelD
        }
    }

    // 计算 SelK 的适应度
    SelK = 2;  // SelK 的初始值
    for (int i = 20; i <= 25; i++)  // 遍历 K 值的范围
    {
        for (int j = 0; j < m_NumOfDiameter; j++)
        {
            // 计算分布 MyDistribution
            MyDistribution[j] = RR(m_Diameter[j], SelD, i / 10.0);  // 使用函数 RR 计算分布
        }
        Uniform1(MyDistribution, MyDistribution, m_NumOfDiameter);  // 归一化 MyDistribution
        //advanlys.h库
        qt_Sub1D(MyDistribution, m_Distribution, m_NumOfDiameter, MyDistribution);  // 计算 MyDistribution 和 Distribution 的差值
        qt_Abs1D(MyDistribution, m_NumOfDiameter, MyDistribution);  // 计算 MyDistribution 的绝对值
        qt_Sum1D(MyDistribution, m_NumOfDiameter, &Total);  // 计算绝对值的总和
        if (Total < Total0)
        {
            Total0 = Total;  // 更新最小误差
            SelK = i / 10.0;  // 更新 SelK
        }
    }

    // 修正 SelD 和 SelK 的值
    SelD = SelD * m_cp[0] + m_cp[1];  // 根据修正系数修正 SelD
    SelK = SelK * cp3 + cp4;  // 根据修正系数修正 SelK

    // 计算最终的分布
    for (int j = 0; j < m_NumOfDiameter; j++)
    {
        m_Distribution[j] = RR(m_Diameter[j], SelD, SelK);  // 使用修正后的 SelD 和 SelK 计算分布
    }
    Uniform1(m_Distribution, m_Distribution, m_NumOfDiameter);  // 归一化最终的分布
    ParticleInfo(m_Diameter, m_Distribution, m_NumOfDiameter, &m_D21, &m_D32, &m_D43, &m_D50, &m_D90);  // 重新计算平均直径
}

void inversion::MyIniRead() {
    // 初始化
    QString filePath = m_path + "1.txt"; // 数据文件名
    QFile file(filePath);

    // 打开文件 1.txt 进行读取
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning() << "Cannot open file" << filePath;
        return;
    }

    QTextStream in(&file);
    m_NumOfWl = 0;
    int i = 0;
    while (!in.atEnd() && i < MaxSize) {
        double fValue, specValue;
        in >> fValue >> specValue;
        if (in.status() != QTextStream::Ok) break;
        // 滤除NaN/inf数据点
        if (!std::isfinite(fValue) || !std::isfinite(specValue))
            continue;
        m_F[i] = fValue;
        m_Spec[i] = specValue;
        m_NumOfWl = i + 1; // 更新光谱数据点数量
        i++;
    }
    file.close();

    if (m_NumOfWl < 2) {
        qWarning() << "Insufficient valid spectrum points:" << m_NumOfWl;
        return;
    }

    // 直接计算
    m_LB = m_ip[0]; // 读取下限值
    m_UB = m_ip[1]; // 读取上限值
    m_NumOfDiameter = qBound(2, static_cast<int>(m_ip[2]), MaxSize); // 读取直径数量
    m_SplineNum = qBound(2, static_cast<int>(m_ip[3]), MaxSize); // 读取样条数
    m_Gamma1 = qMax(0.0, m_ip[4]); // 读取 Gamma 值
    m_L = qMax(0.0, m_ip[5]); // 读取 L 值

    // 计算粒子分布
    LesOrtJc(m_Spec, m_F, m_NumOfWl, m_LB, m_UB, m_NumOfDiameter, m_SplineNum, m_Gamma1, m_L, m_Diameter, m_Distribution, &m_D21, &m_D32, &m_D43);
    // 进行依赖关系修正
    CorrectDependent();

    // 直接保存结果
    QFile resultFile(m_path + "2.txt");
    if (!resultFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
        qWarning() << "Cannot open file 2.txt";
        return;
    }
    QTextStream out(&resultFile);
    out << "D21= " << m_D21 << "  D32= " << m_D32 << " D43= " << m_D43 << " D50= " << m_D50 << " D90= " << m_D90 << "\n";
    resultFile.close();

    QFile distributionFile(m_path + "3.txt");
    if (!distributionFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
        qWarning() << "Cannot open file 3.txt";
        return;
    }
    QTextStream distOut(&distributionFile);
    for (int i = 0; i < m_NumOfDiameter; i++) {
        distOut << m_Diameter[i] << "  " << m_VDistribution[i] << "  " << m_CumuVDistribution[i] << "\n";
    }
    distributionFile.close();
}

