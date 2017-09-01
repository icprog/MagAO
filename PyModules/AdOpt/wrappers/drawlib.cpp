//@File: drawlib.cpp
//
// Python module to use the drawlib.
//
//@

#include <Python.h>
#include <string>
#include "drawlib/drawobjects.h"


extern "C" {
// methods callable from Python
void initdrawlib(void);
static PyObject *PySendLine( PyObject *, PyObject *);
static PyObject *PySendArrow( PyObject *, PyObject *);
static PyObject *PySendCircle( PyObject *, PyObject *);
}


// Registration of methods callable from Python

static struct PyMethodDef drawlib_methods[] = {
    {"sendLine", PySendLine, METH_VARARGS, "Sends a line to a drawing server"},
    {"sendArrow", PySendArrow, METH_VARARGS, "Sends an arrow to a drawing server"},
    {"sendCircle", PySendCircle, METH_VARARGS, "Sends a circle to a drawing server"},
    {NULL, NULL, 0, NULL}
};


//+Function: initdrawlib

// Function called by Python import mechanism

// This function is the equivalent of Python classes' __init__, and is
// automaticaly called by the interpreter when importing the module.
//-

void initdrawlib()
{
   Py_InitModule("drawlib", drawlib_methods);
}


//@Function: PySendLine

static PyObject *PySendLine( PyObject *self, PyObject *args)
{
   float x1, y1, x2, y2;
   int color, timevalid, useccdcoord;
   char *clientName;

    if (!PyArg_ParseTuple( args, "sffffiii", &clientName, &x1, &y1, &x2, &y2, &color, &timevalid, &useccdcoord))
       return NULL;

    drawobject *obj = new line( x1, y1, x2, y2, color, timevalid, useccdcoord);
    sendObject( clientName, obj);
    delete obj;

    return Py_BuildValue("i", Py_None);
}

//@Function: PySendArrow

static PyObject *PySendArrow( PyObject *self, PyObject *args)
{
   float x1, y1, x2, y2;
   int color, timevalid, useccdcoord;
   char *clientName;

    if (!PyArg_ParseTuple( args, "sffffiii", &clientName, &x1, &y1, &x2, &y2, &color, &timevalid, &useccdcoord))
       return NULL;

    drawobject *obj = new arrow( x1, y1, x2, y2, color, timevalid, useccdcoord);
    sendObject( clientName, obj);
    delete obj;

    return Py_BuildValue("i", Py_None);
}

//@Function: PySendCircle

static PyObject *PySendCircle( PyObject *self, PyObject *args)
{
   float cx, cy, diam;
   int color, timevalid, useccdcoord;
   char *clientName;

    if (!PyArg_ParseTuple( args, "sfffiii", &clientName, &cx, &cy, &diam, &color, &timevalid, &useccdcoord))
       return NULL;

    drawobject *obj = new circle( cx, cy, diam, color, timevalid, useccdcoord);
    sendObject( clientName, obj);
    delete obj;

    return Py_BuildValue("i", Py_None);
}
