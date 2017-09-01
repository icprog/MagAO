#ifndef DIAGBUFFER_H_INCLUDED
#define DIAGBUFFER_H_INCLUDED

#include <string>
using namespace std;

#include "Reorder.h"

extern "C" {
#include "iolib.h"
}

class diagBuffer {

public:
   diagBuffer() { _raster = NULL; _dims = NULL; _len=0;}
   virtual ~diagBuffer() { clean(); }

   // VIRTUALS 
   virtual void prepare()=0;

   virtual void init( string /*ccd*/) {};

   virtual int load( string /*filename*/) { return 0; }
   virtual int save( string /*filename*/) { return 0; }

   virtual int setRaw( void * /*data*/, int /*len*/) { return 0; }

   void *raster() { return _raster; }
   virtual void *raw() { return NULL; }

   // NON - VIRTUALS

   void setRaster( void * /*data*/, int /*len*/);
   void *copyRaster();

   int len() { return _len; }

protected:

   void clean() {
      // [TODO] segfaults here!
        /*
       if (_raster) free(_raster); 
       if (_dims) free(_dims);
       */
   }
   void alloc( int len, bool zero=false);

   void *_raster;
   int _len;

   long *_dims;
   int _bpe;    // bytes per element
   Reorder _reorder;
};


// CCD frame / background

class ccdFrame : public diagBuffer {

public:
   ccdFrame( string ccd) : diagBuffer() { prepare(); init( ccd); }
   ccdFrame() : diagBuffer() {prepare(); }
   ~ccdFrame() {};

   void init( string ccd);
   void prepare();

   int load (string filename);
   int save( string filename);
   int setRaw( void *data, int len);
   void *raw();
};


// Slopes


class Slopes : public diagBuffer {

public:
   Slopes( string ccd) : diagBuffer() { prepare(); init( ccd); }
   Slopes() : diagBuffer() {prepare();  }
   ~Slopes() {};

   void prepare();
   void init( string ccd);

   int load (string filename);
   int loadRaw (string filename);
   int save( string filename);
   int setRaw( void *data, int len);
   void *raw();
};

#endif // DIAGBUFFER_H_INCLUDED
