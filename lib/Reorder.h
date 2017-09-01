
#ifndef CONVERT_H_INCLUDED
#define CONVERT_H_INCLUDED

#include "aotypes.h"

#include <vector>
#include <string>
using namespace std;

class Reorder {

public:
   Reorder();

public:
   void Init( string ccd, int size=-1);
   void Init( vector<int> &pixelTable, vector<int> &slopeTable);

   // BCU -> Raster order

   void BCUPixels2Raster( uint32 *pixels, uint32 *dest);
   void BCUPixels2Raster( uint16 *pixels, uint32 *dest);
   void BCUPixels2Raster( uint16 *pixels, uint16 *dest);

   void BCUSlopeX2Raster( float32 *slopes, float32 *dest, int clear=1);
   void BCUSlopeY2Raster( float32 *slopes, float32 *dest, int clear=1);

   // Raster order -> BCU

   void BCURaster2Pixels( uint32 *raster, uint32 *dest);
   void BCURaster2Pixels( uint32 *raster, uint16 *dest);
   void BCURaster2Pixels( uint16 *raster, uint16 *dest);

   void BCURaster2SlopeX( float32 *slopes, float32 *dest);


   int getSize();

   // LUT access

   int *getPixelLUT( int *len) { if (len) *len = pixelTable.size(); return &(pixelTable[0]); }
   int *getSlopeLUT( int *len) { if (len) *len = slopexTable.size(); return &(slopexTable[0]); }



protected:
   void LoadTable( vector<int> &table, string filename);


   string _ccd;
   int _size;

   bool _do_reorder;

   vector<int> pixelTable;
   vector<int> slopexTable;
   vector<int> slopeyTable;

};
#endif // CONVERT_H_INCLUDED


