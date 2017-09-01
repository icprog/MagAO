//@File: msgdrtdb.c
//
// Message Daemon and Real-Time Database main program
//
// This file contains code for the main program of the Message Daemon 
// and Real-Time Database which is part of the LBT Adaptive Optics Supervisor.
//@

//@Main: MsgD-RTDB

// Message Daemon and Real-Time Database

// This program is a common server to all AO Supervisor processes.
// It provides a message passing mechanism for communication among processes
// and operates as a repository for a shared set of variables. Communication
// of clients is network based so that the clients can operate in a
// distributed environment.
//
// Any AO Supervisor process will start by connecting to MsgD-RTDB in order
// to be able to use its services. After that it will be able to send messages
// to other connected processes and to create, read, write variables in the
// shared repository.
//
// MsgD-RTDB provides support for managing shared memory buffers to be used 
// for data communication when the amount of data and/or the related data rate
// is too high to use the variable repository. In this case data exchanges use 
// the underlying shared memory support, so these functions are available only 
// to processes running on the same machine.
//
// Other than communication and variable services MsgD-RTDB also provides support
// for a centralized logging facility.
//
// Several MsgD-RTDB's running on different workstations, together with their own
// set of clients, may be clustered together to create more complex architectures.
//
// Two API's are provided to the programmer in order to use MsgD-RTDB services:
// 1) a lower level C library defining procedures to manage the message exchange 
// and the variable operations and 2) two C++ classes to perform similar functions
// in an OO framework.
//@


#include <stdio.h> 
#include <stdarg.h> 
#include <stdlib.h> 
#include <string.h> 
#include <errno.h> 
#include <ctype.h> 
#include <unistd.h> 
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h> 
#include <netinet/tcp.h> 
#include <arpa/inet.h> 

#include <signal.h> 

#include <pthread.h> 

#include "msgdrtdb.h"

#include "configlib.h"

#include "table.h"
#include "shmlib.h"
#include "rtdb.h"
#include "client.h"
#include "logerr.h"

static Client me;
Client *MySelf=&me;

#define PG_IDENTITY_LNG 100

char ident[PG_IDENTITY_LNG];

struct PGINFO progInfo;

PeerTable peerTable;


int UniqGenerator = 19520709;  // Counter to generate unique numbers.
                               // Used in shmlib.c to uniquely identify
                               // shared memory buffers

long errCount=0;

pthread_mutexattr_t mutexAttributes;

pthread_mutex_t shmMgmtMutex;
pthread_mutex_t LogMutex;

CLtable ClientTable;

VarTable variableTB;

SortedTable notifyTB;
// SortedTable mirrorTB;

//@Procedure: help

//This  routine is called to show a brief program usage help.

//Usually after calling help() the program should exit.
//@


static void help()
{
    printf("\nMSGDRTDB  %s.   L.Fini\n\n", GetVersionID());
    printf("Usage: msgdrtdb [options] \n\n");
    printf("       -c name:ip:p[,name:ip:p]  Connect to peers at given addresses\n");
    printf("                                 (test only: peers are usually defined \n");
    printf("                                  in configuration file)\n\n");
    printf("       -d dir   Look for config and variable setup files in <dir>\n");
    printf("       -D       Enable message debug output\n");
    printf("       -h       print this message and exit.\n\n");
    printf("       -i idn   specify MsgD identity.\n\n");
    /*
    printf("       -g pref  Specify the prefix of variables to be mirrored. It may be\n");
    printf("                repeated. The character * must be escaped. Mirrored (global)\n");
    printf("                variables can also be defined in a file named msgdrtdb.mvar\n");
    printf("                in the configuration directory\n");
    */
    printf("       -l dir   specify logging directory (overrides config and environment).\n\n");
    printf("       -n       do not open the log file, i.e.: send log to stdout\n\n");
    printf("       -N       do not read any config file (debug only)\n\n");
    printf("       -p port  listen on given port (overrides environment and config)\n\n");
    printf("       -q       decrease log verbosity (use more -q for less verbosity)\n\n");
    printf("       -s       disable active peer connection task (but still accept\n");
    printf("                passive peer connections): used for debug only\n");
    printf("       -v       increase log verbosity (use more -v for more verbosity)\n\n");
    printf("\n");
    printf("Configuration file search order:\n");
    printf("  1. -d <dir>/msgdrtdb.conf\n");
    printf("  2. $ADOPT_ROOT/conf/$ADOPT_SUBSYSTEM/current/msgdrtdb.conf\n");
    printf("  3. $ADOPT_ROOT/conf/msgdrtdb.conf\n");
    printf("  4. ./msgdrtdb.conf\n\n");

}


//@Procedure: sighand
//
//  Signal handler for terminating the program
//@

static void sighand(int thesig)
{
   if(thesig==SIGINT) CleanExit(TERM_KBINTERR);
   if(thesig==SIGHUP) CleanExit(TERM_CLNTREQ);
   CleanExit(0);
}

static void checkClients(void)
{
int i;
MUTEX_LOCK(&ClientTable.mutex,"checkClients");

LAO_VTrace(NULL,"Checking client table for clients to be removed");

for(i=1; i<=ClientTable.maxItem; i++) {
    Client *cl=ClientTable.Items[i];

    if(cl && cl->toclose) {
        LAO_Error(cl,EXCD_BLOCK_ERROR,"Client: %s. Closing it",cl->Name);
        RemoveClient(i);
        CleanClient(cl);
        LAO_Info(cl,"Client thread: 0x%lx terminated",cl->thId);
        pthread_cancel(cl->thId);
        FreeClient(cl);
    }
}

MUTEX_UNLOCK(&ClientTable.mutex,"checkClients");
}


static int autodump=AUTODUMP*1000;   // Info dump interval (sec) (may be overridden
                                     // in config file)


// The following routine must be run in a thread. It will provide
// Housekeeping services. It is awakened every HOUSEKEEPSTEP milliseconds
// to perform the following tasks:
//
// 1. Check for clients to shut down
// 2. Periodic info dump according to value of the autodump variable

static void housekeeping(void)
{
   int dumpcounter=0;

   for(;;) {     // Main loop
        msleep(HOUSEKEEPSTEP);

        checkClients();
        if(autodump>0) {
            dumpcounter +=HOUSEKEEPSTEP;
            if( dumpcounter>=autodump) {
                LAO_DumpInfo(0);
                LAO_DumpVstat();
                LAO_DumpCLtable();
                LAO_DumpBufsList();
                LAO_DumpNotifTable(1);
                dumpcounter=0;
           }
       }
   }
}

static int parsePeers(char *arg)
{
int id;
char *buffer=strdup(arg);
char *src;

for(id=0;id<MAXPEERS;id++) {
    peerTable.body[id].cl=NULL;
    peerTable.body[id].name[0]='\0';
    peerTable.body[id].ip[0]='\0';
    pthread_cond_init(&(peerTable.body[id].cond),NULL);
}
peerTable.nPeers=0;

for(id=0,src=buffer;src;) {
   char *next,*colon,*ip;
   char fullname[PROC_NAME_LEN+1];

   next=strchr(src,',');
   if(next) *next++='\0';

   colon=strchr(src,':');
   if(colon) {
      *colon='\0';
      ip=colon+1;
   } else {
      ip="";
   }
   snprintf(fullname,sizeof(fullname),"M_%s",src);
   if(strcmp(fullname,progInfo.MyName)!=0) {
       if(id<MAXPEERS) {
          snprintf(peerTable.body[id].name,sizeof(peerTable.body[id].name),"%s",fullname);
          snprintf(peerTable.body[id].ip,sizeof(peerTable.body[id].ip),"%s",ip);
          NormalizeIP(peerTable.body[id].ip,sizeof(peerTable.body[id].ip),NULL);
          id++;
       } else {
           LAO_Error(NULL,SYNTAX_ERROR,"parsing peer specification");
           break;
       }
   }
   src=next;
}
free(buffer);
peerTable.nPeers=id;
return id;
}


//@Function: PeerConnectRequest
//
// Issues a connection request to a peer
//
//@

static Client *PeerConnectRequest(char *ip, char *name)
{
int stat;
Client *pe=(Client *)malloc(sizeof(Client));

if(!pe) {
    LAO_Error(NULL,MEMORY_ALLOC_ERROR,"in PeerConnectRequest()");
    return NULL;
}
InitClient(pe);
snprintf(pe->conn.ip,sizeof(pe->conn.ip),"%s",ip);
LAO_Debug(NULL,"Trying to connect to Peer %s @ %s",name,ip);
LAO_Trace(NULL,"Peer structure addr:%lx",pe);
stat=ConnectInit(&(pe->conn));
if(IS_NOT_ERROR(stat)) {
   
    LAO_Trace(NULL,"ConnectInit() returns OK - addr:%lx sock:%d",pe,pe->conn.sock);
    MsgBuf *msgb=&(pe->rMsgb);
    char *pt; int len;

    pt=GetVersionID();
    len=strlen(pt)+1;

    FillHeader(0,0,progInfo.MyName,"",REGISTER, msgb);

    FillBody(pt,len,msgb); // Put client identification data

    HDR_FLAGS(msgb)=0;
    HDR_PLOAD(msgb)=PEER_MSGD;

    if(IS_NOT_ERROR(stat=SendToSK(pe->conn.sock,msgb))) {
        double tout=PEER_REQ_TMOUT*0.001;

        LAO_Trace(NULL,"Sent REGISTER request - addr:%lx",pe);
        struct timeval tmout;
        fd_set rd;
        int r;
        int sock=pe->conn.sock;

        LAO_LogMsgSnt(msgb,NULL);
        tmout.tv_sec=0;
        tmout.tv_usec=PEER_REQ_TMOUT;
        FD_ZERO(&rd);
        FD_SET(sock,&rd);
        LAO_Trace(NULL,"Waiting %f ms for reply - addr:%lx",tout,pe);
        r = select(sock+1,&rd,NULL,NULL,&tmout);
        if(r>0) {
            LAO_Trace(NULL,"Reply received - addr:%lx",pe);
                                                  // Wait for ACK/NAK
            if(IS_NOT_ERROR(stat=WaitMessage(pe->conn.sock,msgb))) {
                LAO_LogMsgRec(msgb);
                if(HDR_CODE(msgb)==ACK) {
                    stat=NO_ERROR;
                } else  {
                    if(HDR_CODE(msgb) == NAK)
                        stat = HDR_PLOAD(msgb);
                    else
                        stat = ILLEGAL_MSG_ERROR; 
                }
            }
        } else {
            LAO_Trace(NULL,"Timeout - addr:%lx",pe);
            stat=TIMEOUT_ERROR;
        }
    } else {
        LAO_Trace(NULL,"Cannot send REGISTER request - addr:%lx",pe);
    }
    if(IS_NOT_ERROR(stat)) {
        LAO_Trace(NULL,"Checking peer data - addr:%lx sock:%d",pe,pe->conn.sock);
        strncpy(pe->Name,hdr_FROM(pe->rMsgb),PROC_NAME_LEN);
        pe->Name[PROC_NAME_LEN]='\0';
        pe->Status=CLIENT_JUST_CONNECTED;
        if(strncmp(pe->Name,name,PROC_NAME_LEN)!=0) {
           stat=DUPLICATE_CLIENT_ERROR;
           LAO_Error(pe,stat,"in PeerConnectRequest()");
        } 
    } 
    if(IS_ERROR(stat)) {
        LAO_Trace(NULL,"Closing connection - addr:%lx sock:%d",pe,pe->conn.sock);
        close(pe->conn.sock);
        pe->conn.sock=(-1);
        pe->conn.ip[0]='\0';
    }
}
if(IS_ERROR(stat)) {
    LAO_Trace(NULL,"No connection. Clean - addr:%lx",pe);
    FreeClient(pe);
    pe=NULL;
} else {
    LAO_Trace(NULL,"Connection established - addr:%lx sock:%d ip:%s",pe,pe->conn.sock,pe->conn.ip);
}
return pe;
}

static int Spawn(void *task, void *arg,char *thName, pthread_t *thid)    // Spawns a routine into a thread;
{
pthread_attr_t thrdAttribute;
pthread_t myth;
int stat=NO_ERROR;
                                             // Prepare thread spawning
pthread_attr_init(&thrdAttribute);
pthread_attr_setdetachstate(&thrdAttribute,  // Declare thread "detached"
                     PTHREAD_CREATE_DETACHED);   // So that it dies at end
if(pthread_create(&myth,&thrdAttribute,task,arg)!=0) {
   stat=THREAD_CREATION_ERROR;
   LAO_Error(NULL,stat,"spawning %s [%s]",thName,strerror(errno));
                           // The thread has been successfully spawned. All
                           // data exchanges with the client are managed by
                           // the new thread
} else {
    if(thid) *thid=myth;
}
       
pthread_attr_destroy(&thrdAttribute);        // Attributes can be destroyed
                                             // After spawning the thread
return stat;
}

#define LINELEN 1024

//@Function: setMirrored
//
// Reads mirror variable file (if any) and sets up mirror list
//@

/*
static void setMirrored(char *mirrfile)
{
char line[LINELEN+1];

FILE *in=fopen(mirrfile,"r");
if(!in) return;

while(fgets(line,LINELEN,in)) {
    line[LINELEN]='\0';
    char *start, *end;
    s_item s;
    int pos=0;

    start=line;
    while(isspace(*start)) start++;
    if(*start=='#') continue;
    end=start;
    while(isgraph(*end)) end++;
    if(end<=start) continue;
    *end='\0';
    s.key=strdup(start);
    s.value=NULL;
    sput(&(mirrorTB.stable),&s,&pos);
}
}
*/


//@Function: InitVariable
//
// Creates a variable in RTDB
// @

static Variable* myVar=NULL;
static int myVarLng=0;

static int initVariable(char* vname,  //@P{vname}: Variable name
                        int   vtype,  //@P{vtype}: Variable type
                        void* values, //@P{values}: Values
                        int   nitms)  //@P{nitms}: Number of elements
                                      //@R: completion code
{
   int vlen,totlen;
   int stat;

   vlen=VarSize(vtype,nitms);
   if(IS_ERROR(vlen)) return vlen;
   totlen=vlen+sizeof(Variable);

   if(myVarLng<totlen) {       // Allocate space for variable buffer
      myVar=realloc(myVar,totlen);
      if(!myVar) return MEMORY_ALLOC_ERROR;
      myVarLng=totlen;
   }
   strncpy(myVar->H.Name,vname,VAR_NAME_LEN);
   myVar->H.Owner[0]='\0';
   myVar->H.Type=vtype;
   myVar->H.NItems=nitms;
   memcpy(&(myVar->Value),values,vlen);
   stat=WriteVariable(myVar,MySelf,NULL);
   return stat;
}

//@Procedure: freeVars
//
// Free memory allocated for variable management
//@

static void freeVarRes()
{
   if(myVarLng>0) free(myVar);
   myVar=NULL;
   myVarLng=0;
}

static void syntax_err(char *name, long ln)
{
    LAO_Error(NULL,SYNTAX_ERROR,"Variable file: %s at line: %ld",name,ln);
}

static int nwords(char *line)
{
   int n=0;
   char *l=line;

   for(;;) {
      while(isspace(*l)) l++;
      if(*l=='\0') break;
      n++;
      while(*l && !isspace(*l)) l++;
   }
   return n;
}

static char *nextv(char *v)
{
   while(*v && !isspace(*v)) v++;

   if(*v) *v++='\0';

   while(isspace(*v)) v++;

   return v;

}


//@Procedure: initVariables
//
// Scan config file for varible definitions, and create variables in RTDB
// @

#define LINE_LEN 32768

static void initVariables(char *filename)
{
   FILE *fd;
   char line[LINE_LEN];
   long nline=0;
   void *buffer=NULL;
   size_t buflen=0,nbufl;
   int stat,type;
   char *vname, *vtype,*values;

   fd=fopen(filename,"r");
   if(!fd) {
      LAO_Info(NULL, "No variable file found");
      return;
   }
   LAO_Info(NULL, "Reading variable file: %s",filename);

   for(;;) {
      char *l,*pt,*lpt;

      l=fgets(line,LINE_LEN,fd);
      if(!l) break;
      nline++;
      pt=strchr(l,'#');
      if(pt) *pt='\0';         // Remove comments
      while(isspace(*l)) l++;  // Remove leading blanks
      if(*l=='\0') continue;   // Ignore blank lines

                                  // Remove trailing blanks
      for(pt=l,lpt=NULL;*pt;pt++)
         if(!isspace(*pt)) lpt=pt;
      if(lpt++) *lpt='\0';

      vname=l;
      while(*l && !isspace(*l)) l++;
      if(*l=='\0') {          // Unexpected EOL
         syntax_err(filename,nline);
         continue;
      }
      *l++='\0';
      while(isspace(*l)) l++;
      vtype=l;
      while(*l && !isspace(*l)) l++;
      if(l=='\0') {          // Unexpected EOL
         syntax_err(filename,nline);
         continue;
      }
      *l++='\0';
      while(isspace(*l)) l++;
      values=l;

      if(!isgraph(*vname)) {
         syntax_err(filename,nline);
         continue;
      }
      type=0;
      if(toupper(vtype[0])=='I' ) type=INT_VARIABLE;
      if(toupper(vtype[0])=='R' ) type=REAL_VARIABLE;
      if(toupper(vtype[0])=='C' ) type=CHAR_VARIABLE;
      if(toupper(vtype[0])=='B' ) {
         if(vtype[1]=='8') type=BIT8_VARIABLE;
         if(vtype[1]=='1') type=BIT16_VARIABLE;
         if(vtype[1]=='3') type=BIT32_VARIABLE;
         if(vtype[1]=='6') type=BIT64_VARIABLE;
      }
      if(type==0) {
         syntax_err(filename,nline);
         continue;
      }

      if(type==CHAR_VARIABLE) {
         stat=initVariable(vname,type,values,strlen(values)+1);
      } else {
         char *v;
         int i;
         int nw=nwords(values);
         nbufl=VarSize(type,nw);
         if(nbufl>buflen) {
            buffer=realloc(buffer,nbufl);
            if(!buffer) {
               LAO_Error(NULL,MEMORY_ALLOC_ERROR,"Creating variables from file: %s at line: %ld",filename,nline);
               break;
            }
            buflen=nbufl;
         }
         v=values;
         for(i=0;i<nw;i++) {
            long *longbuf=(long *)buffer;
            double *doublebuf=(double *)buffer;
            unsigned char *b8buf=(unsigned char *)buffer;
            unsigned short *b16buf=(unsigned short *)buffer;
            unsigned long *b32buf=(unsigned long *)buffer;
            unsigned long long *b64buf=(unsigned long long *)buffer;
            char *n;
            n=nextv(v);
            switch(type) {
               case INT_VARIABLE:
                  longbuf[i]=atol(v);
                  break;
               case REAL_VARIABLE:
                  doublebuf[i]=atof(v);
                  break;
               case BIT8_VARIABLE:
                  b8buf[i]=atoi(v);
                  break;
               case BIT16_VARIABLE:
                  b16buf[i]=atoi(v);
                  break;
               case BIT32_VARIABLE:
                  b32buf[i]=atol(v);
                  break;
               case BIT64_VARIABLE:
                  b64buf[i]=atoll(v);
                  break;
            }
            v=n;
         }
         stat=initVariable(vname,type,buffer,nw);
      }
      if(IS_ERROR(stat)) LAO_Error(NULL,stat,"Creating variables from file: %s at line: %ld",filename,nline);
   }

   if(buffer) free(buffer);
   freeVarRes();
}

//@Function: DiscoverPeers
//
// Manages the discovery and connection of peers
//@

static
void DiscoverPeers(void)
{
Client *pe;
int id;

for(id=0;id<peerTable.nPeers;id++) {    // Scan peer table to find peers not connected
    MUTEX_LOCK(&ClientTable.mutex,"PeerManager");
    pe=GetClientByName(peerTable.body[id].name,PEER_MSGD);
    MUTEX_UNLOCK(&ClientTable.mutex,"PeerManager");
    if(pe) continue;
    pe=PeerConnectRequest(peerTable.body[id].ip,peerTable.body[id].name);  // Try to connect
    if(pe) {                       // If successful ...
        Spawn((void *)&PeerConnection,pe,"Peer Connection",NULL);
    }
}
}

//@Procedure: StartClient
//
// Accept client connection and spawn the related thread
// @

static void StartClient(int listener)
{
Client *cl;
socklen_t sin_size;
                          //
                          // Allocate the data structure for 
                          // next connection
cl=(Client *)malloc(sizeof(Client));
if(cl==NULL) 
    LAO_Fatal(NULL,MEMORY_ALLOC_ERROR,strerror(errno));

InitClient(cl);

sin_size = sizeof(struct sockaddr_in); 
	                                     // Wait client connections
if ((cl->conn.sock = accept(listener, 
		          (struct sockaddr *)&(cl->conn.skadr), 
		           &sin_size)) == -1) { 
    LAO_Error(NULL,NETWORK_ERROR,"on accept: %s",strerror(errno));
    FreeClient(cl);
} else {
	                                     // A client has connected

    snprintf(cl->conn.ip,sizeof(cl->conn.ip),"%s:%d", 
                          inet_ntoa(cl->conn.skadr.sin_addr),
                          ntohs(cl->conn.skadr.sin_port));
    LAO_Debug(NULL, "Got connection - sock:%d IP:%s", cl->conn.sock, cl->conn.ip); 

	                                             // spawn client thread
    Spawn((void *)&ClientConnection,cl,"Client Connection",NULL);
}
}


pthread_t MainThread;           // Global variable with main thread ID

pthread_t dumpThread;           // Global variable with info dump thread ID

ShmBufferPool bPool;		// Shared memory buffer pool

int main(int argc, char **argv) {
struct sockaddr_in myaddr;     // my own address

int loglines=LOGLINES;         // Max number of lines in log file (may be overridden
                               // in config file)
int listener;                  // listening socket descriptor 
int yes=1;                     // for setsockopt() SO_REUSEADDR, below 
int i,stat,loglevel=2,logfile=1,nit;
int port=0;
char *logarg=NULL;
char *peerarg=NULL;
int proActive=1;
// int mirrordef=0;
char varfile[MAX_FNAME];
int disablecfg=0;

struct sigaction act;

char cfgdirbuf[MAX_FNAME];
char *cfgdir=NULL;

config_param cfg[7]={ { "logpath", CHAR_VARIABLE, NULL }, 
                      { "loglines", INT_VARIABLE, NULL }, 
                      { "port",    INT_VARIABLE, NULL },
                      { "autodump", INT_VARIABLE, NULL },
                      { "maxretmsgs", INT_VARIABLE, NULL },
                      { "ident", CHAR_VARIABLE, NULL },
                      { "peers", CHAR_VARIABLE, NULL },
                    }; 
char *pt;
char *idspec=NULL;

    gettimeofday(&progInfo.startTime, NULL);   // Store starting time

    SetVersion(VERS_MAJOR,VERS_MINOR);
    progInfo.cfgName[0]='\0';
    progInfo.logPath[0]='\0';
    progInfo.maxRetMsgs=MAX_RET_MSGS;
    progInfo.msgMagic=MSG_HEADER_MAGIC;

    ClientTable.Names[0]=progInfo.MyName;  // Add dummy entry [0] in client table
    ClientTable.Items[0]=MySelf;

    progInfo.MainPID=getpid();
    progInfo.netIoTmout=NETIO_WAIT;

                                       // Initialize all tables
    if(IS_ERROR(stat=InitTables()))  
        LAO_Fatal(NULL,stat,"Initializing internal tables");

    i=1;
    for(i=1;i<argc;i++) {          // Process command arguments
	    if(strcmp(argv[i],"-v")==0) {
	        loglevel++;
           continue;
	    } 
	    if(strcmp(argv[i],"-q")==0) {
	        loglevel--;
           continue;
	    } 
	    if(strcmp(argv[i],"-i")==0) {
	        idspec=argv[++i];
           continue;
	    } 
	    if(strcmp(argv[i],"-c")==0) {
	        peerarg=argv[++i];
           continue;
	    } 
	    if(strcmp(argv[i],"-D")==0) {
	        MsgDebug(1);
           continue;
	    } 
       /*
	    if(strcmp(argv[i],"-g")==0) {
            s_item s;
            int pos;
            s.key=strdup(argv[++i]);
            s.value=NULL;
            sput(&(mirrorTB.stable),&s,&pos);
            mirrordef=1;
            continue;
	    } 
       */
	    if(strcmp(argv[i],"-d")==0) {
	        cfgdir=argv[++i];
           continue;
	    } 
	    if(strcmp(argv[i],"-l")==0) {
	        logarg=argv[++i];
           continue;
	    } 
	    if(strcmp(argv[i],"-N")==0) {
	        disablecfg=1;
           continue;
	    } 
	    if(strcmp(argv[i],"-n")==0) {
	        logfile=0;
           continue;
	    } 
	    if(strcmp(argv[i],"-p")==0) {
	        port=atoi(argv[++i]);
           continue;
	    } 
	    if(strcmp(argv[i],"-s")==0) {
	        proActive=0;
           continue;
	    } 
       help();
       exit(0);
    }

// Initialize mutexes attributes 
// The following initializer will be used for all mutexes
    pthread_mutexattr_init(&mutexAttributes);

#ifdef MUTEX_ERROR      // Note: the following set should detect deadlocks. 
    pthread_mutexattr_settype(&mutexAttributes,PTHREAD_MUTEX_ERRORCHECK);
#else                   // Note: the following default setting will 
                        // not detect deadlocks, but it is supposed
                        // to be more efficient.
    pthread_mutexattr_settype(&mutexAttributes,PTHREAD_MUTEX_NORMAL);
#endif

    if(!disablecfg) {
                                //  Find and read config file
       if (!cfgdir) {
         char *adopt_root = getenv("ADOPT_ROOT");
         char *subsystem = getenv("ADOPT_SUBSYSTEM");
         if ((adopt_root) && (subsystem)) { 
            char subsyst[MAX_FNAME];
            unsigned int i;
            for (i=0; i<strlen(subsystem); i++)
              subsyst[i] = tolower(subsystem[i]);
            subsyst[i]='\0';
            snprintf(cfgdirbuf, MAX_FNAME, "%s/conf/%s/current", adopt_root, subsyst);
            cfgdir = cfgdirbuf;
         }
         else if (adopt_root) {
            snprintf(cfgdirbuf, MAX_FNAME, "%s/conf", adopt_root);
            cfgdir = cfgdirbuf;
         }
         else
            cfgdir = ".";
       }
       snprintf(progInfo.cfgName,MAX_FNAME,"%s/msgdrtdb.conf",cfgdir);
       snprintf(varfile,MAX_FNAME,"%s/msgdrtdb.var",cfgdir);
       stat=loadConfigFile(progInfo.cfgName,cfg,8);
    }

    if(logfile) {
        if(cfg[0].pointer)   // Search log file path in configuration
           snprintf(progInfo.logPath,MAX_FNAME,"%s",(char *)cfg[0].pointer);
        else
           strncpy(progInfo.logPath,LOGPATH,MAX_FNAME);
        if(logarg)          // If logpath specified as argument, use it
           strncpy(progInfo.logPath,logarg,MAX_FNAME);
        else {
           pt=getenv("ADOPT_LOG");  // If existent, overrides config file setting
           if(pt) snprintf(progInfo.logPath,MAX_FNAME,"%s",pt);
        }
    } 

    if(cfg[1].pointer)    // Search number of log lines in configuration
        loglines=*(int *)(cfg[1].pointer);
   
    pt=getenv("MSGD_PORT");  // Search alternate port number in Environment
    if(pt)
        progInfo.lPort=atoi(pt);
    else {
        if(cfg[2].pointer)       // Search alternate port number in configuration
            progInfo.lPort=*(int *)(cfg[2].pointer);
        else
            progInfo.lPort=MSGD_PORT;  // Assign default port number
    }
    if(port>0) progInfo.lPort=port;

    if(cfg[3].pointer) {   // Search autodump in configuration
        autodump=*(int *)(cfg[3].pointer);
        autodump *= 1000;   // put it in millisecs
    }

    if(cfg[4].pointer)    // Search maxretmsgs in configuration
        progInfo.maxRetMsgs=*(int *)(cfg[4].pointer);

    if(!idspec)  {
        pt=getenv("MSGD_IDENT");  // Search identity in Environment
        if(pt)
            idspec=pt;
        else {
            if(cfg[5].pointer)    // Search identity in configuration
                idspec=cfg[5].pointer;
        }
    }
       
    if(idspec) {
        snprintf(progInfo.MyName,sizeof(progInfo.MyName),"M_%s",idspec);
        progInfo.peerOK=1;
    } else {
        snprintf(progInfo.MyName,sizeof(progInfo.MyName),"MSGDRTDB");
        progInfo.peerOK=0;
    }

                                          // Initialize random number generator
    srandom((unsigned int)progInfo.startTime.tv_usec);

    if(peerarg) 
        parsePeers(peerarg);
    else {
        if(cfg[6].pointer) 
            parsePeers((char *)cfg[6].pointer);
    }

                                 // A dummy client structure is set up
                                 // for MsgD-RTDB, to support logging selection
    snprintf(MySelf->Name,sizeof(MySelf->Name),"%s",progInfo.MyName);
    me.toLog=1;
    me.Id=0;
       
                          // Initialize global mutexes
    MUTEX_INIT(&shmMgmtMutex,"SharedMem");
    MUTEX_INIT(&LogMutex,"LogMutex");

    SetDebugLevel(loglevel);
    SetMaxlines(loglines);
    if(logfile) SetLogfile(LOG_START);

    /*
    if(!mirrordef)              // No mirrored variables in command line.
       setMirrored(mirrfile);   // Get list from file
    */

    initVariables(varfile);

    LAO_DumpInfo(1);
    LAO_DumpNotifTable(0);

    if(peerTable.nPeers>0) {
        char *whpeers;
        if(peerarg)
            whpeers="command argument";
        else
            whpeers="config file";
        LAO_Info(NULL,"Peers as defined in %s:",whpeers);
        for(i=0;i<peerTable.nPeers;i++)
            LAO_Info(NULL,"  %s @ %s",peerTable.body[i].name,peerTable.body[i].ip);
    } else
        LAO_Info(NULL,"No peers defined");
                                 // Open a socket to get access requests
    if ((listener = socket(AF_INET, SOCK_STREAM, 0)) == -1) 
	    LAO_Fatal(NULL,GENERIC_SYS_ERROR,"socket()");

    if (setsockopt(listener, SOL_SOCKET, SO_REUSEADDR, &yes, 
		                            sizeof(int)) == -1) 
	    LAO_Fatal(NULL,GENERIC_SYS_ERROR,"setsockopt()");

    if (setsockopt( listener, SOL_TCP, TCP_NODELAY, &yes, sizeof(int)) == -1)
	    LAO_Fatal(NULL,GENERIC_SYS_ERROR,"setsockopt() - NODELAY");

    myaddr.sin_family = AF_INET; 
    myaddr.sin_addr.s_addr = INADDR_ANY; 
    myaddr.sin_port = htons(progInfo.lPort); 
    memset(&(myaddr.sin_zero),  '\0' , 8); 
    if (bind(listener, (struct sockaddr *)&myaddr, sizeof(myaddr)) == -1)
	    LAO_Fatal(NULL,GENERIC_SYS_ERROR,"Maybe another MsgD-RTDB is running on port %d?",progInfo.lPort);

    if (listen(listener, 10) == -1)
	    LAO_Fatal(NULL,GENERIC_SYS_ERROR,"listen()");

    ClientTableInit(peerTable.nPeers);

    act.sa_handler = sighand;
    sigemptyset(&act.sa_mask);
    act.sa_flags = 0;
    sigaction(SIGINT, &act,NULL);      // Install signal handler for Interrupt
    sigaction(SIGHUP, &act,NULL);      // and for SIGHUP

                                // write MsgD.Ident variable
    nit=snprintf(ident,PG_IDENTITY_LNG,"%s - %s", progInfo.MyName, GetVersionID())+1;
    if(nit>=PG_IDENTITY_LNG) nit=PG_IDENTITY_LNG;
    initVariable("MsgD.Ident",CHAR_VARIABLE,ident,nit);

    MainThread=pthread_self();    // Store my own Thread ID.

    LAO_Info(NULL,"Spawning info dump thread. Interval: %d msec",autodump);
                                          // Spawn housekeeping thread
    if(IS_ERROR(stat=Spawn((void*)&housekeeping,NULL,"Dump Thread", NULL))) {
        LAO_Error(NULL,stat,"Terminating due to fatal error");
        exit(0);
    }

    setSeed(0);             // Initialize message sequence number generator
    shmInit();              // Initialize shared buffer pool

    LAO_Info(NULL,"Waiting connections on port %d",progInfo.lPort);

                                  // Begin MsgD-RTDB main loop:
    while(1) {
        double rndfact=999999.0/(double)RAND_MAX;
        struct timeval tmout;
        fd_set rd;
        int r;

        tmout.tv_sec=2;
        tmout.tv_usec=(double)random()*rndfact;
        FD_ZERO(&rd);
        FD_SET(listener,&rd);

        r = select(listener+1,&rd,NULL,NULL,&tmout);
        if(r== -1 && errno==EINTR) continue;
        if(r<0) {
            LAO_Error(NULL,NETWORK_ERROR,"from select(): %s",strerror(errno));
            continue;
        }
        if(r==0) {  
                     // Timeout: try active connection to peers
           if(peerTable.nPeers>0 && proActive) DiscoverPeers();
        } else {
                     // Connection received, start Client thread
           StartClient(listener);
        }
    }
}
