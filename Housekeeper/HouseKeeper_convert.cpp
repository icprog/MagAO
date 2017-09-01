#include "HouseKeeper_convert.h"

//@@Method{SVP}: Saturation Vapour Pressure
// Computes the saturation vapour pressure over water in Pa.
//
// It is valid in the range 0-100C, it is but used beyond this
// range.
// Translated from Guido Brusa's svp.pro
//@
double FunctDewPoint::SVP(double tC) //@P{tC}: temperature in C
                                     //@R: saturation vapour pressure in Pa
{
    double t = tC + 273.15;
    return exp( -2.9912729e3/(t*t) - 6.0170128e3/t + 1.887643854e1 -2.8354721e-2*t + 1.7838301e-5*t*t 
        -8.4150417e-10*t*t*t + 4.4412543e-13*t*t*t*t + 2.858487*log(t) ) ;
}

//@@Method{DewPoint}: Dew Point
//  computes the dew and frost point
//
//  Input is vapour pressure vp = vps*(hr/100.0), where vps is saturation vapour
//  pressure in Pa and hr is relative humidity in percentage (0-100)
// 
//  @see{SVP}
//  Translated from Guido Brusa's dew_point.pro
//@

double FunctDewPoint::DewPoint(double vp) //@P{vp}: vapour pressure in Pa
                                          //@R    : dew point temperature in C
{
    return ( 2.0798233e2 - 2.0156028e1*log(vp) + 4.6778925e-1*pow(log(vp),2) -9.2288067e-6 * pow(log(vp),3) ) / 
    ( 1 -1.3319669e-1*log(vp) + 5.6577518e-3*pow(log(vp),2) -7.5172865e-5 * pow(log(vp),3) )  - 273.15;
}
