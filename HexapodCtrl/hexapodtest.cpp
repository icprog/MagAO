//@File: hexapodtest.cpp
//
// General test for the hexapod
//
// Command based utility to perform test on hexapod
//
//@

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

#include "Hexapod.h"
#include "Utils.h"


#define PATH_LEN 2048



char MyName[11];
int started=0;
int initialized=0;

static int logLevel=0;

static char chbuf[1024];
char def_dir[PATH_LEN+1]=".";

//@Procedure: cmdhelp
//
//  Print a list of accepted commands
//@

static void cmdhelp()
{
    printf("\nCommand list (commands are case insensitive):\n\n");
    printf("dir name               Set default directory for procedures\n");
    printf("exec name              Execute procedure from file name.p (see also: \"dir\")\n");
    printf("help                   Print this help page\n");
    printf("quit                   Terminate program \n");
    printf("sleep n                delay n seconds\n");
    printf("send  *                send raw command to hexapod. see UMAC documentation\n");
    printf("home                   start homing procedure. see CAN 610a046d\n");  
    printf("open                   open brakes.\n");
    printf("close                  close brakes.\n");
    printf("moveto x y z a b c     move to absolute position [micron arcsec]\n");
    printf("moveby x y z a b c     relative motion wrt to present position [micron arcsec]\n");
    printf("sphere r teta phi      move on sphere [m arcsec arcsec]\n");
    printf("getpos                 get present absolute position. [micron arcsec]\n");
    printf("status                 display global status of the hexapod\n");  
    printf("ishomed                check if homing done");
    printf("isready                true if hexapod is ready. see CAN 610a046d\n");
    printf("ismoving               true if hexapod is moving. see CAN 610a046d\n");
    printf("deinit                 deinitialize hexapod. see CAN 610a046d\n");  
}


static void help()
{
    printf("\nhexapodtest  - 2.71828  L.Busoni\n\n");
    printf("Generic tester of hexapod kinematics\n\n");
    printf("Usage: hexapodtest [-d n] [-p dir] [-f configfile]\n\n");

    printf("       hexapodtest -h   print an help message\n\n");

    printf("       -d nn          Set initial debug level (0, 1, 2).\n");
    printf("       -p dir         Set default directory for procedures.\n");
    printf("       -f configfile  Specify configuration file name\n");
}


//static void notinitwarn(void)
//{
//printf("Warning: you must first reinitialize using \"init\"!\n"); 
//}


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


static const char *has_history = "Yes";

static char *my_readline(char *prompt, int script) 
{
char *ret; //,*pt;

if(script)
    ret = my_getline();
else
    ret = readline(prompt);
//if(ret) {
//    pt=index(ret,'#');
//    if(pt) *pt='\0';
//}

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
       stifle_history(100);
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

//@Main: 

// Test Hexapod kinematics
//
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
    int         i,script;
    char        *cmdbuf;
    const char  *configfile = "conf/adsec/672b/processConf/hexapod.conf";
    Logger      *_logger;     

    Logger::setParentName("HEXAPODTEST");
    _logger = Logger::get("HEXAPODTEST", MASTER_LOG_LEV_DEBUG);	
    _logger->setLogFile(  "hexapodtest", Utils::getAdoptLog());

    def_dir[PATH_LEN]='\0';

    i=1;
    for(i=1;i<argc;i++) {     // Process command arguments
        if(*argv[i] != '-') break;

        if(strcmp(argv[i],"-h")==0) {
            help();
            exit(0);
        }

        if(strcmp(argv[i],"-p")==0) {
            if(++i>=argc) {
                help();
                exit(0);
            }
            strncpy(def_dir,argv[i],PATH_LEN);
        }

        if(strcmp(argv[i],"-f")==0) {
            if(++i>=argc) {
                help();
                exit(0);
            }
            configfile = argv[i];
        }
       
    }

    Hexapod hexa(configfile);

    Logger::get("HEXAPOD")->setLevel(Logger::LOG_LEV_TRACE);

    if (hexa.isSimulator())
        snprintf(prompt,80,"\nHexapod SIMULATOR cmd: ");
    else
        snprintf(prompt,80,"\nHexapod cmd: ");

    printf("\nHexapod:  Line editing & history:%s\n\n", has_history);

    cmdbuf=NULL;
    my_using_history();

    script=0;
    my_setinput(NULL);

    while(1) {
        int nok;

        my_add_history(cmdbuf,script);
        my_free(cmdbuf,script);
        cmdbuf=my_readline(prompt,script);
		_logger->log(Logger::LOG_LEV_INFO, "got readline %s", cmdbuf);
        if(!cmdbuf && script) {
           script=0;
           my_setinput(NULL);
           continue;
        }

        *arg1='\0'; *arg2='\0'; *arg3='\0'; *arg4='\0';
        *arg5='\0'; *arg6='\0'; *arg5='\0'; *arg7='\0';
        *arg8='\0'; *arg9='\0';

        nok=sscanf(cmdbuf,"%s %s %s %s %s %s %s %s %s %s",
                   cmd,arg1,arg2,arg3,arg4,arg5,arg6,arg7,arg8,arg9);
        if(nok<1) continue;    // Ignore empty lines

        if(logLevel && script) printf("  M> %s\n",cmdbuf);

                              // Commands which can be used when not initialized
        if(strncasecmp(cmd,"help",2)==0) {
            cmdhelp();
            continue;
        }
        if(strncasecmp(cmd,"quit",4)==0) break;
        if(strncasecmp(cmd,"dir",3)==0) {
            strncpy(def_dir,arg1,PATH_LEN);
            continue;
        }
        if(strncasecmp(cmd,"exec",4)==0) {
            if(script) {
               printf("\nProcedure nesting not allowed!\n");
               continue;
            }
            my_add_history(cmdbuf,script);
            my_free(cmdbuf,script);
            script=my_setinput(arg1);
            continue;
        }
        if(strncasecmp(cmd,"sleep",3)==0) {
            int delay;
            delay=atoi(arg1);
            sleep(delay);
            continue;
        }
        // Hexapod commands
        try {
           if(strncasecmp(cmd,"send",4)==0) {
               string cCommand;
               if (*arg1 != '\0') { cCommand += string(arg1)+" ";}
               if (*arg2 != '\0') { cCommand += string(arg2)+" ";}
               if (*arg3 != '\0') { cCommand += string(arg3)+" ";}
               if (*arg4 != '\0') { cCommand += string(arg4)+" ";}
               cout << "Answer: " << hexa.sendCmd(cCommand) << endl;
               continue;
           }
           if(strncasecmp(cmd,"deinit",5)==0) {
               hexa.DeInit();
               continue;
           }
           if(strncasecmp(cmd,"ishome",6)==0) {
               bool ret=hexa.isHomed();
               cout << "Homed: "<< ((ret==true) ? "yes" : "no") << endl;
               continue;
           }
           if(strncasecmp(cmd,"home",3)==0) {
               hexa.Home();
               continue;
           }
           if(strncasecmp(cmd,"open",3)==0) {
               hexa.OpenBrake();
               continue;
           }
           if(strncasecmp(cmd,"close",3)==0) {
               hexa.CloseBrake();
               continue;
           }
           if(strncasecmp(cmd,"moveto",6)==0) {
               if (*arg1 != '\0' && *arg2 != '\0' && *arg3 != '\0' &&
                   *arg4 != '\0' && *arg5 != '\0' && *arg6 != '\0') {
                   HexaTuple pos( atof(arg1), atof(arg2), atof(arg3),
                                  atof(arg4), atof(arg5), atof(arg6) );
                   hexa.MoveTo(pos);
               } else {
                   cout << "MoveTo requires 6 parameters" << endl;
               }
               continue;
           }
           if(strncasecmp(cmd,"moveby",6)==0) {
               if (*arg1 != '\0' && *arg2 != '\0' && *arg3 != '\0' &&
                   *arg4 != '\0' && *arg5 != '\0' && *arg6 != '\0') {
                   HexaTuple pos( atof(arg1), atof(arg2), atof(arg3),
                                  atof(arg4), atof(arg5), atof(arg6) );
                   hexa.MoveBy(pos);
               } else {
                   cout << "MoveBy requires 6 parameters" << endl;
               }
               continue;
           }
           if(strncasecmp(cmd,"sphere",5)==0) {
               if (*arg1 != '\0' && *arg2 != '\0' && *arg3 != '\0') {
                   hexa.MoveOnSphere(atof(arg1), atof(arg2), atof(arg3));
               } else {
                   cout << "Sphere requires 3 parameters" << endl;
               }
               continue;
           }
           if(strncasecmp(cmd,"getpos",6)==0) {
               cout.precision(1);
               HexaTuple pos = hexa.GetPos();
               cout << fixed << pos << " distance/tilt " << pos.distance() << " "<< pos.tilt() << endl;
               continue;
           }
           if(strncasecmp(cmd,"isopen",6)==0) {
               bool open=hexa.isBrakeOpen();
               cout << "Brake: "<< ((open==true) ? "open" : "close") << endl;
               continue;
           }
           if(strncasecmp(cmd,"isready",7)==0) {
               bool ret=hexa.isInitialized();
               cout << "Ready: "<< ((ret==true) ? "yes" : "no") << endl;
               continue;
           }
           if(strncasecmp(cmd,"ismoving",7)==0) {
               bool ret=hexa.isMoving();
               cout << "Moving: "<< ((ret==true) ? "yes" : "no") << endl;
               continue;
           }
           if(strncasecmp(cmd,"status",3)==0) {
               HexaBool actshomed = hexa.ActsHomed();
               HexaBool actslimit = hexa.ActsLimit();
               HexaBool actsrunning = hexa.ActsRunning();
               HexaBool actsopenloop = hexa.ActsOpenLoop();
               HexaBool actsinposition = hexa.ActsInPosition();
               HexaBool actsfollowingerror = hexa.ActsFollowingError();
               HexaBool actsamplifierfault = hexa.ActsAmplifierFault();
               HexaBool limitspositivehw = hexa.LimitsPositiveHW();
               HexaBool limitsnegativehw = hexa.LimitsNegativeHW();
               cout << "Acts Homed:           "<< actshomed << endl;
               cout << "Acts Limits:          "<< actslimit << endl;
               cout << "Acts Running:         "<< actsrunning << endl;
               cout << "Acts Open Loop:       "<< actsopenloop << endl;
               cout << "Acts in position :    "<< actsinposition << endl;
               cout << "Acts following error: "<< actsfollowingerror << endl;
               cout << "Acts amplifier fault: "<< actsamplifierfault << endl;
               cout << "Limits positive HW:   "<< limitspositivehw << endl;
               cout << "Limits negative HW:   "<< limitsnegativehw << endl;
               cout << "Position  : "<< fixed << hexa.GetPos() << endl;
               cout << "Initialized  : "<< fixed << hexa.isInitialized() << endl;
               cout << "Moving  : "<< fixed << hexa.isMoving() << endl;
               continue;
           }
        } catch (HexapodException &e) {
           cout << e.what() << endl;
           continue; 
        }
        cmdhelp();
    }
    Logger::destroy();
    return 0;
}

