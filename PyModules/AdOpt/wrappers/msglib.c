//@File: msglib.c
//
// Msglib interface for Python functions
//@

#include <Python.h>
#include <stdlib.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

#include "base/msglib.h"
#include "base/errlib.h"
#include "base/rtdblib.h"
#include "idllib/idllib.h"

#include "base/buflib.h"

// Funzioni chiamabili da Python

void initmsglib( void);
static PyObject *PyConnectToServer( PyObject *, PyObject *);
static PyObject *PyCloseServer( PyObject *, PyObject *);
static PyObject *PySendMessage( PyObject *, PyObject *);
static PyObject *PySendBinaryMessage( PyObject *, PyObject *);
static PyObject *PyWaitMessage( PyObject *, PyObject *);
static PyObject *PyCreateVar( PyObject *, PyObject *);
static PyObject *PySetVar( PyObject *, PyObject *);
static PyObject *PySetRealVar( PyObject *, PyObject *);
static PyObject *PySetIntVar( PyObject *, PyObject *);
static PyObject *PyDelVar( PyObject *, PyObject *);
static PyObject *PyDumpVars( PyObject *, PyObject *);
static PyObject *PyGetVar( PyObject *, PyObject *);
static PyObject *PyVarSize( PyObject *, PyObject *);
static PyObject *PyVarList( PyObject *, PyObject *);
static PyObject *PyVarNotif( PyObject *, PyObject *);
static PyObject *PyVarNoNotif( PyObject *, PyObject *);
static PyObject *PyLockVars( PyObject *, PyObject *);
static PyObject *PyUnlockVars( PyObject *, PyObject *);
static PyObject *PyWriteVar( PyObject *, PyObject *);

static PyObject *PyGetErrorString( PyObject *, PyObject *);

static PyObject *PyINT_VARIABLE( PyObject *, PyObject *);
static PyObject *PyREAL_VARIABLE( PyObject *, PyObject *);
static PyObject *PyCHAR_VARIABLE( PyObject *, PyObject *);
static PyObject *PyBIT8_VARIABLE( PyObject *, PyObject *);
static PyObject *PyBIT16_VARIABLE( PyObject *, PyObject *);
static PyObject *PyBIT32_VARIABLE( PyObject *, PyObject *);
static PyObject *PyBIT64_VARIABLE( PyObject *, PyObject *);

static PyObject *PybufRequest( PyObject *, PyObject *);
static PyObject *PybufRead( PyObject *, PyObject *);

static PyObject *PybufCreate( PyObject *, PyObject *);
static PyObject *PybufWrite( PyObject *, PyObject *);

// Funzioni mascherate a Python
static PyObject * ParseMessage(MsgBuf*);


// Registrazione dei metodi chiamabili da Python

static struct PyMethodDef msglib_methods[] = {
    {"ConnectToServer", PyConnectToServer, METH_VARARGS, "Connects and registers itself to the MsgD-RTDB"},
    {"CloseServer", PyCloseServer, METH_VARARGS, "Unregisters itself from the MsgD-RTDB"},
    {"SendMessage", PySendMessage, METH_VARARGS, "Sends a message to the MsgD-RTDB"},
    {"SendBinaryMessage", PySendBinaryMessage, METH_VARARGS, "Sends a message to the MsgD-RTDB"},
    {"WaitMessage", PyWaitMessage, METH_VARARGS, "Waits for a message from the MsgD-RTDB"},
    {"CreateVar", PyCreateVar, METH_VARARGS, "Asks the MsgD-RTDB to create a new variable"},
    {"SetVar", PySetVar, METH_VARARGS, "Sets a variable value"},
    {"SetRealVar", PySetRealVar, METH_VARARGS, "Sets a variable of type REAL"},
    {"SetIntVar", PySetIntVar, METH_VARARGS, "Sets a variable of type INT"},
    {"DelVar", PyDelVar, METH_VARARGS, "Delete a var from rtdb"},
    {"DumpVars", PyDumpVars, METH_VARARGS, "Dump a list of vars to log"},
    {"GetVar", PyGetVar, METH_VARARGS, "Gets a variable value"},
    {"VarSize", PyVarSize, METH_VARARGS, "Returns the size of a variable"},
    {"VarList", PyVarList, METH_VARARGS, "Get a list of variable specified by a regexp"},
    {"VarNotif", PyVarNotif, METH_VARARGS, "Asks to be notified when the variable changes, and optionally gets the current value"},
    {"VarNoNotif", PyVarNoNotif, METH_VARARGS, "Asks to not be notified anymore when the variable changes, and optionally gets the current value"},
    {"LockVars", PyLockVars, METH_VARARGS, "Send request to lock a list of variables"},
    {"UnlockVars", PyUnlockVars, METH_VARARGS, "Send request to unlock a list of variables"},
    {"WriteVar", PyWriteVar, METH_VARARGS, "Write a variable. Create the variable if not existent. Verify completion"},

    {"GetErrorString", PyGetErrorString, METH_VARARGS, "Return the error string corresponding to an error number"},
    {"INT_VARIABLE", PyINT_VARIABLE, METH_VARARGS, "Define the INT_VARIABLE code "},
    {"REAL_VARIABLE", PyREAL_VARIABLE, METH_VARARGS, "Define the REAL_VARIABLE code"},
    {"CHAR_VARIABLE", PyCHAR_VARIABLE, METH_VARARGS, "Define the CHAR_VARIABLE code"},
    {"BIT8_VARIABLE", PyBIT8_VARIABLE, METH_VARARGS, "Define the BIT8_VARIABLE code"},
    {"BIT16_VARIABLE", PyBIT16_VARIABLE, METH_VARARGS, "Define the BIT16_VARIABLE code"},
    {"BIT32_VARIABLE", PyBIT32_VARIABLE, METH_VARARGS, "Define the BIT32_VARIABLE code"},
    {"BIT64_VARIABLE", PyBIT64_VARIABLE, METH_VARARGS, "Define the BIT64_VARIABLE code"},

    {"bufRequest", PybufRequest, METH_VARARGS, "Attach a consumer to a block of shared memory"},
    {"bufRead", PybufRead, METH_VARARGS, "Retrieve a local copy of the shared memory buffer"},

    {"bufCreate", PybufCreate, METH_VARARGS, "Creates a block of shared memory"},
    {"bufWrite", PybufWrite, METH_VARARGS, "Writes data into a shared memory buffer"},

    {NULL, NULL, 0, NULL}
};


// Generic scratch buffer

char buffer[64];
int buffer_len=63;
char clientname[PROC_NAME_LEN+1];		// Name of this client
static int msg_id =1;
PyObject *method, *object;

static int debug_output = 0;

// Funzione __init__, chiamata al momento dell'importazione del modulo

void initmsglib()
{
    PyObject *m = Py_InitModule("msglib", msglib_methods);
    memset( clientname, 0, PROC_NAME_LEN+1);

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

    PyModule_AddIntConstant(m,"WFS_ARB_SET_HO_POLICY", WFS_ARB_SET_HO_POLICY);
    PyModule_AddIntConstant(m,"WFS_ARB_SET_TV_POLICY", WFS_ARB_SET_TV_POLICY);
    PyModule_AddIntConstant(m,"WFS_ARB_SET_AND_POINT_SOURCE_POLICY", WFS_ARB_SET_AND_POINT_SOURCE_POLICY);

}


// 
// Blocking functions callback this function in case of 
// unexpected message. The message is parsed and 
//

static int callPyFunc(MsgBuf *msgb){
    PyObject *py_message;
    
    printf("callPyFunc\n");
    py_message=ParseMessage(msgb);
    PyObject_CallMethodObjArgs(object,method, py_message);
    return 0;
}



// +Entry: PyConnectToServer

// connects to MsgD-RTDB and sends a registration message
// Returns a file descriptor for the communication socket, or an error value (<0)
// Will retry for 10 seconds before giving up.
//
// Required arguments: (1) name of client to register (string)
 
static PyObject *PyConnectToServer( PyObject *self, PyObject *args)
{
	char *name,*addr=NULL;
	int err,len;
	char *default_addr = "127.0.0.1";

	if (!PyArg_ParseTuple( args, "s|t#", &name, &addr, &len))
        return Py_BuildValue("i", PYTHON_WRONG_ARGS);

	if (!addr)
		addr = default_addr;

    if ( strlen(name) > PROC_NAME_LEN) 
        return Py_BuildValue("i", CLIENT_NAME_ERROR); 
	err = ConnectToServer( addr, name, 10);

	strcpy( clientname, name);		// Set the internal client name

	if (err==NO_ERROR)
		err = GetServerSocket();

	return Py_BuildValue("i", err);
}


static PyObject *PyCloseServer( PyObject *self, PyObject *args)
{
	char *name = NULL;
	int err;

	if (!PyArg_ParseTuple( args, "s", &name))
        return Py_BuildValue("i", PYTHON_WRONG_ARGS);	
        
    if ( strlen(name) > PROC_NAME_LEN) 
        return Py_BuildValue("i", CLIENT_NAME_ERROR); 
	err = CloseServer( name);
	
	return Py_BuildValue("i", err);
}


// +Entry: PySendMessage
 
// sends a message to a client. If the destination argument is empty,
// the message is sent to the MsgD-RTDB  
// Returns the SeqNum or an error code
//
// Required arguments: (1) destination name	(string)
//                     (2) message code		(string)
//                     (3) message data		(string)
// Optional argument : (4) seqnum           (int)

static PyObject *PySendMessage( PyObject *self, PyObject *args)
{
	MsgBuf Msg;
	char *data, *to;
	int err,len,code,sqn=-1;


	if (!PyArg_ParseTuple( args, "sis|i", &to, &code, &data, &sqn))
        return Py_BuildValue("i", PYTHON_WRONG_ARGS);

	len = strlen(data)+1;

	InitMsgBuf( &Msg, len);
    if (sqn >= 0) 
	    FillMessage( len, sqn, clientname, to, code, data, &Msg);
    else
        FillMessage( len, msg_id, clientname, to, code, data, &Msg);

	if ( (err = SendToServer(&Msg)) != NO_ERROR){
	    FreeMsgBuf(&Msg);
        return Py_BuildValue("i", err);
    }

	FreeMsgBuf(&Msg);

    err = (sqn >= 0) ? err : msg_id++;
	return Py_BuildValue("i", err);
}

// +Entry: PySendBinaryMessage
 
// sends a message to a client. If the destination argument is empty,
// the message is sent to the MsgD-RTDB  
// Returns an error string ("OK" if no error is present)
//
// Required arguments: (1) destination name	(string)
//                     (2) message code		(string)
//                     (3) message data		(array)

static PyObject *PySendBinaryMessage( PyObject *self, PyObject *args)
{
	MsgBuf Msg;
	char *to, *data;
	int err,addr,size,code;

	if (!PyArg_ParseTuple( args, "sis#", &to, &code, &addr, &size))
        return Py_BuildValue("i", PYTHON_WRONG_ARGS);

	data = (char *)addr;

	InitMsgBuf( &Msg, size);
	FillMessage( size, 0, clientname, to, code, data, &Msg);

	err = SendToServer(&Msg);
	FreeMsgBuf(&Msg);
	return Py_BuildValue("i", err);
}



// +Entry: PyCreateVar 
 
// Sends a message to the MsgD-RTDB asking the creation of a variable.
// Returns "OK" or an error string.  
//
// Required arguments: (1) variable name   (string)
//                     (2) variable type   (integer)
//                     (3) variable mode   (string)
//                     (4) variable number of elements (integer)
//                         NOTE: this must be the return value of python's len()
//                         function, that doesn't count the terminating NULL.
//                         So variable "pippo" has 5 elements. VarSize already account
//                         for the terminating NULL character.

static PyObject *PyCreateVar( PyObject *self, PyObject *args)
{
	char *name, *mode;
	int err, nelem, size, type;
    MsgBuf msgb;

	if (!PyArg_ParseTuple( args, "sisi", &name, &type, &mode, &nelem ))
       return Py_BuildValue("i", PYTHON_WRONG_ARGS);

   size = VarSize(type,nelem); // Variable size in bytes
   if (size == VAR_TYPE_ERROR)
       return Py_BuildValue("i", size);

   InitMsgBuf(&msgb, size);
   err = CreaVar( clientname, name, type, mode, nelem, &msgb);
   FreeMsgBuf(&msgb);

 	return Py_BuildValue("i", err);
}

// +Entry: PySetVar 
 
// Sends a message to the MsgD-RTDB setting the value of a variable 
//
// Required arguments: (1) variable name   (string)
//                     (2) variable type   (integer)
//                     (3) Number of elements (integer). See PyCreateVar infos
//                     (4) Tuple of elements (integer,real or string)

static PyObject *PySetVar( PyObject *self, PyObject *args)
{
	char *name;
	int err,nelem, type, i, size;
	PyObject *ob;
	void *values;
   MsgBuf msgb;

	if (!PyArg_ParseTuple( args, "siiO", &name, &type, &nelem, &ob ))
        return Py_BuildValue("i", PYTHON_WRONG_ARGS);

	size = VarSize( type, nelem);
   if (size == VAR_TYPE_ERROR)
       return Py_BuildValue("i", size);

   InitMsgBuf(&msgb, size);

	values = malloc( size);
	if (values==NULL)
		return Py_BuildValue("i", MEMORY_ALLOC_ERROR);

	if ( type == CHAR_VARIABLE)
		{
		memset( values, 0, size);
		strncpy( values, PyString_AsString( ob), PyString_Size(ob));
		}
	else
	    for (i=0; i< nelem; i++)
		{
		switch(type) {
		    case INT_VARIABLE:
			if (PySequence_Check(ob))
				((long *)values)[i] = PyInt_AsLong( PySequence_GetItem( ob, i));
			else
				((long *)values)[i] = PyInt_AsLong(ob);
			break;
		    case BIT8_VARIABLE:
			if (PySequence_Check(ob))
				((unsigned char *)values)[i] = PyInt_AsLong( PySequence_GetItem( ob, i));
			else
				((unsigned char *)values)[i] = PyInt_AsLong(ob);
			break;
		    case BIT16_VARIABLE:
			if (PySequence_Check(ob))
				((unsigned short *)values)[i] = PyInt_AsLong( PySequence_GetItem( ob, i));
			else
				((unsigned short *)values)[i] = PyInt_AsLong(ob);
			break;
		    case BIT32_VARIABLE:
			if (PySequence_Check(ob))
				((unsigned long *)values)[i] = PyLong_AsUnsignedLong( PySequence_GetItem( ob, i));
			else
				((unsigned long *)values)[i] = PyLong_AsUnsignedLong(ob);
			break;
		    case BIT64_VARIABLE:
			if (PySequence_Check(ob))
				((unsigned long long *)values)[i] = PyLong_AsUnsignedLongLong( PySequence_GetItem( ob, i));
			else
				((unsigned long long *)values)[i] = PyLong_AsUnsignedLongLong(ob);
			break;
		    case REAL_VARIABLE:
			if (PySequence_Check(ob))
				((double *)values)[i] = PyFloat_AsDouble( PySequence_GetItem( ob, i));
			else
				((double *)values)[i] = PyFloat_AsDouble( ob);
			break;
		    default:
			break;
		    }	
		}

	err = SetVar( clientname, name, type, nelem, values, &msgb);
   FreeMsgBuf(&msgb);
	free(values);

	return Py_BuildValue("i", err);
}

// +Entry: PySetRealVar 
 
// Sends a message to the MsgD-RTDB setting the value of a real variable 
//
// Required arguments: (1) variable name   (string)
//                     (2) variable value  (real)
//
// The n. of elements can be only 1. 

static PyObject *PySetRealVar( PyObject *self, PyObject *args)
{
	char *name;
	int err;
	double value;
	int type = REAL_VARIABLE;
	int length=1;

	if (!PyArg_ParseTuple( args, "sd", &name, &value))
        err = PYTHON_WRONG_ARGS;
   else
	     err = SetVar( clientname, name, type, length, &value, NULL);

	return Py_BuildValue("i", err);
}

// +Entry: PySetIntVar 
 
// Sends a message to the MsgD-RTDB setting the value of a real variable 
//
// Required arguments: (1) variable name   (string)
//                     (2) variable value  (int)
//
// The n. of elements can be only 1. 

static PyObject *PySetIntVar( PyObject *self, PyObject *args)
{
	char *name;
	int err;
	int value;
	int type = INT_VARIABLE;
	int length=1;

	if (!PyArg_ParseTuple( args, "si", &name, &value))
        return Py_BuildValue("i", PYTHON_WRONG_ARGS);

	err = SetVar( clientname, name, type, length, &value, NULL);

	return Py_BuildValue("i", err);
}


// +Entry: PyGetVar 
// 
// Sends a message to the MsgD-RTDB asking the value of a variable 
// Returns a (integer) message identifier which will be used by the MsgD-RTDB in its response
// or a negative error code.
//
// Required arguments: (1) variable name   (string)

static PyObject *PyGetVar( PyObject *self, PyObject *args)
{
    
	char *name;
	int err;
	MsgBuf msgb;

	if (!PyArg_ParseTuple( args, "s", &name))
        return Py_BuildValue("i", PYTHON_WRONG_ARGS);

	InitMsgBuf( &msgb,0);
	err = GetVar( clientname, name, msg_id, &msgb);
   FreeMsgBuf(&msgb);
	if (err != NO_ERROR)
        return Py_BuildValue("i", err);

    return Py_BuildValue("i", msg_id++);
}

//@Function{API}: ReadVar
//
// Send request to get a variable value and waits for the reply
//
// This routine sends a request to get the value of a variable from the 
// real time database, then waits for a reply and returns the variable value

// A more flexible (and complicated) managements of variable retrieval
// can be performed by calling GetVar() and then managing returned messages
//@
/*
 * Cosi' e' un po' scomoda: se non so il numero di elementi? 
 * Fa comodo una GetVarSize da chiamare prima della ReadVar.
 *
static PyObject *PyReadVar( PyObject *self, PyObject *args)
{
	char *name;
	int err,timeout=0;
	MsgBuf msgb;

	if (!PyArg_ParseTuple( args, "s|i", &name,&type,&len,&timeout))
		return NULL;


	InitMsgBuf( &msgb,0);
    err = ReadVar(clientname, name, , , timeout, NULL,&msgb);
    FreeMsgBuf(&msgb);
	return Py_BuildValue("i", msg_id-1);
}
*/
//@Function{API}: pyVarSize
//
// Returns the size of a variable
//
// This function returns the minimum size of a buffer which must contain
// a variable of given type and length.
//
// Required arguments: (1) variable type        (integer)
//                     (2) number of elements   (integer)
//@
//
static PyObject *PyVarSize( PyObject *self, PyObject *args)
{
    int err, type, nel;

    if (!PyArg_ParseTuple( args, "ii", &type,&nel))
        return Py_BuildValue("i", PYTHON_WRONG_ARGS);

   err = VarSize(type,nel);
	return Py_BuildValue("i", err);
}
 



// +Entry: PyVarNotif 
// 
// Sends a message to the MsgD-RTDB asking to be notified when a variable changes.
// If the optional second parameter is nonzero, it will also send a GetVar request
//
// Required arguments: (1) variable name   (string)
// Optional arguments: (2) getvar flag (integer) - if nonzero, performs a GetVar()

static PyObject *PyVarNotif( PyObject *self, PyObject *args)
{
	char *varname;
	int stat, get=0;
	MsgBuf msgb;

	if (!PyArg_ParseTuple( args, "s|i", &varname, &get))
        return Py_BuildValue("i", PYTHON_WRONG_ARGS);

	InitMsgBuf( &msgb,0);

	if (get){	
		stat = GetVar( clientname, varname, msg_id++, &msgb);
		if (stat != NO_ERROR){
            FreeMsgBuf(&msgb);
			return Py_BuildValue("i", stat);
		}
    }

	stat = VarNotif( clientname, varname);
    FreeMsgBuf(&msgb);
	return Py_BuildValue("i", stat);
}

// +Entry: PyVarNoNotif 
// 
// Sends a message to the MsgD-RTDB asking to not be notified anymore 
// when a variable changes.
// Required arguments: (1) variable name   (string)

static PyObject *PyVarNoNotif( PyObject *self, PyObject *args)
{
	char *varname;
	int stat;

	if (!PyArg_ParseTuple( args, "s", &varname))
        return Py_BuildValue("i", PYTHON_WRONG_ARGS);

	stat = VarNoNotif( clientname, varname);
	return Py_BuildValue("i", stat);
}


// +Entry: PyVarList 
// 
// Sends a message to the MsgD-RTDB asking the value of a list of variables
// whose name match the string prefix
// Seqnum (aka msg_id) is used by msgd in the response to a VARLIST msg:
// every varreply response use this msgd. 
//
// Required arguments: (1) prefix    (string) Variable name specification. If terminated
//                         with '*' all variables with given prefix are selected,
//                         otherwise requires exact name matching

static PyObject *PyVarList( PyObject *self, PyObject *args)
{
	char *prefix;
	int err;
	MsgBuf msgb;

	if (!PyArg_ParseTuple( args, "s", &prefix))
        return Py_BuildValue("i", PYTHON_WRONG_ARGS);

	InitMsgBuf( &msgb,0);
	err = VarList( clientname, prefix, msg_id, &msgb);
    FreeMsgBuf(&msgb);
	if (err != NO_ERROR)
		return Py_BuildValue("i", err);

	return Py_BuildValue("i", msg_id++);
}

// +Entry: PyDelVar 
// 
// Sends a message to the MsgD-RTDB asking to delete a variable 
// Returns a (integer) message identifier which will be used by the MsgD-RTDB in its response
//
// Required arguments: (1) variable name   (string)
// Optional arguments: (2) timeout  (integer) - timeout [ms]. If 0 wait forever

static PyObject *PyDelVar( PyObject *self, PyObject *args)
{
	char *name;
	int err, timeout=0;
	MsgBuf msgb;

    if (!PyArg_ParseTuple( args, "sOO|i", &name, &object, &method, &timeout))
        return Py_BuildValue("i", PYTHON_WRONG_ARGS);

	InitMsgBuf( &msgb,0);
    err = DelVar(clientname, name, timeout, callPyFunc, &msgb); 
    FreeMsgBuf(&msgb);

	return Py_BuildValue("i", err);

}


//@Function{API}: PyDumpVars
//
// Send a request to Dump a list of variables
//
// This routine sends a request to dump to log system the values of a set
// of variables variables whose names match a given prefix.  
//
// No reply is expected from \msgd\.
//
// Required arguments: (1) variable name   (string) if terminated by '*', it is used as prefix
//                                                  for name matching.
//
//@
static PyObject *PyDumpVars( PyObject *self, PyObject *args)
{
    char *name;
    int err;
    MsgBuf msgb;

    if (!PyArg_ParseTuple( args, "s", &name))
        return Py_BuildValue("i", PYTHON_WRONG_ARGS);

    InitMsgBuf( &msgb,0);
    err = DumpVars(clientname, name, &msgb);
    FreeMsgBuf(&msgb);
	 return Py_BuildValue("i", err);
}

//@Function{API}: LockVars
//
// Send request to lock a list of variables
//
// This routine sends a request to lock a set of variables whose names 
// match a given prefix.  
//
// The \msgd\  replies with an acknowledge message (either ACK or NAK)
// 
// Required arguments: (1) variable name   (string) if terminated by '*', it is used as prefix
//                                                  for name matching.
//@

static PyObject *PyLockVars( PyObject *self, PyObject *args)
{
    char *prefix;
    int err,timeout=0;
    MsgBuf msgb;

    if (!PyArg_ParseTuple( args, "sOO|i", &prefix, &object, &method, &timeout))
    //if (!PyArg_ParseTuple( args, "s|i", &prefix, &timeout))
        return Py_BuildValue("i", PYTHON_WRONG_ARGS);

    InitMsgBuf( &msgb,0);
    err = LockVars(clientname, prefix,  timeout, callPyFunc, &msgb); 
    FreeMsgBuf(&msgb);
	 return Py_BuildValue("i", err);
}

//@Function{API}: UnlockVars
//
// Send request to unlock a list of variables
//
// This routine sends a request to unlock a set of variables whose names 
// match a given prefix.  
//
// The \msgd\  replies with an acknowledge message (either ACK or NAK)
// 
// Required arguments: (1) variable name   (string) if terminated by '*', it is used as prefix
//                                                  for name matching.
//@
static PyObject *PyUnlockVars( PyObject *self, PyObject *args)
{
    char *prefix;
    int err,timeout=0;
    MsgBuf msgb;

    if (!PyArg_ParseTuple( args, "sOO|i", &prefix, &object, &method, &timeout))
    //if (!PyArg_ParseTuple( args, "s|i", &prefix, &timeout))
        return Py_BuildValue("i", PYTHON_WRONG_ARGS);

    InitMsgBuf( &msgb,0);
    err = UnlockVars(clientname, prefix,  timeout, callPyFunc, &msgb); 
    FreeMsgBuf(&msgb);
	 return Py_BuildValue("i", err);
}



//@Function{API}: WriteVar
//
// Write a variable. Create the variable if not existent. Verify completion
//
// This routine sends a request to write a variable in the real-time database.
// then waits for a completion code. If the variable is not defined, it is
// created.
//
// A more flexible (and complicated) managements of variable retrieval
// can be performed by calling SetVar() and then managing returned messages
//
// Required arguments: (1) variable name   (string)
//                     (2) variable type   (integer)
//                     (3) Number of elements (integer)
//                     (4) Tuple of elements (integer,real or string)
//                     (5)
//                     (6)
// Optional arguments: (7) timeout (integer) [ms]
//
//@
static PyObject *PyWriteVar( PyObject *self, PyObject *args)
{
	char *name;
	int err,length, type, i, size, timeout=0;
	PyObject *ob;
	void *values;
    MsgBuf msgb;

    if (!PyArg_ParseTuple( args, "siiOOO|i", &name, &type, &length,&ob,
                &object, &method, &timeout))
        return Py_BuildValue("i", PYTHON_WRONG_ARGS);

    InitMsgBuf( &msgb,0);
	size = VarSize( type, length);
   if (size == VAR_TYPE_ERROR)
       return Py_BuildValue("i", size);

	values = malloc( size);
	if (values==NULL)
		return Py_BuildValue("i", MEMORY_ALLOC_ERROR);

	if ( type == CHAR_VARIABLE){
	    memset( values, 0, size);
		strncpy( values, PyString_AsString( ob), PyString_Size(ob));
	} else { 
        for (i=0; i< length; i++){
		    switch(type) {
            case INT_VARIABLE:
                if (PySequence_Check(ob))
                    ((long *)values)[i] = PyInt_AsLong( PySequence_GetItem( ob, i));
                else
                    ((long *)values)[i] = PyInt_AsLong(ob);
                break;
            case BIT8_VARIABLE:
                if (PySequence_Check(ob))
                    ((unsigned char *)values)[i] = PyInt_AsLong( PySequence_GetItem( ob, i));
                else
                    ((unsigned char *)values)[i] = PyInt_AsLong(ob);
                break;
            case BIT16_VARIABLE:
                if (PySequence_Check(ob))
                    ((unsigned short *)values)[i] = PyInt_AsLong( PySequence_GetItem( ob, i));
                else
                    ((unsigned short *)values)[i] = PyInt_AsLong(ob);
                break;
		    case BIT32_VARIABLE:
                if (PySequence_Check(ob))
                    ((unsigned long *)values)[i] = PyLong_AsUnsignedLong( PySequence_GetItem( ob, i));
                else
                    ((unsigned long *)values)[i] = PyLong_AsUnsignedLong(ob);
                break;
		    case BIT64_VARIABLE:
                if (PySequence_Check(ob))
                    ((unsigned long long *)values)[i] = PyLong_AsUnsignedLongLong( PySequence_GetItem( ob, i));
                else
                    ((unsigned long long *)values)[i] = PyLong_AsUnsignedLongLong(ob);
                break;
		    case REAL_VARIABLE:
                if (PySequence_Check(ob))
                    ((double *)values)[i] = PyFloat_AsDouble( PySequence_GetItem( ob, i));
                else
                    ((double *)values)[i] = PyFloat_AsDouble( ob);
                break;
		    default:
                break;
		    }	
		}
    }
    err = WriteVar(clientname, name, type, length, values,
            timeout, callPyFunc, &msgb); 
    free(values);
    FreeMsgBuf(&msgb);
	 return Py_BuildValue("i", err);
}





// +Entry: PyWaitMessage

// Waits for a message from MsgD-RTDB.
// Returns a dictionary with status, sender, code and body elements
// Status is either "OK" or an error code
//
// No arguments required

static PyObject *PyWaitMessage( PyObject *self, PyObject *args)
{
	MsgBuf Msg;
	int err,
        timeout = 0; //[ms]
	char *status;
	PyObject *py_return;

	if (!PyArg_ParseTuple( args, "|i", &timeout))
		return NULL;

    InitMsgBuf( &Msg, 0);

	if ((err = WaitMsgTimeout(&Msg, timeout)) != NO_ERROR){
		FreeMsgBuf(&Msg);
		status = (char *) lao_strerror(err);
      return Py_BuildValue("{s:s,s:i,s:s,s:s,s:s}", "status", status, "errcode", err, "sender", "", "code", "", "body", "");
    }
    py_return = ParseMessage(&Msg);
	 FreeMsgBuf(&Msg);
    return py_return;
}


static PyObject * ParseMessage(MsgBuf* Msg){
	int bodysize, type, seqnum, len, i;
	char *status, *body;
	Variable *TheVar;
	PyObject *py_object, *py_return;
    idl_answer answer; 

	status = "NO_ERROR";
	bodysize = HDR_LEN(Msg) +1;


	// If the message is malformed, return an empty array
    if ((bodysize > MAX_MSG_LEN) || (bodysize<0))
		{
		status = "Malformed message";
		return Py_BuildValue("{s:s,s:s,s:s,s:s}", "status", status, "sender", "", "code", "", "body", "");  
		}

	body = malloc( bodysize);
	memset( body, 0, bodysize);
	strncpy( body, (char*)MSG_BODY(Msg), bodysize-1);
 
	seqnum = HDR_SEQN(Msg);

    //printf("PyMsgLib: seqnum %d - Len %d - BufLen %d - Code %d - From %s - To %s - Body %s\n",
    //        seqnum, hdr->Len, Msg->buflen, hdr->Code, hdr->From, hdr->To, body);

    if ( HDR_CODE(Msg) == VARLOCKD || HDR_CODE(Msg) == VARERROR || HDR_CODE(Msg) == ECHO) {
		py_return = Py_BuildValue("{s:s,s:s,s:i,s:i}", "status", status, "sender", HDR_FROM(Msg),
                "code", HDR_CODE(Msg), "seqnum", seqnum);
    }
//    else if ( HDR_CODE(Msg) == VARREMOVD){
//		py_return = Py_BuildValue("{s:s,s:s,s:i,s:s,s:i}", "status", status, "sender", HDR_FROM(Msg),
//                "code", HDR_CODE(Msg),"name", MSG_BODY(Msg), "seqnum", seqnum);
//    }
    else if ( HDR_CODE(Msg) == VARREPLY || HDR_CODE(Msg) == VARCHANGD) 
		{
		TheVar = (Variable *)MSG_BODY(Msg);
		type = TheVar->H.Type;
		len = TheVar->H.NItems;

		if (type == CHAR_VARIABLE)
			{
         // Length may include the zero termination. Reduce by 1 in this case
			if (TheVar->Value.Sv[len-1] == 0)
		  		len = strlen(TheVar->Value.Sv);
			py_object = PyString_FromStringAndSize( TheVar->Value.Sv, len);
			}
		else
			{
			if ((type == INT_VARIABLE ) && (len==1) )
				py_object = PyInt_FromLong( TheVar->Value.Lv[0]);

            else if ((type == BIT8_VARIABLE) && (len==1))
				py_object = PyInt_FromLong( TheVar->Value.B8[0]);

            else if ((type == BIT16_VARIABLE) && (len==1))
				py_object = PyInt_FromLong( TheVar->Value.B16[0]);

			else if ((type == BIT32_VARIABLE) && (len==1))
				py_object = PyLong_FromUnsignedLong( TheVar->Value.B32[0]);

			else if ((type == BIT64_VARIABLE) && (len==1))
				py_object = PyLong_FromUnsignedLongLong( TheVar->Value.B64[0]);

			else if ((type == REAL_VARIABLE) && (len==1))
				py_object = PyFloat_FromDouble( TheVar->Value.Dv[0]);

			else
				{
				py_object = PyTuple_New(len);
				for ( i=0; i< len; i++)
					switch(type)
						{
						case INT_VARIABLE:
						PyTuple_SetItem( py_object, i, PyInt_FromLong( TheVar->Value.Lv[i]));
						break;

						case BIT8_VARIABLE:
						PyTuple_SetItem( py_object, i, PyInt_FromLong( TheVar->Value.B8[i]));
						break;
	
						case BIT16_VARIABLE:
						PyTuple_SetItem( py_object, i, PyInt_FromLong( TheVar->Value.B16[i]));
						break;

						case BIT32_VARIABLE:
						PyTuple_SetItem( py_object, i, PyLong_FromUnsignedLong( TheVar->Value.B32[i]));
						break;

						case BIT64_VARIABLE:
						PyTuple_SetItem( py_object, i, PyLong_FromUnsignedLongLong( TheVar->Value.B64[i]));
						break;

						case REAL_VARIABLE:
						PyTuple_SetItem( py_object, i, PyFloat_FromDouble( TheVar->Value.Dv[i]));
						break;

						default:
						PyTuple_SetItem( py_object, i, PyInt_FromLong(0));
						break;
						}
				}
			}
		py_return = Py_BuildValue("{s:s,s:s,s:i,s:s,s:s,s:i,s:i,s:i,s:O}", "status", status, "sender", HDR_FROM(Msg),
                "code", HDR_CODE(Msg),"name", TheVar->H.Name, "owner", TheVar->H.Owner, "seqnum", seqnum,
                "type", type, "mtime", TheVar->H.MTime.tv_sec, "value", py_object);
		Py_DECREF(py_object);
		}	
    /*
    else if( HDR_CODE(Msg) == IDLREPLY){
        PrepareIDLAnswer(&answer); 
        if (GetIDLAnswer(Msg, &answer) == NO_ERROR){
		    py_return = Py_BuildValue("{s:s,s:s,s:i,s:i,s:s,s:s,s:i}", "status", status, "sender", HDR_FROM(Msg),
                "code", HDR_CODE(Msg), "seqnum", seqnum, "stdout",answer.output, "stderr", answer.error, 
                "err_code",answer.error_code);
        }
        FreeIDLAnswer(&answer);
    }
    */
    else if (HDR_CODE(Msg) == CLNREADY){
		py_return = Py_BuildValue("{s:i,s:s,s:s,s:i}", "seqnum", seqnum, "status", status, "sender", 
                HDR_FROM(Msg), "code", HDR_CODE(Msg));  
    }
	else 
		{
		len = HDR_LEN(Msg);
		py_object = Py_BuildValue("s#", body, len);

		py_return = Py_BuildValue("{s:i,s:s,s:s,s:i,s:O}", "seqnum", seqnum, "status", status, "sender", 
                HDR_FROM(Msg), "code", HDR_CODE(Msg), "body", py_object);  
		Py_DECREF(py_object);
		}

	free(body);

	return py_return; 
}

// +Entry: PyGetErrorStringpy_object 

// Returns the error string corresponding to an error number 
//
// Required arguments: (1) error number		(integer) 

static PyObject *PyGetErrorString( PyObject *self, PyObject *args)
{
	int err;

	if (!PyArg_ParseTuple( args, "i", &err))
		return NULL;

	return Py_BuildValue("s", lao_strerror(err));
}



 
// +Entry: PyINT_VARIABLE

// Returns the value corresponding to INT_VARIABLE
//
// Required arguments: none

static PyObject *PyINT_VARIABLE( PyObject *self, PyObject *args)
{
	if (!PyArg_ParseTuple( args, ""))
		return NULL;

	return Py_BuildValue("i", INT_VARIABLE);
}

// +Entry: PyREAL_VARIABLE

// Returns the value corresponding to REAL_VARIABLE
//
// Required arguments: none

static PyObject *PyREAL_VARIABLE( PyObject *self, PyObject *args)
{
	if (!PyArg_ParseTuple( args, ""))
		return NULL;

	return Py_BuildValue("i", REAL_VARIABLE);
}

// +Entry: PyCHAR_VARIABLE

// Returns the value corresponding to CHAR_VARIABLE
//
// Required arguments: none

static PyObject *PyCHAR_VARIABLE( PyObject *self, PyObject *args)
{
	return Py_BuildValue("i", CHAR_VARIABLE);
}

// +Entry: PyBIT8_VARIABLE

// Returns the value corresponding to BIT8_VARIABLE
//
// Required arguments: none

static PyObject *PyBIT8_VARIABLE( PyObject *self, PyObject *args)
{
	return Py_BuildValue("i", BIT8_VARIABLE);
}

// +Entry: PyBIT16_VARIABLE

// Returns the value corresponding to BIT16_VARIABLE
//
// Required arguments: none

static PyObject *PyBIT16_VARIABLE( PyObject *self, PyObject *args)
{
	return Py_BuildValue("i", BIT16_VARIABLE);
}


// +Entry: PyBIT32_VARIABLE

// Returns the value corresponding to BIT32_VARIABLE
//
// Required arguments: none

static PyObject *PyBIT32_VARIABLE( PyObject *self, PyObject *args)
{
	return Py_BuildValue("i", BIT32_VARIABLE);
}


// +Entry: PyBIT64_VARIABLE

// Returns the value corresponding to BIT64_VARIABLE
//
// Required arguments: none

static PyObject *PyBIT64_VARIABLE( PyObject *self, PyObject *args)
{
	return Py_BuildValue("i", BIT64_VARIABLE);
}




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

   if (debug_output) printf("%s requesting buf %s\n", clientname, bufname);
   stat = bufRequest(clientname, bufname, BUFCONT, bufinfo);

   if (debug_output) printf("bufRequest replied %d\n",stat);

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
   int timeout=100; //ms
   BufInfo *bufinfo;
   void *data;


   if (!PyArg_ParseTuple( args, "s#|i", &bufinfo, &len, &timeout))
     return NULL;

   if (debug_output) printf("Got bufinfo, len = %d\n", bufinfo->com.lng);

   data = (void*) malloc((size_t)bufinfo->com.lng);
   stat = bufRead( bufinfo, data, timeout);
    
   if (debug_output) printf("Read frame %d buf len %d timeout %d \n", stat, bufinfo->com.lng, timeout);
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
   free(data); 

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

   if (debug_output) printf("%s creating buf %s\n", clientname, bufname);
   stat = bufCreate(clientname, bufname, BUFCONT, bufsize, maxcons, bufinfo);
   if (stat>=0)
      stat=0;

   if (debug_output) printf("bufCreate replied %d\n",stat);

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

   if (debug_output) printf("Got bufinfo, len = %d\n", bufinfo->com.lng);

   stat = bufWrite( bufinfo, data, counter, timeout);
    
   if (debug_output) printf("Written frame %d buf len %d timeout %d \n", counter, bufinfo->com.lng, timeout);

   return Py_BuildValue("i", stat);
}








