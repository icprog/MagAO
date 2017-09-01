//+File: okos.cpp
//
// Array definition for oko configuration
//-

#include "okos.h"

// Array to hold information about OKO mirrors

struct oko_config oko[] =
{
        // oko #0: old oko

        { 100,                  // null
          99,                   // saturation
          200,                  // bias
          {91,86,98,104,84,101,94,121,102,104,113,89,105,123,96,73,76,88,99,116,115,102,70,63,54,51,80,118,112,111,89,85,61,70,80,101,118}
        },

        // oko #1: new oko

        { 128,                  // null
          127,                  // saturation
          255,                  // bias
          {27, 151, 129, 126, 120, 130, 133, 109, 103, 56, 68, 26, 77, 72, 110, 97, 140, 145, 144, 52, 83, 87, 136, 105, 120, 132, 77, 36, 18, 23, 50, 61,
 64, 42, 15, 47, 40}
        }
};


