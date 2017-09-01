//@ File: common.h

// Global dimensioning parameters for the AO Supervisor

// This file contains definitions of global dimensioning parameters common
// to all source files. It must be included by any source file
// using \msgl,  but there is no need to include it explicitly because it
// is included in {\tt msglib.h}.
//@

#ifndef COMMON_H
#define COMMON_H


// Special compilation flags for debugging. Usually shound be undefined
//#define DBG_HEX_DUMP     // Enable hex dump of message header

#define MSG_HEADER_MAGIC   44          // Message header version. This number must be increased
                                       // whenever a modification is made to Message header.

#define PROC_NAME_LEN     19           // Length of From/To field (Vers. 43 header)
#define LONG_NAME_LEN     39           // Length of From/To field (this changes Message Header!!)

#define VAR_NAME_LEN      79           // Variable name max length

#define MAX_MSG_LEN     3999999        // maximum message body length

#define MAX_SHM_BLOCKS    20    // Max number of total shareable buffers

#define MAX_CONSUMERS     15    // Max number of consumers for a shared buffer
                                // NOTE: After changing this value, semaphore 
                                //       initializers must also be updated in 
                                //       buflib.h

#define DEFAULT_TTL        5    // Default time to live (sec) for messages in queue (see thrdlib)


typedef struct {
   int   code;
   char* name;
   char* descr;
} DbTable;

#define BUILD_TIME_LNG    40
#define VERSION_ID_LNG    80

char * build_time(void);

#endif
