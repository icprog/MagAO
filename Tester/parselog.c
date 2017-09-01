//@File: getlog.c
//
// Join log files and prints to stdout a single log stream
//@


#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <malloc.h>
#include <string.h>
#include <signal.h>
#include <errno.h>
#include <math.h>

#include <time.h>
#include <sys/time.h>

#include <sys/types.h>
#include <dirent.h>

//
//@Procedure help:
//
//  Prints an usage message
//@

static void help()
{
    printf("Usage: getlog [-start timestamp] [-end timestamp] prefix\n");
    printf("       getlog -h\n\n");
    printf("       -h      print an usage message.\n\n");
    printf("       prefix  log file prefix\n");
    printf("       -start timestamp: start time\n");
    printf("       -end   timestamp: end time\n");
    printf("       -name  name: logger name\n");
}


double starttime,endtime;
int count=0;
int terminate=0;
int debug=0;
char *name = NULL;

char *fmt[] = {"%Y-%m-%d %H:%M:%S",
               "%Y-%m-%d"
              };

static double parseTimestamp(char *timestamp)
{
   struct tm dd;
   unsigned int i;

   //printf("Parse timestamp: %s\n", timestamp);

   //timestamp = "2009-03-11 15:29:45.617593"

   // Emulate getdate() but with zero initialization
   int found=0;
   for ( i=0; i<sizeof(fmt)/sizeof(fmt[0]); i++) {
      memset( &dd, sizeof(struct tm), 0);
      if (strptime(timestamp, fmt[i], &dd)) {
         found=1;
         break;
      }
   }

   if (!found) {
      printf("Timestamp not recognized: %sn", timestamp);
      return 0;
   }

   // Now the microsecond part
   char *pos = strchr(timestamp, '.');
   double usec=0;
   if (pos)
      usec = atoi(pos+1) / 1e6;

   return mktime(&dd)+usec;
}

static double parseFilename(char *filename)
{
   char *pos = strchr(filename, '.');
   if (pos) {
      if (strcmp(pos,".log")== 0)
         return 2e9;
      return atoi(pos+1);
   }
   else
      return 0;
}

static void trim(char *str)
{
   int pos;
   pos = strlen(str)-1;
   while ((pos>=0) && ((str[pos]==0x0D) || (str[pos]==0x0A)))
      str[pos--]=0;
}

static void processFile( char *filename, double startTime, double endTime)
{
   if (debug) printf("processing: %s\n", filename);
   FILE *fp = fopen(filename, "r");
   char *line = (char *)malloc(1024);
   unsigned int len;
   double t;

   char timestamp[26];
   char *pos;
   char loggername[64];
   memset(loggername, 0, 64);

   // Prepennd and append a space to the logger name to prevent matching substrings!! 
   if (name) {
      loggername[0] = ' ';
      strncat(loggername, name, 60);
      loggername[60]=0;
      strcat(loggername, " ");
   }


   while (getline( &line, &len, fp) >=0) {

      // Filter on time
      t = parseTimestamp(line+35);
      if ((t>=startTime) && (t<=endTime)) {

         // Filter on name
         if (name) {
            if ((pos = strstr(line, loggername)) == NULL)
               continue;
            if ((pos-line) < 63 || (pos-line)>79)
               continue;
         }

         memcpy(timestamp, line+35, 26);
         timestamp[25]=0;
         printf("%s %s", timestamp, line+82); 
      }
   }
/*
   if (t>endTime)
      terminate=1;
      */

}


//@Main: 
//
//@

int main(int argc, char **argv) 
{
    int i;
    double starttime,endtime;
    char *prefix=NULL;


    starttime=0;
    endtime=1e300;

    i=1;
    for(i=1;i<argc;i++) {     // Process command arguments

    if(strcmp(argv[i],"-start")==0) { starttime=parseTimestamp(argv[++i]); continue; }
    if(strcmp(argv[i],"-end")==0) { endtime=parseTimestamp(argv[++i]); continue; }
    if(strcmp(argv[i],"-name")==0) { name=argv[++i]; continue; }
    if(strcmp(argv[i],"-h")==0) { help(); exit(0); }
    prefix = argv[i];
    }

    if(!prefix) { help(); return 1; }


    /* Open log files with the right prefix and timestamp */


    /* Shell command to get all files in the directory which:
     * 1) are actually files and not directory
     * 2) have the specified prefix
     * 3) one per line
     * 4) in alphabetical order (== timestamp)
     */

    char fmt[] = "ls -1 -pd %s* | grep -v '/' | sort";
    char cmd[FILENAME_MAX];
    snprintf( cmd, FILENAME_MAX, fmt, prefix);

    char *line = malloc(FILENAME_MAX); // Allocated because getline() may realloc() it
    size_t len = FILENAME_MAX;

    printf("%s\n", cmd);

    FILE *fp = popen(cmd, "r");

    char lastfile[FILENAME_MAX];
    lastfile[0]='\0';

    if (debug) printf("Start time: %f\n", starttime);
    if (debug) printf("End time  : %f\n", endtime);

    while (getline( &line, &len, fp) >=0) {

       //fprintf(stderr, "File: %s\n", line);
       trim(line);

      // Skip files with the wrong timestamp (> endtime)
      double fileTimestamp = parseFilename(line);

      //if (fileTimestamp > endtime)
      //   continue;

      // For files with timestamp < starttime, we might want to read them if the interesting
      // timestamp happens in the middle. Therefore, when we get the first file with a valid
      // timestamp, we process the previous file too (if there was one)
      
      if (fileTimestamp >= starttime) {
         //fprintf(stderr, "%s\n", line);
         if (lastfile[0]) {
            processFile(lastfile, starttime, endtime);
            if (terminate)
               break;
            lastfile[0] = '\0';
         }
         processFile(line, starttime, endtime);
            if (terminate)
               break;
      } else {
         // We get here only file timestamp < starttime
         memcpy( lastfile, line, FILENAME_MAX);
      }
   }

   return 0;
}
