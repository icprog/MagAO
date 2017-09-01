
#ifndef OPTLOOPDIAGNOSTICSTRUCTS_H_INCLUDE
#define OPTLOOPDIAGNOSTICSTRUCTS_H_INCLUDE

typedef struct {
   bool saveFrames;
   bool saveSlopes;
   bool saveModes;
   bool saveFFCommands;
   bool saveDistAverages;
   int nFrames;
   bool saveWfsStatus;
   bool saveAdSecStatus;
} optsave_cmd;

typedef struct {
   char outfile[256];
   int saved;
   float speed;
} optsave_reply;


#endif //  OPTLOOPDIAGNOSTICSTRUCTS_H_INCLUDE

