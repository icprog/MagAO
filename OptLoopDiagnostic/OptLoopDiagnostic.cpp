//@File: OptLoopDiagnostic.cpp
//
// Implementation of the OptLoopDiagnostic class
//@

#include <cstdio>
#include <cstring>

#include <sys/stat.h>   // mkdir()
#include <sys/types.h>

#include <cmath>  // sqrt()
#include <ctime>  // time()


#include "Utils.h"

extern "C" {
#include "base/thrdlib.h"
#include "iolib.h" // WriteFitsFile
#include "base/timelib.h"
}

#include "OptLoopDiagnostic.h"

#include "arblib/wfsArb/WfsInterfaceDefines.h"

#include "arblib/base/CommandsExport.h"
#include "arblib/wfsArb/WfsCommandsExport.h"
#include "arblib/adSecArb/AdSecCommandsExport.h"


using namespace Arcetri;
using namespace Arcetri::Arbitrator;

#define DRIFT_BUFSIZE (500)
#define RMS_BUFSIZE (333*10)


//@Member OptLoopDiagnostic
//
//Standard constructor
//@

OptLoopDiagnostic::OptLoopDiagnostic(int argc, char **argv) throw (AOException) :
    AOApp(argc, argv)
{
   Create();
}

void OptLoopDiagnostic::Create() throw (AOException)
{

   ConfigDictionary().dump();

    // Init shm bufname
    _bufname = ReplaceSide( ConfigDictionary()["RawSharedBase"]) + ":" + (string)ConfigDictionary()["RawSharedName"];

    _logger->log(Logger::LOG_LEV_TRACE, " sizeof(OptLoopDiagFrame)   = %d  [%s:%d]",  sizeof(OptLoopDiagFrame), __FILE__, __LINE__ );

    _driftLogger =  Logger::get( "DRIFT", Logger::LOG_LEV_INFO, "TELEMETRY");
    _countsLogger = Logger::get( "COUNTS", Logger::LOG_LEV_INFO, "TELEMETRY");

    _diagf = NULL;
    _pixelBuf = NULL;
    _slopesBuf = NULL;
    _modesBuf = NULL;
    _ffcommandsBuf = NULL;
    _distaveragesBuf = NULL;
    _framesCounterBuf = NULL;
    _mirrorCounterBuf = NULL;
    _safeSkipFrameCounterBuf = NULL;
    _pendingSkipFrameCounterBuf = NULL;
    _wfsGlobalTimeoutBuf = NULL;
    _flTimeoutBuf = NULL;
    _crcErrorsBuf = NULL;
    _timestampBuf = NULL;
    _loopClosedBuf = NULL;
    _antiDriftBuf = NULL;

    _saving = false;

    _antiDrift = false;
    _darkBuf = NULL;
    _subtractedBuf = NULL;
    _darkFixedBuf = NULL;
    _darkLen = 0;
    _totCounts=0;
    _lastDrift=0;
    _lastBin=-1;
    _darkFilename="";
    _lastDarkFile="";
    _lastLut = "";

    try {
        _driftThreshold = ConfigDictionary()["DriftThreshold"];
        _logger->log( Logger::LOG_LEV_INFO, "Drift threshold set to %5.2f (from configuration file)", _driftThreshold);
    } catch(Config_File_Exception& e) {
	_driftThreshold = 2.0;
        _logger->log( Logger::LOG_LEV_WARNING, "Drift threshold not specified in configuration file, set to default= %5.2f counts", _driftThreshold);
    }

    try {
        _driftMinCorrTime = ConfigDictionary()["DriftMinCorrTime"];
        _logger->log( Logger::LOG_LEV_INFO, "Drift min corr interval set to %d (from configuration file)", _driftMinCorrTime);
    } catch(Config_File_Exception& e) {
	_driftMinCorrTime = 30;
        _logger->log( Logger::LOG_LEV_WARNING, "Drift min corr interval not specified in configuration file, set to default= %d seconds", _driftMinCorrTime);
    }
	
}


//@Member $\sim$ OptLoopDiagnostic
//
//Destructor
//@
OptLoopDiagnostic::~OptLoopDiagnostic()
{
    int stat;
    if( IS_ERROR(stat = bufRelease( (char*)MyFullName().c_str(), _info) ) ) {
        _logger->log(Logger::LOG_LEV_WARNING, "Detaching from shared buffer - %s - Perror(%s)  [%s:%d]",
            lao_strerror(stat), strerror(errno), __FILE__, __LINE__ );
    }

   delete _wfsIntf;
   delete _adsecIntf;
}


void OptLoopDiagnostic::InstallHandlers() {

   _logger->log(Logger::LOG_LEV_INFO, "Installing notification handler for OPTSAVE ...");
   int stat;
   if((stat=thHandler(OPTSAVE, "*", 0, optsave_handler, "optsave", this))<0) {
        _logger->log(Logger::LOG_LEV_FATAL, "Error in installing notification handler for OPTSAVE: %s [%s:%d]",
        lao_strerror(stat),__FILE__, __LINE__);
        throw AOException("thHandler error in installing notification handler for OPTSAVE", stat,__FILE__, __LINE__);
   }
}



//@Member: SetupVars
//
//@
void OptLoopDiagnostic::SetupVars()
{
   try {
      _VarEnableDriftReq = RTDBvar( MyFullName(), "DRIFT.ENABLE", REQ_VAR, INT_VARIABLE, 1);
      _VarEnableDriftCur = RTDBvar( MyFullName(), "DRIFT.ENABLE", CUR_VAR, INT_VARIABLE, 1);
      _VarDriftCur = RTDBvar( MyFullName(), "DRIFT.AMOUNT", NO_DIR, REAL_VARIABLE, 1);
      _VarCountsCur = RTDBvar( MyFullName(), "COUNTS", NO_DIR, INT_VARIABLE, 1);
      _VarSlopeRmsCur = RTDBvar( MyFullName(), "SLOPERMS", NO_DIR, REAL_VARIABLE, 1);

      _VarEnableDriftReq.Set(0);
      Notify( _VarEnableDriftReq, varEnableHdlr);

      _ccd39DarkCur = RTDBvar( ReplaceSide( ConfigDictionary()["ccd39_DarkVarCur"]), CHAR_VARIABLE, 256);
      _ccd39DarkReq = RTDBvar( ReplaceSide( ConfigDictionary()["ccd39_DarkVarReq"]), CHAR_VARIABLE, 256);
      _ccd39BinCur  = RTDBvar( ReplaceSide( ConfigDictionary()["ccd39_BinVarCur"]), INT_VARIABLE, 1);
      _scPixelLutCur= RTDBvar("slopecompctrl."+Side()+".PIXELLUT.CUR", CHAR_VARIABLE, 256);
      _nSubapsCur   = RTDBvar("slopecompctrl."+Side()+".FLUXGAIN.CUR", REAL_VARIABLE, 1);
      _fastLinkEnabled = RTDBvar("slopecompctrl."+Side()+".FASTLINK.ENABLE.CUR", INT_VARIABLE, 1);
      Notify(_ccd39DarkCur, ccd39Changed);
      Notify(_ccd39BinCur,  ccd39Changed, 1);
      Notify(_scPixelLutCur, ccd39Changed, 1);
      Notify(_nSubapsCur,   nSubapsChanged, 1);

      _varSaving = RTDBvar( MyFullName(), "SAVING", NO_DIR, INT_VARIABLE, 1);
      _varSaving.Set(0);

      // Telescope variables

      try {
          _dimmVar = RTDBvar( "AOS.DIMM.SEEING@M_ADSEC", REAL_VARIABLE, 1);
          _windSpeedVar = RTDBvar( "AOS.EXTERN.WINDSPEED@M_ADSEC", REAL_VARIABLE, 1);
          _windDirVar = RTDBvar( "AOS.EXTERN.WINDDIRECTION@M_ADSEC", REAL_VARIABLE, 1);
          _guideCamXVar = RTDBvar( "AOS."+Side()+".GUIDECAM.CENTROID.X@M_ADSEC", REAL_VARIABLE, 1);
          _guideCamYVar = RTDBvar( "AOS."+Side()+".GUIDECAM.CENTROID.Y@M_ADSEC", REAL_VARIABLE, 1);
          Notify(_dimmVar, dimmVarChanged, 1);
          Notify(_windSpeedVar, windSpeedVarChanged, 1);
          Notify(_windDirVar, windDirVarChanged, 1);
          Notify(_guideCamXVar, guideCamVarChanged, 1);
          Notify(_guideCamYVar, guideCamVarChanged, 1);
      } catch (AOException &e) {
          _logger->log(Logger::LOG_LEV_ERROR, "Error attaching to AOS variables");
          _logger->log(Logger::LOG_LEV_ERROR, "No information from AOS will be saved in optical loop data");
      }



    } catch (AOException &e) {
      _logger->log(Logger::LOG_LEV_ERROR, "%s", e.what().c_str());
      _logger->log(Logger::LOG_LEV_ERROR, "Anti-drift will not work");
    }

}

void OptLoopDiagnostic::PostInit()
{
  _wfsIntf = new ArbitratorInterface( "wfsarb."+Side(), Logger::LOG_LEV_DEBUG);
  _adsecIntf = new ArbitratorInterface( "adsecarb."+Side()+"@M_ADSEC", Logger::LOG_LEV_DEBUG);
}

int OptLoopDiagnostic::nSubapsChanged(void* thisPtr, Variable* var) {

   OptLoopDiagnostic *myself = (OptLoopDiagnostic *)thisPtr;
   myself->_nSubapsCur.MatchAndSet(var);
   return NO_ERROR;
}

int OptLoopDiagnostic::dimmVarChanged( void *thisPtr, Variable *var) {
   OptLoopDiagnostic *myself = (OptLoopDiagnostic *)thisPtr;
   myself->addDimmValue( (float) var->Value.Dv[0], var->H.MTime);
   return NO_ERROR;
}

int OptLoopDiagnostic::windSpeedVarChanged( void *thisPtr, Variable *var) {
   OptLoopDiagnostic *myself = (OptLoopDiagnostic *)thisPtr;
   myself->addWindSpeedValue( (float) var->Value.Dv[0], var->H.MTime);
   return NO_ERROR;
}

int OptLoopDiagnostic::windDirVarChanged( void *thisPtr, Variable *var) {
   OptLoopDiagnostic *myself = (OptLoopDiagnostic *)thisPtr;
   myself->addWindDirValue( (float) var->Value.Dv[0], var->H.MTime);
   return NO_ERROR;
}

int OptLoopDiagnostic::guideCamVarChanged( void *thisPtr, Variable *var) {
   double x,y;
   OptLoopDiagnostic *myself = (OptLoopDiagnostic *)thisPtr;
   myself->_guideCamXVar.MatchAndSet(var);
   myself->_guideCamYVar.MatchAndSet(var);
   myself->_guideCamXVar.Get(&x);
   myself->_guideCamYVar.Get(&y);
   myself->addGuideCamValue( x, y, var->H.MTime);
   return NO_ERROR;
}

int OptLoopDiagnostic::addDimmValue( float value, struct timeval mtime) {
   if (_saving) {
      valueTS v = { value, mtime.tv_sec + mtime.tv_usec*1e-6};
      _dimmVector.push_back(v);
   }
   return NO_ERROR;
}

int OptLoopDiagnostic::addWindSpeedValue( float value, struct timeval mtime) {
   if (_saving) {
      valueTS v = { value, mtime.tv_sec + mtime.tv_usec*1e-6};
      _windSpeedVector.push_back(v);
   }
   return NO_ERROR;
}

int OptLoopDiagnostic::addWindDirValue( float value, struct timeval mtime) {
   if (_saving) {
      valueTS v = { value, mtime.tv_sec + mtime.tv_usec*1e-6};
      _windDirVector.push_back(v);
   }
   return NO_ERROR;
}

int OptLoopDiagnostic::addGuideCamValue( float x, float y, struct timeval mtime) {
   if (_saving) {
      guideValueTS v = { x, y, mtime.tv_sec + mtime.tv_usec*1e-6};
      _guideCamVector.push_back(v);
   }
   return NO_ERROR;
}

int OptLoopDiagnostic::ccd39Changed(void* thisPtr, Variable* var) {

   OptLoopDiagnostic *myself = (OptLoopDiagnostic *)thisPtr;
   myself->_ccd39DarkCur.MatchAndSet(var);
   myself->_ccd39BinCur.MatchAndSet(var);
   myself->_scPixelLutCur.MatchAndSet(var);

   // Disable everything when the binning changes
   int bin;
   string lut;
   myself->_ccd39BinCur.Get(&bin);
   lut = myself->_scPixelLutCur.Get();
   if ((myself->_lastBin != bin) || (myself->_lastLut != lut)) {
      myself->StopAntiDrift();
      myself->_VarEnableDriftCur.Set(false, 0, FORCE_SEND);
      myself->_lastBin=bin;
      myself->_lastLut=lut;

      if (myself->_darkBuf)
         free(myself->_darkBuf);
      myself->_darkBuf=NULL;

      myself->_diagf = new OptLoopDiagnClass();
   }

   string darkfile = (std::string) myself->_ccd39DarkCur.Get();

   if ((darkfile.compare(myself->_lastDarkFile)!=0) && (!darkfile.empty())) {
      _logger->log(Logger::LOG_LEV_INFO, "Loading dark: %s", darkfile.c_str());
      myself->reloadDark();
   }

   return NO_ERROR;
}

void OptLoopDiagnostic::reloadDark() {

   _darkFilename = (std::string) _ccd39DarkCur.Get();
   string darkFile = Paths::BackgDir( "ccd39", _lastBin) +"/" + _darkFilename;

   if (_darkBuf)
      free(_darkBuf);
   _darkBuf=NULL;

   _darkBuf = (int16*)ReadFitsFile( (char*)darkFile.c_str(), &_darkLen);
   if (!_darkBuf)
      return;

   if (_subtractedBuf)
      free(_subtractedBuf);
   _subtractedBuf = (int16*)malloc(_darkLen);

   if (_darkFixedBuf)
      free(_darkFixedBuf);
   _darkFixedBuf = (int16*)malloc(_darkLen);
}

float OptLoopDiagnostic::computeSlopeRms()  {

   static float buf[RMS_BUFSIZE];
   int i;

  // Compute average
  float slopeavg=0;
  int slopecount=0;
  int nslopes = 1600;
  float32 *slopes = _diagf->slopes_raw();
  for (i=0; i<nslopes; i++)  {
     float slopevalue = slopes[i];
     if (slopevalue != 0) {
        slopecount++;
        slopeavg += slopevalue;
     }
  }
  if (slopecount==0)
     return 0;

  // Compute stddev
  slopeavg /= slopecount;
  float stddev=0;
  for (i=0; i<nslopes; i++) {
     float slopevalue = slopes[i];
     if (slopevalue != 0)
        stddev += (slopevalue - slopeavg) * (slopevalue - slopeavg);
     }
  stddev = sqrt( stddev / slopecount);

  // Append to data
  memmove( buf, buf+1, sizeof(float)*(RMS_BUFSIZE-1));
  buf[RMS_BUFSIZE-1] = stddev;

  // Compute moving average
  float rms=0;
  for (i=0; i< RMS_BUFSIZE; i++)
     rms += buf[i];
  rms /= RMS_BUFSIZE;

  return rms;



}


// Computes the total drift as a difference from the last background frame
// If <quadrants> is not NULL, it will be used as a float[4] array to store
// the drifts of the four different quadrants

float OptLoopDiagnostic::computeDrift( float *quadrants) {

   static int counter=0;
   static float buf[4][DRIFT_BUFSIZE];
   int i,j, x,y;

   // frame = pixel2d
   // frame -= dark
   // get corners (eventually binned)
   // compute drift
   
   if ((!_darkBuf)  || (_darkLen<1))
      return 0;

    uint16 *pixels = _diagf->pixels_raster();
    int pixelLen = _diagf->pixels_len();

    if ((!pixels) || (pixelLen<1))
       return 0;

    int len = min((int)( _darkLen/sizeof(int16)), pixelLen);

    _totCounts=0;
    for (i=0; i<len; i++) {
      _subtractedBuf[i] = (int)pixels[i] - _darkBuf[i];
      _totCounts += _subtractedBuf[i] + (int)_lastDrift;
    }
    _totCounts /= 2;     // 4 pupils and 2 count/photons

    double nSubaps;
    _nSubapsCur.Get(&nSubaps);
    if (nSubaps != 0) _totCounts /= (int)nSubaps;


    float drifts[4];
    for (i=0; i<4; i++)
	drifts[i]=0;

    int dim = 4/_lastBin;
    if (dim<1) dim=1;
    int w = 80/_lastBin;
    for (x=0; x< dim; x++) 
      for (y=0; y< dim; y++) { 
         drifts[0] += _subtractedBuf[ x     + y*w];
         drifts[1] += _subtractedBuf[ w-x-1 + y*w];
         drifts[2] += _subtractedBuf[ x     + (w-y-1)*w];
         drifts[3] += _subtractedBuf[ w-x-1 + (w-y-1)*w];
      }

    for (i=0; i<4; i++) {
        drifts[i] /= (dim*dim);
    	buf[i][counter % DRIFT_BUFSIZE] = drifts[i];
    }
    counter++;

    float totaldrifts[4];
    float totaldrift = 0;
    for (i=0; i< 4; i++) {
	totaldrifts[i]=0;
        for (j=0; j< DRIFT_BUFSIZE; j++)
            totaldrifts[i] += buf[i][j];
        totaldrifts[i] /= DRIFT_BUFSIZE;

        totaldrift += totaldrifts[i];
	if (quadrants)
        	quadrants[i] = totaldrifts[i];
    }

    totaldrift /=4;

    return totaldrift;
}

void OptLoopDiagnostic::correctDrift(float drift) {

   if ((!_darkBuf)  || (!_darkFixedBuf) || (_darkLen<1))
      return;

   _lastDrift = round(drift);

   for (unsigned int i=0; i<_darkLen/sizeof(int16); i++)
      _darkFixedBuf[i] = _darkBuf[i] + (int)_lastDrift;

   int dx = (int)sqrt(_darkLen/sizeof(int16));
   long dims[] = { dx, dx };

   // Correct for bin3 central four pixels
   if (dx==26) {
        int pos = 12*26+12;
        _darkFixedBuf[pos+0] = 0;
        _darkFixedBuf[pos+1] = 0;
        pos = 13*26+12;
        _darkFixedBuf[pos+0] = 0;
        _darkFixedBuf[pos+1] = 0;
   }


   _lastDarkFile = Utils::asciiDateAndTimeCompact() + "_antidrift.fits";
   string darkPath = Paths::BackgDir( "ccd39", _lastBin) +"/" + _lastDarkFile;

   int stat = WriteFitsFile( (char*)darkPath.c_str(), (unsigned char *)_darkFixedBuf, TSHORT, dims, 2);
   if (IS_ERROR(stat)) 
      _logger->log( Logger::LOG_LEV_ERROR, "Error saving temporary dark file %s: (%d) %s", darkPath.c_str(), stat, lao_strerror(stat));
   else {
      _logger->log( Logger::LOG_LEV_INFO, "Written file: %s", darkPath.c_str());
      _ccd39DarkReq.Set(_lastDarkFile, FORCE_SEND);

      if (_saving) {
        darkApplication da;
        da.counter = _framesCounter;
        da.filename = _lastDarkFile;
        _darkApplications.push_back(da);
      }
   }

}

void OptLoopDiagnostic::correctDrift(float *quadrants) {

   if ((!_darkBuf)  || (_darkLen<1) || (!_darkFixedBuf) || (!quadrants))
      return;

   int dx = (int)sqrt(_darkLen/sizeof(int16));
   int x,y;
   for (x=0; x<dx/2; x++)
	for (y=0; y<dx/2; y++)
		_darkFixedBuf[y*dx+x] = _darkBuf[y*dx+x] + (int) round(quadrants[0]);
   for (x=dx/2; x<dx; x++)
	for (y=0; y<dx/2; y++)
		_darkFixedBuf[y*dx+x] = _darkBuf[y*dx+x] + (int) round(quadrants[1]);
   for (x=0; x<dx/2; x++)
	for (y=dx/2; y<dx; y++)
		_darkFixedBuf[y*dx+x] = _darkBuf[y*dx+x] + (int) round(quadrants[2]);
   for (x=dx/2; x<dx; x++)
	for (y=dx/2; y<dx; y++)
		_darkFixedBuf[y*dx+x] = _darkBuf[y*dx+x] + (int) round(quadrants[3]);

   // Correct for bin3 central four pixels
   if (dx==26) {
        int pos = 12*26+12;
        _darkFixedBuf[pos+0] = 0;
        _darkFixedBuf[pos+1] = 0;
        pos = 13*26+12;
        _darkFixedBuf[pos+0] = 0;
        _darkFixedBuf[pos+1] = 0;
   }


   float dd =0; 
   for (int i=0; i<4; i++)
	dd += quadrants[0];
   _lastDrift = round(dd/4);

   long dims[] = { dx, dx };

   _lastDarkFile = Utils::asciiDateAndTimeCompact() + "_antidrift.fits";
   string darkPath = Paths::BackgDir( "ccd39", _lastBin) +"/" + _lastDarkFile;

   int stat = WriteFitsFile( (char*)darkPath.c_str(), (unsigned char *)_darkFixedBuf, TSHORT, dims, 2);
   if (IS_ERROR(stat)) 
      _logger->log( Logger::LOG_LEV_ERROR, "Error saving temporary dark file %s: (%d) %s", darkPath.c_str(), stat, lao_strerror(stat));
   else {
      _logger->log( Logger::LOG_LEV_INFO, "Written file: %s", darkPath.c_str());
      _ccd39DarkReq.Set(_lastDarkFile, FORCE_SEND);

      if (_saving) {
        darkApplication da;
        da.counter = _framesCounter;
        da.filename = _lastDarkFile;
        _darkApplications.push_back(da);
      }
   }

}


//@Function: optsave_handler
//
// handler of a OPTSAVE command. Start data saving
//@

#include <ctime>
#include <fstream>
int OptLoopDiagnostic::optsave_handler(MsgBuf *msgb, void *argp, int /*hndlrQueueSize*/)
{
   OptLoopDiagnostic *pt = (OptLoopDiagnostic*)argp;

   pt->endSaving();

   optsave_cmd *cmd = (optsave_cmd *) MSG_BODY(msgb);

   pt->startSaving(cmd);
   pt->_client = HDR_FROM(msgb);

   thReplyMsg( OPTSAVE, pt->_tracknum.size()+1, pt->_tracknum.c_str(), msgb);
   return NO_ERROR;
}

int OptLoopDiagnostic::varEnableHdlr( void *ptr, Variable *var) {
   OptLoopDiagnostic *myself = (OptLoopDiagnostic *)ptr;
   int enable = (var->Value.Lv[0] != 0) ? 1 : 0;

   if (enable)
      myself->StartAntiDrift();
   else
      myself->StopAntiDrift();

   myself->_VarEnableDriftCur.Set(enable, 0, FORCE_SEND);
   return NO_ERROR;
}

string OptLoopDiagnostic::selectTrackingNum() {

   return Utils::asciiDateAndTimeCompact();
}


void OptLoopDiagnostic::startSaving( optsave_cmd *cmd) {
   
   _nFrames = cmd->nFrames;
   if (_nFrames<0)
      _nFrames=0;

   _framesCounter = 0;

   _tracknum = selectTrackingNum();

   // Create day directory
   _path = Paths::AdSecDataDir( true, _tracknum);
   mkdir(_path.c_str(), 0777);

   // Create acquisition directory
   _path = Paths::AdSecDataDir(true, _tracknum, _tracknum);
   mkdir(_path.c_str(), 0777);

   _logger->log( Logger::LOG_LEV_DEBUG, "Saving directory: %s\n", _path.c_str());


   _pixelLen = _diagf->pixels_len();

   // Clean previous data, if any
   cleanData();

   if (cmd->saveFrames)
      _pixelBuf = new uint16[ _pixelLen * _nFrames];

   if (cmd->saveSlopes)
      _slopesBuf = new float32[ 1600 * _nFrames];

   if (cmd->saveModes)
      _modesBuf = new float32[ 672 * _nFrames];

   if (cmd->saveFFCommands)
      _ffcommandsBuf = new float32[ 672 * _nFrames];

   _framesCounterBuf  = new uint32[ _nFrames];
   _mirrorCounterBuf  = new uint32[ _nFrames];
   _safeSkipFrameCounterBuf = new uint32[ _nFrames];
   _pendingSkipFrameCounterBuf = new uint32[ _nFrames];
   _wfsGlobalTimeoutBuf = new uint32[ _nFrames];
   _flTimeoutBuf = new uint32[ _nFrames];
   _crcErrorsBuf = new uint32[ _nFrames];
   _timestampBuf = new uint32[ _nFrames];
   _loopClosedBuf = new uint32[ _nFrames];
   _antiDriftBuf = new float32[ 4 * _nFrames];

   if (cmd->saveDistAverages)
      _distaveragesBuf = new float32[ 672 * _nFrames];

   _logger->log( Logger::LOG_LEV_INFO, "Starting save of %d frames", _nFrames);

   // Prepare answer
   _prevTime = time(NULL);
   _prevFrame = 0;
   memset( _reply.outfile, 0, sizeof(_reply.outfile));
   strncpy( _reply.outfile, _path.c_str(), sizeof(_reply.outfile)-1);


   // Save WFS status
 
   int ret;
   if (cmd->saveWfsStatus) {
      Wfs_Arbitrator::saveStatusParams wfsParams;
      wfsParams._outputFile = _path+"wfs.fits";

      Command *wfsCmd = new Wfs_Arbitrator::SaveStatus( 1000, wfsParams);
      if ((ret = sendCommandWithReply( _wfsIntf, wfsCmd)) != NO_ERROR) {
         // [TODO]
      }
   }

   if (cmd->saveAdSecStatus) {
      AdSec_Arbitrator::saveStatusParams adsecParams;
      adsecParams._filename = _path+"adsec.sav";
      Command *adsecCmd = new AdSec_Arbitrator::SaveStatus( 1000, adsecParams);
      if ((ret = sendCommandWithReply( _adsecIntf, adsecCmd)) != NO_ERROR) {
         // [TODO]
      }
   }

   // Initialized variable-length telescope values vector
   _dimmVector.clear();
   _windSpeedVector.clear();
   _windDirVector.clear();
   _guideCamVector.clear();
   _darkApplications.clear();

   _saving = true;
   _varSaving.Set(1);
   if (_nFrames == 0)
	endSaving();

}

void OptLoopDiagnostic::cleanData() {

   if (_pixelBuf) {
      delete[] _pixelBuf;
      _pixelBuf = NULL;
   }

   if (_slopesBuf) {
      delete[] _slopesBuf;
      _slopesBuf = NULL;
   }

   if (_modesBuf) {
      delete[] _modesBuf;
      _modesBuf = NULL;
   }

   if (_ffcommandsBuf) {
      delete[] _ffcommandsBuf;
      _ffcommandsBuf = NULL;
   }

   if (_framesCounterBuf ) {
      delete[] _framesCounterBuf ;
      _framesCounterBuf  = NULL;
   }

   if (_mirrorCounterBuf ) {
      delete[] _mirrorCounterBuf ;
      _mirrorCounterBuf  = NULL;
   }

   if (_distaveragesBuf) {
      delete[] _distaveragesBuf;
      _distaveragesBuf = NULL;
   }

   if (_safeSkipFrameCounterBuf) {
      delete[] _safeSkipFrameCounterBuf;
      _safeSkipFrameCounterBuf = NULL;
   }

   if (_pendingSkipFrameCounterBuf) {
      delete[] _pendingSkipFrameCounterBuf;
      _pendingSkipFrameCounterBuf = NULL;
   }

   if (_wfsGlobalTimeoutBuf) {
      delete[] _wfsGlobalTimeoutBuf;
      _wfsGlobalTimeoutBuf = NULL;
   }

   if (_flTimeoutBuf) {
      delete[] _flTimeoutBuf;
      _flTimeoutBuf = NULL;
   }

   if (_crcErrorsBuf) {
      delete[] _safeSkipFrameCounterBuf;
      _safeSkipFrameCounterBuf = NULL;
   }

   if (_timestampBuf) {
      delete[] _timestampBuf;
      _timestampBuf = NULL;
   }

   if (_loopClosedBuf) {
      delete[] _loopClosedBuf;
      _loopClosedBuf = NULL;
   }

   if (_antiDriftBuf) {
      delete[] _antiDriftBuf;
      _antiDriftBuf = NULL;
   }


}

void OptLoopDiagnostic::endSaving() {

   // Check whether we were called by mistake
   if (!_saving)
      return;


   string framesFilename = _path+"Frames_"+_tracknum+".fits";
   string slopesFilename = _path+"Slopes_"+_tracknum+".fits";
   string modesFilename = _path+"Modes_"+_tracknum+".fits";
   string ffcommandsFilename = _path+"Commands_"+_tracknum+".fits";
   string framesCounterFilename = _path+"FramesCounter_"+_tracknum+".fits";
   string distaveragesFilename = _path+"Positions_"+_tracknum+".fits";
   string safeSkipCounterFilename = _path+"SkipCounter_"+_tracknum+".fits";
   string pendingSkipCounterFilename = _path+"PendingCounter_"+_tracknum+".fits";
   string mirrorCounterFilename = _path+"MirrorCounter_"+_tracknum+".fits";
   string wfsGlobalTimeoutFilename = _path+"WFSGlobalTimeout_"+_tracknum+".fits";
   string flTimeoutFilename = _path+"FlTimeout_"+_tracknum+".fits";
   string crcErrorsFilename = _path+"CrcErrors_"+_tracknum+".fits";
   string timestampFilename = _path+"Timestamp_"+_tracknum+".fits";
   string loopClosedFilename = _path+"LoopClosed_"+_tracknum+".fits";
   string antiDriftFilename  = _path+"AntiDrift_"+_tracknum+".fits";
   string dimmVectorFilename = _path+"Dimm_"+_tracknum+".fits";
   string windSpeedVectorFilename = _path+"WindSpeed_"+_tracknum+".fits";
   string windDirVectorFilename = _path+"WindDir_"+_tracknum+".fits";
   string guideCamVectorFilename = _path+"GuideCam_"+_tracknum+".fits";
   string darkApplicationsFilename = _path+"DarkApplications_"+_tracknum+".txt";
   int stat;

   _logger->log( Logger::LOG_LEV_INFO, "Writing on disk....");

   stat = writeFits2D( (char *)dimmVectorFilename.c_str(), &(_dimmVector[0]), TFLOAT, 2, _dimmVector.size());
   if (IS_ERROR(stat)) {
      _logger->log( Logger::LOG_LEV_ERROR, "Error saving dimm data: (%d) %s", stat, lao_strerror(stat));
   }

   stat = writeFits2D( (char *)windSpeedVectorFilename.c_str(), &(_windSpeedVector[0]), TFLOAT, 2, _windSpeedVector.size());
   if (IS_ERROR(stat)) {
      _logger->log( Logger::LOG_LEV_ERROR, "Error saving wind speed data: (%d) %s", stat, lao_strerror(stat));
   }

   stat = writeFits2D( (char *)windDirVectorFilename.c_str(), &(_windDirVector[0]), TFLOAT, 2, _windDirVector.size());
   if (IS_ERROR(stat)) {
      _logger->log( Logger::LOG_LEV_ERROR, "Error saving wind direction data: (%d) %s", stat, lao_strerror(stat));
   }

   stat = writeFits2D( (char *)guideCamVectorFilename.c_str(), &(_guideCamVector[0]), TFLOAT, 3, _guideCamVector.size());
   if (IS_ERROR(stat)) {
      _logger->log( Logger::LOG_LEV_ERROR, "Error saving guide camera data: (%d) %s", stat, lao_strerror(stat));
   }


   // Save frames
   if (_pixelBuf) {
      int dx = (int)sqrt(_pixelLen);
      stat = writeFits3D( (char *)framesFilename.c_str(), _pixelBuf, TSHORT, dx, dx, _nFrames);
      if (IS_ERROR(stat)) {
         _logger->log( Logger::LOG_LEV_ERROR, "Error saving pixel data");
      }
   }

   if (_slopesBuf) {
      long dims[] = { 1600, _nFrames };
      stat = WriteFitsFile( (char*)slopesFilename.c_str(), (unsigned char *)_slopesBuf, TFLOAT, dims, 2);
      if (IS_ERROR(stat)) {
         _logger->log( Logger::LOG_LEV_ERROR, "Error saving slope data");
      }
   }

   if (_modesBuf) {
      long dims[] = { 672, _nFrames };
      stat = WriteFitsFile( (char*)modesFilename.c_str(), (unsigned char *)_modesBuf, TFLOAT, dims, 2);
      if (IS_ERROR(stat)) {
         _logger->log( Logger::LOG_LEV_ERROR, "Error saving modes data");
      }
   }

   if (_ffcommandsBuf) {
      long dims[] = { 672, _nFrames };
      stat = WriteFitsFile( (char*)ffcommandsFilename.c_str(), (unsigned char *)_ffcommandsBuf, TFLOAT, dims, 2);
      if (IS_ERROR(stat)) {
         _logger->log( Logger::LOG_LEV_ERROR, "Error saving command data");
      }
   }

   if (_framesCounterBuf ) {
      long dims[] = { _nFrames };
      stat = WriteFitsFile( (char*)framesCounterFilename.c_str(), (unsigned char *)_framesCounterBuf , TINT, dims, 1);
      if (IS_ERROR(stat)) {
         _logger->log( Logger::LOG_LEV_ERROR, "Error saving frames counter data");
      }
   }

   if (_mirrorCounterBuf ) {
      long dims[] = { _nFrames };
      stat = WriteFitsFile( (char*)mirrorCounterFilename.c_str(), (unsigned char *)_mirrorCounterBuf , TINT, dims, 1);
      if (IS_ERROR(stat)) {
         _logger->log( Logger::LOG_LEV_ERROR, "Error saving frames counter data");
      }
   }

   if (_distaveragesBuf) {
      long dims[] = { 672, _nFrames };
      stat = WriteFitsFile( (char*)distaveragesFilename.c_str(), (unsigned char *)_distaveragesBuf, TFLOAT, dims, 2);
      if (IS_ERROR(stat)) {
         _logger->log( Logger::LOG_LEV_ERROR, "Error saving command data");
      }
   }
   
   if (_safeSkipFrameCounterBuf) {
      long dims[] = { _nFrames };
      stat = WriteFitsFile( (char*)safeSkipCounterFilename.c_str(), (unsigned char *)_safeSkipFrameCounterBuf , TINT, dims, 1);
      if (IS_ERROR(stat)) {
         _logger->log( Logger::LOG_LEV_ERROR, "Error saving skip frame counter data");
      }
   }

   if (_pendingSkipFrameCounterBuf) {
      long dims[] = { _nFrames };
      stat = WriteFitsFile( (char*)pendingSkipCounterFilename.c_str(), (unsigned char *)_pendingSkipFrameCounterBuf , TINT, dims, 1);
      if (IS_ERROR(stat)) {
         _logger->log( Logger::LOG_LEV_ERROR, "Error saving pending frame counter data");
      }
   }

   if (_wfsGlobalTimeoutBuf) {
      long dims[] = { _nFrames };
      stat = WriteFitsFile( (char*)wfsGlobalTimeoutFilename.c_str(), (unsigned char *)_wfsGlobalTimeoutBuf , TINT, dims, 1);
      if (IS_ERROR(stat)) {
         _logger->log( Logger::LOG_LEV_ERROR, "Error saving wfs global timeout data");
      }
   }

   if (_flTimeoutBuf) {
      long dims[] = { _nFrames };
      stat = WriteFitsFile( (char*)flTimeoutFilename.c_str(), (unsigned char *)_wfsGlobalTimeoutBuf , TINT, dims, 1);
      if (IS_ERROR(stat)) {
         _logger->log( Logger::LOG_LEV_ERROR, "Error saving fastlink timeout data");
      }
   }

   if (_crcErrorsBuf) {
      long dims[] = { _nFrames };
      stat = WriteFitsFile( (char*)crcErrorsFilename.c_str(), (unsigned char *)_crcErrorsBuf , TINT, dims, 1);
      if (IS_ERROR(stat)) {
         _logger->log( Logger::LOG_LEV_ERROR, "Error saving crc errors data");
      }
   }

   if (_timestampBuf) {
      long dims[] = { _nFrames };
      stat = WriteFitsFile( (char*)timestampFilename.c_str(), (unsigned char *)_timestampBuf , TINT, dims, 1);
      if (IS_ERROR(stat)) {
         _logger->log( Logger::LOG_LEV_ERROR, "Error saving timestamp data");
      }
   }

   if (_loopClosedBuf) {
      long dims[] = { _nFrames };
      stat = WriteFitsFile( (char*)loopClosedFilename.c_str(), (unsigned char *)_loopClosedBuf , TINT, dims, 1);
      if (IS_ERROR(stat)) {
         _logger->log( Logger::LOG_LEV_ERROR, "Error saving loop closed data");
      }
   }

   if (_antiDriftBuf) {
      long dims[] = { 4, _nFrames };

      fitskeyword keys[1];
      keys[0].datatype = TSTRING;
      strcpy( keys[0].keyname, "DARK");
      keys[0].value = (void*)_darkFilename.c_str();
      keys[0].comment[0] = '\0';

      stat = WriteFitsFileWithKeywords( (char*)antiDriftFilename.c_str(), (unsigned char *)_antiDriftBuf , TFLOAT, dims, 2, keys, 1);
      if (IS_ERROR(stat)) {
         _logger->log( Logger::LOG_LEV_ERROR, "Error saving antidrift data");
      }
   }

   vector<darkApplication>::iterator iter;
   ofstream of(darkApplicationsFilename.c_str());
   for ( iter = _darkApplications.begin(); iter != _darkApplications.end(); iter++)
       of << iter->counter << " " << iter->filename << "\n";
   of.close();

   // Send final reply
   _reply.saved = _framesCounter;
    thSendMsg( sizeof(_reply), _client.c_str(), OPTSAVE_REPLY, 0, (void*)&_reply);

   _saving = false;
   _varSaving.Set(0);

   _logger->log( Logger::LOG_LEV_INFO, "All data saved correctly");

}


void OptLoopDiagnostic::Run()
{
   // int counter=0;
   float quadrants[4];
   static float lastQuadrants[4];
   static float lastDrift = 0;
   static time_t lastCorrectionTime = 0;

   time_t prevTime= time(NULL);

   while (!TimeToDie() && (_diagf)) {

      GetRawData();
      _diagf->setData( (unsigned char *)&_raw);

      float drift = computeDrift(quadrants);
      float slopeRms = computeSlopeRms();

      time_t now = time(NULL);
      if (prevTime != now) {
          prevTime = now;
         _VarDriftCur.Set(drift-lastDrift);
         _VarCountsCur.Set(_totCounts);
         _VarSlopeRmsCur.Set(slopeRms);

         _driftLogger->log( Logger::LOG_LEV_INFO, "%5.1f (lastDrift %5.1f)", drift, lastDrift);
         _driftLogger->log( Logger::LOG_LEV_INFO, "    Quadrants: %5.1f %5.1f %5.1f %5.1f", quadrants[0], quadrants[1], quadrants[2], quadrants[3]);
         _driftLogger->log( Logger::LOG_LEV_INFO, "lastQuadrants: %5.1f %5.1f %5.1f %5.1f", lastQuadrants[0], lastQuadrants[1], lastQuadrants[2], lastQuadrants[3]);
         _countsLogger->log( Logger::LOG_LEV_INFO, "%d", _totCounts);

	 // if in closed loop minimun interval between corrections is used
	 int isClosedLoop;
	 _fastLinkEnabled.Update();
	 _fastLinkEnabled.Get(&isClosedLoop);

         if ((_antiDrift) && 
	     (!isClosedLoop || (now - lastCorrectionTime > _driftMinCorrTime)) && (
             (fabs(quadrants[0] - lastQuadrants[0]) >= _driftThreshold) ||
             (fabs(quadrants[1] - lastQuadrants[1]) >= _driftThreshold) ||
             (fabs(quadrants[2] - lastQuadrants[2]) >= _driftThreshold) ||
             (fabs(quadrants[3] - lastQuadrants[3]) >= _driftThreshold))) {
            correctDrift(quadrants);
	    lastCorrectionTime = now;
	    for (int i=0; i<4; i++) {
		lastQuadrants[i] = quadrants[i];
		lastDrift = drift;
	    }
          
         }
      }

      // Accumulate data
      if (_saving) {

         _logger->log( Logger::LOG_LEV_INFO, "Getting frame %d of %d", _framesCounter+1, _nFrames);

         if (_pixelBuf) {
            int pt = _framesCounter * _pixelLen;
            memcpy( _pixelBuf +pt,  _diagf->pixels_raster(), _pixelLen * sizeof(uint16));
         }

         if (_slopesBuf) {
            int pt = _framesCounter * 1600;
            memcpy( _slopesBuf +pt,  _diagf->slopes_raw(), 1600 * sizeof(float32));
         }

         if (_modesBuf) {
            int pt = _framesCounter * 672;
            memcpy( _modesBuf +pt,  _diagf->modes(), 672 * sizeof(float32));
         }

         if (_ffcommandsBuf) {
            int pt = _framesCounter * 672;
            memcpy( _ffcommandsBuf +pt,  _diagf->ffcommands(), 672 * sizeof(float32));
         }

         if (_framesCounterBuf ) {
            int pt = _framesCounter;
            _framesCounterBuf[pt] =  _diagf->framesCounter();
         }

         if (_mirrorCounterBuf ) {
            int pt = _framesCounter;
            _mirrorCounterBuf[pt] =  _diagf->mirrorFrameCounter();
         }

         if (_distaveragesBuf) {
            int pt = _framesCounter * 672;
            memcpy( _distaveragesBuf +pt,  _diagf->distaverages(), 672 * sizeof(float32));
         }

         if (_safeSkipFrameCounterBuf) {
            int pt = _framesCounter;
            _safeSkipFrameCounterBuf[pt] = _diagf->skipFrameCounter();
         }

         if (_pendingSkipFrameCounterBuf) {
            int pt = _framesCounter;
            _pendingSkipFrameCounterBuf[pt] = _diagf->pendingFrameCounter();
         }

         if (_wfsGlobalTimeoutBuf) {
            int pt = _framesCounter;
            _wfsGlobalTimeoutBuf[pt] = _diagf->wfsGlobalTimeout();
         }

         if (_flTimeoutBuf) {
            int pt = _framesCounter;
            _flTimeoutBuf[pt] = _diagf->flTimeout();
         }

         if (_crcErrorsBuf) {
            int pt = _framesCounter;
            _crcErrorsBuf[pt] = _diagf->crcErrors();
         }

         if (_timestampBuf) {
            int pt = _framesCounter;
            _timestampBuf[pt] = _diagf->timestamp();
         }

         if (_loopClosedBuf) {
            int pt = _framesCounter;
            _loopClosedBuf[pt] = (_diagf->isLoopClosed() ) ? 1 : 0;
         }

         if (_antiDriftBuf) {
            int pt = _framesCounter * 4;
            _antiDriftBuf[pt+0] = quadrants[0];
            _antiDriftBuf[pt+1] = quadrants[1];
            _antiDriftBuf[pt+2] = quadrants[2];
            _antiDriftBuf[pt+3] = quadrants[3];
         }

         // Progress info
         time_t now = time(NULL);
         if (_prevTime != now) {
            _prevTime = now;

            _reply.saved = _framesCounter+1;
            _reply.speed = _framesCounter- _prevFrame;

            thSendMsg( sizeof(_reply), _client.c_str(), OPTSAVE_REPLY, 0, (void*)&_reply);

            _prevFrame = _framesCounter;
         }


         // Write out data
         if ( ++_framesCounter >= _nFrames)
            endSaving();
      } 

   }


}

void OptLoopDiagnostic::Dump() 
{
   static int counter =0;
   char filename[64];

   if (!_diagf)
      return;

   counter++;
   sprintf(filename, "/tmp/frame%d", counter);

   FILE *fp = fopen(filename, "w");
   unsigned int i,j;


   uint32 *start = (uint32 *) _diagf->data();
   uint32 *modes = (uint32 *) _diagf->modes();
   uint32 *cmd = (uint32 *) _diagf->ffcommands();
   uint32 *dist = (uint32 *) _diagf->distaverages();
   fprintf( fp, "\n\n----------------------\n");
   fprintf( fp, "Modes start: %08X\n", (modes - start));
   fprintf( fp, "Cmd start: %08X\n", (cmd - start));
   fprintf( fp, "Dist start: %08X\n", (dist - start));
   for (i=0; i<sizeof(OptLoopDiagFrame)/sizeof(uint32); ) {
      fprintf(fp, "%08X:", i);
      for (j=0; j<4; j++)
         fprintf(fp, " %08X", ((uint32 *) _diagf->data())[i+j]);
      fprintf(fp, "\n");
      i += j;
   }
   fclose(fp);
}
      


//@Member: GetRawData
//
//@
void OptLoopDiagnostic::GetRawData()
{
    int  stat;
    bool loop_again = true;
    int timeout = 1000;

    while(loop_again && TimeToDie() == false) {

         stat = getOptLoopRawData( MyFullName(), _bufname, &_raw, timeout, false, &_info);

			if (IS_ERROR(stat)) 
            msleep(1000);
         else
            loop_again=false;

        }

    //Dump();
}


int OptLoopDiagnostic::sendCommandWithReply( ArbitratorInterface *intf, Command* cmd, Command **reply) {

   Command* cmdReply;
   int success;

   try {
      _logger->log(Logger::LOG_LEV_INFO, " >> Sending command %s request...", cmd->getDescription().c_str());
      cmdReply = intf->requestCommand(cmd);
      _logger->log(Logger::LOG_LEV_INFO, " >> Command reply (%s) received!", cmdReply->getStatusAsString().c_str());
      cmdReply->log();
      success = cmdReply->isSuccess();
      if (reply)
         *reply = cmdReply;
      else
         delete cmdReply;
   }
   catch(...) {
      _logger->log(Logger::LOG_LEV_ERROR, "Command %s error", cmd->getDescription().c_str());
      delete cmd;
      return -1;
   }

   delete cmd;

   if (!success)
      return -1;
   else
      return NO_ERROR;

}

string OptLoopDiagnostic::ReplaceSide( string s) {
    size_t pos;
    while ((pos = s.find("<side>")) != std::string::npos)
        s.replace(pos, 6, Side());
    return s;
}













////////////////////////////////////////
// Versioning information
// 
int         VersMajor     = 0;
int         VersMinor     = 1;
const char  *Date         = "Jun 2009";
const char  *Author       = "A. Puglisi";


static void help()
{
    printf("\nOptLoopDiagnostic  - Vers. %d.%d.  %s, %s\n\n", VersMajor,VersMinor,Author,Date);
    printf("Usage: OptLoopDiagnostic [-v] [-f <config_file>]\n\n");
    printf("   -f    specify configuration file (defaults to fastdiagn.conf)\n");
    printf("   -v    verbose mode (add more -v to increase verbosity)\n");
    printf("\n");
}


// +Main: 
//
// Main function
//-

int main(int argc, char **argv) 
{
    OptLoopDiagnostic *app=NULL;

    SetVersion(VersMajor,VersMinor);

    try{
        app = new OptLoopDiagnostic(argc, argv);        
        app->Exec();
        delete app;
    } catch (AOException &e) {
        Logger::get()->log(Logger::LOG_LEV_FATAL, "%s", e.what().c_str());
        if(app) delete app;
    } catch (...) {
        Logger::get()->log(Logger::LOG_LEV_FATAL, "unknown exception");
        if(app) delete app;
    }
}    
