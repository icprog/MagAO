//@File: aoslib.h
//
// Definitions for the AOS related functions
// 
// This file contains definition neede by functions to be used by
// an application in order to communicate with the AOS
//@

#ifndef AOSLIB_H_INCLUDED
#define AOSLIB_H_INCLUDED

#define AOSLIB_VMAJOR    2
#define AOSLIB_VMINOR    2

#define AOS_REPLY_TIMEOUT  500  // Defualt reply timeout for AOS
#define OFL_ZERN_NUMB      22   // Number of values in offload modes array

#include "aoslib/aoscodes.h"

typedef struct {
    int rows;       // Number of rows
    int cols;       // Number of cols
    unsigned char pix[];
}                        TVImage;

// Data structures for telemetry record of type AOS_TEL_ADSEC1
// Note: this is an example. Details yet TBD

// Generic structure to get rtype
typedef struct {
    int rtype;
} gen_tel_struct;

#define AOS_TEL_ADSEC1      301
typedef struct {
    int rtype;
    double data1;
    double data2;
} aos_tel_adsec1;

// Data structure for telemetry record of type AOS_TEL_ADSEC2
// Note: this is an example. Details yet TBD

#define AOS_TEL_ADSEC2      302
typedef struct {
    int rtype;
    double data1;
    double data2;
    double data3;
} aos_tel_adsec2;

// Data structure for telemetry record of type AOS_TEL_WFS1
// Note: this is an example. Details yet TBD

#define AOS_TEL_WFS1       303
typedef struct {
    int rtype;
    double data1;
    double data2;
    double data3;
} aos_tel_wfs1;

#define AOS_TEL_WFS2       304
typedef struct {
    int rtype;
    double data1;
    double data2;
    double data3;
} aos_tel_wfs2;


char *aos_vers(void);

int aos_init(char *side);     // Initialization 

int aos_enable(int enable);    // Change AOS status

int aos_clean(void);    // Cleanup

int aos_debug(int);     // Set debug level

int aos_isup(int timeout);      // check if AOS is up and running

int aos_log(int);               // Activate/deactivate log

int aos_offload(double zernikes[OFL_ZERN_NUMB]);

int aos_flatlist(char *list);   // Update flat list

int aos_telemetry(void *record); // Send record to TCS telemetry system

int aos_hxpinit(double eX,      // Init hexapod
                double eY,
                double eZ,
                double eA,
                double eB,
                double eG,
                int timeout);

int aos_hxpmoveto(double eX,     // Move hexapod in the current
                  double eY,     // stored reference system
                  double eZ,
                  double eA,
                  double eB,
                  double eG,
                  int timeout);

int aos_hxpmoveby(double dX,    // Move hexapod relative to previous
                  double dY,    // position in the current stored 
                  double dZ,    // reference system
                  double dA,
                  double dB,
                  double dG,
                  int timeout);

int aos_hxpmovsph(double R,    // Move hexapod on a spherical surface
                  double eA,
                  double eB,
                  int timeout);

int aos_hxpnewref(int timeout);        // Store current position as new reference system

double *aos_hxpgetpos(int timeout);    // Return pointer to array containing position
                                        // in the current reference system

double *aos_hxpgetabs(int timeout);    // Return pointer to array containing position
                                        // in the absolute reference system

int aos_hxpbrake(int onoff,             // Activate/deactivate brake
                 int timeout);

int aos_hxpisinitialized(int timeout);  // Check hexapod initialization

int aos_hxpismoving(int timeout);       // Check if hexapod is moving

DbTable* aostxt_c(int code);    // Access to command table
DbTable* aostxt_i(int ix);      // Access to command table
#endif // AOSLIB_H_INCLUDED
