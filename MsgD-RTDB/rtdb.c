//@File: rtdb.c
//
// Real time data base management support for the {\tt MsgD-RTDB} process.
//@

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/time.h>

#include <sys/types.h>
#include <regex.h>

#include <pthread.h>

#include "base/msglib.h"
#include "base/errlib.h"
#include "base/rtdblib.h"

#include "shmlib.h"
#include "client.h"
#include "table.h"
#include "rtdb.h"
#include "logerr.h"

extern Client *MySelf;

extern SortedTable notifyTB;
// extern SortedTable mirrorTB;

extern VarTable variableTB;

#ifdef MEM_DEBUG
#include <stdio.h>

static void *MALLOC(size_t size)
{
void *ptr=malloc(size);
fprintf(stderr,"MALLOC(%ld) --> %lx\n",(long int)size,(long unsigned int)ptr);
return ptr;
}

static void *REALLOC(void *ppt,size_t size)
{
void *ptr=realloc(ppt,size);
fprintf(stderr,"REALLOC(%lx,%ld) --> %lx\n",(long unsigned int)ppt,(long int)size,(long unsigned int)ptr);
return ptr;
}

static void FREE(void * ptr)
{
fprintf(stderr,"FREE(%lx)\n",(long unsigned int)ptr);
free(ptr);
}

#else

#define MALLOC malloc
#define REALLOC realloc
#define FREE  free

#endif

//@Function: addToNotifList
//
// Adds a client to notify list of a variable
//
// This routine adds a client to the notify list managing list creation
// and extension, if necessary.
//
// The notify list is an array of pointers to client who want to be
// notified the change of a variable. The array is dinamically allocated
// and extended when needed. In order to optimize the management of the
// notify list each operation (insertion and removal of a client) is
// made so that all active clients are stored contiguosly  from the 
// beginning of the array, whith no empty element in between. It is also
// guaranteed that the array is terminated with at least one NULL element.
//@

static int addToNotifList(DB_Var *Var,    // Variable structure
                          Client *client, // Client to add
                          char *  fname)  // Full client name
                                          // Returns status code
{
   int i,stat=NO_ERROR;

   MUTEX_LOCK(&(Var->VarMutex),"addToNotifyList");        // Lock var Mutex    ***
   i=0;
   stat=addclist(Var->NList,client,fname);
       if(stat==DUPL_CLIST_ERROR) stat=NO_ERROR;    // Ignore duplicate name error

   MUTEX_UNLOCK(&(Var->VarMutex),"addToNotifyList");      // Lock var Mutex    ***
   return stat;
}

//@Function: delFromNotifList
//
// Removes a client from notify list
//
// This routine removes a client from the notify list of a variable
//@

static int delFromNotifList(DB_Var *vpt,     // Variable structure
                            char *fname)     // Full name of client
                                             // Returns status code
{
    int stat;

    MUTEX_LOCK(&(vpt->VarMutex),"delFromNotifList");
    stat=remclist(vpt->NList,fname);
    MUTEX_UNLOCK(&(vpt->VarMutex),"delFromNotifList");

    return stat;
}

//@Function: addToNotifyTable
//
// Adds an item to the notify table
//
// This routine adds a client to the notify table
//
// The notify table is a sorted array of variable name prefixes with
// associated notify lists (@see{addclist} for a description of notify lists).
//
// This table is searched any time a new variable is defined in order to activate
// notifications (if any).
//@

static int addToNotifyTable(char *prefix, Client *client, char *fname)
{
   int stat,pos;

   s_item item;

   item.key = strdup(prefix);
   item.value=NULL;

   MUTEX_LOCK(&notifyTB.mutex,"addToNotifytable");      //**************************
   stat=sput(&(notifyTB.stable),&item,&pos);
   if(IS_NOT_ERROR(stat)) {
      CList *clst;
      clst=initclist();                                   // Create new client list
      if(!clst) {
         stat=MEMORY_ALLOC_ERROR;
      } else {
         notifyTB.stable.body[pos].value=(void *)clst;
         stat=addclist(clst,client,fname);
      }
   } else {
      FREE(item.key);
      if(stat==DUPL_NAME_ERROR) {                      // The list is already there
         stat=addclist(notifyTB.stable.body[pos].value,client,fname);
      }
   }
   MUTEX_UNLOCK(&notifyTB.mutex,"addToNotifytable");            //*************************
   return stat;
}

//@Function: delFromNotifyTable
//
// Removes a client from the notify table of a specified variable prefix
//
// This routine removes a client from the item of the notify table
// matching the given string.
//
// If the string is "*", the client is removed from any table item
//@

static int delFromNotifyTable(char *prefix, char *fname)
{
int pos,stat=NO_ERROR;

MUTEX_LOCK(&notifyTB.mutex,"delFromNotifytable");  // ***************************
if(*prefix=='*') {
   for(pos=0;pos<notifyTB.stable.nItms;) {
      CList *clst=notifyTB.stable.body[pos].value;
      stat=remclist(clst,fname);
      if(clistempty(clst)) {
            stat=sremove(&(notifyTB.stable),pos);
            freeclist(clst);
      } else
            pos++;  // Increment index only if element has not been removed
   }
} else {
   if(ssearch(&(notifyTB.stable),prefix,&pos)) {
        CList *clst=notifyTB.stable.body[pos].value;
        stat=remclist(clst,fname);
        if(clistempty(clst)) {
           stat=sremove(&(notifyTB.stable),pos);
           freeclist(clst);
        }
   }
}
MUTEX_UNLOCK(&notifyTB.mutex,"delFromNotifytable"); // **********************
return stat;
}

//@Function: LockVariableSet
//
// Lock a variable or group of variables
//
// This function sets a lock on one or more variables.

// If the variable name argument ends with '*' it is considered a prefix,
// and all variables matching the prefix are locked.

// To unlock call @see{UnlockVariableSet()}

// NOTE: Variable specification for lock/unlock must be used with care.
//       If prefix for unlocking is different from the one used for locking,
//       not all locked variables could be possibly unlocked.
//
//@

int LockVariableSet(char*    prefix,  //@P{prefix}: Variable name or prefix
                    Client*  client) //@P{client}: Calling client
{
    Listscan listX;
    int stat=NO_ERROR;
    int cmatch=0;

    InitVarScan(prefix,&listX,LOCKMUTEX); // Initialize Variable list
                                        // retrieval (whith mutex lock)
   
    for(;;) {                             // Scan variable list
	DB_Var *vpt=NextVarScan(&listX);

	if(vpt==NULL) {                   // List is finished
            EndVarScan(&listX);
	    break;
	}
        cmatch++;
        MUTEX_LOCK(&(vpt->VarMutex),"LockVariableSet");
        vpt->Lock=client->Id;
        LAO_Debug(client,"has locked var \"%s\"", vpt->Name);
    }
    if(cmatch==0) {
        stat = VAR_NOT_FOUND_ERROR;
        LAO_Error(client,stat,"in LockVariableSet: no var matching \"%s\"",prefix);
    }

    return stat;
}

//@Function: UnlockVariable
//
// Unlock a single variable
//
// This function removes a lock on a single variable.

// The function is used only when a client unregister
// to reset the status of possibly locked variables
//@

int UnlockVariable(DB_Var  *vpt,
                   Client  *client)
{
    if(vpt->Lock==client->Id) {
        MUTEX_UNLOCK(&(vpt->VarMutex),"UnLockVariableSet");
        vpt->Lock=PUBLIC_VAR;
        return NO_ERROR;
    } else 
        return VAR_LOCK_ERROR;
}

//@Function: UnlockVariableSet
//
// Unlock a variable or group of variables
//
// This function removes a lock on one or more variables.

// If the variable name argument ends with '*' it is considered a prefix,
// and all variables matching the prefix are locked.

// To lock variables call @see{LockVariableSet()}

// NOTE: Variable specification for lock/unlock must be used with care.
//       If prefix for unlocking is different from the one used for locking,
//       not all locked variables could be possibly unlocked.
//@

int UnlockVariableSet(char*   prefix,  //@P{prefix}: Variable name or prefix
                      Client* client) //@P{client}: Calling client's ID
{
    Listscan listX;
    int stat=NO_ERROR;
    int cmatch=0;

    InitVarScan(prefix,&listX,LOCKMUTEX);  // Initialize Variable list
                                         // retrieval (whith mutex lock)
   
    for(;;) {                            // Scan variable list
        int lstat;
	DB_Var *vpt=NextVarScan(&listX);

	if(vpt==NULL) {                   // List is finished
            EndVarScan(&listX);
	    break;
	}
        cmatch++;
        lstat=UnlockVariable(vpt,client);
        if(IS_ERROR(lstat)) {
            LAO_Error(client,lstat,"in UnlockVariableSet: unlocking var \"%s\"",vpt->Name);
            stat=lstat;
        } else
            LAO_Debug(client,"has unlocked var \"%s\"", vpt->Name);
    }
    if(cmatch==0) {
        stat = VAR_NOT_FOUND_ERROR;
        LAO_Error(client,stat,"in UnlockVariableSet: no var matching \"%s\"",prefix);
    }
    return stat;
}


//@Function: DeleteVariable 
//
// Removes a variable from the RTDB
//
// This function removes a variable from the RealTime Database.
// After successful removal the memory associated with the variable is
// freed, but the memory associated with the notify list is mantained
// and a pointer to it is passed back to the caller for proper
// notification.

// After notification the caller must free the NList in order to
// collect the memory allocated to the notify list.
//@

int DeleteVariable(char     *vname,  //@P{vname}: Variable name
                   Client*  client)  //@P{client}: Calling client's ID
                                     //@R: Error status
{
    DB_Var *vpt,*var;
    int stat;
    char* errfmt= "deleting var \"%s\"";

    MUTEX_LOCK(&variableTB.mutex,"DeleteVariable"); // Lock Variable table   
    stat = hsearch(&(variableTB.htable),vname);     // Search variable
    if(stat<0) {
	stat=VAR_NOT_FOUND_ERROR;
    } else {
        var=variableTB.htable.body[stat];
        sremstr(&(variableTB.stable),vname);          // Remove variable from sorted table
        vpt = hremove(&(variableTB.htable),vname);    // Remove variable from hash table
        if(vpt != var)
           stat=SYSTEM_ERROR(TABLE_INCONSIST_ERROR);   // The variable removed is different
                                                        // from the one returned by search
        else {
           MUTEX_DESTROY(&(vpt->VarMutex),"VariableMutex");
           freeclist(vpt->NList);
           FREE(vpt->NList);
           FREE(vpt->Var);
           FREE(vpt);
           stat = NO_ERROR;
        }
    }
    MUTEX_UNLOCK(&variableTB.mutex,"DeleteVariable"); // unlock Variable table   
    if(IS_ERROR(stat)) 
        LAO_Error(client,stat,errfmt,vname);
    else
        LAO_Info(client,"deleted var \"%s\"", vname);

    return stat;
}




//@Function: InitTables
//
// Initializes the variable table
//
// This routine creates the variable table and initializes the hash 
// functions.
//@


int InitTables (void)  // Hint about the foreseen dimension of
                                  // the table
{
int ret;
extern pthread_mutexattr_t mutexAttributes;

if(IS_ERROR(ret=hcreate(&(variableTB.htable),HASH_SIZE))) return ret;
if(IS_ERROR(ret=screate(&(variableTB.stable),SORTED_INITIAL_SIZE,0))) return ret;
if(IS_ERROR(ret=screate(&(notifyTB.stable),SORTED_INITIAL_SIZE,0))) return ret;
// if(IS_ERROR(ret=screate(&(mirrorTB.stable),SORTED_INITIAL_SIZE,0))) return ret;
MUTEX_INIT(&(variableTB.mutex),"VariableTableMutex");
MUTEX_INIT(&(notifyTB.mutex ),"NotifyTableMutex");
// MUTEX_INIT(&(mirrorTB.mutex ),"MirrorTableMutex");
return NO_ERROR;
}

void makeName(char *dst,char *name,char *dom)
{
   if(dom && dom[0]!='\0') 
      snprintf(dst,VAR_NAME_LEN+1,"%s@%s",name,dom);
   else
      snprintf(dst,VAR_NAME_LEN+1,"%s",name);
}


//@Function: CheckNotifyRequests
//
// Check notify requests
//
// This routine is called upon creation of a new variable to check
// wether there are notify requests waiting.
//
//@

static int CheckNotifyRequests(DB_Var *vpt)
{
int i;

MUTEX_LOCK(&notifyTB.mutex,"CheckNotifyRequests");     // *********************
for(i=0; i<notifyTB.stable.nItms; i++) {
   char *pref=notifyTB.stable.body[i].key;
   char *vnam=vpt->Name;
   int found=1;
   for(;*pref;pref++,vnam++) {
      if(*pref=='*') break;
      if(*pref!=*vnam) { found=0; break; }
   }
   if(found) {
        int k,stat;
        CList *clst=(CList *)notifyTB.stable.body[i].value;

        for(k=0;k<clst->nItms;k++) {
            Client *cl=clst->body[k].clPtr;
            char *fname=clst->body[k].fname;
            stat=addToNotifList(vpt,cl,fname);
            if(IS_ERROR(stat))
                LAO_Error(cl,stat,"In CheckNotifyRequest(%s)",vpt->Name);
            else
                LAO_Info(cl, "added to notify list of var \"%s\"", vpt->Name);
        }
   }
}
MUTEX_UNLOCK(&notifyTB.mutex,"CheckNotifyRequests");     // *********************
return NO_ERROR;
}

//@Function: CheckMirrorRequest
//
// Check mirror requests
//
// This routine is called upon creation of a new variable to check
// wether the variable must be mirrored
//
//@

/*
static int CheckMirrorRequests(DB_Var *vpt)
{
int i;

MUTEX_LOCK(&mirrorTB.mutex,"CheckMirrorRequests");                              // ***
for(i=0; i<mirrorTB.stable.nItms; i++) {                                        // ***
   char *pref=mirrorTB.stable.body[i].key;                                      // ***
   char *vnam=vpt->Name;                                                        // ***
   int found=1;                                                                 // ***
   for(;*pref;pref++,vnam++) {                                                  // ***
      if(*pref=='*') break;                                                     // ***
      if(*pref!=*vnam) { found=0; break; }                                      // ***
   }                                                                            // ***
   if(found) {                                                                  // ***
      vpt->Mirrored=1;                                                            // ***
   }                                                                            // ***
}                                                                               // ***
MUTEX_UNLOCK(&mirrorTB.mutex,"CheckMirrorRequests");                            // ***
return NO_ERROR;
}
*/



//@Function: CreaVarInternal 
//
// Creates a new variable in the RT-Database
//
// This routine creates a new variable in the Real-Time database. 
// It is intended for internal use and is called either by CreaVariable()
// or by WriteVariable()
//
// The Variable table mutex must be locked before the call
//@

static
DB_Var *CreaVarInternal(Variable* varb,  // Variable buffer.
                        char*     name,  // Name may be different from
                                         // name stored in buffer
                        int *errc)       // return error code
{
    DB_Var *vpt;
    localVar *vv;
    int  stat,varlen,totlen;
    extern pthread_mutexattr_t mutexAttributes;
	 
#ifdef LONG_IS_64
	 struct timeval tv;
#endif
	 
    varlen=VarSize(varb->H.Type,varb->H.NItems);
    if(varlen<0) { 
        *errc=VAR_SIZE_ERROR;
        return NULL;
    }
    totlen=varlen+sizeof(VHeader);
    
    stat = NO_ERROR;

    vpt = (DB_Var *)MALLOC(sizeof(DB_Var));      // Allocate memory
    if(vpt==NULL) {
        *errc = MEMORY_ALLOC_ERROR;
        return NULL;
    }
    vpt->Var=(localVar *)MALLOC(totlen);
    if(vpt->Var==NULL) {
        FREE(vpt);
        *errc = MEMORY_ALLOC_ERROR;
        return NULL;
    }

    strncpy(vpt->Name,name,VAR_NAME_LEN); 
    vpt->Name[VAR_NAME_LEN]='\0';
	                 
    vpt->len=totlen;
    vpt->NList=initclist();
    vpt->Mirrored=0;
    vv=vpt->Var;
    vv->H.Type=varb->H.Type;
    vv->H.NItems=varb->H.NItems;
    vpt->OwnerID=0;            // Obsolete
    vpt->Lock=PUBLIC_VAR;      // Variable initially unlocked
    vpt->trace=0;              // Intially no trace active
    MUTEX_INIT(&(vpt->VarMutex),"VariableMutex");
    stat=hput(&(variableTB.htable),vpt);         // Put var. in hash table
    if(IS_NOT_ERROR(stat)) {                     // and in sorted table
        int pos;
        s_item item;
        item.key = vpt->Name;
        item.value = (void *)vpt;
        stat=sput(&(variableTB.stable),&item,&pos);
    }

    if(IS_NOT_ERROR(stat)) {                      //
        memset(&(vv->Value),0,varlen);         // clear the variable
		  #ifdef LONG_IS_64
		   gettimeofday(&tv,NULL);
	      vv->H.MTime = tv32_fm_tv(tv);
        #else
		   gettimeofday(&(vv->H.MTime),NULL);     // Set var. time    
        #endif
        
    }  else {
        DB_Var *vrm=hremove(&(variableTB.htable),vpt->Name);
        FREE(vrm);          // deallocate variable memory on error
        vpt=NULL;
    }
    return vpt;
}
	    

//@Function: CreaVariable 
//
// Creates a new variable in the RT-Database
//
// This routine creates a new variable in the Real-Time database. 
//
// RT-DB variables have two access modes: global and proprietary. Global
// variables can be read and written by anybody, proprietary variables
// can be read by anybody but can only be written by the client who 
// created it.
//
// Attempting to create an existing variable is not considered an error
// the second attempt is simply ignored and logged as a warning
//
// Implementation note: each variable is stored both into an hash and into a sorted
// list, so that both sequential access and direct access can be optimized
//@

int CreaVariable(Variable* varb,  // Variable buffer.
	              Client*  cl) 
{
   DB_Var *vpt;
   int  stat;
   char* errfmt= "creating var \"%s\"";
   extern pthread_mutexattr_t mutexAttributes;
   char vnbuf[VAR_NAME_LEN+1];

   rmVDomain(varb->H.Name,vnbuf);

   stat = NO_ERROR;

   MUTEX_LOCK(&variableTB.mutex,"CreaVariable"); // Lock Variable table
                                                            
   if((stat=hsearch(&(variableTB.htable),vnbuf))>=0) {
                                                 // The variable exists 
      MUTEX_UNLOCK(&variableTB.mutex,"CreaVariable"); 
      LAO_Debug(cl,"Attempt to create existing var \"%s\"",vnbuf);
      return NO_ERROR;
   }
  
   vpt = CreaVarInternal(varb,vnbuf,&stat);
   MUTEX_UNLOCK(&variableTB.mutex,"CreaVariable"); 

   if(vpt) {
      LAO_Info(cl,"created var \"%s\"",vnbuf);
//    CheckMirrorRequests(vpt);
      CheckNotifyRequests(vpt);
   } else
      LAO_Error(cl,stat,errfmt,vnbuf);
   return stat;
}
	    

//@Function: GetVariable 
//
// Returns a variable stored in the real time database
//
// This routine returns a variable found in the real time database.
// After the call the MsgBuf length field is properly adjusted
//@


int GetVariable(char*   name,      // Variable name. 
                Client* client,    // Client
                MsgBuf* Msgb)      // Msgbuffer to store variable
                                   // Returns: status code
{
   DB_Var *vpt;
   int found;
   int stat=NO_ERROR;
   int totlen,varlen;
   char* errfmt="getting var \"%s\"";
   char vnbuf[VAR_NAME_LEN+1];
   char *dom=rmVDomain(name,vnbuf);

   MUTEX_LOCK(&variableTB.mutex,"GetVariable");
   found = hsearch(&(variableTB.htable),vnbuf);

   if(found>=0) {
      localVar *src;
      Variable *dst;

      vpt=variableTB.htable.body[found];
      MUTEX_LOCK(&(vpt->VarMutex),"GetVariable");
      src = vpt->Var;
      if(vpt->Lock>=0) {                        // Check global lock
         if(vpt->Lock != client->Id) {
            MUTEX_UNLOCK(&(vpt->VarMutex),"GetVariable");
            MUTEX_UNLOCK(&variableTB.mutex,"GetVariable");
            stat = VAR_LOCKED_ERROR;
            LAO_Error(client,stat,errfmt,vpt->Name);
            return stat;
         }
      }
      varlen = VarSize(src->H.Type,src->H.NItems);
      totlen = varlen+sizeof(VHeader);

      stat=ResizeMsgBuf(Msgb,totlen);
      HDR_LEN(Msgb)=totlen;
      if(IS_ERROR(stat)) {
         MUTEX_UNLOCK(&(vpt->VarMutex),"GetVariable");
         MUTEX_UNLOCK(&variableTB.mutex,"GetVariable");
         return stat;
      }

      dst=(Variable *)MSG_BODY(Msgb);
      dst->H.Type=src->H.Type;
      dst->H.NItems=src->H.NItems;
      makeName(dst->H.Name,vpt->Name,dom);
      dst->H.Owner[0]='\0';
      memcpy(&(dst->H.MTime),&(src->H.MTime),sizeof(dst->H.MTime));
      memcpy(&(dst->Value),&(src->Value),varlen);   // Write variable value
      MUTEX_UNLOCK(&(vpt->VarMutex),"GetVariable");
      MUTEX_UNLOCK(&variableTB.mutex,"GetVariable");
      LAO_Trace(client,"Read var \"%s\"",dst->H.Name);
   } else {
       MUTEX_UNLOCK(&variableTB.mutex,"GetVariable");
       stat=VAR_NOT_FOUND_ERROR;
       LAO_Error(client,stat,errfmt,name);
   }
   return stat;
}

//@Function: SetVarInternal
//
// Called by SetVariable and WriteVariable to set the value of a variable
//
// The variable table may be locked when calling this
//@

static int SetVarInternal(Variable     *varb,   // Variable specification
                          int          vidx,    // Index of database variable
                          Client*      client,  // Client
                          CList**      NList)   // Returned notify list
                                                // If null no notify list is returned
{
   int varlen,stat=NO_ERROR;
   size_t totlen;
   DB_Var *vpt=variableTB.htable.body[vidx];
   localVar *dst;

   MUTEX_LOCK(&(vpt->VarMutex),"SetVarInternal"); 

   dst=vpt->Var;
   if((vpt->Lock<0) || (vpt->Lock==client->Id)) {     // Var not locked by somebody else
      varlen=VarSize(varb->H.Type,varb->H.NItems);
      totlen=varlen+sizeof(VHeader);
      if(totlen>vpt->len) {                          // Variable extension needed
         int pos;
         vpt->Var=(localVar *)REALLOC(vpt->Var, totlen);
         if(vpt==NULL) {
            MUTEX_UNLOCK(&(vpt->VarMutex),"SetVarInternal");
            return MEMORY_ALLOC_ERROR;
         } 
         dst=vpt->Var;
         vpt->len=totlen;
         variableTB.htable.body[vidx]=vpt;        // Update pointers to variable into tables
         if(ssearch(&(variableTB.stable),vpt->Name,&pos)) 
             variableTB.stable.body[pos].value=(void *)vpt;
      } 
      gettimeofday(&(dst->H.MTime),NULL);
      dst->H.Type=varb->H.Type;
      dst->H.NItems=varb->H.NItems;
      memcpy(&(dst->Value),&(varb->Value),varlen);   // Write variable value
      stat=vpt->Mirrored;       // If variable is global, return 1 (a NO_ERROR value)

// Here we write back into the client's variable buffer the modification time
// This is useful for variable change notification: returned variable value
// has the correct modification time
       memcpy(&(varb->H.MTime),&(dst->H.MTime),sizeof(dst->H.MTime));
       if(NList) *NList = vpt->NList;
   } else {                              // We get here if the variable is locked
       stat=VAR_LOCKED_ERROR;
   }

   if(IS_NOT_ERROR(stat) && vpt->trace)  
      LAO_DumpVar(NULL,vpt,1);

   MUTEX_UNLOCK(&(vpt->VarMutex),"SetVarInternal");

   return stat;
}


//@Function: SetVariable 
//
// Set the value of a variable in the RT-Database
//
// This routine gives a new new value to a variable in the Real-Time database. 
//
//@

int SetVariable(Variable  *varb,   // Variable Buffer
                Client*   client,  // Client
                CList   **NList)   // Returned notify list
{
   int stat;

   MUTEX_LOCK(&variableTB.mutex,"SetVariable");
   stat = hsearch(&(variableTB.htable),varb->H.Name);

   if(stat<0) {
       stat = VAR_NOT_FOUND_ERROR;
   } else {
       stat = SetVarInternal(varb,stat,client,NList);
   }
   MUTEX_UNLOCK(&variableTB.mutex,"SetVariable");

   if(IS_ERROR(stat)) 
       LAO_Error(client,stat,"writing var \"%s\"",varb->H.Name);
   else
       LAO_Debug(client,"Written var \"%s\"",varb->H.Name);
   return stat;
}


//@Function: WriteVariable 
//
// Write the value of a variable in the RT-Database, possibly creating it 
//
// This routine gives a new new value to a variable in the Real-Time database. 
//
//@

int WriteVariable(Variable*  varb,   //@P{varb}: Variable Buffer
                  Client*    client, //@P{client}: Client
                  CList**    NList)  //@P{NList}: Return pointer to notify list
                                     //@R: completion code
{
   int stat;
   DB_Var *vpt;
   char vnbuf[VAR_NAME_LEN+1];

   rmVDomain(varb->H.Name,vnbuf);

   MUTEX_LOCK(&variableTB.mutex,"WriteVariable");
   stat = hsearch(&(variableTB.htable),vnbuf);

   if(stat<0) {                               // Variable not found
       vpt=CreaVarInternal(varb,vnbuf,&stat); // ... create it
       if(!vpt) {
           MUTEX_UNLOCK(&variableTB.mutex,"WriteVariable");
           LAO_Error(client,stat,"creating var \"%s\"",vnbuf);
           return stat;
       }
       LAO_Info(client,"created var \"%s\"",vnbuf);
//     CheckMirrorRequests(vpt);
       CheckNotifyRequests(vpt);
       stat = hsearch(&(variableTB.htable),vnbuf);
   }

   stat = SetVarInternal(varb,stat,client,NList);
   MUTEX_UNLOCK(&variableTB.mutex,"WriteVariable");

   if(IS_ERROR(stat)) 
       LAO_Error(client,stat,"writing var \"%s\"",vnbuf);
   else
       LAO_Debug(client,"Written var \"%s\"",vnbuf);

   return stat;
}

//@Function: NotifyMe
//
// Requests notification of variable change
//
// This routine is called when the client software receives a VARNOTIF
// message to add the client to the notify list for the given variable. 
//@

int NotifyMe(Client *client,    // Connected client
               char *prefix )   // Variable prefix
{
   Listscan listX;
   int stat;
   char fname[LONG_NAME_LEN+1];
   char *pta;

   if((pta=strchr(prefix,'@'))) *pta='\0'; // Remove domain from prefix

   if(client->clType==PEER_MSGD) 
      mkFullName(client->Remote,client->Name,fname);
   else
      mkFullName(client->Name,"",fname);

   LAO_Info(NULL,"Client %s requests to be notified on var matching \"%s\"", fname, prefix);
   if(IS_ERROR(stat=addToNotifyTable(prefix,client,fname))) {
      if(IS_ERROR(stat)) LAO_Error(client,stat,"In NotifyMe(%s)",prefix);
      return stat;
   }

   InitVarScan(prefix,&listX,LOCKMUTEX); // Initialize Variable list

   for(;;) {
      DB_Var *vpt=NextVarScan(&listX);
      if(vpt==NULL) break;    // No more matches
        
      stat=addToNotifList(vpt,client,fname);
      if(IS_ERROR(stat)) 
          LAO_Error(client,stat,"In NotifyMe(%s)",prefix);
   }
   EndVarScan(&listX);
   return NO_ERROR;
}

//@Function: NoNotifyMe
//
// Cancels a previous requests of notification of variable change
//
// This routine is called to remove a client from a variable's
// list of notifications.
//@

int NoNotifyMe(Client *client,
               char *prefix,  // If prefix is NULL, removes client completely
               int silent )   // If set to 1, trying to cancel an unregisterd
                              // client is not logged as an error
{
   Listscan listX;
   int stat;
   char fname[LONG_NAME_LEN+1];

   if(!prefix) 
      prefix="*";
   else {
      char *pta;                    // Remove domain from prefix
      if((pta=strchr(prefix,'@'))) *pta='\0';
   }

   if(client->clType==PEER_MSGD) 
      mkFullName(client->Remote,client->Name,fname);
   else
      mkFullName(client->Name,"",fname);

   LAO_Info(NULL, "Client %s requests to be removed from notification on var matching \"%s\"",fname, prefix);

   delFromNotifyTable(prefix,fname);

   InitVarScan(prefix,&listX,LOCKMUTEX); // Initialize Variable list

   for(;;) {
      DB_Var *vpt=NextVarScan(&listX);
      if(vpt==NULL) break;
      stat=delFromNotifList(vpt,fname); 
      if(IS_ERROR(stat)) {
         if(!silent || (stat!=NOT_IN_CLIST_ERROR)) 
            LAO_Error(client,stat,"In NoNotifyMe(%s)",prefix);
      }  else
         LAO_Debug(NULL, "Client %s has been removed from notify list of var \"%s\"",fname,vpt->Name);
   }
   EndVarScan(&listX);
   return NO_ERROR;
}


// The following routines are used to manage the reply to VARLIST
// message.

//@Function: InitVarScan 
//
// Intialize the list of variables
//
// This routine must be called to initialize a variable list
// scanning operation. 
// After initialization NextVarScan can be called to get next variable 
// from the table. At the end NextVarScan returns NULL and EndVarScan() 
// must be called to reset and release resources used.
//
// A trailing "*" in the key means that we want to scan the list returning each 
// variable matching the given prefix. If no trailing "*", an exact match is 
// requested. In this case the initialization retrieves the given variable 
// (if any) via an hash search and sets up the scanning list so that only that 
// variable is returned.
//
//@

int InitVarScan(char *prefix,         // Variable name prefix
                Listscan *listX,      // List status structure
                int lock)             // If set to 1, lock the variable table
                                      // (EndVarScan() will unlock)
{
    int len=strlen(prefix);

    if(len>VAR_NAME_LEN) 
      return PLAIN_ERROR(VARPREFIX_ERROR);

    if(len>0 && prefix[len-1]=='*') { 
        len--;
        sinit(&(variableTB.stable),prefix,len, listX);
    } else {
        int found=hsearch(&(variableTB.htable),prefix);
        if(found>=0)
           listX->dummy=variableTB.htable.body[found];
        else
           listX->dummy=NULL;
        listX->kl=(-1);     // This means that exact matching is required
    }
    if((listX->locked=lock)) {               // Lock Variable table
        MUTEX_LOCK(&variableTB.mutex,"VarScan");
    }
    return NO_ERROR;
}

//@Function: NextVarScan
//
// Get next matching entry in Var List
//
// This routine must be called repeatedly to scan the variable list.
// The scanning mechanism must be initialized with a call to InitVarScan()
//@

DB_Var *NextVarScan(Listscan *listX)   // Returns: pointer to DB_Var, or NULL
{

    DB_Var *ret=NULL;
    s_item *sit;

    if(listX->kl<0) {          // If exact matching, do not scan list
        ret=listX->dummy;
        listX->dummy=NULL;
    } else {
        sit = snext(listX);
        if(sit)
            ret=sit->value;
        else 
            ret=NULL;
    }
    return ret;
}


//@Function{API}: EndVarScan

// Terminates the Var list scanning


// This function must be called at the end of a list scanning
//@

int EndVarScan(Listscan *listX)
{
    if(listX->locked) {
        MUTEX_UNLOCK(&variableTB.mutex,"VarScan");
    }
    return NO_ERROR;
}


//@Function{API}: nVars

// Returns number of variables in RTDB

//@

int nVars(void)
{
    return variableTB.htable.nItms;
}


//@Function{API}: LockTable

// Lock access to variable table

// This function can be called from external program to set a general lock on
// the variable table.

//@

int LockTable(void)
{
    MUTEX_LOCK(&variableTB.mutex,"LockTable");
    return NO_ERROR;
}



//@Function{API}: UnlockTable

// Unlock access to variable table

// This function can be called from external program to release the lock on
// the variable table.

//@

int UnlockTable(void)
{
    MUTEX_UNLOCK(&variableTB.mutex,"LockTable");
    return NO_ERROR;
}




//@Function{API}: VarTrace 
//
// Set trace on variable set
//
// This routine allows a client to set/unset the trace flag of
// the variable set specified.
// When trace is on, the variable value is written onto log file
// whenever the variable value changes.
//@

int VarTrace(Client *client,
             char *prefix,     // Variable name prefix. 
             int  op)          // Operation (0: unset, 1: set)
                               // Returns: status code
{
Listscan listX;
int stat=NO_ERROR;
int cmatch=0;
char *msg;
char vnbuf[VAR_NAME_LEN+1];

rmVDomain(prefix,vnbuf);

InitVarScan(vnbuf,&listX,LOCKMUTEX); // Initialize Variable list
	                                  // retrieval (whith mutex lock)
   
if(op) {
  op=1;
  msg="set";
} else {
  op=0;
  msg="unset";
}

for(;;) {                             // Scan variable list
    DB_Var *vpt=NextVarScan(&listX);

    if(vpt==NULL) {                   // List is finished
        EndVarScan(&listX);
	break;
    }
    cmatch++;
    vpt->trace=op;
    LAO_Info(client,"%s trace on var \"%s\"",msg, vpt->Name);
}
if(cmatch==0) {
    stat = VAR_NOT_FOUND_ERROR;
    LAO_Error(client,stat,"in VarTrace: no var matching \"%s\"",vnbuf);
}

return stat;
}


