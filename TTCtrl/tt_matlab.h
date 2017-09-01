
#ifndef TT_MATLAB_H
#define TT_MATLAB_H

void *matlab_init(void);
void matlab_setTT( void *p, double x, double y);
void matlab_setCL( void *p, double x, double y);
void matlab_setMod( void *p, double freq, double amp);
void matlab_rip( void *p);

#endif // TT_MATLAB_H

