//@File: shmlib.h
//
// Header file for the old shared memory library
// Temporary file: the new shared memory library will render this obsolete
//@

#ifndef SHMLIB_H
#define SHMLIB_H

// List of defined key values for shared memory buffers
// To be deleted after the new library is in place

#define PIXELVIEW_SHARED   (0x3502)
#define PIXELGET_SHARED    (0x3003)

#define KICKER_SHARED	   (0x4002)

#define BCUREAD_SHARED	   (0x460a)

#define PIXELS_SHARED	   (0x4800)

#define SHMKEY_BASE        (0x5000)

#define GETPIXELS_SHARED   (0x6000)
#define GETSLOPES_SHARED   (0x6001)
#define GETCOMMANDS_SHARED (0x6002)

#endif
