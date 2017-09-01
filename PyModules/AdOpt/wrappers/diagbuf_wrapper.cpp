//@File: diagbuf_wrapper.cpp
//
// Python module to interface to ccd reordering library
//-

#include <Python.h>
#include "numpy/libnumarray.h"

#include <stdlib.h>

#include "bcu_diag.h"

#include "aotypes.h"
#include "../lib/base/msglib.h"

#include <vector>

static int debug_output=0;

//@
// diagbuf_wrapper module
//
// Provides access to realtime diagnostic data in the AdOpt system.
//
// ********************************************************
// This is low-level, non-object-oriented interface.
// Use the diagbuf.py module for a higher level interface.
// ********************************************************
//
// API from Python:
//
// intf = diagbuf( ccd)  - initializes an interace to a shared memory buffer, among the known types
//
// pixelsRaster( intf, buf)   - returns an array with pixels in raster order
// pixelsRaw( intf, buf)      - returns an array with pixels in raw order
// slopesRaster( intf, buf)   - returns an array with slopes in raster order (if available)
// slopesRaw( intf, buf)      - returns an array with pixels in raw order    (if available)
//
// release( intf)             - deallocates interface.
//
// Usage:
//
// ---> NOTE: use the diagbuf.py for higher-order interface.
//
// intf = diagbuf_wrapper.diagbuf_wrapper( ccd)
// buf = aoapp.BufRead( bufname)
// pixels = diagbuf_wrapper.pixelsRaster( intf, buf)


extern "C" {
// methods callable from Python
void initdiagbuf_wrapper(void);
static PyObject *PyDiagbuf( PyObject *, PyObject *);
static PyObject *PyPixelsRaster( PyObject *, PyObject *);
static PyObject *PyPixelsRaw( PyObject *, PyObject *);
static PyObject *PySlopesRaster( PyObject *, PyObject *);
static PyObject *PySlopesRaw( PyObject *, PyObject *);
static PyObject *PyRelease( PyObject *, PyObject *);
}

// Internal methods, not API

PyObject *getPixels( PyObject *self, PyObject *args, int raster);
PyObject *getSlopes( PyObject *self, PyObject *args, int raster);

// Diagbuf "object" (not a python object)

typedef struct
{
   AbstractDiagnClass *diagbuf;
} diagbuf_type;


// Registration of methods callable from Python

static struct PyMethodDef diagbuf_wrapper_methods[] = {
    {"Diagbuf", PyDiagbuf, METH_VARARGS, "Initializes interface"},
    {"pixelsRaster", PyPixelsRaster, METH_VARARGS, "Returns an array of pixels in raster order"},
    {"pixelsRaw", PyPixelsRaster, METH_VARARGS, "Returns an array of pixels in raw order"},
    {"slopesRaster", PySlopesRaster, METH_VARARGS, "Returns an array of slopes in raster order"},
    {"slopesRaw", PySlopesRaw, METH_VARARGS, "Returns an array of slopes in raw order"},
    {"release", PyRelease, METH_VARARGS, "Releases interface"},
    {NULL, NULL, 0, NULL}
};

//@Function: initdiagbuf_wrapper

// Function called by Python import mechanism

// This function is the equivalent of Python classes' __init__, and is
// automaticaly called by the interpreter when importing the module.
//@

void initdiagbuf_wrapper()
{
    Py_InitModule("diagbuf_wrapper", diagbuf_wrapper_methods);
    import_libnumarray();
}

PyObject *PyRelease( PyObject *self, PyObject *args)
{
   diagbuf_type *obj;
   int dummy;

   if (!PyArg_ParseTuple( args, "s#", &obj, &dummy))
         return NULL;

   if (obj->diagbuf) {
        delete obj->diagbuf;
        obj->diagbuf = NULL;
    }

   return Py_BuildValue("i", 0);
}

PyObject *PyDiagbuf( PyObject *self, PyObject *args)
{
   int ccd;
   diagbuf_type obj;
  
   if (!PyArg_ParseTuple( args, "i", &ccd))
      return NULL;

   memset( &obj, 0, sizeof(diagbuf_type));

   switch( ccd)
      {
      case 39:
      obj.diagbuf = new OptLoopDiagnClass();
      break;
      case 47:
      obj.diagbuf = new TechViewerDiagnClass();
      break;
      case 100:
      obj.diagbuf = new ThorlabsDiagnClass();
      break;
      default:
      return NULL;
      }            

   return Py_BuildValue("s#", &obj, sizeof(diagbuf_type));
}

PyObject *PyPixelsRaster( PyObject *self, PyObject *args)
{
   return getPixels( self, args, 1);
}

PyObject *PyPixelsRaw( PyObject *self, PyObject *args)
{
   return getPixels( self, args, 0);
}

PyObject *getPixels( PyObject *self, PyObject *args, int raster)
{
   diagbuf_type *obj;
   unsigned char *data;
   int dummy1, dummy2;
   int array_dim[2];

   if (!PyArg_ParseTuple( args, "s#s#", &obj, &dummy1, &data, &dummy2))
         return NULL;

   if (obj->diagbuf)
	{
   	obj->diagbuf->setData(data);
   	if (debug_output) obj->diagbuf->ident();

   	if (raster)
      		obj->diagbuf->pixels_raster_dim( array_dim, array_dim+1);
   	else
      		obj->diagbuf->pixels_raw_dim( array_dim, array_dim+1);

  	if (debug_output) printf("Creating array %dx%d\n", array_dim[0], array_dim[1]);
   	if (raster)
      		return (PyObject *)NA_vNewArray( obj->diagbuf->pixels_raster(), tInt16, 2, array_dim);
   	else
      		return (PyObject *)NA_vNewArray( obj->diagbuf->pixels_raw(), tInt16, 2, array_dim);
	}
   return NULL;
}

PyObject *PySlopesRaster( PyObject *self, PyObject *args)
{
   return getSlopes( self, args, 1);
}

PyObject *PySlopesRaw( PyObject *self, PyObject *args)
{
   return getSlopes( self, args, 0);
}

PyObject *getSlopes( PyObject *self, PyObject *args, int raster)
{
   diagbuf_type *obj;
   unsigned char *data;
   int dummy1, dummy2;

   if (!PyArg_ParseTuple( args, "s#s#", &obj, &dummy1, &data, &dummy2))
         return NULL;

   obj->diagbuf->setData(data);

   int array_dim[2];
   if (raster)
      obj->diagbuf->slopes_raster_dim( array_dim, array_dim+1);
   else
      obj->diagbuf->slopes_raw_dim( array_dim, array_dim+1);

   if (raster)
      return (PyObject *)NA_vNewArray( obj->diagbuf->slopes_raster(), tFloat32, 2, array_dim);
   else
      return (PyObject *)NA_vNewArray( obj->diagbuf->slopes_raw(), tFloat32, 2, array_dim);

   return Py_BuildValue("i", 0);
}





