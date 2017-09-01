//@File: thrdtest.c
//
// General test for the thrdlib

//@

//@Main: thrdtest
//
// Command based utility to perform a variety of tests with MsgD-RTDB
//
// This program is an interactive, command based, utility to perform
// a number of tests on a MsgD-RTDB proces and on the clients connected 
// to it. Because it allows to send virtually all the commands defined
// in MsgD and by clients it can be used to get information on a running
// system for verification or debugging purposes.
//
// When started it tries to conenct to a Message Daemon and then enters
// a loop to receive commands and execute them.
//
// More than 70 commands are currently defined to perform fuctions such as:
//
// . Getting info from MsgD or other clients.
// . Getting info on MsgD message codes and on error codes.
// . Writing various info to logfile.
// . Operate on Variables in repository (create/delete, read/write, etc.)
// . Sending generic messages to clients.
// . Sending AOS specific commands
//
// The programs also provides a very limited set of scripting capabilities 
// which allow to create automatic test procedures.
//
// The program is provided with an integrated help system.
//@

// Version 6.5: Added support for writing variables from file
// Version 6.6: Recompiled with new message protocol (44)
// Version 6.7: Added printout of aoslib version
//              Fixed bug in register value assignment
// Version 6.8: Added numeric version request
// Version 6.9: Improved error messages
// Version 6.10: Added check prior of sending aos.offload command
//               Added info on start arguments
// Version 6.11: Added aos.flatlist command
// Version 6.12: Added getstart time command
// Version 6.13: Added -l flag

#define VERSMAJ  6
#define VERSMIN  13


#include <stdio.h>
#include <ctype.h>
#include <errno.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <pthread.h>

#include <math.h>

#include <time.h>
#include <sys/time.h>

#include "base/thrdlib.h"

#include "base/msgtxt.h"

#include "aoslib/aoslib.h"

#define PATH_LEN 2048

#define ARG_LNG 1024
#define CMD_LNG 1024


char MyName[21];
char prompt[80];

char *aosside=NULL;

pthread_t MainThread;

int started=0;
int initialized=0;

static int logLevel=0;

static char chbuf[2048];

char def_dir[PATH_LEN+1]=".";
char log_file[PATH_LEN+1]="";
char exec_file[PATH_LEN+1]="";

int verbose=0;

int servermode=0;
int goon=1;

char reg0[ARG_LNG];
char reg1[ARG_LNG];
char reg2[ARG_LNG];
char reg3[ARG_LNG];
char reg4[ARG_LNG];
char reg5[ARG_LNG];
char reg6[ARG_LNG];
char reg7[ARG_LNG];
char reg8[ARG_LNG];
char reg9[ARG_LNG];


//@Procedure: sighand
//
//  Signal handler for terminating the program
//@

static void sighand(int thesig)
{
   exit(0);
}


//@Procedure: cmdhelp
//
//  Print a list of accepted commands
//@

static void cmdhelp()
{
    printf("\nCommand list (commands are case insensitive):\n\n");
    printf("aos                    List AOS subcommands\n");
    printf("bdump [@M_xxx]         Send a dump Buffer list request (Code:DUMPBUFS) \n");
    printf("blist                  Send a Buffer list request (Code:BUFLIST) \n");
    printf("cdump [@M_xxx]         Send a dump Client list request (Code:DUMPCLNS)\n");
    printf("clist [@M_xxx]         Send a Client list request (Code:CLNTLIST)\n");
    printf("clean                  call thCleanup()\n");
    printf("close                  Close communication with MsgD-RTDB\n");
    printf("debug n [name]         Set debug verbosity of this program or of client (*: MsgD)\n");
    printf("decr [name]            Decrease log verbosity of this program or of client (*: MsgD)\n");
    printf("del name               Delete specified variable\n");
    printf("dinfo [@M_xxx]         Send a DUMPINFO request\n");
    printf("dir name               Set default directory for procedures\n");
    printf("disable [client]       Disable logging for given client (*= all, none=MsgD)\n");
    printf("echo [-n] ...          Echo arguments onto terminal (-n: no newline)\n");
    printf("endloop                End of loop (see loop)\n");
    printf("enable [client]        Enable logging for given client (*= all, none=MsgD)\n");
    printf("errcode [code]         Write explanation of error code specified either as\n");
    printf("                       a numeric value or as a string\n");
    printf("exec name [r0 r1 ..]   Execute procedure from file name.p (see also: \"dir\")\n");
    printf("exit                   Terminate program\n");
    printf("flags                  Print message flags explanation\n");
    printf("get name [n]           Get specified variable. If n is specified, write into register n\n");
    printf("gstart name            Get start time of specified client (*: MsgD)\n");
    printf("help                   Print this help page\n");
    printf("handlers               Request handlers status to client (*: MsgD)\n");
    printf("haveyou name           Sends a request to know whether a client is registered\n");
   
    printf("hints                  Print some hints on how to test various thrdlib features\n");
    printf("incr [name]            increase log verbosity of this program or of client (*: MsgD)\n");
    printf("info [name]            Request info for this program or to client (\"*\": MSGD)\n");
    printf("init [newname]         Reinitialize and optionally give new name (must be used after clean)\n");
    printf("lock pref*             Lock specified variables\n");
    printf("log N string           Generate a log message [N: level= 3,4,5,6]. Use '|' to put newlines into string\n");
    printf("loop n                 repeat n times commands down to endloop (script mode only)\n");
    printf("                       If n=0, loop forever.\n");
    printf("msg n code pload [N/D] clnt\n");
    printf("                       Send n times message code n and pload to given client (broadcast: 'BCAST')\n");
    printf("                       add N or D to set the NOHANDLE or DISCARD flags\n");
    printf("ndump  [@M_xxx]        Send a DUMPNOTIF request\n");
    printf("newlog                 Close current log file and reopen a new one\n");
    printf("nonotif pref*          Cancel registration for notification on variable\n");
    printf("notif pref*            Register for notification on variable\n");
    printf("notrace pref*          Unset trace on specified variables\n");
    printf("nowait                 Cancel registration for client ready\n");
    printf("quit                   Terminate program\n");
    printf("pause [prompt]         Pause until a newline is entered (script mode only)\n");
    printf("ready                  Notify that I'm ready\n");
    printf("reg n [value]          Set value of register n (show current value if argument not specified)\n");
    printf("set name ty [v v ...]  Set specified variable. If no values are specified an UNDEF vale is written)\n");
    printf("sleep n                Delay n seconds\n");
    printf("start                  Start communication with MsgD-RTDB\n");
    printf("time clnt [n] [tmout]  Send n (default:1) ECHO messages to client with given timeout (default: 1000) (ms)\n");
    printf("                       and measure round trip delay (use name * for MsgD)\n");
    printf("term [clnt]            Terminate MsgD or specified client. Note: clients will receive\n");
    printf("                       a TERMINATE message which might be ignored.\n");
    printf("tmout usec             Set MSGD network timeout (microsecs)\n");
    printf("trace pref*            Set trace on specified variables\n");
    printf("ttl n                  Set default value for message TTL\n");
    printf("unlock pref*           Unlock specified variables\n");
    printf("vdump pref*            Send a DUMPVAR request\n");
    printf("vers [name]            Request numeric version for this program or to client (\"*\": MSGD)\n");
    printf("vlist pref*            Get list of variables\n");
    printf("vstat [@M_xxx]         Send a DUMPVSTAT request\n");
    printf("wait client [tmout]    Wait for client to become ready for tmout ms [default tmout=0: wait forever]\n");
    printf("whatis [msgcode]       Show description of given message code (numeric hexadecimal)\n");
    printf("                       or search into message descriptions for a given string\n");
    printf("write name ty v v ...  Write specified variable (Create if not existent).\n");
    printf("                       If no values are specified an UNDEF vale is written)\n");
    printf("write name file        Write variable of type B8 getting raw values from file.\n\n");
    printf("Note: command arguments may contain special escape sequences:\n");
    printf("        \\$= current client name.\n");
    printf("        \\t= current time.\n");
    printf("        \\?= random digit 0..9\n");
    printf("        \\0 .. \\9 expand value of register (see command \"reg\")\n");
}

//@Procedure: aoshelp
//
//  Print a list of AOS related subcommands
//@
static void aoshelp()
{
    char *enabled;
    if(aosside) 
       enabled="ENABLED";
    else
       enabled="DISABLED";
    printf("\nNOTE: to operate the AOS the program must be started with the -aos option\n\n");
    printf("      Currently -aos flag is: %s\n\n",enabled);
    printf("AOS Command list:\n\n");

    printf("aos.clean            AOS cleanup\n");
    printf("aos.dbg level        AOS debug level [0..4]\n");
    printf("aos.disable          disable commands from AOS\n");
    printf("aos.enable           enable commands from AOS\n");
    printf("aos.flatlist list    update flat list in AOS\n");
    printf("aos.idl 0/1          set IDL status variable\n");
    printf("aos.init             Initialize AOS communication\n");
    printf("aos.isup timeout     AOS check if is up con timeout (int)\n");
    printf("aos.log  int         AOS log - 1:activate, 0:deactivate\n");
    printf("aos.msg code         Print explanation of AOS specific subcommand (no code: full list)\n");
    printf("aos.offload z0 z1 .  send an offload command (up to 22 zernike values)\n");
    printf("aos.version          print out the aos library version\n\n");
    printf("hxp.init eX eY eZ eA eB eG    Init hexapod (6 double arguments)\n");
    printf("hxp.moveto eX eY eZ eA eB eG  Move hexapod in the current system\n");
    printf("hxp.moveby dX dY dZ dA dB dG  Move hexapod relatively to the previous position in the current system\n");
    printf("hxp.movesph R  eA eB          Move hexapod on a spherical surface\n");
    printf("hxp.newref                    Store current position as a new reference system\n");
    printf("hxp.getpos                    Return positition in the current reference system\n");
    printf("hxp.getabs                    Return positition in the absolute reference system\n");
    printf("hxp.brake                     Activate/deactivate brake\n");

}



//@Procedure: hints
//
//  Print some hints about testing special features
//@

static void hints()
{
    printf("\nHINTS:\n\n");
    printf("To test the message expiration features run the ECHOER utility with delay (eg: 0.5)\n");
    printf("and then use command time to send a number of message with shorter timeout\n\n");
    printf("due to the timeout the replies will remain in the message queue until cleared\n");
    printf("by the message expiration mechanism\n\n");
    printf("To fill an handler input queue, run ECHOER and then use command msg to send a number\n");
    printf("of messages with code 1111 (DUMMYMSG). The delay_handler has been set up to manage these\n");
    printf("messages: it will delay 1 second and then return, with NO_ERROR for a few times, and then\n");
    printf("return with a negative value. The handler managing routine should clean the handler queue.\n");
    printf("use command \"handlers\" to show handler queue status.\n\n");
    printf("To test the \"peer\" feature of MsgD you may start MsgD instances as follows (as an\n");
    printf("alternative peer definition can be done with the 'peer' keyword in configuration file):\n");

    printf("   msgdrtdb -i UNO -p 9751 -c UNO:9751,DUE:9752,TRE:9753 &\n");
    printf("   msgdrtdb -i DUE -p 9752 -c UNO:9751,DUE:9752,TRE:9753 &\n");
    printf("   msgdrtdb -i TRE -p 9753 -c UNO:9751,DUE:9752,TRE:9753 &\n\n");
}

static void flghelp()   // Provide help on message flag meaning
{
   printf("\nFlags is the sum of the REPLY and either NOHANDLE or DISCARD:\n\n");
   printf("   %2.2x - NOHANDLE: if a REPLY do not process this message by message handler\n",NOHANDLE_FLAG);
   printf("   %2.2x - DISCARD: if a REPLY discard this message\n",DISCARD_FLAG);
   printf("   %2.2x - REPLY: message is a reply\n\n",REPLY_FLAG);
}

static void mlist()
{
int i=0;
DbTable *pt;

   printf("\n Message list:\n");
   while(1) {
      pt=msgtxt_i(i++);
      if(pt) 
         printf("  %6x: %-15s - %s\n",pt->code,pt->name, pt->descr);
      else
         break;
   }
}  


static void msgexpl(char *arg1) // Print AOS subcommand explanation
{
   if(arg1[0]) {
      int code=atoi(arg1);
      DbTable* pt=aostxt_c(code);

      if(pt)
         printf("\n%4d: %s - %s\n",code,pt->name,pt->descr);
      else
         printf("\n%4d: Unknown code\n",code);
   } else {
      int i=0;
      printf("\nAOS subcommand codes:\n");
      while(1) {
         DbTable* pt=aostxt_i(i++);
         if(!pt) break;
         printf("%4d: %s - %s\n",pt->code,pt->name,pt->descr);
      }
   }
}

static void arg_error(void)
{
printf("%s Argument error!\n",MyName);
}

static void mystrcat(char *dest,char *src,int maxl)
{
   int rem;

   rem=maxl-strlen(dest);
   if(rem>2) {
      strncat(dest," ",1);
      strncat(dest,src,rem-1);
   }
   dest[maxl-1]='\0';
}

static void setregister(char *rgnumb, char *value)
{
switch(*rgnumb) {
   case '0': snprintf(reg0,ARG_LNG,"%s",value); break;
   case '1': snprintf(reg1,ARG_LNG,"%s",value); break;
   case '2': snprintf(reg2,ARG_LNG,"%s",value); break;
   case '3': snprintf(reg3,ARG_LNG,"%s",value); break;
   case '4': snprintf(reg4,ARG_LNG,"%s",value); break;
   case '5': snprintf(reg5,ARG_LNG,"%s",value); break;
   case '6': snprintf(reg6,ARG_LNG,"%s",value); break;
   case '7': snprintf(reg7,ARG_LNG,"%s",value); break;
   case '8': snprintf(reg8,ARG_LNG,"%s",value); break;
   case '9': snprintf(reg9,ARG_LNG,"%s",value); break;
   default: arg_error(); break;
}
}

static void waitready(char *arg0, int ms)
{
   int stat=thWaitReady(arg0,ms);
   if(stat<0) {
      printf("%s Error from thWaitReady: (%d) %s\n",MyName,stat,lao_strerror(stat));
      return;
   }

}


static void DisplayVariable(Variable *var, char *dest)
{
char vtype[100];
char *end;
int nout;
int i;
struct tm *tt;
unsigned long myThrd;

char fval[ARG_LNG];   // Buffer to write into register
#ifdef LONG_IS_64
struct timeval tv;
#endif

switch(var->H.Type) {
   case INT_VARIABLE:
      snprintf(vtype,100,"int[%d]",var->H.NItems);
      break;
   case REAL_VARIABLE:
      snprintf(vtype,100,"real[%d]",var->H.NItems);
      break;
   case CHAR_VARIABLE:
      snprintf(vtype,100,"char[%d]",var->H.NItems);
      break;
   case BIT8_VARIABLE:
      snprintf(vtype,100,"bit8[%d]",var->H.NItems);
      break;
   case BIT16_VARIABLE:
      snprintf(vtype,100,"bit16[%d]",var->H.NItems);
      break;
   case BIT32_VARIABLE:
      snprintf(vtype,100,"bit32[%d]",var->H.NItems);
      break;
   case BIT64_VARIABLE:
      snprintf(vtype,100,"bit64[%d]",var->H.NItems);
      break;
   default:
      myThrd=pthread_self();
      printf("[t:0x%lx] Bad variable buffer format\n", myThrd);
      return;
}


if(var->H.NItems>100) {
    nout=100;
    end=" ...\n";
} else {
    nout=var->H.NItems;
    end="\n";
}

#ifdef LONG_IS_64
tv = tv_fm_tv32(var->H.MTime);
tt = localtime(&(tv.tv_sec));
#else
tt=localtime(&(var->H.MTime.tv_sec)); 

#endif

printf("%s: [%2.2d/%2.2d/%4.4d %2.2d:%2.2d:%2.2d.%6.6d] %s\n",
        var->H.Name, tt->tm_mday, tt->tm_mon+1, tt->tm_year+1900, tt->tm_hour, 
        tt->tm_min, tt->tm_sec, (int)var->H.MTime.tv_usec, vtype);

if(nout==0) {
   printf(" UNDEFINED\n");
   snprintf(fval,ARG_LNG,"UNDEFINED");
} else {
   switch(var->H.Type) {
      case INT_VARIABLE:
         snprintf(fval,ARG_LNG,"%ld",var->Value.Lv[0]);
         for(i=0;i<nout;i++) printf(" %ld",var->Value.Lv[i]);
         break;
      case REAL_VARIABLE:
         snprintf(fval,ARG_LNG,"%f",var->Value.Dv[0]);
         for(i=0;i<nout;i++) printf(" %.2f",var->Value.Dv[i]);
      break;
      case CHAR_VARIABLE:
         printf(" ");
         for(i=0;i<nout;i++) {
		      printf("%c",var->Value.Sv[i]);
            if(i<ARG_LNG) fval[i]=var->Value.Sv[i];
         }
         if(i<ARG_LNG) fval[i]='\0';
         fval[ARG_LNG-1]='\0';
         break;
      case BIT8_VARIABLE:
         snprintf(fval,ARG_LNG,"%02x",var->Value.B8[0]);
         for(i=0;i<nout;i++) printf(" %02x",var->Value.B8[i]);
         break;
      case BIT16_VARIABLE:
         snprintf(fval,ARG_LNG,"%04hx",var->Value.B16[0]);
         for(i=0;i<nout;i++) printf(" %04hx",var->Value.B16[i]);
      break;
      case BIT32_VARIABLE:
         snprintf(fval,ARG_LNG,"%08lx",var->Value.B32[0]);
         for(i=0;i<nout;i++) printf(" %08lx",var->Value.B32[i]);
      break;
      case BIT64_VARIABLE:
         snprintf(fval,ARG_LNG,"%016llx",var->Value.B64[0]);
         for(i=0;i<nout;i++) printf(" %016llx",var->Value.B64[i]);
   }
   printf("%s",end);
}
if(dest) setregister(dest,fval);
}


static int notif_handler(MsgBuf *msgb, void *argp, int nq)
{
Variable *theVar;
int spec=HDR_PLOAD(msgb);  // Get message specification
unsigned long myThrd=pthread_self();

char *specs[3] = { "written", "deleted", "created" };

printf("%s [t:0x%lX] Notification received. Variable %s\n", MyName,myThrd,specs[spec]);

theVar=thValue(msgb);
DisplayVariable(theVar,NULL);
thRelease(msgb);

return NO_ERROR;
}

static int naccum=0;


// The following handler will simply receive messages and exit with some delay (10 sec)
// It is usable to test handler input queue management. This is installed as handler for
// message code DUMMYMSG

static int delay_handler(MsgBuf *msgb, void *argp, int nqueue)
{
int stat=NO_ERROR;
char aux[200];
   printf("%s %s ... holding\n",MyName,StrHeader(msgb,aux,200));

   fflush(stdout);

   printf("%s delay_handler: there are %d messages in queue\n",MyName,nqueue);

msleep(10000);
thRelease(msgb);

if(naccum++<5) {
   printf("%s delay_handler: returning with NO_ERROR\n",MyName);
   stat = NO_ERROR;
} else {
   naccum=0;
   printf("%s delay_handler: returning with error (queue should be cleaned)\n",MyName);
   stat = (-1);
}
return stat;
}

static int term_handler(MsgBuf *msgb, void *argp, int nqueue)
{
   printf("\n[term_handler] %s TERMINATE message received. Stopping\n",MyName);
   servermode=0;  // This will terminate the server mode loop
   goon=0;        // This will terminate the main command loop
   return NO_ERROR;
}

static int gen_handler(MsgBuf *msgb, void *argp, int nq)
{
int lng=HDR_LEN(msgb);
char aux[200];

printf("\n[gen_handler] %s %s\n",MyName,StrHeader(msgb,aux,200));

if(lng>0) {
    unsigned char *body=MSG_BODY(msgb);
    char *tr="";
    int n,i;
    if(lng>40) {
       n=40; 
       tr="(trunc)";
    } else 
       n=lng;
    printf("Message body %s:",tr);
    i=n;
    while(i--) printf(" %2.2x",*(body++));
    printf("\n");
    body=MSG_BODY(msgb);
    printf("Message body %s:",tr);
    i=n;
    while(i--) printf("  %1c",*(body++));
    printf("\n");
}

thRelease(msgb);
return NO_ERROR;
}

static void measuretime(char* dest, char *numb, char *tm)
{
   int ncycl,stat, wait=1, nerrs=0;
   double tmout;
   int onlyone=0;
   double delay;
   double sum,ntot=0.0;
   double maxd=0.0,mind=10000000.;

   if(!dest) dest="";
   if(*dest=='*') dest="";
   if(numb) {
      ncycl=atoi(numb);
      if(ncycl<1) ncycl=1;
   } else {
      ncycl=1;
      onlyone=1;
   }

   if(*tm) {
       tmout=atof(tm);
       wait=(int)tmout;
   } else {
       tmout=1000.0;
       wait=1000;
   }
      
   printf("%s Sending %d ping to %s. Waiting %f ms for reply ...\n",MyName,ncycl,dest,tmout);
   while(ncycl--) {
      stat=thPing(dest,wait,&delay);
      if(stat<0) {
         printf("%s  error (%d) - %s\n",MyName,stat,lao_strerror(stat));
         if(nerrs++ > 10) {
             printf("%s  time loop exiting after 10 errors\n",MyName);
             return;
         }
      } else {
         delay *= 1000;
         ntot += 1.0;
         if(delay<mind) mind=delay;
         if(delay>maxd) maxd=delay;
         sum+=delay;
         if((ncycl%100)==0) printf("%10d\r",ncycl);
      }

   }
   if(onlyone) {
       printf("%s reply received in %f ms\n",MyName,delay);
   } else {
      double avg;
      avg=sum/ntot;
      printf("%s %d packets. delay - Min:%f Max:%f Avg:%f (ms)\n",MyName,(int)ntot,mind,maxd,avg);
   }
}

char inArgs[2048];

static void showstat(void)
{
qInfo *stat=thInfo();

printf("\n%s: %s - Dbg lev.:%d \n",
           MyName,GetVersionID(),logLevel);

printf("Start args: %s\n",inArgs);

printf("Msg Header Id: %d\n",MSG_HEADER_MAGIC);
if(stat->initialized)
    printf("Thrdlib initialized\n");
else
    printf("Thrdlib not initialized\n");
if(started)
    printf("Communication: active\n");
else
    printf("Communication: not active\n");
printf(" #  handlers: %d\n", stat->nHandlers);
printf(" #  tot bufs: %d\n", stat->nTot);
printf(" # wtng msgs: %d\n", stat->nWaiting);
printf(" # free bufs: %d\n", stat->nFree);
printf(" # rmvd msgs: %d\n", stat->nFlushd);
}

static void rqstinfo(char * client)
{
int stat,errcod;
MsgBuf *ret;
char *pt;

if(*client=='\0') {
  showstat();
  return;
}
if(*client=='*') 
   pt="";
else
   pt=client;
stat=thSendMsgPl(0,pt,RQSTINFO,0,NOHANDLE_FLAG,NULL);
if(stat<0) {
    printf("%s Error sending command: RQSTINFO (%d) %s\n",MyName,stat,lao_strerror(stat));
    return;
}

ret = thWaitMsg(INFOREPLY, "*", stat, 1000, &errcod);

if(!ret) {
   printf("%s Error while waiting for reply to info command: (%d) %s\n",MyName,errcod,lao_strerror(errcod));
   return;
}

printf("%s\n",(char *)MSG_BODY(ret));

thRelease(ret);

}

static void rqstvers(char * client)
{
   int vers;
   char *name;

   if(*client=='\0') {
      vers=GetVersion();
      name=MyName;
   } else {
      if(*client=='*') { 
         client=""; 
         name="MsgD"; 
      } else
         name="client"; 
      vers=thVersion(client);
   }
   if(IS_ERROR(vers)) 
      printf("%s Error from thVersion (%d): %s\n\n",MyName,vers,lao_strerror(vers));
   else
      printf("Version from %s: %d\n",name,vers);
}

static void hndlinfo(char * client)
{
int stat,errcod;
MsgBuf *ret;
char *pt;

if(*client=='*') 
   pt="";
else
   pt=client;
stat=thSendMsgPl(0,pt,HNDLINFO,0,NOHANDLE_FLAG,NULL);
if(stat<0) {
    printf("%s Error sending command: HNDLINFO (%d) %s\n",MyName,stat,lao_strerror(stat));
    return;
}

ret = thWaitMsg(INFOREPLY, "*", stat, 1000, &errcod);

if(!ret) {
   printf("%s Error while waiting for reply to info command: (%d) %s\n",MyName,errcod,lao_strerror(errcod));
   return;
}

printf("%s\n",(char *)MSG_BODY(ret));

thRelease(ret);

}

    

static int sendcommand(int cod, char* dest, int pload, int flag, char *msg)
{
int stat,len;

if(msg) 
   len=strlen(msg)+1;
else
   len=0;

stat=thSendMsgPl(len,dest,cod,pload,flag,msg);
if(stat<0) 
    printf("%s Error sending command: %d (%d) %s\n",MyName,cod,stat,lao_strerror(stat));
return stat;
}



static double randfact = 9.999999999/(double)RAND_MAX;
static char *digits[10] = { "0","1","2","3","4","5","6","7","8","9" };

static char* rdigit(void)
{
int ix = rand()*randfact;
return digits[ix];
}

static void expand(char *arg)
{
  char dummy[ARG_LNG];
  char chnow[30];
  struct timeval now;
  char *pd, *pa, *p;
  int lng,max;

  strcpy(dummy,arg);
  max=ARG_LNG-1;
  pd=dummy;pa=arg;lng=0;
  while(lng<max) {
     if(*pd=='\0') break;
     if(*pd=='\\') {
        pd++;
        switch(*pd++) {
           case '$': p=MyName; break;
           case '?': p=rdigit(); break;
           case '0': p=reg0; break;
           case '1': p=reg1; break;
           case '2': p=reg2; break;
           case '3': p=reg3; break;
           case '4': p=reg4; break;
           case '5': p=reg5; break;
           case '6': p=reg6; break;
           case '7': p=reg7; break;
           case '8': p=reg8; break;
           case '9': p=reg9; break;
           case 't': 
               gettimeofday(&now,NULL);
               timeString(&now,chnow);
               chnow[23]='\0';
               p=chnow+11;
               break;
           default:  p="\\"; break;
        }
        while(lng<max && *p) {
          *pa++= *p++;
          lng++;
        }
      } else {
         *pa++= *pd++;
         lng++;
      }
   }
   arg[lng]='\0';
}
 
int intv[40];
unsigned char intb[40];
double realv[40];
char v[40][ARG_LNG];
int i;

static void writevargeneric(char *name, char *type, char *cmdbuf,int mode)
{
int nok,tmout;
int stat=NO_ERROR;
char sbuff[4096];

nok=sscanf(cmdbuf,"%*s %*s %*s %s %s %s %s %s %s %s %s %s %s %s %s %s %s %s %s %s %s %s %s %s %s %s %s %s %s %s %s %s %s %s %s %s %s %s %s %s %s %s %s",
 v[0],  v[1],  v[2],  v[3],  v[4],  v[5],  v[6],  v[7],  v[8],  v[9],
v[10], v[11], v[12], v[13], v[14], v[15], v[16], v[17], v[18], v[19],
v[20], v[21], v[22], v[23], v[24], v[25], v[26], v[27], v[28], v[29],
v[30], v[31], v[32], v[33], v[34], v[35], v[36], v[37], v[38], v[39]);

if(*type=='s' && nok<1) {
    v[0][0]='\0';
    nok=0;
} 
if(nok<0) nok=0;
for(i=0;i<nok;i++) expand(v[i]);

if(mode) 
    tmout=1000;
else
    tmout=(-1);

switch(*type) {
int i,typecode,left;

case 'i':
    typecode=INT_VARIABLE;
    for(i=0;i<nok;i++) intv[i]=atoi(v[i]);
    stat=thWriteVar(name,typecode,nok,intv,tmout);
    break;
case 'r':
    typecode=REAL_VARIABLE;
    for(i=0;i<nok;i++) realv[i]=atof(v[i]);
    stat=thWriteVar(name,typecode,nok,realv,tmout);
    break;
case 's':
    typecode=CHAR_VARIABLE;
    left=4096;
    sbuff[0]='\0';
    for(i=0;i<nok;i++) {
       if(i>0) { strncat(sbuff," ",left); left--; }
       strncat(sbuff,v[i],left);
       left -= strlen(sbuff);
    }
    nok=strlen(sbuff);
    stat=thWriteVar(name,typecode,nok,sbuff,tmout);
    break;
case 'b':
    typecode=BIT8_VARIABLE;
    for(i=0;i<nok;i++) intb[i]=(atoi(v[i])%256);
    stat=thWriteVar(name,typecode,nok,intb,tmout);
    break;
default: 
    printf("%s Illegal variable type",MyName);
    return;
}
if(IS_ERROR(stat)) {
     printf("%s Error from thWriteVar (%d) %s - writing variable: %s\n\n",MyName,stat,lao_strerror(stat),name);
}
}

static void writevarfromfile(char *name, char *file)
{
   FILE *fd;
   char buff[1000000];
   int nc;

   fd=fopen(file,"r");
   if(fd) {
      nc=fread(buff, 1, 1000000, fd);
      if(nc<0) 
         printf("%s Error reading file %s (%s)\n\n",MyName,file,strerror(errno));
      else {
         int stat;
         stat=thWriteVar(name,BIT8_VARIABLE,nc,buff,2000);
         if(IS_ERROR(stat))
            printf("%s Error from thWriteVar (%d) %s - writing variable: %s\n\n",MyName,stat,lao_strerror(stat),name);
      }
      fclose(fd);
   } else {
      printf("%s Error opening file %s (%s)\n\n",MyName,file,strerror(errno));
   }
}


static void writevar(char *name, char *type, char *cmdbuf,int mode)
{
   if(strlen(type)==1)  
      writevargeneric(name, type, cmdbuf, mode);
   else
      writevarfromfile(name, type);
}

static void getvar(char *vname, char *reg)
{
MsgBuf *var;
Variable * theVar;
int stat;

var=thGetVar(vname,100,&stat);

if(!var) {
    printf("%s thGetVar(%s) returned error(%d): %s\n",vname,MyName,stat,lao_strerror(stat));
    return;
}
theVar=thValue(var);
DisplayVariable(theVar,reg);
thRelease(var);
}

static void getvarlist(char *pref)
{
MsgBuf *vars;
Variable * theVar;
int stat;

vars=thReadVar(pref,200,&stat);

if(!vars) {
    printf("%s thReadVar(%s) returned error(%d): %s\n",pref,MyName,stat,lao_strerror(stat));
    return;
}
while(vars)  {
    theVar=thValue(vars);
    DisplayVariable(theVar,NULL);
    vars=thNextVar(vars);
}
}

static void getstartt(char *arg0)
{
   int stat;
   struct timeval tv;
   char tstr[30];

   if(*arg0=='*') arg0[0]='\0';
   stat=thGetStartT(arg0,&tv);
   if(IS_NOT_ERROR(stat)) {
      char *pt;
      if(arg0[0]=='\0')
         pt="MsgD";
      else
         pt=arg0;
      timeString(&tv,tstr);
      printf("%s started @ %s",pt,tstr);
   }  else {
      printf("thGetStartT(%s) returned error(%d): %s\n",arg0,stat,lao_strerror(stat));
   }
}

static void haveyou(char * client)
{
int stat;

stat=thHaveYou(client);

if(stat<0)
   printf("thHaveYou(%s) returned error(%d): %s\n",client,stat,lao_strerror(stat));
else if(stat==0)
   printf("No, I have not %s",client);
else
   printf("Yes, I have %s",client);
}


static int setidlvar(int mode) // Create a fake idl status variable
{
   int stat;
   char name[]="AOARB.x.IDL_STAT";
   name[6]= *aosside;
   int ival;
   ival = mode?1:0;

   stat=thWriteVar(name,INT_VARIABLE,1,&ival,1000);
   if(IS_ERROR(stat)) {
       printf("%s Error %d (%s) writing variable: %s",MyName, stat,lao_strerror(stat),name);
       return 0;
   }
   return 1;
}

static void getCPlist(int l,char *dest)
{
MsgBuf *vars;
Variable * theVar;
int stat;
char *rn,*sp;

if(l==1) {
    rn="thClntsList()";
    sp="client";
    vars=thClList(200,dest,&stat);
} else {
    printf("Internal error in GetCPlist");
}

if(!vars) {
    printf("%s %s returned error(%d): %s\n",MyName,rn,stat,lao_strerror(stat));
    return;
}
printf("%s MsgD %s list:\n",MyName,sp);
while(vars)  {
    theVar=thValue(vars);
    printf("   %s\n",theVar->Value.Sv);
    vars=thNextVar(vars);
}
}

static void getbuflist(void)
{
MsgBuf *vars;
Variable * theVar;
int stat;

vars=thBufsList(200,&stat);

if(!vars) {
    printf("%s thBufsList() returned error(%d): %s\n",MyName,stat,lao_strerror(stat));
    return;
}
printf("%s MsgD shared buffer list:\n",MyName);
while(vars)  {
    theVar=thValue(vars);
    printf("   %s\n",theVar->Value.Sv);
    vars=thNextVar(vars);
}
}

static void help()
{
    printf("\nthrdtest  - %s.   L.Fini, V.Gavryusev\n\n",GetVersionID());
    printf("Generic tester for MsgD-RTDB (thrdlib based)\n\n");
    printf("Usage: thrdtest [-aos l/r] [-n] [-s] [-v] [-d n] [-m IP] [-p dir] [-e macro] [id] [IP]\n\n");

    printf("   thrdtest -h   print an help message\n\n");

    printf("   -aos x        Start in AOS test mode (specify either left or right).\n");
    printf("   -d nn         Set initial debug level (0, 1, 2).\n");
    printf("   -e script[.p] At start execute commands from file script.p\n");
    printf("   -l fname      Print log output (when executing script) to given file.\n");
    printf("                 Default is: stdout. See also: -v.\n");
    printf("   -m IP         Connect to MsgD at given IP number (default: \"127.0.0.1:9752\")\n");
    printf("                 Note: server IP may be also specified as second argument\n");
    printf("   -n            Do not start communication.\n");
    printf("   -p dir        Set default directory for procedures.\n");
    printf("   -s            Do no start command prompt. Just wait for messages\n");
    printf("                 after executing initial macro (if specified).\n");
    printf("   -v            Verbose mode (print commands executed from scripts).\n");
    printf("                 Commends are writte to stdout or to file specified with -l.\n");
    printf("   id            Int. digit: generate unique client name appending the digit to THRDTEST.\n");
    printf("                 String: use it as client's name.\n");
    printf("                 Default: \"THRDTEST\".\n");
    printf("   IP            Server IP:port address (see also -m). Only numeric values accepted.\n\n");

}

static void notstartedwarn(void)
{
    printf("Warning: you must first start communication with MsgD-RTDB!\n"); 
}

static void notinitwarn(void)
{
     printf("Warning: you must first reinitialize using \"init\"!\n"); 
}

static void cleanall(void)
{
int stat;

if(!initialized) {
    notinitwarn();
    return;
}
stat=thCleanup();
if(IS_ERROR(stat)) {
    printf("%s Error %d (%s) from thCleanup() .. exiting",MyName, stat,lao_strerror(stat));
    exit(1);
}
initialized=0;
started=0;
}

static void logset(int mode, char *dest, int lev)
{

if(*dest) {
    if(!started) { notstartedwarn(); return; }
    if(*dest=='*') dest="";
    switch(mode) {
       case 0:
          sendcommand(LOGLEVSET,dest,lev,0,NULL);
          break;
       case 1:
          sendcommand(LOGDECR,dest,0,0,NULL);
          break;
       case 2:
          sendcommand(LOGINCR,dest,0,0,NULL);
          break;
    }
} else {
    switch(mode) {
       case 0:
          logLevel=thDebug(lev);
          break;
       case 1:
          logLevel=thDebug(logLevel-1);
          break;
       case 2:
          logLevel=thDebug(logLevel+1);
          break;
    }
}
}

static void initall(char *newName)
{
   int stat;

   if(initialized) {
      printf("%s You must first use clean to call thCleanup()\n",MyName);
      return;
   }
   if(newName[0]!=0) {
      snprintf(MyName,PROC_NAME_LEN+1,"%s",newName);
      snprintf(prompt,80,"\n%s cmd: ",MyName);
   }
   if(logLevel) printf("%s Init thrdlib ...\n",MyName);

   if(IS_ERROR(stat=thInit(MyName))) {
      printf("%s Error %d (%s) from thInit() .. exiting",MyName, stat,lao_strerror(stat));
      exit(1);
   }

   if(logLevel) printf("%s Installing terminate handler ...\n",MyName);
   if((stat=thHandler(TERMINATE,"*",0,term_handler,"term_handler",NULL))<0) {
       printf("%s Error %d (%s) from thHandler() .. exiting",MyName, stat,lao_strerror(stat));
       exit(1);
   }
   if(logLevel) printf("%s Installing delay handler ...\n",MyName);
   if((stat=thHandler(DUMMYMSG,"*",0,delay_handler,"delay_handler",NULL))<0) {
       printf("%s Error %d (%s) from thHandler() .. exiting",MyName, stat,lao_strerror(stat));
       exit(1);
   }
   if(logLevel) printf("%s Installing variable notification handler ...\n",MyName);
   if((stat=thHandler(VARCHANGD,"*",0,notif_handler,"notif_handler",NULL))<0) {
       printf("%s Error %d (%s) from thHandler() .. exiting",MyName, stat,lao_strerror(stat));
       exit(1);
   }
   if(logLevel) printf("%s Installing generic handler ...\n",MyName);
   if((stat=thHandler(ANY_MSG,"*",0,gen_handler,"gen_handler",NULL))<0) {
       printf("%s Error %d (%s) from thHandler() .. exiting",MyName, stat,lao_strerror(stat));
       exit(1);
   }

   initialized=1;
}

static void enable(char* client, int yesno)
{
int lng=strlen(client)+1;

thSendMsgPl(lng,"",LOGCNTRL,yesno,0,client);

}



static void start(char* server)
{
int stat;
char ip[40];

snprintf(ip,40,"%s",server);
NormalizeIP(ip,40,NULL);

if(started) {
    printf("%s Communication is active. Cannot start!\n",MyName);
    return;
}
printf("Trying to connect to MsgD @ %s\n",ip);

if(IS_ERROR(stat=thStart(ip,1))) {
    printf("%s Error %d (%s) from thStart() .. \n",MyName, stat,lao_strerror(stat));
    return;
}
started=1;
}

static void stop()
{
if(!started) {
    printf("%s Not started. Cannot stop!\n",MyName);
    return;
}
thClose();
started=0;
printf("%s Closed communication with MsgD-RTDB\n",MyName);
}

static FILE* input=NULL;
long loopStart;
int loopIterations=(-1);
int loopDecr;

static char *my_getline()
{
   char *pt;
   pt=fgets(chbuf,2048,input);
   if(!pt) return NULL;
   if ((pt=index(chbuf,'\n'))) *pt='\0';
   return chbuf;
}

static void setloop(int nit)
{
   loopStart=ftell(input);
   if(nit>0) {
      loopIterations=nit;
      loopDecr=1;
   } else {
      loopIterations=1;
      loopDecr=0;
   }
}

static int endloop(void)
{
   if(loopIterations<0) return 0;

   if(loopIterations==0) return 1;

   loopIterations -= loopDecr;
   fseek(input,loopStart,SEEK_SET);
   return 1;
}

static int my_setinput(char *proc)
{
   char fname[PATH_LEN];

   if(!proc) {
      input=stdin;
      return 0;
   }
   if(*proc) {
      if(strchr(proc,'.')) 
          snprintf(fname,PATH_LEN,"%s/%s",def_dir,proc);
      else
          snprintf(fname,PATH_LEN,"%s/%s.p",def_dir,proc);

      input=fopen(fname,"r");
      if (input) return 1;
      input=stdin;
      printf("\n%s Cannot open procedure file: %s\n",MyName,fname);
      return 0;
   }
   printf("\n%s You must specify a procedure name!",MyName);
   input=stdin;
   return 0;
}

#ifdef HAS_HISTORY

#include <readline/readline.h>
#include <readline/history.h>

#define my_using_history using_history


static char *has_history = "Yes";

static char *my_readline(char *prompt, int script) 
{
char *ret,*pt;

if(script)
    ret = my_getline();
else
    ret = readline(prompt);
if(ret) {
    pt=index(ret,'#');
    if(pt) *pt='\0';
}

return ret;
}



static void my_free(char *cmdbuf, int script)
{
   if(script) return;
   if(cmdbuf) free(cmdbuf);
}

static void my_add_history(char *cmdbuf, int script)
{
   if(script) return;
   if(cmdbuf && *cmdbuf) {
       add_history(cmdbuf);
       stifle_history(50);
   }
}

#else

static char *has_history = "No";

static char *my_readline(char *prompt, int script)
{
char *ret,*pt;

if(!script) {
   printf(prompt);
   fflush(stdout);
}
ret = my_getline();
if(ret) {
    pt=index(ret,'#');
    if(pt) *pt='\0';
}

return ret;
}

static void my_free(char *cmdbuf, int script) { }

static void my_using_history(void) { }

static void my_add_history(char *cmdbuf, int script) { }

#endif

char *ok="OK";

char errbuf[1024];

static char *strstat(int stat)  // Returns a string with status/error explanation
{
char *ret=ok;

if(IS_ERROR(stat)) {
snprintf(errbuf,1024,"%d: %s",stat,lao_strerror(stat));
ret=errbuf;
} 

return ret;
}


static int aosenabled(void)  // Check AOS mode
{
if(aosside) return 1;
printf("\nAOS commands are enabled only when program is started with -aos option!\n\n");
return 0;
}

//@Main: 

// Test Msgd-RTDB variable management
//
//  This program connects to the MsgD-RTDB process to test various
//  function, including variable access.
//@
//

static void showregister(char *rgnumb)
{
char *pt;

switch(*rgnumb) {
   case '0': pt=reg0; break;
   case '1': pt=reg1; break;
   case '2': pt=reg2; break;
   case '3': pt=reg3; break;
   case '4': pt=reg4; break;
   case '5': pt=reg5; break;
   case '6': pt=reg6; break;
   case '7': pt=reg7; break;
   case '8': pt=reg8; break;
   case '9': pt=reg9; break;
   default: pt=NULL; break;
}
if(pt)
    printf("Register #%c: %s\n",*rgnumb,pt);
else
    arg_error();
}


char cmd[CMD_LNG];
char arg0[ARG_LNG];
char arg1[ARG_LNG];
char arg2[ARG_LNG];
char arg3[ARG_LNG];
char arg4[ARG_LNG];
char arg5[ARG_LNG];
char arg6[ARG_LNG];
char arg7[ARG_LNG];
char arg8[ARG_LNG];
char arg9[ARG_LNG];

int logSeqN = 0;

static void rtrim(char *buff)
{
   char *p=buff+strlen(buff)-1;
   for(;p>buff;p--) {
      if(! isspace(*p)) break;
      *p='\0';
   }
}

int main(int argc, char **argv) 
{
    int i,id,script;
    int dostart=1;
    char *server;
    char *cmdbuf;
    int loSeqN=0;
    struct sigaction act;
    char *isverb;

    struct timeval tv;

    FILE *outf=stdout;

    def_dir[PATH_LEN]='\0';
    exec_file[PATH_LEN]='\0';
    server="127.0.0.1";

    inArgs[0]='\0';

    for(i=1;i<argc;i++) {     // Save command arguments
       if(strlen(inArgs)+strlen(argv[i])>(sizeof(inArgs)-10)) {
          strcat(inArgs,"...");
          break;
       }
       strcat(inArgs,argv[i]);
       strcat(inArgs," ");
    }

    SetVersion(VERSMAJ,VERSMIN);
    i=1;
    for(i=1;i<argc;i++) {     // Process command arguments
        if(*argv[i] != '-') break;

	     if(strcmp(argv[i],"-h")==0) {
	         help();
	         exit(0);
	     }

	     if(strcmp(argv[i],"-aos")==0) {
           if(++i>=argc) { help(); exit(0); }
           
           switch(toupper(argv[i][0])){
           case 'L':
               aosside="L";
               break;
           case 'R':
               aosside="R";
               break;
           default:
               printf("\n-aos requires either L or R\n");
	       exit(0);
           }
           continue;
        }

	     if(strcmp(argv[i],"-d")==0) {
            int n;
            if(++i>=argc) { help(); exit(0); }

            n=atoi(argv[i]);
            if(n<=0) logLevel=thDebug(MASTER_LOG_LEV_INFO);
            if(n==1) logLevel=thDebug(MASTER_LOG_LEV_VTRACE);
            if(n>1) logLevel=thDebug(MASTER_LOG_LEV_VVTRACE);
            continue;
        }

	     if(strcmp(argv[i],"-e")==0) {
           if(++i>=argc) { help(); exit(0); }
           strncpy(exec_file,argv[i],PATH_LEN);
           continue;
        }
	     if(strcmp(argv[i],"-p")==0) {
           if(++i>=argc) { help(); exit(0); }
           strncpy(def_dir,argv[i],PATH_LEN);
           continue;
        }
	     if(strcmp(argv[i],"-v")==0) {
	         verbose=1;
            continue;
	     }
	     if(strcmp(argv[i],"-l")==0) {
            if(++i>=argc) { help(); exit(0); }
            strncpy(log_file,argv[i],PATH_LEN);
            log_file[PATH_LEN]='\0';
            continue;
	     }
	     if(strcmp(argv[i],"-s")==0) {
	         servermode=1;
            continue;
	     }
	     if(strcmp(argv[i],"-m")==0) {
           if(++i>=argc) { help(); exit(0); }
           server=argv[i];
           continue;
        }
	     if(strcmp(argv[i],"-n")==0) {
	         dostart=0;
            continue;
	     }
       
    }

    if(log_file[0]) {
       verbose=1;
       outf=fopen(log_file,"a");
       if(!outf) {
          printf("Cannot open log file: %s\n",log_file);
          outf=stdout;
       } else {
          printf("Printing logs to: %s\n",log_file);
       }
    }

    if(aosside) {    // In AOS test mode, set client name
        sprintf(MyName,"AOARB.%s",aosside);
    }  else {
        if(i<argc)  {         // Get first argument 
            if(strchr("1234567890",*argv[i])) {
                id=atoi(argv[i++]);                  // First argument is a number
                sprintf(MyName,"THRDTEST-%1.1d",id); // Generate client's name
            } else {                                 // First argument is desired
                strncpy(MyName,argv[i++],20);        // client name: set it
                MyName[20]='\0';
            }
        } else 
            sprintf(MyName,"THRDTEST"); // Set client's name
    }

    if(i<argc)  {         // Get second argument 
       server=argv[i++];
    }

    
    snprintf(prompt,80,"\n%s cmd: ",MyName);

    if(verbose)
        isverb="verbose";
    else
        isverb="quiet";

    printf("\n%s: %s - Dbg lev.:%d, %s, Line edit & history:%s\n\n",
           MyName,GetVersionID(),logLevel, isverb,   has_history);

    MainThread=pthread_self();

    act.sa_handler = sighand;
    sigemptyset(&act.sa_mask);
    act.sa_flags = 0;
    sigaction(SIGINT, &act,NULL);      // Install signal handler for Interrupt
    sigaction(SIGHUP, &act,NULL);      // and for SIGHUP

    initall("");

    *reg0='\0'; *reg1='\0'; *reg2='\0'; *reg3='\0'; *reg4='\0';
    *reg5='\0'; *reg6='\0'; *reg7='\0'; *reg8='\0'; *reg9='\0';

    gettimeofday(&tv,NULL);            // Use random seed for rand()
    srand((unsigned int)tv.tv_usec);

    if(dostart)
        start(server);
    else
        printf("%s Note: communication not started\n",MyName);

    cmdbuf=NULL;
    my_using_history();

    script=0;
    my_setinput(NULL);

    goon=1;
    while(goon) {
        int nok,stat;
        char *pc;

        if(exec_file[0]) {             // Execute initial commands
            script=my_setinput(exec_file);
            exec_file[0]='\0';
            continue;
        }
        my_add_history(cmdbuf,script);
        my_free(cmdbuf,script);
        if((!script) && servermode) break;
        cmdbuf=my_readline(prompt,script);
        if(!cmdbuf) {
           if (script) {
              script=0;
              my_setinput(NULL);
              if(servermode) break;
              continue;
           }
           else {
              printf("\n");
              break;
           }
        }

        *arg0='\0'; *arg1='\0'; *arg2='\0'; *arg3='\0'; *arg4='\0';
        *arg5='\0'; *arg6='\0'; *arg7='\0'; *arg8='\0'; *arg9='\0';

        pc=strchr(cmdbuf,'#');    // Remove comments
        if(pc) *pc='\0';

        nok=sscanf(cmdbuf,"%s %s %s %s %s %s %s %s %s %s %s",
                   cmd,arg0,arg1,arg2,arg3,arg4,arg5,arg6,arg7,arg8,arg9);
        if(nok<1) continue;    // Ignore empty lines

        expand(arg0);
        expand(arg1);
        expand(arg2);
        expand(arg3);
        expand(arg4);
        expand(arg5);
        expand(arg6);
        expand(arg7);
        expand(arg8);
        expand(arg9);

        if(script && verbose) {
           char loghdr[MASTER_LOG_HEADER_LEN];
           logString(MyName,MASTER_LOG_LEV_INFO,NULL,loghdr);
           fprintf(outf,"%s M> %s %s %s %s %s %s %s %s %s %s %s\n",loghdr,cmd,arg0,arg1,arg2,arg3,arg4,arg5,arg6,arg7,arg8,arg9);
        }

                              // Commands which can be used when not initialized
        if(strcasecmp(cmd,"aos")==0) {
            aoshelp();
            continue;
        }
        if(strncasecmp(cmd,"help",2)==0) {
            cmdhelp();
            continue;
        }
        if(strncasecmp(cmd,"hint",2)==0) {
            hints();
            continue;
        }
        if(strncasecmp(cmd,"quit",1)==0) {
           servermode=0;     // Explicit quit, disable server mode
           break;
        }
        if(strncasecmp(cmd,"deb",3)==0) {
            int deblev=atoi(arg0);
            logset(0,arg1,deblev);
            continue;
        }
        if(strncasecmp(cmd,"decr",3)==0) {
            logset(1,arg0,0);
            continue;
        }
        if(strncasecmp(cmd,"incr",3)==0) {
            logset(2,arg0,0);
            continue;
        }
        if(strncasecmp(cmd,"dir",3)==0) {
            strncpy(def_dir,arg0,PATH_LEN);
            continue;
        }
        if(strncasecmp(cmd,"errcod",2)==0) {
            DbTable* pt;
            int errc;
            if(isdigit(*arg0)) {
                sscanf(arg0,"%d",&errc);
                errc=(-errc);
                pt=lao_errinfo(errc);
                if(pt)
                   printf("\nErrC %d  %s - %s\n",errc,pt->name,pt->descr);
                else
                   printf("\nErrC %d: Undefined\n",errc);
            } else {
                int none=1;
                while((pt=lao_errsrc(arg0))) {
                   none=0;
                   printf("\nErrC %d  %s - %s\n",pt->code,pt->name,pt->descr);
                }
                if(none)
                   printf("\nNo Matching string\n");
            }
            continue;
        }
        if(strncasecmp(cmd,"echo",2)==0) {

            if(arg0[0]=='-') {
                char out[4096];
                snprintf(out,4096,"%s %s %s %s %s %s %s %s %s", 
                                  arg1,arg2,arg3,arg4,arg5,arg6,arg7,arg8,arg9);
                rtrim(out);
                printf("%s",out);
                fflush(stdout);
            } else {
                printf("%s %s %s %s %s %s %s %s %s %s\n", 
                        arg0,arg1,arg2,arg3,arg4,arg5,arg6,arg7,arg8,arg9);
            }
            continue;
        }
        if(strncasecmp(cmd,"endloop",3)==0) {
            if(!endloop()) 
               printf("\n%s No loop active!\n",MyName);
            continue;
        }
        if(strncasecmp(cmd,"exec",3)==0) {
            if(script) {
               printf("\n%s Procedure nesting not allowed!\n",MyName);
               continue;
            }
            
            if(arg1[0]) setregister("0",arg1);
            if(arg2[0]) setregister("1",arg2);
            if(arg3[0]) setregister("2",arg3);
            if(arg4[0]) setregister("3",arg4);
            if(arg5[0]) setregister("4",arg5);
            if(arg6[0]) setregister("5",arg6);
            if(arg7[0]) setregister("6",arg7);
            if(arg8[0]) setregister("7",arg8);
            if(arg9[0]) setregister("8",arg9);
            script=my_setinput(arg0);
            continue;
        }
        if(strncasecmp(cmd,"exit",3)==0) {
           servermode=0;     // Explicit quit, disable server mode
           break;
        }
        if(strncasecmp(cmd,"flag",1)==0) {
            flghelp();
            continue;
        }
        if(strncasecmp(cmd,"loop",3)==0) {
            if(!script) {
               printf("\n%s Loop command may be used only in procedures!\n",MyName);
               continue;
            }
            if(*arg0) {
                int iterations=atoi(arg0);
                setloop(iterations);
            } else 
                arg_error();
            continue;
        }
        if(strncasecmp(cmd,"pause",2)==0) {
            char prompt[2048];
            if(!script) {
               printf("\n%s Pause command may be used only in procedures!\n",MyName);
               continue;
            }
            prompt[0]='\0';
            if(arg0[0]) { strncpy(prompt,arg0,2048); strncat(prompt," ",2048); }
            if(arg1[0]) { strncat(prompt,arg1,2048); strncat(prompt," ",2048); }
            if(arg2[0]) { strncat(prompt,arg2,2048); strncat(prompt," ",2048); }
            if(arg3[0]) { strncat(prompt,arg3,2048); strncat(prompt," ",2048); }
            if(arg4[0]) { strncat(prompt,arg4,2048); strncat(prompt," ",2048); }
            if(arg5[0]) { strncat(prompt,arg5,2048); strncat(prompt," ",2048); }
            if(arg6[0]) { strncat(prompt,arg6,2048); strncat(prompt," ",2048); }
            if(arg7[0]) { strncat(prompt,arg7,2048); strncat(prompt," ",2048); }
            if(arg8[0]) { strncat(prompt,arg8,2048); strncat(prompt," ",2048); }
            if(arg9[0]) { strncat(prompt,arg9,2048); strncat(prompt," ",2048); }
            if(prompt[0]) printf("%s",prompt);
            fgets(chbuf,2048,stdin);
            continue;
        }
        if(strncasecmp(cmd,"reg",3)==0) {
            if(*arg1) {
               char allargs[4096];
               strncpy(allargs,arg1,4096);
               if(*arg2) mystrcat(allargs,arg2,4096);
               if(*arg3) mystrcat(allargs,arg3,4096);
               if(*arg4) mystrcat(allargs,arg4,4096);
               if(*arg5) mystrcat(allargs,arg5,4096);
               if(*arg6) mystrcat(allargs,arg6,4096);
               if(*arg7) mystrcat(allargs,arg7,4096);
               if(*arg8) mystrcat(allargs,arg8,4096);
               if(*arg9) mystrcat(allargs,arg9,4096);

               setregister(arg0,allargs);
            } else
                showregister(arg0);
            continue;
        }
        if(strncasecmp(cmd,"whatis",2)==0) {
            int msgcode; int none=1;
            DbTable* dbinfo;

            if(*arg0) {
               if(isdigit(*arg0)) {
                   sscanf(arg0,"%x",&msgcode);
                   dbinfo=msgtxt_c(msgcode);            
                   if(dbinfo)
                       printf("\nmsgcode %x: %-15s - %s\n",msgcode,dbinfo->name,dbinfo->descr);
                   else
                       printf("\nmsgcode %x: Undefined code\n",msgcode);
               } else {
                   while((dbinfo=msgtxt_s(arg0))) {
                       none=0;
                       printf("\nmsgcode %x: %-15s - %s\n",dbinfo->code,dbinfo->name,dbinfo->descr);
                   }
                   if(none)
                       printf("\nstring %s: No match found",arg0);
               }
            } else
               mlist();
            continue;
        }
        if(strncasecmp(cmd,"sleep",3)==0) {
            int delay; int nsteps;
            nsteps=(atof(arg0)*10)+0.5;
            delay=100;
            while(nsteps--) {
               if(!goon) break;
               msleep(delay);
            }
            continue;
        }
        if(strncasecmp(cmd,"init",3)==0) {
            initall(arg0);
            continue;
        }
        if(!initialized) { notinitwarn(); continue; }

                                 // Commands which require previous initialization


        if(strncasecmp(cmd,"star",4)==0) {
            start(server);
            continue;
        }

        if(strncasecmp(cmd,"clea",3)==0) {
            cleanall();
            continue;
        }


        if(!started) { notstartedwarn(); continue; }
                                 // Commands which require previous start


        if(strncasecmp(cmd,"bdump",2)==0) {
            sendcommand(DUMPBUFS,arg0,0,0,NULL);
            continue;
        }
        if(strncasecmp(cmd,"blist",2)==0) {
            getbuflist();
            continue;
        }
        if(strncasecmp(cmd,"cdump",3)==0) {
            sendcommand(DUMPCLNS,arg0,0,0,NULL);
            continue;
        }
        if(strncasecmp(cmd,"clist",3)==0) {
            getCPlist(1,arg0);
            continue;
        }
        if(strncasecmp(cmd,"close",3)==0) {
            stop();
            continue;
        }
        if(strncasecmp(cmd,"del",3)==0) {
            if(strlen(arg0)==0) { arg_error(); continue; }
            thVarOp(arg0,DELVAR,0,100);
            continue;
        }
        if(strncasecmp(cmd,"dinfo",3)==0) {
            sendcommand(DUMPINFO,arg0,0,0,NULL);
            continue;
        }
        if(strncasecmp(cmd,"disable",3)==0) {
            if(strlen(arg0)==0) { arg_error(); continue; }
            enable(arg0,0);
            continue;
        }
        if(strncasecmp(cmd,"enable",3)==0) {
            if(strlen(arg0)==0) { arg_error(); continue; }
            enable(arg0,1);
            continue;
        }
        if(strncasecmp(cmd,"get",2)==0) {
            if(strlen(arg0)==0) { arg_error(); continue; }
            if(strlen(arg1)==0)
               getvar(arg0,NULL);
            else
               getvar(arg0,arg1);
            continue;
        }
        if(strncasecmp(cmd,"gstart",2)==0) {
           if(strlen(arg0)==0) { arg_error(); continue; }
           getstartt(arg0);
           continue;
        }
        if(strncasecmp(cmd,"handlers",3)==0) {
            hndlinfo(arg0);
            continue;
        }
        if(strncasecmp(cmd,"have",3)==0) {
            if(strlen(arg0)==0) { arg_error(); continue; }
            haveyou(arg0);
            continue;
        }
        if(strncasecmp(cmd,"info",3)==0) {
            rqstinfo(arg0);
            continue;
        }
        if(strncasecmp(cmd,"lock",3)==0) {
            if(strlen(arg0)==0) { arg_error(); continue; }
            thVarOp(arg0,LOCKVAR,1,100);
            continue;
        }
        if(strncasecmp(cmd,"log",3)==0) {
            char msg[1000];
            char *pt;
            int lev;
            if(strlen(arg0)==0) { arg_error(); continue; }
            *msg='\0';
            lev=atoi(arg0);
            if(*arg1) strncpy(msg,arg1,100);
            if(*arg2) { strcat(msg," "); strncat(msg,arg2,100); }
            if(*arg3) { strcat(msg," "); strncat(msg,arg3,100); }
            if(*arg4) { strcat(msg," "); strncat(msg,arg4,100); }
            if(*arg5) { strcat(msg," "); strncat(msg,arg5,100); }
            if(*arg6) { strcat(msg," "); strncat(msg,arg6,100); }
            if(*arg7) { strcat(msg," "); strncat(msg,arg7,100); }
            if(*arg8) { strcat(msg," "); strncat(msg,arg8,100); }
            if(*arg9) { strcat(msg," "); strncat(msg,arg9,100); }
            for(pt=msg; *pt; pt++) if(*pt=='|') *pt='\n';
            thLogMsg(lev,loSeqN++,msg);
            continue;
        }
        if(strncasecmp(cmd,"msg",1)==0) {
            int times,code,pload,flag=0;
            char *pt;

            sscanf(arg0,"%x",&times);
            sscanf(arg1,"%x",&code);
            sscanf(arg2,"%d",&pload);
            if(*arg4) {
                if(tolower(*arg3)=='n') flag=NOHANDLE_FLAG;
                if(tolower(*arg3)=='d') flag=DISCARD_FLAG;
                pt=arg4;
            } else {
                pt=arg3;
            }
            while(times--) sendcommand(code,pt,pload,flag,NULL);
            continue;
        }
        if(strncasecmp(cmd,"ndump",2)==0) {
            sendcommand(DUMPNOTIF,arg0,0,0,NULL);
            continue;
        }
        if(strncasecmp(cmd,"new",2)==0) {
            sendcommand(CLOSELOG,"",0,0,NULL);
            continue;
        }
        if(strncasecmp(cmd,"nonotif",3)==0) {
            if(strlen(arg0)==0) { arg_error(); continue; }
            thVarOp(arg0,VARNOTIF,0,100);
            continue;
        }
        if(strncasecmp(cmd,"notif",4)==0) {
            if(strlen(arg0)==0) { arg_error(); continue; }
            thVarOp(arg0,VARNOTIF,1,100);
            continue;
        }
        if(strncasecmp(cmd,"notrace",4)==0) {
            if(strlen(arg0)==0) { arg_error(); continue; }
            thVarOp(arg0,TRACEVAR,0,100);
            continue;
        }
        if(strncasecmp(cmd,"nowait",3)==0) {
            sendcommand(WAITCANCEL,"",0,0,NULL);
            continue;
        }
        if(strncasecmp(cmd,"plist",3)==0) {
            getCPlist(2,arg0);
            continue;
        }
        if(strncasecmp(cmd,"ready",3)==0) {
            sendcommand(SETREADY,"",0,0,NULL);
            continue;
        }
        if(strncasecmp(cmd,"set",2)==0) {
            writevar(arg0,arg1,cmdbuf,0);
            continue;
        }
        if(strncasecmp(cmd,"term",2)==0) {
            sendcommand(TERMINATE,arg0,0,0,NULL);
            if(arg0[0]=='\0') stop();
            continue;
        }
        if(strncasecmp(cmd,"time",2)==0) {
            measuretime(arg0,arg1,arg2);
            continue;
        }
        if(strncasecmp(cmd,"tmout",2)==0) {
            if(strlen(arg0)==0) { arg_error(); continue; }
            sendcommand(SETTMOUT,"",atoi(arg0),0,NULL);
            continue;
        }
        if(strncasecmp(cmd,"trace",2)==0) {
            if(strlen(arg0)==0) { arg_error(); continue; }
            thVarOp(arg0,TRACEVAR,1,100);
            continue;
        }
        if(strncasecmp(cmd,"ttl",2)==0) {
            int ttl;
            ttl=atoi(arg0);
            thDefTTL(ttl);
            continue;
        }
        if(strncasecmp(cmd,"unlock",1)==0) {
            if(strlen(arg0)==0) { arg_error(); continue; }
            thVarOp(arg0,LOCKVAR,0,-1);
            continue;
        }
        if(strncasecmp(cmd,"vdump",2)==0) {
            if(strlen(arg0)==0) { arg_error(); continue; }
            thVarOp(arg0,DUMPVARS,0,500);
            continue;
        }
        if(strncasecmp(cmd,"vers",2)==0) {
            rqstvers(arg0);
            continue;
        }
        if(strncasecmp(cmd,"vlist",2)==0) {
            if(strlen(arg0)==0) { arg_error(); continue; }
            getvarlist(arg0);
            continue;
        }
        if(strncasecmp(cmd,"vstat",2)==0) {
            sendcommand(DUMPVSTAT,arg0,0,0,NULL);
            continue;
        }
        if(strncasecmp(cmd,"wait",2)==0) {
            int ms=0;
            if(arg0[0]=='\0') { arg_error(); continue; }
            if(arg1[0]!='\0')  { ms=atoi(arg1); }
            waitready(arg0,ms);
            continue;
        }
        if(strncasecmp(cmd,"write",2)==0) {
            writevar(arg0,arg1,cmdbuf,1);
            continue;
        }

        if(strncasecmp(cmd,"aos.init",6)==0) {
            if(aosenabled()) {
                printf("aos_init:  initialization for side %s\n",aosside);
                stat = aos_init(aosside);
                printf("aos_init(%s) --> %s\n",aosside,strstat(stat));
                setidlvar(1);
            }
            continue;
        }
        if(strncasecmp(cmd,"aos.clean",6)==0) {
            if(aosenabled()) {
                stat = aos_clean();
                printf("aos_clean() --> %s\n",strstat(stat));
            }
            continue;
        }
        if(strncasecmp(cmd,"aos.idl",6)==0) {
            if(aosenabled()) {
                int mode;
                if(arg0[0]=='\0') { arg_error(); continue; }
                sscanf(arg0,"%d",&mode);
                if(setidlvar(mode))
                   printf("idl mode set to %d\n",mode);
            }
            continue;
        }
        if(strncasecmp(cmd,"aos.dbg",6)==0) {
            if(aosenabled()) {
                int dbglev;
                if(arg0[0]=='\0') { arg_error(); continue; }
                sscanf(arg0,"%d",&dbglev);
                stat = aos_debug(dbglev);
                printf("aos_debug(%d) --> %s\n",dbglev,strstat(stat));
            }
            continue;
        }
        if(strncasecmp(cmd,"aos.disable",6)==0) {
            if(aosenabled()) {
                stat = aos_enable(0);
                printf("aos_enable(0) --> %s\n",strstat(stat));
            }
            continue;
        }
        if(strncasecmp(cmd,"aos.enable",6)==0) {
            if(aosenabled()) {
                stat = aos_enable(1);
                printf("aos_enable(1) --> %s\n",strstat(stat));
            }
            continue;
        }
        if(strncasecmp(cmd,"aos.flatlist",6)==0) {
            if(aosenabled()) {
                stat = aos_flatlist(arg0);
                printf("aos_enable(1) --> %s\n",strstat(stat));
            }
            continue;
        }
        if(strncasecmp(cmd,"aos.isup",6)==0) {
            if(aosenabled()) {
                int timeout;
                if(arg0[0]=='\0') { arg_error(); continue; }
                sscanf(arg0,"%d",&timeout);
                stat = aos_isup(timeout);
                printf("aos_isup(%d) --> %s\n",timeout,strstat(stat));
            }
            continue;
        }
        if(strncasecmp(cmd,"aos.log",6)==0) {
            if(aosenabled()) {
                int onoff;
                if(arg0[0]=='\0') { arg_error(); continue; }
                sscanf(arg0,"%d",&onoff);
                stat = aos_log(onoff);
                printf("aos_log(%d) --> %s\n",onoff,strstat(stat));
            }
            continue;
        }
        if(strncasecmp(cmd,"aos.msg",6)==0) {
            msgexpl(arg1);
            continue;
        }
        if(strncasecmp(cmd,"aos.offload",6)==0) {
            if(aosenabled()) {
                double z[22];
                for(i=0;i<22;i++) z[i]=0.0;
                nok=sscanf(cmdbuf,"%s %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf",
                       cmd,z,z+1,z+2,z+3,z+4,z+5,z+6,z+7,z+8,z+9,z+10, z+11,z+12,z+13,z+14,z+15,z+16,z+17,z+18,z+19,z+20,z+21);
                printf("offload(");
                for(i=0;i<21;i++) printf("%f,",z[i]); 
                printf("%f) --> ",z[21]);
                stat = aos_offload(z);
                printf("%s\n",strstat(stat));
            } else {
               printf("Offload command not sent.\nAOS status incorrect. Call aos.enable to set operating mode");
            }
            continue;
        }

        if(strncasecmp(cmd,"aos.ver",6)==0) {
            printf("AOSLIB version: %s\n",aos_vers());
            continue;
        }

/* hexapod */
        if(strncasecmp(cmd,"hxp.init",6)==0) {
            if(aosenabled()) {
                double d1,d2,d3,d4,d5,d6;
                nok=sscanf(cmdbuf,"%s %lf %lf %lf %lf %lf %lf",cmd,&d1,&d2,&d3,&d4,&d5,&d6);
                if(nok<7) { arg_error(); continue; }
                stat = aos_hxpinit(d1,d2,d3,d4,d5,d6,50000);
                printf("aos_hxpinit(%f,%f,%f,%f,%f,%f,50000) --> %s\n",d1,d2,d3,d4,d5,d6,strstat(stat));
            }
            continue;
        }
        if(strncasecmp(cmd,"hxp.moveto",9)==0) {
            if(aosenabled()) {
                double d1,d2,d3,d4,d5,d6;
                nok=sscanf(cmdbuf,"%s %lf %lf %lf %lf %lf %lf",cmd,&d1,&d2,&d3,&d4,&d5,&d6);
                if(nok<7) { arg_error(); continue; }
                stat = aos_hxpmoveto(d1,d2,d3,d4,d5,d6,50000);
                printf("aos_hxpmoveto(%f,%f,%f,%f,%f,%f,50000) --> %s\n",d1,d2,d3,d4,d5,d6,strstat(stat));
            }
            continue;
        }
        if(strncasecmp(cmd,"hxp.moveby",9)==0) {
            if(aosenabled()) {
                double d1,d2,d3,d4,d5,d6;
                nok=sscanf(cmdbuf,"%s %lf %lf %lf %lf %lf %lf",cmd,&d1,&d2,&d3,&d4,&d5,&d6);
                if(nok<7) { arg_error(); continue; }
                stat = aos_hxpmoveby(d1,d2,d3,d4,d5,d6,50000);
                printf("aos_hxpmoveby(%f,%f,%f,%f,%f,%f,50000) --> %s\n",d1,d2,d3,d4,d5,d6,strstat(stat));
            }
            continue;
        }
        if(strncasecmp(cmd,"hxp.movesph",9)==0) {
            if(aosenabled()) {
                double d1,d2,d3;
                nok=sscanf(cmdbuf,"%s %lf %lf %lf",cmd,&d1,&d2,&d3);
                if(nok<4) { arg_error(); continue; }
                stat = aos_hxpmovsph(d1,d2,d3,50000);
                printf("aos_hxpmovsph(%f,%f,%f,50000) --> %s\n",d1,d2,d3,strstat(stat));
            }
            continue;
        }
        if(strncasecmp(cmd,"hxp.newref",6)==0) {
            if(aosenabled()) {
                stat = aos_hxpnewref(2000);
                printf("aos_hxpnewref(2000) --> %s\n",strstat(stat));
            }
            continue;
        }
        if(strncasecmp(cmd,"hxp.getpos",8)==0) {
            if(aosenabled()) {
                double *pos;
                printf("getting hexapod current position\n");
                pos = aos_hxpgetpos(5000);
                if (pos==NULL)
                   printf("aos_hxpgetpos(5000) --> NULL");
                else 
                   printf("aos_hxpgetpos(5000) -> %lf %lf %lf %lf %lf %lf\n",*pos,*(pos+1),*(pos+2),*(pos+3),*(pos+4),*(pos+5));
            }
            continue;
        }
        if(strncasecmp(cmd,"hxp.getabs",8)==0) {
            if(aosenabled()) {
                double *pos;
                pos = aos_hxpgetabs(5000);
                if (pos!=NULL)
                printf("aos_hxpgetabs(5000) -> %lf %lf %lf %lf %lf %lf\n",*pos,*(pos+1),*(pos+2),*(pos+3),*(pos+4),*(pos+5));
                if (pos==NULL)
                   printf("aos_hxpgetabs() --> NULL");
                else 
                   printf("aos_hxpgetabs() -> %lf %lf %lf %lf %lf %lf\n",*pos,*(pos+1),*(pos+2),*(pos+3),*(pos+4),*(pos+5));
            }
            continue;
        }
        if(strncasecmp(cmd,"hxp.brake",6)==0) {
            if(aosenabled()) {
                int onoff;
                sscanf(arg1,"%d",&onoff);
                stat = aos_hxpbrake(onoff,5000);
                printf("aos_hxpbrake(%d,5000)  --> %s\n",onoff,strstat(stat));
            }
            continue;
        }

        printf("\nIllegal command\n");
        printf("   type help for a list of commands\n");
        printf("   type aos for a list of AOS subcommands\n");
    }
    i=1;
    while(servermode) {   // In server mode wait forever
        if(i) printf("Entering server mode\n");
        i=0;
        msleep(500);
    }
    thCleanup();
    return 0;
}
