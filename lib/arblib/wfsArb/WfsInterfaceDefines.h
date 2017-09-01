
#ifndef WFSARB_INTERFACE_DEFINES_H
#define WFSARB_INTERFACE_DEFINES_H

#include <string>
using namespace std;

namespace Arcetri {
namespace Wfs_Arbitrator {

   // Parameters for wfs control policy

enum wfsPolicy  {

   wfsPolicy_AUTO=0,
   wfsPolicy_MANUAL=1,
   wfsPolicy_UNCHANGED=2
};


  // Data structures:

 // Star description for acquire ref

  class prepareAcquireRefParams
   {
      public:
      string Instr;      // Instrument, for example "IRTC" or "LUCIFER"
      string AOMode;     // either "TT" or "AO"
      float SOCoords[2]; // Expected scientific object position in focal plane mm from the telescope FOV center.
      float ROCoords[2]; // Expected star position in focal plane mm from the telescope FOV center.
      float Elevation;   // Telescope elevation
      float RotAngle;    // Telescope rotator angle
      float GravAngle;   // Telescope rotator angle w/ respect to gravity
      float Mag;         // Expected star magnitude.
      float Color;       // Expected star B-V color index.
      float r0;

      prepareAcquireRefParams() {}
      prepareAcquireRefParams( const prepareAcquireRefParams &b) { 
         Instr  = b.Instr;
         AOMode = b.AOMode;
         SOCoords[0] = b.SOCoords[0];
         SOCoords[1] = b.SOCoords[1];
         ROCoords[0] = b.ROCoords[0];
         ROCoords[1] = b.ROCoords[1];
         Elevation = b.Elevation;
         RotAngle = b.RotAngle;
         GravAngle = b.GravAngle;
         Mag = b.Mag;
         Color = b.Color;
         r0 = b.r0;
      }
   };


 class prepareAcquireRefResult
   {
      public:
      float freq;        // HO frequency
      int binning;       // HO binning
      float TTmod;       // TT modulation amplitude
      int Fw1Pos;        // Filter wheel positions
      int Fw2Pos;
      float gain;
      string pupils;
      string base;
      string rec;
      string filtering;
      int decimation;
      int ovsFreq;
      int nModes;

      void Dump() {
         printf("freq      : %f\n", freq);
         printf("nModes    : %d\n", nModes);
         printf("binning   : %d\n", binning);
         printf("TTmod     : %f\n", TTmod);
         printf("Fw1Pos    : %d\n", Fw1Pos);
         printf("Fw2Pos    : %d\n", Fw2Pos);
         printf("gain      : %f\n", gain);
         printf("pupils    : %s\n", pupils.c_str());
         printf("base      : %s\n", base.c_str());
         printf("rec       : %s\n", rec.c_str());
         printf("decimation: %d\n", decimation);
      }
   };

  typedef struct
{
   string config;        // Power on configuration  (required)
   string boardSetup;    // Optional board setup
   string sourceSetup;   // Optional calibration unit setup
   bool opticalSetup;    // Perform optical setup 
} powerOnParams;


  typedef struct
   {
      float deltaXY[2];  // Offset from initial position in focal plane mm  
      int  nModes;
      float freq;
      int  bin;
      float TTmod;
      int Fw1Pos;
      int Fw2Pos; 
      float starMag;
      char pupils[64];
      char base[64];
      char rec[64];
      char filtering[64];
      float gain;
      int decimation;
      char CLbase[64];
      char CLrec[64];
      float CLfreq;
      unsigned char TVframe[256*256+2*sizeof(int)];
   } acquireRefResult;

   // Struct for getTVSnap output

   typedef struct {
      bool frameValid;
      unsigned char TVframe[256*256+2*sizeof(int)];
   } snapParams;

  // Parameters for modify AO

   typedef struct
   {
      float freq;     // Requested integration time (-1 = no change)
      int   Binning;   // Requested binning (-1 = no change)
      float TTmod;     // Requested tip-tilt modulation (-1 = no change)
      int Fw1Pos;      // Requested FW1 position (-1 = no change)
      int Fw2Pos;      // Requested FW2 position (-1 = no change)
      bool checkCameralens; // Check cameralens position before applying
   } modifyAOparams;

   // Parameters for getsnap

   typedef struct
   {
      float deltaXY[2];  // Offset in mm
      float starMag;     // star magnitude
   } checkRefResult;

   // Parameters for getsnap

   typedef struct
   {
      unsigned char TVframe[256*256];
   } getTVSnapResult;

   // Parameters for refine loop

   static const int wfsNUM_ZERNIKE = 64;  // Number of Zernike modes that can be corrected using slope nulls by WFS

   typedef struct
   {
      float zmodes[wfsNUM_ZERNIKE];
   } wfsLoopParams;

   typedef struct
   {
	   wfsPolicy ccd39;
	   wfsPolicy adc;
	   wfsPolicy tt;
	   wfsPolicy fw1;
	   wfsPolicy rerot;
   } SetHoArmPolicy;

   typedef struct
   {
	   wfsPolicy ccd47;
	   wfsPolicy fw2;
   } SetTvArmPolicy;

   typedef struct
   {
	   wfsPolicy stages;
	   wfsPolicy cube;
	   wfsPolicy lamp;
   } PointAndSourcePolicy;

   typedef struct
   {
      string _outputFile;
   } saveStatusParams;

   typedef struct
   {
      int _nFrames;
      bool _saveFrames;
      bool _saveSlopes;
      bool _saveModes;
      bool _saveCommands;
      bool _savePositions;
      string _trackNum;
      bool _savePsf;
      int _nFramesPsf;
      bool _saveIrtc;
      int _nFramesIrtc;
      bool _savePisces;
      int _nFramesPisces;
   } saveOptLoopDataParams;

   typedef struct
   {
      bool _enableWFS;
      bool _enableOVS;
   } enableDisturbParams;

   typedef struct
   {
      double offsetX;
      double offsetY;
      bool brake;
   } offsetXYparams;

   typedef struct
   {
      double offsetZ;
   } offsetZparams;

   typedef struct
   {
      double modes[64];
   } correctModesParams;

   typedef struct
   {
      string source;
      float magnitude;
   } setSourceParams;

   typedef struct
   {
      bool enable;
   } antiDriftParams;

   typedef struct
   {
      bool rerotTrack;
      bool adcTrack;
      bool lensTrack;
   } autoTrackParams;

   typedef struct
   {
      bool hold;
   } stopLoopAdsecParams;


}
}


#endif // WFSARB_INTERFACE_DEFINES_H
