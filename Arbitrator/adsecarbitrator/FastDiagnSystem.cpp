//@File: $Id$
//@

#include "adsecarbitrator/FastDiagnSystem.h"
#include "adsecarbitrator/AdSecArbitrator.h"
#include "../DiagnApp/diagnlib.h"

using namespace Arcetri::AdSec_Arbitrator;


//
FastDiagnSystem::FastDiagnSystem(bool simulation): AbstractSystem("FASTDIAGN") {

   _arbitrator = AbstractArbitrator::getInstance();
   _target = string(FASTDIAGNSYSTEM_PROCESS) + "." + _arbitrator->Side();
   _offloadEnabled=false;
   _simulation = simulation;

   _varOffload = NULL;
   string offloadVarName = "ADSEC."+_arbitrator->Side()+".OFLMODES";
   _arbitrator->Notify( offloadVarName, offloadHandler);

}

//
FastDiagnSystem::~FastDiagnSystem() {
	
}

//
int FastDiagnSystem::getSafeSkipCounter() {

   if(_simulation) return (-1);      // No frame skipped

   DiagnWhich filter("SwitchSafeSkipCounter", 0, 0);

   ValueDict dict;
   try {
      dict = GetDiagnValue( (char*)_target.c_str(), filter, 5000);
   } catch (AOException &e) {
      _logger->log(Logger::LOG_LEV_ERROR, e.what());
      return e._errcode;
   }

   ValueDict::iterator it = dict.begin(); 
   DiagnValue va = (*it).second;
   return (int)va.Last();
}

//
float FastDiagnSystem::getSafeSkipPercent() {

   if(_simulation) return 0.01;      // Andra' bene? speriamo di si.

   DiagnWhich filter("SkipFrameRate", 0, 0);

   ValueDict dict;
   try {
      dict = GetDiagnValue( (char*)_target.c_str(), filter, 1000);
   } catch (AOException &e) {
      _logger->log(Logger::LOG_LEV_ERROR, e.what());
      return e._errcode;
   }

   ValueDict::iterator it = dict.begin(); 
   DiagnValue va = (*it).second;
   return (float)va.Mean();
}

void FastDiagnSystem::startTTOffload() {
      setTTOffload(true);
}

void FastDiagnSystem::stopTTOffload() {
      setTTOffload(false);
}

void FastDiagnSystem::setTTOffload( bool enable) {

   // NOTE: because the operation consists in setting an RTDB variable
   //       we allow it to be done even in simulation mode
   int intzero=0;
   int intuno =1;
   string errString="";

   _logger->log(Logger::LOG_LEV_DEBUG, "Executing enableTTOffload(%s)...", (enable) ? "true": "false");
   //string varname = _arbitrator->Side() +"." + _target + ".DO_OFFLOAD_MODES.REQ";
   string varname = _target + ".DO_OFFLOAD_MODES.REQ";

   int stat = thWriteVar( varname.c_str(), INT_VARIABLE, 1, enable? (&intuno) : (&intzero), 2000);
   if (IS_ERROR(stat)) {
      _logger->log(Logger::LOG_LEV_ERROR, "Error in thWriteVar: (%d) %s", stat, lao_strerror(stat));
      errString = "Error in enableTTOffload()";
   }
   else
       _offloadEnabled = enable;
}

bool FastDiagnSystem::isTTOffloadEnabled() {
   return _offloadEnabled;
}

int FastDiagnSystem::offloadHandler( void *pt, Variable *var) {

    AdSecArbitrator *adsecarb = (AdSecArbitrator *)pt;
    FastDiagnSystem *system = (FastDiagnSystem*)(adsecarb->getSystems()["FASTDIAGN"]);

    if (!system->varOffload())
        system->varOffload() = new RTDBvar(var);
    else
        system->varOffload()->Set(var);

    return 0;
}

//
double FastDiagnSystem::getTipOffload() {

    double value=0;
    if (varOffload())
        varOffload()->Get(1, &value);

    return value;
}

//
double FastDiagnSystem::getTiltOffload() {

    double value=0;
    if (varOffload())
        varOffload()->Get(2, &value);

    return value;
}

//
double FastDiagnSystem::getFocusOffload() {

    double value=0;
    if (varOffload())
        varOffload()->Get(3, &value);

    return value;
}


//
int FastDiagnSystem::setDiagThresholdsSet(int set) {
 
    int result = -1;


    // two methods, one reading and writing params config files
    // and the other saving and setting direcctly the parameters
    //
#if 0
    // method 1: saves and the params via config files
    switch (set) 
        {
        case FASTDIAGNSYSTEM_RIPPING_SET:
            // first create a backup copy
            DumpDiagnVarParams((char*)_target.c_str(), FASTDIAGNSYSTEM_BACKUP_PARAMS, DIAGNSYSTEM_TIMEOUT );
            // load ripping parameters subset
            LoadDiagnVarParams((char*)_target.c_str(), FASTDIAGNSYSTEM_RIPPING_PARAMS, DIAGNSYSTEM_TIMEOUT);
            break;

        case FASTDIAGNSYSTEM_PREVIOUS_SET:
            // load previously created paramas backup
            LoadDiagnVarParams((char*)_target.c_str(), FASTDIAGNSYSTEM_BACKUP_PARAMS, DIAGNSYSTEM_TIMEOUT);
            break;
        }
#else
    // method 2: save and sets the params directly
	static ParamDict paramsCurrAve_map; // keeps the params map
	static ParamDict paramsCtrlCurr_map; // keeps the params map
	static ParamDict paramsDistRms_map; // keeps the params map

 	DiagnWhich whichCurrAve("CHCURRAVERAGE");
    DiagnWhich whichCtrlCurr("CHINTCONTROLCURRENT");
    DiagnWhich whichDistRms("CHDISTRMS");

    DiagnWhich which;
    ParamDict::iterator it;
    DiagnParam params;

    switch (set) 
        {
        case FASTDIAGNSYSTEM_RIPPING_SET:
            
            params.SetAlarmMin(-std::numeric_limits<float>::infinity());
            params.SetAlarmMax( std::numeric_limits<float>::infinity());

            paramsCurrAve_map = GetDiagnVarParam((char*)_target.c_str(), whichCurrAve, DIAGNSYSTEM_TIMEOUT);
            SetDiagnVarParam((char*)_target.c_str(), whichCurrAve, params, DIAGNSYSTEM_TIMEOUT);

            paramsCtrlCurr_map = GetDiagnVarParam((char*)_target.c_str(), whichCtrlCurr, DIAGNSYSTEM_TIMEOUT);
            SetDiagnVarParam((char*)_target.c_str(), whichCtrlCurr, params, DIAGNSYSTEM_TIMEOUT);

            paramsDistRms_map = GetDiagnVarParam((char*)_target.c_str(), whichDistRms, DIAGNSYSTEM_TIMEOUT);
            SetDiagnVarParam((char*)_target.c_str(), whichDistRms, params, DIAGNSYSTEM_TIMEOUT);

            break;

        case FASTDIAGNSYSTEM_PREVIOUS_SET:
            // restore previous configuration, assuming it was stored before, 
            // otherwise param maps should be empty

            // sets changed mask (!!!)
            for (it = paramsCurrAve_map.begin(); it != paramsCurrAve_map.end(); it++) {
                //it->second.SetMask(DiagnRange::MaskAlarmMin);
                //it->second.SetMask(DiagnRange::MaskAlarmMax);
                it->second.SetAlarmMin( it->second.AlarmMin());
                it->second.SetAlarmMax( it->second.AlarmMax());
            }
            for (it = paramsCtrlCurr_map.begin(); it != paramsCtrlCurr_map.end(); it++) {
                it->second.SetAlarmMin( it->second.AlarmMin());
                it->second.SetAlarmMax( it->second.AlarmMax());
            }
            for (it = paramsDistRms_map.begin(); it != paramsDistRms_map.end(); it++) {
                it->second.SetAlarmMin( it->second.AlarmMin());
                it->second.SetAlarmMax( it->second.AlarmMax());
            }

            SetDiagnVarParam((char*)_target.c_str(), paramsCurrAve_map, DIAGNSYSTEM_TIMEOUT);
            SetDiagnVarParam((char*)_target.c_str(), paramsCtrlCurr_map, DIAGNSYSTEM_TIMEOUT);
            SetDiagnVarParam((char*)_target.c_str(), paramsDistRms_map, DIAGNSYSTEM_TIMEOUT);

            /*
            // Do not use timeout to queue messages on the server.
            for (it = paramsCurrAve_map.begin(); it != paramsCurrAve_map.end(); it++) {
                it->second.SetAlarmMin( it->second.AlarmMin());
                it->second.SetAlarmMax( it->second.AlarmMax());
                SetDiagnVarParam((char*)_target.c_str(), it->first, it->second, -1);
            }
            for (it = paramsCtrlCurr_map.begin(); it != paramsCtrlCurr_map.end(); it++) {
                it->second.SetAlarmMin( it->second.AlarmMin());
                it->second.SetAlarmMax( it->second.AlarmMax());
                SetDiagnVarParam((char*)_target.c_str(), it->first, it->second, -1);
            }
            ParamDict::iterator endit = paramsDistRms_map.end();
            endit--;
            for (it = paramsDistRms_map.begin(); it != endit; it++) {
                it->second.SetAlarmMin( it->second.AlarmMin());
                it->second.SetAlarmMax( it->second.AlarmMax());
                SetDiagnVarParam((char*)_target.c_str(), it->first, it->second, -1);
            }
            // Send last message, whith a timeout to sync on the end
            it->second.SetAlarmMin( it->second.AlarmMin());
            it->second.SetAlarmMax( it->second.AlarmMax());
            SetDiagnVarParam((char*)_target.c_str(), it->first, it->second, 20000);
            */

            break;
        }
#endif
    
    return result;
}
