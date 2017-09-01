//@File: gentest.c
//
// General test for the MsgD-RTDB system

// Command based utility to perform a variety of tests of the MsgD-RTDB system.

//@

#define VERSMAJ  2
#define VERSMIN  3

#include <stdio.h>
#include <ctype.h>
#include <errno.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>

#include <math.h>

#include <time.h>
#include <sys/time.h>

#include "base/msglib.h"

char MyName[11];

int seqNum=140482;

//@Procedure: help
//
//  Print an usage message
//@

static void cmdhelp()
{
    printf("\nCommand list:\n\n");
    printf("Commands are case insensitive\n\n");
    printf("blist                Send a Buffer list dump request (Code:DUMPBUFS) \n");
    printf("clist                Send a Client list dump request (Code:DUMPCLNS)\n");
    printf("decr                 Decrease log verbosity of MsgDRTDB\n");
    printf("del name             Delete specified variable\n");
    printf("dumpvar pref*        Send a dumpvar request\n");
    printf("err1                 Simulate error #1\n");
    printf("get name             Get specified variable\n");
    printf("give pref*           declare public spec. variables\n");
    printf("help                 Print this help page\n");
    printf("incr                 Increase log verbosity of MsgDRTDB\n");
    printf("kill client          Send terminate command to client (No client: MsgDRTDB)\n");
    printf("lock pref*           Lock specified variables\n");
    printf("nonotify pref*       Cancel notification of given variables\n");
    printf("notify pref*         Request notification of given variables\n");
    printf("ping client          Send a PING request to client\n");
    printf("quit                 Terminate program\n");
    printf("ready                Notify that I'm ready\n");
    printf("set name ty v v ...  Set specified variable (Create if not existent)\n");
    printf("term                 Terminate MsgDRTDB\n");
    printf("take pref*           Become owner of spec. variables\n");
    printf("unlock pref*         Unlock specified variables\n");
    printf("varlist pref*        Get list of variables\n");
    printf("wait  client         Wait for client to become ready\n");

}

static int unexpected(MsgBuf *msgb)
{
    printf("Unexpected message from server:\n");
    printf("    From: %s\n",HDR_FROM(msgb));
    printf("      to: %s\n",HDR_TO(msgb));
    printf("  SeqNum: %d\n",HDR_SEQN(msgb));
    printf("    Code: %d\n",HDR_CODE(msgb));
    printf("  MsgLen: %d\n\n",HDR_LEN(msgb));

return NO_ERROR;
}


static void help()
{
    printf("\ngentest  - %s.   L.Fini\n\n",GetVersionID());
    printf("Generic tester for MsgD-RTDB\n\n");
    printf("Usage: gentest [-v] <Id> [<server>]\n\n");

    printf("       gentest -h   print an help message\n\n");

    printf("       Id        Integer digit. Use different values for concurrent run.\n");
    printf("       <server>  Server numeric address (default: \"127.0.0.1\")\n");
}


MsgBuf msgb;
int verbose=0,silent=0;


static int DisplayVariables()
{
    Variable *TheVar;
    int stat;
#ifdef LONG_Is_64
	 timeval tv;
#endif
	 
    if(IS_ERROR(stat=WaitMsgTimeout(&msgb,2000))) {
        printf("Error from server (%d): %s\n\n",stat,lao_strerror(stat));
        return 0;
    }
    if(hdr_CODE(msgb)==VARREPLY) {
        TheVar=(Variable *)msg_BODY(msgb);

    	int i;
        struct tm *tt;
#ifdef LONG_IS_64
                  struct timeval tv;
		  tv = tv_fm_tv32(TheVar->H.MTime);
		  tt = localtime(&(tv.tv_sec));
#else
        tt=localtime(&(TheVar->H.MTime.tv_sec)); 
#endif
        printf("%s: [%2.2d/%2.2d/%4.4d %2.2d:%2.2d:%2.2d.%6.6d]", 
                                                     TheVar->H.Name,
                                                     tt->tm_mday,
                                                     tt->tm_mon,
                                                     tt->tm_year,
                                                     tt->tm_hour,
                                                     tt->tm_min,
                                                     tt->tm_sec,
                                                     (int)TheVar->H.MTime.tv_usec);
        if(TheVar->H.Type==INT_VARIABLE) 
            for(i=0;i<TheVar->H.NItems;i++)
		     printf(" %ld",TheVar->Value.Lv[i]);
        else if(TheVar->H.Type==REAL_VARIABLE) 
            for(i=0;i<TheVar->H.NItems;i++)
		     printf(" %.2f",TheVar->Value.Dv[i]);

        else if(TheVar->H.Type==CHAR_VARIABLE) 
            for(i=0;i<TheVar->H.NItems;i++)
		        printf(" %s",TheVar->Value.Sv);
	else {
 	    printf("Bad variable buffer format\n");
	}
	printf("\n");
    }  else {
        unexpected(&msgb);
    }
    return 1;
}
	 
static void dumpvars(char *name)
{
int stat;

stat=DumpVars(MyName,name,&msgb);

if(IS_ERROR(stat))
    printf("Error calling DumpVars: (%d): %s\n",stat,lao_strerror(stat));

return;
}

static void lockvars(char *name)
{
int stat;

stat=LockVars(MyName,name,1000,unexpected,&msgb);

if(IS_ERROR(stat))
    printf("Error calling LockVars: (%d) %s\n",stat,lao_strerror(stat));

return;
}

static void notify(char *name,int yesno)
{
int stat;

if(yesno) {
    stat=VarNotif(MyName,name);

    if(IS_ERROR(stat))
        printf("Error calling VarNotif: (%d) %s\n",stat,lao_strerror(stat));
} else {
    stat=VarNoNotif(MyName,name);

    if(IS_ERROR(stat))
        printf("Error calling VarNoNotif: (%d) %s\n",stat,lao_strerror(stat));
}

return;
}

static void unlockvars(char *name)
{
int stat;

stat=UnlockVars(MyName,name,1000,unexpected,&msgb);

if(IS_ERROR(stat))
    printf("Error calling UnlockVars: (%d) %s\n",stat,lao_strerror(stat));

return;
}

static void removevar(char *name)
{
int stat;

stat=DelVar(MyName,name,1000,unexpected,&msgb);

if(IS_ERROR(stat))
    printf("Error calling DelVar: (%d) %s\n",stat,lao_strerror(stat));

return;
}

static void sendcommand(int cod, char* dest, int seqnum)
{
int stat;

FillHeader(0,seqnum,MyName, dest,cod,&msgb);  
stat=SendToServer(&msgb);                     // Send the message
if(IS_ERROR(stat))
    printf("Error sending command: %d (%d) %s\n",cod,stat,lao_strerror(stat));
}


static void waitready(char *name)
{
int stat;

for(;;) {
    stat=WaitClientReady(MyName, name,&msgb,unexpected,1000000);
    if(IS_NOT_ERROR(stat)) {
       printf("Client %s has become ready!\n",HDR_FROM(&msgb));
       break;
    }
    printf("Error from WaitClientReady: (%d) %s\n",stat,lao_strerror(stat));
}
}


static void getvarlist(char *name)
{
int stat,count=0;
Variable *TheVar;

stat=VarList(MyName,name,count,&msgb);
if(IS_ERROR(stat))  {
    printf("Error calling VarList: (%d) %s\n",stat,lao_strerror(stat));
    return;
}

while(1) {
    if(IS_ERROR(stat=WaitMsgTimeout(&msgb,2000))) {
        printf("Error from server (%d): %s\n\n",stat,lao_strerror(stat));
        break;
    }
    if(hdr_CODE(msgb)==VARREPLY) {
        TheVar=(Variable *)msg_BODY(msgb);

        printf("%-20s\n", TheVar->H.Name);
    }  else if(hdr_CODE(msgb)==VAREND) {
        printf("\n");
        break;
    }  else {
            printf("Unexpected message from server:\n");
            printf("    From: %s\n",hdr_FROM(msgb));
            printf("      to: %s\n",hdr_TO(msgb));
            printf("  SeqNum: %d\n",hdr_SEQN(msgb));
            printf("    Code: %x\n",hdr_CODE(msgb));
            printf("   pLoad: %d\n",hdr_PLOAD(msgb));
            printf("  MsgLen: %d\n\n",hdr_LEN(msgb));
    }

}
}

static void doerror1()
{
double dummy[3];
int stat;
    
stat= SetVar(MyName,"N!onexistent",REAL_VARIABLE,3,dummy,&msgb);

}

int intv[40];
unsigned char intb[40];
double realv[40];
char v[40][80];


static int Writevar(char *cmdbuf)
{
int nok;
int stat=NO_ERROR;
char cmd[80],name[80],type[80];

nok=sscanf(cmdbuf,"%s %s %s %s %s %s %s %s %s %s %s %s %s %s %s %s %s %s %s %s %s %s %s %s %s %s %s %s %s %s %s %s %s %s %s %s %s %s %s %s %s %s %s",cmd,name,type,
 v[0],  v[1],  v[2],  v[3],  v[4],  v[5],  v[6],  v[7],  v[8],  v[9],
v[10], v[11], v[12], v[13], v[14], v[15], v[16], v[17], v[18], v[19],
v[20], v[21], v[22], v[23], v[24], v[25], v[26], v[27], v[28], v[29],
v[30], v[31], v[32], v[33], v[34], v[35], v[36], v[37], v[38], v[39]);

nok=nok-3;

if(nok<1) return (-1);

switch(*type) {
int i,typecode;

case 'i':
    typecode=INT_VARIABLE;
    for(i=0;i<nok;i++) intv[i]=atoi(v[i]);
    stat=WriteVar(MyName,name,typecode,nok,intv,5000,unexpected,&msgb);
    break;
case 'r':
    typecode=REAL_VARIABLE;
    for(i=0;i<nok;i++) realv[i]=atof(v[i]);
    stat=WriteVar(MyName,name,typecode,nok,realv,5000,unexpected,&msgb);
    break;
case 's':
    typecode=CHAR_VARIABLE;
    stat=WriteVar(MyName,name,typecode,nok,v[00],5000,unexpected,&msgb);
    break;
case 'b':
    typecode=BIT8_VARIABLE;
    for(i=0;i<nok;i++) intb[i]=(atoi(v[i])%256);
    stat=WriteVar(MyName,name,typecode,nok,intb,5000,unexpected,&msgb);
    break;
default: return (-1);
}
return stat;
}

//@Main: 

// Test Msgd-RTDB variable management
//
//  This program connects to the MsgD-RTDB process to test various
//  function, including variable access.
//@


char cmdbuf[1024];
char cmd[1024];
char arg1[1024];
char arg2[1024];

int main(int argc, char **argv) 
{
    int i,stat,id;
    char *server;

    SetVersion(VERSMAJ,VERSMIN);

    i=1;
    for(i=1;i<argc;i++) {     // Process command arguments
        if(*argv[i] != '-') break;

	if(strcmp(argv[i],"-h")==0) {
	    help();
	    exit(0);
	}
    }

    if(i<argc)
        id=atoi(argv[i++]);
    else {
        help();
        exit(0);
    }
    if(i<argc)
	server=argv[i++];
    else 
	server="127.0.0.1";


    sprintf(MyName,"GENTEST-%1.1d",id);

    printf("%s connecting to server at: %s ...",MyName,server);
    fflush(stdout);

                                  // Connect to MsgDRTDB server
    if(IS_ERROR(stat=ConnectToServer(server,MyName,1))) {
	printf("Error connecting to server at: %s\n",server);
	printf("Error code: %d - %s\n\n",stat,lao_strerror(stat));
	return 1;
    }

    printf("done!\n");

    InitMsgBuf(&msgb,0);         // Initialize the message structure

    while(1) {
        int count=1,nok;
        char *pt;
        printf("%s cmd (help): ",MyName);
        fgets(cmdbuf,1024,stdin);
        if((pt=index(cmdbuf,'\n'))) *pt='\0';

        *arg1='\0';
        *arg2='\0';
        nok=sscanf(cmdbuf,"%s %s %s",cmd,arg1,arg2);
        if(nok<1) {
            cmdhelp();
            continue;
        }
        if(strncasecmp(cmd,"b",1)==0) {
            sendcommand(DUMPBUFS,"",count++);
            continue;
        }
        if(strncasecmp(cmd,"c",1)==0) {
            sendcommand(DUMPCLNS,"",count++);
            continue;
        }
        if(strncasecmp(cmd,"dec",3)==0) {
            sendcommand(LOGDECR,"",count++);
            continue;
        }
        if(strncasecmp(cmd,"del",3)==0) {
            removevar(arg1);
            continue;
        }
        if(strncasecmp(cmd,"du",2)==0) {
            dumpvars(arg1);
            continue;
        }
        if(strncasecmp(cmd,"e",1)==0) {
            doerror1();
            continue;
        }
        if(strncasecmp(cmd,"get",2)==0) {
          stat=GetVar(MyName,arg1,count++,&msgb);
	  if(DisplayVariables()==0) break;
          continue;
        }
        if(strncasecmp(cmd,"give",2)==0) {
            GiveVar(MyName,arg1,&msgb);
            continue;
        }
        if(strncasecmp(cmd,"help",1)==0) {
            cmdhelp();
            continue;
        }
        if(strncasecmp(cmd,"inc",1)==0) {
            sendcommand(LOGINCR,"",count++);
            continue;
        }
        if(strncasecmp(cmd,"kill",1)==0) {
            sendcommand(TERMINATE,arg1,count++);
            continue;
        }
        if(strncasecmp(cmd,"l",1)==0) {
            lockvars(arg1);
            continue;
        }
        if(strncasecmp(cmd,"nonotif",3)==0) {
            notify(arg1,0);
            continue;
        }
        if(strncasecmp(cmd,"notif",3)==0) {
            notify(arg1,1);
            continue;
        }
        if(strncasecmp(cmd,"ping",1)==0) {
            sendcommand(ECHO,arg1,count++);
            continue;
        }
        if(strncasecmp(cmd,"q",1)==0) break;

        if(strncasecmp(cmd,"ready",1)==0) {
            sendcommand(SETREADY,"",count++);
            continue;
        }
        if(strncasecmp(cmd,"set",1)==0) {
          stat=Writevar(cmdbuf);
          if(stat==(-1)) { cmdhelp(); continue; }
          if(IS_ERROR(stat)) {
            printf("Error from WriteVar (%d): %s\n\n",stat,lao_strerror(stat));
          }
          continue;
        }
        if(strncasecmp(cmd,"take",2)==0) {
            TakeVar(MyName,arg1,&msgb);
            continue;
        }
        if(strncasecmp(cmd,"term",2)==0) {
            sendcommand(TERMINATE,"",count++);
            continue;
        }
        if(strncasecmp(cmd,"unlock",1)==0) {
            unlockvars(arg1);
            continue;
        }
        if(strncasecmp(cmd,"var",3)==0) {
            getvarlist(arg1);
            continue;
        }
        if(strncasecmp(cmd,"wait",1)==0) {
            waitready(arg1);
            continue;
        }
        cmdhelp();
    }
    return 0;
}
