#ifndef AOPTYPES_H_INCLUDE
#define AOPTYPES_H_INCLUDE

#include <string>

#include <boost/archive/binary_iarchive.hpp>
#include <boost/archive/binary_oarchive.hpp>

#include "AOGlobals.h"
#include "AOArbConst.h"

using namespace std; 

// Types for all command's parameters

namespace Arcetri {
namespace Arbitrator {


/*
 * Structure for AdjustGain parameters
 */
typedef struct {

   double gain;            // [TODO] what are the parameters??
    
    private:
    
	    friend class boost::serialization::access;
		template<class Archive>
		void serialize(Archive& ar, const unsigned int /* version */) {
         ar & gain;
		}
} adjustGainParams;

/*
 * Structure for AdjustIntTime parameters
 */
typedef struct {

   double intTime;            // [TODO] what are the parameters??
    
    private:
    
	    friend class boost::serialization::access;
		template<class Archive>
		void serialize(Archive& ar, const unsigned int /* version */) {
         ar & intTime;
		}
} adjustIntTimeParams;


/*
 * Structure for setAOMode parameters
 */
typedef struct {

   AOModeType mode;
    
    private:
    
	    friend class boost::serialization::access;
		template<class Archive>
		void serialize(Archive& ar, const unsigned int /* version */) {
         ar & mode;
		}
} setAOModeParams;



/*
 * Structure for PresetFlat parameters
 */
typedef struct {

   string flatSpec;
    
    private:
    
	    friend class boost::serialization::access;
		template<class Archive>
		void serialize(Archive& ar, const unsigned int /* version */) {
         ar & flatSpec;
		}
} presetFlatParams;



/*
 * Structure for PresetAO
 */
typedef struct {

    string aoMode;
    string focStation;
    string instr;
    string wfsSpec;
    double soCoord[2];
    double roCoord[2];
    double elevation;
    double rotAngle;
    double gravAngle;
    double mag;
    double color;
    double r0;
    double skyBrgt;
    double windSpeed;
    double windDir;
    bool   adcTracking;
    
    private:
    
	    friend class boost::serialization::access;
		template<class Archive>
		void serialize(Archive& ar, const unsigned int /* version */) {
         ar & aoMode;
         ar & focStation;
         ar & instr;
         ar & wfsSpec;
         ar & soCoord;
         ar & roCoord;
         ar & elevation;
         ar & rotAngle;
         ar & gravAngle;
         ar & mag;
         ar & color;
         ar & r0;
         ar & skyBrgt;
         ar & windSpeed;
         ar & windDir;
         ar & adcTracking;
		}
} presetAOParams;


/*
 * Structure for AcquireRefAO
 */
typedef struct {

   double  deltaXY[2];
   string slNull;
   int    nModes;
   double  freq;
   int    nBins;
   double  ttMod;
   string f1spec;
   string f2spec;
   double gain;
   string pupils;
   string base;
   string rec;
   string filtering;
   int decimation;
   double  strehl;
   double  r0;
   double  mSNratio[672];
   double  starMag;
   string CLbase;
   string CLrec;
   double CLfreq;
   char   TVframe[256*256+2*sizeof(int)];

    private:
    
	    friend class boost::serialization::access;
		template<class Archive>
		void serialize(Archive& ar, const unsigned int /* version */) {
         ar & deltaXY;
         ar & slNull;
         ar & nModes;
         ar & freq;
         ar & nBins;
         ar & ttMod;
         ar & f1spec;
         ar & f2spec;
         ar & gain;
         ar & pupils;
         ar & base;
         ar & rec;
         ar & filtering;
         ar & decimation;
         ar & strehl;
         ar & r0;
         ar & mSNratio;
         ar & starMag;
         ar & CLbase;
         ar & CLrec;
         ar & CLfreq;
         ar & TVframe;
		}

} acquireRefAOResult;

/*
 * Structure for CheckRefAO
 */

typedef struct {

   double deltaXY[2];
   double starMag;

    private:

     friend class boost::serialization::access;

     template<class Archive>
     void serialize(Archive& ar, const unsigned int /* version */) {
         ar & deltaXY;
         ar & starMag;
     }

} checkRefAOResult;


/*
 * Structure for OffsetXY
 */

typedef struct {

   double deltaXY[2];

    private:

     friend class boost::serialization::access;

     template<class Archive>
     void serialize(Archive& ar, const unsigned int /* version */) {
         ar & deltaXY;
     }

} offsetXYParams;


/*
 * Structure for OffsetZ
 */

typedef struct {

   double deltaZ;

    private:

     friend class boost::serialization::access;

     template<class Archive>
     void serialize(Archive& ar, const unsigned int /* version */) {
         ar & deltaZ;
     }

} offsetZParams;


/*
 * Structure for ModifyAO
 */

typedef struct {

   int nModes;
   double freq;
   int   binning;
   double TTmod;
   string f1spec;
   string f2spec;

    private:

     friend class boost::serialization::access;

     template<class Archive>
     void serialize(Archive& ar, const unsigned int /* version */) {
         ar & nModes;
         ar & freq;
         ar & binning;
         ar & TTmod;
         ar & f1spec;
         ar & f2spec;
     }

} modifyAOParams;

/*
 * Structure for CorrectModes
 */

typedef struct {

   double deltaM[N_CORRECTMODES];

    private:

     friend class boost::serialization::access;

     template<class Archive>
     void serialize(Archive& ar, const unsigned int /* version */) {
         ar & deltaM;
     }

} correctModesParams;

/*
 * Structure for setZernikes
 */

typedef struct {

   double deltaM[N_ZERN];

    private:

     friend class boost::serialization::access;

     template<class Archive>
     void serialize(Archive& ar, const unsigned int /* version */) {
         ar & deltaM;
     }

} setZernikesParams;

/*
 * Structure for Stop
 */

typedef struct {

   string msg;

    private:

     friend class boost::serialization::access;

     template<class Archive>
     void serialize(Archive& ar, const unsigned int /* version */) {
         ar & msg;
     }

} stopParams;


/*
 * Structure for RefineAO
 */

typedef struct {

   string method;

    private:

     friend class boost::serialization::access;

     template<class Archive>
     void serialize(Archive& ar, const unsigned int /* version */) {
         ar & method;
     }

} refineAOParams;


/*
 * Structure for UserPanic
 */

typedef struct {

   string msg;

   private:

      friend class boost::serialization::access;

      template<class Archive>
      void serialize(Archive& ar, const unsigned int /* version */) {
         ar & msg;
      }
} userPanicParams;

/*
 * Structure for PowerOnWfs/PwerOffWfs
 */

typedef struct {

   string wfsid;

   private:

      friend class boost::serialization::access;

      template<class Archive>
      void serialize(Archive& ar, const unsigned int /* version */) {
         ar & wfsid;
      }
} wfsOnOffParams;


}
}

#endif /*AOPTYPES_H_INCLUDE*/
