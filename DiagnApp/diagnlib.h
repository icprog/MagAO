//@File: diagnlib.h
//
//@

#ifndef DIAGNLIB_H_INCLUDED
#define DIAGNLIB_H_INCLUDED

#include "DiagnWhich.h"
#include "DiagnParam.h"
#include "AOExcept.h"

using namespace Arcetri;



#include <map>
ParamDict  GetDiagnVarParam(
        char*              diagnapp,  //@P{diagnapp}: DiagnApp's name
        const DiagnWhich & which,     //@P{which}: DiagnVar's filter.  
        int                tmout=0    //@P{tmout}: Timeout in milliseconds, to wait for reply. 
                                      // If 0, wait forever.
        );                            //@R: ParamDict containing the DiagnParam object for each DiagnVar matched in which.


int        SetDiagnVarParam(
        char*               diagnapp, //@P{diagnapp}: DiagnApp's name
        const ParamDict  &  pdict,    //@P{pdict}: ParamDict containing the DiagnParam object for each DiagnVar
        int                 tmout=-1  //@P{tmout}: Timeout in milliseconds, to wait for acknowledgement. 
                                      // If 0, wait forever. If -1, do not request acknowledgement.   
        );                            // @R: Number of items modified if tmout >= 0
                                      //     NO_ERROR if tmout < 0


int        SetDiagnVarParam(
        char*               diagnapp, //@P{diagnapp}: DiagnApp's name
        const DiagnWhich &  which,    //@P{which}: DiagnVar's filter.  
        const DiagnParam &  param,    //@P{param}: Parameter object
        int                 tmout=-1  //@P{tmout}: Timeout in milliseconds, to wait for acknowledgement. 
                                      // If 0, wait forever. If -1, do not request acknowledgement.   
        );                            // @R: Number of items modified if tmout >= 0
                                      //     NO_ERROR if tmout < 0


ValueDict  GetDiagnValue(
        char*               diagnapp, //@P{diagnapp}: DiagnApp's name
        const DiagnWhich &  which,    //@P{which}: DiagnVar's filter.
        int                 tmout=-1  //@P{tmout}: Timeout in milliseconds, to wait for acknowledgement. 
                                      // If 0, wait forever. 
        );                            // @R:  ValueDict containing the DiagnValue object for each DiagnVar matched in which.

BufferDict  GetDiagnBuffer(
        char*               diagnapp, //@P{diagnapp}: DiagnApp's name
        const DiagnWhich &  which,    //@P{which}: DiagnVar's filter.
        int                 tmout=-1  //@P{tmout}: Timeout in milliseconds, to wait for acknowledgement. 
                                      // If 0, wait forever. 
        );                            // @R:  BufferDict containing the DiagnBuffer object for each DiagnVar matched in which.

int        DumpDiagnVarParams(
        char*               diagnapp, //@P{diagnapp}: DiagnApp's name
        char*               filename, //@P{filename}: filename
        int                 tmout=-1  //@P{tmout}: Timeout in milliseconds, to wait for acknowledgement. 
                                      // If 0, wait forever. If -1, do not request acknowledgement.   
        );                            // @R: Number of items modified if tmout >= 0
                                      //     NO_ERROR if tmout < 0

int        LoadDiagnVarParams(
        char*               diagnapp,  //@P{diagnapp}: DiagnApp's name
        char*               filename,  //@P{filename}: filename.
        int                 tmout=-1   //@P{tmout}: Timeout in milliseconds
        );                              


int 		LoadOfflineDiagnFrame(
		char* 				diagnapp, //@P{diagnapp}: DiagnApp's name
		uint32 				frameI    //@P{frameI}: number of the frame to load
		);

#endif


