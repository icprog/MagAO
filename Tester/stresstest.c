//@File: stresstest.c
//
// {\tt stresstest} : Stresstest program for MsgD-RTDB 
//@

// Note: modified for thrdlib by L.Fini (Apr. 2008)

#define VERSMAJ  1
#define VERSMIN  3

#include <stdio.h>
#include <errno.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include <stdarg.h>
#include <sys/timeb.h>

#include "base/thrdlib.h"

#include "stresstest.h"

// **************************************
// Client name and versioning information
// **************************************

#define MAX_NAME_LEN   20

char *MyName = "STRESS";			// Client name (maximum 8 chars, less if you want multiple istances)
int multi=10;					// If >0, will try to connect to MsgD-RTDB changing names to allow <multi> istances

char *Author = "A. Puglisi";

char *types[3] = { "int", "real", "char" };

int minVars=10;      // Minumum number of variables to be created
int defVars=100;     // Default number of variables to be created
int maxVars=3000;    // Maximum number of variables to be created

// Global variables

int verbose=0;				// verbose flag

int num_vars=0;				// Number of local variables
struct Var *vars=NULL; 			// Local copy of the variables

int count_out=0;
int count_in=0;

int listen_handler( MsgBuf *msgb, void *args, int nqueue);

//@Procedure: help
//
//  prints an usage message
//@

static void help()
{
    printf("\n%s  - %s.  %s\n\n", MyName, GetVersionID(),Author);
    printf("Usage: stresstest [-v] [-d delay] [-s <Server Address>] [-p <XX>] [-n <nn>] [-c <nn>] <num>\n");
    printf("       stresstest -h    prints an help message\n\n");
    printf("   <num> required argument to allow multiple copies\n\n");
    printf("   -c    exit after count cycles (default: go on forever)\n");
    printf("   -d    Delay each request the specified number of seconds (fractions allowed)\n");
    printf("   -n    Create given number of variables (default:%d, Min:%d, Max:%d)\n",defVars,minVars,maxVars);
    printf("   -p    Use XX as variable name prefix (two characters)\n");
    printf("   -s    specify server address (default: localhost)\n");
    printf("   -v    verbose mode\n");
    printf("\n");
}




//@Main: 
//
// Example main function
//@

int main(int argc, char **argv) 
{
	int stat,i,count=1,decr=0;
        int id,totVars=defVars;
	char *server = NULL;
	char buffer[PROC_NAME_LEN+1];
        char *pref=NULL;
        struct timespec delay={0,0};

	// ---------- Process command line arguments

    SetVersion(VERSMAJ,VERSMIN);
	
    i=1;
    for(i=1;i<argc;i++) {
        if(*argv[i] != '-') break;

	if(strcmp(argv[i],"-h")==0) {
	    help();
	    exit(0);
	}

	if(strcmp(argv[i],"-v")==0) {
	    verbose=1;
            continue;
        }
    
	if (strcmp( argv[i], "-s") ==0)
		{
		if (++i >= argc)
			break;
		server = argv[i];
                continue;
		}

	if (strcmp( argv[i], "-c") ==0)
		{
		if (++i >= argc)
			break;
		count = atoi(argv[i]);
                decr=1;
                continue;
		}
	if (strcmp( argv[i], "-d") ==0)
                {
                double nsecs;
		if (++i >= argc)
			break;
		nsecs = atof(argv[i]);
                delay.tv_sec=(time_t)nsecs;
                delay.tv_nsec=(nsecs-delay.tv_sec)*1000000000;
                continue;
		}
	if (strcmp( argv[i], "-n") ==0)
		{
		if (++i >= argc)
			break;
		totVars = atoi(argv[i]);
                continue;
		}
	if (strcmp( argv[i], "-p") ==0)
		{
		if (++i >= argc)
			break;
		pref = argv[i];
                if(! isalnum(pref[1]) ) { pref=NULL; continue; }
                if(! isalpha(pref[0]) ) pref=NULL; 
                continue;
		}
        if(++i >= argc) {
            help();
            exit(0);
        }
        id=atoi(argv[i]);
    }

    if (!server)
		server ="127.0.0.1";    // If not specified, server IP defaults to 127.0.0.1

   totVars=(totVars<=maxVars)?totVars:maxVars;
   totVars=(totVars>=minVars)?totVars:minVars;
   vars=calloc(maxVars,sizeof(struct Var));

   if(!vars) {
      perror("Error allocating memory for var table\n");
      exit(0);
   }
   srand(time(NULL));

   sprintf(buffer, "%s%2.2d", MyName, id);

   MyName = buffer;

    if(IS_ERROR(stat=thInit(MyName))) {
        printf("Error %d (%s) from thInit() .. exiting", stat,lao_strerror(stat));
        exit(1);
    }


    printf("Installing reply handler ...\n");
    if((stat=thHandler(ANY_MSG,"*",0,listen_handler,"listen_handler",NULL))<0) {
        printf("Error %d (%s) from thHandler() .. exiting", stat,lao_strerror(stat));
        exit(1);
    }

	// ------------- Connect to MsgD-RTDB
    printf("%s connecting to %s\n",MyName,server);
	
    if(IS_ERROR(stat=thStart(server,1))) {
        printf("Error %d (%s) from thStart() .. exiting\n", stat,lao_strerror(stat));
        exit(1);
    }

    if (verbose) printf("Connected as %s\n", buffer);
	

     thSendMsg(0, "", SETREADY, 0, NULL);    // Notify I'm Ready
	
	// ------------ Now we are up and running. Do our stuff here
	//
	// Server-type operations (reacting to messages) should be done in the Listening thread
	//

	while(count>0)
	{
        int task;
        count-=decr;

        nanosleep(&delay,NULL);
	
	// Choose between 3 tasks
	task = rand() % 3;

//	printf("Task %d\n", task);

	switch(task)
		{
		case 0:			// Task 1: CREAVAR
                   if(totVars>0) {
		       count_out++;
		       CreateRandomVar(pref);
                       totVars--;
                    }
		break;

		case 1:			// Task 2: SETVAR
		if (num_vars >0)
			{
			count_out++;
			SetRandomVar();
			}
		break;

		case 2:			// Task 3: GETVAR
		if (num_vars >0)
			{
                        MsgBuf *ret; int n,errcod;

			count_out++;
			n = rand() % num_vars;
			if (verbose)	
				MyLog("Getting (%d) %s %d %d", n, vars[n].name, vars[n].type, vars[n].size);
			ret=thGetVar( vars[n].name, 200, &errcod);
                        if(ret) 
                            thRelease(ret);
                        else
			    MyLog("Error getting var %s [errcod=%d][", vars[n].name, errcod);
			}
		break;

		default:
		break;
		}

	fprintf(stderr, "N. ops: %d \r", count_out);
	}
        thCleanup();
	exit(0);
}


//@Function: listen_handler
//
// This is a thread listening to messages from MsgD-RTDB and acting on them
//@

int listen_handler( MsgBuf *msgb, void *args, int nqueue)
{
    if (HDR_CODE(msgb) == VARREPLY) {
        Variable *TheVar;
        char *name;
        int myvar;

        TheVar = thValue(msgb);
        name = TheVar->H.Name;
	    
        myvar = FindVar(name);		// Find or create a new variable
        printf("Received variable %s, position %d\n", name, myvar);	
        strcpy( vars[myvar].name, name);
        vars[myvar].type = TheVar->H.Type;
        vars[myvar].size = TheVar->H.NItems;
    } else
        printf("Unexpected Msg - From:%-10s To:%-10s Code:%-6x pLoad:%-6d Flg:%2.2x SeqN:%d Len:%d\n",
                                        HDR_FROM(msgb),HDR_TO(msgb),HDR_CODE(msgb),HDR_PLOAD(msgb),
                                        HDR_FLAGS(msgb),HDR_SEQN(msgb), HDR_LEN(msgb));
    thRelease(msgb);

    return NO_ERROR;
}





//@Procedure: CreateRandomVar   

// creates a random variable
//@
void CreateRandomVar(char *pref)
{
	int type, size, i, start, result;
	char name[MAX_NAME_LEN+4];

        if(num_vars>maxVars) {
            printf("Max number of variables exceeded!\n");
            return;
        }
        if(pref) {
            name[0]=pref[0];
            name[1]=pref[1];
            name[2]='_';
            start=3;
        } else
            start=0;
	for ( i=start; i<MAX_NAME_LEN; i++)
		name[i] = (rand() % 26) + 65;

	name[MAX_NAME_LEN] = 0;

	type = INT_VARIABLE + rand()%3;
	size = (rand()%100) +1;

	if (verbose)	
		MyLog("Creating %s (%s x %d)", name, types[type-INT_VARIABLE], size);
	result = thWriteVar(name, type, size, NULL, 200);
	if (IS_ERROR(result))
		MyLog("Error: (%d) %s", result, (char *)lao_strerror(result));
        else {
            strcpy(vars[num_vars].name,name);
            vars[num_vars].type=type;
            vars[num_vars].size=size;
            num_vars++;
        }

}

//@Procedure: SetRandomVar
//
// chooses a random variable and sets it to a random value
//@

void SetRandomVar()
{
	void *buffer;
	char name[MAX_NAME_LEN+1];
	int n, bytes, type, size,i;

	n = rand() % num_vars;

	strcpy( name, vars[n].name);
	type = vars[n].type;
	size = vars[n].size;

	bytes = VarSize( type, size);
	buffer = malloc(bytes);

	switch(type)
		{
		case INT_VARIABLE:
		for (i=0; i< size; i++)
			((int *)buffer)[i] = rand()%1000000;
		break;

		case REAL_VARIABLE:
		for (i=0; i< size; i++)
			((double *)buffer)[i] = (rand()%1000000) / 1000.0;
		break;


		case CHAR_VARIABLE:
		for (i=0; i< size; i++)
			((char *)buffer)[i] = (rand()%26) + 65;
		((char *)buffer)[size-1] = 0;
		break;
		}

	if (verbose)	
		MyLog("Setting %s (%s x %d)", name, types[type-INT_VARIABLE], size);
	thWriteVar(name, type, size, buffer, 200);
	free(buffer);
}



//@Function: FindVar
//
// find a variable in the local variable list
//
// This function searches the local variable list for the specified variable name,
// and returns its index. If the variable does not exists, a new one is created using
// the specified name, and its index is returned. Thus, the function is guaranteed to
// return the index of a valid variable.
//@

int FindVar( char *name)
{
	if ( num_vars>0)
		{
		int i;

		for (i=0; i< num_vars; i++)
			if (strcmp( vars[i].name, name) == 0)
				return i;
		}

	if (num_vars>0)
		{
		if (num_vars %10 == 0)
			{
			vars = (struct Var *)realloc( vars, sizeof(struct Var)*(num_vars+10));
			memset( vars + num_vars, 0, sizeof(struct Var)*10);
			}
		}
	else
		vars = (struct Var *)malloc( sizeof(struct Var)*10);

	num_vars++;
	return num_vars-1;
}


void MyLog( char *fmt, ...)
{
	struct timeb tp;
	va_list argp;

	ftime(&tp);
	
	fprintf( stdout, "%3d LOG> ", tp.millitm);
	va_start(argp, fmt);
	vfprintf( stdout, fmt, argp);
	va_end( argp);
	fprintf( stdout, "\n");
}

