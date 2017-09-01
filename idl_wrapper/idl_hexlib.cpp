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
#include "idl_hexlib.h"
#include "Logger.h"
#include "stdconfig.h"


using namespace std;
using namespace Arcetri;

extern Config_File _adsecConfig;

int aosinitialize(){
    static bool aos_initalized = false;
    int ret = NO_ERROR;
    if (!aos_initalized) {
        try {
            string side = Utils::getAdoptSide();
            ret = aos_init( (char*) ( side.c_str())  ); 
            if (!IS_ERROR(ret)) {
                aos_initalized = true;
                ret = NO_ERROR;
            }
        } catch (AOException &e) {
            return  NOT_INIT_ERROR; //have to open_superv() ??
        }
    }
    return ret;
}

// ********************************************************************************************************************* //
//
IDL_VPTR IDL_hexmoveto(int Argc, IDL_VPTR Argv[], char *Argk)
{
    typedef struct {  
        IDL_KW_RESULT_FIRST_FIELD;   // Must be first entry in this structure  
        double timeout;  
    } KW_RESULT;  
    static IDL_KW_PAR kw_pars[] = { //Lexically sorted! 
        { (char*)"TIMEOUT"  , IDL_TYP_DOUBLE, 1, 0, 0, (char*)IDL_KW_OFFSETOF(timeout) },  
        { NULL, 0, 0, 0, NULL, NULL }  
    };  
   
    KW_RESULT           kw;
	IDL_VPTR            src;
    double             *src_d;
    int                 tmout, n, ret;

	Logger* _logger = Logger::get("IDL_HEX", Logger::LOG_LEV_TRACE);
    if (IS_ERROR(ret=aosinitialize())) return IDL_GettmpLong(PLAIN_ERROR(ret));

    kw.timeout = 5.0;  
    (void) IDL_KWProcessByOffset(Argc, Argv, Argk, kw_pars, NULL, 1, &kw); 
    tmout = (int) (kw.timeout * 1000);
    IDL_KW_FREE;

	// Check inputs
    src = Argv[0];
 	IDL_ENSURE_SIMPLE(src); 
	IDL_ENSURE_ARRAY(src); 

	if (src->type != IDL_TYP_DOUBLE) src = IDL_CvtDbl(1, &Argv[0]); 
	if (src->value.arr->n_dim != 1)  
  		IDL_Message(IDL_M_NAMED_GENERIC, IDL_MSG_LONGJMP, "Input argument must be a column vector."); 
	n = src->value.arr->dim[0];  
	if (n != 6) IDL_Message(IDL_M_NAMED_GENERIC, IDL_MSG_LONGJMP,  "Input array must have 6 elements");

	src_d = (double *) src->value.arr->data; 
    int timeout_oss = 60000;
	// Move hexapod in the current stored reference system
    _logger->log(Logger::LOG_LEV_TRACE, "hexmoveto [%g,%g,%g,%g,%g,%g] [%s:%d]",
		          src_d[0], src_d[1], src_d[2], src_d[3], src_d[4], src_d[5], __FILE__,__LINE__);
	ret =  aos_hxpmoveto(src_d[0], src_d[1], src_d[2], src_d[3], src_d[4], src_d[5], timeout_oss) ;
    if (src != Argv[0]) IDL_DELTMP(src); // avoid "% Temporary variables are still checked yyout - cleaning up..." message
	return IDL_GettmpLong(PLAIN_ERROR(ret));
}

// ********************************************************************************************************************* //
//
IDL_VPTR IDL_hexmoveby(int Argc, IDL_VPTR Argv[], char *Argk)
{
    typedef struct {  
        IDL_KW_RESULT_FIRST_FIELD;   // Must be first entry in this structure  
        double timeout;  
    } KW_RESULT;  
    static IDL_KW_PAR kw_pars[] = { //Lexically sorted! 
        { (char*)"TIMEOUT"  , IDL_TYP_DOUBLE, 1, 0, 0, (char*)IDL_KW_OFFSETOF(timeout) },  
        { NULL, 0, 0, 0, NULL, NULL }  
    };  
   
    KW_RESULT           kw;
	IDL_VPTR            src;
    double             *src_d;
    int                 tmout, n, ret;

	Logger* _logger = Logger::get("IDL_HEX", Logger::LOG_LEV_TRACE);
    if (IS_ERROR(ret=aosinitialize())) return IDL_GettmpLong(PLAIN_ERROR(ret));

    kw.timeout = 5.0;  
    (void) IDL_KWProcessByOffset(Argc, Argv, Argk, kw_pars, NULL, 1, &kw); 
    tmout = (int) (kw.timeout * 1000);
    IDL_KW_FREE;

	// Check inputs
    src = Argv[0];
 	IDL_ENSURE_SIMPLE(src); 
	IDL_ENSURE_ARRAY(src); 

	if (src->type != IDL_TYP_DOUBLE) src = IDL_CvtDbl(1, &Argv[0]); 
	if (src->value.arr->n_dim != 1)  
  		IDL_Message(IDL_M_NAMED_GENERIC, IDL_MSG_LONGJMP, "Input argument must be a column vector."); 
	n = src->value.arr->dim[0];  
	if (n != 6) IDL_Message(IDL_M_NAMED_GENERIC, IDL_MSG_LONGJMP,  "Input array must have 6 elements");

	src_d = (double *) src->value.arr->data; 
    int timeout_oss = 60000;
	// Move hexapod in the current stored reference system
    _logger->log(Logger::LOG_LEV_TRACE, "hexmoveby [%g,%g,%g,%g,%g,%g] [%s:%d]",
		          src_d[0], src_d[1], src_d[2], src_d[3], src_d[4], src_d[5], __FILE__,__LINE__);
	ret =  aos_hxpmoveby(src_d[0], src_d[1], src_d[2], src_d[3], src_d[4], src_d[5], timeout_oss) ;
    if (src != Argv[0]) IDL_DELTMP(src); // avoid "% Temporary variables are still checked yyout - cleaning up..." message
	return IDL_GettmpLong(PLAIN_ERROR(ret));
}
// ********************************************************************************************************************* //
//
IDL_VPTR IDL_hexmovsph(int Argc, IDL_VPTR Argv[], char *Argk)
{
    typedef struct {  
        IDL_KW_RESULT_FIRST_FIELD;   // Must be first entry in this structure  
        double timeout;  
    } KW_RESULT;  
    static IDL_KW_PAR kw_pars[] = { //Lexically sorted! 
        { (char*)"TIMEOUT"  , IDL_TYP_DOUBLE, 1, 0, 0, (char*)IDL_KW_OFFSETOF(timeout) },  
        { NULL, 0, 0, 0, NULL, NULL }  
    };  
   
    KW_RESULT           kw;
	IDL_VPTR            src;
    double             *src_d;
    int                 tmout, n, ret;

	Logger* _logger = Logger::get("IDL_HEX", Logger::LOG_LEV_TRACE);
    if (IS_ERROR(ret=aosinitialize())) return IDL_GettmpLong(PLAIN_ERROR(ret));

    kw.timeout = 5.0;  
    (void) IDL_KWProcessByOffset(Argc, Argv, Argk, kw_pars, NULL, 1, &kw); 
    tmout = (int) (kw.timeout * 1000);
    IDL_KW_FREE;

	// Check inputs
    src = Argv[0];
 	IDL_ENSURE_SIMPLE(src); 
	IDL_ENSURE_ARRAY(src); 

	if (src->type != IDL_TYP_DOUBLE) src = IDL_CvtDbl(1, &Argv[0]); 
	if (src->value.arr->n_dim != 1)  
  		IDL_Message(IDL_M_NAMED_GENERIC, IDL_MSG_LONGJMP, "Input argument must be a column vector."); 
	n = src->value.arr->dim[0];  
	if (n != 3) IDL_Message(IDL_M_NAMED_GENERIC, IDL_MSG_LONGJMP,  "Input array must have 3 elements");

	src_d = (double *) src->value.arr->data; 
    int timeout_oss = 60000;
	// Move hexapod in the current stored reference system
    _logger->log(Logger::LOG_LEV_TRACE, "hexmoveonsphere [%g,%g,%g] [%s:%d]",
		          src_d[0], src_d[1], src_d[2], __FILE__,__LINE__);
	ret =  aos_hxpmovsph(src_d[0], src_d[1], src_d[2], timeout_oss) ;
    if (src != Argv[0]) IDL_DELTMP(src); // avoid "% Temporary variables are still checked yyout - cleaning up..." message
	return IDL_GettmpLong(PLAIN_ERROR(ret));
}

// ********************************************************************************************************************* //
//
IDL_VPTR IDL_hexbrake(int Argc, IDL_VPTR Argv[], char *Argk)
{
    typedef struct {  
        IDL_KW_RESULT_FIRST_FIELD;   // Must be first entry in this structure  
        UCHAR    open;
        UCHAR    close;
        double timeout;  
        int open_there;
        int close_there;
    } KW_RESULT;  
    static IDL_KW_PAR kw_pars[] = { //Lexically sorted! 
        { (char*)"CLOSE", IDL_TYP_BYTE, 1, 0, (int*)IDL_KW_OFFSETOF(close_there),
            (char*)IDL_KW_OFFSETOF(close) },  
        { (char*)"OPEN", IDL_TYP_BYTE, 1, 0, (int*)IDL_KW_OFFSETOF(open_there),
            (char*)IDL_KW_OFFSETOF(open) },  
        { (char*)"TIMEOUT"  , IDL_TYP_DOUBLE, 1, 0, 0, (char*)IDL_KW_OFFSETOF(timeout) },  
        { NULL, 0, 0, 0, NULL, NULL }  
    };  
 
    KW_RESULT           kw;
    int                 onoff;
    int                 tmout, ret;

	Logger* _logger = Logger::get("IDL_HEX", Logger::LOG_LEV_TRACE);
    if (IS_ERROR(ret=aosinitialize())) return IDL_GettmpLong(PLAIN_ERROR(ret));

    kw.timeout = 5.0;  
    (void) IDL_KWProcessByOffset(Argc, Argv, Argk, kw_pars, NULL, 1, &kw); 
    tmout = (int) (kw.timeout * 1000);
    
    if (kw.close_there && kw.open_there && kw.open == kw.close ) {
  		IDL_Message(IDL_M_NAMED_GENERIC, IDL_MSG_LONGJMP, "Cannot specify both OPEN and CLOSE keywords."); 
        return IDL_GettmpLong(IDL_PAR_NUM_ERROR);
    }
    if (kw.close_there && kw.close == 1)  onoff = 1;
    if (kw.open_there  && kw.open  == 1)  onoff = 0;

    IDL_KW_FREE;

	// Move hexapod in the current stored reference system
    int timeout_oss = 60000;
    _logger->log(Logger::LOG_LEV_TRACE, "hexbrake %d [%s:%d]", onoff, __FILE__,__LINE__);
	ret =  aos_hxpbrake(onoff, timeout_oss) ;
    _logger->log(Logger::LOG_LEV_TRACE, "errorcode %d [%s:%d]", ret, __FILE__,__LINE__);
	return IDL_GettmpLong(PLAIN_ERROR(ret));
}

// ********************************************************************************************************************* //
//
IDL_VPTR IDL_hexhome(int /* Argc */, IDL_VPTR /* Argv */[], char * /* Argk */)
{
    int                 ret;
    int timeout_oss = 120000;

	Logger* _logger = Logger::get("IDL_HEX", Logger::LOG_LEV_TRACE);
    if (IS_ERROR(ret=aosinitialize())) return IDL_GettmpLong(PLAIN_ERROR(ret));

	// Move hexapod in the current stored reference system
    _logger->log(Logger::LOG_LEV_TRACE, "hexhome [%s:%d]", __FILE__,__LINE__);
	ret =  aos_hxpinit(0,0,0,0,0,0, timeout_oss) ;
	return IDL_GettmpLong(PLAIN_ERROR(ret));
}


