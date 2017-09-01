#ifndef AO_SERVICE_STATES_INCLUDE
#define AO_SERVICE_STATES_INCLUDE


#include <string>
using namespace std;

extern "C" {
#include "base/globals.h"
}

namespace Arcetri {

// Type for AO mode
typedef int AOModeType;


/*
 * @Class: AOMode
 * Defines modes for the AO system
 * @
 */
class AOMode {

   public:

      static const AOModeType INVALID        = -1; // Invalid value
      static const AOModeType UNDEFINED      = 0;  // Undefined (system is off)
      static const AOModeType FIX_AO          = 1;  // AdSec only
      static const AOModeType TTM_AO         = 2;  // Wfs and AdSec
      static const AOModeType ICE_AO         = 3;  // Wfs and AdSec Interactive
      static const AOModeType ACE_AO         = 4;  // Wfs and AdSec Automatic
      static const AOModeType LGS_AO         = 5;  // Laser guide star mode

   static string toString(AOModeType aoMode) {
      switch(aoMode) {
         case TTM_AO:
            return AO_MODE_TTM;
         case ACE_AO:
            return AO_MODE_ACE;
         case ICE_AO:
            return AO_MODE_ICE;
         case LGS_AO:
            return AO_MODE_LGS;
         case FIX_AO:
            return AO_MODE_FIX;
         case UNDEFINED:
            return AO_MODE_UNDEFINED;
      }
      return "";
   }

   static AOModeType fromString( const string &mode) {
      if (mode == "Undefined")
         return UNDEFINED;
      else if (mode == AO_MODE_TTM)
         return TTM_AO;
      else if (mode == AO_MODE_ACE)
         return ACE_AO;
      else if (mode == AO_MODE_ICE)
         return ICE_AO;
      else if (mode == AO_MODE_LGS)
         return LGS_AO;
      else if (mode == AO_MODE_FIX)
         return FIX_AO;
      else
         return INVALID;
   }

};

/*
   @Class: AOWfsSpec
  
   Defines Specifications of supported Wavefront Sensors
   @
 */

typedef int WfsSpecType;

class AOWfsSpec {

   public:

      static const WfsSpecType INVALID       = -1;  // Invalid
      static const WfsSpecType WFS_FLAO      =  0;  // FLAO WFS
      static const WfsSpecType WFS_LBTI      =  1;  // LBTI_WFS
      static const WfsSpecType WFS_MAG       =  2;  // LBTI_WFS

   static string toString(WfsSpecType wfsSpec) {
      switch(wfsSpec) {
         case WFS_FLAO:
            return AO_WFS_FLAO;
         case WFS_LBTI:
            return AO_WFS_LBTI;
         case WFS_MAG:
            return AO_WFS_MAG;
      }
      return "";
   }

   static WfsSpecType fromString( const string &wfsSpec) {
      if (wfsSpec == AO_WFS_FLAO)
         return WFS_FLAO;
      else if (wfsSpec == AO_WFS_LBTI)
         return WFS_LBTI;
      else if (wfsSpec == AO_WFS_MAG)
         return WFS_MAG;
      else
         return INVALID;
   }

};




}


#endif // AO_SERVICE_STATES
