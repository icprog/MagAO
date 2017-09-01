//@File: reorder.cpp
//
// Python module to interface to ccd reordering library
//-

#include <Python.h>
#include "numpy/libnumarray.h"

#include <stdlib.h>

#include "bcu_diag.h"

#include "aotypes.h"
#include "../lib/base/msglib.h"

#include "Reorder.h"

#include <vector>

extern "C" {
// methods callable from Python
void initreorder(void);
static PyObject *PyReorder( PyObject *, PyObject *);
static PyObject *PyReorderPrepared( PyObject *, PyObject *);
static PyObject *PyPrealloc( PyObject *, PyObject *);
static PyObject *PyFree( PyObject *, PyObject *);
}


// Quick wrapper around vector<> to see it with multiple types

class ReorderBuf
{
   public:
      ReorderBuf( int size) { resize(size); }

      float32 *asfloat32() { return ((float32 *) &buffer[0]); }
      uint32  *asuint32()  { return ((uint32 *) &buffer[0]); }

      std::vector<uint32>::iterator begin() { return buffer.begin(); }
      std::vector<uint32>::iterator end()   { return buffer.end(); }

      void clear() { buffer.clear(); }
      int  size()  { return buffer.size(); }
      void resize( int size)  { buffer.resize(size); memset( this->asuint32(), 0, this->size()*sizeof(uint32)); }

   protected:
      std::vector<uint32> buffer;
};



// Registration of methods callable from Python

static struct PyMethodDef reorder_methods[] = {
    {"Reorder", PyReorder, METH_VARARGS, "Reorders a ccd buffer"},
    {"Prealloc", PyPrealloc, METH_VARARGS, "Pre-allocs reorder tables"},
    {"Free", PyFree, METH_VARARGS, "Frees reorder tables"},
    {"ReorderPrepared", PyReorderPrepared, METH_VARARGS, "Reorders a ccd buffer"},
    {NULL, NULL, 0, NULL}
};

// C functions which do the actual work

struct reorder_prealloc *c_prealloc( int ccd);
PyObject *c_reorder( struct reorder_prealloc *prealloc, char *data, int size);
void c_free( struct reorder_prealloc *prealloc);



//+Function: initreorder

// Function called by Python import mechanism

// This function is the equivalent of Python classes' __init__, and is
// automaticaly called by the interpreter when importing the module.
//-

void initreorder()
{
        (void) Py_InitModule("reorder", reorder_methods);
        import_libnumarray();
}


struct reorder_prealloc
{
    int ccd;
    AbstractDiagnClass *diagnbuf;
    Reorder *reorder;
    ReorderBuf *pixel_buffer;
    ReorderBuf *slopex_buffer;
    ReorderBuf *slopey_buffer;
};


struct reorder_prealloc *c_prealloc( int ccd)
{
    struct reorder_prealloc *prealloc;

    prealloc = new struct reorder_prealloc;
    memset( prealloc, 0, sizeof(struct reorder_prealloc));
  
    // Stuff valid for all CCDs 
    prealloc->ccd = ccd;
    prealloc->reorder = new Reorder();

    // CCD-specific setup
    switch(ccd)
      {
      case 39:
      prealloc->reorder->Init( "ccd39");
      prealloc->diagnbuf = new OptLoopDiagnClass();

      prealloc->slopex_buffer = new ReorderBuf( prealloc->reorder->getSize());
      prealloc->slopey_buffer = new ReorderBuf( prealloc->reorder->getSize());
      break;

      case 47:
      prealloc->reorder->Init( "ccd47");
      prealloc->diagnbuf = new TechViewerDiagnClass();
      break;

      // Error exit in case of unknown ccd type
      default:
      return NULL;
      }

   prealloc->pixel_buffer = new ReorderBuf( prealloc->reorder->getSize());

   return prealloc;
}

void c_free( struct reorder_prealloc *prealloc)
{
    if (prealloc->reorder) delete prealloc->reorder;
    if (prealloc->diagnbuf) delete prealloc->diagnbuf;
    if (prealloc->slopex_buffer) delete prealloc->slopex_buffer;
    if (prealloc->slopey_buffer) delete prealloc->slopey_buffer;
    if (prealloc->pixel_buffer) delete prealloc->pixel_buffer;

    memset(prealloc, 0, sizeof(struct reorder_prealloc));
}




//+Function: PyPrealloc

static PyObject *PyPrealloc( PyObject *self, PyObject *args)
{
    int ccd;
    struct reorder_prealloc *prealloc;

    if (!PyArg_ParseTuple( args, "i", &ccd))
       return NULL;

    prealloc = c_prealloc( ccd);

    return Py_BuildValue("s#", prealloc, sizeof(struct reorder_prealloc));
}

//+Function: PyFree

static PyObject *PyFree( PyObject *self, PyObject *args)
{
    int dummy;
    struct reorder_prealloc *prealloc;

    if (!PyArg_ParseTuple( args, "s#", &prealloc, &dummy))
       return NULL;

    c_free(prealloc);

    return Py_BuildValue("i", NO_ERROR);
}






PyObject *c_reorder( struct reorder_prealloc *prealloc, char *data, int size)
{
    PyObject *py_return;

    // Load our buffer
    memcpy( prealloc->diagnbuf->data(), data, size);

    prealloc->reorder->BCUPixels2Raster( prealloc->diagnbuf->pixels(), prealloc->pixel_buffer->asuint32());

    // Reorder other stuff if needed
    if (prealloc->ccd == 39)
      {
      prealloc->reorder->BCUSlopeX2Raster( prealloc->diagnbuf->slopes(), prealloc->slopex_buffer->asfloat32());
      prealloc->reorder->BCUSlopeY2Raster( prealloc->diagnbuf->slopes(), prealloc->slopey_buffer->asfloat32());
      }

    // Now build return arrays
    int array_dims[2];
    int ccd_size = (int) sqrt(prealloc->reorder->getSize());
    array_dims[0] = ccd_size;
    array_dims[1] = ccd_size;

    PyArrayObject *pixelarray;
    pixelarray = NA_vNewArray( prealloc->pixel_buffer->asuint32(), tInt32, 2, array_dims);

    switch( prealloc->ccd)
       {
       case 39:
       PyArrayObject *slopexarray;
       PyArrayObject *slopeyarray;
       PyArrayObject *rawarray;

       slopexarray = NA_vNewArray( prealloc->slopex_buffer->asfloat32(), tFloat32, 2, array_dims);
       slopeyarray = NA_vNewArray( prealloc->slopey_buffer->asfloat32(), tFloat32, 2, array_dims);
       rawarray    = NA_vNewArray( prealloc->diagnbuf->pixels(), tInt16, 2, array_dims);

       py_return = Py_BuildValue("{s:O,s:O,s:O,s:O}", "pixels", pixelarray, "slopex", slopexarray, "slopey", slopeyarray, "raw", rawarray);
       Py_DECREF(pixelarray);
       Py_DECREF(slopexarray);
       Py_DECREF(slopeyarray);
       Py_DECREF(rawarray);
       break;

       case 47: 
       py_return = Py_BuildValue("{s:O}", "pixels", pixelarray);
       Py_DECREF(pixelarray);
       break;

       default:
       py_return = NULL;
       }

    return py_return;
}

//+Function: PyReorder
//
// Arguments: 
//             ccd type (int)
//             data (data buffer to reorder)
//             
// Returns:    dict( various buffers depending on ccd type, 'pixels' is always present)
//
//-
static PyObject *PyReorder( PyObject *self, PyObject *args)
{
    char *data;
    int ccd, size;
    PyObject *py_return;
    struct reorder_prealloc *prealloc;
  
    if (!PyArg_ParseTuple( args, "is#", &ccd, &data, &size))
       return NULL;

    prealloc = c_prealloc( ccd);
    if (!prealloc)
      return NULL;

    py_return = c_reorder(prealloc, data, size);

    c_free(prealloc);
    delete prealloc;

    return py_return;

}



//+Function: PyReorderPrepared
//
// Arguments: 
//             prealloc (pre-allocated setup)
//             data (data buffer to reorder)
//             
// Returns:    dict( various buffers depending on ccd type, 'pixels' is always present)
//
//-
static PyObject *PyReorderPrepared( PyObject *self, PyObject *args)
{
    char *data;
    int dummy, size;
    struct reorder_prealloc *prealloc;
  
    if (!PyArg_ParseTuple( args, "s#s#", &prealloc, &dummy, &data, &size))
       return NULL;

    if (!prealloc)
      return NULL;

    return c_reorder( prealloc, data, size);
}

