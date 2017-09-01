//@File: client.c
//
// Client related routines and functions
//@

#include <unistd.h> 
#include <stdlib.h> 
#include <stdio.h> 
#include <string.h> 
#include <signal.h> 
#include <errno.h> 
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h> 
#include <arpa/inet.h> 

#include <pthread.h>

#include "msgdrtdb.h"

#include "base/errlib.h"
#include "base/msglib.h"
#include "base/rtdblib.h"

#include "shmlib.h"
#include "client.h"
#include "logerr.h"
#include "rtdb.h"

#define DO_LOG   1
#define NO_LOG   0

typedef struct {
        int errcod;
        char msg[0];
    } AUX_M;

extern struct PGINFO progInfo;
extern Client *MySelf;
extern ShmBufferPool bPool;

extern pthread_mutex_t shmMgmtMutex;

extern CLtable ClientTable;

extern pthread_mutexattr_t mutexAttributes;

//@Function: InitClient
//
// Initializes the client structure
//@
void InitClient(Client *cl)
{
    cl->thId=pthread_self();
    cl->startTm.tv_sec=0;
    cl->startTm.tv_usec=0;
    InitMsgBuf(&(cl->rMsgb),0);
    cl->Status=0;
    cl->prot43=0;
    cl->aux43=NULL;
    cl->auxln=0;
    cl->toLog=1;
    cl->toclose=0;
    cl->enabled=0;
    cl->Status=CLIENT_NONE;
    cl->blkErrCount=0;
    cl->ClientReady=0;
    cl->Waiting[0]='\0';
    cl->Name[0]='\0';
    cl->version[0]='\0';
    MUTEX_INIT(&(cl->mutex),"ClientMutex");
}

// @Function: ClientTableInit

// Initializes the client table
//
//@

int ClientTableInit(int npeers)
{
int i;

MUTEX_INIT(&(ClientTable.mutex),"ClientTable");
ClientTable.needsCleanup=0;
ClientTable.maxItem=MAXCLIENTS-1;     // last item index available
ClientTable.maxPeer=npeers;
ClientTable.nItems=0;
ClientTable.nPeers=0;
ClientTable.maxCurItem=ClientTable.maxPeer;    // Max index of registered client

for(i=0;i<=ClientTable.maxItem;i++) {
   ClientTable.Items[i]=NULL;
   ClientTable.Names[i]="";
   ClientTable.nConn[i]=0;
}
return NO_ERROR;
}


//@Function: GetClientByName  
//
// Search a client given its name.
//
// The routine is not thread safe, Table mutex must be locked
// before calling
//@

Client *GetClientByName(char *name, 
                          int select)  // @P{select}: Search type: STD_CLIENT find only local clients
                                       //             PEER_MSGD find only peers, 0 find any
                                       // @R: Client structure
{
    int i,first,last;
    Client *cl=NULL;

    switch(select) {
    case PEER_MSGD:      // Matches only for peers
        first=1;
        last=ClientTable.maxPeer;
        break;
    case STD_CLIENT:      // Matches only for local clients
        first=ClientTable.maxPeer+1;
        last=ClientTable.maxCurItem;
        break;
    default:
        first=1;
        last=ClientTable.maxCurItem;
    }

    for(i=1;i<=last;i++) {
        Client *next;
	if((next=ClientTable.Items[i])==NULL) continue;
	if(strcmp(next->Name,name)==0) {
           cl=next;
           break;
        }
    }
    return cl;
}


//@Function: addClientToTable  
//
// Add a new client to the client table

// This function  is called to add a client to the client
// table. If the client has been previously registered and exited,
// it is assigned the same slot, so that its client ID returns
// the same.
//
//@

static
 int AddClientToTable(Client *item,   // @P{item}: Client's structure
                      int status)     // @P{status}: set client to this status
                                      // @R: Completion code
{
   int found=-1,last,first,i;
   int stat=NO_ERROR;
   int peer=(item->clType==PEER_MSGD);
   char *itName=item->Name;

   MUTEX_LOCK(&ClientTable.mutex,"AddClientToTable");

   if(peer) {                                     //  begin critical section *****
      first=1;
      last=ClientTable.maxPeer;
   } else {
      first=ClientTable.maxPeer+1;
      last=ClientTable.maxCurItem;
   }

   if(GetClientByName(itName,0)!=NULL)          {
      stat = DUPLICATE_CLIENT_ERROR;
   } else {
      for(i=first; i<=last; i++)                   // Search an empty slot
         if(ClientTable.Items[i]==NULL)            // with the same name
            if(strncmp(ClientTable.Names[i],
                            itName,PROC_NAME_LEN)==0) {
               found=i;
               break;
            }
      if(found<0) {                     // No named empty slot, append
         if(peer) {
            for(i=first; i<=last; i++)            // Search an empty slot
                if(ClientTable.Items[i]==NULL) {  // in peer area
                   found=i;
                   break;
                }
         } else {
             if(ClientTable.maxCurItem<ClientTable.maxItem)
                 found= ++ClientTable.maxCurItem;
         }
         if(found>0) {
            ClientTable.Names[found] = (char *)malloc(PROC_NAME_LEN+1);
            if(ClientTable.Names[found] == NULL) {
                MUTEX_UNLOCK(&ClientTable.mutex,"AddClientToTable"); // end critical section *****
                return SYSTEM_ERROR(MEMORY_ALLOC_ERROR);
            }
            strncpy(ClientTable.Names[found],itName,PROC_NAME_LEN);
            ClientTable.Names[found][PROC_NAME_LEN]='\0';
            ClientTable.nConn[found]=0;
         }
      } 
      if(found>0) {
         item->Id=found;
         item->Status=status;
         ClientTable.Items[found]=item;
         ClientTable.nItems++;
         if(item->clType==PEER_MSGD) ClientTable.nPeers++;
         ClientTable.nConn[found]++;
      } else
            stat = TOOMANY_ITEMS_ERROR;
   }
   MUTEX_UNLOCK(&ClientTable.mutex,"AddClientToTable");      // end critical section *****
   return stat;
}

//@Procedure: RemoveClient
//
// Remove a client from client table
//
// This routine removes the client from the client table
// Client table must be locked prior of calling
//@

void RemoveClient(int clid)   //@P{clid}: client index in client table
{
    Client *it=ClientTable.Items[clid];
    ClientTable.Items[clid] = NULL;
    ClientTable.nItems--;
    if(it->clType==PEER_MSGD) ClientTable.nPeers--;
}

//@Function: CancelClient 
//
// Cancel registration of a client
//
// This routine removes the client from the list of registered
// client. It locks the Client table prior of doing that.
//
// At the end, closes the client's socket
//@

static 
void CancelClient(Client *it)
{
int myid,stat;

myid=it->Id;

if(it->Status!=CLIENT_REGISTERED) {
   LAO_Error(it,UNREGISTER_ERROR,"RemoveClient(Id:%d)",myid);
   return;
}

MUTEX_LOCK(&ClientTable.mutex,"CancelClient"); // Critical sect //****
if(ClientTable.Items[myid] != it) {                             //****
    stat = TABLE_INCONSIST_ERROR;                               //****
} else {                                                        //****
    RemoveClient(myid);                                         //****
    stat=NO_ERROR;                                              //****
}                                                               //****
MUTEX_UNLOCK(&ClientTable.mutex,"CancelClient");                //****
if(IS_ERROR(stat))
   LAO_Error(it,stat,"CancelClient(Id:%d)",myid);
else
   LAO_Debug(it,"Client removed from table @ slot %d",myid);

LAO_Debug(it,"Client at addr:0x%lx closing sock:%d",it,it->conn.sock);
close(it->conn.sock);
it->Status=CLIENT_NONE;

}


//@Procedure: TerminateAllClientThreads 
//
// Terminates all items threads
//
// This call terminates all active threads related to items, 

// Note: it must be called by the main thread, otherwise it would
// fail when killing the calling thread
//@

void TerminateAllClientThreads(void) 
{
    int i,stat;
    Client *it;

    MUTEX_LOCK((&ClientTable.mutex),"TerminateAllClientThreads");
    for(i=1;i<=ClientTable.maxCurItem;i++)  {
	    if((it=ClientTable.Items[i])==NULL) continue;

	    stat=pthread_cancel(it->thId);
       if(stat!=0)
	       LAO_Error(it,THREAD_CANCEL_ERROR,"in TerminateAllClientThreads()");
       else
	       LAO_Info(it,"has been terminated");
    }
    ClientTable.nItems=0;
    MUTEX_UNLOCK((&ClientTable.mutex),"TerminateAllClientThreads");
}

//@Function: cvt43 

// Converts message buffer into old format as per protocol version 43
//@

static int cvt43(MsgBuf* msgb, Message43 *msg43) {
   char *src, *dst;
   void *msgbody, *msg43body;
   size_t nbt;
    
   Message *msg=MSG_BUFFER(msgb);

   if((strlen(HDR_FROM(msgb))>PROC_NAME_LEN)||
      (strlen(HDR_TO(msgb))>PROC_NAME_LEN)    ) return CVT_43_ERROR;

   msg43->SG.Magic=43;
   msg43->SG.Len=HDR_LEN(msgb);
   msg43->HD.SeqNum=HDR_SEQN(msgb);
   msg43->HD.Code=HDR_CODE(msgb);
   msg43->HD.Flags=HDR_FLAGS(msgb); 
   msg43->HD.ttl=HDR_TTL(msgb);
   msg43->HD.pLoad=HDR_PLOAD(msgb);

   src=msg->HD.To; dst=msg43->HD.To;
   strncpy(dst,src,PROC_NAME_LEN);
   dst[PROC_NAME_LEN]='\0';

   src=msg->HD.From; dst=msg43->HD.From;
   strncpy(dst,src,PROC_NAME_LEN);
   dst[PROC_NAME_LEN]='\0';

   msgbody=msg->Body; msg43body=msg43->Body;
   nbt=msg->SG.Len;
   memcpy(msg43body,msgbody,nbt);
   return NO_ERROR;
}

// @Function getStartT
//
// Returns client's start time
// @

static int getStartT(MsgBuf *imsg)
{
   Client *cl;
   struct timeval *tv=NULL;
   int stat;

   if(*MSG_BODY(imsg)=='\0') 
      tv=&progInfo.startTime;
   else {
      cl=GetClientByName((char *)MSG_BODY(imsg),0);
      if(cl) tv=&(cl->startTm);
   }
   if(tv) {
      SetMsgPload(1,imsg);
      stat=FillBody(tv,sizeof(struct timeval),imsg);
      if(IS_ERROR(stat)) LAO_Error(cl,stat,"in getStartTT");
   } else {
      SetMsgPload(0,imsg);
      SetMsgLength(0,imsg);
      stat=NO_ERROR;
   }
   return stat;
}


// @Function: sendTo

// Send a message to a destination specified by client pointer
//
// This routine sends a full message to another process identified by
// the pointer to client's structure
//@

static int sendTo(Client *cl, MsgBuf *msgb)
{
   int stat,msglen,nfds;
   fd_set writefds;
   struct timeval timeout={0,progInfo.netIoTmout};
   void *buf;

   stat=NO_ERROR;
   if(cl->prot43) {
      msglen=sizeof(Message43)+HDR_LEN(msgb);
      LAO_Trace(cl,"Converting message SeqN:%d len:%d to protocol 43",HDR_SEQN(msgb),HDR_LEN(msgb));
      if(msglen>cl->auxln) {  // Allocate more space for aux buffer
         cl->aux43=realloc(cl->aux43,msglen);
         if(cl->aux43==NULL) return MEMORY_ALLOC_ERROR;
         cl->auxln=msglen;
      }
      stat=cvt43(msgb,cl->aux43);
      if(IS_ERROR(stat)) {
         LAO_Error(cl,stat,"converting to protocol 43 message");
         return stat;
      }
      buf=cl->aux43;
   }else{
      msglen=sizeof(Message)+HDR_LEN(msgb);
      buf=MSG_BUFFER(msgb);
   }
            // Send full message (signature+header+body)
            //
            // Note: The message must be sent in a single call to avoid
            //       long delays due to TCP/IP optimization algorithm
            //
   nfds=cl->conn.sock+1;
   FD_ZERO(&writefds);
   FD_SET(cl->conn.sock,&writefds);
   stat=select(nfds,NULL,&writefds,NULL,&timeout);
   if(stat!=1) 
      return BLOCK_ERROR;
   if((stat=send(cl->conn.sock,buf,msglen,MSG_NOSIGNAL))!=msglen)
       stat = SYSTEM_ERROR(COMMUNICATION_ERROR);
   return(stat);
}

//@Function: markClose
//
// mark a client to be closed at next housekeeping step
//
// Client's mutex is supposd to be locked
//@

static void markClose(Client *cl)
{
  cl->toclose=1;
  LAO_Info(cl, "is marked for forced close");
}

//@Function: sendLog 
//
// Wrapper around sendTo routine which adds logging
//
// This routine has the same effect as SendTo  but adds logging.
//
// This routine assumes thet the Client's mutex has been locked prior
// of the call
//@

static
  int sendLog(Client* cl,      // Recipient
              MsgBuf  *msgb,   // MsgBuf structure
              int   tolog)     // Logging flag.
                               // Returns: Completion code
{
int stat;
if(cl->toclose) return NO_ERROR;
if(!cl->enabled) {
   LAO_Error(cl,SEND_TO_DISABLED_ERROR,"in sendLog()");
   return NO_ERROR;
}

if(tolog && cl->toLog) LAO_LogMsgSnt(msgb,cl);

stat = sendTo(cl,msgb);
switch(stat) {
  case NO_ERROR:
    cl->blkErrCount=0;
    break;
  case BLOCK_ERROR:
    if(++cl->blkErrCount>=MAX_BLOCK_ERRORS) markClose(cl);
    break;
}
return stat;
}

//@Function: sendLogLk
//
// Sends message to client, after locking the client's mutex
// Optionally adds logging
//
// This routine has the same effect as sendLog (see msglib) but adds
// client's mutex locking
//@

static
  int sendLogLk(Client* cl,       // Recipient
                 MsgBuf  *msgb,   // MsgBuf structure
                 int     tolog)   // logginf flag
                                  // Returns: Completion code
{
int stat;

if(cl->toclose) return NO_ERROR;
if(!cl->enabled) {
   LAO_Error(cl,SEND_TO_DISABLED_ERROR,"in sendLogLk()");
   return NO_ERROR;
}

MUTEX_LOCK((&(cl->mutex)),"sendLogLk");
if(tolog && cl->toLog) LAO_LogMsgSnt(msgb,cl);
stat = sendTo(cl,msgb);
switch(stat) {
  case NO_ERROR:
    cl->blkErrCount=0;
    break;
  case BLOCK_ERROR:
    if(++cl->blkErrCount>=MAX_BLOCK_ERRORS) markClose(cl);
    break;
}
MUTEX_UNLOCK((&(cl->mutex)),"sendLogLk");
return stat;
}

//@Function: BcastToPeers
//
// Broadcasts a message to all peears
// 
// This routine is called to broadcast a message to all connected peers
//
// The client table mutex must be locked prior to call BcastToPeers
//@

static
int BcastToPeers(Client *sndCl, int tolog)
{
   Client *Dest;
   MsgBuf *msgb=&(sndCl->rMsgb);
   int i,stat=NO_ERROR,sent=0;
   char fdest[LONG_NAME_LEN+1];

   strncpy(fdest,HDR_TO(msgb),LONG_NAME_LEN);  // Save original destination
   fdest[LONG_NAME_LEN]='\0';

   for(i=1;i<=ClientTable.maxPeer;i++) {
      Dest=ClientTable.Items[i];
      if(Dest) {
         if(Dest->Status==CLIENT_REGISTERED) {
            *HDR_TO(msgb) = '\0';
            if(IS_NOT_ERROR(stat = sendLogLk(Dest,msgb,tolog))) {
               sent++; 
            } else {
               LAO_Error(sndCl,stat,"broadcasting message to peer %s",Dest->Name);
               stat=NO_ERROR;
            }
         }
      }
   }
   SetMsgDest(fdest, msgb);    // Restore original destination name
   if(sent==0) stat=CANNOT_DELIVER_ERROR;
   return stat;
}



//@Function: VarNotify 
//
// Sends messages to clients and peers registered for notification
//
// This routine is called when a variable value is modified to notify
// the event to registered clients.
//
// The routine simply sends registered clients a VARCHANGD message containing the
// same variable buffer as sent by the client who modified the variable.
//
// It is supposed that every field of the message header, except the
// destination field, is ready before the call.
//@

static int VarNotify(Client *fromCl,
                     CList  *NList   )
{
    int stat,src;
    char vname[VAR_NAME_LEN+1];
    Variable *theV;

//                                       Prepare message
    hdr_SEQN(fromCl->rMsgb)=NewSeqN();
    hdr_PLOAD(fromCl->rMsgb)=0;
    hdr_FLAGS(fromCl->rMsgb)=0;

    MUTEX_LOCK(&ClientTable.mutex,"VarNotify");

//  if(bcast) {                        // Variable must be mirrored
//      hdr_CODE(fromCl->rMsgb)=VARMIRROR;
//      LAO_Debug(NULL, "Mirroring variable \"%s\"", vname);
//      BcastToPeers(fromCl, 1);
//  }

    if(NList==NULL) {
       MUTEX_UNLOCK(&ClientTable.mutex,"VarNotify");
       return NO_ERROR;
    }

    theV=(Variable *)msg_BODY(fromCl->rMsgb);
    strncpy(vname,theV->H.Name,VAR_NAME_LEN);
    vname[VAR_NAME_LEN]='\0';

    hdr_CODE(fromCl->rMsgb)=VARCHANGD;
                                       // Notify to registered clients
    for(src=0;src<NList->nItms;src++) {  
       Client *toCl = NList->body[src].clPtr;
       char *fname =  NList->body[src].fname;
       char dest[LONG_NAME_LEN+1];
       char *pta;

       strncpy(dest,fname,LONG_NAME_LEN);
       dest[LONG_NAME_LEN]='\0';
       if((pta=strchr(dest,'@'))) {   // If destination is remote
          *pta='\0';                  // Remove domain from destination
          snprintf(theV->H.Name,      // And add src domain to variable name
              VAR_NAME_LEN+1,"%s@%s",vname,progInfo.MyName);
       } else 
          snprintf(theV->H.Name, VAR_NAME_LEN+1,"%s",vname);

       if(toCl) {
          LAO_Debug(NULL, "Notifying change for variable \"%s\" to: %s", vname, fname);
          SetMsgDest(dest,&(fromCl->rMsgb));
          stat=sendLogLk(toCl,&(fromCl->rMsgb),DO_LOG);
          if(IS_ERROR(stat)) 
             LAO_Error(NULL,stat,"sending notification for variable \"%s\" to %s", vname,fname);

       }
    }
    MUTEX_UNLOCK(&ClientTable.mutex,"VarNotify");
    return NO_ERROR;
}

//@Procedure: WriteClientVar
//
// Add or set the client status variable to RTDB.
//
// This function creates (or sets) the status variable for
// a given client.
//
// The variable name is: "<client_name>.CLSTAT", its value is an integer code 
// specifying current client status.
// See @see{msglib.h} for possible status values.
//@

static void WriteClientVar(Client *aClient, //@P{aClient}: Client structure
                          int     value)    //@P{value}: Variable value (status code)
                                            //@R: Completion code
{
             // The following trick creates a variable structure
             // with space for Variable value
union {
    Variable ComplexHD;
    unsigned char buff[sizeof(Variable)+sizeof(long)];
} varBuf;

long *vlong = (long *)varBuf.ComplexHD.Value.Lv;

CList *Nlist;
int stat;

                  // Prepare the variable structure for the client status variable
    snprintf(varBuf.ComplexHD.H.Name,VAR_NAME_LEN+1,"%s.CLSTAT",aClient->Name);
    varBuf.ComplexHD.H.Owner[0]='\0';
    varBuf.ComplexHD.H.Type=INT_VARIABLE;
    varBuf.ComplexHD.H.NItems=1;
    *vlong = value;
    
    stat = WriteVariable(&(varBuf.ComplexHD),aClient,&Nlist); // Create (or set) the variable

    if(IS_NOT_ERROR(stat)) {   // Send Notify messages reusing client's message buffer and the
                               // variable structure
        LAO_Debug(aClient,"Status variable %s set to %ld",
                                varBuf.ComplexHD.H.Name,value);
        stat=SetMsgSender(progInfo.MyName,&(aClient->rMsgb));      // Message sender is MsgD-RTDB
        SetSeqNum(0,&(aClient->rMsgb));               // Sequence number is forced to zero 
                                                      // for VARCHNGD messages
        stat=FillBody(varBuf.buff,sizeof(varBuf),
                                 &(aClient->rMsgb));  // Put variable value into message buffer
        stat=VarNotify(aClient,Nlist);                // Notify variable change to registered 
        stat=NO_ERROR;
    }  else
        LAO_Error(aClient, stat, "writing client variable %s",
                            varBuf.ComplexHD.H.Name);
}

//@Function: RegisterClient 
//
// Register a new client
//
// This routine registers a new client for service. Any client must
// issue a register request message just after connecting to MsgD-RTDB
//@

static 
  int RegisterClient(Client *cl,      // @P{cl}: Client structure
                     int    cltype)   // @P{cltype}: type of client (STD_CLIENT, PEER_MSGD)
                                      // @R: Completion code
{
   int stat=NO_ERROR;
   MsgBuf* imsg = &cl->rMsgb;
   char *spec;

   if(cl->Status!=CLIENT_JUST_CONNECTED)
      stat = REGISTER_REQUEST_ERROR;
   else {
      switch(cltype) {
      case STD_CLIENT:
      case PEER_MSGD:
         cl->clType=cltype;
         break;
      default:
         stat=ILL_CLIENT_TYPE;
      }
   }
   if(IS_ERROR(stat)) {
      LAO_Error(cl,stat,"RegisterClient()");
      return stat;
   }

   spec=(cltype==STD_CLIENT)?"client":"peer";

   if(cltype==PEER_MSGD) {        // Verify that the peer is authorized to
      int id;                    // connect to us.
      int found=0;
      extern PeerTable peerTable;

      for(id=0;id<peerTable.nPeers;id++) {
         if(strncmp(cl->Name,peerTable.body[id].name,sizeof(peerTable.body[id].name))==0)
            found=1;
      }
      if(!found) {        // The peer is not authorized
         stat=PEER_NOT_AUTH_ERROR;
         LAO_Error(cl,stat,"in RegisterClient()");
         return stat;
      }
  }

  if(IS_NOT_ERROR(stat=AddClientToTable(cl,CLIENT_REGISTERED))) {
     if(HDR_LEN(imsg)>0)         // Client sent version data
        strncpy(cl->version,(char *)MSG_BODY(imsg),VERSION_ID_LNG);
     cl->version[VERSION_ID_LNG-1]='\0';
     gettimeofday(&(cl->startTm),NULL);      // Store start time
     LAO_Info(cl,"%s registered as %s at slot %d",cl->version,spec,cl->Id);
     if(cl->prot43) LAO_Info(cl,"uses old protocol 43. Conversion enabled");
  } else {
     if(cl->clType==PEER_MSGD && stat==DUPLICATE_CLIENT_ERROR) { // Peer registration already ongoing
        LAO_Info(cl,"peer registration ongoing. Request ignored");
        stat=PEER_REG_ONGOING;
     } else
        LAO_Error(cl,stat,"RegisterClient()");
  }
  return stat;
}


//@Function: RouteToClient
//
// Routes a message to a client possibly via a peer
// 
// This routine is called to Route a message to a client. 
//
// The client table mutex is used.
//
//
// mutex
//@

static
void RouteToClient(Client *sndCl)
{
   Client *Dest;
   MsgBuf *msgb=&(sndCl->rMsgb);
   int stat=NO_ERROR;

   MUTEX_LOCK(&ClientTable.mutex,"RouteToClient");

   char *pos;
   if ((pos=strchr(HDR_TO(msgb),'@')) == NULL)  {    // send to local client
      Dest=GetClientByName(HDR_TO(msgb),STD_CLIENT);
      if(Dest) {
         if(Dest->Status!=CLIENT_REGISTERED)
            stat = CLIENT_STATUS_ERROR;
         else {
            if(sndCl->toLog || Dest->toLog) LAO_LogMsgRou(msgb);
            stat = sendLogLk(Dest,msgb,NO_LOG); 
            if(IS_ERROR(stat)) {
                LAO_NoMsgRou(msgb,stat);
                stat=NO_ERROR;
            }
         }
      } else {
         stat=NO_CLIENT_ERROR;         
      }
   } else {                             // Send to remote client
      char *peer = pos+1;
      Dest = GetClientByName(peer,PEER_MSGD);
      if (Dest) {
         if(Dest->Status!=CLIENT_REGISTERED) {
            stat = CLIENT_STATUS_ERROR;
         }
         else {
            char buf[LONG_NAME_LEN+1];
            if(sndCl->toLog || Dest->toLog) LAO_LogMsgRou(msgb);
            pos[0]='\0';               // Remove domain from Dest. field
            mkFullName(HDR_FROM(msgb), progInfo.MyName, buf);
            SetMsgSender(buf, msgb);   // Add domain to sender field
            stat = sendLogLk(Dest,msgb,NO_LOG);
            if(IS_ERROR(stat)) {
                LAO_NoMsgRou(msgb,stat);
                stat=NO_ERROR;
            }
         }
      } else {
         LAO_NoMsgRou(msgb,NO_CLIENT_ERROR);
      }
   }

   MUTEX_UNLOCK(&ClientTable.mutex,"RouteToClient");
   if(IS_ERROR(stat)) LAO_NoMsgRou(msgb,stat);
}


//@Function: SendToClient
//
// Sends a message to a client possibly via a peer
// 
// This routine is called to send a message to a client.
//
// The Client table mutex must have been locked by the caller
//
//@

static
void SendToClient(Client *sndCl,char *to)
{
    Client *Dest;
    MsgBuf *msgb=&(sndCl->rMsgb);
    int stat=NO_ERROR;

    Dest=GetClientByName(to,0);
    if(Dest) {
        if(Dest->Status!=CLIENT_REGISTERED)
            stat = CLIENT_STATUS_ERROR;
        else 
            stat = sendLogLk(Dest,msgb,DO_LOG); 
    } else {
        stat=NO_CLIENT_ERROR;         
    }
    if(IS_ERROR(stat)) 
       LAO_Error(sndCl, stat, "sending CLNREADY message to %s",to);
}


//@Function: IsClientReady       
//
// Checks if a client is in a ready state
//
// Returns 1 if yes, 0 if no or if the client is not found

static int IsClientReady( char *name)   // client name
{
    int stat;
    Client *cl;    

    MUTEX_LOCK(&ClientTable.mutex,"IsClientReady"); 

    cl=GetClientByName(name,0);

    stat = cl && cl->ClientReady;

    MUTEX_UNLOCK(&ClientTable.mutex,"IsClientReady"); 
    return stat;
}
  
//@Procedure: NotifyClientReady 
//
// Notifies that this client has become ready
//
// This routine is called when a client sends the SETREADY command
// to notify to all waiting clients that it has become ready.

// Note: the message used in reply will have the sequence numer set to zero.
//@

static int NotifyClientReady(Client *RdClient)
{
   int i;

   MUTEX_LOCK(&ClientTable.mutex,"NotifyClientReady");

   for(i=1; i<=ClientTable.maxCurItem; i++) {
      if(ClientTable.Items[i]!=NULL) {
         Client *Cl=ClientTable.Items[i];

         if(strncmp(Cl->Waiting,RdClient->Name,LONG_NAME_LEN)==0) {
            *(Cl->Waiting)='\0';
            FillHeader(0,Cl->WSeqN,RdClient->Name,Cl->Name,
                                     CLNREADY,&(RdClient->rMsgb));
            HDR_FLAGS(&(RdClient->rMsgb)) = NOHANDLE_FLAG|REPLY_FLAG;     // Send back right flag
            SendToClient(RdClient,Cl->Name);
         }
	   }
   }
   MUTEX_UNLOCK(&ClientTable.mutex,"NotifyClientReady");
   return NO_ERROR;
}

//@Function: GetVarList
//
// Retrieves a list of variables matching a given prefix expression
//@

static int GetVarList(Client *client,char *prefix)
{
   int stat,seqnum,logflg,count;
   Listscan listX;
   char vnbuf[VAR_NAME_LEN+1];
   char dnbuf[VAR_NAME_LEN+1];
   char *dom=rmVDomain(prefix,vnbuf);
   if(dom) {
      strncpy(dnbuf,dom,VAR_NAME_LEN);
      dnbuf[VAR_NAME_LEN]='\0';
   } else
      dnbuf[0]='\0';

   logflg=client->toLog;

   InitVarScan(vnbuf,&listX,DONTLOCKMUTEX);  // Initialize Variable list
	                                       // retrieval
   
   hdr_CODE(client->rMsgb)=VARREPLY; // Set VARREPLY code in msg
   seqnum=hdr_SEQN(client->rMsgb);

   MUTEX_LOCK(&(client->mutex),"GetVarList");
   count=0;
   for(;;) {                       // Scan variable list
      DB_Var *vpt;
      Variable *dst;
      localVar *src;
      int totlen,varlen;

      vpt=NextVarScan(&listX);

      if((vpt==NULL)||(++count>progInfo.maxRetMsgs)) {             // List is finished or max exceeded
         EndVarScan(&listX);
         FillHeader(0,seqnum,NULL,NULL,VAREND,&(client->rMsgb));
         if(vpt==NULL)
            hdr_PLOAD(client->rMsgb) = NO_ERROR;
         else {
            hdr_PLOAD(client->rMsgb) = VLIST_TRUNC_ERROR;
            LAO_Error(client,VLIST_TRUNC_ERROR,"");
         }
	      stat=sendLog(client,&(client->rMsgb),DO_LOG);
         if(IS_ERROR(stat))
            LAO_Error(client,stat,"sending VAREND code");
            
	      break;
	   }
      src=vpt->Var;
      varlen = VarSize(src->H.Type,src->H.NItems);
      totlen = varlen+sizeof(VHeader);

      stat=ResizeMsgBuf(&(client->rMsgb),totlen);
      if(IS_ERROR(stat)) {
         LAO_Error(client,stat,"in GetVarList()");
         break;
      }
      dst=(Variable *)msg_BODY(client->rMsgb);
      dst->H.Type=src->H.Type;
      dst->H.NItems=src->H.NItems;
      HDR_LEN(&(client->rMsgb))=totlen;
      makeName(dst->H.Name,vpt->Name,dnbuf);
      dst->H.Owner[0]='\0';
      memcpy(&(dst->H.MTime),&(src->H.MTime),sizeof(src->H.MTime));
      memcpy(&(dst->Value),&(src->Value),varlen);   // Write variable value
      stat=sendLog(client,&(client->rMsgb),DO_LOG);
      if(IS_ERROR(stat)) {
         LAO_Error(client,stat,"sending variable %s",vpt->Name);
         break;
      }
   }
   MUTEX_UNLOCK(&(client->mutex),"GetVarList");
   return stat;
}


//@Function: ReturnInfo
//
// Sends back info about MSGD
//@

#define BUF_SPACE 2048

static int ReturnInfo(Client *aClient)
{
static char* errfmt="returning MsgD info to %s";
   int lng,stat;
   char buf[BUF_SPACE];

   lng=LAO_InfoString(buf,BUF_SPACE);

   if(lng<0) {
      LAO_Error(NULL,INFO_TRUNC_ERROR,errfmt,aClient->Name);
      lng=BUF_SPACE;
   } 
   FillBody(buf,lng,&(aClient->rMsgb));
   hdr_CODE(aClient->rMsgb)=INFOREPLY;
   stat=sendLogLk(aClient,&(aClient->rMsgb),DO_LOG);
   if(IS_ERROR(stat))
      LAO_Error(NULL,stat,errfmt,aClient->Name);
   return NO_ERROR;
}

//@Function: ReturnHndl
//
// Sends back reply to HNDLINFO
//@

static int ReturnHndl(Client *aClient)
{
   char info[100];
   int msglen,stat;

   msglen=snprintf(info,100,"%s: no handlers defined",MySelf->Name);
   if(++msglen>100) msglen=100;
   FillBody(info,msglen,&(aClient->rMsgb));
   hdr_CODE(aClient->rMsgb)=INFOREPLY;
   stat=sendLogLk(aClient,&(aClient->rMsgb),DO_LOG);
   if(IS_ERROR(stat)) 
      LAO_Error(NULL,stat,"returning Handlers info to %s",aClient->Name);
   return NO_ERROR;
}

static void HaveI(Client *cl)
{
   int stat;
   char *clname=(char *)msg_BODY(cl->rMsgb);

   if(GetClientByName(clname,0))
     hdr_PLOAD(cl->rMsgb)=1;
   else
     hdr_PLOAD(cl->rMsgb)=0;

   hdr_CODE(cl->rMsgb)=ACK;
   hdr_LEN(cl->rMsgb)=0;
   stat=sendLogLk(cl,&(cl->rMsgb),DO_LOG);
   if(IS_ERROR(stat))
      LAO_Error(NULL,stat,"returning HaveI info to %s",cl->Name);
}

//@Function: GetCLlist
//
// Sends back the list of clients
//@

static int GetCLlist(Client *aClient)
{

static char *errfmt="returning client info to %s";
    int stat,seqnum,i,count;
#ifdef LONG_IS_64
	 struct timeval tv;
#endif
    struct {
       Variable var;
       char body[CLNT_INFO_LNG];
    } vb;



    vb.var.H.Type=CHAR_VARIABLE;       // Prepare a dummy variable header
    vb.var.H.NItems=0;
    vb.var.H.Owner[0]='\0';
    
#ifdef LONG_IS_64
	gettimeofday(&tv,NULL);
	vb.var.H.MTime = tv32_fm_tv(tv);
#else
    gettimeofday(&(vb.var.H.MTime),NULL);    

#endif
	 
   SetMsgCode(CLNTRPLY,&(aClient->rMsgb)); // Set message code
   seqnum=HDR_SEQN(&(aClient->rMsgb));

   MUTEX_LOCK(&(ClientTable.mutex),"GetCLlist");           // Critical section        *****
   MUTEX_LOCK(&(aClient->mutex),"GetCLlist");
   count=0;
   for(i=1;i<=ClientTable.maxCurItem;i++,count++) {
      if(count>=progInfo.maxRetMsgs) break;
      Client *cl;
      if((cl=ClientTable.Items[i])!=NULL) {     // Scan the list of clients.
         strncpy(vb.var.H.Name,cl->Name,PROC_NAME_LEN);
         vb.var.H.Name[PROC_NAME_LEN]='\0';
         vb.var.H.NItems=LAO_clientstr(cl,ClientTable.nConn[i],vb.var.Value.Sv,CLNT_INFO_LNG);
         FillBody(&vb,sizeof(vb),&(aClient->rMsgb));
         stat=sendLog(aClient,&(aClient->rMsgb),DO_LOG);
         if(IS_ERROR(stat)) {
            LAO_Error(NULL,stat,errfmt,aClient->Name);
            break;
         }
      }
   }
   FillHeader(0,seqnum,NULL,NULL,CLNTEND,&(aClient->rMsgb));
   stat=sendLog(aClient,&(aClient->rMsgb),DO_LOG);
   if(IS_ERROR(stat)) LAO_Error(NULL,stat,errfmt,aClient->Name);
   MUTEX_UNLOCK(&(aClient->mutex),"GetCLlist");
   MUTEX_UNLOCK(&(ClientTable.mutex),"GetCLlist");
   return stat;
}


//@Function: GetShmList
//
// Retrieves the list of shared buffers currently defined
//@

static int GetShmList(Client *aClient)
{
static char *errfmt="sending buffer info to %s";
    int stat,seqnum;
    ShmListscan listX;
    int count;
    struct {
       Variable var;
       char body[CLNT_INFO_LNG];
    } vb;
#ifdef LONG_IS_64
	struct timeval tv;
#endif
	 
    vb.var.H.Type=CHAR_VARIABLE;       // Prepare a dummy variable header
    vb.var.H.NItems=0;
    vb.var.H.Owner[0]='\0';
    
    
#ifdef LONG_IS_64
	 gettimeofday(&tv,NULL);
	 vb.var.H.MTime = tv32_fm_tv(tv);
#else
    gettimeofday(&(vb.var.H.MTime),NULL);    
#endif   

    seqnum=hdr_SEQN(aClient->rMsgb);
    hdr_CODE(aClient->rMsgb)=BUFREPLY; // Set BUFREPLY code in msg

    MUTEX_LOCK(&shmMgmtMutex,"GetShmList");
    
    InitShmList(&listX);            // Initialize Shared Buffer list retrieval
    count=0;
    MUTEX_LOCK(&(aClient->mutex),"GetShmList");
    while(1) {       // Scan shared buffer list
	int vv;

	vv=NextShmList(&listX);

        if(vv>0) {
            snprintf(vb.var.H.Name,VAR_NAME_LEN,"buf_%4.4d",count);
            vb.var.H.NItems=LAO_DumpBstring(&listX,vb.body,CLNT_INFO_LNG);
	    FillBody(&vb,sizeof(vb),&(aClient->rMsgb));
	    stat=sendLog(aClient,&(aClient->rMsgb),DO_LOG);
            if(IS_ERROR(stat)) {
                LAO_Error(NULL,stat,errfmt,aClient->Name);
                break;
            }
        } else {                          // List is finished
	    FillHeader(0,seqnum,NULL,NULL,BUFEND,&(aClient->rMsgb));
	    stat=sendLog(aClient,&(aClient->rMsgb),DO_LOG);
            if(IS_ERROR(stat)) 
                LAO_Error(NULL,stat,errfmt,aClient->Name);
	    break;
	}
    }
    MUTEX_UNLOCK(&(aClient->mutex),"GetShmList");
    MUTEX_UNLOCK(&shmMgmtMutex,"GetShmList");
    return stat;
}

//@Function: DetachConsumer

// Detaches a consumer from a shared buffer

// This function is essentially a wrapper around shmDetach which adds
// log end error management
//@

static int DetachConsumer(Client *cl, int bufid, int slot)
{
BufCom *info=&bPool.bInfo[bufid];
int ret;

ret=shmDetach(cl,bufid,slot,&shmMgmtMutex);

if(IS_NOT_ERROR(ret)) {
    LAO_Info(cl,"has been detached from buffer %s:%s (bufid:%d,slot:%d)", 
                                                              info->producer,
                                                              info->name, bufid,slot);
} else
    LAO_Error(cl,ret,"detaching from buffer %s:%s (bufid:%d,slot:%d)", 
                                                              info->producer,
                                                              info->name, bufid,slot);
return ret;
}

//@Function: DestroyBuffer

// Destroys a shared buffer 

// This function is called when a client which is a producer sends
// a detach request or unregisters from the Msgd-RTDB

// This function is essentially a wrapper around shmDestroy which adds
// log and error management.
//@

static int DestroyBuffer(Client* client, BufCom *inp)
{
char chbuf[200];
int bID=inp->bufID;
int stat;

    snprintf(chbuf,200,"%s:%s",inp->producer,inp->name);

    stat=shmDestroy(inp->bufID,&shmMgmtMutex);

    if(IS_ERROR(stat))
        LAO_Error(client,stat,"destroying shared buffer %s (bufid:%d)",chbuf,bID);
    else 
 	LAO_Info(client,"destroyed shared buffer %s (bufid:%d)",chbuf,bID);

    return stat;
}


//@Function: Broadcast 

// Broadcasts a message to all compatible clients
//
// This routine is called by the main client servicing routine to execute
// the command specified by a correctly received message.
//@

static int Broadcast(Client *aClient) // Client structure (contains message buffer)
                                      // Returns: Completion code
{
int i,stat;
    MUTEX_LOCK(&ClientTable.mutex,"Broadcast");

    for(i=1;i<=ClientTable.maxCurItem;i++)  {
	if(ClientTable.Items[i]==NULL) continue;
	if(ClientTable.Items[i]!=aClient) {        // Do not broadcast to me
            Client *Dest=ClientTable.Items[i];
            SetMsgDest(Dest->Name,&(aClient->rMsgb));
            stat=sendLogLk(Dest,&(aClient->rMsgb),DO_LOG); 
            if(IS_ERROR(stat))
                LAO_Error(aClient,stat,"broadcasting to %s",Dest->Name);
        }
    }

    MUTEX_UNLOCK(&ClientTable.mutex,"Broadcast");
    return NO_ERROR;
}

static int LogCntrl(Client *client,
                    int    spec,
                    char *clname)
{
Client *target;
char *what;
int stat=NO_ERROR;

if(spec!=0) {
   spec=1;
   what="enabled log for client %s";
} else {
   spec=0;
   what="disabled log for client %s";
}

MUTEX_LOCK(&ClientTable.mutex,"LogCntrl");

if(*clname=='*') {  // Operate on all clients
    int i;
    for(i=1;i<=ClientTable.maxCurItem;i++) {
	if((target=ClientTable.Items[i])==NULL) continue;
        target->toLog=spec;
        LAO_Info(client,what,target->Name);
    }
} else {
    target=GetClientByName(clname,STD_CLIENT);

    if(target) {
       target->toLog=spec;
       LAO_Info(client,what,target->Name);
    } else {
       stat=NO_CLIENT_ERROR;
       LAO_Error(client,stat,"enabling/disabling log");
    }
}
MUTEX_UNLOCK(&ClientTable.mutex,"LogCntrl");
return NO_ERROR;
}


//@Function: WaitClientMsg 

// Inizial Rendez-vous with client
//
// This routine is called by the main client servicing routine to perform
// the intial Rendez-vous when a client requests connection
//@

static int WaitClientMsg(Client *cl) // Client structure
                                     // Returns: Completion code
{
int stat;

if(IS_ERROR(stat=WaitMessage(cl->conn.sock, &(cl->rMsgb)))) {
    LAO_Error(cl,stat,"in WaitClientMsg()");
    if(PLAIN_ERROR(stat)!=CLOSEDCONN_ERROR) {
        if(cl->rMsgb.hd_ok) {
            char buf[140];
            LAO_Error(cl,stat,"Header OK - %s",StrHeader(&(cl->rMsgb),buf,140));
        } else {
            char buf[500];
            LAO_Error(cl,stat,"Corrupt header - %s",HexHeader(&(cl->rMsgb),buf,500));
        }
    }
}
return stat;
}

static void RegisterReply(Client *cl, int stat)
{
   MsgBuf *imsg = &(cl->rMsgb);

   SetMsgSender(progInfo.MyName,imsg); // Set sender name
   if(cl->clType==PEER_MSGD)
      SetMsgDest("",imsg);           // Clear destination name
   else
      SetMsgDest(cl->Name,imsg);     // Set destination name
   
   SET_REPLY(imsg);                  // Set reply flag

   if(IS_NOT_ERROR(stat)) {
       int pLoad,len;
       char *pt=GetVersionID();
                                    // Create client status variable
       pLoad=cl->Id<<16;                    // Generate unique seed for sequence numbers
       SetMsgCode(ACK,imsg);
       len=strlen(pt)+1;
       FillBody(pt, len, imsg);    // Send back identification into ACK message
       SetMsgPload(pLoad,imsg);    // Send back sequence seed in payload
       LAO_Info(cl,"<-- sending identification. SeqSeed: %d",pLoad);
   } else {
       SetMsgCode(NAK,imsg);
       SetMsgLength(0,imsg);
       SetMsgPload(stat,imsg);  // Return error code in payload
       cl->Name[0]='*';         // Fake client's name for logging
       LAO_Info(cl,"<-- sending NACK");
   }
   stat=sendLogLk(cl,imsg,DO_LOG); // Send reply to client
   if(IS_ERROR(stat))
       LAO_Error(cl,stat,"replying to register request");
}


//@Function: ClientLoop 

// Receives commands from clients and executes them
//
// This routine is called by the main client servicing routine to loop
// executing commands from a client.
//@

static void ClientLoop(Client *cl) // Client structure
                                   // Returns: Completion code
                                   // 0: No error
                                   // <0: Error
                                   // 10: Unregister request
{
int stat,msgC,pLoad,aux,svln,loop;
Variable *theVar;
MsgBuf* imsg;
char *to;
char* rplyfmt="sending reply";
stat=1; loop=1;

LAO_Info(cl,"Starting client loop");
if(cl->clType==PEER_MSGD) {              // Force peers to ready status
   cl->ClientReady=1;                    // ready flag
   NotifyClientReady(cl);                // Notify to waiting clients
   WriteClientVar(cl,CLIENT_IS_READY);   // Set client status variable
}
while(loop) {                 // Loop waiting messages from client
   char *dompt;
   if(IS_ERROR(stat=WaitClientMsg(cl))) {
      loop=0;
      continue;
   }
                                    // Message has been received. Now execute it
   imsg = &(cl->rMsgb);
   if(HDR_MAGIC(imsg)==43) HDR_MAGIC(imsg)=MSG_HEADER_MAGIC;
   msgC = HDR_CODE(imsg);
   pLoad = HDR_PLOAD(imsg);
   to=HDR_TO(imsg);

   if(strncmp(to,"BCAST",5)==0) {  // Broadcast message
      if(cl->toLog) LAO_LogMsgRec(imsg);
      stat=Broadcast(cl);
      if(IS_ERROR(stat)) LAO_Error(cl,stat,"Broadcasting message");
      continue;
   }
   if(to[0]!='\0') {  // Check if message must be routed
      RouteToClient(cl);
      continue;
   } 
                                            // The message is for me. 
                                            // I've got do some work
   if(cl->toLog) LAO_LogMsgRec(imsg);
   ReverseHeader(imsg);                     // Prepare buffer for reply

   if((dompt=strchr(to,'@'))) {             // The mesasge comes from
      char buf[LONG_NAME_LEN+1];            // remote domain
      *dompt='\0';                          // Remove domain from dest field
      mkFullName(HDR_FROM(imsg), progInfo.MyName, buf);
      SetMsgSender(buf, imsg);              // Add domain to sender field
   } 

   strncpy(cl->Remote,to,PROC_NAME_LEN);    // Store remote client name,
   cl->Remote[PROC_NAME_LEN]='\0';          // if client is local, copy name

   SET_REPLY(imsg);                         // Set reply flag

//----------------------------------- Command decode loop
   switch(msgC) {
   char *name;
   str_LOGSTAT *logstat;
   char *pta;

   CList *NList;
   BufCom *inp, *retInfo;

   case GETVAR:                //--------------------------- MsgCode: GETVAR

      name=(char *)msg_BODY(cl->rMsgb);
      if(IS_NOT_ERROR(stat=GetVariable(name,cl,imsg)))
         HDR_CODE(imsg)=VARREPLY;
      else {
         if(stat==VAR_LOCKED_ERROR)
            HDR_CODE(imsg)=VARLOCKD;
         else
            HDR_CODE(imsg)=VARERROR;
         HDR_LEN(imsg)=0;
      }
      stat = sendLogLk(cl,imsg,DO_LOG);
      if(IS_ERROR(stat)) LAO_Error(cl,stat,rplyfmt);
      break;


//------------------------------------------------------------- MsgCode: SETVAR
   case SETVAR:
      theVar=(Variable *)msg_BODY(cl->rMsgb);

      if((pta=strchr(theVar->H.Name,'@'))) *pta='\0';        // Remove domain from var name
      if(IS_NOT_ERROR(stat=SetVariable(theVar,cl,&NList))) 
         stat=VarNotify(cl,NList);                           // Notify registered clients
      break;

//------------------------------------------------------------- MsgCode: VARMIRROR
   case VARMIRROR:
      theVar=(Variable *)msg_BODY(cl->rMsgb);
  
      stat=WriteVariable(theVar,cl,&NList);
      if(IS_ERROR(stat)) 
         LAO_Error(cl,stat,rplyfmt);
      else
         stat=VarNotify(cl,NList);       // Variable written, notify registered clients
      break;
//------------------------------------------------------------- MsgCode: WRITEVAR
   case WRITEVAR:
      theVar=(Variable *)msg_BODY(cl->rMsgb);

      if((pta=strchr(theVar->H.Name,'@'))) *pta='\0';        // Remove domain from var name
      stat=WriteVariable(theVar,cl,&NList);
      if(IS_NOT_ERROR(stat)) {
         HDR_CODE(imsg)=VARWRITN;
         HDR_PLOAD(imsg)=0;
      } else {
         HDR_CODE(imsg)=VARERROR;
         HDR_PLOAD(imsg)=stat;
      }
      svln=HDR_LEN(imsg);    // Save message lenght for notify
      HDR_LEN(imsg)=0;       // Reply message needs only header
      aux = sendLogLk(cl,imsg,DO_LOG);  // Return status to client
      if(IS_ERROR(aux)) LAO_Error(cl,aux,rplyfmt);

      if(IS_NOT_ERROR(stat)) {
         HDR_LEN(imsg)=svln;            // Restore message length
         stat=VarNotify(cl,NList);      // Variable written, notify registered clients
      }
      break;

//----------------------------------------------------------- MsgCode: VARLIST
   case VARLIST:
      stat = GetVarList(cl,(char *)MSG_BODY(imsg));
      break;


//----------------------------------------------------------- MsgCode: LOGSTAT
   case LOGSTAT:
      logstat=(str_LOGSTAT *)msg_BODY(cl->rMsgb);
      LAO_remLog(cl,logstat->lev,logstat->seq,logstat->txt);
      break;

//----------------------------------------------------------- MsgCode: LOGCNTRL
   case LOGCNTRL:
      stat = LogCntrl(cl,HDR_PLOAD(imsg),(char *)MSG_BODY(imsg));
      break;

//----------------------------------------------------------- MsgCode: SHMREQST
   case SHMREQST:

      inp = (BufCom *)msg_BODY(cl->rMsgb);
      stat=shmAttach(cl,inp,&shmMgmtMutex);
                                                    
      if (IS_ERROR(stat)) {     // Some error detected
         LAO_Error(cl,stat,"attaching to buffer %s:%s (bufid:%d)", 
                                                         inp->producer,
                                                         inp->name,
                                                         inp->bufID);
         HDR_PLOAD(imsg)=stat;          // Store return status 
      } else {
         retInfo=&bPool.bInfo[stat];                     // Set return status
         stat=(retInfo->accMode&ACCMODE_MODE_MASK) | CONSUMER;
         LAO_Info(cl,"has been attached to buffer %s:%s (bufid:%d,slot:%d)",
                                 retInfo->producer, retInfo->name,
                                 retInfo->bufID,retInfo->slot);
         memcpy(msg_BODY(cl->rMsgb),retInfo,sizeof(BufCom));
         HDR_PLOAD(imsg)=NO_ERROR;      // Store return status 
      }
      inp->accMode=stat;
      stat = sendLogLk(cl,imsg,DO_LOG);
      if(IS_ERROR(stat)) LAO_Error(cl,stat,rplyfmt);
      break;

//-------------------------------------------------------------- MsgCode: ECHO
   case ECHO:
      HDR_CODE(imsg)=ECHOREPLY;
      stat = sendLogLk(cl, imsg,DO_LOG);
      if(IS_ERROR(stat)) LAO_Error(cl,stat,rplyfmt);
      break;


//----------------------------------------------------------- MsgCode: LOCKVAR
   case LOCKVAR:
      if(HDR_PLOAD(imsg))
         stat = LockVariableSet((char *)MSG_BODY(imsg),cl);
      else
         stat = UnlockVariableSet((char *)MSG_BODY(imsg),cl);
      if(IS_NOT_ERROR(stat))
         HDR_CODE(imsg)=ACK;
      else
         HDR_CODE(imsg)=NAK;
      SetMsgLength(0, imsg);
      stat = sendLogLk(cl,imsg,1);
      if(IS_ERROR(stat)) LAO_Error(cl,stat,rplyfmt);
      break;

//------------------------------------------------------------ MsgCode: BUFLIST
   case BUFLIST:
      stat = GetShmList(cl);
      break;

//----------------------------------------------------------- MsgCode: SETREADY
   case SETREADY:
      cl->ClientReady=1;                         // ready flag
      NotifyClientReady(cl);                     // Notify to waiting clients
      WriteClientVar(cl,CLIENT_IS_READY);        // Set client status variable
      break;

//----------------------------------------------------------- MsgCode: VARNOTIF
   case VARNOTIF:
      if(HDR_PLOAD(imsg))
         stat = NotifyMe(cl,(char *)MSG_BODY(imsg));
      else 
         stat = NoNotifyMe(cl,(char *)MSG_BODY(imsg),0);
      break;

//----------------------------------------------------------- MsgCode: GIVEVAR
   case GIVEVAR:
      LAO_Info(cl,"WARNING: obsolete command GIVEVAR");
      break;

//----------------------------------------------------------- MsgCode: TAKEVAR
   case TAKEVAR:
      LAO_Info(cl,"WARNING: obsolete command TAKEVAR");
      break;
//----------------------------------------------------------- MsgCode: TRACEVAR
   case TRACEVAR:
      stat = VarTrace(cl,(char *)MSG_BODY(imsg),HDR_PLOAD(imsg));
      break;

//----------------------------------------------------------- MsgCode: WAITCLRDY
    case WAITCLRDY:

        if(strcmp(cl->Name,(char*)msg_BODY(cl->rMsgb))==0) { // Cannot wait for myself
            LAO_Error(cl,WAIT_MYSELF_ERROR,"");
            break;
        }
                                                  // Check if the client asked
                                                  // for is immediately ready
        if(IsClientReady((char*)msg_BODY(cl->rMsgb))) {
            HDR_CODE(imsg)=CLNREADY;   // Prepare code for reply
            SetMsgLength(0,imsg);
            stat = sendLogLk(cl,imsg,1);
            break;
        }
        if(cl->Waiting[0]!='\0') {   // Client is already waiting for someone
            LAO_Error(cl,WAITING_ALREADY_ERROR,"");
            break;
        }
        strncpy(cl->Waiting,(char*)msg_BODY(cl->rMsgb),LONG_NAME_LEN);
        cl->Waiting[LONG_NAME_LEN]='\0';

        LAO_Info(cl,"is waiting for %s to be ready",cl->Waiting);
        cl->WSeqN=HDR_SEQN(imsg);          // Remember sequence number
        stat = NO_ERROR;
        break;

//----------------------------------------------------------- MsgCode: WAITCANCEL
   case WAITCANCEL:
      LAO_Info( cl,"cancelled client ready wait");
      cl->Waiting[0]='\0';
      stat = NO_ERROR;
      break;
//---------------------------------------------------------- MsgCode: TERMINATE
   case TERMINATE:
      LAO_Info( cl,"requests a shutdown");
                                 // We just send a SIGKILL to the main thread. 
                                 // He will provide to cleanup all running 
                                 // threads, including this one
      kill(progInfo.MainPID, SIGHUP);  // Terminate the main thread
      stat = NO_ERROR;
      break;
//------------------------------------------------------------ MsgCode: CREAVAR
   case CREAVAR:
      theVar=(Variable *)msg_BODY(cl->rMsgb);
      stat = CreaVariable(theVar,cl);
      break;

//------------------------------------------------------------- MsgCode: DELVAR
   case DELVAR:
      if(IS_NOT_ERROR(stat=DeleteVariable((char *)MSG_BODY(imsg),cl))) {
         stat = sendLogLk(cl,imsg,1);         // Reply
         if(IS_ERROR(stat)) LAO_Error(cl,stat,"Sending DELVAR ACK");
      } else {
         SetMsgCode(NAK,imsg);                    // prepare code for reply
         stat = sendLogLk(cl,imsg,1);             // Reply
         if(IS_ERROR(stat)) LAO_Error(cl,stat,"Sending DELVAR NAK");
      }
      break;

//------------------------------------------------------------ MsgCode: CLNDISCON
   case CLNDISCON:
      strncpy(cl->Remote,(char *)MSG_BODY(imsg),PROC_NAME_LEN); // Store remote client name,
      cl->Remote[PROC_NAME_LEN]='\0';   
      stat = NoNotifyMe(cl,NULL,1);               // Remove remote client from notifications
      break;
//------------------------------------------------------------ MsgCode: SHMCREA
   case SHMCREA:
      inp = (BufCom *)msg_BODY(cl->rMsgb);
            
      stat=shmCreate(inp,&shmMgmtMutex);
      if (stat<0) {                            // Some error detected
         LAO_Error(cl,stat,"Creating shared buffer %s:%s (%d bytes)",
                                          cl->Name,inp->name,inp->lng);
      } else  {
         BufCom *retInfo = &bPool.bInfo[stat];
         inp->accMode=(retInfo->accMode&ACCMODE_MODE_MASK)|PRODUCER;
         LAO_Info(cl,"created shared memory buffer %s:%s (%d bytes bufid:%d Magic:%d)",
                                                      retInfo->producer,retInfo->name,
                                                      retInfo->lng,retInfo->bufID,
                                                      retInfo->uniq);
         memcpy(msg_BODY(cl->rMsgb),retInfo,sizeof(BufCom));
      }

      HDR_PLOAD(imsg)=stat;
      stat = sendLogLk(cl,imsg,1);
      if(IS_ERROR(stat)) LAO_Error(cl,stat,"Sending SHMCREA ACK/NAK");
      break;
//------------------------------------------------------------------------ MsgCode: SHMDETACH
   case SHMDETACH:
      inp = (BufCom *)msg_BODY(cl->rMsgb);
            
      if(((inp->accMode&ACCMODE_TYPE_MASK)==PRODUCER) ||
         ((inp->accMode&ACCMODE_MODE_MASK)==BUFSNAP)    )
          stat = DestroyBuffer(cl,inp);
      else 
          stat = DetachConsumer(cl,inp->bufID,inp->slot);
      break;
//------------------------------------------------------------ MsgCode: HAVEYOUCL
   case HAVEYOUCL:
      HaveI(cl);
      break;
//------------------------------------------------------------ MsgCode: CLNTLIST
   case CLNTLIST:
      stat = GetCLlist(cl);
      break;
//------------------------------------------------------------ MsgCode: CLOSELOG
   case CLOSELOG:
      SetLogfile(LOG_REOPEN);               // Reopen log file (if any)
      break;
//------------------------------------------------------------ MsgCode: DUMPVARS
   case DUMPVARS:
      stat= LAO_DumpVarList(cl,(char *)MSG_BODY(imsg));
      break;
//------------------------------------------------------------ MsgCode: DUMPCLNS
   case DUMPCLNS:
      stat = LAO_DumpCLtable();
      break;
//------------------------------------------------------------ MsgCode: DUMPBUFS
   case DUMPBUFS:
      stat = LAO_DumpBufsList();
      break;
//------------------------------------------------------------ MsgCode: DUMPINFO
   case DUMPINFO:
      stat = LAO_DumpInfo(0);
      break;
//------------------------------------------------------------ MsgCode: DUMPNOTIF
   case DUMPNOTIF:
      stat = LAO_DumpNotifTable(1);
      break;
//------------------------------------------------------------ MsgCode: DUMPVSTAT 
   case DUMPVSTAT:
      stat = LAO_DumpVstat();
      break;
//------------------------------------------------------------ MsgCode: RQSTINFO
   case RQSTINFO:
      stat = ReturnInfo(cl);
      break;
//------------------------------------------------------------ MsgCode: LOGINCR
   case LOGINCR:
      stat=IncrDebugLevel();
      LAO_Info(cl,"increased log verbosity to %d",stat);
      stat = NO_ERROR;
      break;
//------------------------------------------------------------ MsgCode: LOGDECR
   case LOGDECR:
      stat=DecrDebugLevel();
      LAO_Info(cl,"decreased log verbosity to %d",stat);
      stat = NO_ERROR;
      break;
//------------------------------------------------------------ MsgCode: LOGLEVSET
   case LOGLEVSET:
      stat=HDR_PLOAD(imsg);
      stat=SetDebugLevel(stat);
      LAO_Info(cl,"set log verbosity to %d",stat);
      stat = NO_ERROR;
      break;
//------------------------------------------------------------ MsgCode: SETTMOUT
   case SETTMOUT:
      stat=HDR_PLOAD(imsg);
      if(stat<(10) || stat > 999999)
         LAO_Error(cl,stat,"illegal netIO timeout: %d",stat);
      else {
         progInfo.netIoTmout=stat;
         LAO_Info(cl,"set netIO timeout to %d",stat);
      }
      stat = NO_ERROR;
      break;
//------------------------------------------------------------ MsgCode: HNDLINFO
   case HNDLINFO:
      stat = ReturnHndl(cl);
      break;
//------------------------------------------------------------ MsgCode: UNREGISTER
   case UNREGISTER:
      LAO_Info(cl,"Client unregister");
      loop=0;
      break;
//------------------------------------------------------------ MsgCode: GETVERSION
   case GETVERSION:
      HDR_CODE(imsg)=VERSREPLY;
      HDR_PLOAD(imsg)=GetVersion();
      stat = sendLogLk(cl, imsg,DO_LOG);
      if(IS_ERROR(stat)) LAO_Error(cl,stat,rplyfmt);
      break;
//------------------------------------------------------------ MsgCode: GETSTARTT
   case GETSTARTT:
      HDR_CODE(imsg)=STARTREPLY;
      stat=getStartT(imsg);
      if(IS_NOT_ERROR(stat)) {
         stat = sendLogLk(cl, imsg,DO_LOG);
         if(IS_ERROR(stat)) LAO_Error(cl,stat,rplyfmt);
      }
      break;
//------------------------------------------------------------ MsgCode: LOGFREEZE
   case LOGFREEZE:
      LAO_Info(cl,"Freezing for %d ms",HDR_PLOAD(imsg));
      LAO_Freeze(1);
      msleep(HDR_PLOAD(imsg));
      LAO_Freeze(0);
      LAO_Info(cl,"Freezing done");
      stat=NO_ERROR;
      break;
//---------------------------------------------------------------------------- MsgCode: ???
   default:                         // If we get here, the message has not been recognised
      stat= ILLEGAL_MSG_ERROR;
      LAO_Error(cl,stat,"received illegal code: %4.4x",msgC);
      break;
   }
}
LAO_Info(cl,"Client loop ends");
}


//@Function{API}: FreeClient 
//
// Releases resources when a client terminates.
//
// This routine is called upon termnination of client's thread
// to release resources used by client
//@

int FreeClient(Client *aClient)  // Pointer to client structure
                                 // Returns status code
{
MUTEX_DESTROY(&(aClient->mutex),"ClientMutex");
FreeMsgBuf(&(aClient->rMsgb));
free(aClient);

return NO_ERROR;
}



//@Function: CleanClient 
//
// Cleans everything up when a client or peer terminates.
//
// This routine is called just before exiting from ClientConnection routine
// in order to close sockets, deallocate buffers and so on
//@

int CleanClient(Client *aClient)  // Pointer to client structure
                                  // Returns status code
{
int ret,bufid,slot;
Listscan listX;
DB_Var *vpt;

LAO_Debug(aClient,"Removing client from services (buffer lists, notify lists, etc.)",aClient);
                              // Search the shared buffer consumer list to see
                              // whether we are listed as consumer of some producer
for(bufid=0;bufid<=bPool.maxUsedBuffer;bufid++) {     // Loop over all buffers
    BufCom *inp = &(bPool.bInfo[bufid]);
    for(slot=0;slot<inp->maxC;slot++) { // Loop over all consumers
        if(bPool.cInfo[bufid].cList[slot]==aClient) { // if slot not empty
             ret=DetachConsumer(aClient,bufid,slot);
                                     // In snapshot mode, also destroy producer
             if(inp->accMode&BUFSNAP)
                 ret=DestroyBuffer(MySelf,inp);
        }
    }
}
                              // Check wether we are producers of some shared buffer

for(bufid=0;bufid<=bPool.maxUsedBuffer;bufid++) {     // Loop over all buffers
    BufCom *inp = &(bPool.bInfo[bufid]);
    if(strncmp(aClient->Name,inp->producer,PROC_NAME_LEN)==0) {
                                                      // Force detachment of all consumers
        for(slot=0;slot<inp->maxC;slot++) {    // Loop over all consumers
            Client *cl;
            if((cl=bPool.cInfo[bufid].cList[slot])!=NULL)    // if slot not empty
                ret=DetachConsumer(cl,bufid,slot);           // Detach the client
        }
        ret=DestroyBuffer(MySelf,inp);                       // Destroy the shared buffer
    }
}
                                         // Remove all variable notifications
NoNotifyMe(aClient, NULL, 1);
                                         // And variable locks
InitVarScan("*",&listX,LOCKMUTEX);
while ((vpt = NextVarScan(&listX))) {
    if(vpt->Lock == aClient->Id)
        UnlockVariable(vpt,aClient);
}
EndVarScan(&listX);

return NO_ERROR;
}

//@Function: NotifyPeers
//
// Notifies all peers that a local client disconnected
//
// This functions sends messages to all peers notifying that
// the specified client has disconnected
//
//@

static void NotifyPeers(Client *cl) //@P{cl}: disconnecting client
{
   MsgBuf *msgb=&(cl->rMsgb);
   int stat;

   MUTEX_LOCK(&ClientTable.mutex,"NotifyPeers");

   stat=FillMessage(PROC_NAME_LEN+1,0,"","",CLNDISCON,cl->Name,msgb);

   if(IS_NOT_ERROR(stat)) 
      BcastToPeers(cl,1);
   else
      LAO_Error(NULL,stat,"Notifying to peers disconnection by %s",cl->Name);

   MUTEX_UNLOCK(&ClientTable.mutex,"NotifyPeers");
}

//@Function{API}: ClientConnection
//
// Manages a single client connection
//
// This procedure is called by pthread\_create to define a new
// thread which manages the communication with a client
//
// The thread remains active until the client closes the corresponding
// socket. It is declared detached so that it dies when the routine
// returns.
//@

int ClientConnection(Client *cl)  // New client data
                                  // Returns status code
{
int stat;
MsgBuf *imsg;
int msgC, pLoad;

cl->thId=pthread_self();
cl->Status=CLIENT_JUST_CONNECTED;
stat=NO_ERROR;

LAO_Info(NULL,"Client at addr:0x%lx initialized on thread: 0x%lx. Initial rendez-vous",cl,cl->thId);
LAO_Debug(NULL,"Client at addr:0x%lx info: sock:%d IP:%s",cl,cl->conn.sock,cl->conn.ip);

cl->enabled=1;                           // Set client enable flag
if(IS_NOT_ERROR(stat=WaitClientMsg(cl))) {

   imsg = &(cl->rMsgb);
   msgC = HDR_CODE(imsg);
   pLoad = HDR_PLOAD(imsg);

   if(msgC!=REGISTER) return CLIENT_STATUS_ERROR;

   strncpy(cl->Name,HDR_FROM(imsg),PROC_NAME_LEN);
   cl->Name[PROC_NAME_LEN]='\0';

   if(HDR_MAGIC(imsg)==43) {
      cl->prot43=1;
      HDR_MAGIC(imsg)=MSG_HEADER_MAGIC;
   }
   stat=RegisterClient(cl,pLoad);   // Register the new client
   RegisterReply(cl,stat);
   if(IS_NOT_ERROR(stat)) {
        WriteClientVar(cl,CLIENT_IS_REGISTERED);
        ClientLoop(cl);             // Client loop. Exit on termination
        cl->enabled=0;              // Clear client enable flag
        WriteClientVar(cl,CLIENT_IS_UNREGISTERED);
        LAO_Info(cl,"Client at addr:0x%lx terminating and cleaning up",cl);
        NotifyPeers(cl);
        CleanClient(cl);
        CancelClient(cl);         // Remove Client from client list
    } else {
       cl->enabled=0;            // Clear client enable flag
       LAO_Debug(NULL,"Unregistered client at addr:0x%lx closing sock:%d",cl,cl->conn.sock);
       close(cl->conn.sock);
    }
}
LAO_Info(NULL,"Client thread: 0x%lx terminated",pthread_self());
FreeClient(cl);
return NO_ERROR;
}

//@Function{API}: PeerConnection
//
// Manages a single peer connection
//
// This procedure is called by pthread\_create to define a new
// thread which manages the communication with a peer which requested connection
//
// The thread remains active until the client closes the corresponding
// socket. It is declared detached so that it dies when the routine
// returns.
//@

int PeerConnection(Client *pe)  // New client data
                                // Returns status code
{
int stat;

pe->thId=pthread_self();
pe->Status=CLIENT_JUST_CONNECTED;

LAO_Info(NULL,"Peer at ma:0x%lx initialized on thread 0x%lx.",pe,pe->thId);
LAO_Debug(NULL,"Peer at addr:0x%lx info: sock:%d IP:%s",pe,pe->conn.sock,pe->conn.ip);

pe->enabled=1;                       // Set client enable flag
stat=RegisterClient(pe,PEER_MSGD);   // Register the new client
// RegisterReply(pe,stat);
if(IS_NOT_ERROR(stat)) {
     WriteClientVar(pe,CLIENT_IS_REGISTERED);
     ClientLoop(pe);
     pe->enabled=0;            // Clear client enable flag
     WriteClientVar(pe,CLIENT_IS_UNREGISTERED);
     LAO_Debug(pe,"Peer at addr:0x%lx cleaning up",pe);
     CleanClient(pe);
     CancelClient(pe);         // Remove Client from client list
} else {
     pe->enabled=0;            // Clear client enable flag
     LAO_Debug(NULL,"Unregistered peer at addr:0x%lx closing sock:%d",pe,pe->conn.sock);
     close(pe->conn.sock);
}
LAO_Info(NULL,"Peer thread: 0x%lx terminated",pthread_self());
FreeClient(pe);
return NO_ERROR;
}
