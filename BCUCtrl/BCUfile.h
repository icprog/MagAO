//+File: BCUfile.cpp
//
// Class definition for a BCU configuration filoe
//-

#ifndef BCUFILE_H_INCLUDED
#define BCUFILE_H_INCLUDED

#include "aotypes.h"
#include "commlib.h"

class BCUfile
{
public:
	BCUfile( unsigned int itemsize=0);			// Standard constructor
	BCUfile( std::string filename, unsigned int itemsize);	// constructor with file loading

	~BCUfile();

	int Load( std::string filename);

	unsigned char *getBuffer();	// Direct buffer access
	float32 *asFloat();
	uint32 *asInt();
	uint16 *asShortInt();

	int setFilename( std::string filename);
   std::string getFilename();
	int setItemLen( int len);

   int clear(void);

public:
	int itemsize;			// Length of a single item in bytes
	int itemslen;			// Total length in items
	int byteslen;			// Total length in bytes

protected:
   std::string filename;
	unsigned char *buffer;
};

#endif // BCUFILE_H_INCLUDED


