//+File: eev39.h
//
// Header file for eev39.c
//-

#ifndef EEV39_H_INCLUDED
#define EEV39_H_INCLUDED

int LoadEEV39Tables( void);
int EEV39toFrame( int *from, int *to, int ccd_dim);
int FreeEEV39Tables( void);

#endif // EEV39_H_INCLUDED

