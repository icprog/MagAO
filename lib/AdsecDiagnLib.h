// @File: AdsecDiagnLib.h
//
// Definitions and prototypes for the adsec diagnostic
//
// These functions are intended for consumers of Fast Diagnostic information.
//
// HISTORY
// 2009 Created by Alfio Puglisi (AP) <puglisi@arcetri.astro.it>  deriving from FastDiagnostic code
// 
//@

#ifndef ADSECDIAGNLIB_H_INCLUDE

#define ADSECDIAGNLIB_H_INCLUDE

#include "AdSecConstants.h"
#include "aotypes.h"

using namespace Arcetri::AdSecConstants;

#include "AdsecDiagnLibStructs.h"
#include "bcu_diag.h"

#include "AOExcept.h"


// --------------- INTERFACE FUNCTIONS -------------------


int getFastDiagnosticRawData( string MyName, string bufname, AdsecDiagnRaw *rawData, int timeout, bool loop=true, BufInfo **info=NULL);

int getOptLoopRawData( string MyName, string bufname, OptLoopDiagFrame *rawData, int timeout, bool loop=true, BufInfo **info=NULL);


// --------------- Internal functions -------------

void attachToFastDiagnostic( string MyName, string bufname, BufInfo **info, int bufLen = sizeof(AdsecDiagnRaw)) throw (AOShMemException);
void TryAttachRawShMem( string MyName, string bufname, BufInfo *info, int bufLen = sizeof(AdsecDiagnRaw)) throw (AOShMemException);



#endif //ADSECDIAGNLIB_H_INCLUDE
