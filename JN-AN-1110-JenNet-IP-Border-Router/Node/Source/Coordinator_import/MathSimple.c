#include "MathSimple.h"
#include "ErrorHandler.h"
//---------------------------------------------------------------------------

#define MATHSIMPLE_E 2.7182818284590451

PRIVATE uint8 m_MathSimplePrecision = 8;
//---------------------------------------------------------------------------

PUBLIC double MathSimplePow (double x, double y){
if (y==0)
	return 1.0;
if (y<0){
	x=1/x;
	y=-y;
}

double expArg = y * MathSimpleLog(x, MATHSIMPLE_E);
return (expArg>=0) ? MathSimpleExp(expArg) : 1/MathSimpleExp(-expArg);
}
//---------------------------------------------------------------------------

PUBLIC double MathSimplePowInt8 (double x, int8 y){
double rez = x;
uint8 i;
if (y == 0)
	return 1.0;
if (y<0){
	x=1/x;
	y=-y;
}
for (i = 0; i < y - 1; i++)
	rez *= x;
return rez;
}
//---------------------------------------------------------------------------

PUBLIC double MathSimpleAbs (double x){
return (x < 0) ? -x : x;
}
//---------------------------------------------------------------------------

PUBLIC uint32 MathSimpleFactorial (uint16 number){
if (number <= 1)
	return 1;
else
	return number * MathSimpleFactorial(number - 1);
}
//---------------------------------------------------------------------------

PUBLIC double MathSimpleExp (double x){
double p = (long)x;
double q = x - p;
double ex = MATHSIMPLE_E;
double y = 1;
long i = 1;
double s = 1;
double a = 1;
double e = MathSimplePowInt8(0.1, m_MathSimplePrecision);

if (x > 0){
	if (p > 0){
		do{
			y *= ex;
			i++;
		}
		while (i <= MathSimpleAbs(p));

		if (p < 0)
			y = 1 / y;
	}

	if (q > 0){
		i = 1;
		do{
			a *= q / i;
			s += a;
			i++;
		}
		while (MathSimpleAbs(a) >= e / y);
		y *= s;
	}
}
else
	return 0;
return y;
}
//---------------------------------------------------------------------------

PUBLIC double MathSimpleLog (double x, double a)
{
if (x <= 0){
	EHCall(MATHSIMPLE_MOD_ID, 1);
	return 0;
}
if (a <=1){
	EHCall(MATHSIMPLE_MOD_ID, 2);
	return 0;
}
double y, z, t, eps;
y = 0.0;
z = x;
t = 1.0;
eps = MathSimplePowInt8(0.1, m_MathSimplePrecision);

while (MathSimpleAbs(t) >= eps || z <= 1.0 / a || z >= a){
    if (z >= a){
        z = z / a;
        y = y + t;
    }
	else if (z <= 1.0 / a){
        z = z * a;
        y = y - t;
    }
	else{
        z = z * z;
        t = t / 2.0;
    }
}
return y;
}
//---------------------------------------------------------------------------
