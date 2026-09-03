#include <cvidef.h>

int DLLIMPORT myvar;


int DLLEXPORT __stdcall myaddfunction(int, int);
void DLLEXPORT __stdcall mymfunction(double r,double cv,double f, double* att, double* csu);
void DLLEXPORT __stdcall ini(double MM[],double MP[]);

void DLLEXPORT __stdcall myinverse(double* Spec,double* F,int NumOfWl,double LB,double UB,int NumOfDiameter,
    int SplineNum,double Gamma,double L,double* Diameter,double* Distribution,
    double* D21,double* D32,double*D43);


