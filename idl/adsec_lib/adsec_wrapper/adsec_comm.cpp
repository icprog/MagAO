/* 
 * @File: adsec_comm.cpp
 * Implementation of the Bcu communication IDL interface for AdSec.
 * @
 */

#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>


extern "C"{
	#include "idl_export.h"
	#include "msglib.h"
	#include "commlib.h"
	#include "errlib.h"
	#include "errordb.h"
	#include "buflib.h"
	#include "rtdblib.h" 
	#include "common.h"
	#include "thrdlib.h"
}

#include "Logger.h"
#include "BcuCommon.h"
#include "diagnlib.h"
#include "adsec_comm.h"

using namespace Arcetri;
using namespace Arcetri::Bcu;

#include <string>
using namespace std;

//Global variables definition
int idl_setup_ok = 0;
IDL_LONG timeout = 0;
char *server_ip    = "127.0.0.1";
char MyName[PROC_NAME_LEN];
int ErrC = 0;
int flags = 0;

/*
 * Command sender used to communicate with the new MirrorCtrl
 * and a logger.
 */
BcuIdMap 	    _bcuIdMap;
BcuMasterUdpMap _bcuMasterUdpMap;
BcuIpMap 	    _bcuIpMap;
CommandSender*  _comSender = NULL;
Logger*		    _logger = Logger::get("MAIN", Logger::LOG_LEV_DEBUG);	// Use the same name of the AOApp main logger


// ****************** DLM LOAD SUPPORT *********************** //

// Define message codes and their corresponding printf(3) format
// strings. Note that message codes start at zero and each one is
// one less that the previous one. Codes must be monotonic and
// contiguous.
static IDL_MSG_DEF msg_arr[] = {
#define M_TM_INPRO                       0
     {  "M_TM_INPRO",   "%NThis is from a loadable module procedure." },
#define M_TM_INFUN                       -1
       {  "M_TM_INFUN",   "%NThis is from a loadable module function." },
};

// The load function fills in this message block handle with the
// opaque handle to the message block used for this module. The other
// routines can then use it to throw errors from this block.
static IDL_MSG_BLOCK msg_block;


void idl_free_cb(unsigned char* loc) {
    Logger::get()->log(Logger::LOG_LEV_TRACE, "IDL freeing memory starting at %p",(void*)loc);
    free(loc);
}


// --------------------------- NEW SECTION USING DLM FILE ---------------------------- //
// ----------------------------------------------------------------------------------- //


IDL_VPTR getBcuId(int lArgc, IDL_VPTR Argv[]) {
 	if(lArgc == 1) {
 		string bcuName = IDL_VarGetString(Argv[0]);
 		_logger->log(Logger::LOG_LEV_DEBUG, "Retrieving ID for %s", bcuName.c_str());
 		try {
 			int bcuId = _bcuIdMap[bcuName];
 			return IDL_GettmpLong(bcuId);
 		}
 		catch (BcuIdMapException& e) {
 			_logger->log(Logger::LOG_LEV_ERROR, "ID for %s is not existing !", bcuName.c_str());
 		}
 	}	
 	return IDL_GettmpLong(-1);
}


IDL_VPTR getBcuIp(int lArgc, IDL_VPTR Argv[]) {
 	if(lArgc == 1) {
 		string bcuName  = IDL_VarGetString(Argv[0]);
 		_logger->log(Logger::LOG_LEV_DEBUG, "Retrieving IP port for %s", bcuName.c_str());
 		try {
 			string bcuIp = _bcuIpMap[bcuName];
 			return IDL_StrToSTRING((char*)bcuIp.c_str());
 		}
 		catch (BcuIpMapException& e) {
 			_logger->log(Logger::LOG_LEV_ERROR, "IP port for %s is not existing !", bcuName.c_str());
 		}
 	}	
 	return IDL_StrToSTRING("");
}


IDL_VPTR getBcuMasterUdpPort(int lArgc, IDL_VPTR Argv[]) {
 	if(lArgc == 1) {
 		string bcuName = IDL_VarGetString(Argv[0]);
 		_logger->log(Logger::LOG_LEV_DEBUG, "Retrieving UDP MASTER port for %s", bcuName.c_str());
 		try {
 			int bcuUdpMasterPort = _bcuMasterUdpMap[bcuName];
 			return IDL_GettmpLong(bcuUdpMasterPort);
 		}
 		catch (BcuUdpMapException& e) {
 			_logger->log(Logger::LOG_LEV_ERROR, "UDP MASTER port for %s is not existing !", bcuName.c_str());
 		}
 	}	
 	return IDL_GettmpLong(-1);
}


///////////////////////////////////////////////////////////////////////////////////////////////////////
static IDL_STRUCT_TAG_DEF param_tags[] = {  
    { "FAMILY", 0, (void *) IDL_TYP_STRING},  
    { "INDEX", 0, (void *) IDL_TYP_LONG},  
    { "ALARM_MIN", 0, (void *) IDL_TYP_DOUBLE},  
    { "WARNING_MIN", 0, (void *) IDL_TYP_DOUBLE},  
    { "WARNING_MAX", 0, (void *) IDL_TYP_DOUBLE},  
    { "ALARM_MAX", 0, (void *) IDL_TYP_DOUBLE},  
    { "MEAN_PERIOD", 0, (void *) IDL_TYP_DOUBLE},  
    { "CAF", 0, (void *) IDL_TYP_ULONG},  
    { "ENABLED", 0, (void *) IDL_TYP_BYTE},  
    { "SLOW", 0, (void *) IDL_TYP_BYTE},  
    { 0 }  
};

typedef struct param_struct {  
    IDL_STRING family;  
    IDL_LONG   index;  
    double     alarm_min;
    double     warning_min;
    double     warning_max;
    double     alarm_max;
    double     mean_period;
    IDL_ULONG  caf;
    UCHAR      enabled;
    UCHAR      slow;
} IDL_Param;

static IDL_Param    *s_param;  
//
// GETDIAGNPARAM
//
// This function get diagnostic parameters from a diagnostic application (DiagnApp)
// A filter (family, from, to) is given as input to match a subset of the diagnostic variables (DiagnVars)
// 
// err = GetDiagnParam(diagnapp, family, from, to, vars , timeout=timeout)
// 
// INPUTS
//      diagnapp        Diagnostic application name ('FASTDGN00', 'HOUSEKPR00')
//      family          Family name of the DiagnVars to retrieve, wildcards allowed ('ChDistAverage', '*Temp*')
//      from            Index of the first DiagnVar to read from, -1 means "from the first one"
//      to              Index of the last DiagnVar to read from, -1 means "to the last one"
//
// OUTPUT
//      vars            Matched Vars. Array of structures {FAMILY, INDEX, ALARM_MIN, WARNING_MIN, WARNING_MAX,
//                                                         ALARM_MAX, MEAN_PERIOD, CAF, ENABLED, SLOW}
//                      Each element of the array corresponds to a matched DiagnVar where,
//                      FAMILY          is the family name,
//                      INDEX           is the index,
//                      ALARM_MIN       below this threshold an alarm event is triggered
//                      WARNING_MIN     below this threshold an warning event is triggered
//                      WARNING_MAX     above this threshold an warning event is triggered
//                      ALARM_MAX       above this threshold an alarm event is triggered
//                      MEAN_PERIOD     period of the running mean average [s]
//                      CAF             Consecutive Allowed Faults. Should be 0. Untested
//                      ENABLED         1 if the DiagnVar is enabled, 0 if is disabled
//                      SLOW            1 if the DiagnVar run in the "slow" thread, 0 if it runs in the "fast" thread
// 
// KEYWORDS
//      TIMEOUT         timeout in the communication with the DiagnApp [s]

IDL_VPTR IDLGetDiagnParam(int Argc, IDL_VPTR Argv[], char *Argk)
{
    typedef struct {  
        IDL_KW_RESULT_FIRST_FIELD;   // Must be first entry in this structure  
        double timeout;  
    } KW_RESULT;  
    static IDL_KW_PAR kw_pars[] = { //Lexically sorted! 
        { "TIMEOUT"  , IDL_TYP_DOUBLE, 1, 0, 0, (char*)IDL_KW_OFFSETOF(timeout) },  
        { NULL }  
    };  
   
    KW_RESULT           kw;
    char                *diagnapp,*family;
    int                 from, to, tmout, i;
    ParamDict           dict;
    IDL_VPTR            params;  
    static IDL_MEMINT   n_ele = 0;  

    kw.timeout = 1.0;  
    (void) IDL_KWProcessByOffset(Argc, Argv, Argk, kw_pars, NULL, 1, &kw); 
    tmout = (int) (kw.timeout * 1000);
    IDL_KW_FREE;

    diagnapp=IDL_VarGetString(Argv[0]);
    family  =IDL_VarGetString(Argv[1]);
    from = IDL_LongScalar(Argv[2]);
    to   = IDL_LongScalar(Argv[3]);
    params  = Argv[4];

    _logger->log(Logger::LOG_LEV_TRACE, "app %s, family %s, from %d, to %d, timeout %d",
            diagnapp, family, from, to, tmout);
  
    try{
        DiagnWhich which(family, from, to);
        dict = GetDiagnVarParam( diagnapp,which, tmout);
    } catch (AOException &e) { 
        _logger->log(Logger::LOG_LEV_ERROR, e.what());
        return IDL_GettmpLong(e._errcode);
    }

    
    // Create the structure definition   
    void *p = IDL_MakeStruct(0, param_tags);  

    // Allocate memory for struct array
    n_ele=dict.size();
    s_param   = (IDL_Param*)malloc(n_ele*sizeof(IDL_Param));

    // Fill structure fields
    i=0;
    for (ParamDict::iterator it = dict.begin(); it != dict.end();  it++){
        DiagnWhich wh = (*it).first;
        DiagnParam pa = (*it).second;
       
        std::ostringstream oss; oss << wh << " :: " << pa;
        _logger->log(Logger::LOG_LEV_DEBUG,  oss.str() );

        IDL_StrStore(&s_param[i].family, (char*)(wh.Family().c_str()));
        s_param[i].index = wh.From();

        DiagnRange<double> rng = pa.Range();
        s_param[i].alarm_min   = rng.AlarmMin();
        s_param[i].warning_min = rng.WarningMin();
        s_param[i].warning_max = rng.WarningMax();
        s_param[i].alarm_max   = rng.AlarmMax();
        s_param[i].mean_period = pa.MeanPeriod();
        s_param[i].caf         = pa.ConsAllowFaults();
        s_param[i].enabled     = pa.isEnabled();
        s_param[i].slow        = pa.isSlow();

        i++;
    }

    // Import the data area s_matched into an IDL structure, note that no data are moved. 
    IDL_VPTR v_pa = IDL_ImportArray( 1, &n_ele, IDL_TYP_STRUCT, (UCHAR *) s_param  , idl_free_cb, p);
    IDL_VarCopy(v_pa,params);

    return IDL_GettmpLong(NO_ERROR);
}


//
///
// SETDIAGNPARAM
//
// This function set diagnostic parameters from a diagnostic application (DiagnApp)
// A filter (family, from, to) is given as input to match a subset of the diagnostic variables (DiagnVars)
// One or more parameters can be changed using the corresponding keywords without affecting the other parameters.
//
// 
// mod_vars = SetDiagnParam(diagnapp, family, from, to, vars , ALARM_MAX=alarm_max, ALARM_MIN=alarm_min, 
//                     CONS_ALLOWED_FAULTS=caf, ENABLED=enabled, MEAN_PERIOD=mean_period, SLOW=slow, TIMEOUT=timeout,
//                     WARNING_MAX=warning_max,  WARNING_MIN=warning_min)
// 
// RETURN VALUE
//       This function return the number of modified DiagnVars
// 
//
// INPUTS
//      diagnapp        Diagnostic application name ('FASTDGN00', 'HOUSEKPR00')
//      family          Family name of the DiagnVars to match, wildcards allowed ('ChDistAverage', '*Temp*')
//      from            Index of the first DiagnVar to match, -1 means "from the first one"
//      to              Index of the last DiagnVar to match, -1 means "to the last one"
//
// KEYWORDS
//      ALARM_MIN       below this threshold an alarm event is triggered
//      WARNING_MIN     below this threshold an warning event is triggered
//      WARNING_MAX     above this threshold an warning event is triggered
//      ALARM_MAX       above this threshold an alarm event is triggered
//      MEAN_PERIOD     period of the running mean average [s]
//      CAF             Consecutive Allowed Faults. Should be 0. Untested
//      ENABLED         1 if the DiagnVar is enabled, 0 if is disabled
//      SLOW            1 if the DiagnVar run in the "slow" thread, 0 if it runs in the "fast" thread
//      TIMEOUT         timeout in the communication with the DiagnApp [s]
//

IDL_VPTR IDLSetDiagnParam(int Argc, IDL_VPTR Argv[], char *Argk)
{
    typedef struct {  
        IDL_KW_RESULT_FIRST_FIELD;   // Must be first entry in this structure  
        double alarm_min;  
        double warning_min;  
        double warning_max;  
        double alarm_max;  
        double mean_period;  
        IDL_ULONG caf;
        UCHAR    ena;
        UCHAR    slow;
        double timeout;  
        int alarm_min_there;  
        int warning_min_there;  
        int warning_max_there;  
        int alarm_max_there;  
        int mean_period_there;  
        int caf_there;
        int ena_there;
        int slow_there;
    } KW_RESULT;  
    static IDL_KW_PAR kw_pars[] = { //Lexically sorted! 
        { "ALARM_MAX"  , IDL_TYP_DOUBLE, 1, 0, (int*)IDL_KW_OFFSETOF(alarm_max_there), 
            (char*)IDL_KW_OFFSETOF(alarm_max) },  
        { "ALARM_MIN"  , IDL_TYP_DOUBLE, 1, 0, (int*)IDL_KW_OFFSETOF(alarm_min_there), 
            (char*)IDL_KW_OFFSETOF(alarm_min) },  
        { "CONS_ALLOWED_FAULTS", IDL_TYP_LONG, 1, 0, (int*)IDL_KW_OFFSETOF(caf_there),
            (char*)IDL_KW_OFFSETOF(caf) },  
        { "ENABLED", IDL_TYP_BYTE, 1, 0, (int*)IDL_KW_OFFSETOF(ena_there),
            (char*)IDL_KW_OFFSETOF(ena) },  
        { "MEAN_PERIOD", IDL_TYP_DOUBLE, 1, 0, (int*)IDL_KW_OFFSETOF(mean_period_there),
            (char*)IDL_KW_OFFSETOF(mean_period) },  
        { "SLOW", IDL_TYP_BYTE, 1, 0, (int*)IDL_KW_OFFSETOF(slow_there),
            (char*)IDL_KW_OFFSETOF(slow) },  
        { "TIMEOUT"    , IDL_TYP_DOUBLE, 1, 0, 0, (char*)IDL_KW_OFFSETOF(timeout) },  
        { "WARNING_MAX"  , IDL_TYP_DOUBLE, 1, 0, (int*)IDL_KW_OFFSETOF(warning_max_there), 
            (char*)IDL_KW_OFFSETOF(warning_max) },  
        { "WARNING_MIN"  , IDL_TYP_DOUBLE, 1, 0, (int*)IDL_KW_OFFSETOF(warning_min_there), 
            (char*)IDL_KW_OFFSETOF(warning_min) },  
        { NULL }  
    };  
    KW_RESULT           kw;


    char                *diagnapp,*family;
    int                 from, to, tmout, c_matched;
    DiagnParam          params;
    IDL_VPTR            idl_params, s_tag;  
    IDL_VPTR            c_args[5];       //contains argv after keyword processing
    IDL_MEMINT          ret;
    
    // set to NULL to check if params struct has been passed as a plain
    // argument
    c_args[4] = NULL;     


    kw.timeout = 1.0;  
    (void) IDL_KWProcessByOffset(Argc, Argv, Argk, kw_pars, c_args, 1, &kw); 
    tmout = (int) (kw.timeout * 1000);

    // c_args and Argv are the same, because the previous call 
    // sorts the plain arguments into the front of Argv.
    // But c_args[4] is not null iif the params struct has been 
    // passed as a plain argument
    diagnapp=IDL_VarGetString(Argv[0]);
    family  =IDL_VarGetString(Argv[1]);
    from = IDL_LongScalar(Argv[2]);
    to   = IDL_LongScalar(Argv[3]);

    // build DiagnParam object
    if(c_args[4] !=NULL ){   // params struct passed as a plain argument, ignore keywords 
        IDL_KW_FREE;
        idl_params  = Argv[4];
        if(idl_params->type != IDL_TYP_STRUCT) return IDL_GettmpLong(IDL_INPUT_TYPE);

        ret = IDL_StructTagInfoByName(idl_params->value.s.sdef, "ALARM_MIN", 0,  &s_tag);
        if (ret != -1 && s_tag->type == IDL_TYP_DOUBLE)
            params.SetAlarmMin( *(double*)(idl_params->value.s.arr->data + ret) );
        else return IDL_GettmpLong(IDL_INPUT_TYPE);

        ret = IDL_StructTagInfoByName(idl_params->value.s.sdef, "ALARM_MAX", 0,  &s_tag);
        if (ret != -1 && s_tag->type == IDL_TYP_DOUBLE) 
            params.SetAlarmMax( *(double*)(idl_params->value.s.arr->data + ret) );
        else return IDL_GettmpLong(IDL_INPUT_TYPE);

        ret = IDL_StructTagInfoByName(idl_params->value.s.sdef, "WARNING_MIN", 0,  &s_tag);
        if (ret != -1 && s_tag->type == IDL_TYP_DOUBLE) 
            params.SetWarningMin( *(double*)(idl_params->value.s.arr->data + ret) );
        else return IDL_GettmpLong(IDL_INPUT_TYPE);

        ret = IDL_StructTagInfoByName(idl_params->value.s.sdef, "WARNING_MAX", 0,  &s_tag);
        if (ret != -1 && s_tag->type == IDL_TYP_DOUBLE) 
            params.SetWarningMax( *(double*)(idl_params->value.s.arr->data + ret) );
        else return IDL_GettmpLong(IDL_INPUT_TYPE);

        ret = IDL_StructTagInfoByName(idl_params->value.s.sdef, "MEAN_PERIOD", 0,  &s_tag);
        if (ret != -1 && s_tag->type == IDL_TYP_DOUBLE) 
            params.SetMeanPeriod( *(double*)(idl_params->value.s.arr->data + ret) );
        else return IDL_GettmpLong(IDL_INPUT_TYPE);

        ret = IDL_StructTagInfoByName(idl_params->value.s.sdef, "CAF", 0,  &s_tag);
        if (ret != -1 && s_tag->type == IDL_TYP_ULONG) 
            params.SetConsAllowFaults( *(IDL_ULONG*)(idl_params->value.s.arr->data + ret) );
        else return IDL_GettmpLong(IDL_INPUT_TYPE);

        ret = IDL_StructTagInfoByName(idl_params->value.s.sdef, "ENABLED", 0,  &s_tag);
        if (ret != -1 && s_tag->type == IDL_TYP_BYTE) 
            params.SetEnabled( *(UCHAR*)(idl_params->value.s.arr->data + ret) );
        else return IDL_GettmpLong(IDL_INPUT_TYPE);

        ret = IDL_StructTagInfoByName(idl_params->value.s.sdef, "SLOW", 0,  &s_tag);
        if (ret != -1 && s_tag->type == IDL_TYP_BYTE) 
            params.SetSlow( *(UCHAR*)(idl_params->value.s.arr->data + ret) );
        else return IDL_GettmpLong(IDL_INPUT_TYPE);

    } else { // params defined by keywords
        if (kw.alarm_min_there)     params.SetAlarmMin(kw.alarm_min);
        if (kw.alarm_max_there)     params.SetAlarmMax(kw.alarm_max);
        if (kw.warning_min_there)   params.SetWarningMin(kw.warning_min);
        if (kw.warning_max_there)   params.SetWarningMax(kw.warning_max);
        if (kw.mean_period_there)   params.SetMeanPeriod(kw.mean_period);
        if (kw.caf_there)           params.SetConsAllowFaults(kw.caf);
        if (kw.ena_there)           params.SetEnabled(kw.ena);
        if (kw.slow_there)          params.SetSlow(kw.slow);
    }
    IDL_KW_FREE;


    try{
        DiagnWhich which(family, from, to);
        std::ostringstream oss; oss  << "[SetDiagnParam] " << diagnapp << " " << which << " --- " << params;
        _logger->log(Logger::LOG_LEV_DEBUG, oss.str());
        c_matched = SetDiagnVarParam( diagnapp, which, params, tmout);
    } catch (AOException &e) { 
        _logger->log(Logger::LOG_LEV_ERROR, e.what());
        return IDL_GettmpLong(e._errcode);
    }

    return IDL_GettmpLong(c_matched);
}

///////////////////////////////////////////////////////////////////////////////////////////


static IDL_STRUCT_TAG_DEF value_tags[] = {  
    { "FAMILY", 0, (void *) IDL_TYP_STRING},  
    { "INDEX", 0, (void *) IDL_TYP_LONG},  
    { "AVERAGE", 0, (void *) IDL_TYP_DOUBLE},  
    { "STDDEV", 0, (void *) IDL_TYP_DOUBLE},  
    { "LAST", 0, (void *) IDL_TYP_DOUBLE},  
    { "TIME", 0, (void *) IDL_TYP_DOUBLE},  
    { "STATUS", 0, (void *) IDL_TYP_LONG},  
    { 0 }  
};  

typedef struct value_struct {  
    IDL_STRING family;  
    IDL_LONG   index;  
    double     mean;
    double     stddev;
    double     last;
    double     time;
    IDL_LONG   status;
} IDL_Value;  

static IDL_Value    *s_value;  
//
// GETDIAGNVALUE
//
// This function get diagnostic values from a diagnostic application (DiagnApp)
// A filter is given as input to match a subset of the diagnostic variables (DiagnVars)
// 
//
// 
// err = GetDiagnValue(diagnapp, family, from, to, vars , timeout=timeout)
// 
// INPUTS
//      diagnapp        Diagnostic application name ('FASTDGN00', 'HOUSEKPR00')
//      family          Family name of the DiagnVars to retrieve, wildcards allowed ('ChDistAverage', '*Temp*')
//      from            Index of the first DiagnVar to read from, -1 means "from the first one"
//      to              Index of the last DiagnVar to read from, -1 means "to the last one"
//
// OUTPUT
//      vars            Matched Vars. Array of structures {FAMILY, INDEX, AVERAGE, STDDEV, LAST, TIME, STATUS}.
//                      Each element of the array corresponds to a matched DiagnVar where,
//                      FAMILY  is the family name,
//                      INDEX   is the index,
//                      AVERAGE is the average value of the running mean buffer (see GetDiagParam),
//                      STDDEV  is the stddev of the values in the running mean buffer
//                      LAST    is the last value inserted in the running mean buffer
//                      TIME    is the timestamp corresponding to the average value
//                      STATUS  can be one of 0=OK, 1=WARNING, 2=ALARM
//
// KEYWORDS
//      TIMEOUT         timeout in the communication with the DiagnApp [s]
//
IDL_VPTR IDLGetDiagnValue(int Argc, IDL_VPTR Argv[], char *Argk)
{
    typedef struct {  
        IDL_KW_RESULT_FIRST_FIELD;   // Must be first entry in this structure  
        double timeout;  
    } KW_RESULT;  
    static IDL_KW_PAR kw_pars[] = { //Lexically sorted! 
        { "TIMEOUT"  , IDL_TYP_DOUBLE, 1, 0, 0, (char*)IDL_KW_OFFSETOF(timeout) },  
        { NULL }  
    };  
   
    KW_RESULT           kw;
    char                *diagnapp,*family;
    int                 from, to, tmout, i;
    ValueDict           dict;
    IDL_VPTR            values;  
    static IDL_MEMINT   n_ele = 0;  

    kw.timeout = 1.0;  
    (void) IDL_KWProcessByOffset(Argc, Argv, Argk, kw_pars, NULL, 1, &kw); 
    tmout = (int) (kw.timeout * 1000);
    IDL_KW_FREE;

    diagnapp=IDL_VarGetString(Argv[0]);
    family  =IDL_VarGetString(Argv[1]);
    from = IDL_LongScalar(Argv[2]);
    to   = IDL_LongScalar(Argv[3]);
    values  = Argv[4];

    try{
        DiagnWhich which(family, from, to);
        dict = GetDiagnValue( diagnapp,which, tmout);
    } catch (AOException &e) { 
        _logger->log(Logger::LOG_LEV_ERROR, e.what());
        return IDL_GettmpLong(e._errcode);
    }

    
    // Create the structure definition   
    void *p = IDL_MakeStruct(0, value_tags);  

    // Allocate memory for struct array
    n_ele=dict.size();
    s_value   = (IDL_Value*)malloc(n_ele*sizeof(IDL_Value));

    // Fill structure fields
    i=0;
    for (ValueDict::iterator it = dict.begin(); it != dict.end();  it++,i++){
        DiagnWhich wh = (*it).first;
        DiagnValue va = (*it).second;

        IDL_StrStore(&s_value[i].family, (char*)(wh.Family().c_str()));
        s_value[i].index = wh.From();

        s_value[i].mean        = va.Mean();
        s_value[i].stddev      = va.Stddev();
        s_value[i].last        = va.Last();
        s_value[i].time        = va.Time();
        s_value[i].status      = va.Status();

        _logger->log(Logger::LOG_LEV_DEBUG, "[GetDiagnValue] %s  %s --- %s [%s:%d]",
                diagnapp, wh.str().c_str(), va.str().c_str(), __FILE__,__LINE__);
    }

    // Import the data area s_matched into an IDL structure, note that no data are moved. 
    //matched = IDL_ImportNamedArray("pippo", 1, &n_ele, IDL_TYP_STRUCT, (UCHAR *) s_matched, 0, s);
    IDL_VPTR v_va = IDL_ImportArray( 1, &n_ele, IDL_TYP_STRUCT, (UCHAR *) s_value  , idl_free_cb, p);
    IDL_VarCopy(v_va,values);

    return IDL_GettmpLong(NO_ERROR);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static IDL_MEMINT values_dims[] = { 1, 1 };
static IDL_MEMINT times_dims[]  = { 1, 1 };
static IDL_STRUCT_TAG_DEF buffer_tags[] = {  
    { "FAMILY", 0, (void *) IDL_TYP_STRING},  
    { "INDEX", 0, (void *) IDL_TYP_LONG},  
    { "TIMES",  times_dims,  (void *) IDL_TYP_DOUBLE},  
    { "VALUES", values_dims, (void *) IDL_TYP_DOUBLE},  
    { 0 }  
};  


//
// GETDIAGNBUFFER
//
// This function retrieve the values in the running mean buffers from a diagnostic application (DiagnApp)
// A filter is given as input to match a subset of the diagnostic variables (DiagnVars)
// A DiagnVar stores in the running mean buffer both the values and the timestamps.
// 
// 
// err = GetDiagnBuffer(diagnapp, family, from, to, vars , timeout=timeout)
// 
// INPUTS
//      diagnapp        Diagnostic application name ('FASTDGN00', 'HOUSEKPR00')
//      family          Family name of the DiagnVars to retrieve, wildcards allowed ('ChDistAverage', '*Temp*')
//      from            Index of the first DiagnVar to read from, -1 means "from the first one"
//      to              Index of the last DiagnVar to read from, -1 means "to the last one"
//
// OUTPUT
//      vars            Matched Vars. Structure of structures {V0000, V0001, V0002, V000N} 
//                      where N+1 is the number of matched variables
//                      Each Vxxx is of the form:
//                      FAMILY     family name,
//                      INDEX      index,
//                      TIMES      timestamps of the values in the running mean buffer [s]
//                      VALUES     values in the running mean buffer
//
// KEYWORDS
//      TIMEOUT         timeout in the communication with the DiagnApp [s]
//
// EXAMPLE
// In the following the buffers of every DiagnVar matching the name '*Temp*' of the DiagnApp HOUSEKPR00
// are retrieved. 
//
// IDL> print, getdiagnbuffer('HOUSEKPR00', '*Temp*', -1, -1, vars, time=2)
// IDL> help, vars
// VARS            STRUCT    = -> <Anonymous> Array[1]
// IDL> help, vars, /str
// ** Structure <a03a474>, 31 tags, length=2656, data length=2656, refs=1:
//   V0000            STRUCT    -> <Anonymous> Array[1]
//   V0001            STRUCT    -> <Anonymous> Array[1]
//   V0002            STRUCT    -> <Anonymous> Array[1]
//   V0003            STRUCT    -> <Anonymous> Array[1]
//   V0004            STRUCT    -> <Anonymous> Array[1]
//   V0005            STRUCT    -> <Anonymous> Array[1]
// IDL> help, vars.v0000, /str
// ** Structure <a0359c4>, 4 tags, length=96, data length=96, refs=2:
//   FAMILY          STRING    'BCUPowerTemp'
//   INDEX           LONG                 0
//   TIMES           DOUBLE    Array[5]
//   VALUES          DOUBLE    Array[5]
//IDL> print, vars.v0000.times
//       209.85261       210.88453       211.91236       212.94063       213.96821
//IDL> print, vars.v0000.values
//       31.562500       31.562500       31.500000       31.500000       31.500000
//
//  

IDL_VPTR IDLGetDiagnBuffer(int Argc, IDL_VPTR Argv[], char *Argk)
{
    typedef struct {  
        IDL_KW_RESULT_FIRST_FIELD;   // Must be first entry in this structure  
        double timeout;  
    } KW_RESULT;  
    static IDL_KW_PAR kw_pars[] = { //Lexically sorted! 
        { "TIMEOUT"  , IDL_TYP_DOUBLE, 1, 0, 0, (char*)IDL_KW_OFFSETOF(timeout) },  
        { NULL }  
    };  
   
    KW_RESULT           kw;
    char                *diagnapp,*family;
    int                 from, to, tmout, i;
    BufferDict          dict;
    IDL_VPTR            buffer;  
    static IDL_MEMINT   uno=1;  

    kw.timeout = 1.0;  
    (void) IDL_KWProcessByOffset(Argc, Argv, Argk, kw_pars, NULL, 1, &kw); 
    tmout = (int) (kw.timeout * 1000);
    IDL_KW_FREE;

    diagnapp=IDL_VarGetString(Argv[0]);
    family  =IDL_VarGetString(Argv[1]);
    from = IDL_LongScalar(Argv[2]);
    to   = IDL_LongScalar(Argv[3]);
    buffer  = Argv[4];

    // Get data from DiagnApp
    try{
        DiagnWhich which(family, from, to);
        dict = GetDiagnBuffer( diagnapp,which, tmout);
    } catch (AOException &e) { 
        _logger->log(Logger::LOG_LEV_ERROR, e.what());
        return IDL_GettmpLong(e._errcode);
    }

    Logger::get()->log(Logger::LOG_LEV_TRACE, "[GetDiagnBuffer] Returned %d elements in dicts  [%s:%d]", 
            dict.size(), __FILE__,__LINE__);
    
    // check n of returned elements
    if (dict.size() == 0) {
        IDL_VPTR v_bu  = IDL_GettmpLong(0);
        IDL_VarCopy(v_bu,buffer);
        return IDL_GettmpLong(NO_ERROR);
    }
    

    // Alloc size for data
    int size=0;
    for (BufferDict::iterator it = dict.begin(); it != dict.end();  it++){
        DiagnBufferPtr bu = (*it).second;
        size += sizeof(IDL_STRING);
        size += sizeof(IDL_LONG);
        size += bu->Values().NElem()*sizeof(double);
        size += bu->Times().NElem()*sizeof(double);
    }
    char *s_data = (char*)malloc(size);
    Logger::get()->log(Logger::LOG_LEV_TRACE, "[GetDiagnBuffer] Malloc'ed %d bytes in %p [%s:%d]", 
            size, s_data, __FILE__,__LINE__);


    // I need to create the IDL_STRUCT_TAG_DEF [] at run time
    // because I don't know a priory the number of objects
    IDL_STRUCT_TAG_DEF *struct_tags = (IDL_STRUCT_TAG_DEF*) malloc(sizeof(IDL_STRUCT_TAG_DEF) * (dict.size()+1) );
    IDL_STRUCT_TAG_DEF *tag;
    for (unsigned int i=0; i<dict.size(); i++){
        tag =  &struct_tags[i];
        tag->name=(char*)malloc(6);
        snprintf(tag->name,6,"V%04d",i);
        tag->dims=(IDL_MEMINT*) malloc(2*sizeof(IDL_MEMINT));
        tag->dims[0]=1;
        tag->dims[1]=1;
        tag->type=NULL;
        tag->flags=0;
    }
    // terminating the array of IDL_STRUCT_TAG_DEF    
    tag =  &struct_tags[dict.size()];
    tag->name=0;

    // create substructs
    i=0;
    char **s = (char**) malloc(dict.size() * sizeof(void*));
    for (BufferDict::iterator it = dict.begin(); it != dict.end();  it++, i++){
        DiagnBufferPtr bu = (*it).second;

        times_dims[1]   = bu->Times().NElem();
        values_dims[1]  = bu->Values().NElem();
        s[i] = (char*)IDL_MakeStruct(NULL, buffer_tags);
        Logger::get()->log(Logger::LOG_LEV_TRACE, "[GetDiagnBuffer]  s[%d] %p [%s:%d]", 
                i,s[i] , __FILE__,__LINE__);
        struct_tags[i].type =s[i] ;
    }
    // create  main struct
    char * struct_s = (char*)IDL_MakeStruct(NULL, struct_tags); 
 
    
    i=0;
    double * d_ptr;
    IDL_STRING * s_ptr;
    IDL_LONG * l_ptr;
    char * tmp_ptr = s_data;
    for (BufferDict::iterator it = dict.begin(); it != dict.end();  it++, i++){
        DiagnBufferPtr bu = (*it).second;
        DiagnWhich  wh = (*it).first;

        // Family
        s_ptr = (IDL_STRING *)(tmp_ptr); 
        IDL_StrStore(s_ptr, (char*)(wh.Family().c_str())  ); 
        Logger::get()->log(Logger::LOG_LEV_TRACE, "[GetDiagnBuffer] Assigned %p: %s (%s) [%s:%d]", 
                s_ptr, IDL_STRING_STR(s_ptr), (char*)(wh.Family().c_str()) , __FILE__,__LINE__);
        tmp_ptr += sizeof(IDL_STRING);

        // From
        l_ptr = (IDL_LONG *)( tmp_ptr); 
        *l_ptr = wh.From();
        Logger::get()->log(Logger::LOG_LEV_TRACE, "[GetDiagnBuffer] Assigned %p: %d (%d) [%s:%d]", 
                l_ptr, *l_ptr, wh.From() , __FILE__,__LINE__);
        tmp_ptr += sizeof(IDL_LONG);

        // Times
        d_ptr = (double *)(tmp_ptr); 
        RunningMean<double>& rm_times =  bu->Times();
        for(RunningMean<double>::iterator rm = rm_times.begin(); rm != rm_times.end(); rm++){
            *d_ptr++ = *rm;
            Logger::get()->log(Logger::LOG_LEV_TRACE, "[GetDiagnBuffer] Assigned %p: %g (%g) [%s:%d]", 
                    d_ptr, *(d_ptr-1), *rm, __FILE__,__LINE__);
            tmp_ptr += sizeof(double);
            }

        // Values
        d_ptr = (double *)(tmp_ptr); 
        RunningMean<double>& rm_values =  bu->Values();
        for(RunningMean<double>::iterator rm = rm_values.begin(); rm != rm_values.end(); rm++){
            *d_ptr++ = *rm;
            Logger::get()->log(Logger::LOG_LEV_TRACE, "[GetDiagnBuffer] Assigned %p: %g (%g) [%s:%d]", 
                    d_ptr, *(d_ptr-1), *rm, __FILE__,__LINE__);
            tmp_ptr += sizeof(double);
        }
        Logger::get()->log(Logger::LOG_LEV_DEBUG, "[GetDiagnBuffer] %s  %s --- %s [%s:%d]",
                diagnapp, wh.str().c_str(), (*bu).str().c_str(), __FILE__,__LINE__);
    }

    // attach data to the created structure
    IDL_VPTR vv = IDL_ImportArray( 1, &uno, IDL_TYP_STRUCT, (UCHAR *)s_data, idl_free_cb, struct_s);
    IDL_VarCopy(vv, buffer);
    
    return IDL_GettmpLong(NO_ERROR);
}


//
// SetLogLevel
// 
// SetLogLevel(AOApp, /decr, /incr, /time)
// 
// INPUT
//      AOApp           AO application name ('FASTDGN', 'TTCTRL', ...) 
//                      '' means this library
// 
// KEYWORD
//      increase        increase verbosity
//      decrease        decrease verbosity
//      timeout         timeout [s] (def. 1s)
//
// RETURN
//      NO_ERROR
//
IDL_VPTR IDLSetLogLevel(int Argc, IDL_VPTR Argv[], char *Argk)
{
	
    typedef struct {  
        IDL_KW_RESULT_FIRST_FIELD;   // Must be first entry in this structure  
        UCHAR  increase;
        UCHAR  decrease;
        double timeout;  
        int    increase_there;
        int    decrease_there;
    } KW_RESULT;  
    static IDL_KW_PAR kw_pars[] = { //Lexically sorted! 
        { "DECREASE" , IDL_TYP_BYTE,    1, 0,  (int*)IDL_KW_OFFSETOF(decrease_there), 
             (char*)IDL_KW_OFFSETOF(decrease) },  
        { "INCREASE" , IDL_TYP_BYTE,    1, 0,  (int*)IDL_KW_OFFSETOF(increase_there), 
             (char*)IDL_KW_OFFSETOF(increase) },  
        { "TIMEOUT"  , IDL_TYP_DOUBLE, 1, 0, 0, (char*)IDL_KW_OFFSETOF(timeout) },  
        { NULL }  
    };  
   
    KW_RESULT           kw;
    char                *aoapp;
    int                 tmout;

    kw.timeout = 1.0;  
    (void) IDL_KWProcessByOffset(Argc, Argv, Argk, kw_pars, NULL, 1, &kw); 
    tmout = (int) (kw.timeout * 1000);

    aoapp=IDL_VarGetString(Argv[0]);

    if ( kw.decrease_there && kw.increase_there ) {
        IDL_KW_FREE;
        return  IDL_GettmpLong(NO_ERROR);
    }
    if (kw.increase_there){
        if (strcmp(aoapp,"")==0) {
            _logger->setLevel( _logger->getLevel()+1);
        } else {
            thSendMsg(0,aoapp,LOGINCR,DISCARD_FLAG, NULL);
        }
        _logger->log(Logger::LOG_LEV_INFO, "log level of [%s] increased",aoapp);
    }
    if (kw.decrease_there){
        if (strcmp(aoapp,"")==0) {
            _logger->setLevel( _logger->getLevel()-1);
        } else {
            thSendMsg(0,aoapp,LOGDECR,DISCARD_FLAG, NULL);
        }
        _logger->log(Logger::LOG_LEV_INFO, "log level of [%s] decreased",aoapp);
    }
    IDL_KW_FREE;
    return IDL_GettmpLong(NO_ERROR);
}




// ----------------------- OLD SECTION FOR EXTARNAL-CALL ---------------------------- //
// ---------------------------------------------------------------------------------- //



IDL_INT sendMultiBcuCommand(int priority, int firstBcu, int lastBcu,
							int firstDsp, int lastDsp, 	int opcode, 
							int address, int datalenBytes, unsigned char *data, 	
							int timeout_ms, int flags) {
	
	_logger->log(Logger::LOG_LEV_DEBUG, "Addressing BCU %d-%d", firstBcu, lastBcu);
	
	try {
		// Is able to address:
		// 	- BcuSwitch stand alone
		//	- A subset of contiguous BcuMirror
		if(!((firstBcu == lastBcu && firstBcu == _bcuIdMap["BCU_SWITCH"]) || 
		   (firstBcu >= _bcuIdMap["BCU_MIRROR_0"] && lastBcu <= _bcuIdMap["BCU_MIRROR_5"] && firstBcu <= lastBcu))) {
		    _logger->log(Logger::LOG_LEV_ERROR, "Impossible to address BCU %d-%d", firstBcu, lastBcu);
            return PLAIN_ERROR(VALUE_OUT_OF_RANGE_ERROR);
        }
	}
	catch (BcuIdMapException &e){
		return PLAIN_ERROR(VALUE_OUT_OF_RANGE_ERROR);
	}
							
	int ErrC;
    try {
		if(_comSender == NULL) {
     		_comSender = new CommandSender();
     	}
     	_logger->log(Logger::LOG_LEV_DEBUG, "Calling sendMultiBcuCommand...");
     	_comSender->sendMultiBcuCommand(priority, firstBcu, lastBcu, firstDsp, lastDsp, 
     							   	    opcode, address, datalenBytes, 
     							   		data, timeout_ms, flags);
     	ErrC = NO_ERROR;
     }
     catch(CommandSenderTimeoutException& e) {
		_logger->log(Logger::LOG_LEV_WARNING, "%s", e.what().c_str());
		delete _comSender;	// !!! see BcuCommon.h->Bcu::CommandSender::sendMultiBcuCommand(...) !!!
		_comSender = NULL;	
		ErrC = TIMEOUT_ERROR;
	}
	catch(CommandSenderException& e) {
		_logger->log(Logger::LOG_LEV_WARNING, "%s", e.what().c_str());
		ErrC = COMMUNICATION_ERROR;
	}

	return PLAIN_ERROR(ErrC);
 }
 


// -------------------- MIRROR CTRL COMMUNICATION ------------------ //
// ---------------------------------------------------------------- //


IDL_LONG read_seq_multi(int Argc, void *Argv[])
{

	memset(&flags, 0, sizeof(flags));
    flags = FLAG_WANTREPLY;
     
    IDL_INT firstBCU, lastBCU, firstDSP, lastDSP;
    IDL_LONG dspAddress, dataLength;
    UCHAR *ReceiveBuffer, setPD;
    IDL_LONG time;
    int opcode,i;
     
    ErrC=0;
    // Check if the number of argument is correct
    if (Argc != 9) return IDL_PAR_NUM_ERROR;
     
    // Check if we are connected with the Supervisor
    if (! idl_setup_ok) return MISSING_CLIENTNAME_ERROR;
     
    firstBCU    = *((IDL_INT *)(Argv[0]));
    lastBCU    = *((IDL_INT *)(Argv[1]));
    firstDSP  = *((IDL_INT *)(Argv[2]));
    lastDSP   = *((IDL_INT *)(Argv[3]));
    dspAddress= *((IDL_LONG *)(Argv[4]));
    dataLength= *((IDL_LONG *)(Argv[5]));
    ReceiveBuffer = (UCHAR  *)(Argv[6]);
    time = *((IDL_LONG *)(Argv[7]));
    setPD = *((UCHAR *)(Argv[8]));
     
    // Data compatibility check
    if (setPD >= 6 || setPD == 1) return IDL_PAR_NUM_ERROR;
     
    // verifica il valore di data length
    if (dataLength<=0) return IDL_DATALENGTH_ERROR;
     
    // verifica la congruenza dei dati
    // last DSP > first DSP
    if (lastDSP<firstDSP) return IDL_WRONG_DSP_ERROR;
     
     /* Opcode definitionsetPD = 0 (OL from IDL) -> DSP MEMORY
        setPD = 1 (1L from IDL) -> EMPTY MEMORY
        setPD = 2 (2L from IDL) -> SDRAM MEMORY 
        setPD = 3 (3L from IDL) -> SRAM MEMORY 
        setPD = 4 (4L from IDL) -> FLASH MEMORY 
        setPD = 5 (5L from IDL) -> FPGA MEMORY
	 */
    switch (setPD) 
    {
       	case 0 : opcode = MGP_OP_RDSEQ_DSP;
         	break;
      	case 2 : opcode = MGP_OP_RDSEQ_SDRAM;
         	break;
       	case 3 : opcode = MGP_OP_RDSEQ_SRAM;
         	break;
       	case 4 : opcode = MGP_OP_RDSEQ_FLASH;
         	break;
       	case 5 : opcode = MGP_OP_RDSEQ_DIAGBUFF;
         	break;
    }
     
    _logger->log(Logger::LOG_LEV_TRACE, "OPCODE  : %d", opcode);
    _logger->log(Logger::LOG_LEV_TRACE, "firtsBcu : %d", firstBCU);
    _logger->log(Logger::LOG_LEV_TRACE, "lastBcu : %d", lastBCU);
    _logger->log(Logger::LOG_LEV_TRACE, "firstDSP: %d", firstDSP);
    _logger->log(Logger::LOG_LEV_TRACE, "lastDSP : %d", lastDSP);
    _logger->log(Logger::LOG_LEV_TRACE, "dspAddress : %ld", dspAddress);
    _logger->log(Logger::LOG_LEV_TRACE, "dataLength : %ld", dataLength);
    _logger->log(Logger::LOG_LEV_TRACE, "timeout : %ld", time);
    _logger->log(Logger::LOG_LEV_TRACE, "flags: %d", flags);
    if(_logger->getLevel() >= Logger::LOG_LEV_TRACE) {
        int ii = 0;
        int j = 0;
        for (j=firstDSP;j<=lastDSP;j++) {
            for (i=0;i<dataLength;i++) {
               printf("%08X(%g),", ((int *)(ReceiveBuffer))[ii], ((float *)(ReceiveBuffer))[ii]);
               ii++;
            }
         	printf("\n");
        }
    }
     
    return sendMultiBcuCommand(Priority::LOW_PRIORITY, firstBCU, lastBCU, firstDSP, lastDSP, 
     						   opcode, dspAddress, dataLength*Constants::DWORD_SIZE, 
     						   ReceiveBuffer, time, flags);
	  
     //data e' *uchar   
     //tutti sono int
     //myname *char
}


IDL_LONG write_seq_multi(int Argc, void *Argv[])
{
     
     memset(&flags, 0, sizeof(flags));

     IDL_INT firstBCU, lastBCU, firstDSP, lastDSP;
     IDL_LONG dspAddress, dataLength;
     UCHAR *ReceiveBuffer, setPD;
     IDL_LONG time, flags_tmp;
     int opcode;
     ErrC=0;
     // Check if the number of argument is correct
     if (Argc != 10) return IDL_PAR_NUM_ERROR;
     
     // Check if we are connected with the Supervisor
     if (! idl_setup_ok)
       return MISSING_CLIENTNAME_ERROR;
     
     firstBCU    = *((IDL_INT *)(Argv[0]));
     lastBCU    = *((IDL_INT *)(Argv[1]));
     firstDSP  = *((IDL_INT *)(Argv[2]));
     lastDSP   = *((IDL_INT *)(Argv[3]));
     dspAddress = *((IDL_LONG *)(Argv[4]));
     dataLength= *((IDL_LONG *)(Argv[5]));
     ReceiveBuffer = (UCHAR  *)(Argv[6]) ;
     time = *((IDL_LONG *)(Argv[7]));
     setPD = *((UCHAR *)(Argv[8]));
     flags_tmp = *((IDL_LONG *)(Argv[9]));
     
     // Data compatibility check
     if (setPD >= 7 || setPD == 1) return IDL_PAR_NUM_ERROR;
     
     // verifica il valore di data length
     if (dataLength<=0){
       _logger->log(Logger::LOG_LEV_TRACE, "dataLength : %ld", dataLength);
       return IDL_DATALENGTH_ERROR;
     }
     // verifica la congruenza dei dati
     // last DSP > first DSP
     if (lastDSP<firstDSP)
       return IDL_WRONG_DSP_ERROR;
     
     /* Opcode definition
        setPD = 0 (OL from IDL) -> DSP MEMORY
        setPD = 1 (1L from IDL) -> NOT USED       // Modified by AR
        setPD = 2 (2L from IDL) -> SDRAM MEMORY 
        setPD = 3 (3L from IDL) -> SRAM MEMORY 
        setPD = 4 (4L from IDL) -> FLASH MEMORY 
        setPD = 5 (5L from IDL) -> FPGA MEMORY 
     */
     switch (setPD) 
       {
       case 0 : opcode =  MGP_OP_WRSEQ_DSP;
         break;
       case 1 : // return an error code; Note by AR
         break;
       case 2 : opcode = MGP_OP_WRSEQ_SDRAM;
         break;
       case 3 : opcode = MGP_OP_WRSEQ_SRAM;
         break;
       case 4 : opcode = MGP_OP_WRITE_FLASH;
         break;
       case 5 : opcode = MGP_OP_WRSEQ_DIAGBUFF;
         break;
       case 6 : opcode = MGP_OP_WRITE_SIGGEN_RAM;
         break;
       }
     
     _logger->log(Logger::LOG_LEV_TRACE, "OPCODE  : %d", opcode);
     _logger->log(Logger::LOG_LEV_TRACE, "firstBcu : %d", firstBCU);
     _logger->log(Logger::LOG_LEV_TRACE, "lastBcu : %d", lastBCU);
     _logger->log(Logger::LOG_LEV_TRACE, "firstDSP: %d", firstDSP);
     _logger->log(Logger::LOG_LEV_TRACE, "lastDSP : %d", lastDSP);
     _logger->log(Logger::LOG_LEV_TRACE, "dspAddress : %ld", dspAddress);
     _logger->log(Logger::LOG_LEV_TRACE, "dataLength : %ld", dataLength);
     _logger->log(Logger::LOG_LEV_TRACE, "flags : %ld", flags_tmp);
     _logger->log(Logger::LOG_LEV_TRACE, "timeout : %ld", time);
	 if(_logger->getLevel() >= Logger::LOG_LEV_TRACE) {
         int ii = 0;
         int i,j;
         for (j=firstDSP;j<=lastDSP;j++) {
            for (i=0;i<dataLength;i++) {
               printf("%08X(%g),", ((int *)(ReceiveBuffer))[ii], ((float *)(ReceiveBuffer))[ii]);
               ii++;
            }
         printf("\n");
         }
	 }
     
     return sendMultiBcuCommand(Priority::LOW_PRIORITY, firstBCU, lastBCU, firstDSP, lastDSP, 
     							opcode, dspAddress, dataLength*Constants::DWORD_SIZE, 
     							ReceiveBuffer, time, flags_tmp);
     
     //data e' *uchar   
     //tutti sono int
     //myname *char
}



IDL_LONG write_same_multi(int Argc, void *Argv[])
{  
	memset(&flags, 0, sizeof(flags));
	    
	IDL_INT firstBCU, lastBCU, firstDSP, lastDSP;
	IDL_LONG dspAddress, dataLength;
	UCHAR *ReceiveBuffer, setPD;
	IDL_LONG time, flags_tmp;
  	int opcode,i;
  	ErrC=0;
  	// Check if the number of argument is correct
  	if (Argc != 10) return IDL_PAR_NUM_ERROR;

  	// Check if we are connected with the Supervisor
  	if (!idl_setup_ok) return MISSING_CLIENTNAME_ERROR;
	
	firstBCU = *((IDL_INT *)(Argv[0]));
	lastBCU = *((IDL_INT *)(Argv[1]));
	firstDSP  = *((IDL_INT *)(Argv[2]));
	lastDSP   = *((IDL_INT *)(Argv[3]));
	dspAddress = *((IDL_LONG *)(Argv[4]));
	dataLength= *((IDL_LONG *)(Argv[5]));
	ReceiveBuffer = (UCHAR  *)(Argv[6]) ;
	time = *((IDL_LONG *)(Argv[7]));
	setPD = *((UCHAR *)(Argv[8]));
	flags_tmp = *((IDL_LONG *)(Argv[9]));

   	// Data compatibility check
   	if (setPD >= 7 || setPD == 1 || setPD == 4) return IDL_PAR_NUM_ERROR;

  	 // verifica il valore di data length
     if (dataLength<=0) return IDL_DATALENGTH_ERROR;

   	// verifica la congruenza dei dati
   	// last DSP > first DSP
    if (lastDSP<firstDSP) return IDL_WRONG_DSP_ERROR;

   /* Opcode definition
         setPD = 0 (OL from IDL) -> DSP MEMORY
         setPD = 1 (1L from IDL) -> EMPTY MEMORY
         setPD = 2 (2L from IDL) -> SDRAM MEMORY 
         setPD = 3 (3L from IDL) -> SRAM MEMORY 
         setPD = 4 (4L from IDL) -> EMPTY MEMORY 
         setPD = 5 (5L from IDL) -> FPGA MEMORY
   */
      switch (setPD) 
      {
      case 0 : opcode = MGP_OP_WRSAME_DSP;
               break;
      case 1 : //error 
               break;
      case 2 : opcode = MGP_OP_WRSAME_SDRAM;
               break;
      case 3 : opcode = MGP_OP_WRSAME_SRAM;
               break;
      case 4 : //error 
               break;
      case 5 : opcode = MGP_OP_WRSAME_DIAGBUFF;
               break;
      case 6 : opcode = MGP_OP_WRITE_SIGGEN_RAM;
               break;
      }

   	_logger->log(Logger::LOG_LEV_TRACE, "OPCODE  : %d", opcode);
    _logger->log(Logger::LOG_LEV_TRACE, "firstBCU : %d", firstBCU);
    _logger->log(Logger::LOG_LEV_TRACE, "lastBCU : %d", lastBCU);
    _logger->log(Logger::LOG_LEV_TRACE, "firstDSP: %d", firstDSP);
    _logger->log(Logger::LOG_LEV_TRACE, "lastDSP : %d", lastDSP);
    _logger->log(Logger::LOG_LEV_TRACE, "dspAddress : %ld", dspAddress);
    _logger->log(Logger::LOG_LEV_TRACE, "dataLength : %ld", dataLength);
    _logger->log(Logger::LOG_LEV_TRACE, "timeout : %ld", time);
    if(_logger->getLevel() >= Logger::LOG_LEV_TRACE) {
      	for (i=0;i<dataLength*4;i++)
         	printf("%08X", ((int *)(ReceiveBuffer))[i]);
      	printf("\n");
   	}  

     return sendMultiBcuCommand(Priority::LOW_PRIORITY, firstBCU, lastBCU, firstDSP, lastDSP, 
     							opcode, dspAddress, dataLength*Constants::DWORD_SIZE, 
     							ReceiveBuffer, time, flags_tmp);
   //data e' *uchar  
   //tutti sono int
   //myname *char
}


IDL_LONG read_seq(int Argc, void *Argv[])
{

	memset(&flags, 0, sizeof(flags));
    flags = FLAG_WANTREPLY;
     
    IDL_INT BCU_num, firstDSP, lastDSP;
    IDL_LONG dspAddress, dataLength;
    UCHAR *ReceiveBuffer, setPD;
    IDL_LONG time;
    int opcode,i;
     
    ErrC=0;
    // Check if the number of argument is correct
    if (Argc != 8) return IDL_PAR_NUM_ERROR;
     
    // Check if we are connected with the Supervisor
    if (! idl_setup_ok) return MISSING_CLIENTNAME_ERROR;
     
    BCU_num    = *((IDL_INT *)(Argv[0]));
    firstDSP  = *((IDL_INT *)(Argv[1]));
    lastDSP   = *((IDL_INT *)(Argv[2]));
    dspAddress= *((IDL_LONG *)(Argv[3]));
    dataLength= *((IDL_LONG *)(Argv[4]));
    ReceiveBuffer = (UCHAR  *)(Argv[5]);
    time = *((IDL_LONG *)(Argv[6]));
    setPD = *((UCHAR *)(Argv[7]));
     
    // Data compatibility check
    if (setPD >= 6 || setPD == 1) return IDL_PAR_NUM_ERROR;
     
    // verifica il valore di data length
    if (dataLength<=0) return IDL_DATALENGTH_ERROR;
     
    // verifica la congruenza dei dati
    // last DSP > first DSP
    if (lastDSP<firstDSP) return IDL_WRONG_DSP_ERROR;
     
     /* Opcode definitionsetPD = 0 (OL from IDL) -> DSP MEMORY
        setPD = 1 (1L from IDL) -> EMPTY MEMORY
        setPD = 2 (2L from IDL) -> SDRAM MEMORY 
        setPD = 3 (3L from IDL) -> SRAM MEMORY 
        setPD = 4 (4L from IDL) -> FLASH MEMORY 
        setPD = 5 (5L from IDL) -> FPGA MEMORY
	 */
    switch (setPD) 
    {
       	case 0 : opcode = MGP_OP_RDSEQ_DSP;
         	break;
      	case 2 : opcode = MGP_OP_RDSEQ_SDRAM;
         	break;
       	case 3 : opcode = MGP_OP_RDSEQ_SRAM;
         	break;
       	case 4 : opcode = MGP_OP_RDSEQ_FLASH;
         	break;
       	case 5 : opcode = MGP_OP_RDSEQ_DIAGBUFF;
         	break;
    }
     
    _logger->log(Logger::LOG_LEV_TRACE, "OPCODE  : %d", opcode);
    _logger->log(Logger::LOG_LEV_TRACE, "BCU_num : %d", BCU_num);
    _logger->log(Logger::LOG_LEV_TRACE, "firstDSP: %d", firstDSP);
    _logger->log(Logger::LOG_LEV_TRACE, "lastDSP : %d", lastDSP);
    _logger->log(Logger::LOG_LEV_TRACE, "dspAddress : %ld", dspAddress);
    _logger->log(Logger::LOG_LEV_TRACE, "dataLength : %ld", dataLength);
    _logger->log(Logger::LOG_LEV_TRACE, "timeout : %ld", time);
    _logger->log(Logger::LOG_LEV_TRACE, "flags: %d", flags);
    if(_logger->getLevel() >= Logger::LOG_LEV_TRACE) {
        int ii = 0;
        int j = 0;
        for (j=firstDSP;j<=lastDSP;j++) {
            for (i=0;i<dataLength;i++) {
               printf("%08X(%g),", ((int *)(ReceiveBuffer))[ii], ((float *)(ReceiveBuffer))[ii]);
               ii++;
            }
         	printf("\n");
        }
    }
     
    return sendMultiBcuCommand(Priority::LOW_PRIORITY, BCU_num, BCU_num, firstDSP, lastDSP, 
     						   opcode, dspAddress, dataLength*Constants::DWORD_SIZE, 
     						   ReceiveBuffer, time, flags);
	  
     //data e' *uchar   
     //tutti sono int
     //myname *char
}


IDL_LONG write_seq(int Argc, void *Argv[])
{
     
     memset(&flags, 0, sizeof(flags));

     IDL_INT BCU_num, firstDSP, lastDSP;
     IDL_LONG dspAddress, dataLength;
     UCHAR *ReceiveBuffer, setPD;
     IDL_LONG time, flags_tmp;
     int opcode;
     ErrC=0;
     // Check if the number of argument is correct
     if (Argc != 9) return IDL_PAR_NUM_ERROR;
     
     // Check if we are connected with the Supervisor
     if (! idl_setup_ok)
       return MISSING_CLIENTNAME_ERROR;
     
     BCU_num    = *((IDL_INT *)(Argv[0]));
     firstDSP  = *((IDL_INT *)(Argv[1]));
     lastDSP   = *((IDL_INT *)(Argv[2]));
     dspAddress = *((IDL_LONG *)(Argv[3]));
     dataLength= *((IDL_LONG *)(Argv[4]));
     ReceiveBuffer = (UCHAR  *)(Argv[5]) ;
     time = *((IDL_LONG *)(Argv[6]));
     setPD = *((UCHAR *)(Argv[7]));
     flags_tmp = *((IDL_LONG *)(Argv[8]));
     
     // Data compatibility check
     if (setPD >= 7 || setPD == 1) return IDL_PAR_NUM_ERROR;
     
     // verifica il valore di data length
     if (dataLength<=0){
       _logger->log(Logger::LOG_LEV_TRACE, "dataLength : %ld", dataLength);
       return IDL_DATALENGTH_ERROR;
     }
     // verifica la congruenza dei dati
     // last DSP > first DSP
     if (lastDSP<firstDSP)
       return IDL_WRONG_DSP_ERROR;
     
     /* Opcode definition
        setPD = 0 (OL from IDL) -> DSP MEMORY
        setPD = 1 (1L from IDL) -> NOT USED       // Modified by AR
        setPD = 2 (2L from IDL) -> SDRAM MEMORY 
        setPD = 3 (3L from IDL) -> SRAM MEMORY 
        setPD = 4 (4L from IDL) -> FLASH MEMORY 
        setPD = 5 (5L from IDL) -> FPGA MEMORY 
     */
     switch (setPD) 
       {
       case 0 : opcode =  MGP_OP_WRSEQ_DSP;
         break;
       case 1 : // return an error code; Note by AR
         break;
       case 2 : opcode = MGP_OP_WRSEQ_SDRAM;
         break;
       case 3 : opcode = MGP_OP_WRSEQ_SRAM;
         break;
       case 4 : opcode = MGP_OP_WRITE_FLASH;
         break;
       case 5 : opcode = MGP_OP_WRSEQ_DIAGBUFF;
         break;
       case 6 : opcode = MGP_OP_WRITE_SIGGEN_RAM;
         break;
       }
     
     _logger->log(Logger::LOG_LEV_TRACE, "OPCODE  : %d", opcode);
     _logger->log(Logger::LOG_LEV_TRACE, "BCU_num : %d", BCU_num);
     _logger->log(Logger::LOG_LEV_TRACE, "firstDSP: %d", firstDSP);
     _logger->log(Logger::LOG_LEV_TRACE, "lastDSP : %d", lastDSP);
     _logger->log(Logger::LOG_LEV_TRACE, "dspAddress : %ld", dspAddress);
     _logger->log(Logger::LOG_LEV_TRACE, "dataLength : %ld", dataLength);
     _logger->log(Logger::LOG_LEV_TRACE, "flags : %ld", flags_tmp);
     _logger->log(Logger::LOG_LEV_TRACE, "timeout : %ld", time);
	 if(_logger->getLevel() >= Logger::LOG_LEV_TRACE) {
         int ii = 0;
         int i,j;
         for (j=firstDSP;j<=lastDSP;j++) {
            for (i=0;i<dataLength;i++) {
               printf("%08X(%g),", ((int *)(ReceiveBuffer))[ii], ((float *)(ReceiveBuffer))[ii]);
               ii++;
            }
         printf("\n");
         }
	 }
     
     return sendMultiBcuCommand(Priority::LOW_PRIORITY, BCU_num, BCU_num, firstDSP, lastDSP, 
     							opcode, dspAddress, dataLength*Constants::DWORD_SIZE, 
     							ReceiveBuffer, time, flags_tmp);
     
     //data e' *uchar   
     //tutti sono int
     //myname *char
}


IDL_LONG write_same(int Argc, void *Argv[])
{  
	memset(&flags, 0, sizeof(flags));
	    
	IDL_INT BCU_num, firstDSP, lastDSP;
	IDL_LONG dspAddress, dataLength;
	UCHAR *ReceiveBuffer, setPD;
	IDL_LONG time, flags_tmp;
  	int opcode,i;
  	ErrC=0;
  	// Check if the number of argument is correct
  	if (Argc != 9) return IDL_PAR_NUM_ERROR;

  	// Check if we are connected with the Supervisor
  	if (!idl_setup_ok) return MISSING_CLIENTNAME_ERROR;
	
	BCU_num   = *((IDL_INT *)(Argv[0]));
	firstDSP  = *((IDL_INT *)(Argv[1]));
	lastDSP   = *((IDL_INT *)(Argv[2]));
	dspAddress = *((IDL_LONG *)(Argv[3]));
	dataLength= *((IDL_LONG *)(Argv[4]));
	ReceiveBuffer = (UCHAR  *)(Argv[5]) ;
	time = *((IDL_LONG *)(Argv[6]));
	setPD = *((UCHAR *)(Argv[7]));
	flags_tmp = *((IDL_LONG *)(Argv[8]));

   	// Data compatibility check
   	if (setPD >= 7 || setPD == 1 || setPD == 4) return IDL_PAR_NUM_ERROR;

  	 // verifica il valore di data length
     if (dataLength<=0) return IDL_DATALENGTH_ERROR;

   	// verifica la congruenza dei dati
   	// last DSP > first DSP
    if (lastDSP<firstDSP) return IDL_WRONG_DSP_ERROR;

   /* Opcode definition
         setPD = 0 (OL from IDL) -> DSP MEMORY
         setPD = 1 (1L from IDL) -> EMPTY MEMORY
         setPD = 2 (2L from IDL) -> SDRAM MEMORY 
         setPD = 3 (3L from IDL) -> SRAM MEMORY 
         setPD = 4 (4L from IDL) -> EMPTY MEMORY 
         setPD = 5 (5L from IDL) -> FPGA MEMORY
   */
      switch (setPD) 
      {
      case 0 : opcode = MGP_OP_WRSAME_DSP;
               break;
      case 1 : //error 
               break;
      case 2 : opcode = MGP_OP_WRSAME_SDRAM;
               break;
      case 3 : opcode = MGP_OP_WRSAME_SRAM;
               break;
      case 4 : //error 
               break;
      case 5 : opcode = MGP_OP_WRSAME_DIAGBUFF;
               break;
      case 6 : opcode = MGP_OP_WRITE_SIGGEN_RAM;
               break;
      }

   	_logger->log(Logger::LOG_LEV_TRACE, "OPCODE  : %d", opcode);
    _logger->log(Logger::LOG_LEV_TRACE, "BCU_num : %d", BCU_num);
    _logger->log(Logger::LOG_LEV_TRACE, "firstDSP: %d", firstDSP);
    _logger->log(Logger::LOG_LEV_TRACE, "lastDSP : %d", lastDSP);
    _logger->log(Logger::LOG_LEV_TRACE, "dspAddress : %ld", dspAddress);
    _logger->log(Logger::LOG_LEV_TRACE, "dataLength : %ld", dataLength);
    _logger->log(Logger::LOG_LEV_TRACE, "timeout : %ld", time);
    if(_logger->getLevel() >= Logger::LOG_LEV_TRACE) {
      	for (i=0;i<dataLength*4;i++)
         	printf("%08X", ((int *)(ReceiveBuffer))[i]);
      	printf("\n");
   	}  

     return sendMultiBcuCommand(Priority::LOW_PRIORITY, BCU_num, BCU_num, firstDSP, lastDSP, 
     							opcode, dspAddress, dataLength*Constants::DWORD_SIZE, 
     							ReceiveBuffer, time, flags_tmp);
   //data e' *uchar  
   //tutti sono int
   //myname *char
}



/*
 * [Fixed for new MirrorCtrl]
 */
IDL_LONG clear_mem(int Argc, void *Argv[])
{
      
     memset(&flags, 0, sizeof(flags));
     flags = FLAG_WANTREPLY;
     IDL_INT BCU_num, firstDSP, lastDSP;
     IDL_LONG dspAddress, dataLength;

     UCHAR *ReceiveBuffer, setPD;
     IDL_LONG time;
     int opcode;
     ErrC=0;
   // Check if the number of argument is correct
      if (Argc != 7) return IDL_PAR_NUM_ERROR;

   // Check if we are connected with the Supervisor
      if (! idl_setup_ok)
         return MISSING_CLIENTNAME_ERROR;

      BCU_num   = *((IDL_INT *)(Argv[0]));
      firstDSP  = *((IDL_INT *)(Argv[1]));
      lastDSP   = *((IDL_INT *)(Argv[2]));
      dspAddress = *((IDL_LONG *)(Argv[3]));
      dataLength= *((IDL_LONG *)(Argv[4]));
      ReceiveBuffer = (UCHAR  *)(Argv[5]) ;
      setPD = *((UCHAR *)(Argv[6]));
      time = timeout;
      
   // Data compatibility check
      if (setPD >= 5 || setPD <= 0 || setPD == 1) return IDL_DATATYPE_ERROR;

   // verifica il valore di data length
      if (dataLength<=0)
         return IDL_DATALENGTH_ERROR;

   // verifica la congruenza dei dati
   // last DSP > first DSP
      if (lastDSP<firstDSP)
         return IDL_WRONG_DSP_ERROR;

   /* Opcode definition
         setPD = 0 (OL from IDL) -> not used 
         setPD = 1 (OL from IDL) -> EMPTY MEMORY 
         setPD = 2 (1L from IDL) -> SDRAM MEMORY 
         setPD = 3 (1L from IDL) -> SRAM MEMORY 
         setPD = 4 (1L from IDL) -> FLASH MEMORY 
   */
   	switch (setPD) 
    {
      	case 2 : opcode = 140;
            break;
   		case 3 : opcode = 145;
            break;
   		case 4 : opcode = 130;
            break;
   	}

	_logger->log(Logger::LOG_LEV_TRACE, "OPCODE  : %d", opcode);
	_logger->log(Logger::LOG_LEV_TRACE, "BCU_num : %d", BCU_num);
	_logger->log(Logger::LOG_LEV_TRACE, "firstDSP: %d", firstDSP);
	_logger->log(Logger::LOG_LEV_TRACE, "lastDSP : %d", lastDSP);
	_logger->log(Logger::LOG_LEV_TRACE, "dspAddress to clear: %ld", dspAddress);
	_logger->log(Logger::LOG_LEV_TRACE, "dataLength : %ld", dataLength);
	_logger->log(Logger::LOG_LEV_TRACE, " and length : %d", *((int *)ReceiveBuffer));
	_logger->log(Logger::LOG_LEV_TRACE, "timeout : %ld", time);  

   	return sendMultiBcuCommand(Priority::LOW_PRIORITY, BCU_num, BCU_num, firstDSP, lastDSP, 
     						   opcode, dspAddress, dataLength*Constants::DWORD_SIZE, 
     						   ReceiveBuffer, time, flags);
	//data e' *uchar  
	//tutti sono int
	//myname *char
}


/*
 * [Fixed for new MirrorCtrl]
 */
IDL_LONG lock_flash(int Argc, void *Argv[])
{
   
  	memset(&flags, 0, sizeof(flags));
  	flags = FLAG_WANTREPLY;
   	IDL_INT BCU_num, firstDSP, lastDSP;
   	IDL_LONG dspAddress, dataLength;    
   	UCHAR *ReceiveBuffer;
  	IDL_LONG time;
   	int opcode;
   	ErrC=0;
	// Check if the number of argument is correct
   	if (Argc != 5) return IDL_PAR_NUM_ERROR;

	// Check if we are connected with the Supervisor
   	if (! idl_setup_ok) return MISSING_CLIENTNAME_ERROR;

   	BCU_num   = *((IDL_INT *)(Argv[0]));
   	firstDSP  = *((IDL_INT *)(Argv[1]));
   	lastDSP   = *((IDL_INT *)(Argv[2]));
   	dspAddress = *((IDL_LONG *)(Argv[3]));
   	dataLength= *((IDL_LONG *)(Argv[4]));
   	ReceiveBuffer = (UCHAR  *)(Argv[5]) ;
   	time=timeout;
	// verifica il valore di data length
   	if (dataLength<=0) return IDL_DATALENGTH_ERROR;

	// verifica la congruenza dei dati
	// last DSP > first DSP
   	if (lastDSP<firstDSP)
      	return IDL_WRONG_DSP_ERROR;

   	opcode = 128;

	_logger->log(Logger::LOG_LEV_TRACE, "OPCODE : %d", opcode);
	_logger->log(Logger::LOG_LEV_TRACE, "Locking FLASH Memory in BCU number %d", BCU_num);
	_logger->log(Logger::LOG_LEV_TRACE, " from DSP: %d", firstDSP);
	_logger->log(Logger::LOG_LEV_TRACE, " to DSP %d", lastDSP);
	_logger->log(Logger::LOG_LEV_TRACE, " from address %ld", dspAddress);
	_logger->log(Logger::LOG_LEV_TRACE, " and length : %ld", *((IDL_LONG *)(Argv[5])));
	_logger->log(Logger::LOG_LEV_TRACE, "timeout : %ld", time);

   	return sendMultiBcuCommand(Priority::LOW_PRIORITY, BCU_num, BCU_num, firstDSP, lastDSP, 
     						   opcode, dspAddress, dataLength*Constants::DWORD_SIZE, 
     						   ReceiveBuffer, time, flags);
	//data e' *uchar  
	//tutti sono int
	//myname *char
}


/*
 * [Fixed for new MirrorCtrl]
 */
IDL_LONG unlock_flash(int Argc, void *Argv[])
{ 
  	memset(&flags, 0, sizeof(flags));
  	flags = FLAG_WANTREPLY;
   	IDL_INT BCU_num, firstDSP, lastDSP;
   	IDL_LONG dspAddress, dataLength;
   	UCHAR *ReceiveBuffer;
  	IDL_LONG time;
   	int opcode;
  	ErrC=0;
	// Check if the number of argument is correct
  	if (Argc != 5) return IDL_PAR_NUM_ERROR;

	// Check if we are connected with the Supervisor
   	if (! idl_setup_ok) return MISSING_CLIENTNAME_ERROR;

   	BCU_num   = *((IDL_INT *)(Argv[0]));
   	firstDSP  = *((IDL_INT *)(Argv[1]));
   	lastDSP   = *((IDL_INT *)(Argv[2]));
   	dspAddress = *((IDL_LONG *)(Argv[3]));
   	dataLength= *((IDL_LONG *)(Argv[4]));
   	ReceiveBuffer = (UCHAR  *)(Argv[5]) ;
   	time=timeout;

	// verifica il valore di data length
   	if (dataLength<=0) return IDL_DATALENGTH_ERROR;

	// verifica la congruenza dei dati
	// last DSP > first DSP
   	if (lastDSP<firstDSP) return IDL_WRONG_DSP_ERROR;

   	opcode = 129;

	_logger->log(Logger::LOG_LEV_TRACE, "OPCODE : %d", opcode);
	_logger->log(Logger::LOG_LEV_TRACE, "Unlocking FLASH Memory in BCU number %d", BCU_num);
	_logger->log(Logger::LOG_LEV_TRACE, " from DSP: %d", firstDSP);
	_logger->log(Logger::LOG_LEV_TRACE, " to DSP %d", lastDSP);
	_logger->log(Logger::LOG_LEV_TRACE, " from address %ld", dspAddress);
	_logger->log(Logger::LOG_LEV_TRACE, " and length : %ld", *((IDL_LONG *)(Argv[5])));
	_logger->log(Logger::LOG_LEV_TRACE, "timeout : %ld", time);

   return sendMultiBcuCommand(Priority::LOW_PRIORITY, BCU_num, BCU_num, firstDSP, lastDSP, 
     						   opcode, dspAddress, dataLength*Constants::DWORD_SIZE, 
     						   ReceiveBuffer, time, flags);
	//data e' *uchar  
	//tutti sono int
	//myname *char
}


/*
 * [Fixed for new MirrorCtrl]
 */
IDL_LONG reset_devices(int Argc, void *Argv[])
{
	_logger->printStatus();
	_logger->log(Logger::LOG_LEV_INFO, ">>> IDL resetting devices...");
   
  	memset(&flags, 0, sizeof(flags));
  	flags = FLAG_WANTREPLY;
   	IDL_INT BCU_num, firstDSP, lastDSP;
   	IDL_LONG dspAddress, dataLength;
   	UCHAR *ReceiveBuffer;  //, setPD;
  	IDL_LONG time;
   	int opcode;
   	ErrC=0;
	// Check if the number of argument is correct
   	if (Argc != 6) return IDL_PAR_NUM_ERROR;

	// Check if we are connected with the Supervisor
   	if (! idl_setup_ok) return MISSING_CLIENTNAME_ERROR;

   	BCU_num   = *((IDL_INT *)(Argv[0]));
   	firstDSP  = *((IDL_INT *)(Argv[1]));
   	lastDSP   = *((IDL_INT *)(Argv[2]));
   	dspAddress = *((IDL_LONG *)(Argv[3]));
   	dataLength= *((IDL_LONG *)(Argv[4]));
   	ReceiveBuffer = (UCHAR  *)(Argv[5]) ;
   	time=timeout;
	// verifica la congruenza dei dati
	// last DSP > first DSP
   	if (lastDSP < firstDSP ) return IDL_WRONG_DSP_ERROR;

    opcode = OpCodes::MGP_OP_RESET_DEVICES;

	_logger->log(Logger::LOG_LEV_TRACE, "\tOPCODE : %d", opcode);
	_logger->log(Logger::LOG_LEV_TRACE, "\tReset devices:");
	_logger->log(Logger::LOG_LEV_TRACE, "\tBCU num: %d", BCU_num);
	_logger->log(Logger::LOG_LEV_TRACE, "\tFirst DSP: %d", firstDSP);
	_logger->log(Logger::LOG_LEV_TRACE, "\tLast DSP %d", lastDSP);
    _logger->log(Logger::LOG_LEV_TRACE, "\tTimeout : %ld", time); 

   return sendMultiBcuCommand(Priority::LOW_PRIORITY, BCU_num, BCU_num, firstDSP, lastDSP, 
     						   opcode, dspAddress, dataLength*Constants::DWORD_SIZE, 
     						   ReceiveBuffer, time, flags);

	//data e' *uchar  
	//tutti sono int
	//myname *char
	// ErrC = NO_ERROR;
}


IDL_LONG reset_devices_multi(int Argc, void *Argv[])
{
	_logger->printStatus();
	_logger->log(Logger::LOG_LEV_INFO, ">>> IDL resetting MULTI devices...");
   
  	memset(&flags, 0, sizeof(flags));
  	flags = FLAG_WANTREPLY;
   	IDL_INT firstBcu, lastBcu, firstDSP, lastDSP;
   	IDL_LONG dspAddress, dataLength;
   	UCHAR *ReceiveBuffer;  //, setPD;
  	IDL_LONG time;
   	int opcode;
   	ErrC=0;
	// Check if the number of argument is correct
   	if (Argc != 7) return IDL_PAR_NUM_ERROR;

	// Check if we are connected with the Supervisor
   	if (! idl_setup_ok) return MISSING_CLIENTNAME_ERROR;

   	firstBcu   = *((IDL_INT *)(Argv[0]));
   	lastBcu   = *((IDL_INT *)(Argv[1]));
   	firstDSP  = *((IDL_INT *)(Argv[2]));
   	lastDSP   = *((IDL_INT *)(Argv[3]));
   	dspAddress = *((IDL_LONG *)(Argv[4]));
   	dataLength= *((IDL_LONG *)(Argv[5]));
   	ReceiveBuffer = (UCHAR  *)(Argv[6]) ;
   	time=timeout;
	// verifica la congruenza dei dati
	// last DSP > first DSP
   	if (lastDSP < firstDSP ) return IDL_WRONG_DSP_ERROR;

    opcode = OpCodes::MGP_OP_RESET_DEVICES;

	_logger->log(Logger::LOG_LEV_TRACE, "\tOPCODE : %d", opcode);
	_logger->log(Logger::LOG_LEV_TRACE, "\tReset devices:");
	_logger->log(Logger::LOG_LEV_TRACE, "\tFirst Bcu: %d", firstBcu);
	_logger->log(Logger::LOG_LEV_TRACE, "\tLast Bcu: %d", lastBcu);
	_logger->log(Logger::LOG_LEV_TRACE, "\tFirst DSP: %d", firstDSP);
	_logger->log(Logger::LOG_LEV_TRACE, "\tLast DSP %d", lastDSP);
    _logger->log(Logger::LOG_LEV_TRACE, "\tTimeout : %ld", time); 

   return sendMultiBcuCommand(Priority::LOW_PRIORITY, firstBcu, lastBcu, firstDSP, lastDSP, 
     						   opcode, dspAddress, dataLength*Constants::DWORD_SIZE, 
     						   ReceiveBuffer, time, flags);

	//data e' *uchar  
	//tutti sono int
	//myname *char
	// ErrC = NO_ERROR;
}

IDL_LONG reset_devices_multi_wait(int Argc, void *Argv[])
{
	_logger->printStatus();
	_logger->log(Logger::LOG_LEV_INFO, ">>> IDL resetting MULTI devices...");
   
  	memset(&flags, 0, sizeof(flags));
   	IDL_INT firstBcu, lastBcu, firstDSP, lastDSP;
   	IDL_LONG dspAddress, dataLength;
   	UCHAR *ReceiveBuffer;  //, setPD;
   	int opcode;
   	ErrC=0;
	// Check if the number of argument is correct
   	if (Argc != 7) return IDL_PAR_NUM_ERROR;

	// Check if we are connected with the Supervisor
   	if (! idl_setup_ok) return MISSING_CLIENTNAME_ERROR;

   	firstBcu   = *((IDL_INT *)(Argv[0]));
   	lastBcu   = *((IDL_INT *)(Argv[1]));
   	firstDSP  = *((IDL_INT *)(Argv[2]));
   	lastDSP   = *((IDL_INT *)(Argv[3]));
   	dspAddress = *((IDL_LONG *)(Argv[4]));
   	dataLength= *((IDL_LONG *)(Argv[5]));
   	ReceiveBuffer = (UCHAR  *)(Argv[6]);
	// verifica la congruenza dei dati
	// last DSP > first DSP
   	if (lastDSP < firstDSP ) return IDL_WRONG_DSP_ERROR;

    opcode = OpCodes::MGP_OP_RESET_DEVICES;

	_logger->log(Logger::LOG_LEV_TRACE, "\tOPCODE : %d", opcode);
	_logger->log(Logger::LOG_LEV_TRACE, "\tReset devices:");
	_logger->log(Logger::LOG_LEV_TRACE, "\tFirst Bcu: %d", firstBcu);
	_logger->log(Logger::LOG_LEV_TRACE, "\tLast Bcu: %d", lastBcu);
	_logger->log(Logger::LOG_LEV_TRACE, "\tFirst DSP: %d", firstDSP);
	_logger->log(Logger::LOG_LEV_TRACE, "\tLast DSP %d", lastDSP);
    _logger->log(Logger::LOG_LEV_TRACE, "\tTimeout : %ld", time); 

	// --- 1) Send the RESET message, without any timeout ---
	int ret;
	ret = sendMultiBcuCommand(Priority::LOW_PRIORITY, firstBcu, lastBcu, firstDSP, lastDSP, 
	     					  opcode, dspAddress, 
	     					  dataLength*Constants::DWORD_SIZE,  // ??? Isn't zero ?
     						  ReceiveBuffer, 					 // ??? Isn't NULL ?
     						  0,  								 // Timeout: doesn't wait the reply !!!
     						  0); 								 // Flags: doesn't want the reply !!!						 
	// Note that it can't be a Timeout
	if(IS_ERROR(ret)) {
		_logger->log(Logger::LOG_LEV_WARNING, "Reset request failed");
		return ret;
	}
	
	// --- 2) Poll the BCU to check the reset completion ---
	ret = NO_ERROR;
	int wordsToRead = 10;
	BYTE readBuf[wordsToRead * Constants::DWORD_SIZE * (lastBcu - firstBcu + 1)];
	_logger->log(Logger::LOG_LEV_INFO, ">>> IDL waiting for MULTI devices reset...");
	while(true) {
		ret = sendMultiBcuCommand(Priority::LOW_PRIORITY, 
								  firstBcu, lastBcu, 
								  255, 255, 
 						    	  MGP_OP_RDSEQ_SDRAM, 
 						    	  0, 			// Dsp address
 						    	  wordsToRead,  // Datalen
 						    	  readBuf,		// Receive buffer
 						    	  0, 
 						    	  Constants::WANTREPLY_FLAG);
 						    	  
		if(ret == TIMEOUT_ERROR) {
			_logger->log(Logger::LOG_LEV_INFO, "...waiting for reset completed...");
			sleep(1);
		}
		else if (IS_ERROR(ret)){
			_logger->log(Logger::LOG_LEV_WARNING, "Reset request failed");
			return ret;
		}
		else {
			_logger->log(Logger::LOG_LEV_INFO, "Reset completed !");
			return ret;
		}
	}

	//data e' *uchar  
	//tutti sono int
	//myname *char
	// ErrC = NO_ERROR;
}



/*
 * [Fixed for new MirrorCtrl]
 */
IDL_LONG powerup(int Argc, void *Argv[])
{
   
  	memset(&flags, 0, sizeof(flags));
  	flags = FLAG_WANTREPLY;
   	IDL_INT BCU_num, firstDSP, lastDSP;
   	IDL_LONG dspAddress, dataLength;
   	UCHAR *ReceiveBuffer; //, setPD;
  	IDL_LONG time;
   	int opcode;
   	ErrC=0;
	// Check if the number of argument is correct
   	if (Argc != 7) return IDL_PAR_NUM_ERROR;

	// Check if we are connected with the Supervisor
   	if (! idl_setup_ok) return MISSING_CLIENTNAME_ERROR;

   	BCU_num   = *((IDL_INT *)(Argv[0]));
   	firstDSP  = *((IDL_INT *)(Argv[1]));
   	lastDSP   = *((IDL_INT *)(Argv[2]));
   	dspAddress = *((IDL_LONG *)(Argv[3]));
   	dataLength= *((IDL_LONG *)(Argv[4]));
   	ReceiveBuffer = (UCHAR  *)(Argv[5]) ;
   	time = *((IDL_LONG *)(Argv[6]));
	// verifica la congruenza dei dati
	// last DSP > first DSP
   	if (lastDSP < firstDSP ) return IDL_WRONG_DSP_ERROR;

   	opcode = 11;

	_logger->log(Logger::LOG_LEV_TRACE, "OPCODE : %d", opcode);
	_logger->log(Logger::LOG_LEV_TRACE, "Powerup:");
	_logger->log(Logger::LOG_LEV_TRACE, "BCU num: %d", BCU_num);
	_logger->log(Logger::LOG_LEV_TRACE, "timeout : %ld", time);

   return sendMultiBcuCommand(Priority::LOW_PRIORITY, BCU_num, BCU_num, firstDSP, lastDSP, 
     						   opcode, dspAddress, dataLength*Constants::DWORD_SIZE, 
     						   ReceiveBuffer, time, flags);

	//data e' *uchar  
	//tutti sono int
	//myname *char
	// ErrC = NO_ERROR;
}


// -------------------- RTDB VAR MANAGEMENT ------------------ //
// ----------------------------------------------------------- //


//@Function: createvar to create variable into the MsgRTDB
IDL_LONG createvar(int Argc, void *Argv[])
{
    char *VarName, *VarMode;
    int VarType, NElem;
    //int ErrC;

    // Check if the number of argument is correct
    if (Argc != 4) return IDL_PAR_NUM_ERROR;

    VarName = ((IDL_STRING *)Argv[0])->s;
    if (strlen(VarName) >= VAR_NAME_LEN) return IDL_DATATYPE_ERROR;

    VarMode = ((IDL_STRING *)Argv[1])->s;
    if (strlen(VarMode) > 1) return IDL_DATATYPE_ERROR;

    VarType = *(IDL_LONG *)Argv[2];
    NElem   = *(IDL_LONG *)Argv[3];
    

    if ((VarMode[0] != 'P') && (VarMode[0] != 'G'))  
        return IDL_DATATYPE_ERROR;
    
    _logger->log(Logger::LOG_LEV_WARNING, "CreaVar is obsolete. Use WriteVar to create&set a variable");
    //ErrC = CreaVar( MyName, VarName, VarType, VarMode, NElem, NULL);
    return NO_ERROR;
}

//@Function: getvar
//
// Wrapper for the GetVar() rtdblib function. Required arguments:
//
// 1) variable name (string)
// 2) variable value (array or scalar)
// 3) variable length (long)
// 4) timeout in milliseconds (long)
//
// Returns an integer error code.
//
// The array or scalar passed as second argument will be filled with the variable value
//@
IDL_LONG getvar( int Argc, void *Argv[])
{
    char *varname;
    UCHAR *varvalue;
    IDL_LONG timeout, varlen;
    MsgBuf *msgb;
    Variable *var;

    int ErrC =0;


    // Check if the number of arguments is correct
    if (Argc != 4) return IDL_PAR_NUM_ERROR;


    // Check if we are connected with the Supervisor
    if (! idl_setup_ok)
        return MISSING_CLIENTNAME_ERROR;


    varname = ((IDL_STRING *)Argv[0])->s;
    varvalue = (UCHAR *)(Argv[1]);
    varlen = *((IDL_LONG *)(Argv[2]));
    timeout  = *((IDL_LONG *)(Argv[3]));
    //timeout = IDL_LongScalar(Argv[3]);

    msgb = thGetVar(varname,timeout);
    if (msgb == NULL) return PLAIN_ERROR(ErrC);
    var = thValue(msgb);
    _logger->log(Logger::LOG_LEV_TRACE, "type %d, nitems %d size %d", 
            var->H.Type, var->H.NItems, VarSize(var->H.Type, var->H.NItems) );
    memcpy(varvalue,  var->Value.B8, VarSize(var->H.Type, var->H.NItems));
    thRelease(msgb);
    
    //if ((ErrC = ReadVar( MyName, varname, varvalue, varlen, timeout, NULL, NULL)) <0)
    //    return PLAIN_ERROR(ErrC);
    return NO_ERROR;
} 

//@Function: writevar
//
// Wrapper for the WriteVar() rtdblib function. Required arguments:
//
// 1) variable name (string)
// 2) variable type (long)
// 3) variable length (long)
// 4) variable value (array or scalar)
// 5) timeout in milliseconds (long)
//
// Returns an integer error code.
//
//@
IDL_LONG writevar( int Argc, void *Argv[])
{
    char *varname; 
    UCHAR *varvalue;
    IDL_LONG timeout, varlen, vartype;

    int ErrC =0;


    // Check if the number of arguments is correct
    if (Argc != 5) return IDL_PAR_NUM_ERROR;


    // Check if we are connected with the Supervisor
    if (! idl_setup_ok)
        return MISSING_CLIENTNAME_ERROR;


    varname  = ((IDL_STRING *)Argv[0])->s;
    vartype  = *((IDL_LONG *)(Argv[1]));
    varlen   = *((IDL_LONG *)(Argv[2]));
    if (vartype == CHAR_VARIABLE){
        varvalue = (UCHAR *) ( ((IDL_STRING *)Argv[3])->s);
    } else {
        varvalue = (UCHAR *)(Argv[3]);
    }
    timeout  = *((IDL_LONG *)(Argv[4]));



    _logger->log(Logger::LOG_LEV_TRACE, "varname: %s - vartype: %d - varlen: %d",
            varname, vartype, varlen);

    if ((ErrC = thWriteVar(varname, vartype, varlen, varvalue, timeout)) < 0)
    //if ((ErrC = WriteVar( MyName, varname, vartype, varlen, varvalue, timeout, NULL, NULL)) <0)
        return PLAIN_ERROR(ErrC);

    return NO_ERROR;
}


// Funciton to get MsgRTDB variable type
IDL_LONG get_vartypes(int Argc, void *Argv[])
{
    IDL_LONG datalen;
    int *buffer;

    datalen = *((IDL_LONG *)(Argv[0]));
    buffer = (int *)(Argv[1]);
    if (datalen != 7) return IDL_PAR_NUM_ERROR;
    *buffer = INT_VARIABLE;
    *(buffer+1) = REAL_VARIABLE;
    *(buffer+2) = CHAR_VARIABLE;
    *(buffer+3) = BIT8_VARIABLE;
    *(buffer+4) = BIT16_VARIABLE;
    *(buffer+5) = BIT32_VARIABLE;
    *(buffer+6) = BIT64_VARIABLE;

    return NO_ERROR;

}



////////////////////////////////////////////////
//
// Below, new functions implemented using DLM 
//
////////////////////////////////////////////////





// --------------------- SHARED MEMORY MANAGEMENT -------------------- //
// ------------------------------------------------------------------- //


// Shared memory variables
BufInfo *Info = NULL;
int info_size=0;

//Function to check if we are already attached to a buffer.
// Returns a pointer to the buffer Info struct or NULL if we are
// not attached
//
BufInfo *is_attached( char *bufname)
{
    char *pos, *onlyname;
    int i;

    pos = strchr(bufname, ':');
    *pos = 0;   // Zero-terminate after the producer
    onlyname = pos+1; // points to the buffer name

    if ((info_size>0) && (Info != NULL))
        for (i=0; i< info_size; i++)
            {
            _logger->log(Logger::LOG_LEV_TRACE, "name: %s - onlyname: %s - prod: %s - bufname; %s",
                    Info[i].com.name, onlyname, Info[i].com.producer, bufname);
            if (strncmp( Info[i].com.name, onlyname, VAR_NAME_LEN) == 0)
                if (strncmp( Info[i].com.producer, bufname, PROC_NAME_LEN) ==0)
                    {
                    *pos = ':';    // Restore the corret name
                    return Info+i;
                    }
            }

    *pos = ':';    // Restore the corret name
    return NULL;
}
  

// Function to resize the Info vector destroying the 'bufname' element
void resize_Info( char *bufname)
{
    char *pos, *onlyname;
    int i;

    pos = strchr(bufname, ':');
    *pos = 0;   // Zero-terminate after the producer
    onlyname = pos+1; // points to the buffer name
    _logger->log(Logger::LOG_LEV_TRACE, "Resizing buffer queue...");

    if ((info_size>0) && (Info != NULL))
        for (i=0; i< info_size; i++)
            {
                _logger->log(Logger::LOG_LEV_TRACE, "name: %s - onlyname: %s - prod: %s - bufname; %s",
                    Info[i].com.name, onlyname, Info[i].com.producer, bufname);
            if (strncmp( Info[i].com.name, onlyname, VAR_NAME_LEN) == 0)
                if (strncmp( Info[i].com.producer, bufname, PROC_NAME_LEN) ==0)
                    {
                    *pos = ':';    // Restore the corret name
                    memmove(Info+i,Info+i+1,(info_size-i-1)*sizeof(BufInfo));
                    info_size--;
                    _logger->log(Logger::LOG_LEV_TRACE, "Resized Info array, deleted elements %d, new info_size %d", i, info_size);
                    return;
                    }
            }

    _logger->log(Logger::LOG_LEV_TRACE, "Buffer not found...");
    *pos = ':';    // Restore the corret name
    return;
}



// Function to attach to a new shared buffer 
int idl_attach( char *bufname, int type, BufInfo **extern_info)
{
   BufInfo *local_info;

   // Allocate a new vector element 
   if (!Info)
        {
        Info = (BufInfo *) malloc( sizeof(BufInfo));
        info_size =1;
        }
   else
        {
        info_size++;
        Info = (BufInfo *) realloc( Info, sizeof(BufInfo) * info_size);
        }

    if (!Info)
        {
            _logger->log(Logger::LOG_LEV_ERROR, "Malloc error");
        info_size=0;
        return MEMORY_ALLOC_ERROR;
        }

    local_info = &(Info[info_size-1]);

    _logger->log(Logger::LOG_LEV_INFO, "Attacching to %s as %s...", bufname, MyName);
    ErrC = bufRequest(MyName, bufname, type, local_info);
    _logger->log(Logger::LOG_LEV_TRACE, "shm name: %s, producer: %s", Info[info_size-1].com.name, Info[info_size-1].com.producer );
    if (PLAIN_ERROR(ErrC) != 0) {
        info_size--;
        Info = (BufInfo *) realloc( Info, sizeof(BufInfo) * info_size);
        _logger->log(Logger::LOG_LEV_WARNING, "BufRequest failed (errno %d), realloc to infosize: %d", errno, info_size);
        return PLAIN_ERROR(ErrC);
    }
    _logger->log(Logger::LOG_LEV_TRACE, "BufRequest returned %s (error code %d)", lao_strerror(ErrC), ErrC);
    _logger->log(Logger::LOG_LEV_TRACE, "Data readable length: %d", local_info->com.lng);
    _logger->log(Logger::LOG_LEV_TRACE, "Attacched. Info_size: %d", info_size);

    if (extern_info)
        *extern_info = local_info;

    return NO_ERROR;
}


// Function to call first of all to initialize the client IDL in order to connect with the Supervisor 
IDL_LONG idl_setup( int lArgc, void *Argv[])
{
    char *pchClientname;

    // clientname and timeout required
    if (lArgc !=2 ) return(IDL_PAR_NUM_ERROR);
    pchClientname =  ((IDL_STRING *)Argv[0])->s;//IDL_VarGetString(Argv[0]);
    timeout       =  *((IDL_LONG *)Argv[1]); // IDL_LongScalar(Argv[1]);
    memset( MyName, 0, PROC_NAME_LEN);
    strncpy( MyName, pchClientname , PROC_NAME_LEN-1);
    
    // Initialize thrdlib
    ErrC = thInit(pchClientname);
    if (IS_ERROR(ErrC)){ 
        _logger->log(Logger::LOG_LEV_WARNING, "thInit failed: %s (errcode %d)",lao_strerror(ErrC), ErrC);
        return PLAIN_ERROR(ErrC);
    }
    
    ErrC = thStart(server_ip, 5);
    if (IS_ERROR(ErrC)){ 
        _logger->log(Logger::LOG_LEV_WARNING, "thStart failed: %s (errcode %d)",lao_strerror(ErrC), ErrC);
        return PLAIN_ERROR(ErrC);
    }

    idl_setup_ok=1; 
    return NO_ERROR;
}


IDL_LONG smreadbuf(int Argc, void *Argv[])
{
   char *ptrBufName;
   ErrC = 0;
   int ErrC1 = 0;
   int ErrC2 = 0;
   int ErrC3 = 0;
   int retry, n_retry=1;
   int TypeBuf;
   IDL_INT type;
   unsigned char *dataptr;
   BufInfo *local_info;
   
//   printf("Argc,%d\n", Argc);

   if (Argc !=4) return(IDL_PAR_NUM_ERROR);
   
   ptrBufName = ((IDL_STRING *)Argv[0])->s;
   type = *((IDL_INT *)Argv[1]);
   timeout = *((IDL_LONG *)Argv[2]);
   dataptr = (UCHAR *)Argv[3];

   _logger->log(Logger::LOG_LEV_TRACE, "smreadbuf argv: %s %d %d", ptrBufName, type, timeout);

   if (type <= 0 || type >=3 ) return (IDL_DATATYPE_ERROR);
   switch (type) 
   {
      case 1 : TypeBuf=BUFCONT;
      break;
      case 2 : TypeBuf=BUFSNAP;
      break;
   }

   for (retry=0; retry <= n_retry; retry++)
   {
      if (! (local_info = is_attached(ptrBufName)))
      {
          _logger->log(Logger::LOG_LEV_INFO, "I'm attaching with idl_attach (# retry  %d)...",retry);
         ErrC1 = idl_attach(ptrBufName, TypeBuf, &local_info);
       
         _logger->log(Logger::LOG_LEV_TRACE, "BufRequest returned %s (errcode %d)", lao_strerror(ErrC1), ErrC1);
         if (IS_ERROR(ErrC1)) return PLAIN_ERROR(ErrC1);
      }
    
      ErrC2 = bufRead (local_info, dataptr, timeout);
      _logger->log(Logger::LOG_LEV_TRACE, "bufRead returned %s (errcode %d, errno %d)", lao_strerror(ErrC2), ErrC2, errno);

      if (PLAIN_ERROR(ErrC2) == SEM_LOCK_ERROR) {

         ErrC3 = bufRelease(MyName, local_info);
         if (IS_ERROR(ErrC3)) return PLAIN_ERROR(ErrC3);

         memset(local_info, 0, sizeof(local_info));
         _logger->log(Logger::LOG_LEV_TRACE, "Memset done, old shared buffer cleared.");
         resize_Info(ptrBufName);

      } else {
          _logger->log(Logger::LOG_LEV_TRACE, "Memset not done!");
           break;
      }
   }

   if (IS_ERROR(ErrC2)) return PLAIN_ERROR(ErrC2);
   //ErrC3 = bufRelease (MyName, &Info);
   //printf("ErrC3 from BufRelease %d\n", PLAIN_ERROR(ErrC3));
   return PLAIN_ERROR(ErrC2) ;
 
}

// TODO questa e' da rivedere: non si puo' chiudere 
// il socket alla msglib in questo modo.
IDL_LONG idl_close( int lArgc, void *lArgv[])
{
    ErrC = 0;
    //int i;
    //int socket = GetServerSocket();
    //close(socket);
    ErrC = thClose();
    _logger->log(Logger::LOG_LEV_TRACE, "thClose returned %s (errcode %d)",lao_strerror(ErrC), ErrC);
    if (IS_ERROR(ErrC)) {
        return PLAIN_ERROR(ErrC);
    }

    //Detach from shared memory buffers 
    if ((Info != NULL) && (info_size>0))
    {
//          for (i=0; i<info_size; i++)
 //             bufRelease( MyName, Info+i);
        free(Info);
        Info = NULL;
        info_size=0;
    }
    idl_setup_ok =0;
    return NO_ERROR;
}


int IDL_Load(void)
{
  // These tables contain information on the functions and procedures
  // that make up the TESTMODULE DLM. The information contained in these
  // tables must be identical to that contained in testmodule.dlm.
  //
   static IDL_SYSFUN_DEF2 function_addr[] = {
      { {(IDL_SYSRTN_GENERIC) IDLGetDiagnParam}, 	"GETDIAGNPARAM" , 			5, 5, IDL_SYSFUN_DEF_F_KEYWORDS , 0},
      { {(IDL_SYSRTN_GENERIC) IDLSetDiagnParam}, 	"SETDIAGNPARAM" , 			4, 5, IDL_SYSFUN_DEF_F_KEYWORDS , 0},
      { {(IDL_SYSRTN_GENERIC) IDLGetDiagnValue}, 	"GETDIAGNVALUE" , 			5, 5, IDL_SYSFUN_DEF_F_KEYWORDS , 0},
      { {(IDL_SYSRTN_GENERIC) IDLGetDiagnBuffer},	"GETDIAGNBUFFER", 			5, 5, IDL_SYSFUN_DEF_F_KEYWORDS , 0},
      { {(IDL_SYSRTN_GENERIC) IDLSetLogLevel},   	"SETLOGLEVEL", 	  			1, 1, IDL_SYSFUN_DEF_F_KEYWORDS , 0},
      
      { {(IDL_SYSRTN_GENERIC) getBcuId},         	"GETBCUID_WRAP",  			1, 1, 0 , 0},
      { {(IDL_SYSRTN_GENERIC) getBcuIp},   		    "GETBCUIP_WRAP",         	1, 1, 0 , 0},
      { {(IDL_SYSRTN_GENERIC) getBcuMasterUdpPort}, "GETBCUMASTERUDPPORT_WRAP", 1, 1, 0 , 0},
      //{ grab_capture, "GRAB_CAPTURE", 1, 2, 0, 0},
   };
   static IDL_SYSFUN_DEF2 procedure_addr[] = {
   //   { (IDL_SYSRTN_GENERIC) testpro, "TESTPRO", 0, IDL_MAX_ARRAY_DIM, 0, 0},
   };

   // Create a message block to hold our messages. Save its handle where
   // the other routines can access it.
   if (!(msg_block = IDL_MessageDefineBlock("diagnlib", IDL_CARRAY_ELTS(msg_arr), msg_arr))) return IDL_FALSE;

   // Register our routine. The routines must be specified exactly the same
   // as in testmodule.dlm.
   return IDL_SysRtnAdd(function_addr, TRUE, IDL_CARRAY_ELTS(function_addr)) &&
      IDL_SysRtnAdd(procedure_addr, FALSE, IDL_CARRAY_ELTS(procedure_addr));
}

