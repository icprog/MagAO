
#include "Reorder.h"

#include "Paths.h"

#include <iostream>
#include <fstream>
#include <cstring>
using namespace std;

static int debug_output = 0;

Reorder::Reorder()
{
   _size = -1;
}

// Init
//
// Initializes reorder object with the current system tables, given the ccd.
//
// if ccd == "noreorder" then size must be specified, otherwise it is ignored (the table size is used instead)

void Reorder::Init( string ccd, int size)
{
   _ccd  = ccd;

   if (ccd == "noreorder")
      {
      _do_reorder = false;
      _size = size;
      }
   else
      {
      _do_reorder = true;
      if (debug_output) printf("Reorder::Init(): loading tables for %s\n",  ccd.c_str());
      LoadTable( pixelTable, Arcetri::Paths::CurDisplayLUT( ccd));
      LoadTable( slopexTable, Arcetri::Paths::CurSlopeXLUT( ccd));
      LoadTable( slopeyTable, Arcetri::Paths::CurSlopeYLUT( ccd));
      if (debug_output) printf("Reorder::Init(): loading done\n");
      }
}

// Init
//
// Initializes reorder object with a specific table

void Reorder::Init( vector<int>& pixelTable, vector<int>& slopeTable)
{
   _do_reorder = true;
   this->pixelTable = pixelTable;
   this->slopexTable = slopeTable;
   this->slopeyTable = slopeTable;
   if (debug_output) printf("Reorder::Init() done\n");
}


void Reorder::LoadTable( vector<int> &table, string filename)
{
   int value;
   table.clear();

   if (debug_output) printf("Reading table %s\n", filename.c_str());

   ifstream f(filename.c_str());

   // When >> fails, an error bit is set and f.good() fails.
   f >> value;
   while (f.good())
      {
      table.push_back(value);
      //printf("LoadTable (%s): element %i value %d\n", filename.c_str(), i++, value);

      f >> value;
      }

}


int Reorder::getSize()
{
   if (_do_reorder)
      return pixelTable.size();
   else
      return _size;
}

void Reorder::BCURaster2Pixels( uint32 *raster, uint32 *dest)
{
   if (debug_output) printf("Reorder::BCURaster2Pixels( uint32 *, uint32 *)\n");
   if ((!raster) || (!dest))
      return;
   if (debug_output) printf("Starting reorder, do_reorder=%d\n", _do_reorder);

   if (_do_reorder)
      {
      int maxpos = pixelTable.size();
      for (unsigned int i=0; i< pixelTable.size(); i++)
         {
         int pos = pixelTable[i];
         if ((pos>=0) && (pos < maxpos))
            dest[ pixelTable[i]] = raster[i];
         }
      }
   else
      memcpy( dest, raster, sizeof(uint32)*_size);
}

void Reorder::BCURaster2Pixels( uint32 *raster, uint16 *dest)
{
   if (debug_output) printf("Reorder::BCURaster2Pixels( uint32 *, uint 16*)\n");
   if ((!raster) || (!dest))
      return;
   if (debug_output) printf("Starting reorder, do_reorder=%d\n", _do_reorder);

   if (_do_reorder) {
      int maxpos = pixelTable.size();
      for (unsigned int i=0; i< pixelTable.size(); i++) {
         int pos = pixelTable[i];
         if ((pos>=0) && (pos < maxpos))
            dest[ pixelTable[i]] = raster[i];
      }
   } else {
      for (int i=0; i<_size; i++)
         dest[i] = raster[i];
   }
}

void Reorder::BCURaster2Pixels( uint16 *raster, uint16 *dest)
{
   if (debug_output) printf("Reorder::BCURaster2Pixels( uint16 *, uint 16*)\n");
   if ((!raster) || (!dest))
      return;

   if (debug_output) printf("Starting reorder, do_reorder=%d\n", _do_reorder);

   if (_do_reorder) {
      int maxpos = pixelTable.size();
      if (debug_output) printf("Maxpos: %d\n", maxpos);
      for (unsigned int i=0; i< pixelTable.size(); i++) {
         int pos = pixelTable[i];
         if (debug_output) printf("i=%d, pos=%d\n", i, pos);
         if ((pos>=0) && (pos < maxpos))
            {
            dest[ pixelTable[i]] = raster[i];
            if (debug_output) printf("dest[%d] = raster[%d]\n", pixelTable[i], i);
            }
      }
   } else {
      for (int i=0; i<_size; i++)
         dest[i] = raster[i];
   }
}

void Reorder::BCUPixels2Raster( uint32 *pixels, uint32 *dest)
{
   if ((!pixels) || (!dest))
      return;

   if (_do_reorder) {
      int maxpos = pixelTable.size();
      for (unsigned int i=0; i< pixelTable.size(); i++) {
         int pos = pixelTable[i];
         if ((pos>=0) && (pos< maxpos))
            {
            dest[i] = pixels[pos];
            //printf("dest[%d] = pixels[%d]\n", i, pos);
            }
      }
   } else
      memcpy( dest, pixels, sizeof(uint32)*_size);
}

void Reorder::BCUPixels2Raster( uint16 *pixels, uint32 *dest)
{
   if ((!pixels) || (!dest))
      return;

   if (_do_reorder) {
      int maxpos = pixelTable.size();
      for (unsigned int i=0; i< pixelTable.size(); i++) {
         int pos = pixelTable[i];
         if ((pos>=0) && (pos< maxpos))
            {
            dest[i] = pixels[pos];
            //printf("dest[%d] = pixels[%d]\n", i, pos);
            }
      }
   } else {
      for (int i=0; i<_size; i++)
         dest[i] = pixels[i];
   }
}

void Reorder::BCUPixels2Raster( uint16 *pixels, uint16 *dest)
{
   if (debug_output) printf("Reorder::BCUPixels2Raster()\n");

   if ((!pixels) || (!dest))
      {
      printf("Reorder::BCUPixels2Raster(): arguments are NULL, skipping\n");
      return;
      }


   if (_do_reorder) {
      int maxpos = pixelTable.size();
      for (unsigned int i=0; i< pixelTable.size(); i++) {
         int pos = pixelTable[i];
         if ((pos>=0) && (pos< maxpos))
            {
            dest[i] = pixels[pos];
            //printf("dest[%d] = pixels[%d]\n", i, pos);
            }
      }
   } else {
      for (int i=0; i<_size; i++)
         dest[i] = pixels[i];
   }

}

void Reorder::BCUSlopeX2Raster( float32 *slopes, float32 *dest, int clear)
{
   if (debug_output) printf("Reorder::BCUSlopeX2Raster()\n");

   if ((!slopes) || (!dest))
      return;

   for (unsigned int i=0; i< slopexTable.size(); i++)
      if (slopexTable[i] >=0)
         {
         dest[i] = slopes[ slopexTable[i]];
         //printf("dest[%d] = slopes[%d]\n", i, slopexTable[i]);
         }
      else if (clear)
         dest[i] = 0;
}

void Reorder::BCUSlopeY2Raster( float32 *slopes, float32 *dest, int clear)
{
   if ((!slopes) || (!dest))
      return;

   for (unsigned int i=0; i< slopeyTable.size(); i++)
      if (slopeyTable[i] >=0)
         dest[i] = slopes[ slopeyTable[i]];
      else if (clear)
         dest[i] = 0;
}


void Reorder::BCURaster2SlopeX( float32 *raster, float32 *slopes)
{
   if ((!slopes) || (!raster))
      return;

   for (unsigned int i=0; i< slopexTable.size(); i++)
      if (slopexTable[i] >=0)
         slopes[ slopexTable[i]] = raster[i];
}

