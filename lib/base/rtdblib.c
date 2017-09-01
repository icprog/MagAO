//@File: rtdblib.c
//
// Real time data base management routines
//
// This file contains utilities to manipulate the Real-Time database
//
//@

///////////////////////////////////////////////////////////////////////
//

#include <string.h>
#include <sys/time.h>

#define LIBRARY_DEVELOPER 1

#include "base/msglib.h"
#include "base/errlib.h"

#include "base/rtdblib.h"

#if LONG_MAX > 2147483647
#define LONG_IS_64
//This means we're on a 64 bit long machine (e.g. x86_64)
//But msglib assumes 32 bit longs if it is on a 32 bit machine (e.g. i386)
//We must deal with timeval, long, and unsigned long
#endif

static int Debug=0;

//@Function: rmVDomain
//
// Removes domain from variable name (Internal use)
//
// This function returns a variable name into the give buffer.
// And a pointer to the domain suffix.
//@ 

char *rmVDomain(const char *name,  // @P{name}: variable full name
                char      *vnbuf)  // @P{vnbuf}: work buffer it must be
                                   // @R: pointer to domain or NULL
                                   // at least VAR_NAME+LEN+1 in length
{
   char *dom=strchr(name,'@');
   int nc;

   if(dom) {
      nc=dom-name;
      dom++;
   } else
      nc=VAR_NAME_LEN;
   strncpy(vnbuf,name,nc);
   vnbuf[nc]='\0';

   return dom;
}


//@Function[API]: VarSize
//
// Returns the size of a variable
//
// This function returns the minimum size of a buffer which must contain
// a variable of given type and length.
// 
// Note: a value 0 for the number of elements is legal and is used to
//       represent variables with undefined value
//@

int VarSize(int VarType,   //@P{VarType}: Variable type
	    int NElem)     //@P{NElem}: Number of elements in array
                           //@R: length of variable or negative completion code
{
    int size;

    switch(VarType) {
	case REAL_VARIABLE:
	    size = sizeof(double)*NElem;
	    break;
	case INT_VARIABLE:
		#ifdef LONG_IS_64
		size = sizeof(int)*NElem; //needs to be int
		#else
	    size = sizeof(long)*NElem;
		#endif
	    break;
	case CHAR_VARIABLE:
	    size = sizeof(unsigned char)*NElem+1;  // Char variables include 
                                                   // the trailing '\0'
	    break;
	case BIT8_VARIABLE:
	    size = sizeof(unsigned char)*NElem;
	    break;
	case BIT16_VARIABLE:
	    size = sizeof(unsigned short)*NElem;
	    break;
	case BIT32_VARIABLE:
		#ifdef LONG_IS_64
	    size = sizeof(unsigned int)*NElem;
		#else
		 size = sizeof(unsigned long)*NElem;
		#endif
	    break;
	case BIT64_VARIABLE:
	    size = sizeof(unsigned long long)*NElem;
	    break;
	default:
	    size=PLAIN_ERROR(VAR_TYPE_ERROR);
    }
    return size;
}


//@Function[API]: CreaVar
//
// Send to MsgD-RTDB a request to create a variable
//
// This routine sends to the Real Time database process  a request to
// create a new variable.
//
// Max length of variable name is defined by VAR_NAME_LEN in file common.h.

// The namespace is global and flat, i.e.: variables names are shared among
// all the clients. Each client must give proper names to variables to ensure
// that they are unique.
//
// The routine doesn't perform any check on the result of variable creation;
// the calling program must do an explicit check by calling  VarInfo()
//@

int CreaVar(char *Client,        //@P{Client}: Client's name
	         char *VarName,       //@P{VarName}: Variable name
	         int   VarType,       //@P{VarType}: Variable type (as defined in rtdblib.h)
	         char *VarMode,       //@P{VarMode}:  Obsolete: currently unused
	         int  NElements,      //@P{NElements}: Variable length (number of elements)
	         MsgBuf *imsgb)       //@P{imsgb}: Pointer to a suitable message structure
                                 // If NULL the structure is created locally. You
                                 // may wanto to specify an MsgBuf structure of
                                 // your to avoid the overhead of creating and
                                 // destroying it at each call.
                                 // @R: completion code
{
    MsgBuf lmsgb;
    MsgBuf *msgb;
    Variable theVar;
    char *clname;
    int stat,msglen;
    char destbuf[PROC_NAME_LEN+1];
    char *dest, *dpt;

    if((dpt=strchr(VarName,'@')))
       dest=mkFullName("",++dpt,destbuf);
    else
       dest="";

    stat=strlen(VarName);

    if((stat>VAR_NAME_LEN) || (stat==0))
	     return PLAIN_ERROR(VAR_NAME_ERROR);

    clname=Client;

    msglen=sizeof(VHeader);

    if(imsgb==NULL) {                     // If no buffer provided
        stat=InitMsgBuf(&lmsgb,msglen);   // create it.
        if(IS_ERROR(stat)) return stat;
	     msgb=&lmsgb;
    } else
	     msgb=imsgb;

    theVar.H.Type=VarType;
    theVar.H.NItems=NElements;

    memset(&theVar.H.Owner,0,sizeof(theVar.H.Owner));
    strcpy(theVar.H.Owner,clname);
    memset(&theVar.H.MTime,0,sizeof(theVar.H.MTime));
    strcpy(theVar.H.Name,VarName);

    stat=FillMessage(msglen,0,Client,dest,CREAVAR,(void *)&theVar,msgb);

    if(IS_NOT_ERROR(stat)) {
        stat=SendToServer(msgb);
    }
    if(imsgb==NULL) FreeMsgBuf(msgb);
    return stat;
}

//@Function[API]: DelVar
//
// Send to MsgD-RTDB a request to delete a variable
//
// This routine sends to the Real Time database process  a request to
// delete a variable
//
// The routine doesn't perform any check on the result. The RTDB could
// refuse to delete the variable (e.g.: because the requesting client is not
// the owner).

// When a variable is actually deleted, clients which are  registered for notification
// will receive proper messages.
//@

int DelVar(char *Client,   //@P{Client}: Client's name
           char *VarName,  //@P{VarName}: Variable name
           int tmout,      //@P{tmout}: Timeout in milliseconds. If 0, wait
                           // forever.
           MsgProc callbk, //@P{callbk}: function to call when receiving 
                           // unexpected messages from MsgD-RTDB. If NULL,
                           // unexpected messages are ignored.
	        MsgBuf *imsgb) //@P{imsgb}: Pointer to a suitable message structure
                           // If NULL the structure is created locally. You
                           // may wanto to specify an MsgBuf structure of
                           // your to avoid the overhead of creating and
                           // destroying it at each call.
                           // @R: completion code
{
    MsgBuf lmsgb;
    MsgBuf *msgb;
    int stat,msglen,sqn;
    char destbuf[PROC_NAME_LEN+1];
    char *dest, *dpt;

    if((dpt=strchr(VarName,'@')))
       dest=mkFullName("",++dpt,destbuf);
    else
       dest="";

    msglen=strlen(VarName)+1;

    if((msglen>VAR_NAME_LEN) || (msglen==0))
	    return PLAIN_ERROR(VAR_NAME_ERROR);

    if(imsgb==NULL) {                     // If no buffer provided
        stat=InitMsgBuf(&lmsgb,msglen);   // create it.
        if(IS_ERROR(stat)) return stat;
	    msgb=&lmsgb;
    } else 
	    msgb=imsgb;

    sqn=NewSeqN();
    stat=FillMessage(msglen,sqn,Client,dest,DELVAR,(void *)VarName,msgb);

    if(IS_NOT_ERROR(stat)) {
        stat=SendToServer(msgb);
    }
    if(IS_NOT_ERROR(stat)) stat = WaitMessageSet(&sqn,1,msgb,callbk,tmout);
    if(IS_NOT_ERROR(stat)) {
        if(HDR_CODE(msgb)!=VARCHANGD)  {
            if(HDR_CODE(msgb)==NAK) 
                stat = NAK_RECEIVED;
            else
                stat = UNEXPECTED_REPLY_ERROR;
        }
    }
    if(imsgb==NULL) FreeMsgBuf(msgb);
    return stat;
}

//@Function[API]: GetVar
//
// Send request to get a variable from the Real-Time Database
//
// This routine sends a request to get the value of a variable from the 
// real time database. The calling process must properly manage the
// reply sent back by the MsgDRTDB process
//@

int GetVar(char *Client,   //@P{Client}: client's name
	        char *VarName,  //@P{VarName}: Variable name
	        int SeqNumber,  //@P{SeqNumber}: Message sequence number. This argument
	                        // has no effect: it is replicated in the
	                        // reply message
	        MsgBuf *imsgb)  //@P{msgb}: Pointer to a suitable message structure
                           // If NULL the structure is created locally. You
                           // may wanto to specify an MsgBuf structure of
                           // your to avoid the overhead of creating and
                           // destroying it at each call. 
                           // @R: completion code
{
    int stat,sqn;
    MsgBuf lmsgb;
    MsgBuf *msgb;
    int msglen;
    char destbuf[PROC_NAME_LEN+1];
    char *dest, *dpt;

    if((dpt=strchr(VarName,'@')))
       dest=mkFullName("",++dpt,destbuf);
    else
       dest="";

    msglen=strlen(VarName)+1;

    if(imsgb==NULL) {                     // If no buffer provided
        stat=InitMsgBuf(&lmsgb,msglen);   // create it.
        if(IS_ERROR(stat)) return stat;
	     msgb=&lmsgb;
        sqn=0;
    } else 
	     msgb=imsgb;

    stat=FillMessage(msglen,SeqNumber,Client,dest,GETVAR,VarName,msgb);

    if(IS_NOT_ERROR(stat)) {
        stat = SendToServer(msgb);
    }
    if (imsgb==NULL) FreeMsgBuf(msgb);
    return stat;
}


//@Function[API]: ReadVar
//
// Send request to get a variable value and waits for the reply
//
// This routine sends a request to get the value of a variable from the 
// real time database, then waits for a reply and returns the number of
// variable item actually read.

// A more flexible (and complicated) managements of variable retrieval
// can be performed by calling GetVar() and then managing returned messages
//@

int ReadVar(char *Client,   //@P{Client}: client's name
            char *VarName,  //@P{VarName}: Variable name
            void *varBuf,   //@P{varBuf}: Buffer to store variable value
                            // Warning it must be long enough to hold all
                            // the variable elements!
            int  nItms,     // Number of variable items in varBuf (if lower
                            // than variable length, only as many items
                            // will be stored. If longer than variable length
                            // the last items will not be affected.
            int tmout,      //@P{tmout}: Timeout in milliseconds. If 0, wait
                            // forever.
            MsgProc callbk, //@P{callbk}: function to call when receiving 
                            // unexpected messages from MsgD-RTDB. If NULL,
                            // unexpected messages are ignored.
            MsgBuf *imsgb)  //@P{msgb}: Pointer to a suitable message structure
                            // If NULL the structure is created locally. You
                            // may wanto to specify an MsgBuf structure of
                            // your to avoid the overhead of creating and
                            // destroying it at each call.
                            // @R: If > 0, is the number of elements 
                            //             returned into varBuf.
                            //     If < 0, error code
{
int stat,nit,sqn;
MsgBuf lmsgb;
MsgBuf *msgb;
Variable *theVar;
int len;
char destbuf[PROC_NAME_LEN+1];
char *dest, *dpt;

if((dpt=strchr(VarName,'@')))
   dest=mkFullName("",++dpt,destbuf);
else
   dest="";

len=strlen(VarName)+1;

if(imsgb==NULL) {                     // If no buffer provided
    stat=InitMsgBuf(&lmsgb,len);        // create it.
    if(IS_ERROR(stat)) return stat;
    msgb=&lmsgb;
} else 
    msgb=imsgb;

sqn=NewSeqN();
stat=FillMessage(len,sqn,Client,dest,GETVAR,VarName,msgb);

if(IS_NOT_ERROR(stat)) { 
    stat = SendToServer(msgb);
}
if(IS_NOT_ERROR(stat)) stat = WaitMessageSet(&sqn,1,msgb,callbk,tmout);
if(IS_NOT_ERROR(stat)) {
    if(HDR_CODE(msgb)==VARREPLY) {
        int i;

        theVar=(Variable *)MSG_BODY(msgb);
        nit=(nItms<theVar->H.NItems) ? nItms : theVar->H.NItems;
        switch(theVar->H.Type) {
        case INT_VARIABLE:
			#ifdef LONG_IS_64
            for(i=0;i<nit;i++) ((int*)varBuf)[i]=theVar->Value.Lv[i];
			#else
				for(i=0;i<nit;i++) ((long*)varBuf)[i]=theVar->Value.Lv[i];
			#endif	
            stat=nit; break;
        case REAL_VARIABLE:
            for(i=0;i<nit;i++) ((double*)varBuf)[i]=theVar->Value.Dv[i];
            stat=nit; break;
        case CHAR_VARIABLE:
            strncpy((char *)varBuf,theVar->Value.Sv,nit);
            stat=nit; break;
        case BIT8_VARIABLE:
            for(i=0;i<nit;i++) ((unsigned char*)varBuf)[i]=theVar->Value.B8[i];
            stat=nit; break;
        case BIT16_VARIABLE:
            for(i=0;i<nit;i++) ((unsigned short*)varBuf)[i]=theVar->Value.B16[i];
            stat=nit; break;
        case BIT32_VARIABLE:
			#ifdef LONG_IS_64
				for(i=0;i<nit;i++) ((unsigned int*)varBuf)[i]=theVar->Value.B32[i];
			#else
            for(i=0;i<nit;i++) ((unsigned long*)varBuf)[i]=theVar->Value.B32[i];
			#endif
            stat=nit; break;
        case BIT64_VARIABLE:
            for(i=0;i<nit;i++) ((unsigned long long*)varBuf)[i]=theVar->Value.B64[i];
            stat=nit; break;
        default:
            stat=VAR_TYPE_UNDEF_ERROR;
       }
    } else
        stat=UNEXPECTED_REPLY_ERROR;
}

if (imsgb==NULL) FreeMsgBuf(msgb);
return stat;
}




//@Function: VarCmd

// Sends the variable request command common to WriteVar and SetVar

// Note: this function is also called for internal use of thrdlib
//@

int VarCmd(const char *Client,  //@P{Client}: Client's name
           int  MsgCode,        //@P{MsgCode}: use SETVAR for no reply, use WRITEVAR
                                //             if acknowledge from \msgd\  is wanted.
           const char *VarName, //@P{VarName}: Variable name
           int   VarType,       //@P{VarType}: Variable type (as defined in rtdblib.h)
                                // Ignored except for WRITEVAR operation
           int  NElements,      //@P{NElements}: Variable length (number of elements)
	        const void *Value,   //@P{Value}: Pointer to variable value. If NULL, the
                                // variable is zero-filled.
	        MsgBuf *imsgb)       //@P{imsgb}: Pointer to a suitable message structure
                                // If NULL the structure is created locally. You
                                // may want to specify an MsgBuf structure of
                                // your to avoid the overhead of creating and
                                // destroying it at each call. 
                                // @R: if <0: error code
                                //     else generated sequence number
{
int stat,sqn;
int varlen,msglen;
MsgBuf lmsgb;
MsgBuf *msgb;
Variable *theVar;
char destbuf[PROC_NAME_LEN+1];
char *dest, *dpt;

   stat=strlen(VarName);

   if((stat>VAR_NAME_LEN) || (stat==0))
       return PLAIN_ERROR(VAR_NAME_ERROR);

   if((dpt=strchr(VarName,'@')))
      dest=mkFullName("",++dpt,destbuf);
   else
      dest="";

   varlen=VarSize(VarType,NElements);

   if(varlen<0) 
       return PLAIN_ERROR(VAR_SIZE_ERROR);

   msglen=sizeof(VHeader)+varlen;

   if(imsgb==NULL) {                       // If no buffer provided
       stat=InitMsgBuf(&lmsgb,msglen);     // create it.
       if(IS_ERROR(stat)) return stat;
       msgb=&lmsgb;
   } else {
       msgb=imsgb;
       ResizeMsgBuf(msgb,msglen);
   }

   sqn=NewSeqN();
   if(IS_ERROR(stat=FillHeader(msglen,sqn,Client,  // Make msg header (and
                       dest,MsgCode, msgb))) {     // adjust the buffer)
       if(imsgb==NULL) FreeMsgBuf(msgb);
       return stat;
   }

   theVar = (Variable *)MSG_BODY(msgb);

   theVar->H.Type=VarType;
   theVar->H.NItems=NElements;

   memset(theVar->H.Owner,0,sizeof(theVar->H.Owner));
   memset(&(theVar->H.MTime),0,sizeof(theVar->H.MTime));
   strcpy(theVar->H.Name,VarName);
   if(VarType==CHAR_VARIABLE && NElements==0) Value="";
   if(Value)
       memcpy(&(theVar->Value),Value,varlen);
   else
       memset(&(theVar->Value),0,varlen);

   stat=SendToServer(msgb);
   if(imsgb==NULL) FreeMsgBuf(msgb);

   if(IS_ERROR(stat)) return stat;
   return sqn;
}

//@Function[API]: SetVar
//
// Send to MsgD-RTDB a request to write value(s) into a variable
//
// This routine sends to the Real Time database process  a request to
// write values into a variable in the RTDB.
//
//@

int SetVar(char *Client,        //@P{Client}: Client's name
           char *VarName,       //@P{VarName}: Variable name
           int   VarType,       //@P{VarType}: Variable type (as defined in rtdblib.h)
           int  NElements,      //@P{NElements}: Variable length (number of elements)
	   void *Value,         //@P{Value}: Variable value
	   MsgBuf *imsgb)       //@P{imsgb}: Pointer to a suitable message structure
                                // If NULL the structure is created locally. You
                                // may want to specify an MsgBuf structure of
                                // your to avoid the overhead of creating and
                                // destroying it at each call. 
                                // @R: completion code
{
    return VarCmd(Client,SETVAR,VarName,VarType,NElements,Value,imsgb);
}

//@Function[API]: WriteVar
//
// Write a variable. Create the variable if not existent. Verify completion
//
// This routine sends a request to write a variable in the real-time database.
// then waits for a completion code. If the variable is not defined, it is
// created.

// A more flexible (and complicated) managements of variable retrieval
// can be performed by calling SetVar() and then managing returned messages
// 
//@

int WriteVar(char *Client,  //@P{Client}: client's name
             char *VarName, //@P{VarName}: Variable name
             int   VarType, //@P{VarType}: Variable type (as defined in rtdblib.h)
             int  NElements,//@P{NElements}: Variable length (number of elements)
             void *Value,   //@P{Value}: Variable value
             int tmout,     //@P{tmout}: Timeout in milliseconds. If 0, wait
                            // forever.
            MsgProc callbk, //@P{callbk}: function to call when receiving 
                            // unexpected messages from MsgD-RTDB. If NULL,
                            // unexpected messages are ignored.
            MsgBuf *imsgb)  //@P{msgb}: Pointer to a suitable message structure
                            // If NULL the structure is created locally. You
                            // may wanto to specify an MsgBuf structure of
                            // your to avoid the overhead of creating and
                            // destroying it at each call.
                            // @R: If > 0, is the number of elements 
                            //             returned into varBuf.
                            //     If < 0, error code
{
int stat;
int varlen,msglen;
MsgBuf lmsgb;
MsgBuf *msgb;

stat=strlen(VarName);

if((stat>VAR_NAME_LEN) || (stat==0))
    return PLAIN_ERROR(VAR_NAME_ERROR);

varlen=VarSize(VarType,NElements);

if(varlen<0) 
    return PLAIN_ERROR(VAR_SIZE_ERROR);

msglen=sizeof(VHeader)+varlen;

if(imsgb==NULL) {                       // If no buffer provided
    stat=InitMsgBuf(&lmsgb,msglen);     // create it.
    if(IS_ERROR(stat)) return stat;
    msgb=&lmsgb;
} else {
    msgb=imsgb;
    ResizeMsgBuf(msgb,msglen);
}

stat = VarCmd(Client,WRITEVAR,VarName,VarType,NElements,Value,msgb);
if(stat>=0) stat = WaitMessageSet(&stat,1,msgb,callbk,tmout);

if(IS_NOT_ERROR(stat)) {
    if(HDR_CODE(msgb)!= VARWRITN) stat=NOT_OWNER_ERROR;
}

if(imsgb==NULL) FreeMsgBuf(msgb);
return stat;

}

//@Function[API]: GiveVar
//
// This routine is obsolete
//
// No reply is expected from \msgd.
//@

int GiveVar(char *Client,   //@P{Client}: Client's name
            char *vname,    //@P{vname}: Variable name specification.
                             //  if terminated by '*', it is used as aprefix
                             //  for name matching.
            MsgBuf *imsgb)   //@P{msgb}: Pointer to a suitable message structure
                             // If NULL the structure is created locally. You
                             // may wanto to specify an MsgBuf structure of
                             // your to avoid the overhead of creating and
                             // destroying it at each call.
                             // @R: return status
{
   return NO_ERROR;
}


//@Function[API]: TakeVar
//
// This routine is obsolete
//
// No reply is expected from \msgd.
//@

int TakeVar(char *Client,   //@P{Client}: Client's name
            char *vname,    //@P{vname}: Variable name specification.
                             //  if terminated by '*', it is used as aprefix
                             //  for name matching.
            MsgBuf *imsgb)   //@P{msgb}: Pointer to a suitable message structure
                             // If NULL the structure is created locally. You
                             // may wanto to specify an MsgBuf structure of
                             // your to avoid the overhead of creating and
                             // destroying it at each call.
                             // @R: return status
{
   return NO_ERROR;
}


//@Function[API]: DumpVars
//
// Send request to Dump a list of variables
//
// This routine sends a request to dump to log system the values of a set
// of variables variables whose names match a given prefix.  
//
// No reply is expected from \msgd.
//
//@

int DumpVars(char *Client,   //@P{Client}: Client's name
             char *prefix,   //@P{prefix}: Variable name specification.
                             //  if terminated by '*', it is used as aprefix
                             //  for name matching.
	     MsgBuf *imsgb)  //@P{imsgb}: Pointer to a suitable message structure
                             // If NULL the structure is created locally. You
                             // may wanto to specify an MsgBuf structure of
                             // your to avoid the overhead of creating and
                             // destroying it at each call.
                             // @R: completion code
{
    MsgBuf lmsgb;
    MsgBuf *msgb;
    int stat,len;
    char destbuf[PROC_NAME_LEN+1];
    char *dest, *dpt;

    if((dpt=strchr(prefix,'@')))
       dest=mkFullName("",++dpt,destbuf);
    else
       dest="";

    len=strlen(prefix)+1;    // Add 1 to include terminating '\0'

    if(imsgb==NULL) {                  // If no buffer provided create it.
        stat=InitMsgBuf(&lmsgb,len);
        if(IS_ERROR(stat)) return stat;
	     msgb=&lmsgb;
    } else 
	     msgb=imsgb;

    stat=FillMessage(len,0,Client,dest,DUMPVARS,prefix,msgb);
    if(IS_NOT_ERROR(stat)) {
        stat = SendToServer(msgb);
    }
    if(imsgb==NULL) FreeMsgBuf(msgb);
    return stat;
}

//@Function[API]: LockVars
//
// Send request to lock a list of variables
//
// This routine sends a request to lock a set of variables whose names 
// match a given prefix.  
//
// The \msgd\  replies with an acknowledge message (either ACK or NAK)
//
//@

int LockVars(char *Client,   //@P{Client}: Client's name
             char *prefix,   //@P{prefix}: Variable name specification.
                             //  if terminated by '*', it is used as aprefix
                             //  for name matching.
             int tmout,      //@P{tmout}: Timeout in milliseconds. If 0, wait
                             // forever.
             MsgProc callbk, //@P{callbk}: function to call when receiving 
                             // unexpected messages from MsgD-RTDB. If NULL,
                             // unexpected messages are ignored.
	          MsgBuf *imsgb)  //@P{imsgb}: Pointer to a suitable message structure
                             // If NULL the structure is created locally. You
                             // may wanto to specify an MsgBuf structure of
                             // your to avoid the overhead of creating and
                             // destroying it at each call.
                             // @R: completion code
{
    MsgBuf lmsgb;
    MsgBuf *msgb;
    int stat,len,sqn;
    char destbuf[PROC_NAME_LEN+1];
    char *dest, *dpt;

    if((dpt=strchr(prefix,'@')))
       dest=mkFullName("",++dpt,destbuf);
    else
       dest="";

    len=strlen(prefix)+1;    // Add 1 to include terminating '\0'

    if(imsgb==NULL) {                  // If no buffer provided create it.
        stat=InitMsgBuf(&lmsgb,len);
        if(IS_ERROR(stat)) return stat;
	     msgb=&lmsgb;
    } else 
	     msgb=imsgb;

    sqn=NewSeqN();
    stat=FillMessage(len,sqn,Client,dest,LOCKVAR,prefix,msgb);
    HDR_PLOAD(msgb)=1;
    if(IS_NOT_ERROR(stat))  {
        stat = SendToServer(msgb);
    }
    if(IS_NOT_ERROR(stat)) stat = WaitMessageSet(&sqn,1,msgb,callbk,tmout);
    if(IS_NOT_ERROR(stat)) {
        if(HDR_CODE(msgb)!=ACK)  {
            if(HDR_CODE(msgb)==NAK) 
                stat = NAK_RECEIVED;
            else
                stat = UNEXPECTED_REPLY_ERROR;
        }
    }
    if(imsgb==NULL) FreeMsgBuf(msgb);
    return stat;
}



//@Function[API]: UnlockVars
//
// Send request to lock a list of variables
//
// This routine sends a request to lock a set of variables whose names 
// match a given prefix.  
//
// The \msgd\  replies with an acknowledge message (either ACK or NAK)
//
//@

int UnlockVars(char *Client,   //@P{Client}: Client's name
               char *prefix,   //@P{prefix}: Variable name specification.
                               //  if terminated by '*', it is used as aprefix
                               //  for name matching.
               int tmout,      //@P{tmout}: Timeout in milliseconds. If 0, wait
                               // forever.
               MsgProc callbk, //@P{callbk}: function to call when receiving 
                               // unexpected messages from MsgD-RTDB. If NULL,
                               // unexpected messages are ignored.
	            MsgBuf *imsgb)  //@P{imsgb}: Pointer to a suitable message structure
                               // If NULL the structure is created locally. You
                               // may wanto to specify an MsgBuf structure of
                               // your to avoid the overhead of creating and
                               // destroying it at each call.
                               // @R: completion code
{
    MsgBuf lmsgb;
    MsgBuf *msgb;
    int stat,len,sqn;
    char destbuf[PROC_NAME_LEN+1];
    char *dest, *dpt;

    if((dpt=strchr(prefix,'@')))
       dest=mkFullName("",++dpt,destbuf);
    else
       dest="";

    len=strlen(prefix)+1;    // Add 1 to include terminating '\0'

    if(imsgb==NULL) {                  // If no buffer provided create it.
        stat=InitMsgBuf(&lmsgb,len);
        if(IS_ERROR(stat)) return stat;
	     msgb=&lmsgb;
    } else 
	     msgb=imsgb;

    sqn=NewSeqN();
    stat=FillMessage(len,sqn,Client,dest,LOCKVAR,prefix,msgb);
    HDR_PLOAD(msgb)=0;
    if(IS_NOT_ERROR(stat))  {
        stat = SendToServer(msgb);
    }
    if(IS_NOT_ERROR(stat)) stat = WaitMessageSet(&sqn,1,msgb,callbk,tmout);
    if(IS_NOT_ERROR(stat)) {
        if(HDR_CODE(msgb)!=ACK)  {
            if(HDR_CODE(msgb)==NAK) 
                stat = NAK_RECEIVED;
            else
                stat = UNEXPECTED_REPLY_ERROR;
        }
    }
    if(imsgb==NULL) FreeMsgBuf(msgb);
    return stat;
}

//@Function[API]: VarList
//
// Send request to get from the Real-Time Database a list of variables
//
// This routine sends a request to get the values of variables whose names
// match a given prefix.  The  \msgd\ server replies to this
// command by sending a sequence of VARREPLY messages terminated by a 
// VAREND message.
//
// If no variable matches the request, only the VAREND message is sent back.
//
// The calling process must properly manage the reply messages sent back 
// by the \msgd\  process.
//@

int VarList(char *Client,   //@P{Client}: Client's name
            char *prefix,   //@P{prefix}: Variable name specification. If terminated
                            //  with '*' all variables with given prefix are selected,
                            // otherwise requires exact matching.
                            //  name matching
	         int SeqNumber,  //@PSeqNumber}: Message sequence number. 
	         MsgBuf *imsgb)  //@P{imsgb}: Pointer to a suitable message structure
                            // If NULL the structure is created locally. You
                            // may want to specify an MsgBuf structure of
                            // your to avoid the overhead of creating and
                            // destroying it at each call.
                            // @R: completion code
{
    MsgBuf lmsgb;
    MsgBuf *msgb;
    int stat,len;
    char destbuf[PROC_NAME_LEN+1];
    char *dest, *dpt;

    if((dpt=strchr(prefix,'@')))
       dest=mkFullName("",++dpt,destbuf);
    else
       dest="";

    len=strlen(prefix)+1;    // Add 1 to include terminating '\0'

    if(imsgb==NULL) {                  // If no buffer provided create it.
        stat=InitMsgBuf(&lmsgb,len);
        if(IS_ERROR(stat)) return stat;
	     msgb=&lmsgb;
    } else 
	     msgb=imsgb;

    stat=FillMessage(len,SeqNumber,Client,dest,VARLIST,prefix,msgb);
    if(IS_NOT_ERROR(stat)) {
        stat = SendToServer(msgb);
    }
    if(imsgb==NULL) FreeMsgBuf(msgb);
    return stat;
}


//@Function[API]: VarNotif
//
// Request notification of variable change

// Sends a request to the Real-Time Database to enter the notification queue
// for the specified variable.

// When the client is registered, any write operation to the variable is notifyed
// with a VARCHANGD message containing the Variable value in the message body.

// If the variable is deleted (@see{DelVar}), the client is notified via
// a VAREMOVD message.

//@

int VarNotif(char *Client,    //@P{Client}: Client's name
             char *prefix)    //@P{VarName}: Variable name
{
   int stat, len;
   MsgBuf buf;
   MsgBuf *msgb = &buf;
    char destbuf[PROC_NAME_LEN+1];
    char *dest, *dpt;

    if((dpt=strchr(prefix,'@')))
       dest=mkFullName("",++dpt,destbuf);
    else
       dest="";

   len = strlen(prefix)+1;

   InitMsgBuf(msgb, len);

   if(IS_ERROR(stat=FillMessage(len, 0,Client,  // Make msg header
	          dest,VARNOTIF, prefix, msgb))) { 
         FreeMsgBuf(msgb);
         return stat;
   }
   HDR_PLOAD(msgb)=1;
   stat =  SendToServer(msgb);
   FreeMsgBuf(msgb);
   return stat;
}


//@Function[API]: VarNoNotif
//
// Cancel previous notification request

// Sends a request to the Real-Time Database to leave the notification queue
// for the specified variable.
//@

int VarNoNotif(char *Client,     //@P{Client}:Client's name
               char *prefix)     //@P{prefix}:Variable name
{
int stat, len;
MsgBuf buf;
MsgBuf *msgb = &buf;
char destbuf[PROC_NAME_LEN+1];
char *dest, *dpt;

if((dpt=strchr(prefix,'@')))
   dest=mkFullName("",++dpt,destbuf);
else
   dest="";

len = strlen( prefix)+1;

if(IS_ERROR(stat=InitMsgBuf( msgb, len))) return stat;

if(IS_ERROR(stat=FillMessage(len,0,Client,      // Make msg header
             dest,VARNOTIF, prefix, msgb))) {
    FreeMsgBuf(msgb);
    return stat;
}
HDR_PLOAD(msgb)=0;

stat =  SendToServer(msgb);
FreeMsgBuf(msgb);
return stat;
}

//@Routine[API]: VarDebug
//
// Set or reset the debug flag
//
// This routine is obsolete. It does nothing
//@

void VarDebug(int deb)   //@P{deb}: either 1 (set) or 0 (reset)
{
    Debug=deb;
}

#ifdef LONG_IS_64
struct timeval tv_fm_tv32(struct timeval32 tv32)
{
        struct timeval tv;
        tv.tv_sec = tv32.tv_sec;
        tv.tv_usec = tv32.tv_usec;
        return tv;
}

struct timeval32 tv32_fm_tv(struct timeval tv)
{
        struct timeval32 tv32;
        tv32.tv_sec = tv.tv_sec;
        tv32.tv_usec = tv.tv_usec;
        return tv32;
}
#endif



