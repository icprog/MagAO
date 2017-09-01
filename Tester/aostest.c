//@File: aostest.c
//
// General test for the aoslib

// Command based utility to perform a variety of tests against the  aoslib
//  by using facilities of the thrdlib

//@


#define VERSMAJ 1
#define VERSMIN 2

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

#include "base/thrdlib.h"
#include "aoslib/aoslib.h"

#include "base/msgtxt.h"

#define PATH_LEN 2048



char MyName[11];
char AOSVAR[16];
int started=0;
int initialized=0;

int deb_lev=0;

static char chbuf[1024];
char def_dir[PATH_LEN+1]=".";

//@Procedure: cmdhelp
//
//  Print a list of accepted commands
//@

static void cmdhelp()
{
    printf("\nCommand list (commands are case insensitive):\n\n");
    printf("quit                   Terminate program\n");
    printf("-----------------\n");
    printf("init               AOS initialization \n");
    printf("clean              AOS cleanup\n");
    printf("dbg level          AOS debug level [0..4]\n");
    printf("isup timeout       AOS check if is up con timeout (int)\n");
    printf("log  int           AOS log - 1:activate, 0:deactivate\n");
    printf("msg code           Print explanation of AOS specific subcommand (no code: full list)\n");
    printf("offload z0 z1 ...  senmd an offload command (up to 22 zernike values)\n");
    printf("hxp_init eX eY eZ eA eB eG    Init hexapod (6 double arguments)\n");
    printf("moveto   eX eY eZ eA eB eG    Move hexapod in the current system\n");
    printf("moveby   dX dY dZ dA dB dG    Move hexapod relatively to the previous position in the current system\n");
    printf("movesph  R  eA eB             Move hexapod on a spherical surface\n");
    printf("newref                        Store current position as a new reference system\n");
    printf("getpos                        Return positition in the current reference system\n");
    printf("getabs                        Return positition in the absolute reference system\n");
    printf("brake                         Activate/deactivate brake\n");
    printf("start              AOS complex initialization \n");

}


static void DisplayVariable(Variable *var)
{

int i;
struct tm *tt;
#ifdef LONG_IS_64
struct timeval tv;
tv = tv_fm_tv32(var->H.MTime);
tt=localtime(&tv); 
#else
tt=localtime(&(var->H.MTime.tv_sec));
#endif
printf("%s: [%2.2d/%2.2d/%4.4d %2.2d:%2.2d:%2.2d.%6.6d]  owner:%s\n",
                                                     var->H.Name,
                                                     tt->tm_mday,
                                                     tt->tm_mon,
                                                     tt->tm_year+1900,
                                                     tt->tm_hour,
                                                     tt->tm_min,
                                                     tt->tm_sec,
                                                     (int)var->H.MTime.tv_usec,
                                                     var->H.Owner);
if(var->H.Type==INT_VARIABLE) 
            for(i=0;i<var->H.NItems;i++)
		     printf(" %ld",var->Value.Lv[i]);
else if(var->H.Type==REAL_VARIABLE) 
            for(i=0;i<var->H.NItems;i++)
		     printf(" %.2f",var->Value.Dv[i]);
else if(var->H.Type==CHAR_VARIABLE) {
            printf(" ");
            for(i=0;i<var->H.NItems;i++)
		        printf("%c",var->Value.Sv[i]);
} else if(var->H.Type==BIT8_VARIABLE) 
            for(i=0;i<var->H.NItems;i++)
		        printf("%02x",var->Value.B8[i]);
else if(var->H.Type==BIT16_VARIABLE) 
            for(i=0;i<var->H.NItems;i++)
		        printf("%04hx",var->Value.B16[i]);
else if(var->H.Type==BIT32_VARIABLE) 
            for(i=0;i<var->H.NItems;i++)
		        printf("%08lx",var->Value.B32[i]);
else if(var->H.Type==BIT64_VARIABLE) 
            for(i=0;i<var->H.NItems;i++)
		        printf("%016llx",var->Value.B64[i]);
else {
 	    printf("Bad variable buffer format\n");
}
printf("\n");
}


static int notif_handler(MsgBuf *msgb, void *argp, int nq)
{
Variable *theVar;

printf("Notification of  variable change received\n");

theVar=thValue(msgb);
DisplayVariable(theVar);
thRelease(msgb);

return NO_ERROR;
}

static int gen_handler(MsgBuf *msgb, void *argp, int nq)
{
printf("Msg - From:%-10s To:%-10s Code:-%6x pLoad:-%6d Flg:%2x SeqN:%d Len:%d Flags:\n",
          HDR_FROM(msgb),HDR_TO(msgb),HDR_CODE(msgb),HDR_PLOAD(msgb),HDR_FLAGS(msgb),HDR_SEQN(msgb), HDR_LEN(msgb));

thRelease(msgb);
return NO_ERROR;
}


int intv[40];
unsigned char intb[40];
double realv[40];
char v[40][80];

static void writevar(char *cmdbuf,int mode)
{
int nok,tmout;
int stat=NO_ERROR;
char cmd[80],name[80],type[80];

nok=sscanf(cmdbuf,"%s %s %s %s %s %s %s %s %s %s %s %s %s %s %s %s %s %s %s %s %s %s %s %s %s %s %s %s %s %s %s %s %s %s %s %s %s %s %s %s %s %s %s",cmd,name,type,
 v[0],  v[1],  v[2],  v[3],  v[4],  v[5],  v[6],  v[7],  v[8],  v[9],
v[10], v[11], v[12], v[13], v[14], v[15], v[16], v[17], v[18], v[19],
v[20], v[21], v[22], v[23], v[24], v[25], v[26], v[27], v[28], v[29],
v[30], v[31], v[32], v[33], v[34], v[35], v[36], v[37], v[38], v[39]);

nok=nok-3;

if(nok<1) {
    printf("Illegal command format");
    return;
}

if(mode) 
    tmout=1000;
else
    tmout=(-1);

switch(*type) {
int i,typecode;

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
    stat=thWriteVar(name,typecode,nok,v[00],tmout);
    break;
case 'b':
    typecode=BIT8_VARIABLE;
    for(i=0;i<nok;i++) intb[i]=(atoi(v[i])%256);
    stat=thWriteVar(name,typecode,nok,intb,tmout);
    break;
default: 
    printf("Illegal variable type");
    return;
}
if(IS_ERROR(stat)) {
     printf("Error from writevar (%d): %s\n\n",stat,lao_strerror(stat));
}
}



static void help()
{
    printf("\naostest  - %s.   V.Gavryusev\n\n",GetVersionID());
    printf("Generic tester for AOSLIB (thrdlib based)\n\n");
    printf("Usage: aostest  [-d n] [-p dir] S<ide> [<server>]\n\n");

    printf("       aostest -h   print an help message\n\n");

    printf("       -d nn     Set initial debug level (0, 1, 2).\n");
    printf("       -p dir    Set default directory for procedures.\n");
    printf("       Side      String: defines AOS Side.\n");
    printf("                        String: use it as client's name.\n");
    printf("       <server>  Server numeric address (default: \"127.0.0.1\")\n");
}

static void notstartedwarn(void)
{
printf("Warning: you must first start  MsgD-RTDB!\n");
}

static void notinitwarn(void)
{
printf("Warning: you must first reinitialize using \"init\"!\n"); 
}



static void initall(void)
{
int stat;

if(initialized) {
   printf("You must first use clean to call thCleanup()\n");
   return;
}
if(deb_lev) printf("Init thrdlib ...\n");
if(IS_ERROR(stat=thInit(MyName))) {
    printf("Error %d (%s) from thInit() .. exiting", stat,lao_strerror(stat));
    exit(1);
}
if(deb_lev) printf("Installing variable notification handler ...\n");
if((stat=thHandler(VARCHANGD,"*",0,notif_handler,"notif_handler",NULL))<0) {
    printf("Error %d (%s) from thHandler() .. exiting", stat,lao_strerror(stat));
    exit(1);
}
if(deb_lev) printf("Installing generic handler ...\n");
if((stat=thHandler(ANY_MSG,"*",0,gen_handler,"gen_handler",NULL))<0) {
    printf("Error %d (%s) from thHandler() .. exiting", stat,lao_strerror(stat));
    exit(1);
}

initialized=1;
}

static void start(char* server)
{
int stat;

if(started) {
    printf("Communication is active. Cannot start!\n");
    return;
}
if(IS_ERROR(stat=thStart(server,5))) {
    printf("Error %d (%s) from thStart() .. exiting\n", stat,lao_strerror(stat));
    exit(1);
}
started=1;
}

/*
static void stop()
{
if(!started) {
    printf("Not started. Cannot stop!\n");
    return;
}
thClose();
started=0;
printf("Closed communication with MsgD-RTDB\n");
}
*/

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



static FILE* input=NULL;

static char *my_getline()
{
   char *pt;
   chbuf[1024]='\0';
   pt=fgets(chbuf,1023,input);
   if(!pt) return NULL;
   if ((pt=index(chbuf,'\n'))) *pt='\0';
   return chbuf;
}

static int my_setinput(char *proc)
{
   char fname[PATH_LEN];

   if(proc) {
      snprintf(fname,PATH_LEN,"%s/%s.p",def_dir,proc);
      input=fopen(fname,"r");
      if (input) return 1;
      input=stdin;
      printf("\nCannot open procedure file: %s\n",fname);
      return 0;
   }
   input=stdin;
   return 1;
}

#ifdef HAS_HISTORY

#include <readline/readline.h>
#include <readline/history.h>

#define my_using_history using_history


static char *has_history = "Yes";

static char *my_readline(char *prompt, int macro) 
{
if(macro)
    return my_getline();
return readline(prompt);
}



static void my_free(char *cmdbuf, int macro)
{
   if(macro) return;
   if(cmdbuf) free(cmdbuf);
}

static void my_add_history(char *cmdbuf, int macro)
{
   if(macro) return;
   if(cmdbuf && *cmdbuf) {
       add_history(cmdbuf);
       stifle_history(50);
   }
}

#else

static char *has_history = "No";

static char *my_readline(char *prompt,int macro)
{
   if(!macro) {
      printf(prompt);
      fflush(stdout);
   }
   return my_getline();
}

static void my_free(char *cmdbuf, int macro) { }

static void my_using_history(void) { }

static void my_add_history(char *cmdbuf, int macro) { }

#endif

//@Main: 

// Test AOSLIB communication commands
//
//  This program connects to the MsgD-RTDB process to test various
//  functions, implemented in AOSLIB.
//@


char prompt[80];
char cmd[132];
char arg1[132];
char arg2[132];
char arg3[132];
char arg4[132];
char arg5[132];
char arg6[132];
char arg7[132];
char arg8[132];
char arg9[132];


int main(int argc, char **argv) 
{
    int i,macro;
    char *server;
    char *cmdbuf;

    char *side;
    int timeout,stat;
    double d1,d2,d3,d4,d5,d6;
    double z[22];
    double *pos;

    def_dir[PATH_LEN]='\0';

    SetVersion(VERSMAJ,VERSMIN);

    i=1;
    for(i=1;i<argc;i++) {     // Process command arguments
        if(*argv[i] != '-') break;

	if(strcmp(argv[i],"-h")==0) {
	    help();
	    exit(0);
	}

	if(strcmp(argv[i],"-d")==0) {
           if(++i>=argc) {
              help();
              exit(0);
           }
           deb_lev=atoi(argv[i]);
        }

	if(strcmp(argv[i],"-p")==0) {
           if(++i>=argc) {
              help();
              exit(0);
           }
           strncpy(def_dir,argv[i],PATH_LEN);
        }
       
    }

    if(i<argc)  {         // Get first argument 
        if(strchr("LR",*argv[i])) {
            side=argv[i++];                        // First argument is a side letter
            sprintf(MyName,"AOSTEST-%s",side);    // Generate client's name
            sprintf(AOSVAR,"set %1s.AOS_CONSTATUS i 0",side);  // Set AOS variable name
        } else {                                
            printf("Please, chose the AOSside (L/R) as a first argument !!!\n");
            exit(0);
        }
    } else {
        help();
        exit(0);
    }
    if(i<argc)
	server=argv[i++];
    else 
	server="127.0.0.1";

    snprintf(prompt,80,"\n%s cmd: ",MyName);

    printf("\n%s: %s - Debug lev.:%d - Line editing & history:%s\n\n",
           MyName,GetVersionID(),deb_lev,    has_history);
    thDebug(deb_lev);

    initall();
    if(!initialized) { notinitwarn(); exit (0); }

    cmdbuf=NULL;
    my_using_history();

    macro=0;
    my_setinput(NULL);


    start(server);
    if(!started) { notstartedwarn(); exit (0); }

    writevar(AOSVAR,0);



    while(1) {
        int nok;

        my_add_history(cmdbuf,macro);
        my_free(cmdbuf,macro);
        cmdbuf=my_readline(prompt,macro);
        if(!cmdbuf && macro) {
           macro=0;
           my_setinput(NULL);
           continue;
        }

        if(deb_lev && macro) printf("  M> %s\n",cmdbuf);

        *arg1='\0'; *arg2='\0'; *arg3='\0'; *arg4='\0';
        *arg5='\0'; *arg6='\0'; *arg5='\0'; *arg7='\0';
        *arg8='\0'; *arg9='\0';

        nok=sscanf(cmdbuf,"%s %s %s %s %s %s %s %s %s %s",
                   cmd,arg1,arg2,arg3,arg4,arg5,arg6,arg7,arg8,arg9);
        if(nok<1) continue;    // Ignore empty lines


        if(strncasecmp(cmd,"quit",1)==0) break;

/*  communication with the AOS */
        if(strncasecmp(cmd,"start",2)==0) {
            printf("AOS initialization for side %s\n",side);
            stat = aos_clean();
            printf("aos_clean   status= %d\n",stat);
            stat = aos_init(side);
            printf("aos_init   status= %d\n",stat);
            stat = aos_isup(500);
            printf("aos_isup   status= %d\n",stat);
            continue;
        }
        if(strncasecmp(cmd,"init",2)==0) {
            printf("aos_init:  initialization for side %s\n",side);
            stat = aos_init(side);
            printf("aos_init   status= %d\n",stat);
            continue;
        }
        if(strncasecmp(cmd,"clean",1)==0) {
            stat = aos_clean();
            printf("aos_clean   status= %d\n",stat);
            continue;
        }
        if(strncasecmp(cmd,"dbg",1)==0) {
            sscanf(arg1,"%d",&timeout);
            stat = aos_debug(timeout);
            printf("aos_debug   status= %d\n",stat);
            continue;
        }
        if(strncasecmp(cmd,"isup",2)==0) {
            sscanf(arg1,"%d",&timeout);
            stat = aos_isup(timeout);
            printf("aos_isup   status= %d\n",stat);
            continue;
        }
        if(strncasecmp(cmd,"log",1)==0) {
            sscanf(arg1,"%d",&timeout);
            stat = aos_log(timeout);
            printf("aos_log   status= %d\n",stat);
            continue;
        }
        if(strncasecmp(cmd,"msg",1)==0) {
            msgexpl(arg1);
            continue;
        }
        if(strncasecmp(cmd,"offload",2)==0) {
            for(i=0;i<22;i++) z[i]=0.0;
            nok=sscanf(cmdbuf,"%s %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf",
                       cmd,z,z+1,z+2,z+3,z+4,z+5,z+6,z+7,z+8,z+9,z+10, z+11,z+12,z+13,z+14,z+15,z+16,z+17,z+18,z+19,z+20,z+21);
            printf("offload request:");
            for(i=0;i<22;i++) printf(" %f",z[i]); 
            printf("\n");
            stat = aos_offload(z);
            printf("aos_offload status= %d\n",stat);
            continue;
        }


/* hexapod */
        if(strncasecmp(cmd,"hxp_init",5)==0) {
            nok=sscanf(cmdbuf,"%s %lf %lf %lf %lf %lf %lf",cmd,&d1,&d2,&d3,&d4,&d5,&d6);
            printf("hexapod initialization at  %lf %lf %lf %lf %lf %lf\n",d1,d2,d3,d4,d5,d6);
            stat = aos_hxpinit(d1,d2,d3,d4,d5,d6,50000);
            printf("aos_hxpinit   status= %d\n",stat);
            continue;
        }
        if(strncasecmp(cmd,"moveto",5)==0) {
            nok=sscanf(cmdbuf,"%s %lf %lf %lf %lf %lf %lf",cmd,&d1,&d2,&d3,&d4,&d5,&d6);
            printf("hexapod moving to  %lf %lf %lf %lf %lf %lf\n",d1,d2,d3,d4,d5,d6);
            stat = aos_hxpmoveto(d1,d2,d3,d4,d5,d6,50000);
            printf("aos_hxpmoveto   status= %d\n",stat);
            continue;
        }
        if(strncasecmp(cmd,"moveby",5)==0) {
            nok=sscanf(cmdbuf,"%s %lf %lf %lf %lf %lf %lf",cmd,&d1,&d2,&d3,&d4,&d5,&d6);
            printf("hexapod relative moving by  %lf %lf %lf %lf %lf %lf\n",d1,d2,d3,d4,d5,d6);
            stat = aos_hxpmoveby(d1,d2,d3,d4,d5,d6,50000);
            printf("aos_hxpmoveby   status= %d\n",stat);
            continue;
        }
        if(strncasecmp(cmd,"movesph",5)==0) {
            nok=sscanf(cmdbuf,"%s %lf %lf %lf",cmd,&d1,&d2,&d3);
            printf("hexapod  moving on spherical surface  %lf %lf %lf\n",d1,d2,d3);
            stat = aos_hxpmovsph(d1,d2,d3,50000);
            printf("aos_hxpmovsph   status= %d\n",stat);
            continue;
        }
        if(strncasecmp(cmd,"newref",1)==0) {
            stat = aos_hxpnewref(5000);
            printf("aos_hxpnewref   status= %d\n",stat);
            continue;
        }
        if(strncasecmp(cmd,"getpos",4)==0) {
            printf("getting hexapod current position\n");
            pos = aos_hxpgetpos(5000);
            if (pos!=NULL)
            printf("aos_hxpgetpos: %lf %lf %lf %lf %lf %lf\n",*pos,*(pos+1),*(pos+2),*(pos+3),*(pos+4),*(pos+5));
            continue;
        }
        if(strncasecmp(cmd,"getabs",4)==0) {
            printf("getting hexapod absolute position\n");
            pos = aos_hxpgetabs(5000);
            if (pos!=NULL)
            printf("aos_hxpgetabs: %lf %lf %lf %lf %lf %lf\n",*pos,*(pos+1),*(pos+2),*(pos+3),*(pos+4),*(pos+5));
            continue;
        }
        if(strncasecmp(cmd,"brake",2)==0) {
            sscanf(arg1,"%d",&timeout);
            stat = aos_hxpbrake(timeout,5000);
            printf("aos_hxpbrake    status= %d\n",stat);
            continue;
        }
        cmdhelp();
    }
    thCleanup();
    return 0;
}
