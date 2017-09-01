
#ifndef INTMATLIB_H_INCLUDED
#define INTMATLIB_H_INCLUDED

#include <string>
#include <iostream>
#include <sstream>
using namespace std;

#include <dirent.h>     // DT_REG
#include <stdlib.h>
#include <sys/stat.h>   // mkdir()
#include <sys/types.h>  // mkdir()


#include "Paths.h"
#include "Utils.h"

#include "arblib/base/ArbitratorInterface.h"

using namespace Arcetri;
using namespace Arcetri::Arbitrator;

extern "C" {
#include "iolib.h"
}

namespace Arcetri {
   namespace IntmatLib {


string generatePPDisturb( int modesNum, int framesNum, int cyclesNum, string m2c, string ampFile, string ampEnvelopeFile);

int calcLen( string disturbFile, int iterations);


int acquireIntmat( ArbitratorInterface *wfsIntf, ArbitratorInterface *adsecIntf,
                    string m2c, string disturbTracknum, string savePath,
                    bool closedloop, int iterations,
                    vector<string> &savedFiles,
                    void (*logCallback)(void *, string)=NULL, void (*warningCallback)(void *, string)=NULL, void *argp=NULL );

int analyseIntmat( string intmatAcqDir, string intmatFile, string recFile, int skipFrame, int avgFrame, int nModes, int cutModes, void (*warningCallback)(void *, string)=NULL, void *argp=NULL);


int genIntmat( string intmatAcqDir, string intmatFile, int skipFrame, int avgFrame, int nModes, void (*warningCallback)(void *, string)=NULL, void *argp=NULL);

int testAmplitude(  string intmatAcqDir, string ampFile, string outputFile, int skipFrame, int avgFrame, int nModes, void (*warningCallback)(void *, string)=NULL, void *argp=NULL);


string acquireIntmat_old( ArbitratorInterface *wfsIntf, ArbitratorInterface *adsecIntf, float loopfreq, string m2c, string cmddisturb, int iterations, void (*logCallback)( string, void *)=NULL, void (*warningCallback)(string, void*)=NULL, void *argp=NULL );
string doCloop(int nFrames);
void sinus_acquire( ArbitratorInterface *wfsIntf, ArbitratorInterface *adsecIntf, int seconds, int nframes, string disturb, string outfile, void (*logCallback)( string, void *)=NULL, void (*warningCallback)(string, void*)=NULL, void *argp=NULL );



int sendCommandWithReply( ArbitratorInterface *intf, Command* cmd, Command **reply=NULL);

int setLoopFreq( ArbitratorInterface *wfsIntf, int binning, float freq, float modulation);
int setRec( ArbitratorInterface *adsecIntf, string m2c, string rec);
int setGain( ArbitratorInterface *adsecIntf,  string m2c, float gain);
int setDisturb( ArbitratorInterface *adsecIntf, string disturb);
int enableDisturb( ArbitratorInterface *wfsIntf, bool wfs, bool ovs);
int openLoop( ArbitratorInterface *wfsIntf);
int closeLoop( ArbitratorInterface *wfsIntf);
string saveOptData( ArbitratorInterface *wfsIntf, int nFrames, int nCcd47, int nIrtc);

} // namespace IntmatLib 
} // namespace Arcetri

#endif // INTMATLIB_H_INCLUDED

