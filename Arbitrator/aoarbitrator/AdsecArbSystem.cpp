#include "aoarbitrator/AOArbitrator.h"
#include "aoarbitrator/AdsecArbSystem.h"
#include "arblib/adSecArb/AdSecCommands.h"
#include "arblib/base/Commands.h"

#include <math.h>
#include "AdsecDiagnLib.h"

#include "Paths.h"
#include "Utils.h"

using namespace Arcetri::AdSec_Arbitrator;

string AdsecArbSystem::_adsecLastGainFilename = "";

// Define this to avoid sending any commands to AdSec and always reply OK
#undef TEST


AdsecArbSystem::AdsecArbSystem(bool simulation): AbstractSystem(AO_ADSEC_FLAO) {
   _simulation = simulation;
   _arbitrator = AbstractArbitrator::getInstance();
   _adSecArbIntf = NULL;
   _msgdName = Utils::getClientname("adsecarb");
   _powerOn = false;
   _mirrorSet = false;
   _clStatReady = false;
   _tipOffload = 0;
   _tiltOffload = 0;
   _focusOffload = 0;
   _safeSkipPercent = 0;

   // Set some sensible initial values (will be used by the seeing limited simulation)
   _decimation = 2;
   _freq = 400;
   _ovsFreq = 800;
   _adSecStatus = "Unknown";
   _adsecCurrGain = RTDBvar(Utils::getClientname("ADSEC")+".G_GAIN_A", CHAR_VARIABLE, 50, false);
}

AdsecArbSystem::~AdsecArbSystem() {

}

void AdsecArbSystem::setArbInterface(ArbitratorInterface * intf)
{
   _adSecArbIntf = intf;
}

bool AdsecArbSystem::sync() {

   RequestAdSecStatus request(1000);
   RequestAdSecStatus *reply = NULL;

   static int skipAlert=0;

   if(_simulation) {
      _powerOn = true;
      _clStatReady = true;
      _mirrorSet = true;
      return true;
   }

   try {
      reply = (RequestAdSecStatus *)_adSecArbIntf->requestCommand(&request);
      Logger::get()->log( Logger::LOG_LEV_DEBUG, "thGetStartT(%s)", _adSecArbIntf->getTargetArbitrator().c_str()); 
      if (thGetStartT(_adSecArbIntf->getTargetArbitrator().c_str(), &_arbStartTime) != NO_ERROR)
          Logger::get()->log( Logger::LOG_LEV_WARNING, "Cannot get arbitrator start time for %s",
                              _adSecArbIntf->getTargetArbitrator().c_str());
   } catch (AOException &e) {
      Logger::get()->log( Logger::LOG_LEV_DEBUG, "Error requesting ADSEC status");
      _clStatReady = false;
      return false;
   }

   if (!reply->isSuccess()) {
       _clStatReady = false;
       return false;
   }

   // Get on/off status
   _adSecStatus = reply->getArbitratorStatus();
   if ((_adSecStatus == "PowerOff") || (_adSecStatus == "PowerOn") )
      _powerOn = false;
   else
      _powerOn = true;

   if ((_adSecStatus == "Failure") || (_adSecStatus == "Panic") || (_adSecStatus == "PowerOn") || (_adSecStatus == "PowerOff") || (_adSecStatus == "Ready")  )
      _mirrorSet = false;
   else
      _mirrorSet = true;

   _clStatReady = reply->getClStatReady();
   _safeSkipPercent = reply->safeSkipPercent();
   _tipOffload = reply->tipOffload();
   _tiltOffload = reply->tiltOffload();
   _focusOffload = reply->focusOffload();

   return true;
}

bool AdsecArbSystem::simpleCommand( Command *cmd) {
   _logger->log(Logger::LOG_LEV_INFO, "Requesting %s to AdSecArbitrator", cmd->getDescription().c_str());

   bool isSuccess = false;

   Command *reply = _adSecArbIntf->requestCommand(cmd);
   setCommandResultFromReply(reply);

   isSuccess = reply->isSuccess();
   delete reply;
       
   return isSuccess;
}

void AdsecArbSystem::powerOn() {

#ifdef TEST
      setCommandResult(SysCmdSuccess);
      return;
#endif

   _logger->log(Logger::LOG_LEV_INFO, "Executing PowerOn...");


   try {
      sync();     // Get AdSecArb status

      On powerOn(180*1000); 
      LoadProgram loadProgram(60*1000);  

      if (_adSecStatus == "PowerOff") 
         simpleCommand(&powerOn) && simpleCommand(&loadProgram);

      else if (_adSecStatus == "PowerOn") 
         simpleCommand(&loadProgram);

   }
   catch(ArbitratorInterfaceException& e) {
      _logger->log(Logger::LOG_LEV_ERROR, "PowerOn failed: %s", e.what().c_str());
   }

}

void AdsecArbSystem::mirrorSet() {

#ifdef TEST
      setCommandResult(SysCmdSuccess);
      return;
#endif

   _logger->log(Logger::LOG_LEV_INFO, "Executing mirrorSet...");


   try {
      sync();     // Get AdSecArb status

      SetFlatAo setFlatAO(180*1000);

      if (_adSecStatus == "Ready") 
         simpleCommand(&setFlatAO);

   }
   catch(ArbitratorInterfaceException& e) {
      _logger->log(Logger::LOG_LEV_ERROR, "mirrorSet failed: %s", e.what().c_str());
   }

}

void AdsecArbSystem::powerOff() {

#ifdef TEST
    setCommandResult(SysCmdSuccess);
    return;
#endif

   _logger->log(Logger::LOG_LEV_INFO, "Executing PowerOff...");

   try {
      Off powerOff(20*1000); 

      sync();     // Get AdSecArb status

      if (_adSecStatus == "Ready") 
         simpleCommand(&powerOff);
   }
   catch(ArbitratorInterfaceException& e) {
      _logger->log(Logger::LOG_LEV_ERROR, "PowerOff failed: %s", e.what().c_str());
   }

}

void AdsecArbSystem::mirrorRest() {

#ifdef TEST
    setCommandResult(SysCmdSuccess);
    return;
#endif

   _logger->log(Logger::LOG_LEV_INFO, "Executing mirrorRest...");

   try {
      Rest rest(20*1000);  

      sync();     // Get AdSecArb status

      if (_adSecStatus == "AOSet" || _adSecStatus == "ChopSet" || _adSecStatus == "SLSet") 
         simpleCommand(&rest); 

   }
   catch(ArbitratorInterfaceException& e) {
      _logger->log(Logger::LOG_LEV_ERROR, "Mirror Rest failed: %s", e.what().c_str());
   }

}



void AdsecArbSystem::presetFlat(string flatSpec) {

#ifdef TEST
    setCommandResult(SysCmdSuccess);
    return;
#endif

   _logger->log(Logger::LOG_LEV_INFO, "Executing PresetFlat...");

   Command* loadShapeReply = NULL;

   shapeParams params;
   params._shapeFile = flatSpec;

   try {
      LoadShape loadShape(20000, params); // TODO obtain from somewhere!!!
      _logger->log(Logger::LOG_LEV_INFO, "Requesting LoadShape to AdSecArbitrator...");
      loadShapeReply = _adSecArbIntf->requestCommand(&loadShape);
      setCommandResultFromReply(loadShapeReply);
   }
   catch(ArbitratorInterfaceException& e) {
      _logger->log(Logger::LOG_LEV_ERROR, "PresetFlat failed: %s", e.what().c_str());
   }
}

void AdsecArbSystem::presetAO( float gain, string m2c, string rec, string filtering, float freq, int decimation) {

   // Get gain filename
   ostringstream oss;
   oss << "gain";
   oss.width(3);
   oss.precision(1);
   oss << gain << ".fits";

   presetAO( oss.str(), m2c, rec, filtering, freq, decimation);
}

void AdsecArbSystem::presetAO( string gain, string m2c, string rec, string filtering, float freq, int decimation) {

    _logger->log(Logger::LOG_LEV_INFO, "Executing presetAO...");

   AdSec_Arbitrator::gainParams gParams;
   gParams._gainFile = Paths::GainDir(  m2c, true) + gain;

   // Compute oversampling frequency
   float maxOvsFreq;
   try {
      maxOvsFreq = _arbitrator->ConfigDictionary()["MaxOvsFreq"];
   } catch(Config_File_Exception &e) {
   _logger->log(Logger::LOG_LEV_INFO, "Max OVS frequency not specified in config file. Using default value");
      maxOvsFreq = 820.0;
   }

   if ((freq <1) || (freq>1000)) {
      _logger->log(Logger::LOG_LEV_ERROR, "AdsecArbSystem::runAO() rejected loop frequency %f", freq);
      setCommandResult( SysCmdError);
      return;
   }


   // JAR JUN2011: 
   // Different settings during commissioning run
   // NOTE: commented out and left original code after 
   //       the commissioning as the ovs is now disabled
   //       when the loop is closed.
   // XX: nuova formula
   //float ovsFreq = min(freq - (7*freq)/100, maxOvsFreq);
   //float ovsFreq = 93.0;

   float ovsFreq = freq;
   if (ovsFreq > maxOvsFreq)
      ovsFreq = maxOvsFreq;

   int maxiter=100;
   while((ovsFreq+ freq < maxOvsFreq) && (maxiter-- >=0))
      ovsFreq += freq;

      if (maxiter <=0) {
         _logger->log(Logger::LOG_LEV_ERROR, "AdsecArbSystem::runAO() Unexpected problem searching for the OVS frequency for loop frequency %f", freq);
         setCommandResult( SysCmdError);
         return;
      }

   ovsFreq += 40; // 

#ifdef TEST
   printf("PresetAO: gainfile %s, m2c %s, rec %s, aDelay %s, bDelay %s, freq %f, decimation %d, ovsFreq %f\n", gParams._gainFile.c_str(), m2c.c_str(), rec.c_str(), aDelay.c_str(), bDelay.c_str(), freq, decimation, ovsFreq);
    setCommandResult(SysCmdSuccess);
    return;
#endif

   _logger->log(Logger::LOG_LEV_INFO, "freq %f, decimation %d, ovsFreq %f", freq, decimation, ovsFreq);

   _logger->log(Logger::LOG_LEV_INFO, "Executing PresetAO...");

   Command* reply = NULL;

   try {

      // Set reconstructor
      AdSec_Arbitrator::recMatParams rParams;

      string path = Paths::M2CDir( m2c, true);
      string filterPath = path + "filtering/" + filtering + "/";

      rParams._m2cFile = path + "m2c.fits";
      rParams._recMatFile = path + "RECs/Rec_"+rec+".fits";
      rParams._aDelayFile = filterPath + "mode_delay.fits";
      rParams._bDelayFile = filterPath + "slope_delay.fits";

      SetRecMat setRecMat( 20000, rParams);
      reply = _adSecArbIntf->requestCommand(&setRecMat);
      if(!reply->isSuccess()) {
         setCommandResultFromReply(reply);
         return;
      }


      // Set gain
      SetGain setGain(2000, gParams);

      reply = _adSecArbIntf->requestCommand(&setGain);
      if(!reply->isSuccess()) {
         setCommandResultFromReply(reply);
         return;
      }


      // Remember params for RunAO
      _decimation = decimation;
      _freq = freq;
      _ovsFreq = ovsFreq;

   setCommandResult(SysCmdSuccess);

   }
   catch(ArbitratorInterfaceException& e) {
      _logger->log(Logger::LOG_LEV_ERROR, "PresetAO failed: %s", e.what().c_str());
   }
}

void AdsecArbSystem::pause() {
   
    _logger->log(Logger::LOG_LEV_INFO, "Executing pause...");
   try {
      PauseAo pause(5*1000);
      simpleCommand(&pause); 
   }
   catch(ArbitratorInterfaceException& e) {
       _logger->log(Logger::LOG_LEV_ERROR, "PauseAo failed: %s", e.what().c_str());
   }
}

void AdsecArbSystem::resume() {

   _logger->log(Logger::LOG_LEV_INFO, "Executing resume...");
   try {
      ResumeAo resume(5*1000);
      simpleCommand(&resume); 
   }
   catch(ArbitratorInterfaceException& e) {
       _logger->log(Logger::LOG_LEV_ERROR, "ResumeAo failed: %s", e.what().c_str());
   }
}

void AdsecArbSystem::enableTTOffload( bool enable) {

   try {
      ttOffloadParams params;
      params.enable = enable;
      TTOffload ttoffload(5*1000, params);
      simpleCommand(&ttoffload); 
   }
   catch(ArbitratorInterfaceException& e) {
       _logger->log(Logger::LOG_LEV_ERROR, "enableTTOffload failed: %s", e.what().c_str());
   }
}

void AdsecArbSystem::recoverFailure() {

#ifdef TEST
      setCommandResult(SysCmdSuccess);
      return;
#endif

   _logger->log(Logger::LOG_LEV_INFO, "Executing recoverFailure...");


   try {
      sync();     // Get AdSecArb status

      RecoverFailure recoverfailure(300*1000); 

      if (_adSecStatus == "Failure") 
         simpleCommand(&recoverfailure);

   }
   catch(ArbitratorInterfaceException& e) {
      _logger->log(Logger::LOG_LEV_ERROR, "recoverFailure failed: %s", e.what().c_str());
   }

}


void AdsecArbSystem::checkRefAO() {
   setCommandResult(SysCmdSuccess);
   // At the moment does nothing!
}


void AdsecArbSystem::setZernikes(setZernikesParams params) {

    _logger->log(Logger::LOG_LEV_INFO, "AdSec Executing SetZernikes");
    try {
        AdSec_Arbitrator::setZernikesParams adscParams;
        for(int i=0;i<N_ZERN;i++) adscParams.deltaM[i]=params.deltaM[i];
        AdSec_Arbitrator::SetZernikes setZern( 10000, adscParams);   // [TODO] obtain timeout from somewhere
        Command *reply = _adSecArbIntf->requestCommand(&setZern);
        setCommandResultFromReply(reply);
        delete reply;
    } catch (AOException &e) {
        _logger->log(Logger::LOG_LEV_ERROR, "setZernikes failed: %s", e.what().c_str());
    }
}

void AdsecArbSystem::modifyAO() {
   setCommandResult(SysCmdSuccess);
   // At the moment does nothing!
}
void AdsecArbSystem::offsetXY() {
   setCommandResult(SysCmdSuccess);
   // At the moment does nothing!
}
void AdsecArbSystem::offsetZ() {
   setCommandResult(SysCmdSuccess);
   // At the moment does nothing!
}
void AdsecArbSystem::refineAO() {
   setCommandResult(SysCmdSuccess);
   // At the moment does nothing!
}
void AdsecArbSystem::startAO() {

   // Set RunAO
   _logger->log(Logger::LOG_LEV_INFO, "Executing startAO...");
   try {
       AdSec_Arbitrator::runAoParams aoParams;

       aoParams._decimation = _decimation;
       aoParams._loopFreq = _freq;
       aoParams._ovsFreq = _ovsFreq;

       RunAo runAo(10000, aoParams);
       Command *reply = _adSecArbIntf->requestCommand(&runAo);

       setCommandResultFromReply(reply);
       delete reply;
    } catch (AOException &e) {
        _logger->log(Logger::LOG_LEV_ERROR, "startAO failed: %s", e.what().c_str());
    }
}

void AdsecArbSystem::stop( bool hold) {

   _logger->log(Logger::LOG_LEV_INFO, "Executing stop...");
   try {
       AdSec_Arbitrator::stopAoParams params;

       params._restoreShape = !hold;

       AdSec_Arbitrator::StopAo stopAO(20*1000, params);

       Command *reply = _adSecArbIntf->requestCommand(&stopAO);
       setCommandResultFromReply(reply);
       delete reply;
    } catch (AOException &e) {
        _logger->log(Logger::LOG_LEV_ERROR, "stopAO failed: %s", e.what().c_str());
    }
}

void AdsecArbSystem::userPanic() {
   setCommandResult(SysCmdSuccess);
   // At the moment does nothing!
}

bool AdsecArbSystem::setDisturbance( string filename)  {

   _logger->log(Logger::LOG_LEV_INFO, "Executing setDisturbance...");
   try {
       disturbParams dist;
       dist._disturbFile = Paths::DisturbDir( "", true) + filename;
       dist._on = true;

       AdSec_Arbitrator::SetDisturb setDisturb( 20000, dist);

       Command *reply = _adSecArbIntf->requestCommand(&setDisturb);
       bool ret = reply->isSuccess();
       delete reply;
       return ret;
   } catch (AOException &e) {
       _logger->log(Logger::LOG_LEV_ERROR, "setDisturbance failed: %s", e.what().c_str());
       return false;
   }
}

bool AdsecArbSystem::set10modesGain() {

   _logger->log(Logger::LOG_LEV_DEBUG, "Executing setGain...");

   string filename;
   filename = Paths::GainDir( "KL", true);
 
   filename = "gain0.1_10modi.fits";

   return setGain(filename);
}

bool AdsecArbSystem::setGain( float gain) {

   _logger->log(Logger::LOG_LEV_DEBUG, "Executing setGain...");

   string filename;
   filename = Paths::GainDir( "KL", true);

   if (gain == 0) 
       filename += "gain0.fits";
   else {
       char str[32];
       snprintf( str, 32, "gain%3.1f.fits", gain);
       filename += str;
   }

   return setGain(filename);
}

bool AdsecArbSystem::setGain(string filename) {

    _logger->log(Logger::LOG_LEV_INFO, "Executing setGain(%s)...", filename.c_str());
    
    try {
        gainParams g;
        
        g._gainFile = filename;
        AdSec_Arbitrator::SetGain setGain( 20000, g);
        Command *reply = _adSecArbIntf->requestCommand(&setGain);
        bool ret = reply->isSuccess();
        delete reply;
        return ret;
    }
    catch (AOException &e) {
        _logger->log(Logger::LOG_LEV_ERROR, "setGain failed: %s", e.what().c_str());
        return false;
    }
}


void AdsecArbSystem::selectFocalStation( string focalStation) {

   _logger->log(Logger::LOG_LEV_INFO, "Executing selectFocalStation...");

   try {
       selectFocalStationParams p;
       p._focalStation = focalStation;
       
       AdSec_Arbitrator::SelectFocalStation selectFocalStation( 10000, p);
       Command *reply = _adSecArbIntf->requestCommand(&selectFocalStation);
       setCommandResultFromReply(reply);
       delete reply;
   } catch (AOException &e) {
       _logger->log(Logger::LOG_LEV_ERROR, "selectFocalStation failed: %s", e.what().c_str());
   }
}


bool AdsecArbSystem::FeedbackRequest() {

    RequestFeedback feedbackReq(500);
    Command* feedbackReqReply;
    bool res = false;

    if(_simulation) {
       _logger->log(Logger::LOG_LEV_INFO, "Simulation mode. I do not try to register for alerts notification from AdSec arbitrator");
       return true;
    }

    _logger->log(Logger::LOG_LEV_INFO, "Trying to register for alerts notification from AdSec arbitrator...");
    
    try {
        // --- Request for feedback ---
        feedbackReqReply = _adSecArbIntf->requestCommand(&feedbackReq);
		if(feedbackReqReply->isSuccess()) {
			_logger->log(Logger::LOG_LEV_DEBUG, "Alerts notifications succesfully requested!");
            res = true;
		}
		else {
			_logger->log(Logger::LOG_LEV_WARNING, "Alerts notifications request failed");
		}
		delete feedbackReqReply;
    }
    catch(...) {
		_logger->log(Logger::LOG_LEV_ERROR, "Connection to AdSec arbitrator failed");
	}
   return res;
}

void AdsecArbSystem::reduceGain() {

    try {
        _adsecCurrGain.Update();
    }
    catch (...) {
        _logger->log(Logger::LOG_LEV_WARNING, "No gain on adsec - not applying reduced gain");
        return;
    }
    _adsecLastGainFilename = _adsecCurrGain.Get();
    setGain(0.1);
}

void AdsecArbSystem::zeroGain() {

    try {
        _adsecCurrGain.Update();
    }
    catch (...) {
        _logger->log(Logger::LOG_LEV_WARNING, "No gain on adsec - not applying reduced gain");
        return;
    }
    _adsecLastGainFilename = _adsecCurrGain.Get();
    setGain(0);
}

void AdsecArbSystem::restoreGain() {
    
    if (_adsecLastGainFilename != "") {
        if (setGain(_adsecLastGainFilename))
            _adsecLastGainFilename = "";
    }
    else {
        _logger->log(Logger::LOG_LEV_WARNING, "No previous gain - not restoring gain");
    }
}

#define NMODES (BcuMirror::N_CRATES*BcuMirror::N_DSP_CRATE*BcuMirror::N_CH_DSP_)
#define NGAINS (672)


int AdsecArbSystem::optimizeGain(double start_val, double range, double ttratio, int steps, int samples) {

    int stat;
    double gains[NGAINS];
    double gainsbest[NGAINS];
    int stepbest;
    double peaktopeak;
        

    // set starting point
    for (int i=0; i<NGAINS; i++) {
        if (i <= 1)
            gains[i] = start_val/ttratio;
        else
            gains[i] = start_val;
    }

    double rangestart = 0.0;
    double rangeend = range;
    double results[steps +1];

    // first move all together
    while (true) {
        stat = sweepGain(gains, rangestart, rangeend, ttratio, steps, 0, NGAINS - 1, samples, results, gainsbest, stepbest, peaktopeak);

        if (stat != 0) {
            return stat;
        }

        // check for real minimun (!!!)
        if ((stepbest > 2) and (stepbest < steps-2))
            break;

        rangestart += range/2.0;
        rangeend += range/2.0;

        if (_arbitrator->getStopFlag()) {
            _logger->log(Logger::LOG_LEV_ERROR, "Autogain sequence stopped by external alert");
            return GENERIC_SYS_ERROR;
        }
    }

    // copy best gains to base gain
    memcpy(gains, gainsbest, NGAINS*sizeof(double));

    // now move only tt
    rangestart = -range/2.0;
    rangeend = range/2.0;

    while (true) {
        stat = sweepGain(gains, rangestart, rangeend, ttratio, steps, 0, 1, samples, results, gainsbest, stepbest, peaktopeak);

        if (stat != 0) {
            return stat;
        }

        // check for real minimun (!!!)
        if ((stepbest > 2) and (stepbest < steps-2))
            break;

        rangestart += range/2.0;
        rangeend += range/2.0;

        if (_arbitrator->getStopFlag()) {
            _logger->log(Logger::LOG_LEV_ERROR, "Autogain sequence stopped by external alert");
            return GENERIC_SYS_ERROR;
            }
    }
    
    // copy best gains to base gain
    memcpy(gains, gainsbest, NGAINS*sizeof(double));

    
    // now move only ho
    rangestart = -range/2.0;
    rangeend = range/2.0;

    while (true) {
        stat = sweepGain(gains, rangestart, rangeend, ttratio, steps, 2, NGAINS-1, samples, results, gainsbest, stepbest, peaktopeak);

        if (stat != 0) {
            return stat;
        }

        // check for real minimun (!!!)
        if ((stepbest > 2) and (stepbest < steps-2))
            break;

        rangestart += range/2.0;
        rangeend += range/2.0;

        if (_arbitrator->getStopFlag()) {
            _logger->log(Logger::LOG_LEV_ERROR, "Autogain sequence stopped by external alert");
            return GENERIC_SYS_ERROR;
            }
    }
        
    string filename = Paths::GainDir( "KL", true) + "optimized_gain.fits";
    writeFits1D((char *)filename.c_str(), gainsbest, TDOUBLE, NGAINS);
    
    _logger->log(Logger::LOG_LEV_INFO, "Saved optimized gains file on %s", filename.c_str());

    return 0;
}    


/*
 *
 *  gainbase     starting gain vector
 *  start        starting offset from base gain (0 starts from the given gain array)
 *  end          ending offset from base gain
 *  ttratio      ratio applied to tt gains (step divided ttratio)
 *  steps        number of steps
 *  order1       lower order to be taken on rms
 *  order2       higher order to be taken on rms
 *  samples      number of measurements (that get averaged)
 *  results      rms results (array of size steps + 1)
 *  gainbest     best gain according to minimal rms
 */
int AdsecArbSystem::sweepGain(double gainbase[], double start, double end, double ttratio, int steps, int order1, int order2, int samples, double results[], double gainbest[], int &stepbest, double &peaktopeak)
{
    double bestrms = HUGE_VAL;
    double worserms = 0.0;
    
    stepbest = -1;
    peaktopeak = 0.0;

    double gstep = (end-start)/double(steps);

    double gainnew[NGAINS];

    // copy starting base gain
    memcpy(gainnew, gainbase, sizeof(double)*NGAINS);

    // correct to starting point
    for (int i = 0; i<NGAINS; i++) {
        if ((i >= order1) && (i <= order2)) {
            if (i <= 1)
                // for tt apply ratio
                gainnew[i] = fmax(gainnew[i] + start/ttratio, 0.0);
            else
                gainnew[i] = fmax(gainnew[i] + start, 0.0);
        }
    }

    for (int s = 0; s < steps + 1; s++) {
        char filename[40];
        sprintf(filename, "%soptimize_gain_step%d.fits", Paths::GainDir( "KL", true).c_str(), s);
        writeFits1D(filename, gainnew, TDOUBLE, NGAINS);

        // set gains
        setGain(filename);

        // wait a bit
        usleep(500000);

        // measure
        results[s] = getModesDiffRmsPart(order1, order2, samples);
        if (results[s] == HUGE_VAL) {
            // something went wrong
            return GENERIC_SYS_ERROR;
        }
        _logger->log(Logger::LOG_LEV_INFO, "Step %d, rms(%d,%d) = %.4g", s, order1, order2, results[s]);

        if (results[s] < bestrms) {
            bestrms = results[s];
            stepbest = s;
            memcpy(gainbest, gainnew, NGAINS*sizeof(double));
        }

        if (results[s] > worserms) {
            worserms = results[s];
        }

        // increment gains
        for (int i=0; i<NGAINS; i++) {
            if ((i >= order1) && (i <= order2)) {
                if (i <= 1) 
                    // for tt apply ratio
                    gainnew[i] += gstep/ttratio;
                else
                    gainnew[i] += gstep;
            }
        }
        if (_arbitrator->getStopFlag()) {
            _logger->log(Logger::LOG_LEV_ERROR, "Autogain sequence stopped by external alert");
            return GENERIC_SYS_ERROR;
            }
        
    }
    peaktopeak = worserms - bestrms;
    _logger->log(Logger::LOG_LEV_INFO, "Gain sweep finished, best rms %.4g (step %d - peak to peak %.4g)",
                 bestrms, stepbest, peaktopeak);
    
    return 0;
}


double AdsecArbSystem::getModesDiffRmsPart(int order1, int order2, int samples)
{
    int stat;
    double rms[NMODES];

    if IS_ERROR(stat = getModesDiffRms(rms, samples)) {
        _logger->log(Logger::LOG_LEV_ERROR, "Cannot read partial rms");
        return HUGE_VAL;
    }    

    double res = 0.0;
    for (int i=order1; i<=order2; i++)
        res += rms[i]*rms[i];
    
    return (sqrt(res/double(order2-order1+1)));
}
    


int AdsecArbSystem::getModesDiffRms(double *rms, int samples) {

    int stat;
    double diff[NMODES];

    for (int mod = 0; mod<NMODES; mod++)
        rms[mod] = 0.0;

    // do not take average as differences should converge to zero
    for (int samp = 0; samp<samples; samp++) {
        if IS_ERROR(stat = getModesDiff(diff)) {
            return stat;
        }
        for (int mod = 0; mod<NMODES; mod++)
            rms[mod] += diff[mod]*diff[mod];
    }
    for (int mod = 0; mod<NMODES; mod++)
        rms[mod] = sqrt(rms[mod]/double(samples));

    return 0;
}



int  AdsecArbSystem::getModesDiff(double *diff) {

    string bufname = "masterdiagnostic." + Utils::getAdoptSide() + ":ADSECBUF";
    AdsecDiagnRaw raw1, raw2; 
    int timeout = 300;
    BufInfo	* info;
    int stat;

    int tries1 = 10;
    do {
        // get first frame
        if IS_ERROR(stat = getFastDiagnosticRawData( _arbitrator->MyFullName(), bufname, &raw1, timeout, false, &info)) {
            _logger->log(Logger::LOG_LEV_ERROR, "Cannot read adsec raw data");
            return stat;
        }

        int tries2 = 10;
        do {
            // get second frame
            if IS_ERROR(stat = getFastDiagnosticRawData( _arbitrator->MyFullName(), bufname, &raw2, timeout, false, &info)) {
                _logger->log(Logger::LOG_LEV_ERROR, "Cannot read adsec raw data");
                return stat;
            }
            tries2--;
        } while ((raw1.crate[0].header.WFSFrameCounter == raw2.crate[0].header.WFSFrameCounter) && (tries2>0));
        tries1--;
    } while (((raw1.crate[0].header.WFSFrameCounter + 1) != raw2.crate[0].header.WFSFrameCounter) && (tries1>0));

    if (tries1 == 0) {
        _logger->log(Logger::LOG_LEV_ERROR, "Could not get consecutive adsec frames");
        return TIMEOUT_ERROR;
    }
    
    for (int cr=0; cr<BcuMirror::N_CRATES; cr++)
        for (int i=0; i<BcuMirror::N_DSP_CRATE; i++)
            for (int j=0; j<BcuMirror::N_CH_DSP_; j++)
                {
                    diff[cr*BcuMirror::N_CH_CRATE  + i*BcuMirror::N_CH_DSP_ + j] = 
                        raw2.crate[cr].dsp[i].Modes[j] - raw1.crate[cr].dsp[i].Modes[j];
                }

    return 0;
}
            

void AdsecArbSystem::setAcquireRefResult( acquireRefAOResult &result) {

	this->result = result;
}

acquireRefAOResult& AdsecArbSystem::getAcquireRefResult() {
	return result;
}

