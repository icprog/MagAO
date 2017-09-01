//@File: rtdb.h
//
//Definitions and prototypes for the module rtdb.c
//

#ifndef RTDB_H
#define RTDB_H

#include "types.h"

#define HASH_SIZE  50000           // Variable hash table length
                                   // A number conveniently greater than
                                   // the number of system variables in RT
                                   // database
#define SORTED_INITIAL_SIZE 5000   // Variable sorted table initial lenght
                                   // Sorted table can extend dynamically


#define CLISTCHUNK  50    // Number of elements to add to notify list
                          // when extending

// Prototypes


int InitTables(void);

int CreaVariable(Variable *, Client* owner);
int GetVariable(char *name, Client* cl, MsgBuf *b);
int SetVariable (Variable *varb, Client* owner, CList **List);
int NotifyMe(Client *, char *);
int NoNotifyMe(Client *, char *, int);
int InitVarScan(char *, Listscan *listX, int);
DB_Var *NextVarScan(Listscan *);
int EndVarScan(Listscan *listX);
int DeleteVariable(char *, Client*);
int UnlockVariableSet(char *vname, Client *);
int UnlockVariable(DB_Var *vv, Client *);
int LockVariableSet(char *vname, Client *);
int UnlockTable(void);
int LockTable(void);
int WriteVariable (Variable *varb, Client* owner, CList **List);
int VarTrace(Client *, char *vname, int op);
int nVars(void);
void makeName(char *dst,char *name,char *dom);

#endif
