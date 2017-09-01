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

#include "OptLoopDiagnostic_nuvola.h"

#include "arblib/wfsArb/WfsInterfaceDefines.h"

#include "arblib/base/CommandsExport.h"
#include "arblib/wfsArb/WfsCommandsExport.h"
#include "arblib/adSecArb/AdSecCommandsExport.h"


using namespace Arcetri;
using namespace Arcetri::Arbitrator;

#define COUNTS_BUFSIZE (10)


//@Member OptLoopDiagnostic
//
//Standard constructor
//@

OptLoopDiagnostic::OptLoopDiagnostic(const std::string& conffile) throw (AOException) :
    AOApp(conffile)
{
   Create();
}

OptLoopDiagnostic::OptLoopDiagnostic(int argc, char **argv) throw (AOException) :
    AOApp(argc, argv)
{
   Create();
}

void OptLoopDiagnostic::Create() throw (AOException)
{
   printf("Create\n");

   ConfigDictionary().dump();

    // Init shm bufname
    _bufname = (string)ConfigDictionary()["RawSharedBase"] + ":" + (string)ConfigDictionary()["RawSharedName"];
    _initialGainFile = (string)ConfigDictionary()["initialGainFile"];

    _logger->log(Logger::LOG_LEV_TRACE, " sizeof(OptLoopDiagFrame)   = %d  [%s:%d]",  sizeof(OptLoopDiagFrame), __FILE__, __LINE__ );

    _diagf = NULL;

    _darkBuf = NULL;
    _subtractedBuf = NULL;
    _driftFixedBuf = NULL;
    _darkLen = 0;
    _totCounts=0;
    _lastBin=-1;
    _darkFilename="";

    printf("Done create\n");
}


//@Member $\sim$ OptLoopDiagnostic
//
//Destructor
//@
OptLoopDiagnostic::~OptLoopDiagnostic()
{
    int stat;
    if( IS_ERROR(stat = bufRelease( (char*)MyName().c_str(), _info) ) ) {
        _logger->log(Logger::LOG_LEV_WARNING, "Detaching from shared buffer - %s - Perror(%s)  [%s:%d]",
            lao_strerror(stat), strerror(errno), __FILE__, __LINE__ );
    }

   delete _adsecIntf;
}



//@Member: ReadConfig
//
//@
void OptLoopDiagnostic::ReadConfig()
{
    _cfg=ConfigDictionary();

    // insert stuff here
}

void OptLoopDiagnostic::InstallHandlers() {

}



//@Member: SetupVars
//
//@
void OptLoopDiagnostic::SetupVars()
{
   int stat;

   try {
      _ccd39DarkCur = RTDBvar((std::string) ConfigDictionary()["ccd39_DarkVarCur"], CHAR_VARIABLE, 256);
      _ccd39BinCur  = RTDBvar((std::string) ConfigDictionary()["ccd39_BinVarCur"], INT_VARIABLE, 1);
      Notify(_ccd39DarkCur, ccd39Changed);
      Notify(_ccd39BinCur,  ccd39Changed);
 
      // First value
      MsgBuf *msgb = thGetVar( _ccd39BinCur.complete_name().c_str(), 0, &stat);
      if (msgb) {
         ccd39Changed( this, thValue(msgb));
         thRelease(msgb);
      }

      int stat;
      msgb = thGetVar( _ccd39DarkCur.complete_name().c_str(), 0, &stat);
      if (msgb) {
         ccd39Changed( this, thValue(msgb));
         thRelease(msgb);
      }

    } catch (AOException &e) {
      _logger->log(Logger::LOG_LEV_ERROR, "%s", e.what().c_str());
      _logger->log(Logger::LOG_LEV_ERROR, "Anti-drift will not work");
    }

}

void OptLoopDiagnostic::PostInit()
{
  _adsecIntf = new ArbitratorInterface( "ADSECARB00", Logger::LOG_LEV_DEBUG);
}

int OptLoopDiagnostic::ccd39Changed(void* thisPtr, Variable* var) {

   OptLoopDiagnostic *myself = (OptLoopDiagnostic *)thisPtr;
   myself->_ccd39DarkCur.MatchAndSet(var);
   myself->_ccd39BinCur.MatchAndSet(var);

   // Disable everything when the binning changes
   int bin;
   myself->_ccd39BinCur.Get(&bin);
   if (myself->_lastBin != bin) {
      myself->_lastBin=bin;

      if (myself->_darkBuf)
         free(myself->_darkBuf);
      myself->_darkBuf=NULL;

      myself->_diagf = new OptLoopDiagnClass( Side());
   }

   string darkfile = (std::string) myself->_ccd39DarkCur.Get();

   if (!darkfile.empty()) {
      _logger->log(Logger::LOG_LEV_INFO, "Loading dark: %s", darkfile.c_str());
      myself->reloadDark();
   }

   return NO_ERROR;
}

void OptLoopDiagnostic::reloadDark() {

   int bin;

   _ccd39BinCur.Get(&bin);
   _darkFilename = (std::string) _ccd39DarkCur.Get();
   string darkFile = Paths::BackgDir( Side(), "ccd39", bin) +"/" + _darkFilename;

   if (_darkBuf)
      free(_darkBuf);
   _darkBuf=NULL;

   _darkBuf = (int16*)ReadFitsFile( (char*)darkFile.c_str(), &_darkLen);
   if (!_darkBuf)
      return;

   if (_subtractedBuf)
      free(_subtractedBuf);
   _subtractedBuf = (int16*)malloc(_darkLen);

   if (_driftFixedBuf)
      free(_driftFixedBuf);
   _driftFixedBuf = (int16*)malloc(_darkLen);
}

float OptLoopDiagnostic::computeCounts() {

   static int counter=0;
   static float buf[COUNTS_BUFSIZE];
   int i;

   if ((!_darkBuf)  || (_darkLen<1))
      return 0;

    uint16 *pixels = _diagf->pixels_raster();
    int pixelLen = _diagf->pixels_len();

    if ((!pixels) || (pixelLen<1))
       return 0;

    int bin;
    _ccd39BinCur.Get(&bin);

    int len = min((int)( _darkLen/sizeof(int16)), pixelLen);

    _totCounts=0;
    for (i=0; i<len; i++) {
      _subtractedBuf[i] = (int)pixels[i] - _darkBuf[i];
      _totCounts += _subtractedBuf[i];
    }
    _totCounts /= 8;     // 4 pupils and 2 count/photons

    buf[counter++ % COUNTS_BUFSIZE] = _totCounts;

    float totalcounts = 0;
    for (i=0; i< COUNTS_BUFSIZE; i++)
       totalcounts += buf[i];
    totalcounts /= COUNTS_BUFSIZE;

    return totalcounts;
}


void OptLoopDiagnostic::setGainZero() {

   int ret;
   try {
      AdSec_Arbitrator::gainParams gainPar;
      gainPar._gainFile = "/towerdata/adsec_calib/M2C/KL/gain/gain0.fits";
      Command *gainCmd = new AdSec_Arbitrator::SetGain( 1000, gainPar);
      if ((ret = sendCommandWithReply( _adsecIntf, gainCmd)) != NO_ERROR) {
         _logger->log(Logger::LOG_LEV_ERROR, "Error setting zero gain: %s", lao_strerror(ret));
      }
   }
   catch (AOException *e) {
      _logger->log(Logger::LOG_LEV_ERROR, "Error setting zero gain: %s", e->what().c_str());
   }
}



void OptLoopDiagnostic::setInitialGain() {

   int ret;
   try {
      AdSec_Arbitrator::gainParams gainPar;
      gainPar._gainFile = _initialGainFile;
      Command *gainCmd = new AdSec_Arbitrator::SetGain( 1000, gainPar);
      if ((ret = sendCommandWithReply( _adsecIntf, gainCmd)) != NO_ERROR) {
         _logger->log(Logger::LOG_LEV_ERROR, "Error setting initial gain: %s", lao_strerror(ret));
      }
   }
   catch (AOException *e) {
      _logger->log(Logger::LOG_LEV_ERROR, "Error setting initial gain: %s", e->what().c_str());
   }
}






void OptLoopDiagnostic::Run()
{
   time_t start = time(NULL);
   time_t now;

   // Get initial count

   float initial_counts;
   for (int i=0; i< COUNTS_BUFSIZE; i++) {
      GetRawData();
      _diagf->setData( (unsigned char *)&_raw);
      initial_counts = computeCounts();
   }
   _logger->log(Logger::LOG_LEV_INFO, "Initial counts: %5.1f", initial_counts);

   // Loop surveillance

   bool loopSuspended=false;
   while (!TimeToDie() && (_diagf)) {

      GetRawData();
      _diagf->setData( (unsigned char *)&_raw);

      float counts = computeCounts();

      now = time(NULL);
      if (now-start >0) {
         _logger->log(Logger::LOG_LEV_INFO, "Current counts: %5.1f", counts);
         start = now;
      }

      if ((counts < initial_counts/2.0) && (!loopSuspended)) {
         // Counts dropped! Suspend loop
         _logger->log(Logger::LOG_LEV_WARNING, "DANGER: %5.1f counts instead of %5.1f - Setting gain zero!", counts, initial_counts);
         setGainZero();
         loopSuspended=true;
      }
      else if ((counts >= initial_counts/2.0) && (loopSuspended)) {
         // Light is coming back. Resume loop
         _logger->log(Logger::LOG_LEV_INFO, "Current counts: %5.1f - Loop restarted", counts);
         setInitialGain();
         loopSuspended=false;
      }


   }


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

         stat = getOptLoopRawData( MyName(), _bufname, &_raw, timeout, false, &_info);

			if (IS_ERROR(stat)) 
            msleep(1000);
         else
            loop_again=false;

        }

}




//@Member: Periodic
//
//@
void OptLoopDiagnostic::Periodic()
{
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
    const char  *configfile = "conf/left/optloopdiagn.conf";
    OptLoopDiagnostic *app=NULL;

    SetVersion(VersMajor,VersMinor);

    try{
        if (argc > 1) 
            app = new OptLoopDiagnostic(argc, argv);        
        else
            app = new OptLoopDiagnostic(configfile);        
        // go
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
