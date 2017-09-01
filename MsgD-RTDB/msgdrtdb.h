
#ifndef MSGDRTDB_H
#define MSGDRTDB_H

// Version 9.5  implements remote notification of variable change
// Version 10.1 implements better cleanup of notification tables
//              Also added the -N switch
// Version 10.2 Added set client ready for connected peers and
//              Numerical version retrieval
// Version 10.3 Fixed bug in variable notification to remote domains
// Version 10.4 Restored client variable notification removed in vers 10.3
// Version 10.5 Changed configuration file search order
// Version 10.6 Added LOGFREEZE command (FOR DEBUG PURPOSES ONLY!!!)
// Version 10.7 MsgD start log properly marked. Fixed bug in reading config.
// Version 10.8 Fixed bug in variable notification removal when peer disconnects
// Version 11.1 Includes initialization of RTDB variables from config file
// Version 11.2 Added more info to Message 43 conversion log
// Version 11.3 Added Sequence number to var notif. messages
// Version 11.4 Solved possible timing inversion in log output
// Version 11.5 Added support for get client's start time request

#define VERS_MAJOR  11
#define VERS_MINOR  5

// ---------------------------------------------------------------------
// Configuration parameters
// The following symbols provide defaults for configuration parameters.

#define LOGPATH      "."      // Path for logfile
#define LOGLINES     1000000  // Maximum number of lines in log file. 
#define AUTODUMP     300      // Time interval (sec) between info dump
                              // (Use 0 to disable)
#define MAX_RET_MSGS 1000     // Max number of messages returned in reply
                              // to CLNTLIST, VARLIST, BUFLIST
// ---------------------------------------------------------------------

#define PEER_REQ_TMOUT  500000  // Timeout for peer connection request.
                                // in microseconds < 1000000

#define HOUSEKEEPSTEP   1000    // Housekeep step time (millisec)

#define MAX_FNAME 2048

#define MAX_PEER  8

#include <sys/time.h>
#include <time.h>

#include "base/msglib.h"

 struct PGINFO {
    char MyName[PROC_NAME_LEN+1];    // Process name
    char cfgName[MAX_FNAME];         // Configuration file name
    char logPath[MAX_FNAME];         // Log file base name
    int maxRetMsgs;                  // Max number of replies to any client
    int netIoTmout;                  // Timeout for messages sent to clients (usec)
    struct timeval startTime;        // Start time
    int   MainPID;                   // PID of main thread
    int   lPort;                     // Listening port
    int   peerOK;                    // Can connect to peers
    unsigned msgMagic;               // Msg header version
};

#endif
