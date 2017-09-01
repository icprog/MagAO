//@File: SlopeComp.cpp
//
// Implementation of the BCU class.
//
//@

#include <string>

#include <stdio.h>		// FILENAME_MAX 
#include <string.h>		// strcpy(), memcpy()
//#include <unistd.h>		// usleep()
#include <stdlib.h>		// free()
#include <math.h>		// sin()


extern "C" {
#include "base/thrdlib.h"
#include "iolib.h"
}

#include "BcuLib/BCUmsg.h"
#include "BCUfile.h"
#include "BCUlbt.h"
#include "SlopeComp.h"
#include "BcuLib/bcucommand.h"
#include "Logger.h"

#include "../Housekeeper/HouseKeeperRawStruct.h"

// Define this to use the command history from the slope computer
// (disabled in the SwitchBCU scheme)
//#define USE_COMMANDHISTORY

//@Function: SlopeComp
//
// Standard constructor: sets the IP address and sets pointers to NULL
//@

SlopeComp::SlopeComp( int number, char *clientname) : BCUlbt( number, clientname)
{
    gainoffset = BCUfile( sizeof(float32));

    n_subap = 0;
    paramblockselector = 0x00;
    reorder = NULL;
}

//@Function: ~SlopeComp
//
// Standard deconstructor: frees the allocated buffers
//@

SlopeComp::~SlopeComp()
{
   if (reorder)
      delete reorder;
}

//@Function: setNsubap
//
// Sets a new # of subapertures. Allocates or re-allocates
// arrays as needed.
//
// For a NxN sensor, the # of subapertures is defined as the
// total no. of subapertures, so it's close to NxN
//
// Usual values are:
//
// 800  (bin 1)
// 200  (bin 2)
//
//@

int SlopeComp::setNsubap( int n_subap)
{
    int stat;
    uint32 nslopes_bcu = n_subap/2;     // it's actually *2/4

    // Don't change our internal state if not needed

    // Change inconditionally
    //if ( this->n_subap == n_subap)
    //    return NO_ERROR;

    printf("Sending new # of slopes (%d) to BCU\n", nslopes_bcu);

    if (( stat = WriteBCU_DSPword( SLPCOMP__BSCUB_NUMSLOPES, nslopes_bcu)) != NO_ERROR)
        return stat;

    printf("Reallocating arrays\n");

    // Enlarged to maximum dimension
    int gainoffsetlen = n_subap*32/2;   // 32 items for each 8-pixel group

    //int gainoffsetlen = 800*32/2; // Always full length

    gainoffset.setItemLen( gainoffsetlen );
    gainoffset.clear();
    printf("Gainoffset is %d long\n", gainoffsetlen);

    this->n_subap = n_subap;

    return NO_ERROR;
}

//@Function: getNslopes
//
// Calculates the number of slopes as a function of the
// number of subapertures
//@

int SlopeComp::getNslopes( int n_subap)
{
    int n_slopes = n_subap*2;

    // Must be a multiple of 8.
    n_slopes &= 0xFFFFFFF0;

    return n_slopes;
}


//@Function: getNsubap
//
// Returns the # of subabertures
//@

int SlopeComp::getNsubap()
{
    return n_subap;
}

//@Function: setGainOffset
//
// Reads a GainOffset file and optionally sends it.
// File must be in hexadecimal format
//@

int SlopeComp::setGainOffset( std::string filename, int send)
{
    int stat;

    if ((stat = gainoffset.Load(filename)) != NO_ERROR)
        return stat;

    if (send)
        return sendGainOffset();

    return NO_ERROR;
}

//@Function: setSlopeOffsets
//
// Sets the slope offsets to a single value
// Optionally sends the entire GainOffset table
//@

int SlopeComp::setSlopeOffsets( float32 value, int send)
{
    int i, index;

    index=0;
    for (i=0; i< n_subap/2; i++)
        {
        gainoffset.asFloat()[index+20] = value;
        gainoffset.asFloat()[index+21] = value;
        gainoffset.asFloat()[index+22] = value;
        gainoffset.asFloat()[index+23] = value;
        index+= 32;
        }

    slopeoffsetfilename = "";
    if (send)
        return sendGainOffset();

    return NO_ERROR;
} 

//@Function: setTimeHistoryOffsets
//
// Sets the time history offsets to a single value
// Optionally sends the entire GainOffset table
//@

// MODIFIED TO MAKE A RAMP OF VALUES FROM 1 TO n !!!!!

int SlopeComp::setTimeHistoryOffsets( float32 value, int send)
{
    int i, index;

    index=0;
    for (i=0; i< n_subap/2; i++)
        {
        gainoffset.asFloat()[index+24] = value;
        gainoffset.asFloat()[index+25] = value+1;
        gainoffset.asFloat()[index+26] = value+2;
        gainoffset.asFloat()[index+27] = value+3;

	    value += 4;
        index+= 32;
        }

    timehistoryfilename = "";
    if (send)
        return sendGainOffset();

    return NO_ERROR;
}

int SlopeComp::setTimeHistoryOffsets( std::string filename, int send)
{
   unsigned char *buffer;
   int len, i;

   // If no filename given, use the last one
   if (filename == "")
       filename = timehistoryfilename;

   buffer = ReadBinaryFile( (char *)filename.c_str(), &len);

   if (buffer)
        {
        // Length must be a multiple of four data items
        int step = 4 * sizeof(float32);

        if (len % step) {
            len += step -(len % step);
            buffer = (unsigned char *)realloc( buffer, len);
        }

        int index_file=0;
        int index_bcu = 0;
        
        for (i=0; i< n_subap/2; i++)
            {
            gainoffset.asFloat()[index_bcu+24] = ((float32 *)buffer)[index_file+0];
            gainoffset.asFloat()[index_bcu+25] = ((float32 *)buffer)[index_file+1]; 
            gainoffset.asFloat()[index_bcu+26] = ((float32 *)buffer)[index_file+2];
            gainoffset.asFloat()[index_bcu+27] = ((float32 *)buffer)[index_file+3];
            index_bcu+= 32;
            index_file+=4;
            }
        timehistoryfilename = filename;
        }
    else
        throw AOException(filename, FILE_ERROR, __FILE__, __LINE__);

    if (send)
        return sendGainOffset();

    return NO_ERROR;
}


//@Function: setSlopeOffsets
//
// Reads the slope offsets from a file and sets them.
// File must have slopes in the same order as they are
// seen by the CCD, that is, they must have been saved
// from the same CCD configuration.
// Optionally send the whole GainOffset table up.
//@

int SlopeComp::setSlopeOffsets( std::string filename, int send)
{
   unsigned char *buffer;
   int len, i, index, counter, slopelen;


   // If no filename given, use the last one
   if (filename == "")
       filename = slopeoffsetfilename;

   buffer = ReadBinaryFile( (char *)filename.c_str(), &len);
   if (!buffer)
        throw AOException(filename, FILE_ERROR, __FILE__, __LINE__);

   slopelen = n_subap*2;

   // Length must be a multiple of four data items
   int step = 4;

   if (len % step)
       slopelen += step -(len % step);

   float32 *buffer2 = (float32 *)buffer;

    // we find it in ALMOST the right order from the BCU memory
    //
    // We only need to descramble them from:
    //
    // X1Y1X2Y2 X3Y3X4Y4
    //
    // to:
    //
    // X1X2Y1Y2 X3X4Y3Y4
    //
    // Also negate the sign.

    index=0;
    counter=0;
    for (i=0; i< n_subap/2; i++)
        {
        printf("i = %d, slope = %5.2f\n", i, buffer2[counter]);

        gainoffset.asFloat()[index+20] = -buffer2[counter+0];
        gainoffset.asFloat()[index+21] = -buffer2[counter+1];
        gainoffset.asFloat()[index+22] = -buffer2[counter+2];
        gainoffset.asFloat()[index+23] = -buffer2[counter+3];

        counter += 4;
        index += 32;
        }

    free(buffer);
//  slopeoffsetfilename = filename;

    if (send)
        return sendGainOffset();

    return NO_ERROR;
}

//@Function: getCurrentFrameNumber
//
// Asks the BCU for the current frame number and returns it
//@

int SlopeComp::getCurrentFrameNumber()
{
    uint32 addr = SLPCOMP__BSCUB_FRAMESCOUNTER;
    int opcode = MGP_OP_RDSEQ_DSP;
    uint32 value = 0;

    return GetBuffer( number, BCU_DSP, BCU_DSP, (unsigned char *)&value, sizeof(uint32), addr, opcode);
}

//@Function: readFLtimeouts
//
// Read the current FL timeouts counter
//@

int SlopeComp::readFLtimeouts( uint32 *value)
{
    uint32 addr = SLPCOMP__BSCUB_NUMFLTIMEOUT;
    int opcode = MGP_OP_RDSEQ_DSP;
    return GetBuffer( number, BCU_DSP, BCU_DSP, (unsigned char *)value, sizeof(uint32), addr, opcode);
}

//@Function: readCRCerrors
//
// Read the current CRC error counter
//@

int SlopeComp::readCRCerrors( uint32 *value)
{
    uint32 addr = SLPCOMP__BSCUB_NUMFLCRCERR;
    int opcode = MGP_OP_RDSEQ_DSP;
    return GetBuffer( number, BCU_DSP, BCU_DSP, (unsigned char *)value, sizeof(uint32), addr, opcode);
}


//@Function: setPixelGains
//
// Sets all the pixel gains to a fixed value
// and optionally sends the entire table
//@

int SlopeComp::setPixelGains( float32 value, int send)
{
    int i;
    int npixels = n_subap * 4;

    printf("npixels: %d\n", npixels);
    int index=0;
    for (i=0; i<npixels; i++)
        {
//      printf("index: %d\n", index);
        gainoffset.asFloat()[index] = value;
        index += 2;
        if ((i%8)==7)
            index += 16;
        }

    pixelgainfilename = "";

    if (send)
        return sendGainOffset();

    return NO_ERROR;
}

//@Function: setPixelOffsets
//
// Set the pixel offsets reading from a saved file in raster order.
// and optionally sends the entire table up.
//
// The file must have type int16./
//@

int SlopeComp::setPixelOffsets( std::string filename, int send)
{
    unsigned char *buffer;
    int i, index, len;
    int npixels;

    // If no filename give, re-use the last one
    if (filename == "")
        filename = pixeloffsetfilename;

    buffer = ReadBinaryFile( (char *)filename.c_str(), &len);

    if (!buffer)
        throw AOException(filename, FILE_ERROR, __FILE__, __LINE__);

    printf("Buffer len: %d\n",len);

    // Lenght must be a multiple of 8 data items
    int bpe = sizeof(int16);
    int step = 8 * bpe;
    if (len % step)
        {
        len += step - (len % step);
        printf("New buffer len: %d\n",len);
        buffer = (unsigned char *)realloc(buffer, len);
        }

    npixels = len / bpe;

     printf("Using total length of %d\n", npixels);


    uint16 *buffer_temp = new uint16[npixels];
    float32 *buffer_bcu = new float32[npixels];

    // Convert to BCU ordering
    if (!reorder)
       throw AOException("Pupil LUT not loaded", NOT_INIT_ERROR, __FILE__, __LINE__);

    reorder->BCURaster2Pixels( (uint16 *)buffer, buffer_temp);

    // Cut to the no. of subapertures available
    if (npixels > n_subap*4) {
      npixels = n_subap*4;
      printf("Length cut to %d (n_subap*4)\n", npixels);
      }


    // Now from BCU order we need just a little scramble because
    // we find it in ALMOST the right order from the BCU memory
    //
    // We only need to descramble them from:
    //
    // a1b1a2b2 c1d1c2d2
    //
    // to:
    //
    // a1a2b1b2 c1c2d1d2
    //
    // In addition, convert to float32 as the bcu requires and NEGATE the sign.

    Logger::get()->log( Logger::LOG_LEV_TRACE, "npixels=%d    _realNsubaps=%d", npixels, _realNsubaps);

    for (i=0; i< npixels; i+= 8)
        {
        // Set background to zero for all pixels outside the pupil.
        // This is *required* otherwise the slope normalization might be wrong.
        float k1 = (i   < _realNsubaps*4) ? 1 : 0;
        float k2 = (i+1 < _realNsubaps*4) ? 1 : 0;

        buffer_bcu[i +0] = -(float32)(buffer_temp[i +0]) *k1;
        buffer_bcu[i +1] = -(float32)(buffer_temp[i +2]) *k2;
        buffer_bcu[i +2] = -(float32)(buffer_temp[i +1]) *k1;
        buffer_bcu[i +3] = -(float32)(buffer_temp[i +3]) *k2;
        buffer_bcu[i +4] = -(float32)(buffer_temp[i +4]) *k1;
        buffer_bcu[i +5] = -(float32)(buffer_temp[i +6]) *k2;
        buffer_bcu[i +6] = -(float32)(buffer_temp[i +5]) *k1;
        buffer_bcu[i +7] = -(float32)(buffer_temp[i +7]) *k2;
        }

   // Simpatico ordine tabella gainoffset
    index=1;
    printf("n = %d\n", npixels);
    for (i=0; i< npixels; i++) {
        gainoffset.asFloat()[index] = buffer_bcu[i];

        Logger::get()->log( Logger::LOG_LEV_DEBUG, "i=%d, gainoffset[0x%04X] = %5.2f", i, index, gainoffset.asFloat()[index]);
        index +=2;
        if ((i%8) == 7)
            index += 16;
    }
    delete[] buffer_bcu;
    delete[] buffer_temp;
    free(buffer);
    pixeloffsetfilename = filename;

    if (send)
        return sendGainOffset();

    return NO_ERROR;
}

int SlopeComp::setPixelOffsets( float32 value, int send)
{
    int i, index;
    int npixels = n_subap * 4;

    index=1;
    for (i=0; i< npixels; i++)
        {
        gainoffset.asFloat()[index] = value;
        index +=2;
        if ((i%8)==7)
             index += 16;
        }

    pixeloffsetfilename = "";

    if (send)
        return sendGainOffset();

    return NO_ERROR;
}

//@Function: sendGainOffset
//
// Private function to send the gainoffset buffer to the BCU
//@

int SlopeComp::sendGainOffset()
{
	printf("Writing gainoffset table starting from 0x%08X, total 0x%08X words\n", DEFAULT_PARAMSLOPEPIXELAREAPTR, gainoffset.byteslen/4);

    return SendBuffer( number, BCU_DSP, BCU_DSP, gainoffset.getBuffer(), gainoffset.byteslen, DEFAULT_PARAMSLOPEPIXELAREAPTR, MGP_OP_WRSEQ_DSP);

}

//+Function: setPixelGains
//
// Set the pixel gains reading from an ASCII file
// and optionally sends the entire table up.
//-

int SlopeComp::setPixelGains( std::string filename, int send)
{
    unsigned char *buffer;
    int i, index, len;
    int npixels = n_subap * 4;

    if (filename != "")
        {
        buffer = ReadASCIIFile( (char *)filename.c_str(), &len);
        if (buffer)
            {
            // Fix npixels to the smallest available
              
            if (npixels != len/4)
                {
                printf("Npixels and len differ: %d and %d\n", npixels, len/4);
                if (npixels >len/4)
                       npixels = len/4;
                printf("Using total length of %d\n", npixels);
                }
             

            unsigned char *buffer2 = new unsigned char[len];
            memcpy(buffer2, buffer, sizeof(unsigned char)*len);
            index=0;

            // Convert from:
            // 
            // a1b1c1d1a2b2c2d2
            //
            // to:
            //
            // a1a2b1b2c1c2d1d2
            //
            // Values are written every each pixel (the other values are offsets)

            for (i=0; i< 3200/8; i++) {

               gainoffset.asFloat()[index+0] = ((float32 *)buffer2)[i*8+0];
               gainoffset.asFloat()[index+2] = ((float32 *)buffer2)[i*8+4];
               gainoffset.asFloat()[index+4] = ((float32 *)buffer2)[i*8+1];
               gainoffset.asFloat()[index+6] = ((float32 *)buffer2)[i*8+5];
               gainoffset.asFloat()[index+8] = ((float32 *)buffer2)[i*8+2];
               gainoffset.asFloat()[index+10] = ((float32 *)buffer2)[i*8+6];
               gainoffset.asFloat()[index+12] = ((float32 *)buffer2)[i*8+3];
               gainoffset.asFloat()[index+14] = ((float32 *)buffer2)[i*8+7];

               // Skip the 16 pixels just written, plus the 16 word slope-related block
               index +=32;
            }

            delete[] buffer2;

            free(buffer);
            pixelgainfilename = filename;
            }
        else
            throw AOException(filename, FILE_ERROR, __FILE__, __LINE__);
           }

    if (send)
        return sendGainOffset();

      return NO_ERROR;
    } 


//@Function: setTimeHistoryVector
//
// Sets the time history vector to a sinus wave with the specified
// amplitude, fitting the specified number of periods in the
// time history length.
//
// The first element is always set to zero to prevent errors when the
// time history feature is disabled.
//@

int SlopeComp::setTimeHistoryVector( double amp, double num_periods)
{
	int i, stat;
	int history_len = DEFAULT_TIMEHISTORYEND - DEFAULT_TIMEHISTORYPTR +1;
	printf("Time history length is %d\n", history_len);

    float32 *history_buffer = new float32[ history_len];

	float count=1;
	int diff=1;
	int dir=1;
        for (i=0; i<history_len; i++)
                {
		float32 value;

		// Sinus wave
		if (1)
			{
			if (i==0)
				value=0;
			else
       	       	value = amp * sin( num_periods * i * 2*3.1415 / history_len);
			}

		// Force to 0
		if (0)
			value = 0;

		// 1-5 ramp
		if (0)
			{
			value = count;
			count += dir;
			if ((count >4) || (count<1))
				dir = -dir;
			}
			
		// Always-increasing ste
		if (0)
			{
			value = count / 1000.0;
//			value = count;
			count = count +diff;
			diff++;
			}

        history_buffer[i] = value;
		//printf("Time history (pos %d): %5.2f\n", i, value);
        }

    stat = SendBuffer( number, BCU_DSP, BCU_DSP, (unsigned char *)history_buffer, sizeof(float32)*history_len, DEFAULT_TIMEHISTORYPTR, MGP_OP_WRSEQ_DSP);


    delete[] history_buffer;

    return stat;
}

// Sets the time history vector to a fixed value for all elements

int SlopeComp::setTimeHistoryVector( float32 value)
{
        int i, stat;
        int history_len = DEFAULT_TIMEHISTORYEND - DEFAULT_TIMEHISTORYPTR +1;
        printf("Time history length is %d\n", history_len);

    	float32 *history_buffer = new float32[ history_len];
        for (i=0; i<history_len; i++)
		history_buffer[i] = value;

	stat = SendBuffer( number, BCU_DSP, BCU_DSP, (unsigned char *)history_buffer, sizeof(float32)*history_len, DEFAULT_TIMEHISTORYPTR, MGP_OP_WRSEQ_DSP);

	delete[] history_buffer;

	return stat;
}

// Reads an ASCII file with the data for the fastlink
// and uploads it to the BCU

int SlopeComp::setFastLinkArea( std::string filename)
{
    int bufferlen, stat;
    unsigned char *buffer = ReadASCIIFile( (char *)filename.c_str(), &bufferlen);

    if (!buffer)
         return FILE_ERROR;

    Logger::get()->log( Logger::LOG_LEV_INFO, "Setting fastlink area: %s", filename.c_str());

    stat = SendBuffer( number, BCU_DSP, BCU_DSP, buffer, bufferlen, SLPCOMP__BSCUB_FASTLINKCMD, MGP_OP_WRSEQ_DSP);
    free(buffer);
    return stat;
}

// Reads an ASCII file with the data for the command history area
// and uploads it to the BCU
/*
int SlopeComp::setCommandHistoryArea( std::string filename)
{
    int bufferlen, stat;

#ifndef USE_COMMANDHISTORY
    return NO_ERROR;
#endif

    unsigned char *buffer = ReadASCIIFile( (char *)filename.c_str(), &bufferlen);

    if (!buffer)
         return FILE_ERROR;

    printf("File %s read, len = %d\n", filename.c_str(), bufferlen);

    unsigned int maxlen = DEFAULT_MODEHISTORYEND - DEFAULT_MODEHISTORYPTR +1;
    if (bufferlen / sizeof(int32) > maxlen)
	bufferlen = maxlen * sizeof(int32);

    stat = SendBuffer( number, BCU_DSP, BCU_DSP, buffer, bufferlen, DEFAULT_MODEHISTORYPTR, MGP_OP_WRSEQ_DSP);
    printf("Command history area OK\n");
    free(buffer);
    return stat;
}
*/

// Enable time history

int SlopeComp::TimeHistoryStart()
{
	int32 cmd = 0x02;		// Start command
	printf("Writing 0x%08X to 0x%08X\n", cmd, SLPCOMP__BSCUB_TIMEHISTORYACT);
	return SendBuffer( number, BCU_DSP, BCU_DSP, (unsigned char *)&cmd, sizeof(int32), SLPCOMP__BSCUB_TIMEHISTORYACT, MGP_OP_WRSEQ_DSP);
}

// Disable time history

int SlopeComp::TimeHistoryStop()
{
	int32 cmd = 0x01;		// Stop command
	printf("Writing 0x%08X to 0x%08X\n", cmd, SLPCOMP__BSCUB_TIMEHISTORYACT);
	return SendBuffer( number, BCU_DSP, BCU_DSP, (unsigned char *)&cmd, sizeof(int32), SLPCOMP__BSCUB_TIMEHISTORYACT, MGP_OP_WRSEQ_DSP);
}

// Enable command history
/*
int SlopeComp::CommandHistoryStart()
{
#ifndef USE_COMMANDHISTORY
    return NO_ERROR;
#endif

	int32 cmd = 0x02;		// Start command
	return SendBuffer( number, BCU_DSP, BCU_DSP, (unsigned char *)&cmd, sizeof(int32), SLPCOMP_MODEHISTORYACT, MGP_OP_WRSEQ_DSP);
}

// Disable command history

int SlopeComp::CommandHistoryStop()
{
#ifndef USE_COMMANDHISTORY
    return NO_ERROR;
#endif

	int32 cmd = 0x01;		// Stop command
	return SendBuffer( number, BCU_DSP, BCU_DSP, (unsigned char *)&cmd, sizeof(int32), SLPCOMP_MODEHISTORYACT, MGP_OP_WRSEQ_DSP);
}
*/
// Reads the time history vector from a file and uploads it to the BCU

int SlopeComp::setTimeHistoryVector( std::string filename)
{
    int bufferlen, stat;
    unsigned char *buffer = ReadASCIIFile( (char *)filename.c_str(), &bufferlen);

    if (!buffer)
	return FILE_ERROR;

    printf("File read, len = %d\n", bufferlen);

    unsigned int maxlen = DEFAULT_TIMEHISTORYEND - DEFAULT_TIMEHISTORYPTR +1;
    if (bufferlen / sizeof(int32) > maxlen)
	bufferlen = maxlen * sizeof(int32);

    stat = SendBuffer( number, BCU_DSP, BCU_DSP, buffer, bufferlen, DEFAULT_TIMEHISTORYPTR, MGP_OP_WRSEQ_DSP);
    delete[] buffer;

    return stat;
}

//@Function: ParamBlockSelector_enable
//
// Enables the specified bits of paramblockselector.
//
// Some bits are mutually exclusive, setting one of these bits
// will automatically disable the others
//@

int SlopeComp::ParamBlockSelector_enable( int bitmask)
{
	int stat;
	uint32 pbs = paramblockselector;

	if (bitmask & PBS_BLOCK)
		pbs |= PBS_BLOCK;
	if (bitmask & PBS_SH)
		{
		pbs |= PBS_SH;
		pbs &= ~PBS_TOMOGRAPHY;
		pbs &= ~PBS_CONSTANT;
		}
	if (bitmask & PBS_TOMOGRAPHY)
		{
		pbs &= ~PBS_SH;
		pbs |= PBS_TOMOGRAPHY;
		pbs &= ~PBS_CONSTANT;
		}
	if (bitmask & PBS_CONSTANT)
		{
		pbs &= ~PBS_SH;
		pbs &= ~PBS_TOMOGRAPHY;
		pbs |= PBS_CONSTANT;
		}
	if (bitmask & PBS_SLOPELUT)
		pbs |= PBS_SLOPELUT;
	if (bitmask & PBS_MANTEGAZZA_STYLE)
		pbs |= PBS_MANTEGAZZA_STYLE;
	if (bitmask & PBS_DIAGNOSTIC)
		pbs |= PBS_DIAGNOSTIC;
	if (bitmask & PBS_FASTLINK)
		pbs |= PBS_FASTLINK;
   if (bitmask & PBS_DISTURB_WFS)
      pbs |= PBS_DISTURB_WFS;
   if (bitmask & PBS_DISTURB_OVS)
      pbs |= PBS_DISTURB_OVS;

   printf("%d\n", pbs);
   fflush(stdout);

	stat = SendBuffer_verify( number, BCU_DSP, BCU_DSP, (unsigned char *)&pbs, sizeof(uint32), SLPCOMP__BSCUB_PARAMSELECTOR, MGP_OP_WRSEQ_DSP, MGP_OP_RDSEQ_DSP);

	// Update local copy only if the BCU was update successfully
	if (stat == NO_ERROR)
		paramblockselector = pbs;

	return stat;
}

int SlopeComp::setRealNsubaps( float32 value)
{
   _realNsubaps = (int)value;

   // Write the average flux gain area
   return SendBuffer( number, BCU_DSP, BCU_DSP, (unsigned char *)&value, sizeof(float32), DEFAULT_PARAMAVERAGEFLUXAREAPTR+2, MGP_OP_WRSEQ_DSP);
}

int SlopeComp::setSlopeClip( float32 value)
{
   return SendBuffer( number, BCU_DSP, BCU_DSP, (unsigned char *)&value, sizeof(float32), SLPCOMP__BSCFB_SLOPECLIPTHRESHOLD, MGP_OP_WRSEQ_DSP);
}


//@Function: ParamBlockSelector_disable
//
// Disables the specified bits of paramblockselector.
//@

int SlopeComp::ParamBlockSelector_disable( int bitmask)
{
	int stat;
        uint32 pbs = paramblockselector;

	// Since there are no dependencies between bits we can
	// do this the easy way.
	pbs &= ~bitmask;

	stat = SendBuffer_verify( number, BCU_DSP, BCU_DSP, (unsigned char *)&pbs, sizeof(uint32), SLPCOMP__BSCUB_PARAMSELECTOR, MGP_OP_WRSEQ_DSP, MGP_OP_RDSEQ_DSP);

	// Update local copy only if the BCU was update successfully
	if (stat == NO_ERROR)
		paramblockselector = pbs;

	return stat;
}



int SlopeComp::sendPixelLUT( std::string filename)
{
   // Call base class function
   int stat = BCUlbt::sendPixelLUT(filename);

   if (reorder)
      delete reorder;

   reorder = new Reorder();
   reorder->Init( "ccd39");
   return stat;
}
