#ifndef UTMATCH_H
#define UTMATCH_H
#include <qmath.h>

const double PI = 3.141592653;


class Math
{
public:
    //计算一个声程数组的最大声程；
    static double Mymax(double *t, int N)
    {
        double tmax = *t;
        for (int i = 0; i < N; i++)
        {
            if (*(t+i) > tmax)
                tmax = *(t+i);
        }
        return tmax;
    }
    static double Mymin(double *t, int N)
    {
        double tmin = *t;
        for (int i = 0; i < N; i++)
        {
            if (t[i] < tmin)
                tmin = t[i];
        }
        return tmin;
    }
    static double Mintime(double *t, double x1, double y1, double x2, double y2, double end, double start, double v1,double v2)
    {
        double T;
        int i=0;
        if(end<start)
        {
            int tnum = static_cast<int>(start - end) + 1;
            for (int n = 0; n < tnum; n++)
            {
                double k = start - static_cast<double>(n);
                t[i]=sqrt(pow((x1-k),2)+pow((y1),2))/v1+sqrt(pow((x2-k),2)+pow((y2),2))/v2;
                i=i+1;
            }
            T=Mymin(t, tnum);
        }
        else
        {
            for (double k = start; k <= end; k=k+1)
            {
                /*double tt1, tt2, tt3, tt4, tt5, tt6, tt7;
                tt1 = pow((x1-k),2);
                tt2 = pow((y1),2);
                tt3 = pow((x2-k),2);
                tt4 = pow((y2),2);
                tt5 = sqrt(tt3+tt4)/v2;
                tt6 = sqrt(tt1+tt2)/v1;
                //t[i]=sqrt(pow((x1-k),2)+pow((y1),2))/v1+sqrt(pow((x2-k),2)+pow((y2),2))/v2;
                tt7 = tt5+tt6;
                t[i] = tt7;
                */
                t[i]=sqrt(pow((x1-k),2)+pow((y1),2))/v1+sqrt(pow((x2-k),2)+pow((y2),2))/v2;

                i=i+1;
            }
            int tnum=(end-start)/1+1;
            T=Mymin(t, tnum);
        }
        return T;
    }
    /**
        * 将分贝转换成倍数
        */
    static double DDB2Multi(double ddB)
    {
            double multi;

            multi = pow(10, ddB / 20.0);
            return multi;
    }

    /**
    * 将倍数转换成分贝
    */
    static double Multi2ddB(double mult)
    {
        double d;
        d = log10(mult) * 20;
        return d;
    }
    /*
    * 浮点数转换成16位Q12的定点数
    * val		: 待转换的浮点数
    * Q		: 定点数的小数点后二进制位数，如若Q=10，则称为Q12定点数
    * return	: 转换后的16位定点数
    */
    static unsigned short Float2Fix16(double val, unsigned int Q)
    {
        //	#define Q	12
        int fixp;
        unsigned short res;

        fixp = val * (1 << Q);
        res = fixp & 0xFFFF;
        return res;
    }
    //保留小数点后一位小数
    static void FixAngle(double &angle)
    {
        angle = ((int)(angle*10))/10.0;
    }
    static int MMtoPX(double mm ,double width_mm,unsigned int width_px)
    {
        int tem = (int)((mm/width_mm)*width_px + 0.5);
        tem = tem > (int)width_px ? (int)width_px : tem;
        return tem;
    }
    static double PXtoMM(unsigned int px ,double width_mm,unsigned int width_px)
    {
        double tem = (((double)px/width_px)*width_mm /*+ 0.5*/);
        tem = tem > width_mm ? width_mm : tem;
        return tem;
    }
    static unsigned int MMtoPT(double mm , double vel,int sampling_fre)
    {
        double ns = (mm * 2 / vel)*1000000.0;
        unsigned int pt = ns / sampling_fre + 0.5;

        return pt;
    }
    static double PTtoMM(unsigned int pt , double vel,int sampling_fre)
    {
        double ns = pt * sampling_fre;
        double mm = (ns / 1000000.0) * vel / 2 /*+ 0.5*/;

        return mm;
    }
    static unsigned int MMtoNS(double mm , double vel,int sampling_fre)
    {
		(void)sampling_fre;
        unsigned int ns = (mm * 2 / vel)*1000000.0;

        return ns;
    }
    static double NStoMM(int ns, double vel,int sampling_fre)
    {
		(void)sampling_fre;
        double mm = (ns * vel) / (1000000.0 * 2);
        return mm;
    }

    static bool ValidityCheck(double &val,double max,double min)
    {
        bool ret = false;
        if(val > max)
        {
            val = max;
            ret = false;
        }else
        if(val < min)
        {
            val = min;
            ret = false;
        }else
            ret = true;

        return ret;
    }
    static bool ValidityCheck(int &val,int max,int min)
    {
        bool ret = false;
        if(val > max)
        {
            val = max;
            ret = false;
        }else
        if(val < min)
        {
            val = min;
            ret = false;
        }else
            ret = true;

        return ret;
    }
    static bool ValidityCheckCopy(double &outVal,const double inVal,const double max,const double min)
    {
        bool ret = false;

        if(inVal > max)
        {
            ret = false;
        }else
        if(inVal < min)
        {
            ret = false;
        }else
        {
            outVal = inVal;
            ret = true;
        }

        return ret;
    }
    static bool ValidityCheckCopy(int &outVal,const int inVal,const int max,const int min)
    {
        bool ret = false;

        if(inVal > max)
        {
            ret = false;
        }else
        if(inVal < min)
        {
            ret = false;
        }else
        {
            outVal = inVal;
            ret = true;
        }

        return ret;
    }


//public:
//    static bool ValidityCheck(Type::CustomType<double> &val,double max,double min)
//    {
//        bool ret = false;
//        if(val > max)
//        {
//            val = max;
//            ret = false;
//        }else
//        if(val < min)
//        {
//            val = min;
//            ret = false;
//        }else
//            ret = true;

//        return ret;
//    }
//    static bool ValidityCheck(Type::CustomType<int> &val,int max,int min)
//    {
//        bool ret = false;
//        if(val > max)
//        {
//            val = max;
//            ret = false;
//        }else
//        if(val < min)
//        {
//            val = min;
//            ret = false;
//        }else
//            ret = true;

//        return ret;
//    }
    /* ya[i] is the function value at xa[i], for i = 1..n.  We will compute
       y[i], the estimate of the function at x[i], for i = 1..nelem, using
       a natural spline interpolation on the xa,ya.
       Note that for values of x[i] outside the range (x[0], x[n-1]),
       the values of y[i] will be extrapolated and can be quite wild.

       Phil Hodge, 1997 Oct 22:
        Functions created.
    */

    static int splint_nr (double *xa, double *ya, int n,
                            double *x, double *y, int nelem)
    {

        double *ua;		/* scratch space */
        double *y2a;		/* array of second derivatives at xa */
        double a, b, h;
        int i;
        int k, klo, khi;

        ua = (double *)calloc (n, sizeof (double));
        y2a = (double *)calloc (n, sizeof (double));
        if (ua == NULL || y2a == NULL)
            //return (111);
            goto over;

        /* Compute the spline fit for xa vs ya. */
        spline_nr (xa, ya, y2a, ua, n);

        /* Evaluate the fit at each element of x. */
        for (i = 0;  i < nelem;  i++) {

            /* Find the segment containing x[i]. */
            klo = 0;
            khi = n - 1;
            while (khi - klo > 1) {
            k = (klo + khi) / 2;
            if (xa[k] > x[i])
                khi = k;
            else
                klo = k;
            }

            /* Evaluate. */

            h = xa[khi] - xa[klo];
            if (h == 0.)
               // break;
                return (113);

            a = (xa[khi] - x[i]) / h;
            b = (x[i] - xa[klo]) / h;

            y[i] = a * ya[klo] + b * ya[khi] +
            ((a*a*a - a) * y2a[klo] + (b*b*b - b) * y2a[khi]) * h * h / 6.;
        }
over:
        if(ua!=NULL)
            free (ua);
        if(y2a!=NULL)
            free (y2a);

        return (0);
    }

    static void spline_nr (double *xa, double *ya, double *y2a, double *ua, int n)
    {

    /* arguments:
    double xa[n]     i: array of X coordinates
    double ya[n]     i: Y values at corresponding xa
    double y2a[n]    o: second derivatives needed by splint
    double ua[n]     o: scratch space
    int n            i: size of arrays
    */

        double sig, p;
        int i;

        /* "natural" spline */
        y2a[0] = 0.;
        y2a[n-1] = 0.;

        /* This is the decomposition loop of the tridiagonal algorithm.
           y2a and ua are used for temporary storage of the decomposed factors.
        */
        for (i = 1;  i < n-1;  i++) {
            sig = (xa[i] - xa[i-1]) / (xa[i+1] - xa[i-1]);
            p = sig * y2a[i-1] + 2.;
            y2a[i] = (sig - 1.) / p;
            ua[i] = (6. *
            ((ya[i+1] - ya[i]) / (xa[i+1] - xa[i]) -
             (ya[i] - ya[i-1]) / (xa[i] - xa[i-1])) /
            (xa[i+1] - xa[i-1]) - sig * ua[i-1]) / p;
        }

        /* This is the backsubstitution loop of the tridiagonal algorithm. */
        for (i = n-2;  i >= 0;  i--)
            y2a[i] = y2a[i] * y2a[i+1] + ua[i];
    }

    static int find_nearest(double *array,int array_size, double targetVal)
    {
        if (array_size <= 0)
            return -1;

        int index = 0;

        for(index = 0; index < array_size-1; index++)
        {
            if(targetVal >= array[index] && targetVal < array[index+1])
            {
                if((targetVal - array[index]) > (array[index+1] - targetVal))
                    index = index+1;
                break;
            }
        }

        // 兜底: 未找到区间时返回最接近的端点
        if (index >= array_size - 1) {
            if (targetVal <= array[0])
                index = 0;
            else
                index = array_size - 1;
        }


        return index;
    }
};


#endif // UTMATCH_H
