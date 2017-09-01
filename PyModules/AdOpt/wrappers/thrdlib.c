//@File: thrdlib.c
//
// Thrdlib interface for Python functions
//@

#include <Python.h>
#include "structmember.h"

#include <stdlib.h>

#include "../lib/base/thrdlib.h"

static int debug=0;

// Funzioni chiamabili da Python

void initthrdlib( void);

static PyObject* PySetVersion(PyObject*, PyObject*);
static PyObject* PythInit(PyObject*, PyObject*);
static PyObject* PythHandler(PyObject*, PyObject*, PyObject *);
static PyObject* PythStart(PyObject*, PyObject*, PyObject *);
static PyObject* PythClose(PyObject*, PyObject*);
static PyObject* PythCleanup(PyObject*, PyObject*);
static PyObject* PythSendMsg(PyObject*, PyObject*, PyObject *);
static PyObject* PythSendMessage(PyObject*, PyObject*, PyObject *);
static PyObject* PythReplyMsg(PyObject*, PyObject*, PyObject *);
static PyObject* PythCheckMsg(PyObject*, PyObject*, PyObject*); 
static PyObject* PythWriteVar(PyObject*, PyObject*,PyObject*);
static PyObject* PythWaitMsg(PyObject*, PyObject*, PyObject*);
static PyObject* PythGetVar(PyObject*, PyObject*, PyObject*);
static PyObject* PythReadVar(PyObject*, PyObject*, PyObject*) ;
static PyObject* PythClntsList(PyObject*, PyObject*,PyObject*) ;
static PyObject* PythVarOp(PyObject*, PyObject*,PyObject*);
static PyObject* PythWaitReady(PyObject*, PyObject*,PyObject*);
static PyObject* PythFlush(PyObject*, PyObject*, PyObject*); 
static PyObject* PythPing(PyObject*, PyObject*, PyObject*); 
static PyObject* PythDebug(PyObject*, PyObject*);
static PyObject* PythLogMsg(PyObject*, PyObject*,PyObject*);

static PyObject *PybufRequest( PyObject *, PyObject *);
static PyObject *PybufRead( PyObject *, PyObject *);
static PyObject *PybufCreate( PyObject *, PyObject *);
static PyObject *PybufWrite( PyObject *, PyObject *);


// Funzioni mascherate a Python
static PyObject * ParseMessage(MsgBuf*);
unsigned char *valsToBytes( PyObject *vals, int vartype);
static PyObject *PyVar( MsgBuf *msgb);
static PyObject *PyVarValues(Variable *TheVar);
int myPyObject_Size( PyObject *obj);


// tabella dei metodi chiamabili da Python

static struct PyMethodDef thrdlib_methods[] = {
   { "SetVersion", PySetVersion, METH_VARARGS, "Set Version" },
   { "thInit", PythInit, METH_VARARGS, "Initializes the library" },
   { "thHandler", (PyCFunction)PythHandler, METH_VARARGS|METH_KEYWORDS,"Install message handlers" },
   { "thStart", (PyCFunction)PythStart, METH_VARARGS|METH_KEYWORDS, "Start communication with MSGD" },
   { "thClose", PythClose, METH_VARARGS, "Close communication with MSGD" },
   { "thCleanup", PythCleanup, METH_VARARGS, "Cleanup the library" },
   { "thSendMsg", (PyCFunction)PythSendMsg, METH_VARARGS|METH_KEYWORDS, "Send a message" },
   { "thSendMessage", (PyCFunction)PythSendMessage, METH_VARARGS|METH_KEYWORDS, "Send a message (extended)" },
   { "thReplyMsg", (PyCFunction)PythReplyMsg, METH_VARARGS|METH_KEYWORDS, "Replies to message" },
   { "thCheckMsg", (PyCFunction)PythCheckMsg, METH_VARARGS|METH_KEYWORDS, "Check if messages are available" },
   { "thWaitMsg", (PyCFunction)PythWaitMsg, METH_VARARGS|METH_KEYWORDS, "Wait for messages" },
   { "thWriteVar", (PyCFunction)PythWriteVar, METH_VARARGS|METH_KEYWORDS, "Write a variable" },
   { "thGetVar", (PyCFunction)PythGetVar, METH_VARARGS|METH_KEYWORDS, "Get a Variable" },
   { "thReadVar", (PyCFunction)PythReadVar, METH_VARARGS|METH_KEYWORDS, "Read a variable set" },
   { "thClntsList", (PyCFunction)PythClntsList, METH_VARARGS|METH_KEYWORDS, "Get current client list" },
   { "thVarOp", (PyCFunction)PythVarOp, METH_VARARGS|METH_KEYWORDS, "Generic variable operation" },
   { "thWaitReady", (PyCFunction)PythWaitReady, METH_VARARGS|METH_KEYWORDS, "wait for a client to become ready" },
   { "thFlush", (PyCFunction)PythFlush, METH_VARARGS, "Flush message queue" },
   { "thPing", (PyCFunction)PythPing, METH_VARARGS, "Pings a client" },
   { "thDebug", (PyCFunction)PythDebug, METH_VARARGS, "Change logging level" },
   { "thLogMsg", (PyCFunction)PythLogMsg, METH_VARARGS|METH_KEYWORDS, "Log a message to MSGD" },

    {"bufRequest", PybufRequest, METH_VARARGS, "Attach a consumer to a block of shared memory"},
    {"bufRead", PybufRead, METH_VARARGS, "Retrieve a local copy of the shared memory buffer"},
    {"bufCreate", PybufCreate, METH_VARARGS, "Creates a block of shared memory"},
    {"bufWrite", PybufWrite, METH_VARARGS, "Writes data into a shared memory buffer"},

   {NULL, NULL, 0, NULL}
};


// Auxiliary routine to call a Python handler
// This is called by the handler management queue
//
// The python callable object which is the handler has been stored
// as the argument by the internal call to thHandler()
//

static int callPyHandler(MsgBuf *msgb, void *harg, int nq){
    PyObject *pyMsg, *result;

    PyGILState_STATE gstate = PyGILState_Ensure();

    pyMsg=ParseMessage(msgb);

    /* Debug 
    PyObject *nameattr;
    char *name;
    nameattr=PyObject_GetAttrString(harg,"__name__");   // Get name of handler
    PyArg_Parse(nameattr,"s",&name);
    printf("Calling handler: %s\n", name);
    */



    result=PyEval_CallObject((PyObject *)harg, Py_BuildValue("(O)", pyMsg));

    PyGILState_Release(gstate);

    thRelease(msgb);
    Py_DECREF(pyMsg);    // CHECK
    if(result==NULL)
        return PYTHON_HNDLR_ERROR;
    else
        {
        Py_DECREF(result);
        return NO_ERROR;
        }
}


static PyObject *ThrdErrGeneric;
static PyObject *ThrdErrVarNotFound;

static char strbuf[1024];

static void reportError(char *resource, int code)
{
snprintf(strbuf,1024,"%s error(%d): %s",resource,PLAIN_ERROR(code),lao_strerror(code));

if (code == VAR_NOT_FOUND_ERROR)
   PyErr_SetString( ThrdErrVarNotFound, strbuf);
else
   PyErr_SetString(ThrdErrGeneric, strbuf );
}

// Funzione __init__, chiamata al momento dell'importazione del modulo

PyMODINIT_FUNC initthrdlib()
{
   PyObject *m;

   m= Py_InitModule("thrdlib", thrdlib_methods);

                                       // Adding exception
   ThrdErrGeneric=PyErr_NewException("thrdlib.error", NULL, NULL);
   Py_INCREF(ThrdErrGeneric);
   PyModule_AddObject(m, "error", ThrdErrGeneric);

   ThrdErrVarNotFound=PyErr_NewException("thrdlib.varNotFound", NULL, NULL);
   Py_INCREF(ThrdErrVarNotFound);
   PyModule_AddObject(m, "varNotFound", ThrdErrVarNotFound);

// Adding constants

   PyModule_AddIntConstant(m,"INT_VARIABLE", INT_VARIABLE);
   PyModule_AddIntConstant(m,"REAL_VARIABLE", REAL_VARIABLE);
   PyModule_AddIntConstant(m,"CHAR_VARIABLE", CHAR_VARIABLE);
   PyModule_AddIntConstant(m,"BIT8_VARIABLE", BIT8_VARIABLE);
   PyModule_AddIntConstant(m,"BIT16_VARIABLE", BIT16_VARIABLE);
   PyModule_AddIntConstant(m,"BIT32_VARIABLE", BIT32_VARIABLE);
   PyModule_AddIntConstant(m,"BIT64_VARIABLE", BIT64_VARIABLE);

   PyModule_AddIntConstant(m,"DUMPVARS", DUMPVARS);
   PyModule_AddIntConstant(m,"GIVEVAR", GIVEVAR);
   PyModule_AddIntConstant(m,"LOCKVAR", LOCKVAR);
   PyModule_AddIntConstant(m,"TAKEVAR", TAKEVAR);
   PyModule_AddIntConstant(m,"TRACEVAR", TRACEVAR);
   PyModule_AddIntConstant(m,"VARNOTIF", VARNOTIF);
   PyModule_AddIntConstant(m,"VARREPLY", VARREPLY);
   PyModule_AddIntConstant(m,"VARCHANGD", VARCHANGD);
   PyModule_AddIntConstant(m,"WFSARB_CMD", WFSARB_CMD);
   PyModule_AddIntConstant(m,"TERMINATE", TERMINATE);

   
   // Prepare for multithread operation
   PyEval_InitThreads();

}

// @Entry: PySetVersion

// Set the main program version
//
// Required arguments: (2) Major version number, Minor version number (int,int)
 
static PyObject *PySetVersion( PyObject *self, PyObject *args)
{
   int vmj,vmi;

   if (!PyArg_ParseTuple( args, "ii", &vmj, &vmi)) {
      PyErr_SetString(PyExc_TypeError, "wrong arguments");
      return NULL;
   }

   SetVersion(vmj,vmi);
   
   Py_INCREF(Py_None);
   return Py_None;
}


// @Entry: PythInit

// connects to MsgD-RTDB and sends a registration message
// Returns a file descriptor for the communication socket, or an error value (<0)
// Will retry for 10 seconds before giving up.
//
// Required arguments: (1) name of client to register (string)
 
static PyObject *PythInit( PyObject *self, PyObject *args)
{
   static char *me="thInit";
   char *name;
   int err;

   if (!PyArg_ParseTuple( args, "s", &name)) {
      PyErr_SetString(PyExc_TypeError, "wrong arguments");
      return NULL;
   }

   if (strlen(name) > PROC_NAME_LEN) {
      reportError(me,CLIENT_NAME_ERROR);
      return NULL;
   }
   if(IS_ERROR(err = thInit(name))) {
      reportError(me,err);
      return NULL;
   }
   Py_INCREF(Py_None);
   return Py_None;
}

// @Entry: PythHandler
//
// Install a message handler
//
// Required arguments: 
//       handler  (callable object)
//
// Optional arguments:
//       code     (int)
//       sender   (string)
//       seqn     (int)
//@
 

static PyObject *PythHandler( PyObject *self, PyObject *args, PyObject *kwargs)
{
static char *me="thHandler";
static char*kwlist[]= { "handler", "code", "sender", "seqn", NULL };

   char *name;
   int msgc=ANY_MSG;
   int seqnum=0;
   char * sender="*";
   int err;
   PyObject *pyHndlr;
   PyObject *nameattr;

   if (!PyArg_ParseTupleAndKeywords( args, kwargs, "O|isi",
                   kwlist, &pyHndlr,&msgc,&sender,&seqnum)) return NULL;

   if(!PyCallable_Check(pyHndlr)) {
      PyErr_SetString(PyExc_TypeError, "handler must be callable");
      return NULL;
   }
   nameattr=PyObject_GetAttrString(pyHndlr,"__name__");   // Get name of handler
   PyArg_Parse(nameattr,"s",&name);

   err = thHandler(msgc, sender, seqnum, callPyHandler, name, (void *)pyHndlr);
   if(IS_ERROR(err)) {
      reportError(me,err);
      return NULL;
   }

   Py_INCREF(pyHndlr);
   Py_INCREF(Py_None);
   return Py_None;
}


// @Entry: PythStart
//
// Connects to the message daemon and starts everything
//
// Required arguments: 
//       none
//
// Optional keyword arguments:
//       IP    (string)
//       ntry  (int)
//@
 
static PyObject *PythStart(PyObject *self, PyObject *args, PyObject *kwargs)
{
static char *me="thStart";
static char*kwlist[]= { "IP", "ntry", NULL };
char *msgdIP="127.0.0.1";
int ntry=1,ret;

   if (!PyArg_ParseTupleAndKeywords( args, kwargs, "|si",
                                      kwlist,&msgdIP,&ntry)) return NULL;

   ret=thStart(msgdIP,ntry);
   if(IS_ERROR(ret)) {
      reportError(me,ret);
      return NULL;
   }
   Py_INCREF(Py_None);
   return Py_None;
}


// @Entry: PythClose
//
// Close connection with the message daemon
//
// Required arguments: 
//       none
//
//@
 
static PyObject *PythClose( PyObject *self, PyObject *args)
{
static char *me="thClose";
int ret;

ret=thClose();
if(IS_ERROR(ret)) {
   reportError(me,ret);
   return NULL;
}
Py_INCREF(Py_None);
return Py_None;
}



// @Entry: PythCleanup
//
// Close connection with the message daemon and deallocate all resources
//
// Required arguments: 
//       none
//
//@
 
static PyObject *PythCleanup( PyObject *self, PyObject *args)
{
static char *me="thCleanup";
int ret;

   ret=thCleanup();
   if(IS_ERROR(ret)) {
      reportError(me,ret);
      return NULL;
   }
   Py_INCREF(Py_None);
   return Py_None;
}



// @Entry: PythSendMsg
//
// Send a generic message
//
// Required arguments: 
//       code         (int)
//
// Optional arguments: 
//       dest         (string)
//       data         (string)
//       payload      (int)
//       ignf         (int)
//@
 
static PyObject *PythSendMsg( PyObject *self, PyObject *args, PyObject *kwargs)
{
static char *me="thSendMsg";
static char*kwlist[]= { "code", "dest", "data", "length", "payload", "ignf", NULL };
char *dest="";
char *data=NULL;
int pload=0,ignf=0,code;
int len=0,ret;

   if (!PyArg_ParseTupleAndKeywords( 
         args, kwargs, "i|ss#iii",kwlist,&code,&dest,&data,&len,&pload,&ignf)) return NULL;

   ret=thSendMsgPl(len,dest,code,pload,ignf,data);
   if(IS_ERROR(ret)) {
      reportError(me,ret);
      return NULL;
   }
   return Py_BuildValue("i",ret);
}

// @Entry: PythSendMessage
//
// Send a generic message (also specify payload and ttl)
//
// Required arguments: 
//       code         (int)
//
// Optional arguments: 
//       dest         (string)
//       data         (string)
//       payload      (int)
//       ignf         (int)
//       ttl          (int)
//@
 
static PyObject *PythSendMessage( PyObject *self, PyObject *args, PyObject *kwargs)
{
static char *me="thSendMessage";
static char*kwlist[]= { "code", "dest", "data", "payload", "ignf", "ttl", NULL };
char *dest="";
char *data=NULL;
int payload=0,ignf=0,code,ttl=0;
int len=0,ret;

   if (!PyArg_ParseTupleAndKeywords( 
         args, kwargs, "i|ss#iii",kwlist,&code,&dest,&data,&len,&payload,&ignf,&ttl)) return NULL;

   ret=thSendMessage(len,dest,code,payload,ignf,ttl,data);
   if(IS_ERROR(ret)) {
      reportError(me,ret);
      return NULL;
   }
   return Py_BuildValue("i",ret);
}


// @Entry: PythReplyMsg
//
// Send a generic reply message
//
// Required arguments: 
//       code         (int)
//       msg          (msgb) original message
//
// Optional arguments
//       data         (string)
//@
 
static PyObject *PythReplyMsg( PyObject *self, PyObject *args, PyObject *kwargs)
{
static char *me="thReplyMsg";
static char*kwlist[]= { "msgb", "code", "data","payload", NULL };
char *data=NULL;
int code, msgb_ptr;
int len=0,pload=0,ret;

   if (!PyArg_ParseTupleAndKeywords( 
         args, kwargs, "ii|s#i",kwlist,&msgb_ptr,&code,&data,&len,&pload)) return NULL;

   ret=thReplyMsgPl(code,pload,len,data,(MsgBuf *)msgb_ptr);
   if(IS_ERROR(ret)) {
      reportError(me,ret);
      return NULL;
   }
   return Py_BuildValue("i",ret);
}


// @Entry: PythCheckMsg
//
// Send a generic medssage
//
// Required arguments: None
//
// Optional keyword arguments: 
//       code         (int)
//       sender       (string)
//       seqn         (int)
//@
 
static PyObject *PythCheckMsg( PyObject *self, PyObject *args, PyObject *kwargs)
{
static char*kwlist[]= { "code", "sender", "seqn", NULL };
char *sender="*";
int code=ANY_MSG;
int seqn=0;
int ret;

   if (!PyArg_ParseTupleAndKeywords( 
         args, kwargs, "|isi",kwlist,&code,&sender,&seqn)) return NULL;

   ret=thCheckMsg(code,sender,seqn);
   return Py_BuildValue("i",ret);
}


// @Entry: PythWaitMsg
//
// Waits for a message 
//
// Required arguments: None
//
// Optional keyword arguments: 
//       code         (int)
//       sender       (string)
//       seqn         (int)
//       timeout      (int)
//@
 
static PyObject *PythWaitMsg( PyObject *self, PyObject *args, PyObject *kwargs)
{
static char *me="thWaitMsg";
static char*kwlist[]= { "code", "sender", "seqn", "timeout", NULL };
char *sender="*";
int code=ANY_MSG;
int seqn=0;
int timeout=0;
int errcod=NO_ERROR;
MsgBuf *msg;
PyObject* pyMsg;

   if (!PyArg_ParseTupleAndKeywords( 
         args, kwargs, "|isii",kwlist,&code,&sender,&seqn,&timeout)) return NULL;

   Py_BEGIN_ALLOW_THREADS

   msg=thWaitMsg(code,sender,seqn,timeout,&errcod);

   Py_END_ALLOW_THREADS


   if(msg) {
      pyMsg=ParseMessage(msg);
      thRelease(msg);
   } else {
      pyMsg=NULL;
      reportError(me,errcod);
   }
   return pyMsg;
}

int myPyObject_Size( PyObject *obj)
{
   if ( PySequence_Check(obj) || PyMapping_Check(obj))
      return PyObject_Size(obj);
   else
      return 1;
}

// @Entry: PythWriteVar
//
// Waits for a message 
//
// Required arguments: 
//       name  (string)
//       type  (int)
//       values (sequence)
//
// Optional keyword arguments: 
//       timeout      (int)
//@
 
static PyObject *PythWriteVar( PyObject *self, PyObject *args, PyObject *kwargs)
{
static char *me="thWriteVar";
static char*kwlist[]= { "name", "type", "values", "timeout", NULL };
char *name;
int nelm,vtype,ret,tmout=0;
PyObject* vals;
unsigned char *value;

   if (!PyArg_ParseTupleAndKeywords( 
         args, kwargs, "siO|i",kwlist,&name,&vtype,&vals,&tmout)) return NULL;

   nelm=myPyObject_Size(vals);

   value=valsToBytes(vals,vtype);
   ret=thWriteVar(name, vtype, nelm, (void*)value, tmout);
   free(value);
   if(IS_ERROR(ret)) {
      reportError(me,ret);
      return NULL;
   }
   Py_INCREF(Py_None);
   return Py_None;
}



// @Entry: PythGetVar
//
// Get value of a variable
//
// Required arguments: 
//       name  (string)
//
// Optional keyword arguments: 
//       timeout      (int)
//@
 
static PyObject *PythGetVar( PyObject *self, PyObject *args, PyObject *kwargs)
{
static char errstr[VAR_NAME_LEN+1 + 9];
static char*kwlist[]= { "name", "timeout", NULL };
char *name;
int ret,tmout=0;
MsgBuf* msg;

   strcpy(errstr, "thGetVar ");

   if (!PyArg_ParseTupleAndKeywords( 
         args, kwargs, "s|i",kwlist,&name,&tmout)) return NULL;

   msg=thGetVar(name,tmout,&ret);

   if(!msg) {
      strcat(errstr, name);
      reportError(errstr,ret);
      return NULL;
   }
   PyObject *py_obj = PyVar(msg);
   thRelease(msg);

   return py_obj;
}



// @Entry: PythReadVar
//
// Get value of a variable
//
// Required arguments: 
//       prefix  (string)
//
// Optional keyword arguments: 
//       timeout      (int)
//@
 
static PyObject *PythReadVar( PyObject *self, PyObject *args, PyObject *kwargs)
{
static char *me="thReadVar";
static char*kwlist[]= { "prefix", "timeout", NULL };
char *name;
int tmout=5000;
int ret=NO_ERROR;
PyObject* vals;
MsgBuf* msg;

   if (!PyArg_ParseTupleAndKeywords( 
         args, kwargs, "s|i",kwlist,&name,&tmout)) return NULL;

   msg=thReadVar(name,tmout,&ret);

   if(!msg && IS_ERROR(ret)) {
      reportError(me,ret);
      return NULL;
   }

   vals=Py_BuildValue("[]");
   while(msg) {
      PySequence_InPlaceConcat(vals,Py_BuildValue("[N]", PyVar(msg)));
      Py_DECREF(vals);  // CHECK
      msg=thNextVar(msg);
   }
//   Py_INCREF(vals);   // CHECK removal
   return vals;
}



// @Entry: PythClntsList
//
// Get value of a variable
//
// Required arguments: 
//       prefix  (string)
//
// Optional keyword arguments: 
//       timeout      (int)
//@
 
static PyObject *PythClntsList( PyObject *self, PyObject *args, PyObject *kwargs)
{
static char *me="thClntsList";
static char*kwlist[]= { "timeout", NULL };
char *name;
int vtype,tmout=0;
int ret=NO_ERROR;
PyObject* vals;
MsgBuf* msg;

   if (!PyArg_ParseTupleAndKeywords( 
         args, kwargs, "|i",kwlist,&name,&vtype,&tmout)) return NULL;

   msg=thClntsList(tmout,&ret);

   if(!msg && IS_ERROR(ret)) {
      reportError(me,ret);
      return NULL;
   }

   vals=Py_BuildValue("[]");
   while(msg) {
      PySequence_InPlaceConcat(vals,Py_BuildValue("[N]", PyVar(msg)));
      Py_DECREF(vals);     // CHECK
      msg=thNextVar(msg);
   }

//   Py_INCREF(vals);      // CHECK removal
   return vals;
}

// @Entry: PyVar
//
// Builds a Python dictionary with a variable's fields.
//
// Required arguments:
//       msgb   (MsgBuf *)

static PyObject *PyVar( MsgBuf *msgb)
{
   Variable *TheVar = thValue(msgb);

   PyObject *value = PyVarValues(TheVar);

  return Py_BuildValue("{s:s,s:i,s:s,s:s,s:i,s:i,s:i, s:N}", "sender", HDR_FROM(msgb),
                 "code", HDR_CODE(msgb),"name", TheVar->H.Name, "owner", TheVar->H.Owner, "seqnum", HDR_SEQN(msgb),
                  "type", TheVar->H.Type, "mtime", TheVar->H.MTime.tv_sec, "value",value);
     
}


   



// @Entry: PythVarOp
//
// Get value of a variable
//
// Required arguments: 
//       prefix  (string)
//       opcode  (int)
//
// Optional keyword arguments: 
//       timeout      (int)
//@
 
static PyObject *PythVarOp( PyObject *self, PyObject *args, PyObject *kwargs)
{
static char *me="thVarOp";
static char*kwlist[]= { "prefix", "opcode", "subcode", "timeout", NULL };
char *name;
int opcode,subcode=0,tmout=0;
int ret;

   if (!PyArg_ParseTupleAndKeywords( 
         args, kwargs, "si|ii",kwlist,&name,&opcode,&subcode, &tmout)) return NULL;

   ret=thVarOp(name,opcode,subcode,tmout);

   if(IS_ERROR(ret)) {
      reportError(me,ret);
      return NULL;
   }

   Py_INCREF(Py_None);
   return Py_None;
}



// @Entry: PythWaitReady
//
// Wait for a client to be ready
//
// Required arguments: 
//       client  (string)
//
// Optional keyword arguments: 
//       timeout      (int)
//@
 
static PyObject *PythWaitReady( PyObject *self, PyObject *args, PyObject *kwargs)
{
static char *me="thWaitReady";
static char*kwlist[]= { "client", "timeout", NULL };
char *name;
int tmout=0;
int ret;

   if (!PyArg_ParseTupleAndKeywords( 
         args, kwargs, "s|i",kwlist,&name,&tmout)) return NULL;

   ret=thWaitReady(name,tmout);

   if(IS_ERROR(ret)) {
      reportError(me,ret);
      return NULL;
   }

   Py_INCREF(Py_None);
   return Py_None;
}



// @Entry: PythDebug
//
// Get value of a variable
//
// Required arguments: 
//       level  (int)
//
//@
 
static PyObject *PythDebug(PyObject* self, PyObject* args)
{
int level;

   if (!PyArg_ParseTuple(args, "i",&level)) return NULL;

   thDebug(level);

   Py_INCREF(Py_None);
   return Py_None;
}



// @Entry: PythPing
//
// Wrapper to thPing
//
// Required arguments: 
//       client  (string)
//
// Optional keyword arguments: 
//       timeout      (int)
//@
 
static PyObject *PythPing( PyObject *self, PyObject *args, PyObject *kwargs)
{
static char *me="thPing";
static char*kwlist[]= { "client", "timeout", NULL };
char *name;
int tmout=0;
double delay;
int ret;

   if (!PyArg_ParseTupleAndKeywords( 
         args, kwargs, "s|i",kwlist,&name,&tmout)) return NULL;

   ret=thPing(name,tmout,&delay);

   if(IS_ERROR(ret)) {
      reportError(me,ret);
      return NULL;
   }
   return Py_BuildValue("d",delay);
}

// @Entry: PythLogMsg
//
// Get value of a variable
//
// Optional keyword arguments: 
//       code   (int)
//       mesg   (string)
//@
 
static PyObject *PythLogMsg( PyObject *self, PyObject *args, PyObject *kwargs)
{
static char *me="thLogMsg";
static char*kwlist[]= { "level", "seqn", "msg", NULL };
char *msg;
int seqn=0;
int ret;
int level;

   if (!PyArg_ParseTupleAndKeywords( 
         args, kwargs, "|iis",kwlist,&level,&seqn,&msg)) return NULL;

   ret=thLogMsg(level,seqn,msg);

   if(IS_ERROR(ret)) {
      reportError(me,ret);
      return NULL;
   }

   Py_INCREF(Py_None);
   return Py_None;
}



// @Entry: PythFlush
//
// Flush the message queue
//
// Optional keyword arguments: 
//       code     (int)
//       sender   (string)
//       seqn     (string)
//@
 
static PyObject *PythFlush( PyObject *self, PyObject *args, PyObject *kwargs)
{
static char *me="thFlush";
static char*kwlist[]= { "code", "sender", "seqn", NULL };
char *sender="*";
int code=ANY_MSG,seqn=0;
int ret;

   if (!PyArg_ParseTupleAndKeywords( 
         args, kwargs, "|isi",kwlist,&code,&sender,seqn)) return NULL;

   ret=thFlush(code,sender,seqn);

   if(IS_ERROR(ret)) {
      reportError(me,ret);
      return NULL;
   }

   return Py_BuildValue("i",ret);
}

unsigned char *valsToBytes( PyObject *vals, int vartype)
{
   unsigned char *buf;
   int len, i;

   if (vartype == CHAR_VARIABLE) {
      char *tempbuf;
      PyString_AsStringAndSize( vals, &tempbuf, &len);
      len++;
      buf = malloc(len);
      memcpy(buf, tempbuf, len);
      buf[len-1]=0;     // Ensure zero-termination (maybe not needed)
      return buf;
   }

   len = myPyObject_Size(vals);
   buf = malloc(VarSize(vartype, len));

   for (i=0; i<len; i++)
      {
      switch(vartype) {
         case INT_VARIABLE:
            if (PySequence_Check(vals))
               ((int *)buf)[i] = PyInt_AsLong( PySequence_GetItem(vals, i));
            else
               ((int *)buf)[0] = PyInt_AsLong(vals);
            break;
         case REAL_VARIABLE:
            if (PySequence_Check(vals))
               ((double *)buf)[i] = PyFloat_AsDouble(  PySequence_GetItem(vals, i));
            else
               ((double *)buf)[0] = PyFloat_AsDouble(vals);
            break;
         case BIT8_VARIABLE:
            if (PySequence_Check(vals))
               ((unsigned char *)buf)[i] = PyInt_AsLong( PySequence_GetItem( vals, i));
            else
               ((unsigned char *)buf)[i] = PyInt_AsLong(vals);
            break;
         case BIT16_VARIABLE:
            if (PySequence_Check(vals))
               ((unsigned short *)buf)[i] = PyInt_AsLong( PySequence_GetItem( vals, i));
            else
               ((unsigned short *)buf)[i] = PyInt_AsLong(vals);
            break;
         case BIT32_VARIABLE:
            if (PySequence_Check(vals))
               ((unsigned long *)buf)[i] = PyLong_AsUnsignedLong( PySequence_GetItem( vals, i));
            else
               ((unsigned long *)buf)[i] = PyLong_AsUnsignedLong(vals);
            break;
         case BIT64_VARIABLE:
            if (PySequence_Check(vals))
               ((unsigned long long *)buf)[i] = PyLong_AsUnsignedLongLong( PySequence_GetItem( vals, i));
            else
               ((unsigned long long *)buf)[i] = PyLong_AsUnsignedLongLong(vals);
         break;
         }
      }

   return buf;
}

static PyObject *PyVarValues(Variable *TheVar)
{
   int type = TheVar->H.Type;
   int len = TheVar->H.NItems;
   PyObject *py_object=Py_None;
   int i;

   switch(type) {
      case CHAR_VARIABLE:
               // Empty strings
         if (len==0)
            return PyString_FromString("");
	 for (i=0; i<len; i++)
            // we need this because strlen() might not work - strings do not need to be zero-terminated
            if (TheVar->Value.Sv[i] == '\0') {
                len =i;
                break;
            } 
         py_object = PyString_FromStringAndSize( TheVar->Value.Sv, len);
         break;
      case INT_VARIABLE:
         if (len==1) 
            py_object = PyInt_FromLong( TheVar->Value.Lv[0]);
         else {
            py_object = PyTuple_New(len);
            for ( i=0; i< len; i++)
               PyTuple_SetItem( py_object, i, PyInt_FromLong( TheVar->Value.Lv[i]));
         }
         break;
      case REAL_VARIABLE:
         if (len==1) 
            py_object = PyFloat_FromDouble( TheVar->Value.Dv[0]);
         else {
            py_object = PyTuple_New(len);
            for ( i=0; i< len; i++)
               PyTuple_SetItem( py_object, i, PyFloat_FromDouble( TheVar->Value.Dv[i]));
         }
         break;
      case BIT8_VARIABLE:
         if (len==1) 
            py_object = PyInt_FromLong( TheVar->Value.B8[0]);
         else {
            py_object = PyTuple_New(len);
            for ( i=0; i< len; i++)
               PyTuple_SetItem( py_object, i, PyInt_FromLong( TheVar->Value.B8[i]));
         }
         break;
      case BIT16_VARIABLE:
         if (len==1) 
            py_object = PyInt_FromLong( TheVar->Value.B16[0]);
         else {
            py_object = PyTuple_New(len);
            for ( i=0; i< len; i++)
               PyTuple_SetItem( py_object, i, PyInt_FromLong( TheVar->Value.B16[i]));
         }
         break;
      case BIT32_VARIABLE:
         if (len==1) 
            py_object = PyLong_FromUnsignedLong( TheVar->Value.B32[0]);
         else {
            py_object = PyTuple_New(len);
            for ( i=0; i< len; i++)
               PyTuple_SetItem( py_object, i, PyLong_FromUnsignedLong( TheVar->Value.B32[i]));
         }
         break;
      case  BIT64_VARIABLE:
         if (len==1) 
            py_object = PyLong_FromUnsignedLongLong( TheVar->Value.B64[0]);
         else {
            py_object = PyTuple_New(len);
            for ( i=0; i< len; i++)
               py_object = PyLong_FromUnsignedLongLong( TheVar->Value.B64[i]);
         }
   }

   if (py_object == Py_None)
      Py_INCREF(Py_None);

   return py_object;

}





///////////////////////////////////////////////////////////////////////////
static PyObject * ParseMessage(MsgBuf* msg){
   int bodysize, len, code;
   PyObject *py_object, *py_return;
   char *me = "ParseMessage";

   bodysize = HDR_LEN(msg);

   // If the message is malformed, return an empty array
   if ((bodysize > MAX_MSG_LEN) || (bodysize<0)) {
      reportError(me,MSG_LEN_ERROR);
      return NULL;
   }

   code=HDR_CODE(msg);

   switch(code) {    // Var reply messages are decoded here
      case VARREPLY:
      case VARCHANGD:
         return PyVar(msg);
         break;

      case CLNREADY:
        py_return = Py_BuildValue("{s:i,s:s,s:i}", "seqnum", HDR_SEQN(msg), "sender",
                              HDR_FROM(msg), "code", HDR_CODE(msg));
        break;

      default:
         len = HDR_LEN(msg);
         // Remove null termination for strings - Python doesn't want it
         if (MSG_BODY(msg)[len-1] == 0) len--;
         py_object = Py_BuildValue("s#", MSG_BODY(msg), len);
         py_return = Py_BuildValue("{s:i,s:s,s:i,s:O,s:i}", "seqnum", HDR_SEQN(msg), "sender", 
                HDR_FROM(msg), "code", HDR_CODE(msg), "body", py_object, "msgb", (int)msg);  

         Py_DECREF(py_object);   // CHECK
         break;
      }

   return py_return; 
}

////////////////////////
// Buflib wrapper
//


//+Function: pybufRequest
// Attach a consumer to a block of shared memory
//
// Arguments: 
//             client name (string)
//             buffer name (string)
//             
// Returns:    dict( 'errcode': error code, 'bufinfo': buffer structure (as a string) to be used in subsequenct calls)
//
//-

static PyObject *PybufRequest( PyObject *self, PyObject *args)
{
    char *clientname, *bufname;
    int stat;

   if (!PyArg_ParseTuple( args, "ss", &clientname, &bufname))
       return NULL;

   BufInfo *bufinfo = (BufInfo *)malloc(sizeof(BufInfo));

   if (debug) printf("%s requesting buf %s\n", clientname, bufname);
   stat = bufRequest(clientname, bufname, BUFCONT, bufinfo);

   if (debug) printf("bufRequest replied %d\n",stat);

   return Py_BuildValue("{s:i, s:s#}", "errcode", stat, "bufinfo", bufinfo, sizeof(BufInfo));
}



//@Function: PybufRead
// Retrieve a local copy of the shared memory buffer
//
// Argument: bufinfo, optional timeout in ms (defaults to 100)
// 
// Returns: dict('errcode': error code, 'counter': data counter, 'data': data buffer (string))
//
//@

static PyObject *PybufRead( PyObject *self, PyObject *args)
{
   int stat, len, errcode, counter;
   int timeout=500; //ms
   BufInfo *bufinfo;
   static void *data=NULL;
   static int datalen =0;


   if (!PyArg_ParseTuple( args, "s#|i", &bufinfo, &len, &timeout))
     return NULL;

   if (debug) printf("Got bufinfo, len = %d\n", bufinfo->com.lng);

   // Realloc data buffer if necessary
   int size = (size_t)bufinfo->com.lng;
   if (size != datalen)
      if (data) {
         free(data);
         data = NULL;
      }
   if (!data)
     data = (void*) malloc(size);

   stat = bufRead( bufinfo, data, timeout);
    
   if (debug) printf("Read frame %d buf len %d timeout %d \n", stat, bufinfo->com.lng, timeout);
   if (stat<0)
      {
      errcode = stat;
      counter=0;
      }
   else
      { 
      errcode = NO_ERROR;
      counter = stat;
      }

	PyObject *pyret = Py_BuildValue("{s:i, s:i, s:s#}", "errcode", errcode, "counter", counter, "data", data, bufinfo->com.lng);

	return pyret;
}

//
//+Function: pybufCreate
// Creates a block of shared memory
//
// Arguments: 
//             client name (string)
//             buffer name (string)
//             buffer size in bytes (int)
//             max no. of consumers (int)
//             
// Returns:    dict( 'errcode': error code, 'bufinfo': buffer structure (as a string) to be used in subsequenct calls)
//
//-
static PyObject *PybufCreate( PyObject *self, PyObject *args)
{
    char *clientname, *bufname;
    int bufsize, maxcons;
    int stat;

    if (!PyArg_ParseTuple( args, "ssii", &clientname, &bufname, &bufsize, &maxcons))
       return NULL;

   BufInfo *bufinfo = (BufInfo *)malloc(sizeof(BufInfo));

   if (debug) printf("%s creating buf %s\n", clientname, bufname);
   stat = bufCreate(clientname, bufname, BUFCONT, bufsize, maxcons, bufinfo);
   if (stat>=0)
      stat=0;

   if (debug) printf("bufCreate replied %d\n",stat);

   return Py_BuildValue("{s:i, s:s#}", "errcode", stat, "bufinfo", bufinfo, sizeof(BufInfo));
}


//@Function: PybufWrite
// Writes data into a shared memory buffer
//
// Arguments: 
//             bufinfo (block returned by bufCreate)
//             data buffer (any buffer object)
//             counter (int)
//             timeout (optional timeout in ms)
// 
// Returns:    errcode (int)
//
//@

static PyObject *PybufWrite( PyObject *self, PyObject *args)
{
   int stat, len, counter, datalen;
   int timeout=100; //ms
   BufInfo *bufinfo;
   void *data;


   if (!PyArg_ParseTuple( args, "s#s#i|i", &bufinfo, &len, &data, &datalen, &counter, &timeout))
     return NULL;

   if (debug) printf("Got bufinfo, len = %d\n", bufinfo->com.lng);

   stat = bufWrite( bufinfo, data, counter, timeout);
    
   if (debug) printf("Written frame %d buf len %d timeout %d \n", counter, bufinfo->com.lng, timeout);

   return Py_BuildValue("i", stat);
}








