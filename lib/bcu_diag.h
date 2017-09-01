//+File: bcu_diag.h
//
// Header file for bcu diagnostics
//
//-


#ifndef BCUDIAG_H_INCLUDED
#define BCUDIAG_H_INCLUDED


#include "aotypes.h"
#include "diagBuffer.h"

#include "AdsecDiagnLibStructs.h"
#include "../OptLoopDiagnostic/OptLoopDiagnosticStructs.h"
#include "../BCUCtrl/ParamBlockSelector.h"


///////////////////////////////////////////////////////////////////////
//
// BCU buffer structures
//
//////////////////////////////////////////////////////////////

// Define this when the slopes are in the slope computer diagnostics
//#define HAS_SLOPES_IN_SLOPECOMPUTER

// OLD slope computer diagnostic frame

typedef struct
{
        uint32 framenumber;             // header (4 words)
        uint32 paramblockselector;
        uint32 timehistorypointer;
        uint32 dummy;
        uint32 pixels[ 3200 + 3200];    // slope pixels and contur pixels
        float32 average_intensity;      // frame intensity are (4 words)
        float32 boh;
        float32 dummy2[2];
        float32 slopes[ 1600];          // slopes 
        uint32 start_rtr[4];            // start_rtr (4 words)
        float32 modes[48];              // Modes computed by DSP boards (48 elements)
        uint32 start_mm[4];             // start_mm (4 words)
        float32 commands[48];           // Mirror commands (48 modes)
        uint32 framenumber_check;       // footer (4 words)
        uint32 paramblockselector_check;
        uint32 timehistorypointer_check;
        uint32 dummy_check;

} old_slopecomp_diagframe;

// A slope computer diagnostic frame with pixels only

typedef struct
{
        uint32 framenumber;             // header (4 words)
        uint32 paramblockselector;
        uint32 timehistorypointer;
        uint32 dummy;
        uint16 pixels[ 3200 + 3200];    // all frame pixels
        float32 average_intensity;      // frame intensity are (4 words)
        float32 boh;
        float32 dummy2[2];
        uint32 framenumber_check;       // footer (4 words)
        uint32 paramblockselector_check;
        uint32 timehistorypointer_check;
        uint32 dummy_check;

} slopecomp_diagframe_pixels_only;

// A slope computer diagnostic frame with both pixels and slopes

typedef struct
{
        uint32 framenumber;             // header (4 words)
        uint32 paramblockselector;
        uint32 timehistorypointer;
        uint32 dummy;
        uint16 pixels[ 3200 + 3200];    // all frame pixels
        float32 average_intensity;      // frame intensity are (4 words)
        float32 boh;
        float32 dummy2[2];
        float32 slopes[ 1600];          // slopes 
        uint32 framenumber_check;       // footer (4 words)
        uint32 paramblockselector_check;
        uint32 timehistorypointer_check;
        uint32 dummy_check;

} slopecomp_diagframe_pixels_slopes;

#ifdef HAS_SLOPES_IN_SLOPECOMPUTER
typedef slopecomp_diagframe_pixels_slopes slopecomp_diagframe;
#else
typedef slopecomp_diagframe_pixels_only slopecomp_diagframe;
#endif


typedef struct
{
        uint32 framenumber;             // header (4 words)
        uint32 paramblockselector;
        uint32 timehistorypointer;
        uint32 dummy;
        uint16 pixels[1024*1024];    // slope pixels and contur pixels
        uint32 framenumber_check;       // footer (4 words)
        uint32 paramblockselector_check;
        uint32 timehistorypointer_check;
        uint32 dummy_check;

} techviewer_diagframe;

typedef struct
{
	uint32 framenumber;		// Header (4 words)
	uint32 pointer;
	uint32 paramblockselector;
	uint32 dummy;
	float32 dist_accumulator[4];	// Accumulated linearized position
	float32 curr_accumulator[4];	// Accumulated current
	float32 dist_variance[4];	// Linearized position variance
	float32 curr_variance[4];	// current variance
	float32 modes[4];		// Modes
	float32 new_delta_command[4];	// Delta command to apply
	float32 ff_command[4];		// Delta Feed-Forward command
	float32 ff_current[4];		// Delta Feed-Forward current
	float32 ff_pure_current[4];	// Integrated Feed-Forward current
	uint32 framenumber_check;	// Footer (4 words)
	uint32 pointer_check;
	uint32 paramblockselector_check;
	uint32 dummy_check;
} secondary_diagframe;


typedef struct
{
   uint32 dummy;
} switchbcu_diagframe;

///////////////////////////////////////////////////////////////////////
//
// Master diagnostic definitions
//
//////////////////////////////////////////////////////////////

typedef struct
{
   uint32 bitmask;
   slopecomp_diagframe_pixels_slopes slopecomp;
   MirrorOutputDiagn adsec;
} OptLoopDiagFrame;

typedef struct
{
   techviewer_diagframe tech_viewer;
} TechViewerDiagFrame;

// +2 per header
typedef struct
{
   uint16 pixels[1024*768];
} ThorlabsDiagFrame;

typedef struct
{
   uint16 pixels[320*256];
} IrtcDiagFrame;

typedef struct
{
   uint16 pixels[1024*1024];
} PiscesDiagFrame;

///////////////////////////////////////////////////////////////////
//
// Master diagnostic data access classes
//
// These classes allow access to a BCU diagnostic buffer and permit,
// when ccd is specified, to access data in raster order.
//
// Usage: instantiate either OptLoopDiagnClass or TechViewerDiagnClass (using an AbstractDiagnClass *)
// use setData() to pass the bcu diagnostic buffer
// and use pixels(), slopes() and data() to access diagnostic data
//
// Binning is automatically taken into account using the current ccd binning.

class AbstractDiagnClass
{
public:
   AbstractDiagnClass() {_ccd=""; _myBinning=0; };
   virtual ~AbstractDiagnClass();

   virtual void setData( unsigned char * /*data*/) {}

   virtual void ident() { printf("abstract!!\n"); }

public:
   virtual void *data() = 0;

   virtual int pixels_len() { return 0;}

   virtual uint16 *pixels_raster() { return NULL; }
   virtual uint16 *pixels_raw(){ return NULL; }
   virtual float32 *slopes_raster(){ return NULL; }
   virtual float32 *slopes_raw(){ return NULL; }

   virtual void pixels_raster_dim( int * /*dx*/, int * /*dy*/) {}
   virtual void pixels_raw_dim( int * /*dx*/, int * /*dy*/) {}

   virtual void slopes_raster_dim( int * /*dx*/, int * /*dy*/) {}
   virtual void slopes_raw_dim( int * /*dx*/, int * /*dy*/) {}


protected:
   int getCurBinning();
   int checkBinning();

   string _ccd;
   int _myBinning;

};

class OptLoopDiagnClass : public AbstractDiagnClass
{
public:
   OptLoopDiagnClass();

   void setData( unsigned char *data);
   void *data();

   void ident() { printf("OptLoop\n"); }

   int pixels_len();

   uint16 *pixels_raster();
   uint16 *pixels_raw();
   void pixels_raster_dim( int *dx, int *dy);
   void pixels_raw_dim( int *dx, int *dy);

   float32 *slopes_raster();
   float32 *slopes_raw();
   void slopes_raster_dim( int *dx, int *dy);
   void slopes_raw_dim( int *dx, int *dy);

   float32 *modes();
   int modes_len();

   float32 *ffcommands();
   int ffcommands_len();

   float32 *distaverages();
   int distaverages_len();

   uint32 framesCounter() { return _data.slopecomp.framenumber; }
   uint32 skipFrameCounter() { return _data.adsec.safeSkipFrameCounter; }
   uint32 pendingFrameCounter() { return _data.adsec.pendingSkipFrameCounter; }
   uint32 wfsGlobalTimeout() { return _data.adsec.wfsGlobalTimeout; }
   uint32 flTimeout() { return _data.adsec.flTimeout; }
   uint32 crcErrors() { return _data.adsec.crcErrors; }
   uint32 timestamp() { return _data.adsec.timestamp; }
   uint32 mirrorFrameCounter() { return _data.adsec.header.MirrorFrameCounter; }
   bool isLoopClosed() { return (_data.adsec.header.ParamBlockSelector & PBS_FASTLINK) != 0; }


//protected:
public:
   OptLoopDiagFrame _data;
   ccdFrame _pixels;
   Slopes _slopes;
};

class TechViewerDiagnClass : public AbstractDiagnClass
{
public:
   TechViewerDiagnClass();

   void ident() { printf("TechView\n"); }

   void setData( unsigned char *data);
   void *data();
   int pixels_len();

   uint16 *pixels_raster();
   uint16 *pixels_raw();
   void pixels_raster_dim( int *dx, int *dy);
   void pixels_raw_dim( int *dx, int *dy);
 
protected:
   TechViewerDiagFrame _data;
   ccdFrame _pixels;
};

class ThorlabsDiagnClass : public AbstractDiagnClass
{
public:
   ThorlabsDiagnClass();

   void ident() { printf("Thorlabs\n"); }

public:
   void setData( unsigned char *data);
   void *data();
   int pixels_len();
   
   uint16 *pixels_raster();
   uint16 *pixels_raw() { return pixels_raster(); } ;
   void pixels_raster_dim( int *dx, int *dy);

protected:
   ThorlabsDiagFrame _data;
   ccdFrame _pixels;
};

class IrtcDiagnClass : public AbstractDiagnClass
{
public:
   IrtcDiagnClass();

   void ident() { printf("IRTC\n"); }

public:
   void setData( unsigned char *data);
   void *data();
   int pixels_len();
   
   uint16 *pixels_raster();
   uint16 *pixels_raw() { return pixels_raster(); } ;
   void pixels_raster_dim( int *dx, int *dy);

protected:
   IrtcDiagFrame _data;
   ccdFrame _pixels;
};

class PiscesDiagnClass : public AbstractDiagnClass
{
public:
   PiscesDiagnClass();

   void ident() { printf("PISCES\n"); }

public:
   void setData( unsigned char *data);
   void *data();
   int pixels_len();
   
   uint16 *pixels_raster();
   uint16 *pixels_raw() { return pixels_raster(); } ;
   void pixels_raster_dim( int *dx, int *dy);

protected:
   PiscesDiagFrame _data;
   ccdFrame _pixels;
};

// -------------------- OPT LOOP saving interface --------------- //

int OptLoopSave_Start( optsave_cmd cma, string &savedFile, bool wait=true, int timeout=1000);
int OptLoopSave_Sync( int nframes);

#endif //BCUDIAG_H_INCLUDED

