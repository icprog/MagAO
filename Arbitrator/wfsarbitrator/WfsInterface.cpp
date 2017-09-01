
#include <string>
#include <iostream>
#include <sstream>
using namespace std;

#include <assert.h>

extern "C" {
#include "base/thrdlib.h"
}

#include "AOGlobals.h"
#include "WfsInterface.h"
#include "WfsArbitrator.h"


using namespace Arcetri::Wfs_Arbitrator;

//#define TRACE_FUNC()    printf("TRACE %s\n", __func__);
#define TRACE_FUNC()


WfsInterface::WfsInterface()
{
   TRACE_FUNC()
   _decimation=0;
   _sourceName = "UNDEFINED";
   _sourceMag = 0;

   // Default settings for sensor and instrument
   _sensor = AO_WFS_FLAO;
   _instrument = "IRTC";

   _loopFreq =0;
   _loopBinning=1;
   _loopModulation=0;
}

// ************************************+
// init() - choose between an embedded Python interpreter or an external one.
//
// Setting <useEmbeddedPython> to 1, an embedded Python interpreter will be used and
// the externalPython value is ignored.
// Otherwise, an external one is assumed and will be commanded using
// MsgD messages. The client name must be specified in the externalPython variable.



void WfsInterface::init( Logger *logger, int useEmbeddedPython, string externalPython)
{
   TRACE_FUNC()

   this->_useEmbeddedPython = useEmbeddedPython;
   this->_externalPython = externalPython;
   this->_externalPythonTimeout = 300*1000;

   this->_logger = logger;

   // Initialize embedded interpreter

   if (_useEmbeddedPython)
      {
      printf("Using Embedded Python interpreter\n");

      Py_Initialize();
      PyRun_SimpleString("import sys\n");
      PyRun_SimpleString("sys.argv=[]\n");
      PyRun_SimpleString("from AdOpt import *\n");
      PyRun_SimpleString("from AdOpt import WfsArbScripts\n");
      PyRun_SimpleString("self = thAOApp(myname='pyarg')\n");
      PyRun_SimpleString("WfsArbScripts.initArb(self)\n");

      // Get a reference to the globals() dictionary
      _mainModule = PyImport_AddModule("__main__");
      _mainDict   = PyModule_GetDict(_mainModule);

      // Release the GIL - it was locked by the thread module while creating the thAOApp object
      PyEval_ReleaseLock();

      }

   // - OR -

   // Check communication with the external interpreter
   // (will throw exception in case of error)

   else
      {
      printf("Using external Python interpreter %s\n", externalPython.c_str());
      commTest();
      }


   _lastStr = "";
   _lastCode = SysCmdSuccess;

   _returnCodes["OK"] = SysCmdSuccess;
   _returnCodes["ERROR"] = SysCmdError;
   _returnCodes["RETRY"] = SysCmdRetry;

}

WfsInterface::~WfsInterface()
{
   TRACE_FUNC()

// This often segfaults and is probably not needed..
//   if (_useEmbeddedPython)
//      Py_Finalize();
}

void WfsInterface::setSensorInstr( string sensor, string instrument) {

    _sensor = sensor;
    _instrument = instrument;
}

string WfsInterface::canbeNaN(float value)
{
if isnan(value) return "None";
std::ostringstream o;
o<<value;
return o.str();
}

string WfsInterface::canbeNaN(double value)
{
if isnan(value) return "None";
std::ostringstream o;
o<<value;
return o.str();
}


string WfsInterface::getErrorString()
{
   TRACE_FUNC()
   return _lastStr;
}

string WfsInterface::param( string name, string value) {

   ostringstream cmd;
   cmd << ", " << name << " = " << value;
   return cmd.str();
}

string WfsInterface::stringParam( string name, string value) {

   ostringstream cmd;
   cmd << ", " << name << " = '" << value << "'";
   return cmd.str();
}

string WfsInterface::intParam( string name, int value) {

   ostringstream cmd;
   cmd << ", " << name << " = " << value;
   return cmd.str();
}

string WfsInterface::commandStart( string command) {

   ostringstream cmd;
   cmd << "WfsArbScripts.";
   cmd << command << "(self";
   cmd << stringParam("sensor", _sensor);
   cmd << stringParam("instr", _instrument);
   return cmd.str();
}

string WfsInterface::commandEnd() {

   return ")";
}

AbstractSystemCommandResult WfsInterface::setOperating( powerOnParams params)
{
   TRACE_FUNC()
   ostringstream cmd;
   cmd << commandStart("setOperating");
   cmd << stringParam("config", params.config);
   cmd << stringParam("boardSetup", params.boardSetup);
   if (params.opticalSetup)
      cmd << param("opticalSetup", "True");
   else
      cmd << param("opticalSetup", "False");

   cmd << commandEnd();

   AbstractSystemCommandResult ret = sendCommand(cmd.str());
   if (ret != SysCmdSuccess)
      return ret;

//   if (params.sourceSetup == "")
//      return ret;
//   else
//      return setSource( params.sourceSetup, 0);

   return ret;

}

AbstractSystemCommandResult WfsInterface::setSource( string config, float magnitude)
{
   TRACE_FUNC()
   ostringstream cmd;

   cmd << commandStart("setSource");
   cmd << stringParam("config", config);
   cmd << param("magnitude", canbeNaN(magnitude)); 
   cmd << commandEnd();

   AbstractSystemCommandResult ret = sendCommand(cmd.str());

   if (ret == SysCmdSuccess) {
      _sourceName = config;
      _sourceMag  = magnitude;
   }
   return ret;
}


AbstractSystemCommandResult WfsInterface::prepareAcquireRef( prepareAcquireRefParams *params, prepareAcquireRefResult *outputParams)
{
    TRACE_FUNC()
    ostringstream cmd;

    assert(params);
    assert(outputParams);

    cmd << commandStart("prepareAcquireRef");
    cmd << stringParam("mode", params->AOMode);
    cmd << param("starxpos", canbeNaN(params->ROCoords[0]));
    cmd << param("starypos", canbeNaN(params->ROCoords[1]));
    cmd << param("telElevation", canbeNaN(params->Elevation));
    cmd << param("derotAngle", canbeNaN(params->RotAngle));
    cmd << param("starMag", canbeNaN(params->Mag));
    cmd << param("starColor", canbeNaN(params->Color));
    cmd << param("r0", canbeNaN(params->r0));
    cmd << param("v0", "0");
    cmd << commandEnd();

    AbstractSystemCommandResult ret = sendCommand(cmd.str());

    if (ret == SysCmdSuccess) {
      char rec[256], pupils[256], base[256], filtering[256];
      float freq;
      float rangemin, rangemax;
      sscanf( _lastStr.c_str(), "%f %d %f %d %d %f %s %s %s %s %d %f %f", &freq, &(outputParams->binning), &(outputParams->TTmod),
                                                        &(outputParams->Fw1Pos), &(outputParams->Fw2Pos), &(outputParams->gain),
                                                         pupils, base, rec, filtering, &(outputParams->nModes), &rangemin, &rangemax);
      outputParams->rec = rec;
      outputParams->base = base;
      outputParams->pupils = pupils;
      outputParams->freq = freq;
      outputParams->filtering = filtering;

      _loopFreq = freq;
      _loopBinning = outputParams->binning;
      _loopModulation = outputParams->TTmod;
      _rangemin = rangemin;
      _rangemax = rangemax;

      computeDecimation(freq);
      outputParams->decimation = _decimation;

      outputParams->Dump();
      _lastStr = "";	// Clear error string
    }

    return ret;
}

AbstractSystemCommandResult WfsInterface::acquireRef(  acquireRefResult *outputParams )
{
    TRACE_FUNC()
    ostringstream cmd;

    cmd << commandStart("acquireRef");
    cmd << commandEnd();

    AbstractSystemCommandResult ret = sendCommand(cmd.str());

    if (ret == SysCmdSuccess)
       {
       int framelen;
       float freq;
       sscanf( _lastStr.c_str(), "%f %f %f %d %d %f %d %d %f %s %s %s %s %f %s %s %f %d",  &(outputParams->deltaXY[0]), &outputParams->deltaXY[1],
                                                              &freq, &(outputParams->nModes), &(outputParams->bin),
                                                              &(outputParams->TTmod), &(outputParams->Fw1Pos), &(outputParams->Fw2Pos),
                                                              &(outputParams->starMag), outputParams->pupils, outputParams->base, 
                                                              outputParams->rec, outputParams->filtering, &(outputParams->gain),
                                                              outputParams->CLbase, outputParams->CLrec, &(outputParams->CLfreq),
                                                              &framelen);

       outputParams->freq = freq;

      _loopFreq = freq;
      _loopBinning = outputParams->bin;
      _loopModulation = outputParams->TTmod;

       computeDecimation(freq);
       outputParams->decimation = _decimation;

       memcpy( outputParams->TVframe, _lastStr.substr( _lastStr.find("IMAGE")+5).data(), framelen);
       _lastStr = "";	// Clear error string
       }

   return ret;
}

AbstractSystemCommandResult WfsInterface::getTVSnap( snapParams *outputParams)
{
   TRACE_FUNC();
   ostringstream cmd;

   cmd << commandStart("getTVSnap");
   cmd << commandEnd();

   AbstractSystemCommandResult ret = sendCommand(cmd.str());

   if (ret == SysCmdSuccess) {
      int framelen;
      sscanf( _lastStr.c_str(), "%d", &framelen);

      outputParams->frameValid = true;
      memcpy( outputParams->TVframe, _lastStr.substr( _lastStr.find("IMAGE")+5).data(), framelen);
      _lastStr = "";	// Clear error string
   }
   else
      outputParams->frameValid = false;

   return ret;
}




AbstractSystemCommandResult WfsInterface::checkRef( checkRefResult *outputParams)
{
   TRACE_FUNC();
   ostringstream cmd;

   cmd << commandStart("checkRef");
   cmd << commandEnd();

   AbstractSystemCommandResult ret = sendCommand(cmd.str());

   if (ret == SysCmdSuccess) {
      float deltaX, deltaY, mag;
      sscanf( _lastStr.c_str(), "%f %f %f", &deltaX, &deltaY, &mag);

      outputParams->deltaXY[0] = deltaX;
      outputParams->deltaXY[1] = deltaY;
      outputParams->starMag = mag;
      _lastStr = "";	// Clear error string
   }

   return ret;
}


AbstractSystemCommandResult WfsInterface::setPowerOff()
{
   TRACE_FUNC()

   ostringstream cmd;
   cmd << commandStart("setPowerOff");
   cmd << commandEnd();
   return sendCommand(cmd.str());
}

AbstractSystemCommandResult WfsInterface::closeLoop()
{
   TRACE_FUNC()

   ostringstream cmd;
   cmd << commandStart("closeLoop");
   cmd << commandEnd();
   return sendCommand(cmd.str());
}

AbstractSystemCommandResult WfsInterface::stopLoop()
{
   TRACE_FUNC()
   ostringstream cmd;
   AbstractSystemCommandResult res = SysCmdSuccess;

   /* TODO check for disturb and disable it

   cmd.str("");
   cmd << commandStart("enableDisturb");
   cmd << param("wfs", "False");
   cmd << param("ovs", "False");
   cmd << commandEnd();
   res = sendCommand(cmd.str());

   // [TODO] This wait should be enough to get a few slopes to the adsec in all situations.
   // A proper procedure should be to check with the adsec that the disturbance flat has been disabled.
   msleep(50);
   */

 
   if (res == SysCmdSuccess) {
       cmd.str("");
       cmd << commandStart("stopLoop");
       cmd << commandEnd();
       res = sendCommand(cmd.str());
   }
   return res;
}

AbstractSystemCommandResult WfsInterface::pauseLoop()
{
   TRACE_FUNC()
   ostringstream cmd;
   cmd << commandStart("pauseLoop");
   cmd << commandEnd();
   return sendCommand(cmd.str());
}

AbstractSystemCommandResult WfsInterface::resumeLoop()
{
   TRACE_FUNC()
   ostringstream cmd;
   cmd << commandStart("resumeLoop");
   cmd << commandEnd();
   return sendCommand(cmd.str());
}


AbstractSystemCommandResult WfsInterface::saveStatus( saveStatusParams params)
{
   TRACE_FUNC()
   ostringstream cmd;
   cmd << commandStart("saveStatus");
   cmd << stringParam("outputFile", params._outputFile);
   cmd << commandEnd();
   return sendCommand(cmd.str());
}

void WfsInterface::computeDecimation( float freq) {

   WfsArbitrator *arb =  (WfsArbitrator*)AbstractArbitrator::getInstance();
   float master_speed = arb->_maxMasterSpeed;
   if (master_speed == 0)
      master_speed = 100;   // Use a low value if something goes wrong
   _decimation = int(freq/master_speed);          // This computation is repeated into WfsArbScripts.py !!
}

float WfsInterface::getRangeMin() {
   return _rangemin;
}

float WfsInterface::getRangeMax() {
   return _rangemax;
}

int WfsInterface::getDecimation() {
   return _decimation;
}

float WfsInterface::getLoopFreq() {
   return _loopFreq;
}

int WfsInterface::getBinning() {
   return _loopBinning;
}

float WfsInterface::getModulation() {
   return _loopModulation;
}

string WfsInterface::getSourceName() {
   return _sourceName;
}

float WfsInterface::getSourceMag() {
   return _sourceMag;
}


AbstractSystemCommandResult WfsInterface::test()
{
   TRACE_FUNC()
   ostringstream cmd;
   cmd << commandStart("test");
   cmd << commandEnd();
   return sendCommand(cmd.str());
}

AbstractSystemCommandResult WfsInterface::calibrateHODark( int nframes)
{
   TRACE_FUNC()
   ostringstream cmd;
   cmd << commandStart("calibHODark");
   cmd << intParam("num", nframes);
   cmd << commandEnd();
   return sendCommand(cmd.str());
}

AbstractSystemCommandResult WfsInterface::enableDisturb( enableDisturbParams params)
{
   TRACE_FUNC()
   ostringstream cmd;
   cmd << commandStart("enableDisturb");
   cmd << param("wfs", (params._enableWFS) ? "True" : "False");
   cmd << param("ovs", (params._enableOVS) ? "True" : "False");
   cmd << commandEnd();
   return sendCommand(cmd.str());
}

AbstractSystemCommandResult WfsInterface::savePsf( string filename, int nframes)
{
   TRACE_FUNC()
   ostringstream cmd;
   cmd << commandStart("savePsf");
   cmd << stringParam("filename", filename);
   cmd << intParam("num", nframes);
   cmd << commandEnd();
   return sendCommand(cmd.str());
}

AbstractSystemCommandResult WfsInterface::saveIrtc( string filename, int nframes)
{
   TRACE_FUNC()
   ostringstream cmd;
   cmd << commandStart("saveIrtc");
   cmd << stringParam("filename", filename);
   cmd << intParam("num", nframes);
   cmd << commandEnd();
   return sendCommand(cmd.str());
}

AbstractSystemCommandResult WfsInterface::savePisces( string filename, int nframes)
{
   TRACE_FUNC()
   ostringstream cmd;
   cmd << commandStart("savePisces");
   cmd << stringParam("filename", filename);
   cmd << intParam("num", nframes);
   cmd << commandEnd();
   return sendCommand(cmd.str());
}

AbstractSystemCommandResult WfsInterface::offsetXY( offsetXYparams params)
{
   TRACE_FUNC()
   ostringstream cmd;
   cmd << commandStart("offsetXY");
   cmd << param("offsetx", canbeNaN( params.offsetX));
   cmd << param("offsety", canbeNaN( params.offsetY));
   cmd << param("brake", (params.brake) ? "True" : "False");
   cmd << commandEnd();
   return sendCommand(cmd.str());
}

AbstractSystemCommandResult WfsInterface::offsetZ( offsetZparams params)
{
   TRACE_FUNC()
   ostringstream cmd;
   cmd << commandStart("offsetZ");
   cmd << param("offsetz", canbeNaN( params.offsetZ));
   cmd << commandEnd();
   return sendCommand(cmd.str());
}

AbstractSystemCommandResult WfsInterface::correctModes( correctModesParams /* params */)
{
   TRACE_FUNC()
   ostringstream cmd;
   cmd << commandStart("correctModes");
   cmd << commandEnd();
   return sendCommand(cmd.str());
}

AbstractSystemCommandResult WfsInterface::emergencyOff()
{
   TRACE_FUNC()
   ostringstream cmd;
   cmd << commandStart("emergencyOff");
   cmd << commandEnd();
   return sendCommand(cmd.str());
}

AbstractSystemCommandResult WfsInterface::modifyAO( modifyAOparams params, modifyAOparams *result)
{
   TRACE_FUNC()
   ostringstream cmd;

   if (params.freq>0.0) {
      _loopFreq = params.freq;
      computeDecimation( _loopFreq);
   }
   if (params.Binning > 0)
      _loopBinning = params.Binning;

   if (params.TTmod >=0)
      _loopModulation = params.TTmod;

   cmd << commandStart("modifyAO");
   cmd << param("freq", canbeNaN(_loopFreq));
   cmd << intParam("binning", _loopBinning);
   cmd << param("TTmod", canbeNaN(_loopModulation));
   cmd << intParam("Fw1Pos", params.Fw1Pos);
   cmd << intParam("Fw2Pos", params.Fw2Pos);
   cmd << intParam("decimation", _decimation);
   cmd << stringParam("checkCameralens", (params.checkCameralens) ? "True" : "False");
   cmd << commandEnd();

   AbstractSystemCommandResult ret = sendCommand(cmd.str());

   if (ret == SysCmdSuccess)
      {
      // Just transfer the input params!
      memcpy( result, &params, sizeof(modifyAOparams));
      }
   return ret;
       
}

AbstractSystemCommandResult WfsInterface::optimizeGain()
{
   TRACE_FUNC()
   ostringstream cmd;
   cmd << commandStart("optimizeGain");
   cmd << commandEnd();
   return sendCommand(cmd.str());
}

AbstractSystemCommandResult WfsInterface::calibrateTVDark( int nframes)
{
   TRACE_FUNC()
   ostringstream cmd;
   cmd << commandStart("calibTVDark");
   cmd << intParam("num", nframes);
   cmd << commandEnd();
   return sendCommand(cmd.str());
}

AbstractSystemCommandResult WfsInterface::calibrateIRTCDark( int nframes)
{
   TRACE_FUNC()
   ostringstream cmd;
   cmd << commandStart("calibIRTCDark");
   cmd << intParam("num", nframes);
   cmd << commandEnd();
   return sendCommand(cmd.str());
}

AbstractSystemCommandResult WfsInterface::calibratePISCESDark( int nframes)
{
   TRACE_FUNC()
   ostringstream cmd;
   cmd << commandStart("calibIRTCDark");
   cmd << intParam("num", nframes);
   cmd << param("apply", "False");
   cmd << stringParam("ccd", "pisces");
   cmd << commandEnd();
   return sendCommand(cmd.str());
}

AbstractSystemCommandResult WfsInterface::calibrateMovements()
{
   TRACE_FUNC()
   ostringstream cmd;
   cmd << commandStart("calibMovements");
   cmd << commandEnd();
   return sendCommand(cmd.str());
}

AbstractSystemCommandResult WfsInterface::calibrateSlopenull( int nframes)
{
   TRACE_FUNC()
   ostringstream cmd;
   cmd << commandStart("calibSlopenull");
   cmd << intParam("num", nframes);
   cmd << commandEnd();
   return sendCommand(cmd.str());
}

AbstractSystemCommandResult WfsInterface::setHOarmPolicy( SetHoArmPolicy hoArmPolicy)
{
   TRACE_FUNC()

   ostringstream cmd;
   cmd << "WfsArbScripts.setHOarmPolicy(self, ";

   cmd << addPolicy( "ccd39", hoArmPolicy.ccd39) << ",";
   cmd << addPolicy( "adc",   hoArmPolicy.adc)   << ",";
   cmd << addPolicy( "tt",    hoArmPolicy.tt)    << ",";
   cmd << addPolicy( "fw1",   hoArmPolicy.fw1)   << ",";
   cmd << addPolicy( "rerot", hoArmPolicy.rerot) << ")";

   return sendCommand(cmd.str(), 1);
}

AbstractSystemCommandResult WfsInterface::setTVarmPolicy( SetTvArmPolicy tvArmPolicy)
{
   TRACE_FUNC()

   ostringstream cmd;
   cmd << "WfsArbScripts.setTVarmPolicy(self, ";

   cmd << addPolicy( "ccd47", tvArmPolicy.ccd47) << ",";
   cmd << addPolicy( "fw2",   tvArmPolicy.fw2)   << ")";

   return sendCommand(cmd.str(), 1);

}

AbstractSystemCommandResult WfsInterface::setPointAndSourcePolicy( PointAndSourcePolicy pointAndSourcePolicy)
{
   TRACE_FUNC()

   ostringstream cmd;
   cmd << "WfsArbScripts.setPointingPolicy(self, ";

   cmd << addPolicy( "stages", pointAndSourcePolicy.stages) << ",";
   cmd << addPolicy( "cube",   pointAndSourcePolicy.cube)   << ",";
   cmd << addPolicy( "lamp",   pointAndSourcePolicy.lamp)   << ")";

   return sendCommand(cmd.str(), 1);

}

string WfsInterface::addPolicy( string device, wfsPolicy policy)
{
   TRACE_FUNC()
   ostringstream cmd;

   if (policy == wfsPolicy_AUTO)
      cmd << device << "='auto'";
   else if (policy == wfsPolicy_MANUAL)
      cmd << device << "='manual'";
   else
      cmd << device << "=None";

   return cmd.str();
}

AbstractSystemCommandResult WfsInterface::sendCommand( string command, int async)
{
   TRACE_FUNC()

   printf("Command:\n %s\n", command.c_str());
   _logger->log(Logger::LOG_LEV_INFO, "Python command: %s", command.c_str());

   // Use embedded interpreter

   if (_useEmbeddedPython)
      {
      PyObject *res0, *res1;

      PyGILState_STATE gstate = PyGILState_Ensure();
      PyObject *res = PyRun_String( command.c_str(), Py_eval_input, _mainDict, _mainDict);

      // Non-trapped exception? e.g. a syntax error before arriving in our exception wrapper
         if (PyErr_Occurred()) {
               printf("error occurred\n");
               PyErr_Print();
               PyObject *type, *value, *traceback;
               PyErr_Fetch( &type, &value, &traceback);

               _lastStr = "Unknown exception";
               if (value) {
                  PyObject *str = PyObject_Str(value);
                  if (str) {
                     char *cstr = PyString_AsString(str);
                     printf("Exception: %s\n", cstr);
                     _logger->log(Logger::LOG_LEV_ERROR, "Python exception: %s", cstr);
                     _lastStr = cstr;
                     Py_XDECREF(str);
                  }
                  Py_XDECREF(value);
               }
               if (traceback) {
                  PyObject *str = PyObject_Str(traceback);
                  if (str) {
                     char *cstr = PyString_AsString(str);
                     printf("Traceback: %s\n", cstr);
                     _logger->log(Logger::LOG_LEV_ERROR, "Python traceback: %s", cstr);
                     Py_XDECREF(str);
                  }
                  Py_XDECREF(traceback);
               }

               PyErr_Clear();
               _lastCode = SysCmdError;
               goto done;
               }

         if (res == Py_None) {
               printf("Code returned None\n");
               _lastCode = SysCmdError;
               _lastStr = "Code returned None";
               goto done;
               }


         if (!PySequence_Check(res)) {
               printf("Returned value is not a sequence\n");
               PyObject_Print( res, stdout, 0);
               printf("\n");
               _lastCode = SysCmdError;
               goto done;
               }

         res0 =  PySequence_GetItem( res, 0);
         res1 =  PySequence_GetItem( res, 1);
         if (PyString_Check(res0))
            {
            string resCode = PyString_AsString( res0);
            _lastCode = _returnCodes[resCode];
            }
         else
            printf("Result 0 is not a string!\n");

         if (PyString_Check(res1))
            {
            // Code that allows zero bytes in the string
            char *buffer;
            int len;
            PyString_AsStringAndSize( res1, &buffer, &len);
            _lastStr = string( buffer, len);
            }
         else
            printf("Result 1 is not a string!\n");

done:
     PyGILState_Release(gstate);
     return _lastCode;
     }

   // - OR -
   //
   // Send message to external interpreter and wait for result


   else
   {
      int errcode;
      int code = WFSARB_CMD;
      if (async)
        code = WFSARB_ASYNC_CMD;

      int seqid = thSendMsg( command.size(), (char *)_externalPython.c_str(), code, 0, (void *) command.c_str());
      MsgBuf *msgb = thWaitMsg( WFSARB_CMD, (char *)_externalPython.c_str(), seqid, _externalPythonTimeout, &errcode);

      if (msgb == NULL)
         {
         _lastCode = SysCmdError;
         _lastStr = "Timeout";      // Here we should get some error description, if possible
          return SysCmdError;
          }

      externalPythonAnswer *answ = (externalPythonAnswer *)msgb->Msg->Body;
      _lastCode = _returnCodes[answ->code];
      _lastStr = answ->str;

      return _lastCode;
   }

}

void WfsInterface::setTimeout( int ms)
{
   TRACE_FUNC()
   _externalPythonTimeout = ms;
}
