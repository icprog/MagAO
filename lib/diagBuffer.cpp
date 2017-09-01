
#include <string>
#include <cstring>
using namespace std;

extern "C" {
#include "iolib.h"
}

#include "diagBuffer.h"

static int debug_output =0;

void diagBuffer::alloc( int len, bool zero)
{
   if ((!_raster) || (_len != len))
      {
      clean();
      _raster = (void *) malloc( _bpe*len);
      if (debug_output) printf("diagBuffer::alloc(%d): allocated %d bytes\n", len, _bpe*len);
      if (zero)
          memset( _raster, 0, _bpe*len);
      _len = len;
      }
}

void diagBuffer::setRaster( void *data, int len)
{
   alloc(len);
   memcpy( _raster, data, _bpe*len);
}

void *diagBuffer::copyRaster()
{
   void *data = malloc( _len * _bpe);
   memcpy( data, _raster, _len * _bpe);
   return data;
}

// CCD frame / background

void ccdFrame::init( string ccd)
{
   _reorder.Init( ccd);
}

void ccdFrame::prepare()
{
   _raster = NULL;
   _bpe = sizeof(uint16);
   _dims = (long *)malloc(sizeof(long)*2);
   _len=0;
   if (debug_output) printf("ccdFrame::prepare(): _bpe is %d\n", _bpe);
}

int ccdFrame::load( string filename)
{
   int *lut, lutlen;
   vector<int> pixelTable;
   vector<int> slopeTable;

   clean();

  if (debug_output) printf("Reading %s\n", filename.c_str());
   _raster = ReadFitsFileWithLUT( (char *)filename.c_str(), &_len, 2, NULL, _dims, &lut, &lutlen);
   if (lut)
	{
   	for (int i=0; i<lutlen; i++)
      		pixelTable.push_back(lut[i]);
   	_reorder.Init( pixelTable, slopeTable);
   	free(lut);
	}
   if (debug_output) printf("Len: %d\n", _len);
   return true;
}

int ccdFrame::save( string filename)
{
   int lutlen;
   int *lut = _reorder.getPixelLUT( &lutlen);
   WriteFitsFileWithLUT((char *) filename.c_str(), (unsigned char *)_raster, TSHORT, _dims, 2, lut, lutlen);
   return true;
}

int ccdFrame::setRaw( void *data, int len)
{
   if (debug_output) printf("ccdFrame::setRaw()\n");

   alloc(len);
   _reorder.BCUPixels2Raster( (uint16*)data, (uint16*)_raster);
   return true;
}

void *ccdFrame::raw()
{
   void *data = malloc( _len * _bpe);
   _reorder.BCURaster2Pixels( (uint16*)_raster, (uint16*)data);
   return data;
}


// Slopes

void Slopes::init( string ccd)
{
   _reorder.Init( ccd);
}

void Slopes::prepare()
{
   _raster = NULL;
   _bpe = sizeof(float32);
   _len=0;
   _dims = (long *)malloc(sizeof(long)*2);
}

int Slopes::load( string filename)
{
   int *lut, lutlen;
   vector<int> pixelTable;
   vector<int> slopeTable;
   clean();

   _raster = ReadFitsFileWithLUT( (char *)filename.c_str(), &_len, 2, NULL, _dims, &lut, &lutlen);
   if (lut)
	{
   	for (int i=0; i<lutlen; i++)
      		slopeTable.push_back(lut[i]);
   	_reorder.Init( pixelTable, slopeTable);
   	free(lut);
	}
   return true;
}

int Slopes::loadRaw( string filename) {

   int len;
   void *data = ReadFitsFile( (char *)filename.c_str(), &len);
   setRaw(data, len);
   if (data)
      free(data);
}
   

int Slopes::save( string filename)
{
   int lutlen;
   int *lut = _reorder.getPixelLUT( &lutlen);
   WriteFitsFileWithLUT( (char *)filename.c_str(), (unsigned char *)_raster, TFLOAT, _dims, 2, lut, lutlen);
   return true;
}

int Slopes::setRaw( void *data, int /*len*/)
{
   //alloc(len);
   alloc(80*80, true);
   _reorder.BCUSlopeX2Raster( (float32*)data, (float32*)_raster);
   return true;
}

void *Slopes::raw()
{
   void *data = malloc( _len * _bpe);
   _reorder.BCURaster2SlopeX( (float32*)_raster, (float32*)data);
   return data;
}

