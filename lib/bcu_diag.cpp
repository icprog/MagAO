//@File: bcu_diag.c
//
//@

#include <stdlib.h>
#include <string.h>

#include "bcu_diag.h"
#include "diagBuffer.h"
#include "Paths.h"

extern "C" {
#include "base/thrdlib.h"
}

static int debug_output=0;

// --------------- MASTER DIAGNOSTIC DATA ACCESS ------------- //
//
// All method are placed here instead of the .h to avoid problems
// in shared library loading from Python

int AbstractDiagnClass::getCurBinning()
{
   char buf[PATH_MAX+1];

   string lutdir = Arcetri::Paths::CurLUTDir( _ccd);
   memset( buf, 0, PATH_MAX+1);
   readlink( lutdir.c_str(), buf, PATH_MAX);

   // Search for "/binX"
   char *pos = strstr( buf, "bin");
   if (pos)
      return atoi(pos+3);
   else
      return 0;
}

int AbstractDiagnClass::checkBinning()
{
   int newBinning = getCurBinning();
   if (debug_output) printf("checkBinning(): current binning = %d\n", newBinning);
   if (_myBinning != newBinning)
      {
      _myBinning = newBinning;
      return 1;
      }
   else
      return 0;
}
   



AbstractDiagnClass::~AbstractDiagnClass() {}

OptLoopDiagnClass::OptLoopDiagnClass( ) : AbstractDiagnClass()
{
   _pixels.init( "ccd39");
   _slopes.init( "ccd39");
   _ccd = "ccd39";
   checkBinning();
}

void OptLoopDiagnClass::setData( unsigned char *data)
{
   memcpy( &_data, data, sizeof(OptLoopDiagFrame));
   checkBinning();
   if (_myBinning == 0)
       return;
   _pixels.setRaw((uint16*)_data.slopecomp.pixels, (80  /_myBinning) * (80 / _myBinning));
   // Mask out central pixels at 3x3 binning
   if (_myBinning ==3) {
   	uint16* pix = (uint16*) _pixels.raster();
	int pos = 12*26+12;
	pix[pos+0] = 0;
	pix[pos+1] = 0;
	pos = 13*26+12;
	pix[pos+0] = 0;
	pix[pos+1] = 0;
    }
   _slopes.setRaw((float32*)_data.slopecomp.slopes, 1600);
}
int OptLoopDiagnClass::pixels_len() {

   if (_myBinning == 0)
       return 0;
    return (80  /_myBinning) * (80 / _myBinning);
}

int OptLoopDiagnClass::modes_len() { return 672; }
int OptLoopDiagnClass::ffcommands_len() { return 672; }
int OptLoopDiagnClass::distaverages_len() { return 672; }

void OptLoopDiagnClass::pixels_raster_dim( int *dx, int *dy) {
   if (_myBinning == 0)
       return;
   if (dx) *dx = 80 / _myBinning;
   if (dy) *dy = 80 / _myBinning;
}
void OptLoopDiagnClass::pixels_raw_dim( int *dx, int *dy) {
   if (_myBinning == 0)
       return;
   if (dx) *dx = 80 / _myBinning;
   if (dy) *dy = 80 / _myBinning;
}
void OptLoopDiagnClass::slopes_raster_dim( int *dx, int *dy) {
   if (_myBinning == 0)
       return;
   if (dx) *dx = 80 / _myBinning;
   if (dy) *dy = 80 / _myBinning;
}
void OptLoopDiagnClass::slopes_raw_dim( int *dx, int *dy) {
   if (dx) *dx = 1600;
   if (dy) *dy = 1;
}

void *OptLoopDiagnClass::data() { return &_data; }

uint16 *OptLoopDiagnClass::pixels_raster()  {  return (uint16 *)  _pixels.raster(); }
uint16 *OptLoopDiagnClass::pixels_raw()     {  return (uint16 *)  _data.slopecomp.pixels; }
float32 *OptLoopDiagnClass::slopes_raster() {  return (float32 *) _slopes.raster(); }
float32 *OptLoopDiagnClass::slopes_raw()    {  return (float32 *) _data.slopecomp.slopes; }

float32 *OptLoopDiagnClass::modes() { return (float32 *) _data.adsec.Modes; }
float32 *OptLoopDiagnClass::ffcommands() { return (float32 *) _data.adsec.FFCommand; }
float32 *OptLoopDiagnClass::distaverages() { return (float32 *) _data.adsec.DistAverage; }

TechViewerDiagnClass::TechViewerDiagnClass() : AbstractDiagnClass()
{
   _pixels.init( "ccd47");
   _ccd = "ccd47";
}

void TechViewerDiagnClass::setData( unsigned char *data)
{
   checkBinning();
   if (_myBinning == 0)
       return;
   int size = 1024*1024 / (_myBinning*_myBinning)*2 + 8*4;
   memcpy( &_data, data, size);
   _pixels.setRaw((uint16 *)_data.tech_viewer.pixels, 1024*1024 / (_myBinning*_myBinning));
}
void *TechViewerDiagnClass::data() { return &_data; }
int TechViewerDiagnClass::pixels_len() { return 1024*1024 / (_myBinning*_myBinning); }
void TechViewerDiagnClass::pixels_raster_dim( int *dx, int *dy) {
   if (_myBinning == 0)
       return;
   if (dx) *dx = 1024 / _myBinning;
   if (dy) *dy = 1024 / _myBinning;
}
void TechViewerDiagnClass::pixels_raw_dim( int *dx, int *dy) {
   if (_myBinning == 0)
       return;
   if (dx) *dx = 1024 / _myBinning;
   if (dy) *dy = 1024 / _myBinning;
}

uint16 *TechViewerDiagnClass::pixels_raster()  {  return (uint16 *) _pixels.raster(); }
uint16 *TechViewerDiagnClass::pixels_raw()     {  return (uint16 *) _data.tech_viewer.pixels; }

ThorlabsDiagnClass::ThorlabsDiagnClass() : AbstractDiagnClass() {}

void ThorlabsDiagnClass::setData( unsigned char *data)
{
   memcpy( &_data, data, sizeof(ThorlabsDiagFrame));
}
void *ThorlabsDiagnClass::data() { return &_data; }

uint16 *ThorlabsDiagnClass::pixels_raster()  {  return (uint16 *) _data.pixels; }
int ThorlabsDiagnClass::pixels_len() { return 1024*768; }
void ThorlabsDiagnClass::pixels_raster_dim( int *dx, int *dy) {
   if (dx) *dx = 1024;
   if (dy) *dy = 768;
}

IrtcDiagnClass::IrtcDiagnClass() : AbstractDiagnClass() {}

void IrtcDiagnClass::setData( unsigned char *data)
{
   memcpy( &_data, data, sizeof(IrtcDiagFrame));
}
void *IrtcDiagnClass::data() { return &_data; }

uint16 *IrtcDiagnClass::pixels_raster()  {  return (uint16 *) _data.pixels; }
int IrtcDiagnClass::pixels_len() { return 320*256; }
void IrtcDiagnClass::pixels_raster_dim( int *dx, int *dy) {
   if (dx) *dx = 1024;
   if (dy) *dy = 256;
}

PiscesDiagnClass::PiscesDiagnClass() : AbstractDiagnClass() {}

void PiscesDiagnClass::setData( unsigned char *data)
{
   memcpy( &_data, data, sizeof(PiscesDiagFrame));
}
void *PiscesDiagnClass::data() { return &_data; }

uint16 *PiscesDiagnClass::pixels_raster()  {  return (uint16 *) _data.pixels; }
int PiscesDiagnClass::pixels_len() { return 1024*1024; }
void PiscesDiagnClass::pixels_raster_dim( int *dx, int *dy) {
   if (dx) *dx = 1024;
   if (dy) *dy = 1024;
}

// -------------------- OPT LOOP SAVE INTERFACE -------- //

// Define this if you are not interested in the optical loop interface (e.g. Python wrapper)
#ifndef NO_OPTLOOP

int OptLoopSave_Start( optsave_cmd cmd, string &savedFile, bool wait, int timeout) {

   char target[] = "OPTLOOPDIAGN00";
   int seq = thSendMsg( sizeof(cmd), target, OPTSAVE, 0, (void*)&cmd);
   if ((IS_ERROR(seq)) || (!wait))
      return seq;

   int errCode;
   MsgBuf *msg = thWaitMsg( ANY_MSG, "*", seq, timeout, &errCode);
   if (!msg)
      return errCode;

   optsave_reply reply = *( (optsave_reply*) MSG_BODY(msg));
   savedFile = reply.outfile;
   return NO_ERROR; 
}

int OptLoopSave_Sync( int nframes) {

   int timeout = 3000;   // Replies are supposed to come at 1Hz

   while (1) {
      int errCode;
      MsgBuf *msg = thWaitMsg( OPTSAVE_REPLY, "*", 0, timeout, &errCode);
      if (!msg)
         return errCode;

      optsave_reply reply = *( (optsave_reply*) MSG_BODY(msg));
      if (reply.saved >= nframes)
         return NO_ERROR;
   }
}

#endif






