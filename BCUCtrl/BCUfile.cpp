//+File: BCUfile.cpp
//
// Class implementation for a BCU configuration filoe
//-

#include <string>
#include <cstring>

extern "C" {
#include "base/errlib.h"
#include "commlib.h"
#include "iolib.h"
}


#include "BCUfile.h"


//+Function: BCUfile
//
// Standard constructor
// itemsize is the size of a single item in the configuration in bytes.
// For example, set to 4 if the config. file will use single-precision floats
//-

BCUfile::BCUfile( unsigned int itemsize)
{
	filename="";
	buffer = NULL;
	itemslen=0;
	byteslen=0;
	this->itemsize = itemsize;
}

//+Function: 
//
//

BCUfile::BCUfile( std::string filename, unsigned int itemsize)
{
	filename="";
	buffer = NULL;
	itemslen=0;
	byteslen=0;
	this->itemsize = itemsize;

	Load(filename);
}

BCUfile::~BCUfile()
{
	if (buffer)
		free(buffer);
}

int BCUfile::Load( std::string filename)
{
	int len;

	// If no filename give, reuse the last one
	if (filename == "")
		filename = this->filename;

//    Disable this check - we should compare contents instead

//	if (filename == this->filename)
//		return NO_ERROR;

	if (buffer)
		free(buffer);

	buffer = ReadASCIIFile( (char *)filename.c_str(), &len);
	if (!buffer)
		return FILE_ERROR;

	printf("File %s length: %d\n", filename.c_str(), len);
	byteslen = len;
	itemslen = len/itemsize;
	this->filename = filename;
	return NO_ERROR;
}

unsigned char *BCUfile::getBuffer()
{
	return buffer;
}

float32 *BCUfile::asFloat()
{
	return (float32 *)buffer;
}

uint32 *BCUfile::asInt()
{
	return (uint32 *)buffer;
}

int BCUfile::setFilename( std::string filename)
{
	this->filename = filename;
	return 0;
}

int BCUfile::setItemLen( int len)
{
	if (!buffer)
		buffer = (unsigned char *)calloc( len, itemsize);
	else
		buffer = (unsigned char *)realloc( buffer, len*itemsize);

	if (!buffer)
		return MEMORY_ALLOC_ERROR;

	itemslen = len;
	byteslen = len * itemsize;
	return NO_ERROR;
}

std::string BCUfile::getFilename()
{
	return filename;
}

int BCUfile::clear()
{

   memset(buffer, 0, byteslen);
   return NO_ERROR;
}
