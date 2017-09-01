//@File: aoslib.c
//
// Functions for communication with the AOS
// 
// This file contains routines to be used by
// an application in order to communicate with the AOS
//@

#include "base/thrdlib.h"

#include "aoslib/aoslib.h"

#include <stdio.h>
#include <string.h>
#include <ctype.h>

static char tel_side;
static int initialized=0;

static char csVarName[20] = "AOARB.x.ServStat";   // Template for service status variable
static char aosName[9]    = "magaoi.x";           // Template for AOS client name

//@Function: aos_wakeup
//
// Send a wakekup
//
// This routine sends a wakeup command to AOS
//@

static void aos_wakeup(void)
{                             //@R: Completion code
     
thSendMsgPl(0, aosName, AOS_HOUSKEEP, AOS_WAKEUP, DISCARD_FLAG, NULL); 
}



//@Function: hxp_cmd
//
// Send hexapod related commands to AOS and analyze return
//
//@

static size_t dleng=sizeof(double [6]);

static int hxp_cmd(int code,          //@P(code): Command code
                   double inargs[6],  //@P(inargs): Input arguments
                   double outargs[6], //@P(outargs): Retuned arguments
                   int timeout)       //@P(timeout): Command return timeout
                                      //              0: wait forever
                                      //             -1: return immediately
{
MsgBuf *imsg;
int stat;

if(!initialized) return NOT_INIT_ERROR;

                             // First send message
switch(code) {
case AOS_HXPGETPOS:         // These commands do not require input arguments
case AOS_HXPGETABS:
case AOS_HXPOPENBRAKE:
case AOS_HXPCLOSEBRAKE:
case AOS_HXPNEWREF:
case AOS_HXPISINITIALIZED:
case AOS_HXPISMOVING:
    stat=thSendMsgPl(0, aosName, AOS_HXP_CMD, code, NOHANDLE_FLAG, NULL);
    break;
case AOS_HXPINIT:
case AOS_HXPMOVETO:      // These commands require position specification
case AOS_HXPMOVEBY:
case AOS_HXPMOVSPH:
    stat=thSendMsgPl(dleng, aosName, AOS_HXP_CMD, code, NOHANDLE_FLAG, (void *)inargs);
    break;
default:
    stat = INVALID_ARGUMENT_ERROR;
}
if(stat<0) return stat;

imsg=thWaitMsg(ANY_MSG, aosName, stat, timeout, &stat);  // Wait for reply from AOS 

if(imsg==NULL) return stat;

code=HDR_CODE(imsg);     // Analyze return message

switch(code) {
    case AOS_ACK:
       if(outargs) memcpy(outargs,(double *)MSG_BODY(imsg),sizeof(inargs));
       stat=HDR_PLOAD(imsg);
       break;
    case AOS_NACK:
       stat=HDR_PLOAD(imsg);
       break;
    default:
       stat=UNEXPECTED_REPLY_ERROR;
}
thRelease(imsg);
return stat;
}


//@Function{API}: aos_init
//
// Checks if everything is properly initialized
//
// This routine initializes the AOS communication library.
//
// Prior of calling aos_init, any application must properly
// start the communication with the MsgD-RTDB. 
//
// If the AOS is not up and running the routine returns with COMMUNICATION_ERROR
// status. The calling program may poll by repeatedly calling aos_init()
//
// If AOS is running, The variable AOARB.x.ServStat is created
// and 0 is written into it
//
//@

int aos_init(char *side)     //@P{side}: either "L" or "R"
                             //@R: Completion code (see also description above)
{
   int zero=0;
   int ret;

   if(initialized) return ALRDY_INIT_ERROR;

   if(toupper(*side)=='L')
     tel_side='L';
   else if(toupper(*side)=='R')
     tel_side='R';
   else
     return INVALID_ARGUMENT_ERROR;

   aosName[7]=tel_side;   // Setup AOS client name
   csVarName[6]=tel_side;

   if(!aos_isup(AOS_REPLY_TIMEOUT)) return AOS_NOT_RESPONDING_ERROR;

   initialized=1;

   return NO_ERROR;
}


//@Function{API}: aos_debug
//
// Set debug level in AOS
//
// This routine changes the debug level of AOS
//
//@

int aos_debug(int level)      //@P{level}: 0..4
                              //@R: Completion code
{
int pload,stat;

if(!initialized) return NOT_INIT_ERROR;

if(level<0) level=0;
if(level>4) level=4;
switch(level) {
   case 0:
      pload=AOS_DBG_00; break;
   case 1:
      pload=AOS_DBG_01; break;
   case 2:
      pload=AOS_DBG_02; break;
   case 3:
      pload=AOS_DBG_03; break;
   case 4:
      pload=AOS_DBG_04; break;
}
stat=thSendMsgPl(0, aosName, AOS_HOUSKEEP, pload, DISCARD_FLAG, NULL); 
return stat;
}

//@Function{API}: aos_clean
//
// Cleanup AOS library
//
// This routine will clean up all the resources allocated by aos_init
//@

int aos_clean(void)  // Cleanup
{
   if(!initialized) return NOT_INIT_ERROR;

   initialized=0;

   return NO_ERROR;
}

//@Function{API}: aos_isup
//
// Check proper communication with AOS
//
// This routine checks wether the AOS is up and running.
// Checking is performed by sending an ECHO request message,
// and waiting for a reply.
//
// Note: 
//@

int aos_isup(int timeout)  //@P{timeout}: timeout (ms) to wait for AOS reply.
                           // 0 or negative: wait forever
                           //@R: true if AOS is responding
{
MsgBuf *imsg;
int stat;

if(!initialized) return NOT_INIT_ERROR;

if(timeout<=0) timeout=0;
stat=thSendMsg(0, aosName, ECHO, NOHANDLE_FLAG, NULL);   // Check if AOS is up
imsg=thWaitMsg(ECHOREPLY,aosName,stat,timeout,&stat);
if(imsg == NULL) return 0;
thRelease(imsg);
return 1;
}

//@Function{API}: aos_enable
//
// Enables/disables AOS
//
// This routine changes the status of AOS service Status. If enable the status is OPERATING,
// if disabled it is STANDALONE
//
// First the proper value is written into the connection status variable, 
// then a WAKEUP message is sent
//
//@

int aos_enable(int enable)  //@P{stat}: 1:enable, 2: disable
                             //@R: error status return
{
int _stat,ret;

if(!initialized) return NOT_INIT_ERROR;

if(enable)
   _stat=1;
else
   _stat=0;

ret = thWriteVar(csVarName, INT_VARIABLE, 1, (void *)&_stat, AOS_REPLY_TIMEOUT);
aos_wakeup();

return ret;
}

//@Function{API}: aos_log
//
// Activate/deactivate mirror logging
//
// This routine sends to AOS a request to activate or deactivate
// mirroring of log messages generated by AOS.
// 
// After activation, logging messages are sent to MsgD with the standard
// message code: LOGSTAT
//
//@

int aos_log(int on)  //@P{on}: if true activate logging, else deactivate
                     //@R: error status return
{
MsgBuf *imsg;
int code,stat;

if(!initialized) return NOT_INIT_ERROR;
if(on) 
    code=AOS_LOGON;
else
    code=AOS_LOGOFF;

stat=thSendMsgPl(0, aosName, AOS_HOUSKEEP, code, NOHANDLE_FLAG, NULL);   // Check if AOS is up
imsg=thWaitMsg(ANY_MSG,aosName,stat,AOS_REPLY_TIMEOUT,&stat);    // Wait reply
if(imsg == NULL) return stat;                     // ... and ignore it.
thRelease(imsg);
return NO_ERROR;
}

//@Function{API}: aos_offload
//
// Offload request to AOS
//
// This routines sends to AOS an offload request
// No reply from AOS is expected
//@

int aos_offload(double zernikes[OFL_ZERN_NUMB]) // @P{zernikes}: array of modal errors
                                                 //@R: completion code
{
int stat;
int lng=sizeof(double)*OFL_ZERN_NUMB;

if(!initialized) return NOT_INIT_ERROR;

stat=thSendMsgPl(lng, aosName, AOS_OFFLOAD,0, 0, zernikes);
return stat;

}


//@Function{API}: aos_telemetry
//
// Request to store a telemetry record
//
// This routines sends to AOS a request to store a telemetry record
// into the TCS telemetry system.
//
// No reply from AOS is expected
//@

int aos_telemetry(void *record) // @P{record}: pointer to telemetry data
                                // Telemetry data structures are defined in aoslib.h
                                // Each structure must have the rtype field initialized 
                                // whith the corresponding data type
{
int stat=NO_ERROR;
int lng=0;
int rtype=((gen_tel_struct *)record)->rtype;

if(!initialized) return NOT_INIT_ERROR;

switch (rtype) {
   case AOS_TEL_ADSEC1:
      lng=sizeof(aos_tel_adsec1);
      break;
   case AOS_TEL_ADSEC2:
      lng=sizeof(aos_tel_adsec2);
      break;
   case AOS_TEL_WFS1:
      lng=sizeof(aos_tel_wfs1);
      break;
   case AOS_TEL_WFS2:
      lng=sizeof(aos_tel_wfs2);
      break;
   default:
      stat=AOS_TELTYPE_ERROR;
}
if(IS_NOT_ERROR(stat)) stat=thSendMsgPl(lng, aosName, AOS_TELEMETRY,0, 0, record);
return stat;
}

//@Function{API}: aos_flat_list
//
// Sends a flat list update command
//
// This routines sends to AOS a a flat list update command.
// AOS will rewrite the Flat list file
// No reply from AOS is expected
//@

int aos_flatlist(char *list) // @P{list}: List of flat names (semicolon separated)
                              // @R: completion code
{
int stat;
int lng=strlen(list)+1;

if(!initialized) return NOT_INIT_ERROR;

stat=thSendMsgPl(lng, aosName, AOS_HOUSKEEP,AOS_FLAT_LIST, DISCARD_FLAG, list);
return stat;

}



//@Function{API}: aos_hxpinit
//
// Initialize hexapod
//
// This routines sends to AOS the command to move the hexapode in 
// given absolute position, then assumes this position as initial reference system
//@

int aos_hxpinit(double eX,     // @P{eX}: origin coordinate in micron
                double eY,     // @P{eY}: origin coordinate in micron
                double eZ,     // @P{eZ}: origin coordinate in micron
                double eA,     // @P{eA}: axis rotation in arcsec
                double eB,     // @P{eB}: axis rotation in arcsec
                double eG,     // @P{eG}: axis rotation in arcsec
                int timeout)   //@P(timeout): Command return timeout
                               //@R: completion code
{
int stat;
double aux[6];

aux[0]=eX; aux[1]=eY; aux[2]=eZ; aux[3]=eA; aux[4]=eB; aux[5]=eG;
stat=hxp_cmd(AOS_HXPINIT,aux, NULL,timeout);
return stat;
}

//@Function{API}: aos_hxpmoveto
//
// Move hexapod to given position
//
// This routines sends to AOS the command to move the hexapode in 
// given position, with respect to current reference system
//@

int aos_hxpmoveto(double eX,     // @P{eX}: point coordinate in micron
                  double eY,     // @P{eY}: point coordinate in micron
                  double eZ,     // @P{eZ}: point coordinate in micron
                  double eA,     // @P{eA}: axis rotation in arcsec
                  double eB,     // @P{eB}: axis rotation in arcsec
                  double eG,     // @P{eG}: axis rotation in arcsec
                  int timeout)   //@P(timeout): Command return timeout
                                 //@R: completion code
{
int stat;
double aux[6];

aux[0]=eX; aux[1]=eY; aux[2]=eZ; aux[3]=eA; aux[4]=eB; aux[5]=eG;
stat=hxp_cmd(AOS_HXPMOVETO,aux,NULL,timeout);
return stat;
}

//@Function{API}: aos_hxpmoveby
//
// Move hexapod to given position (delta displacement)
//
// This routines sends to AOS the command to move the hexapode in 
// given position, with respect to current reference system. The
// new position is defined as displacements with respect to current.
//@

int aos_hxpmoveby(double dX,     // @P{dX}: point displacement in micron
                  double dY,     // @P{dY}: point displacement in micron
                  double dZ,     // @P{dZ}: point displacement in micron
                  double dA,     // @P{dA}: angular displacement in arcsec
                  double dB,     // @P{dB}: angular displacement in arcsec
                  double dG,     // @P{dG}: angular displacement in arcsec
                  int timeout)   //@P(timeout): Command return timeout
                                 //@R: completion code
{
int stat;
double aux[6];

aux[0]=dX; aux[1]=dY; aux[2]=dZ; aux[3]=dA; aux[4]=dB; aux[5]=dG;
stat=hxp_cmd(AOS_HXPMOVEBY,aux,NULL,timeout);
return stat;
}

//@Function{API}: aos_hxpmovsph
//
// Move hexapod to given position (on a spherical surface)
//
// This routines sends to AOS the command to move the hexapode in 
// given position, on a spherical with respect to current reference system.
//@

int aos_hxpmovsph(double R,      // @P{R}: radius of sphere (along the Z-axis)
                  double eA,     // @P{eA}: angular displacement in arcsec
                  double eB,     // @P{eB}: angular displacement in arcsec
                  int timeout)   //@P(timeout): Command return timeout
                                 //@R: completion code
{
int stat;
double aux[6];

aux[0]=R; aux[1]=eA; aux[2]=eB; aux[3]=0; aux[4]=0; aux[5]=0;
stat=hxp_cmd(AOS_HXPMOVSPH,aux,NULL,timeout);
return stat;
}


//@Function{API}: aos_hxpnewref
//
// Assume current position as new reference system
//
// This routines change the hexapod reference system assuming
// the current position (and angles) as origin.
//@

int aos_hxpnewref(int timeout)   //@P(timeout): Command return timeout
                                 //@R: completion code
{
int stat;

stat=hxp_cmd(AOS_HXPNEWREF,NULL,NULL,timeout);
return stat;
}

//@Function{API}: aos_hxpgetpos
//
// Returns current position
//
// This routines returns a pointer to a six element vector
// containing the current position of the hexapod with respect
// of the current reference system
//
//@

double *aos_hxpgetpos(int timeout) //@P(timeout): Command return timeout
                                   // @R: pointer to position vector (order is:
                                   // (X,Y,Z,A,B,G)
                                   // On error the function returns NULL;
{
int stat;
static double aux[6];

stat=hxp_cmd(AOS_HXPGETPOS,NULL,aux,timeout);
if(stat==NO_ERROR) return aux;
return NULL;
}

//@Function{API}: aos_hxpgetabs
//
// Returns current absolute position
//
// This routines returns a pointer to a six element vector
// containing the current position of the hexapod with respect
// to the absolute reference system
//
//@

double *aos_hxpgetabs(int timeout) //@P(timeout): Command return timeout
                                   // @R: pointer to position vector (order is:
                                   // (X,Y,Z,A,B,G)
                                   // On error the function returns NULL;
{
int stat;
static double aux[6];

stat=hxp_cmd(AOS_HXPGETABS,NULL,aux,timeout);
if(stat==NO_ERROR) return aux;
return NULL;
}

//@Function{API}: aos_hxpbrake
//
// Operate hexapod brake
//
// This routines sends AOS a command to operate the hexapod
// brake.
//
//@

int aos_hxpbrake(int onoff,    //@P{onoff): 1, close brake, 0: open brake
                 int timeout)  //@P(timeout): Command return timeout
                               //@R: completion code
{
int stat;

switch(onoff) {
case 0:
    stat=hxp_cmd(AOS_HXPOPENBRAKE,NULL,NULL,timeout);
    break;
case 1:
    stat=hxp_cmd(AOS_HXPCLOSEBRAKE,NULL,NULL,timeout);
    break;
default:
    stat = INVALID_ARGUMENT_ERROR;
}
return stat;
}

//@Function{API}: aos_hxpisinitialized
//
// Check hexapod initialization status
//
// Returns 1 if the hexapod has been initialized
// 0 if hexapod has not been initialized
// negative error code on error
//
//@

int aos_hxpisinitialized(int timeout) //@P(timeout): Command return timeout
                                      // @R: 1 if the hexapod has been initialized
                                      // 0 if hexapod has not been initialized
                                      // On error the function returns a negative error code;
{
return hxp_cmd(AOS_HXPISINITIALIZED,NULL,NULL,timeout);
}

//@Function{API}: aos_hxpismoving
//
// Check if hexapod is moving
//
// Returns 1 if the hexapod is moving
// 0 if hexapod is not moving
// negative error code on error
//
//@

int aos_hxpismoving(int timeout) //@P(timeout): Command return timeout 
                                 // @R: 1 if the hexapod is moving
                                 // 0 if hexapod is not moving 
                                 // On error the function returns a negative error code;
{
return hxp_cmd(AOS_HXPISMOVING,NULL,NULL,timeout);
}


#include "aoscodes.c"

//@Function{API}: aostxt_c
//
// Return explanation given item code from a DbTable
//
// This routine allows exploration of a DbTable list (used for message
// codes and for error codes).  It returns the long item explanation,
// given the item code.
// @

DbTable*  aostxt_c(int code) //@P{code}: Message code to explain
                            //@R: DbTable item (NULL if not found)
{
    DbTable* pt = NULL;
    int i;

    for(i=0;i<aoscodesTabLen;i++) {
       if(aoscodesTable[i].code == code) {
          pt=aoscodesTable+i;
          break;
       }
    }
    return pt;
}

//@Function{API}: aostxt_i
//
// Return explanation given item index from a DbTable
//
// This routine allows exploration of a DbTable list (used for message
// codes and for error codes).  It returns the item explanation,
// given the item index. Indexes start at 0.
//@

DbTable*  aostxt_i(int id) //@P{id}: Message index 
                           //@R: pointer to DbTable (NULL for invalid index)
{
    DbTable *pt = NULL;

    if(id<0 || id>=aoscodesTabLen) return pt;
    
    pt=aoscodesTable+id;
    return pt;
}

//@Function{API}: aos_vers
//
// Return version number of current library
//
// This functions returns a pointer to a statically allocated string containing
// the current version of the library  itself.
//@

char *aos_vers(void)
{
   static char chbuf[10];
   snprintf(chbuf,10,"%d.%d",AOSLIB_VMAJOR,AOSLIB_VMINOR);
   return chbuf;
}

