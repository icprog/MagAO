//@File: joelib.h
//
// Header file for the LittleJoe library
//@

#ifndef JOELIB_H_INCLUDED
#define JOELIB_H_INCLUDED

// Constant values for LittleJoe

#define JOE_RUNNING	1
#define JOE_NOT_RUNNING	0

int initLibrary(void);

int SendJoeCommand(const char *command, char *answer, int max_answer_length, int want_answer);
int SendJoeCommand_lockflag(const char *command, char *answer, int max_answer_length, int want_answer, int lockflag);

int InitJoeLink(const char *joe_addr, int joe_port);

int TestJoeLink(void);
int CheckJoe(void);
int GetJoeStatus(void);

int GetJoeVersion( char *version, int max_version_length);
int RecallSettings(void);

int StartJoe(void);
int StopJoe(void);
int GetJoeTemperature( float *t1, float *t2, float *t3);
int GetBlackLevel(int n);
int SetJoeProgram( int program);
int SetJoeBlack( int blacknum, int value);
int SetJoeRepetitions( int rep);
int SendXmodemFile(const char *upload_cmd,const char *filename);
int ReloadSettings();

int SetJoeClampLevel(int level);
int SetJoeSampleLevel(int level);
int GetJoeClampLevel();
int GetJoeSampleLevel();

#endif // JOELIB_H_INCLUDED
