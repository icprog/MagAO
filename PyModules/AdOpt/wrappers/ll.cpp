//+File ll.c
//
// Low-level functions for Python scripts
//-


// TO BE WRITTEN:
// SetTupleVariable() to set array variables using tuples, like SetSingleVariable()

extern "C" {
#include <Python.h>
#include "numarray/libnumarray.h"


#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>	// dup()

#include <sys/time.h>	// gettimeofday()

//#include "Numeric/arrayobject.h"


#include "aotypes.h"

#include "msglib.h"
#include "BCUmsg.h"

#include "commlib.h"
#include "shmlib.h"
}

#include "BcuCommon.h"
//#include "convert.h"

// Debug output
static int debug=0;

// Global variables
int global_seqnum=0;

#define GETPIXELS	(1)
#define GETSLOPES	(2)
#define GETCOMMANDS	(3)

#define STATUS_MOVING	(0x002)

#define LL_TIMEOUT	(2000)

// We've got a complete diagnostic frame (!)

//#define HAS_SLOPES_IN_SLOPECOMPUTER
//#include "bcucommand.h"

// Utility functions

Variable *LL_GetVar( MsgBuf *msg, char *format, ...);
PyObject *LL_MakeIntTuple( int len, int *array);
PyObject *LL_MakeDoubleTuple( int len, double *array);
int LL_WaitMessage( int seqnum, MsgBuf *msg);
int LL_HandleMessage( MsgBuf *msg);
PyObject *SetSingleVariable( PyObject *args, char *device_type, char *variable_name, int type);
PyObject *SetTupleVariable( PyObject *args, char *device_type, char *variable_name, int type);
PyObject *GetSingleVariable( PyObject *args, char *device_type, char *variable_name, int type);
PyObject *GetTupleVariable( PyObject *args, char *device_type, char *variable_name, int type);

extern "C" {

// Module functions
	
void initll( void);

// Msg-D connection functions

static PyObject *PyConnectToMsgD( PyObject *, PyObject *);
//static PyObject *PyDisconnectFromMsgD( PyObject *, PyObject *);

// CCD functions

static PyObject *PyCCD_GetProfile( PyObject *, PyObject *);
static PyObject *PyCCD_SetReadoutSpeed( PyObject *, PyObject *);
static PyObject *PyCCD_SetRepetitions( PyObject *, PyObject *);
static PyObject *PyCCD_SetBlackLevels( PyObject *, PyObject *);
static PyObject *PyCCD_SetBinning( PyObject *, PyObject *);
static PyObject *PyCCD_SetRunState( PyObject *, PyObject *);
static PyObject *PyCCD_GetFrames( PyObject *, PyObject *);
static PyObject *PyCCD_PrepareGetCurFrame( PyObject *, PyObject *);
static PyObject *PyCCD_GetCurFrame( PyObject *, PyObject *);
static PyObject *PyCCD_GetPreviousFrame( PyObject *, PyObject *);
static PyObject* PyCCD_GetCompleteFrame(PyObject*, PyObject*);
static PyObject *PyCCD_GetReadoutSpeed( PyObject *, PyObject *);
static PyObject *PyCCD_GetBinning( PyObject *, PyObject *);
static PyObject *PyCCD_GetBlackLevels( PyObject *, PyObject *);
static PyObject *PyCCD_GetTemperatures( PyObject *, PyObject *);
static PyObject *PyCCD_GetErrorMsg( PyObject *, PyObject *);

static PyObject *PyWFS_SetRunState( PyObject *, PyObject *);
static PyObject *PyWFS_SetPixelLUT( PyObject *, PyObject *);
static PyObject *PyWFS_SetNsubap( PyObject *, PyObject *);
static PyObject *PyWFS_GetNsubap( PyObject *, PyObject *);
static PyObject *PyWFS_SetSlopeOffsets( PyObject *, PyObject *);
static PyObject *PyWFS_SetDigitalNoise( PyObject *, PyObject *);
static PyObject *PyWFS_EnableDigitalNoise( PyObject *, PyObject *);
static PyObject *PyRTR_SetREC( PyObject *, PyObject *);
static PyObject *PyRTR_SetLoopState( PyObject *, PyObject *);
static PyObject *PyRTR_SetNsubap( PyObject *, PyObject *);

// static PyObject *PyBCU_Read( PyObject *, PyObject *);

static PyObject *PyMOV_GetProfile( PyObject *, PyObject *);
static PyObject *PyMOV_GetCurPos( PyObject *, PyObject *);
static PyObject *PyMOV_MoveTo( PyObject *, PyObject *);
static PyObject *PyMOV_GetErrorMsg( PyObject *, PyObject *);
static PyObject *PyMOV_IsMoving( PyObject *, PyObject *);

static PyObject *PyOKO_GetFlat( PyObject *, PyObject *);

static PyObject *PySendBinaryMessage( PyObject *, PyObject *);
//static PyObject *PySetVar( PyObject *, PyObject *);
}


// Registrazione dei metodi chiamabili da Python

static struct PyMethodDef msglib_methods[] = {
    {"ConnectToMsgD", PyConnectToMsgD, METH_VARARGS, "Connects and registers itself to the MsgD-RTDB"},
//    {"DisconnectFromMsgD", PyDisconnectFromMsgD, METH_VARARGS, "Disconnects from MsgD-RTDB"},

    {"CCD_GetProfile", PyCCD_GetProfile, METH_VARARGS, "Gets the CCD profile"},
    {"CCD_SetReadoutSpeed", PyCCD_SetReadoutSpeed, METH_VARARGS, "Sets the CCD readout speed"},
    {"CCD_SetRepetitions", PyCCD_SetRepetitions, METH_VARARGS, "Sets the CCD repetition number"},
    {"CCD_SetBlackLevels", PyCCD_SetBlackLevels, METH_VARARGS, "Sets the CCD black levels"},
    {"CCD_SetBinning", PyCCD_SetBinning, METH_VARARGS, "Sets the CCD binning"},
    {"CCD_SetRunState", PyCCD_SetRunState, METH_VARARGS, "Sets the CCD run state"},
    {"CCD_GetReadoutSpeed", PyCCD_GetReadoutSpeed, METH_VARARGS, "Gets the CCD readout speed"},
    {"CCD_GetBinning", PyCCD_GetBinning, METH_VARARGS, "Gets the CCD binning"},
    {"CCD_GetFrames", PyCCD_GetFrames, METH_VARARGS, "Get frames from the CCD"},
    {"CCD_PrepareGetCurFrame", PyCCD_PrepareGetCurFrame, METH_VARARGS, "Get frames from the CCD"},
    {"CCD_GetCurFrame", PyCCD_GetCurFrame, METH_VARARGS, "Get frames from the CCD"},
    {"CCD_GetPreviousFrame", PyCCD_GetPreviousFrame, METH_VARARGS, "Get frames from the CCD"},
    {"CCD_GetCompleteFrame", PyCCD_GetCompleteFrame, METH_VARARGS, "Get frames from the CCD"},
    {"CCD_GetTemperatures", PyCCD_GetTemperatures, METH_VARARGS, "Gets the CCD temperatures"},
    {"CCD_GetBlackLevels", PyCCD_GetBlackLevels, METH_VARARGS, "Gets the CCD black levels"},
    {"CCD_GetErrorMsg", PyCCD_GetErrorMsg, METH_VARARGS, "Gets the CCD error message"},

    {"WFS_SetRunState", PyWFS_SetRunState, METH_VARARGS, "Starts or stops the WFS BCU"},
    {"WFS_SetPixelLUT", PyWFS_SetPixelLUT, METH_VARARGS, "Sets a new Pixel Lookup table on the WFS BCU"},
    {"WFS_SetNsubap", PyWFS_SetNsubap, METH_VARARGS, "Sets a new number of subapertures on the WFS BCU"},
    {"WFS_GetNsubap", PyWFS_GetNsubap, METH_VARARGS, "Gets the number of subapertures on the WFS BCU"},
    {"WFS_SetSlopeOffsets", PyWFS_SetSlopeOffsets, METH_VARARGS, "Sets a new slope offsets pattern"},
    {"WFS_SetDigitalNoise", PyWFS_SetDigitalNoise, METH_VARARGS, "Sets a new digital noise pattern"},
    {"WFS_EnableDigitalNoise", PyWFS_EnableDigitalNoise, METH_VARARGS, "Enables/disables digital noise"},

    {"RTR_SetREC", PyRTR_SetREC, METH_VARARGS, "Sets a new reconstruction matrix"},
    {"RTR_SetNsubap", PyRTR_SetNsubap, METH_VARARGS, "Sets a new number of subapertures on the RTR BCU"},
    {"RTR_SetLoopState", PyRTR_SetLoopState, METH_VARARGS, "Opens or closes the RTR loop"},

    {"OKO_GetFlat", PyOKO_GetFlat, METH_VARARGS, "Gets the flat commands for the given OKO"},

//     {"BCU_Read", PyBCU_Read, METH_VARARGS, "Reads memory from BCU"},

    {"MOV_GetProfile", PyMOV_GetProfile, METH_VARARGS, "Gets the movement profile"},
    {"MOV_GetCurPos", PyMOV_GetCurPos, METH_VARARGS, "Gets the current position"},
    {"MOV_MoveTo", PyMOV_MoveTo, METH_VARARGS, "Move to the specified position"},
    {"MOV_GetErrorMsg", PyMOV_GetErrorMsg, METH_VARARGS, "Gets the current error message"},
    {"MOV_IsMoving", PyMOV_IsMoving, METH_VARARGS, "Checks if the motor is moving"},

    {"SendBinaryMessage", PySendBinaryMessage, METH_VARARGS, "Sends a message to MSGD-RTDB"},

    {NULL, NULL, 0, NULL}
};


char clientname[ PROC_NAME_LEN+1];		// Name of this client
int shm_ok = 0;                         // shared memory ok flag
BufInfo bufinfo;

//+Function: initll
//
// Function called by python at importing time
//
// This function is the "init" module function, and is automatically called
// by Python when importing the module.
// It initializes the module functions and zeroes out the client name
//-

void initll()
{
    (void) Py_InitModule("ll", msglib_methods);
    memset( clientname, 0, PROC_NAME_LEN+1);
//    import_array();
    import_libnumarray();
}



//+Function: PyConnectToMsgD
//
// connects to MsgD-RTDB and sends a registration message
// 
// Returns a file descriptor for the communication socket, or an error value (<0)
// Will retry for 10 seconds before giving up.
//
// Required arguments: (1) name of client to register (string)
//-

static PyObject *PyConnectToMsgD( PyObject *self, PyObject *args)
{
	char *name, *address;
	int err;

	if (!PyArg_ParseTuple( args, "ss", &name, &address))
		return NULL;

	err = ConnectToServer( address, name, 10);

	strcpy( clientname, name);		// Set the internal client name

	if (err==NO_ERROR)
		err = GetServerSocket();

	return Py_BuildValue("i", err);
}

//+Function: PyCCD_GetProfile
//
// Gets the CCD profile
//
// returns a python dictionary with the CCD complete status
// 
// Required arguments: (1) ccd number (int)
//-

static PyObject *PyCCD_GetProfile( PyObject *self, PyObject *args)
{
	Variable *TheVar;
	MsgBuf Msg;

	int ccdnum, len;
	int dx, dy, cur_speed, cur_xbin, cur_ybin, status_code;
	char errmsg[64], name[64];
	PyObject *xbins, *ybins, *speeds, *blacks, *temps, *py_return;

	if (!PyArg_ParseTuple( args, "i", &ccdnum))
		return NULL;

	InitMsgBuf(&Msg, 0);

	TheVar = LL_GetVar( &Msg, "CCD%02dDX", ccdnum);
	dx = TheVar->Value.Lv[0];

	TheVar = LL_GetVar( &Msg, "CCD%02dDY", ccdnum);
	dy = TheVar->Value.Lv[0];

	TheVar = LL_GetVar( &Msg, "CCD%02dSPEED", ccdnum);
	cur_speed = TheVar->Value.Lv[0];

	TheVar = LL_GetVar( &Msg, "CCD%02dSTATUS", ccdnum);
	status_code = TheVar->Value.Lv[0];

	TheVar = LL_GetVar( &Msg, "CCD%02dXBIN", ccdnum);
	cur_xbin = TheVar->Value.Lv[0];

	TheVar = LL_GetVar( &Msg, "CCD%02dYBIN", ccdnum);
	cur_ybin = TheVar->Value.Lv[0];

	TheVar = LL_GetVar( &Msg, "CCD%02dERRMSG", ccdnum);
	strcpy( errmsg, TheVar->Value.Sv);

	TheVar = LL_GetVar( &Msg, "CCD%02dNAME", ccdnum);
	strcpy( name, TheVar->Value.Sv);

	TheVar = LL_GetVar( &Msg, "CCD%02dXBINS", ccdnum);
	len = TheVar->H.NItems;
	xbins = LL_MakeIntTuple( len, (int *)(TheVar->Value.Lv));

	TheVar = LL_GetVar( &Msg, "CCD%02dYBINS", ccdnum);
	len = TheVar->H.NItems;
	ybins = LL_MakeIntTuple( len, (int *)(TheVar->Value.Lv));

	TheVar = LL_GetVar( &Msg, "CCD%02dYSPEEDS", ccdnum);
	len = TheVar->H.NItems;
	speeds = LL_MakeIntTuple( len, (int *)(TheVar->Value.Lv));

	TheVar = LL_GetVar( &Msg, "CCD%02dTEMPS", ccdnum);
	len = TheVar->H.NItems;
	temps = LL_MakeIntTuple( len, (int *)(TheVar->Value.Lv));

	TheVar = LL_GetVar( &Msg, "CCD%02dBLACKS", ccdnum);
	len = TheVar->H.NItems;
	blacks = LL_MakeIntTuple( len, (int *)(TheVar->Value.Lv));

	py_return = Py_BuildValue("{s:i, s:i, s:i, s:i, s:i, s:i, s:s, s:s, s:O, s:O, s:O, s:O, s:O}",
					"xdim", dx, "ydim", dy, "cur_speed", cur_speed, "status_code", status_code,
					"cur_xbin", cur_xbin, "cur_ybin", cur_ybin, "error_msg", errmsg,
					"name", name, "avail_xbins", xbins, "avail_ybins", ybins, "avail_speeds", speeds,
					"temperatures", temps, "black_levels", blacks);

	FreeMsgBuf( &Msg);

	Py_DECREF(xbins);
	Py_DECREF(ybins);
	Py_DECREF(speeds);
	Py_DECREF(temps);
	Py_DECREF(blacks);

	return py_return;
}

//+Function LL_GetVar
//
// Low level routine to issue a GetVar command and wait for the answer
// Needs an already set up MsgBuf
//-

Variable *LL_GetVar( MsgBuf *msg, char *format, ...)
{
	char varname[ VAR_NAME_LEN+1];
	va_list argp;

	va_start( argp, format);
	vsprintf( varname, format, argp);
	va_end(argp);

    if (debug) printf("Sending getvar (%s), client=%s, seqnum=%d, ptr=%d\n", varname, clientname, global_seqnum, (unsigned int)msg);
	GetVar( clientname, varname, global_seqnum, msg);
    if (debug) printf("Done, waiting\n");
	LL_WaitMessage( global_seqnum++, msg );

    if (debug) printf("Got answer\n");

	if (msg->bufmem->Hdr.Code == VARERROR)
        {
        printf("VARERROR\n");
		return (Variable *) (MSG_BODY(msg));	// Here we should return an invalid variable
        }
	else
		return (Variable *) (MSG_BODY(msg));
}

//+Function LL_MakeIntTuple
//
// Builds a Python tuple out of an integer array
//
// Returns a PyObject pointer. The object must be erased calling
// Py_DECREF() with the pointer as an argument.

PyObject *LL_MakeIntTuple( int len, int *array)
{
	PyObject *py_object;
	int i;

	py_object = PyTuple_New( len);
	for ( i=0; i< len; i++)
		PyTuple_SetItem( py_object, i, PyInt_FromLong( array[i]));

	return py_object;
}	
//
//+Function LL_MakeDoubleTuple
//
// Builds a Python tuple out of an double array
//
// Returns a PyObject pointer. The object must be erased calling
// Py_DECREF() with the pointer as an argument.

PyObject *LL_MakeDoubleTuple( int len, double *array)
{
	PyObject *py_object;
	int i;

	py_object = PyTuple_New( len);
	for ( i=0; i< len; i++)
		PyTuple_SetItem( py_object, i, PyFloat_FromDouble( array[i]));

	return py_object;
}	

//+Function LL_WaitMessage
//
// Waits for the message with the specified seqnum
// Handles whatever unexpected messages arrive in the meantime
// Needs an already setup MsgBuf
//-


int LL_WaitMessage( int seqnum, MsgBuf *msg)
{
	int err, seq;

	while(1)
		{
        if (debug) printf("WaitMsgTimeout()\n");
		if ((err = WaitMsgTimeout(msg, LL_TIMEOUT )) != NO_ERROR)
			return err;

        if (debug) printf("Done\n");
		seq = HDR_SEQN(msg);

        if (debug) printf("Seq: %d   Seqnum: %d\n", seq, seqnum);
		if (seq == seqnum)
			return NO_ERROR; 

		// WE RETURN EVEN ON HIGHER SEQNUM - it means that we missed the return packet
		if (seq > seqnum)
			return NETWORK_ERROR;

        if (debug) printf("HandleMessage\n");
		LL_HandleMessage(msg);
		}
}

//+Function LL_HandleMessage
//
// Handles housekeeping messages like ECHO
//-


int LL_HandleMessage( MsgBuf *msg)
{
	return NO_ERROR;
}


//+Function: PyCCD_SetReadoutSpeed
//
// Sets the CCD readout speed
//
// Required arguments: (1) ccd number (int)
//                     (2) readout speed (int)
//-

static PyObject *PyCCD_SetReadoutSpeed( PyObject *self, PyObject *args)
{
	return SetSingleVariable( args, "CCD", "SPEED", INT_VARIABLE);
}



//+Function: PyCCD_SetRepetitions
//
// Sets the CCD repetitions number
//
// Required arguments: (1) ccd number (int)
//                     (2) repetition number (int)
//-

static PyObject *PyCCD_SetRepetitions( PyObject *self, PyObject *args)
{
	return SetSingleVariable( args, "CCD", "REP", INT_VARIABLE);
}


//+Function: PyCCD_SetRunState
//
// Sets the CCD run state
//
// Required arguments: (1) ccd number (int)
//                     (2) run state (int)
//-

static PyObject *PyCCD_SetRunState( PyObject *self, PyObject *args)
{
	return SetSingleVariable( args, "CCD", "GO", INT_VARIABLE);
}

//+Function: SetSingleVariable
//
// Sets a single variable in the MsgD-RTDB
//
// This is a generic function intented to make it easy to write
// functions that only sets a variable in the MsgD-RTB
//
// The Python wrapper function only needs to call this routine
// with the PyObject arguments, the device type ("CCD", "MV" and so on),
// the actual variable name (the part after the device number) and its type.
//
// The function will parse the Python argument correctly, and send a message
// to MsgD-RTDB with a SetVar command.
//-

PyObject *SetSingleVariable( PyObject *args, char *device_type, char *variable_name, int type)
{
	int device_number, err; 
	int i_value;
	double f_value;
	char *c_value;

	void *value = NULL;
	int value_len =1;

	char varname[ VAR_NAME_LEN+1];

	switch(type)
		{
		case INT_VARIABLE:
		if (!PyArg_ParseTuple( args, "ii", &device_number, &i_value))
			return NULL;
		value = (void *)&i_value;
		break;

		case REAL_VARIABLE:
		if (!PyArg_ParseTuple( args, "id", &device_number, &f_value))
			return NULL;
		value = (void *)&f_value;
		break;

		case CHAR_VARIABLE:
		if (!PyArg_ParseTuple( args, "is#", &device_number, &c_value, &value_len))
			return NULL;
		value = (void *)c_value;
		printf("Value is %s, valuelen is %d\n", (char *)value, value_len);
		break;
		}

	if (!value)
		return NULL;

	if (strcmp( device_type, "CCD") == 0)
		sprintf( varname, "CCD%02d%s", device_number, variable_name);
	else if (strcmp( device_type, "MV") == 0)
		sprintf( varname, "MV%02d%s", device_number, variable_name);
	else if (strcmp( device_type, "WFS") == 0)
		sprintf( varname, "WFS%02d%s", device_number, variable_name);
	else if (strcmp( device_type, "RTR") == 0)
		sprintf( varname, "RTR%02d%s", device_number, variable_name);

	err = SetVar( clientname, varname, type, value_len, value, NULL);

	return Py_BuildValue("i", err);
}



//+Function SetTupleVariable
//
// Sets an array  variable in the MsgD-RTDB
//
// Works in the same way as SetSingleVariable, but taking tuple arguments
// to set array variables
//

PyObject *SetTupleVariable( PyObject *args, char *device_type, char *variable_name, int type)
{
	int device_number, err; 
	void *array;
	int len,i;
	PyObject *ob;

	char varname[ VAR_NAME_LEN+1];

	if (!PyArg_ParseTuple( args, "iO", &device_number, &ob))
		return NULL;

	// Must be a sequence type
	if ( !PyTuple_Check(ob))
		return NULL;

	// Get the # of elements
	len = PyTuple_Size(ob);

	// Allocate the array
	array = malloc( VarSize( type, len));
	if (!array)
		return NULL;

	// Fill the array
	for (i=0; i<len; i++)
		switch(type)
			{
			case INT_VARIABLE:
			((int *)array)[i] = PyInt_AsLong( PyTuple_GetItem( ob, i));
			break;

			case REAL_VARIABLE:
			((double *)array)[i] = PyFloat_AsDouble( PyTuple_GetItem( ob, i));
			break;
			}

	if (strcmp( device_type, "CCD") == 0)
		sprintf( varname, "CCD%02d%s", device_number, variable_name);
	else if (strcmp( device_type, "MV") == 0)
		sprintf( varname, "MV%02d%s", device_number, variable_name);

	err = SetVar( clientname, varname, type, len, array, NULL);

	return Py_BuildValue("i", err);
}


//+Function: PyCCD_SetBlackLevels
//
// Sets the CCD run state
//
// Required arguments: (1) ccd number (int)
//                     (2) black levels (tuple)
//-

static PyObject *PyCCD_SetBlackLevels( PyObject *self, PyObject *args)
{
	return SetTupleVariable( args, "CCD", "BLACKS", INT_VARIABLE);
}


//+Function: PyOKO_GetFlat
//
// Returns the commands needed to flat the given OKO
//
// Required arguments: (1) oko number (int)
//-

static PyObject *PyOKO_GetFlat( PyObject *self, PyObject *args)
{
	int oko_num;

	if (!PyArg_ParseTuple( args, "i", &oko_num))
		return NULL;

	return Py_BuildValue("i", 1);
	
}


//+Function: PyCCD_SetBinning
//
// Sets the CCD binning
//
// Required arguments: (1) ccd number (int)
//                     (2) X and Y binning (tuple of integers)
//-

static PyObject *PyCCD_SetBinning( PyObject *self, PyObject *args)
{
	int xbin, ybin, ccdnum, err;
	PyObject *ob;

	char varname[ VAR_NAME_LEN+1];

	if (!PyArg_ParseTuple( args, "iO", &ccdnum, &ob))
		return NULL;

	// Must be a sequence type
	if ( !PyTuple_Check(ob))
		return NULL;

	// Tuple must be 2 elements
	if (PyTuple_Size(ob) != 2)
		return NULL;

	xbin = PyInt_AsLong( PyTuple_GetItem( ob, 0));
	ybin = PyInt_AsLong( PyTuple_GetItem( ob, 1));

	sprintf( varname, "CCD%02dXBIN", ccdnum);
	err = SetVar( clientname, varname, INT_VARIABLE, 1, &xbin, NULL);

	if (err != NO_ERROR)
		return Py_BuildValue("i", err);

	sprintf( varname, "CCD%02dYBIN", ccdnum);
	err = SetVar( clientname, varname, INT_VARIABLE, 1, &ybin, NULL);

	return Py_BuildValue("i", err);
}


//+Function: PyCCD_GetReadoutSpeed
//
// Gets the value of the Readout speed variable

static PyObject *PyCCD_GetReadoutSpeed( PyObject *self, PyObject *args)
{
	return GetSingleVariable( args, "CCD", "SPEED", INT_VARIABLE);
}

//+Function: PyCCD_GetBinning
//
// Gets the value of the CCD binning
//
// This function is a placeholder for the real one. For now, it returns
// the X binning, assuming that the binning is equal in both X and Y directions

static PyObject *PyCCD_GetBinning( PyObject *self, PyObject *args)
{
	return GetSingleVariable( args, "CCD", "XBIN", INT_VARIABLE);
}


//+Function: PyCCD_GetErrorMsg
//
// Gets the value of the Error message variable

static PyObject *PyCCD_GetErrorMsg( PyObject *self, PyObject *args)
{
	return GetSingleVariable( args, "CCD", "ERRMSG", CHAR_VARIABLE);
}

//+Function: GetSingleVariable
//
// Gets the value of a single variable in the MsgD-RTDB
//-

PyObject *GetSingleVariable( PyObject *args, char *device_type, char *variable_name, int type)
{
	Variable *TheVar;
	MsgBuf Msg;
	int device_number;
	PyObject *ob = NULL;

	if (!PyArg_ParseTuple( args, "i", &device_number))
		return NULL;

   InitMsgBuf(&Msg, 0);
	TheVar = LL_GetVar( &Msg, "%s%02d%s", device_type, device_number, variable_name);
        printf("Type: %d\n", type);
	switch(type)
		{
		case INT_VARIABLE:
		ob = Py_BuildValue("i", TheVar->Value.Lv[0]);
		break;

		case REAL_VARIABLE:
		ob = Py_BuildValue("f", TheVar->Value.Dv[0]);
		break;

		case CHAR_VARIABLE:
		ob = Py_BuildValue("s", TheVar->Value.Sv);
		break;
		}

	FreeMsgBuf( &Msg);

	return ob;
}


//+Function: PyCCD_GetTemperatures
//
// Gets the CCD temperature tuple
//-

static PyObject *PyCCD_GetTemperatures( PyObject *self, PyObject *args)
{
	return GetTupleVariable( args, "CCD", "TEMPS", INT_VARIABLE);
}
//
//
//+Function: PyCCD_GetBlackLevels
//
// Gets the CCD black levels as a tuple of integers
//-

static PyObject *PyCCD_GetBlackLevels( PyObject *self, PyObject *args)
{
	return GetTupleVariable( args, "CCD", "BLACKS", INT_VARIABLE);
}
	
//+Function: GetTupleVariable
//
// Gets an array variable from MsgD-RTDB as a tuple
//-

PyObject *GetTupleVariable( PyObject *args, char *device_type, char *variable_name, int type)
{
	int device_number;
	int len;
	PyObject *ob;
	Variable *TheVar;
	MsgBuf Msg;

	if (!PyArg_ParseTuple( args, "i", &device_number))
		return NULL;

	InitMsgBuf (&Msg, 0);

	TheVar = LL_GetVar( &Msg, "%s%d%s", device_type, device_number, variable_name); 
	len = TheVar->H.NItems;

	switch(type)
		{
		case INT_VARIABLE:
	 	ob = LL_MakeIntTuple( len, (int *)(TheVar->Value.Lv));
		break;

		case REAL_VARIABLE:
		ob = LL_MakeDoubleTuple( len, (double *)(TheVar->Value.Dv));
		break;

		case CHAR_VARIABLE:
		ob = Py_BuildValue("i", 0);
		break;

        default:
        ob = NULL;
        break;
		}

	FreeMsgBuf (&Msg);
	return ob;
}

//+Function: PyCCD_GetFrames
//
// Gets frames from the CCD
//
// returns a binary buffer with the frames read from the CCD
// 
// Required arguments: (1) ccd number (int)
//                     (2) number of frames (int)
//                     (3) first frame to retrieve (int)
//                     (4) what to retrieve: 0x01 pixels, 0x02 slopes, 0x03 commands
//-


SharedMem *pixel_shared = NULL;
SharedMem *slope_shared = NULL;
SharedMem *command_shared = NULL;

static PyObject *PyCCD_GetFrames( PyObject *self, PyObject *args)
{
   /*
	Variable *TheVar;
	MsgBuf Msg;
	SharedMem **shared;
	int *pixel_buffer;
	int framesize,bufsize,i, err, firstframe, what;
	unsigned char *dest;
	PyArrayObject *returnvalue;

	int array_dims[3];

	int ccdnum, nframes;
	int dx, dy, cur_xbin, cur_ybin;
   */

    printf("PyCCD_GetFrames(): DON'T USE THIS ROUTINE!!\n");

  return NULL;
}
/*
	if (!PyArg_ParseTuple( args, "iiii", &ccdnum, &nframes, &firstframe, &what))
		return NULL;

	InitMsgBuf(&Msg, 0);

	// Get data about CCD dimensions and binning
	
	TheVar = LL_GetVar( &Msg, "CCD%02dDX", ccdnum);
	dx = TheVar->Value.Lv[0];

	TheVar = LL_GetVar( &Msg, "CCD%02dDY", ccdnum);
	dy = TheVar->Value.Lv[0];

	TheVar = LL_GetVar( &Msg, "CCD%02dXBIN", ccdnum);
	cur_xbin = TheVar->Value.Lv[0];

	TheVar = LL_GetVar( &Msg, "CCD%02dYBIN", ccdnum);
	cur_ybin = TheVar->Value.Lv[0];

//	printf("dx %d, dy %d, xbin %d, ybin %d\n", dx, dy, cur_xbin, cur_ybin);

	// Calculate the buffer size, according to what we are going to retrieve
	int x,y;

	switch(what)
		{
		case GETPIXELS:
		x = dx / cur_xbin;
		y = dy / cur_ybin;
		framesize = x*y;
 		shared = &pixel_shared;
		break;

		case GETSLOPES:
		x = (dx/2) / cur_xbin;
		y = (dy/2) / cur_ybin;
		if ( x== 13) x = 12;
		if ( y== 13) y = 12;

		framesize = x*y *2;
		// Fix number of slopes to the smallest multiplier of 8
		printf("Framesize before: %d\n", framesize);
//		framesize += (framesize % 8);
//		framesize &= 0xFFFFFFF0;
		printf("Framesize after: %d\n", framesize);
		shared = &slope_shared;
		break;

		case GETCOMMANDS:
		framesize = 10;
		shared = &command_shared;
		break;

		default:
        shared = NULL;
        framesize=0;
		break;
		}

	bufsize = framesize * nframes * sizeof(int);

	// See if we have to change the dimensions of the shared buffer
	if ((*shared != NULL) && ((*shared)->blocksize < framesize*sizeof(int)))
		{
		printf("Changing shared buffer dimension from %d to %d\n", (*shared)->blocksize, framesize*sizeof(int));
		shmFree(*shared);
		*shared = NULL;
		}

	printf("framesize is %d\n", framesize);
	if (! *shared)
		{
		int bufsize = framesize * sizeof(int);
		bufsize += 1024 - (bufsize % 1024);
		printf("Rounding bufsize to %d \n", bufsize);
		*shared = shmAlloc( framesize*sizeof(int));
		}

//	printf("Bufsize is %d\n", bufsize);

	pixel_buffer = (int*)malloc( bufsize);

	int nap = dx/cur_xbin;

	// Loop over the frames sending messages to BCUCtrl
	for (i=0; i<nframes; i++)
		{
		int num = (i+firstframe) % 0xBA5;

		switch(what)
			{
			case GETPIXELS:
			askPixels( *shared, clientname, BCU_WFS, num, global_seqnum);
			break;

			case GETSLOPES:
			askSlopes( *shared, clientname, BCU_WFS, num, global_seqnum);
			break;

			case GETCOMMANDS:
			askCommands( *shared, clientname, BCU_WFS, num, global_seqnum);
			break;

			default:
			break;
			}
				

		err = LL_WaitMessage( global_seqnum++, &Msg);

		if (err == NO_ERROR)
			{
			dest = (unsigned char *)pixel_buffer + framesize*i*sizeof(int);
	
			// Reorder data from BCU ordering to normal one
			switch (what)
				{
				case GETPIXELS:
				BCUtoFrame( (int *)((*shared)->shm_addr), (int *)dest, nap);
				break;

				case GETSLOPES:
				BCUtoSlopes( (float *)((*shared)->shm_addr), (float *)dest, nap);	
				break;

				case GETCOMMANDS:
				BCUtoCMD( (*shared)->shm_addr, dest);
				dest[38]=0;
				dest[39]=0;
				break;
				}
			}
		}

	// Build the output array value
	switch (what)
		{
		case GETPIXELS:	
		printf("Making pixels\n");

		array_dims[0] = nframes;
		array_dims[1] = nap;
		array_dims[2] = nap;

		returnvalue = NA_vNewArray( pixel_buffer, tInt32, 3, array_dims);
		break;

		case GETSLOPES:
		printf("Making slopes, nframes %d, dy %d, dx %d\n", nframes, dy, dx);
		printf("binning: %d, %d\n", cur_xbin, cur_ybin);

		array_dims[0] = nframes;
		array_dims[1] = nap;
		array_dims[2] = nap/2;

		returnvalue = NA_vNewArray( pixel_buffer, tFloat32, 3, array_dims);


		printf("Array has %d elements\n", (int) NA_elements(returnvalue));
		break;

		case GETCOMMANDS:
		printf("Making commands\n");
		array_dims[0] = nframes;
		array_dims[1] = 40;
		returnvalue = NA_vNewArray( pixel_buffer, tUInt8, 2, array_dims);
		break;
		
		default:
		printf("Unknown data type\n");
		returnvalue = NULL;
		break;
		}

	return Py_BuildValue("O", returnvalue);
}

*/

int dx, dy, cur_xbin, cur_ybin, nsubap;

//@Function: PyCCD_PrepareGetCurFrame
//
// Prepares a CCD acquisition. Asks the system about the current
// CCD characteristics (dimension, binning, etc.) memorizes
// them for the following acquisition.
//

static PyObject *PyCCD_PrepareGetCurFrame( PyObject *self, PyObject *args)
{
    MsgBuf Msg;
    Variable *TheVar;

    int ccdnum;

    if (!PyArg_ParseTuple( args, "i", &ccdnum))
        return NULL;

    // Get data about CCD dimensions and binning
    InitMsgBuf(&Msg, 0);

     TheVar = LL_GetVar( &Msg, "CCD%02dDX", ccdnum);
     dx = TheVar->Value.Lv[0];
                 
     TheVar = LL_GetVar( &Msg, "CCD%02dDY", ccdnum);
     dy = TheVar->Value.Lv[0];

     TheVar = LL_GetVar( &Msg, "CCD%02dCURXBIN", ccdnum);
     cur_xbin = TheVar->Value.Lv[0];

     TheVar = LL_GetVar( &Msg, "CCD%02dCURYBIN", ccdnum);
     cur_ybin = TheVar->Value.Lv[0];

     TheVar = LL_GetVar( &Msg, "WFS%02dNSUBAP", ccdnum);
     nsubap = TheVar->Value.Lv[0];

     FreeMsgBuf(&Msg);

     return Py_BuildValue("i", NO_ERROR);
}

//@Function: PyCCD_GetCurFrame
//
// Gets the current frame from the CCD. Can integrate
// for a given number of frames
//
// Returns a numarray buffer with a data cube.
//
// Required arguments: (1) ccd number (int)
//                     (2) number of frames (int)
//                     (3) what to retrieve: 0x01 pixels, 0x02 slopes, 0x03 commands
// Optional arguments:
//                     (4) reorder to raster format (int, optional, default = 1)
//                     (5) prepared flag (int)
//                     (6) buffername (char)

static PyObject *PyCCD_GetCurFrame( PyObject *self, PyObject *args)
{
    Variable *TheVar;
    MsgBuf Msg;
    int framesize, i, what, stat;
    unsigned char *dest, *buffer;
    PyArrayObject *returnvalue;
    slopecomp_diagframe diagnostic_frame;
    techviewer_diagframe tv_diagnostic_frame;
    char *buffername = "MIRCTRL39:PIPPO";
    int bufferlen;

    int array_dims[3];

    int ccdnum,  nframes;
    int do_reorder = 1;
    int prepared = 0;
    nsubap =0;


    if (!PyArg_ParseTuple( args, "iii|iis#", &ccdnum, &nframes, &what, &do_reorder, &prepared, &buffername, &bufferlen))
        return NULL;

    if (debug) printf("GetCurFrame\n");
    printf("Get cur frame from buffer %s\n", buffername);
    InitMsgBuf(&Msg, 0);

    if (!prepared)
      {
      // Get data about CCD dimensions and binning
     
      TheVar = LL_GetVar( &Msg, "CCD%02dDX", ccdnum);
      dx = TheVar->Value.Lv[0];
                 
      TheVar = LL_GetVar( &Msg, "CCD%02dDY", ccdnum);
      dy = TheVar->Value.Lv[0];

      TheVar = LL_GetVar( &Msg, "CCD%02dCURXBIN", ccdnum);
      cur_xbin = TheVar->Value.Lv[0];

      TheVar = LL_GetVar( &Msg, "CCD%02dCURYBIN", ccdnum);
      cur_ybin = TheVar->Value.Lv[0];

//      TheVar = LL_GetVar( &Msg, "WFS%02dNSUBAP", ccdnum);
//      nsubap = TheVar->Value.Lv[0];
      }

// DEBUG
     printf("dx %d, dy %d, xbin %d, ybin %d, nsubap=%d\n", dx, dy, cur_xbin, cur_ybin, nsubap);

    // Calculate the buffer size, according to what we are going to retrieve
    int x,y;

    switch(what)
        {
        case GETPIXELS:
        x = dx / cur_xbin;
        y = dy / cur_ybin;
        framesize = x*y;
        break;

        case GETSLOPES:
        framesize = nsubap*2;
        break;

        case GETCOMMANDS:
        framesize = 48;
        break;

        default:
        framesize=0;
        break;
        }

    framesize *= sizeof(uint32);

    if (debug)
        printf("framesize is %d\n", framesize);
    buffer = (unsigned char *)malloc(framesize*nframes);
    memset(buffer, 0, framesize*nframes);

    int nap = dx/cur_xbin;

    // Attach to shared buffer

      if (!shm_ok)
        {
//        stat = bufRequest( clientname, "MIRCTRL39:PIPPO", BUFCONT, &bufinfo);
        printf("Buffer: %s\n", buffername);
        stat = bufRequest( clientname, buffername, BUFCONT, &bufinfo);
        if (debug)
            printf("Buf request stat: %d\n", stat);
        shm_ok = 1;
        }

    // Loop over the frames reading from the shared memory
    for (i=0; i<nframes; i++)
        {
        int counter=0;
        while (counter < 10)
            {
            if (ccdnum==1)
               stat = bufRead( &bufinfo, &diagnostic_frame, 100);
            if (ccdnum==3)
               stat = bufRead( &bufinfo, &tv_diagnostic_frame, 100);
            if (stat >=0) break;
            counter++;
            }
        if (counter==10)
            {
           printf("**** ERROR IN BUFREAD **** abort measurement\n");
           printf("%d\n", stat);
           return Py_BuildValue("i", stat);
            }

        dest = (unsigned char *)buffer + framesize*i;

        switch(what)
            {
            case GETPIXELS:
               /* Commented out because of the new Reorder class - use that!
            if (ccdnum ==1)
               {
               if (do_reorder)
                  reorder_pixels( diagnostic_frame.pixels, (uint32 *)dest, nap, ccdnum);
               else
                  memcpy( dest, diagnostic_frame.pixels, framesize);
               }
            else if (ccdnum ==3)
               {
               if (do_reorder)
                  reorder_pixels( tv_diagnostic_frame.pixels, (uint32 *)dest, nap, ccdnum);
               else
                  memcpy( dest, tv_diagnostic_frame.pixels, framesize);
               }
               */

            break;

            case GETSLOPES:
            memcpy( dest, diagnostic_frame.slopes, framesize);
            break;

            //case GETCOMMANDS:
            //memcpy( dest, diagnostic_frame.commands, framesize);
            //break;
            }
       }

    // Build the output array value
    switch (what)
        {
        case GETPIXELS:
        if (debug) printf("Making pixels\n");

        array_dims[0] = nframes;
        array_dims[1] = nap;
        array_dims[2] = nap;

	     if (debug) printf("%d - %d - %d\n", nframes, nap, nap);
        returnvalue = NA_vNewArray( buffer, tInt32, 3, array_dims);
	     printf("Got a pixel frame\n");
        break;

        case GETSLOPES:
// DEBUG
//        printf("Making slopes, nframes %d, dy %d, dx %d\n", nframes, dy, dx);
//        printf("binning: %d, %d\n", cur_xbin, cur_ybin);

/*      3d version - only makes sense when converting to raster order
        array_dims[0] = nframes;
        array_dims[1] = nap;
        array_dims[2] = nap/2;

        returnvalue = NA_vNewArray( buffer, tFloat32, 3, array_dims);
*/

        array_dims[0] = nframes;
        array_dims[1] = nsubap*2;
        returnvalue = NA_vNewArray( buffer, tFloat32, 2, array_dims);

        if (debug)
            printf("Array has %d elements\n", (int) NA_elements(returnvalue));
        break;

        case GETCOMMANDS:
        if (debug) printf("Making commands\n");
        array_dims[0] = nframes;
        array_dims[1] = 40;
        returnvalue = NA_vNewArray( buffer, tInt32, 2, array_dims);
        break;

        default:
        printf("Unknown data type\n");
        returnvalue = NULL;
        break;
        }

        free(buffer);
        return Py_BuildValue("O", returnvalue);
 }

//@Function: PyCCD_GetPreviousFrame
//
// Gets an arbitrary frame from the BCU memory
//
// Returns a numarray buffer with a data frame
//
// Required arguments: (1) ccd number (int)
//                     (2) frame number (int)
//                     (3) what to retrieve: 0x01 pixels, 0x02 slopes, 0x03 commands
//                     (4) reorder to raster format (int, optional, default = 1)

static PyObject *PyCCD_GetPreviousFrame( PyObject *self, PyObject *args)
{

/*
    Variable *TheVar;
    MsgBuf Msg;
    int framesize, what, stat;
    unsigned char *buffer;
    PyArrayObject *returnvalue;
    slopecomp_diagframe diagnostic_frame;
    static int seqnum=0;

    int array_dims[2];

    int ccdnum, framenum;
    int do_reorder = 1;
    int prepared = 0;


    if (!PyArg_ParseTuple( args, "iii|ii", &ccdnum, &framenum, &what, &do_reorder, &prepared))
        return NULL;
        */

    printf("GetPreviousFrame: routine must be rewritten for threadlib code!!!!\n");

    return NULL;
}

/*
    if (debug) printf("GetPreviousFrame\n");
    InitMsgBuf(&Msg, 0);

    if (!prepared)
      {
      // Get data about CCD dimensions and binning
     
      TheVar = LL_GetVar( &Msg, "CCD%02dDX", ccdnum);
      dx = TheVar->Value.Lv[0];
                 
      TheVar = LL_GetVar( &Msg, "CCD%02dDY", ccdnum);
      dy = TheVar->Value.Lv[0];

      TheVar = LL_GetVar( &Msg, "CCD%02dCURXBIN", ccdnum);
      cur_xbin = TheVar->Value.Lv[0];

      TheVar = LL_GetVar( &Msg, "CCD%02dCURYBIN", ccdnum);
      cur_ybin = TheVar->Value.Lv[0];

      TheVar = LL_GetVar( &Msg, "WFS%02dNSUBAP", ccdnum);
      nsubap = TheVar->Value.Lv[0];
      }

// DEBUG
//     printf("dx %d, dy %d, xbin %d, ybin %d, nsubap=%d\n", dx, dy, cur_xbin, cur_ybin, nsubap);

    // Calculate the buffer size, according to what we are going to retrieve
    int x,y;

    switch(what)
        {
        case GETPIXELS:
        x = dx / cur_xbin;
        y = dy / cur_ybin;
        framesize = x*y;
        break;

        case GETSLOPES:
        framesize = nsubap*2;
        break;

        case GETCOMMANDS:
        framesize = 48;
        break;

        default:
        framesize=0;
        break;
        }

    framesize *= sizeof(float32);
    if (debug)
        printf("framesize is %d\n", framesize);
    buffer = (unsigned char *)malloc(framesize);
    memset(buffer, 0, framesize);

    int nap = dx/cur_xbin;

    int diag_size = sizeof(diagnostic_frame) / sizeof(uint32);
    int address = framenum * diag_size;

    int counter=0;
    while (counter < 10)
        {
       // stat = sendBCUcommand( clientname, 39, 255, 255, MGP_OP_RDSEQ_SDRAM, address, (unsigned char *)&diagnostic_frame, sizeof(diagnostic_frame), 4000, seqnum++, 0);

      //  Reading only the interesting parts should be faster 
        int offset; 
        switch(what)
            {   
            case GETPIXELS:
            offset = ((char *)&(diagnostic_frame.pixels) - ((char *)&diagnostic_frame)) / sizeof(uint32);
            stat = sendBCUcommand( clientname, 39, 255, 255, MGP_OP_RDSEQ_SDRAM, address + offset, (unsigned char *)&diagnostic_frame.pixels, sizeof(diagnostic_frame.pixels), 4000, seqnum++, 0);
            break;

            case GETSLOPES:
            offset = ((char *)&(diagnostic_frame.slopes) - ((char *)&diagnostic_frame)) / sizeof(uint32);
            stat = sendBCUcommand( clientname, 39, 255, 255, MGP_OP_RDSEQ_SDRAM, address + offset, (unsigned char *)&diagnostic_frame.slopes, sizeof(diagnostic_frame.slopes), 4000, seqnum++, 0);
            break;

            case GETCOMMANDS:
            offset = ((char *)&(diagnostic_frame.commands) - ((char *)&diagnostic_frame)) / sizeof(uint32);
            stat = sendBCUcommand( clientname, 39, 255, 255, MGP_OP_RDSEQ_SDRAM, address + offset, (unsigned char *)&diagnostic_frame.commands, sizeof(diagnostic_frame.commands), 4000, seqnum++, 0);
            break;
            }
        if (stat >=0) break;
        counter++;
        }
    if (counter==10)
        {
       printf("**** ERROR IN sendBCUcommand() **** abort measurement\n");
       return Py_BuildValue("i", stat);
        }
    else
      {
       printf("Frame %d read\n", framenum);
       }

        switch(what)
            {
            case GETPIXELS:
            if (do_reorder)
                reorder_pixels( diagnostic_frame.pixels, (uint32 *)buffer, nap, ccdnum);
            else
                memcpy( buffer, diagnostic_frame.pixels, framesize);
            break;

            case GETSLOPES:
            memcpy( buffer, diagnostic_frame.slopes, framesize);
            break;

            case GETCOMMANDS:
            memcpy( buffer, diagnostic_frame.commands, framesize);
            break;
            }

    // Build the output array value
    switch (what)
        {
        case GETPIXELS:
        if (debug) printf("Making pixels\n");

        array_dims[0] = nap;
        array_dims[1] = nap;

	     if (debug) printf("%d - %d\n", nap, nap);
        returnvalue = NA_vNewArray( buffer, tInt32, 2, array_dims);
        break;

        case GETSLOPES:
        array_dims[0] = nsubap*2;
        returnvalue = NA_vNewArray( buffer, tFloat32, 1, array_dims);

        if (debug)
            printf("Array has %d elements\n", (int) NA_elements(returnvalue));
        break;

        case GETCOMMANDS:
        if (debug) printf("Making commands\n");
        array_dims[0] = 48;
        returnvalue = NA_vNewArray( buffer, tInt32, 1, array_dims);
        break;

        default:
        printf("Unknown data type\n");
        returnvalue = NULL;
        break;
        }

        free(buffer);
        return Py_BuildValue("O", returnvalue);
 }

*/


//@Function: PyCCD_GetCompleteFrame
//
// Gets the current frame from the CCD. Can integrate
// for a given number of frames
//
// Returns a numarray buffer with a data cube.
//
// Required arguments: (1) ccd number (int)
//                     (2) reorder to raster format (int, optional, default = 1)

static PyObject *PyCCD_GetCompleteFrame( PyObject *self, PyObject *args)
{
    Variable *TheVar;
    MsgBuf Msg;
    int framesize, i, what, stat;
    unsigned char *dest, *buffer;
    PyArrayObject *returnvalue;
    slopecomp_diagframe diagnostic_frame;

    int array_dims[3];

    int ccdnum,  nframes;
    int dx, dy, cur_xbin, cur_ybin, nsubap;
    int do_reorder = 1;

    printf("Parsing arguments\n");

    if (!PyArg_ParseTuple( args, "i|i", &ccdnum, &do_reorder))
        return NULL;

    printf("GetCompleteFrame - THIS FUNCTION IS NOT FINISHED YET\n");
    InitMsgBuf(&Msg, 0);

    // Get data about CCD dimensions and binning
     
     TheVar = LL_GetVar( &Msg, "CCD%02dDX", ccdnum);
     dx = TheVar->Value.Lv[0];
                 
     TheVar = LL_GetVar( &Msg, "CCD%02dDY", ccdnum);
     dy = TheVar->Value.Lv[0];

     TheVar = LL_GetVar( &Msg, "CCD%02dCURXBIN", ccdnum);
     cur_xbin = TheVar->Value.Lv[0];

     TheVar = LL_GetVar( &Msg, "CCD%02dCURYBIN", ccdnum);
     cur_ybin = TheVar->Value.Lv[0];

     TheVar = LL_GetVar( &Msg, "WFS%02dNSUBAP", ccdnum);
     nsubap = TheVar->Value.Lv[0];

// DEBUG
//     printf("dx %d, dy %d, xbin %d, ybin %d, nsubap=%d\n", dx, dy, cur_xbin, cur_ybin, nsubap);

    // Attach to shared buffer

      if (!shm_ok)
        {
        stat = bufRequest( clientname, "MIRCTRL39:PIPPO", BUFCONT, &bufinfo);
        if (debug)
            printf("Buf request stat: %d\n", stat);
        shm_ok = 1;
        }

    // Calculate the buffer size, according to what we are going to retrieve
    int x,y;

    printf("ok1\n");
    switch(what)
        {
        case GETPIXELS:
        x = dx / cur_xbin;
        y = dy / cur_ybin;
        framesize = x*y;
        break;

        case GETSLOPES:
        framesize = nsubap*2;
        break;

        case GETCOMMANDS:
        framesize = 48;
        break;

        default:
        break;
        }

    framesize *= sizeof(float32);
    printf("ok2\n");
    if (debug)
        printf("framesize is %d\n", framesize);
    buffer = (unsigned char *)malloc(framesize*nframes);
    memset(buffer, 0, framesize*nframes);

    int nap = dx/cur_xbin;

    // Loop over the frames reading from the shared memory
    for (i=0; i<nframes; i++)
        {
        stat = bufRead( &bufinfo, &diagnostic_frame, 100);
        if (debug)
            printf("Buf read stat: %d\n", stat);
        dest = (unsigned char *)buffer + framesize*i;

        switch(what)
            {
            case GETPIXELS:
/* DEBUG
            printf("Copying %d bytes to dest\n", framesize);
            for (i=0; i<100; i++)
                {
                diagnostic_frame.pixels[i] = i;
                printf(" %08X", diagnostic_frame.pixels[i]);
                }
            printf("\n");
*/
               /* Commented out because of the new Reorder class - Use that instead!
            if (do_reorder)
                reorder_pixels( diagnostic_frame.pixels, (uint32 *)dest, nap, ccdnum);
            else
                memcpy( dest, diagnostic_frame.pixels, framesize);
                */
/* DEBUG
            for (i=0; i<100; i++)
                printf(" %08X", ((uint32 *)dest)[i]);
            printf("\n");
*/
            break;

            case GETSLOPES:
            memcpy( dest, diagnostic_frame.slopes, framesize);

/* Debug
            for (i=0; i<100; i++)
                printf("slope %d: %5.2f\n", i, ((float32 *)dest)[i]);
*/

            break;

            //case GETCOMMANDS:
            //memcpy( dest, diagnostic_frame.commands, framesize);
            //break;
            }
       }

    // Build the output array value
    switch (what)
        {
        case GETPIXELS:
        printf("Making pixels\n");

        array_dims[0] = nframes;
        array_dims[1] = nap;
        array_dims[2] = nap;

	printf("%d - %d - %d\n", nframes, nap, nap);
        returnvalue = NA_vNewArray( dest, tInt32, 3, array_dims);
	printf("Done\n");
        break;

        case GETSLOPES:
// DEBUG
//        printf("Making slopes, nframes %d, dy %d, dx %d\n", nframes, dy, dx);
//        printf("binning: %d, %d\n", cur_xbin, cur_ybin);

/*      3d version - only makes sense when converting to raster order
        array_dims[0] = nframes;
        array_dims[1] = nap;
        array_dims[2] = nap/2;

        returnvalue = NA_vNewArray( dest, tFloat32, 3, array_dims);
*/

        array_dims[0] = nframes;
        array_dims[1] = nsubap*2;
        returnvalue = NA_vNewArray( dest, tFloat32, 2, array_dims);

        if (debug)
            printf("Array has %d elements\n", (int) NA_elements(returnvalue));
        break;

        case GETCOMMANDS:
        printf("Making commands\n");
        array_dims[0] = nframes;
        array_dims[1] = 40;
        returnvalue = NA_vNewArray( dest, tInt32, 2, array_dims);
        break;

        default:
        printf("Unknown data type\n");
        returnvalue = NULL;
        break;
        }

        return Py_BuildValue("O", returnvalue);
 }

    














//+Function: PyBCU_Read
// 
// Reads a buffer of memory from the specified BCU
//-

/*
static PyObject *PyBCU_Read( PyObject *self, PyObject *args)
{
	unsigned int address, len;
	int opcode, err;
	BCUmessage cmd;
	MsgBuf msgb;
	SharedMem *shared;
	PyObject *ob;

	if (!PyArg_ParseTuple( args, "iii", &address, &len, &opcode))
		return NULL;

	shared = shmAlloc( len*sizeof(int));

	cmd.BCUnumber = BCU_WFS;
	strcpy( cmd.Client, clientname);
	cmd.firstDSP = 0xFF;
	cmd.lastDSP = 0xFF;
	cmd.address = address;
	cmd.len = len*4;
	cmd.command = opcode;
	cmd.buffer_id = shared->shm_id;


	InitMsgBuf(&msgb, 0);
	FillMessage( sizeof(cmd), global_seqnum, clientname, "MIRCTRL39", BCUMSG, &cmd, &msgb);
	SendToServer(&msgb);
	FreeMsgBuf(&msgb);
	
	InitMsgBuf(&msgb, 0);
	err = LL_WaitMessage( global_seqnum++, &msgb);

	if (err == NO_ERROR)
		ob = Py_BuildValue("s#", shared->shm_addr, len*4);
	else
		ob = Py_BuildValue("i", err);


	shmFree(shared);

	return ob;	
}
*/


//+Function: SendBinaryMessage
//
// sends a message to a client. If the destination argument is empty,
// the message is sent to the MsgD-RTDB  
// Returns an error string ("OK" if no error is present)
//
// Required arguments: (1) destination name	(string)
//                     (2) message code		(int)
//                     (3) message data		(array)
//		       (4) (optional) wantreply (int) if set, waits for reply
//-

static PyObject *PySendBinaryMessage( PyObject *self, PyObject *args)
{
	MsgBuf Msg;
	char *to,  *data;
	int err,addr,size,code;
	int wantreply=0;

	if (!PyArg_ParseTuple( args, "sis#|i", &to, &code, &addr, &size, &wantreply))
		return NULL;

	data = (char *)addr;

	// fill message and immediately increment seqnum
	InitMsgBuf( &Msg, size);
	FillMessage( size, global_seqnum++, clientname, to, code, data, &Msg);

	if ( (err = SendToServer(&Msg)) != NO_ERROR)
		{
		printf("Error in SendBinaryMessage: (%d) %s\n", err, lao_strerror(err));
		return Py_BuildValue("s", lao_strerror(err));
		}

	// Wait for answer if requested
	if (wantreply)
		LL_WaitMessage( global_seqnum-1, &Msg);

	FreeMsgBuf(&Msg);
	return Py_BuildValue("s", "OK");
}

//+Function: PyWFS_SetRunState
//
// Starts or stops the WFS BCU dsp loop 
//
// Required arguments: (1) wfs number (int)
//                     (2) run state. 1= start, 0 =stop (int)
//-

static PyObject *PyWFS_SetRunState( PyObject *self, PyObject *args)
{
	return SetSingleVariable( args, "WFS", "GO", INT_VARIABLE);
}

//+Function: PyWFS_SetPixelLUT
//
// Sets a new pixel lookup table
//
// Required arguments: (1) wfs number (int)
//                     (2) pixel lookup table filename (string)
//-

static PyObject *PyWFS_SetPixelLUT( PyObject *self, PyObject *args)
{
	return SetSingleVariable( args, "WFS", "PIXELLUT", CHAR_VARIABLE);
}

//+Function: PyWFS_SetNsubap
//
// Sets a new number of subapertures on a WFS BCU
//
// Required arguments: (1) wfs number (int)
//                     (2) number of subapertures (int)
//-

static PyObject *PyWFS_SetNsubap( PyObject *self, PyObject *args)
{
	return SetSingleVariable( args, "WFS", "NSUBAP", INT_VARIABLE);
}

//+Function: PyWFS_GetNsubap
//
// Gets the number of subapertures on a WFS BCU
//
// Required arguments: (1) wfs number (int)
//-

static PyObject *PyWFS_GetNsubap( PyObject *self, PyObject *args)
{
    return GetSingleVariable( args, "WFS", "NSUBAP", INT_VARIABLE);
}

//+Function: PyWFS_SetSlopeOffsets
//
// Sets a new slope offset pattern
//
// Required arguments: (1) wfs number (int)
//                     (2) slope array file (string)
//-

static PyObject *PyWFS_SetSlopeOffsets( PyObject *self, PyObject *args)
{
    char *filename;
    int wfsnum, len, err;
    char varname[ VAR_NAME_LEN+1];

	if (!PyArg_ParseTuple( args, "is#", &wfsnum, &filename, &len))
		return NULL;

    sprintf( varname, "WFS%02d%s", wfsnum, "SLPNULL");

    err = SetVar( clientname, varname, CHAR_VARIABLE, len, filename, NULL);

	return Py_BuildValue("i", err);
}


//+Function: PyWFS_SetDigitalNoise
//
// Sets a new digital noise pattern
//
// Required arguments: (1) wfs number (int)
//                     (2) slope array file (string)
//-

static PyObject *PyWFS_SetDigitalNoise( PyObject *self, PyObject *args)
{
    char *filename;
    int wfsnum, len, err;
    char varname[ VAR_NAME_LEN+1];

	if (!PyArg_ParseTuple( args, "is#", &wfsnum, &filename, &len))
		return NULL;

    sprintf( varname, "WFS%02d%s", wfsnum, "CMD_OFFSETS");

    printf("Setting digital noise to %s\n", filename);

    err = SetVar( clientname, varname, CHAR_VARIABLE, len, filename, NULL);

	return Py_BuildValue("i", err);
}


//+Function: PyWFS_EnableDigitalNoise
//
// Enable/Disable digital noise
//
// Required arguments: (1) wfs number (int)
//                     (2) enable flag (int)
//-

static PyObject *PyWFS_EnableDigitalNoise( PyObject *self, PyObject *args)
{
   return SetSingleVariable( args, "WFS", "CMD_ENABLE", INT_VARIABLE);
}



//+Function: PyRTR_SetREC
//
// Sets a new reconstructor matrix
//
// Required arguments: (1) rtr number (int)
//                     (2) reconstructor matrix filename (string)
//-

static PyObject *PyRTR_SetREC( PyObject *self, PyObject *args)
{
	return SetSingleVariable( args, "RTR", "REC", CHAR_VARIABLE);
}

//+Function: PyRTR_SetNsubap
//
// Sets a new number of subapertures
//
// Required arguments: (1) rtr number (int)
//                     (2) number of subapertures (int)
//-

static PyObject *PyRTR_SetNsubap( PyObject *self, PyObject *args)
{
	return SetSingleVariable( args, "RTR", "NSUBAP", INT_VARIABLE);
}


//+Function: PyRTR_SetLoopState
//
// Opens or closes the RTR loop 
//
// Required arguments: (1) rtr number (int)
// 		       (2) loop state: 1 = closed, 0 = opened (int)
//-

static PyObject *PyRTR_SetLoopState( PyObject *self, PyObject *args)
{
	return SetSingleVariable( args, "RTR", "CLOSE", INT_VARIABLE);
}


//+Function: PyMOV_GetProfile
//
// Gets the movement profile
//
// returns a python dictionary with the CCD complete status
// 
// Required arguments: (1) movement number (int)
//-

static PyObject *PyMOV_GetProfile( PyObject *self, PyObject *args)
{
	Variable *TheVar;
	MsgBuf Msg;

	int movnum;
	int loend, hiend, curpos, targetpos, status_code, cur_speed, hi_speed;
	char errmsg[64], name[64];
	PyObject *py_return;

	if (!PyArg_ParseTuple( args, "i", &movnum))
		return NULL;

	InitMsgBuf(&Msg, 0);

	TheVar = LL_GetVar( &Msg, "MV%02dLOEND", movnum);
	loend = TheVar->Value.Lv[0];

	TheVar = LL_GetVar( &Msg, "MV%02dHIEND", movnum);
	hiend = TheVar->Value.Lv[0];

	TheVar = LL_GetVar( &Msg, "MV%02dCURPOS", movnum);
	curpos = TheVar->Value.Lv[0];

	TheVar = LL_GetVar( &Msg, "MV%02dTGTPOS", movnum);
	targetpos = TheVar->Value.Lv[0];

	TheVar = LL_GetVar( &Msg, "MV%02dSTATUS", movnum);
	status_code = TheVar->Value.Lv[0];

	TheVar = LL_GetVar( &Msg, "MV%02dSPEED", movnum);
	cur_speed = TheVar->Value.Lv[0];

	TheVar = LL_GetVar( &Msg, "MV%02dHISPEED", movnum);
	hi_speed = TheVar->Value.Lv[0];

	TheVar = LL_GetVar( &Msg, "MV%02dERRMSG", movnum);
	strcpy( errmsg, TheVar->Value.Sv);

	TheVar = LL_GetVar( &Msg, "MV%02dNAME", movnum);
	strcpy( name, TheVar->Value.Sv);

	py_return = Py_BuildValue("{s:i, s:i, s:i, s:i, s:i, s:i, s:s, s:s, s:O, s:O, s:O, s:O, s:O}",
					"loend", loend, "hiend", hiend, "cur_position", curpos, "target_position", targetpos,
					"statu_code", status_code, "cur_speed", cur_speed, "hi_speed", hi_speed,
					"name", name, "errmsg", errmsg);

	FreeMsgBuf( &Msg);


	return py_return;
}

//+Function: PyMOV_GetCurPos
//
// Gets the current position of the movement
//-

static PyObject *PyMOV_GetCurPos( PyObject *self, PyObject *args)
{
	return GetSingleVariable( args, "MV", "CURPOS", REAL_VARIABLE);
}


//+Function: PyMOV_IsMoving
//
// Returns 1 if the motor is moving, 0 if not
//-

static PyObject *PyMOV_IsMoving( PyObject *self, PyObject *args)
{
	int status, movnum;
	Variable *TheVar;
	MsgBuf Msg;

	if (!PyArg_ParseTuple( args, "i", &movnum))
		return NULL;

	InitMsgBuf(&Msg,0);

	TheVar = LL_GetVar( &Msg, "MV%02dSTATUS", movnum);
	status = TheVar->Value.Lv[0];

	if (status & STATUS_MOVING)
		return Py_BuildValue("i", 1);
	else
		return Py_BuildValue("i", 0);
}

//+Function: MoveTo
//
// Sets the Target position variable 
//-

static PyObject *PyMOV_MoveTo( PyObject *self, PyObject *args)
{
	return SetSingleVariable( args, "MV", "TGTPOS", REAL_VARIABLE);
}

//+Function: PyMOV_GetErrorMsg
//
// Gets the value of the Error message variable

static PyObject *PyMOV_GetErrorMsg( PyObject *self, PyObject *args)
{
	return GetSingleVariable( args, "MV", "ERRMSG", CHAR_VARIABLE);
}



// +Entry: PyCreateVar 
 
// Sends a message to the MsgD-RTDB asking the creation of a variable.
// Returns "OK" or an error string.  
//
// Required arguments: (1) variable name   (string)
//                     (2) variable type   (integer)
