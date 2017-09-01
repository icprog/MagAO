//@File: errlib.h
//
// Include file for errlib.c
//@


#ifndef ERROR_H_INCLUDED
#define ERROR_H_INCLUDED



                                // Log levels used throughout the  Supervisor Software
                                // Always and Disabled are DEPRECATED
#define MASTER_LOG_LEV_ALWAYS       1
#define MASTER_LOG_SYM_ALWAYS       "ALW"
#define MASTER_LOG_LEV_DISABLED     2
#define MASTER_LOG_SYM_DISABLED     "DIS"
#define MASTER_LOG_LEV_FATAL        3
#define MASTER_LOG_SYM_FATAL        "FAT"
#define MASTER_LOG_LEV_ERROR        4
#define MASTER_LOG_SYM_ERROR        "ERR"
#define MASTER_LOG_LEV_WARNING      5
#define MASTER_LOG_SYM_WARNING      "WAR"
#define MASTER_LOG_LEV_INFO         6
#define MASTER_LOG_SYM_INFO         "INF"
#define MASTER_LOG_LEV_DEBUG        7
#define MASTER_LOG_SYM_DEBUG        "DEB"
#define MASTER_LOG_LEV_TRACE        8
#define MASTER_LOG_SYM_TRACE        "TRA"
#define MASTER_LOG_LEV_VTRACE       9
#define MASTER_LOG_SYM_VTRACE       "VTR"
#define MASTER_LOG_LEV_VVTRACE      10
#define MASTER_LOG_SYM_VVTRACE      "VVT"

// Log messages common header
//                         Proc.    Seq.    
//                         name R/L Num. Time Level
#define MASTER_LOG_HEADER "%-19s|%3s|%10d|%26s| "
//
// Length of log string (including trailing NUL)
#define MASTER_LOG_HEADER_LEN   66      


// Note:

// The numerical error codes are modified by negating two bits, as defined
// by the following masks:

#define FATAL_ERROR_FLG   0x20000    // Fatal error flag

#define SYSTEM_ERROR_FLG  0x10000    // System error flag

// The bits will be negated to indicate the specified conditions. Thus
// The actual error value in decimal notation will be as in the following
// Example:

//  Plain
//  Error   System   Fatal   Fatal+System

// -1000    -17384  -33768    -50152
// -1001    -17385  -33769    -50153


// The following macros have been defined to manipulate errors:

// Check error flags. True if flag is set

#define IS_FATAL_ERROR(x)  (((x)<0) && ((~x)&FATAL_ERROR_FLG))
#define IS_SYSTEM_ERROR(x) (((x)<0) && ((~x)&SYSTEM_ERROR_FLG))


//                       Restore error numerical value (assert both flags)
#define PLAIN_ERROR(x)   (((x)<0)?((x)|FATAL_ERROR_FLG|SYSTEM_ERROR_FLG):(x))
#define CLEAN_ERROR(x)   (((x)<0)?((x)|FATAL_ERROR_FLG|SYSTEM_ERROR_FLG):(x))

//                       Set flag for FATAL error
#define FATAL_ERROR(x)   (((x)<0)?((x)&(~FATAL_ERROR_FLG)):(x))

//                       Set flag for SYSTEM error
#define SYSTEM_ERROR(x)  (((x)<0)?((x)&(~SYSTEM_ERROR_FLG)):(x))

//                       Set both FATAL and SYSTEM flags
#define ALL_ERROR(x)     (((x)<0)?((x)&(~(FATAL_ERROR_FLG|SYSTEM_ERROR_FLG))):(x))

//                       UnSet flag for SYSTEM error
#define NO_FATAL_ERROR(x)     (((x)<0)?((x)|FATAL_ERROR_FLG):(x))

//                       UnSet flag for SYSTEM error
#define NO_SYSTEM_ERROR(x)  (((x)<0)?((x)|SYSTEM_ERROR_FLG):(x))

//			Quick check for error in a return code
#define CHECK_SUCCESS(a)                if(a<0) return a;
#define CHECK_SUCCESS_NORETURN(a)       if(a<0) return;
#define CHECK_SUCCESS_SENDBCU(a)        if(a<0) { error = stat; break; }


//                     Error check macros

#define IS_ERROR(x) ((x)<0)
#define IS_NOT_ERROR(x) ((x)>=0)

// Log levels

#define LOG_LEVEL1              (0x01)
#define LOG_LEVEL2              (0x02)
#define LOG_LEVEL3              (0x03)


#define LOG_MAX  1024    // Max lenght of an error or log message

// Special structure for LOGSTAT message:

typedef struct  {
        int lev;
        int seq;
        char txt[LOG_MAX];
    } str_LOGSTAT;



#include "base/common.h"
#include "base/errordb.h"



//********************************************
// Prototypes
//


int lao_errno(void);

void lao_seterr(int errcode);

const char *lao_strtable(int code, DbTable *table, int tableLen);

const char* lao_strerror(int errcode);

DbTable* lao_strsrc(char * match, DbTable *table, int tableLen);

DbTable* lao_errinfo(int errcode);
DbTable* lao_errsrc(char * match);
DbTable *lao_scan(int ix, DbTable *table, int tabLen);
DbTable *lao_errscan(int ix);

#endif // ERROR_H_INCLUDED

