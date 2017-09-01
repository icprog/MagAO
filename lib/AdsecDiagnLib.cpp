//@File: AdsecDiagnLib.cpp
//
// Implementation of the AdsecDiagnLib interface functions
//@

#include <cstdio>
#include <cstring>
#include <sstream>
#include <errno.h>


extern "C" {
#include "base/thrdlib.h"
#include "iolib.h" //ReadBinaryFile
}

#include "AdsecDiagnLib.h"
#include "Logger.h"


//@Member: AttachRawShMem
//
//This is part of the post initialization phase.
//@

void attachToFastDiagnostic( string MyName, string bufname, BufInfo **info, int bufLen) throw (AOShMemException)
{
	static Logger* logger = Logger::get("GET_RAW_DATA");
    logger->log(Logger::LOG_LEV_TRACE, "Entering attachToFastDiagnostic() [%s:%d]", __FILE__, __LINE__);
    try {
		   // Non guarda in faccia a nessuno e si stacca senza neanche sapere se era attaccata
		  bufRelease( (char*)MyName.c_str(), *info);
        *info = new BufInfo();
        TryAttachRawShMem( MyName, bufname, *info, bufLen);
    } catch (AOShMemException& e) {
        logger->log(Logger::LOG_LEV_INFO, "Attaching to raw shared memory failed ... is MasterDiagnostic up? [%s:%d]", __FILE__, __LINE__);
		  bufRelease( (char*)MyName.c_str(), *info);
        delete *info;
        *info = NULL;
        if (e._errcode == MEMORY_ALLOC_ERROR || e._errcode == BUF_MODE_ERROR) throw(e);
        msleep(1000); // pass here if bufRequest failed. MasterDiagnostic is down? Wait a little bit
    }
}

//@Member: TryAttachRawShMem
//
//@
void TryAttachRawShMem( string MyName, string bufname, BufInfo *info, int bufLen) throw (AOShMemException)
{
	static Logger* logger = Logger::get("GET_RAW_DATA");

    //int   crate_n;
    int   stat;
    // ------------ Connect to raw shared memories --------------------------------------
    // bufRequest
    if( IS_ERROR(stat=bufRequest(
                    (char*)MyName.c_str(),
                    (char*)bufname.c_str(),
                    BUFCONT,
                    info) ) )
    {
        logger->log(Logger::LOG_LEV_ERROR, "Attaching to shared buffer: %s - %s - Perror(%s) [%s:%d]",
                bufname.c_str(), lao_strerror(stat), strerror(errno), __FILE__, __LINE__ );
        throw AOShMemException("Error attaching to shmem", *info, PLAIN_ERROR(stat), __FILE__, __LINE__);
    }
    // Check shared buffers are continous mode
    if( (info->com.accMode & ACCMODE_MODE_MASK)!=BUFCONT ) {
        logger->log(Logger::LOG_LEV_ERROR, "Attached buffer %s:%s is not continous mode [%s:%d]",
            info->com.producer, *info->com.name,__FILE__,__LINE__);
        throw AOShMemException("Attached buffer is not continous mode", *info, BUF_MODE_ERROR, __FILE__, __LINE__);
    }
    // Check  shared buffers are same length than sdram_diagn_struct
    if( (unsigned)info->com.lng != (unsigned)bufLen) {
        logger->log(Logger::LOG_LEV_ERROR, "Attached buffer %s:%s is %d bytes long instead of %d bytes [%s:%d]",
            info->com.producer, info->com.name, info->com.lng, bufLen, __FILE__, __LINE__ );
        throw AOShMemException("Attached buffer is wrong size", *info, MEMORY_ALLOC_ERROR, __FILE__, __LINE__);
    }
}


//@Member: getFastDiagnosticRawData
//
// Use the loop flag to wait indefinitely for a frame.
// If you want control over the return value, loop=false will to one try and exit returning the status code. Use the info parameter to keep track...
//@

int getFastDiagnosticRawData( string MyName, string bufname, AdsecDiagnRaw *rawData, int timeout, bool loop, BufInfo **info)
{
	static Logger* logger = Logger::get("GET_RAW_DATA");
   BufInfo *myInfo = NULL;

    int stat;
    if (!info) {
       info = &myInfo;
    }

    do {
		logger->log(Logger::LOG_LEV_TRACE, "Calling bufRead with timeout %d [%s:%d]", timeout, __FILE__, __LINE__);
		stat=bufRead(*info, rawData, timeout); // Safe even with NULLs...

		if (IS_ERROR(stat)) {
			if (stat != TIMEOUT_ERROR) {
				logger->log(Logger::LOG_LEV_WARNING, "Error reading shared buffer: %s - trying to reattach [%s:%d]",
						lao_strerror(stat),  __FILE__, __LINE__);
				attachToFastDiagnostic( MyName, bufname, info);
			} else {
				logger->log(Logger::LOG_LEV_DEBUG, "bufRead returned timeout: [%s:%d]", __FILE__, __LINE__);
			}
		}
		else {
			 loop = false;
		}
   }
   while (loop);

   return stat;
}



//@Member: getOptLoopRawData
//
// Use the loop flag to wait indefinitely for a frame.
// If you want control over the return value, loop=false will to one try and exit returning the status code. Use the info parameter to keep track...
//@

int getOptLoopRawData( string MyName, string bufname, OptLoopDiagFrame *rawData, int timeout, bool loop, BufInfo **info)
{
	static Logger* logger = Logger::get("GET_RAW_DATA");
   BufInfo *myInfo = NULL;

    int stat;
    if (!info) {
       info = &myInfo;
    }

    do {
		logger->log(Logger::LOG_LEV_TRACE, "Calling bufRead with timeout %d [%s:%d]", timeout, __FILE__, __LINE__);
		stat=bufRead(*info, rawData, timeout); // Safe even with NULLs...

		if (IS_ERROR(stat)) {
			if (stat != TIMEOUT_ERROR) {
				logger->log(Logger::LOG_LEV_WARNING, "Error reading shared buffer: %s - trying to reattach [%s:%d]",
						lao_strerror(stat),  __FILE__, __LINE__);
				attachToFastDiagnostic( MyName, bufname, info, sizeof(OptLoopDiagFrame));
			} else {
				logger->log(Logger::LOG_LEV_DEBUG, "bufRead returned timeout: [%s:%d]", __FILE__, __LINE__);
			}
		}
		else {
			 loop = false;
		}
   }
   while (loop);

   return stat;
}



