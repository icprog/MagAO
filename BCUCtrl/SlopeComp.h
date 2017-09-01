//@File: SlopeComp.h
//
// Class definition for the SlopeComp object
//

//@

#ifndef SLOPECOMP_H_INCLUDED
#define SLOPECOMP_H_INCLUDED


#include "SlopeComp_memorymap.h"
#include "SlopeComp_defaults.h"
#include "ParamBlockSelector.h"
#include "Reorder.h"

extern "C" {
#include "base/common.h"
}

class SlopeComp : public BCUlbt
{
public:
	SlopeComp( int number, char *clientname);	// Standard constructor
	~SlopeComp();	 				// Standard deconstructor

public:
    int setNsubap( int n_subap);        // Sets the # of subapertures
    int getNsubap();            // Gets the # of subapertures
    int getNslopes( int n_subap);       // Gets the # of slopes to be calculated as a function of subapertures

    int getCurrentFrameNumber();        // Returns the current frame # the BCU is processing

    // Virtual - Overridden to re-init the reorder tables
    int sendPixelLUT( std::string filename);

   int setPixelGains( std::string filename, int send=1);    // Reads a pixel gains file and optionally sends
                                                       // the whole GainOffset table to the BCU

   int setPixelGains( float32 value, int send=1);  // Sets all the pixel gains to a fixed value

   int setPixelOffsets( std::string filename, int send=1);
   int setPixelOffsets( float32 value, int send=1);

   int setSlopeOffsets( std::string filename, int send=1);
   int setSlopeOffsets( float32 value, int send=1);

   int setTimeHistoryOffsets( std::string filename, int send=1);
   int setTimeHistoryOffsets( float32 value, int send=1);

   int setGainOffset( std::string filename, int send=1);

   int setTimeHistoryVector( double amp, double num_periods);
   int setTimeHistoryVector( float32 value);
   int setTimeHistoryVector( std::string filename);

   int setRealNsubaps( float32 value);

   int setFastLinkArea( std::string filename);
//   int setCommandHistoryArea( std::string filename);

   int TimeHistoryStart();
   int TimeHistoryStop();
   /*
   int CommandHistoryStart();
   int CommandHistoryStop();
   */

   int ParamBlockSelector_enable( int bitmask);		// Enables the specified bits of ParamBlockSelector
   int ParamBlockSelector_disable( int bitmask);		// Disables the specified bits of ParamBlockSelector

   int readCRCerrors( uint32 *value);          // Return the current CRC error counter
   int readFLtimeouts( uint32 *value);         // Return the current FL timeout counter

   int setSlopeClip( float32 value);

protected:
    int sendGainOffset();

    int n_subap;
    int _realNsubaps;

    BCUfile gainoffset;

    std::string pixelgainfilename;       // Last filename used for pixel gains
    std::string pixeloffsetfilename;     // Last filename used for pixel offets
    std::string slopeoffsetfilename;     // Last filename used for slope offets
    std::string timehistoryfilename;     // Last filename used for time history

    uint32 paramblockselector;		// Current ParamBlockSelector value

    Reorder *reorder;                    // Reorder table access

};

#endif // SLOPECOMP_H_INCLUDED
