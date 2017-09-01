#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <string>


extern "C"{
	#include "idl_export.h"
	#include "base/errordb.h"
	#include "base/common.h"
	#include "base/thrdlib.h"
	#include "aoslib/aoslib.h"
}

#include "idl_diagnlib.h"
#include "diagnlib.h"
#include "mastdiagnlib.h"
#include "Logger.h"
#include "stdconfig.h"

using namespace Arcetri;


void idl_free_cb_diagn(unsigned char* loc) {
    Logger::get()->log(Logger::LOG_LEV_TRACE, "IDL freeing memory starting at %p",(void*)loc);
    free(loc);
}


static IDL_STRUCT_TAG_DEF param_tags[] = {
    { (char*)"FAMILY", 0, (void *) IDL_TYP_STRING, 0},
    { (char*)"INDEX", 0, (void *) IDL_TYP_LONG, 0},
    { (char*)"ALARM_MIN", 0, (void *) IDL_TYP_DOUBLE, 0},
    { (char*)"WARNING_MIN", 0, (void *) IDL_TYP_DOUBLE, 0},
    { (char*)"WARNING_MAX", 0, (void *) IDL_TYP_DOUBLE, 0},
    { (char*)"ALARM_MAX", 0, (void *) IDL_TYP_DOUBLE, 0},
    { (char*)"MEAN_PERIOD", 0, (void *) IDL_TYP_DOUBLE, 0},
    { (char*)"CAF", 0, (void *) IDL_TYP_ULONG, 0},
    { (char*)"ENABLED", 0, (void *) IDL_TYP_BYTE,0 },
    { (char*)"SLOW", 0, (void *) IDL_TYP_BYTE,0 },
    { 0, 0, NULL, 0 }
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
//
IDL_VPTR IDLGetDiagnParam(int Argc, IDL_VPTR Argv[], char *Argk)
{
	Logger* logger = Logger::get();

    typedef struct {
        IDL_KW_RESULT_FIRST_FIELD;   // Must be first entry in this structure
        double timeout;
    } KW_RESULT;
    static IDL_KW_PAR kw_pars[] = { //Lexically sorted!
        { (char*)"TIMEOUT"  , IDL_TYP_DOUBLE, 1, 0, 0, (char*)IDL_KW_OFFSETOF(timeout) },
        { NULL, 0, 0, 0, NULL, NULL }
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

    logger->log(Logger::LOG_LEV_TRACE, "app %s, family %s, from %d, to %d, timeout %d",
            diagnapp, family, from, to, tmout);

    try{
        DiagnWhich which(family, from, to);
        dict = GetDiagnVarParam( diagnapp,which, tmout);
    } catch (AOException &e) {
        logger->log(Logger::LOG_LEV_ERROR, e.what());
        return IDL_GettmpLong(e._errcode);
    }

    n_ele=dict.size();
    if (n_ele == 0) return IDL_GettmpLong(VALUE_OUT_OF_RANGE_ERROR);

    // Create the structure definition
    void *p = IDL_MakeStruct(0, param_tags);

    // Allocate memory for struct array
    s_param   = (IDL_Param*)malloc(n_ele*sizeof(IDL_Param));

    // Fill structure fields
    i=0;
    for (ParamDict::iterator it = dict.begin(); it != dict.end();  it++){
        DiagnWhich wh = (*it).first;
        DiagnParam pa = (*it).second;

        std::ostringstream oss; oss << wh << " :: " << pa;
        logger->log(Logger::LOG_LEV_DEBUG,  oss.str() );

        IDL_StrStore(&s_param[i].family, (char*)(wh.Family().c_str()));
        s_param[i].index = wh.From();

        DiagnRange<float> rng  = pa.Range();
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
    IDL_VPTR v_pa = IDL_ImportArray( 1, &n_ele, IDL_TYP_STRUCT, (UCHAR *) s_param  , idl_free_cb_diagn, (_idl_structure*)p);
    IDL_VarCopy(v_pa,params);

    return IDL_GettmpLong(NO_ERROR);
}


//
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
	Logger* logger = Logger::get();

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
        { (char*)"ALARM_MAX"  , IDL_TYP_DOUBLE, 1, 0, (int*)IDL_KW_OFFSETOF(alarm_max_there),
            (char*)IDL_KW_OFFSETOF(alarm_max) },
        { (char*)"ALARM_MIN"  , IDL_TYP_DOUBLE, 1, 0, (int*)IDL_KW_OFFSETOF(alarm_min_there),
            (char*)IDL_KW_OFFSETOF(alarm_min) },
        { (char*)"CONS_ALLOWED_FAULTS", IDL_TYP_LONG, 1, 0, (int*)IDL_KW_OFFSETOF(caf_there),
            (char*)IDL_KW_OFFSETOF(caf) },
        { (char*)"ENABLED", IDL_TYP_BYTE, 1, 0, (int*)IDL_KW_OFFSETOF(ena_there),
            (char*)IDL_KW_OFFSETOF(ena) },
        { (char*)"MEAN_PERIOD", IDL_TYP_DOUBLE, 1, 0, (int*)IDL_KW_OFFSETOF(mean_period_there),
            (char*)IDL_KW_OFFSETOF(mean_period) },
        { (char*)"SLOW", IDL_TYP_BYTE, 1, 0, (int*)IDL_KW_OFFSETOF(slow_there),
            (char*)IDL_KW_OFFSETOF(slow) },
        { (char*)"TIMEOUT"    , IDL_TYP_DOUBLE, 1, 0, 0, (char*)IDL_KW_OFFSETOF(timeout) },
        { (char*)"WARNING_MAX"  , IDL_TYP_DOUBLE, 1, 0, (int*)IDL_KW_OFFSETOF(warning_max_there),
            (char*)IDL_KW_OFFSETOF(warning_max) },
        { (char*)"WARNING_MIN"  , IDL_TYP_DOUBLE, 1, 0, (int*)IDL_KW_OFFSETOF(warning_min_there),
            (char*)IDL_KW_OFFSETOF(warning_min) },
        { NULL, 0, 0, 0, NULL, NULL }
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

        ret = IDL_StructTagInfoByName(idl_params->value.s.sdef, (char*)"ALARM_MIN", 0,  &s_tag);
        if (ret != -1 && s_tag->type == IDL_TYP_DOUBLE)
            params.SetAlarmMin( *(double*)(idl_params->value.s.arr->data + ret) );
        else return IDL_GettmpLong(IDL_INPUT_TYPE);

        ret = IDL_StructTagInfoByName(idl_params->value.s.sdef, (char*)"ALARM_MAX", 0,  &s_tag);
        if (ret != -1 && s_tag->type == IDL_TYP_DOUBLE)
            params.SetAlarmMax( *(double*)(idl_params->value.s.arr->data + ret) );
        else return IDL_GettmpLong(IDL_INPUT_TYPE);

        ret = IDL_StructTagInfoByName(idl_params->value.s.sdef, (char*)"WARNING_MIN", 0,  &s_tag);
        if (ret != -1 && s_tag->type == IDL_TYP_DOUBLE)
            params.SetWarningMin( *(double*)(idl_params->value.s.arr->data + ret) );
        else return IDL_GettmpLong(IDL_INPUT_TYPE);

        ret = IDL_StructTagInfoByName(idl_params->value.s.sdef, (char*)"WARNING_MAX", 0,  &s_tag);
        if (ret != -1 && s_tag->type == IDL_TYP_DOUBLE)
            params.SetWarningMax( *(double*)(idl_params->value.s.arr->data + ret) );
        else return IDL_GettmpLong(IDL_INPUT_TYPE);

        ret = IDL_StructTagInfoByName(idl_params->value.s.sdef, (char*)"MEAN_PERIOD", 0,  &s_tag);
        if (ret != -1 && s_tag->type == IDL_TYP_DOUBLE)
            params.SetMeanPeriod( *(double*)(idl_params->value.s.arr->data + ret) );
        else return IDL_GettmpLong(IDL_INPUT_TYPE);

        ret = IDL_StructTagInfoByName(idl_params->value.s.sdef, (char*)"CAF", 0,  &s_tag);
        if (ret != -1 && s_tag->type == IDL_TYP_ULONG)
            params.SetConsAllowFaults( *(IDL_ULONG*)(idl_params->value.s.arr->data + ret) );
        else return IDL_GettmpLong(IDL_INPUT_TYPE);

        ret = IDL_StructTagInfoByName(idl_params->value.s.sdef, (char*)"ENABLED", 0,  &s_tag);
        if (ret != -1 && s_tag->type == IDL_TYP_BYTE)
            params.SetEnabled( *(UCHAR*)(idl_params->value.s.arr->data + ret) );
        else return IDL_GettmpLong(IDL_INPUT_TYPE);

        ret = IDL_StructTagInfoByName(idl_params->value.s.sdef, (char*)"SLOW", 0,  &s_tag);
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
        logger->log(Logger::LOG_LEV_DEBUG, oss.str());
        c_matched = SetDiagnVarParam( diagnapp, which, params, tmout);
    } catch (AOException &e) {
        logger->log(Logger::LOG_LEV_ERROR, e.what());
        return IDL_GettmpLong(e._errcode);
    }

    return IDL_GettmpLong(c_matched);
}

///////////////////////////////////////////////////////////////////////////////////////////


static IDL_STRUCT_TAG_DEF value_tags[] = {
    { (char*)"FAMILY", 0, (void *) IDL_TYP_STRING, 0},
    { (char*)"INDEX", 0, (void *) IDL_TYP_LONG, 0},
    { (char*)"AVERAGE", 0, (void *) IDL_TYP_DOUBLE, 0},
    { (char*)"STDDEV", 0, (void *) IDL_TYP_DOUBLE, 0},
    { (char*)"LAST", 0, (void *) IDL_TYP_DOUBLE, 0},
    { (char*)"TIME", 0, (void *) IDL_TYP_DOUBLE, 0},
    { (char*)"STATUS", 0, (void *) IDL_TYP_LONG, 0},
    { 0, 0, NULL, 0 }
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
	Logger* logger = Logger::get();

    typedef struct {
        IDL_KW_RESULT_FIRST_FIELD;   // Must be first entry in this structure
        double timeout;
    } KW_RESULT;
    static IDL_KW_PAR kw_pars[] = { //Lexically sorted!
        { (char*)"TIMEOUT"  , IDL_TYP_DOUBLE, 1, 0, 0, (char*)IDL_KW_OFFSETOF(timeout) },
        { NULL , 0, 0, 0, NULL, NULL}
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
        logger->log(Logger::LOG_LEV_ERROR, e.what());
        return IDL_GettmpLong(e._errcode);
    }

    n_ele=dict.size();
    if (n_ele == 0) return IDL_GettmpLong(VALUE_OUT_OF_RANGE_ERROR);

    // Create the structure definition
    void *p = IDL_MakeStruct(0, value_tags);

    // Allocate memory for struct array
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

        logger->log(Logger::LOG_LEV_DEBUG, "[GetDiagnValue] %s  %s --- %s [%s:%d]",
                diagnapp, wh.str().c_str(), va.str().c_str(), __FILE__,__LINE__);
    }

    // Import the data area s_matched into an IDL structure, note that no data are moved.
    //matched = IDL_ImportNamedArray("pippo", 1, &n_ele, IDL_TYP_STRUCT, (UCHAR *) s_matched, 0, s);
    IDL_VPTR v_va = IDL_ImportArray( 1, &n_ele, IDL_TYP_STRUCT, (UCHAR *) s_value  , idl_free_cb_diagn, (_idl_structure*)p);
    IDL_VarCopy(v_va,values);

    return IDL_GettmpLong(NO_ERROR);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static IDL_MEMINT values_dims[] = { 1, 1 };
static IDL_MEMINT times_dims[]  = { 1, 1 };
static IDL_STRUCT_TAG_DEF buffer_tags[] = {
    { (char*)"FAMILY", 0, (void *) IDL_TYP_STRING, 0},
    { (char*)"INDEX", 0, (void *) IDL_TYP_LONG, 0},
    { (char*)"TIMES",  times_dims,  (void *) IDL_TYP_DOUBLE, 0},
    { (char*)"VALUES", values_dims, (void *) IDL_TYP_DOUBLE, 0},
    { 0 , 0, NULL, 0}
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
IDL_VPTR IDLGetDiagnBuffer(int Argc, IDL_VPTR Argv[], char *Argk)
{
	Logger* logger = Logger::get();

    typedef struct {
        IDL_KW_RESULT_FIRST_FIELD;   // Must be first entry in this structure
        double timeout;
    } KW_RESULT;
    static IDL_KW_PAR kw_pars[] = { //Lexically sorted!
        { (char*)"TIMEOUT"  , IDL_TYP_DOUBLE, 1, 0, 0, (char*)IDL_KW_OFFSETOF(timeout) },
        { NULL, 0, 0, 0, NULL, NULL }
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
        logger->log(Logger::LOG_LEV_ERROR, e.what());
        return IDL_GettmpLong(e._errcode);
    }

    logger->log(Logger::LOG_LEV_TRACE, "[GetDiagnBuffer] Returned %d elements in dicts  [%s:%d]",
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
    logger->log(Logger::LOG_LEV_TRACE, "[GetDiagnBuffer] Malloc'ed %d bytes in %p [%s:%d]",
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
        logger->log(Logger::LOG_LEV_TRACE, "[GetDiagnBuffer]  s[%d] %p [%s:%d]",
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
        logger->log(Logger::LOG_LEV_TRACE, "[GetDiagnBuffer] Assigned %p: %s (%s) [%s:%d]",
                s_ptr, IDL_STRING_STR(s_ptr), (char*)(wh.Family().c_str()) , __FILE__,__LINE__);
        tmp_ptr += sizeof(IDL_STRING);

        // From
        l_ptr = (IDL_LONG *)( tmp_ptr);
        *l_ptr = wh.From();
        logger->log(Logger::LOG_LEV_TRACE, "[GetDiagnBuffer] Assigned %p: %d (%d) [%s:%d]",
                l_ptr, *l_ptr, wh.From() , __FILE__,__LINE__);
        tmp_ptr += sizeof(IDL_LONG);

        // Times
        d_ptr = (double *)(tmp_ptr);
        RunningMean<double>& rm_times =  bu->Times();
        for(RunningMean<double>::iterator rm = rm_times.begin(); rm != rm_times.end(); rm++){
            *d_ptr++ = *rm;
            logger->log(Logger::LOG_LEV_TRACE, "[GetDiagnBuffer] Assigned %p: %g (%g) [%s:%d]",
                    d_ptr, *(d_ptr-1), *rm, __FILE__,__LINE__);
            tmp_ptr += sizeof(double);
            }

        // Values
        d_ptr = (double *)(tmp_ptr);
        RunningMean<double>& rm_values =  bu->Values();
        for(RunningMean<double>::iterator rm = rm_values.begin(); rm != rm_values.end(); rm++){
            *d_ptr++ = *rm;
            logger->log(Logger::LOG_LEV_TRACE, "[GetDiagnBuffer] Assigned %p: %g (%g) [%s:%d]",
                    d_ptr, *(d_ptr-1), *rm, __FILE__,__LINE__);
            tmp_ptr += sizeof(double);
        }
        logger->log(Logger::LOG_LEV_DEBUG, "[GetDiagnBuffer] %s  %s --- %s [%s:%d]",
                diagnapp, wh.str().c_str(), (*bu).str().c_str(), __FILE__,__LINE__);
    }

    // attach data to the created structure
    IDL_VPTR vv = IDL_ImportArray( 1, &uno, IDL_TYP_STRUCT, (UCHAR *)s_data, idl_free_cb_diagn, (_idl_structure*)struct_s);
    IDL_VarCopy(vv, buffer);

    return IDL_GettmpLong(NO_ERROR);
}

////////////////////////////////////////////////////////////////////
//
// DumpDiagnParams
//
// DumpDiagnParams(DiagnApp, file, timeout=timeout)
//
// INPUT
//      DiagnApp           Diagnostic application name ('FASTDGN00', 'HOUSEKPR00', ...)
//      file               dump file name
//
// KEYWORD
//      timeout         timeout [s] (def. 1s)
//
// RETURN
//      1 if dump file written, 0 on error
//
IDL_VPTR IDLDumpDiagnParams(int Argc, IDL_VPTR Argv[], char *Argk)
{
	Logger* logger = Logger::get();

    typedef struct {
        IDL_KW_RESULT_FIRST_FIELD;   // Must be first entry in this structure
        double timeout;
    } KW_RESULT;
    static IDL_KW_PAR kw_pars[] = { //Lexically sorted!
        { (char*)"TIMEOUT"  , IDL_TYP_DOUBLE, 1, 0, 0, (char*)IDL_KW_OFFSETOF(timeout) },
        { NULL, 0, 0, 0, NULL, NULL }
    };

    KW_RESULT           kw;
    char                *diagnapp;
    char                *filename;
    int                 tmout;
	int                 ret;

    kw.timeout = 1.0;
    (void) IDL_KWProcessByOffset(Argc, Argv, Argk, kw_pars, NULL, 1, &kw);
    tmout = (int) (kw.timeout * 1000);

    diagnapp=IDL_VarGetString(Argv[0]);
    filename=IDL_VarGetString(Argv[1]);

	try {
	    ret = DumpDiagnVarParams(diagnapp, filename, tmout);
	} catch (AOException &e) {
        logger->log(Logger::LOG_LEV_ERROR, e.what());
        return IDL_GettmpLong(e._errcode);
    }

    IDL_KW_FREE;
    return IDL_GettmpLong(ret);
}


////////////////////////////////////////////////////////////////////
//
// IDLMastdiagnDumpHistory
//
// IDLMastdiagnDumpHistory()
//
// INPUT
//      masterdiagonstic conf file name ("conf/right/....")
//
// RETURN
//      NO_ERROR if dump file command correctly sent, IDL_HISTORY_DUMP_ERROR on error
//
IDL_VPTR IDLMastdiagnDumpHistory(int /* Argc */, IDL_VPTR Argv[]) {
	Logger* logger = Logger::get();
	try {
	    const char *masterdiagnosticconffile = IDL_VarGetString(Argv[0]);
        MastDiagnInterface _mastDiagn(masterdiagnosticconffile, Logger::LOG_LEV_DEBUG);
		logger->log(Logger::LOG_LEV_DEBUG, (char*)"Requesting MasterDiagnostic to dump history to file...");
		_mastDiagn.dumpAdSecDiagnHistory();
		logger->log(Logger::LOG_LEV_DEBUG, (char*)"Dump command successfully sent");
		return IDL_GettmpLong(NO_ERROR);
	}
	catch(MastDiagnInterfaceException& e) {
		return IDL_GettmpLong(IDL_HISTORY_DUMP_ERROR);
	}
}


////////////////////////////////////////////////////////////////////
//
// IDLLoadOfflineDiagnFrames
//
// IDLLoadOfflineDiagnFrames(DiagnApp, frameIndex)
//
// INPUT
//      DiagnApp           Diagnostic application name ('FASTDGN00', 'HOUSEKPR00', ...)
//      frameIndex         index of frame wanted
//
// RETURN
//      NO_ERROR if frame loaded, an error code otherwise
//
IDL_VPTR IDLLoadOfflineDiagnFrames(int /* Argc */, IDL_VPTR Argv[]) {
	Logger* logger = Logger::get();

	 //if (Argc != 1) {
	 //   	logger->log(Logger::LOG_LEV_ERROR, "IDLLoadOfflineDiagnFrames: wrong parameters number (1 required, %d received)", Argc);
	 //   	return NULL;
	 //}

	 // Get parameters
	 char *diagnapp = IDL_VarGetString(Argv[0]);
	 int frameIndex = IDL_LongScalar(Argv[1]);

	int ret;
	 try {
		 logger->log(Logger::LOG_LEV_DEBUG, "Requesting FastDiagnostic to load offline frame %d...", frameIndex);
		 ret = LoadOfflineDiagnFrame(diagnapp, frameIndex);
	 } catch (AOException &e) {
		 logger->log(Logger::LOG_LEV_ERROR, e.what());
	     return IDL_GettmpLong(e._errcode);
	 }

	 return IDL_GettmpLong(ret);
}
