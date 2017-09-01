//@File: iolib.h
//
// Input/output routines used by BCU programs & others
//@

#ifndef IOLIB_H_INCLUDED
#define IOLIB_H_INCLUDED

#include "../contrib/cfitsio/include/fitsio.h"


// API

unsigned char *ReadASCIIFile( char *path, int *len);
unsigned char *ReadBinaryFile( char *path, int *len);
unsigned char *ReadFitsFile( char *path, int *len);
unsigned char *ReadFitsFileWithLUT( char *path, int *len, int maxdim, int *ndims, long *dims, int **lut, int *lutlen);
int PeekFitsFile( char *path, int *datatype, int *ndims, long **dims);
int ReadFitsKeyword( char *path, char *keyword, int datatype, void *value);


typedef struct {
   int datatype;
   char keyname[8];
   void *value;
   char comment[60];
} fitskeyword;


int WriteFitsFile( char *path, unsigned char *buffer, int type, long *dims, int ndims);
int WriteFitsFileWithLUT( char *path, unsigned char *buffer, int type, long *dims, int ndims, int *lut, int lutlen);
int WriteFitsFileWithKeywords( char *path, unsigned char *buffer, int type, long *dims, int ndims, fitskeyword *keywords, int nkeywords);

int writeFits1D( char *filename, void *buf, int type, int dim1);
int writeFits2D( char *filename, void *buf, int type, int dim1, int dim2);
int writeFits3D( char *filename, void *buf, int type, int dim1, int dim2, int dim3);



// Internal

#include "../contrib/cfitsio/include/fitsio.h"

unsigned char *ReadFitsHDU( fitsfile *fptr, int *len, int maxdim, int *naxis, long *axes, int *getdatasize);

#endif // IOLIB_H_INCLUDED
