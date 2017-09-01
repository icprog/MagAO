//+File: terminal.c
//


#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <signal.h>

#include <readline/readline.h>
#include <readline/history.h>

#include "base/errlib.h"
#include "base/msglib.h"
#include "base/rtdblib.h"
#include "idllib/idllib.h"

char MyName[] = "IDLTERM00";
char CtrlName[] = "idlctrl.L";    // Side will be substituted at runtime
int counter=0;

int VersMajor = 1;
int VersMinor = 0;
char *Date = "April 2005";

// Input buffer len
#define BUFLEN (1024)
char buffer[BUFLEN];

// controller pid
int ctrl_pid = 0;
char PidVarname[VAR_NAME_LEN+1];

//
//+Entry help
//
//  help prints an usage message
//

static void help()
{
    printf("\n%s  - Vers. %d.%d.   A.Puglisi, %s\n\n",
	    MyName, VersMajor,VersMinor,Date);
    printf("Usage: StagesCmd [-v] [-s <Server Address>]\n");
    printf("   -s    specify server address (defaults to localhost)\n");
    printf("   -v    verbose mode\n");
    printf("\n");
}

// Ctrl-C handler

static void ctrlc( int signal)
{
   if (ctrl_pid>0)
      {
      int stat = kill( ctrl_pid, SIGUSR1);
      if (stat == 0)
         printf("Ctrl-C - forwarded to controller (pid = %d)\n", ctrl_pid);
      else
         printf("Ctrl-C forwarding FAILED (pid = %d)\n", ctrl_pid);
      }
}

// +Main: 
//
// StagesCmd   Command-line interface to StagesCtrl program
//

int main(int argc, char **argv) 
{
    int stat,i,idl_reply_received;
    int verbose=0;
    unsigned int seqnum=0;
    MsgBuf msgb;
    char *server = NULL;
    char *line;
    idl_answer answer;

    signal(SIGINT, ctrlc);

    // Correct for ADOPT_SIDE
    char *adoptside = getenv("ADOPT_SIDE");
    if (adoptside) {
       CtrlName[ strlen(CtrlName)-1] = adoptside[0];
    }
    strcpy(PidVarname, CtrlName);
    strcat(PidVarname, ".pid");

     
    i=1;
    for(i=1;i<argc;i++) {     // Process command arguments
        if(*argv[i] != '-') break;

	if(strcmp(argv[i],"-h")==0) {
	    help();
	    exit(0);
	}

	if(strcmp(argv[i],"-v")==0)
	    verbose=1;
    
	if (strcmp( argv[i], "-s") ==0)
		{
		if (++i >= argc)
			break;
		server = argv[i];
		}
	}

    if (!server)
		server ="127.0.0.1";    // If not specified, server IP defaults to 127.0.0.1

    if (verbose)
		printf("%s connecting to %s\n",MyName,server);


    // ------- Connect to MsgD-RTDB. Loops using different names to allow multiple terminals
    while(1)
	{
	if ((stat = ConnectToServer(server, MyName, 10)) == NO_ERROR)
		break;

	if ((counter < 100-1) && (stat == DUPLICATE_CLIENT_ERROR))
		{
		counter++;
 		sprintf(MyName, "IDLTERM%02d", counter);
		continue;
		}

	printf("Error connecting to server at: %s\n",server);
	printf("Error code: %d - %s",stat, lao_strerror(stat));
	if(SYSTEM_ERROR(stat))
	    printf(" - %s",strerror(errno));
	printf("\n\n");
	return 1;
	}

   // ---------- Read idl_ctrl pid (for signal forwarding)

   stat = ReadVar( MyName, PidVarname, &ctrl_pid, 1, 1000, NULL, NULL);
   if (stat == 1)
      {
      stat = VarNotif( MyName, PidVarname);
      if (stat != NO_ERROR)
          printf("Error asking notifications for %s: (%d) %s\n", PidVarname, stat, lao_strerror(stat));
      }
   else
      printf("Error reading %s: (%d) %s\n", PidVarname, stat, lao_strerror(stat));

	// ---------- Start input loop

	InitMsgBuf(&msgb,0);
	using_history();
	PrepareIDLAnswer(&answer);

	while( (line = readline("AdOpt>")))
		{
      // Check whether a VARCHANGD message arrived
      stat = WaitMsgTimeout(&msgb, 0);
      if (stat != TIMEOUT_ERROR)
         if (hdr_CODE(msgb) == VARCHANGD)
            {
            Variable *TheVar = (Variable *)msg_BODY(msgb);
            if (strcmp( TheVar->H.Name, PidVarname) == 0)
               {
               ctrl_pid = TheVar->Value.Lv[0];
               printf("New idl_ctrl process id: %d\n", ctrl_pid);
               }
            }

		if (strlen(line) == 0)
			continue;

        if (strcmp(line, "exit")==0)
            break;

		FillMessage(strlen(line), ++seqnum, MyName, CtrlName, IDLCMD, line, &msgb);
		stat = SendToServer(&msgb);
		if (stat != NO_ERROR)
			fprintf(stderr, "Error: (%d) %s\n", stat, lao_strerror(stat));

		add_history(line);
		free(line);

		// Waits for the answer
		do
			{
			idl_reply_received=0;
			if ((stat = WaitMsgFromServer(&msgb)) != NO_ERROR)
				{
				fprintf( stderr, "Error: (%d) %s\n", stat, lao_strerror(stat));
				FreeMsgBuf(&msgb);
				exit(0);
				}

		    	// Output and errors from IDL, packed into the same messagge
			if ((hdr_SEQN(msgb)==seqnum) && (hdr_CODE(msgb)==IDLREPLY))
				if (GetIDLAnswer(&msgb, &answer) == NO_ERROR)
					{
					printf("%s", answer.error);
					printf("%s", answer.output);
					idl_reply_received = 1;
					}	
			}
		while (!idl_reply_received);

		}

	FreeIDLAnswer(&answer);
	FreeMsgBuf(&msgb);

	exit(0);
}

