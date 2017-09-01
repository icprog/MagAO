#ifndef ADSECPTYPES_H_INCLUDE
#define ADSECPTYPES_H_INCLUDE

#include <string>

#include <boost/archive/binary_iarchive.hpp>
#include <boost/archive/binary_oarchive.hpp>

#include "arblib/aoArb/AOArbConst.h"

using namespace std; 

// Types for all command's parameters

namespace Arcetri {
namespace AdSec_Arbitrator {


/*
 * A serializable structure containing all the parameters 
 * for the 'set_rec_mat' and derived commands
 */
struct recMatParams {

   string _m2cFile;
   string _recMatFile;
   string _aDelayFile;
   string _bDelayFile;
    
    private:
    
	    friend class boost::serialization::access;
		template<class Archive>
		void serialize(Archive& ar, const unsigned int /* version */) {
			ar & _m2cFile;
			ar & _recMatFile;
			ar & _aDelayFile;
			ar & _bDelayFile;
		}
};

/*
 * A serializable structure containing all the parameters 
 * for the 'set_disturb' and derived commands
 */
struct disturbParams {

   bool _on;
   string _disturbFile;
    
    private:
    
	    friend class boost::serialization::access;
		template<class Archive>
		void serialize(Archive& ar, const unsigned int /* version */) {
			ar & _on;
			ar & _disturbFile;
		}
};

struct applyCommandParams {

   string commandsFile;

    private:
    
	    friend class boost::serialization::access;
		template<class Archive>
		void serialize(Archive& ar, const unsigned int /* version */) {
			ar & commandsFile;
		}
};


/*
 * A serializable structure containing all the parameters 
 * for the flat commands
 */
struct flatParams {

   string _dummy;
    
    private:
    
	    friend class boost::serialization::access;
		template<class Archive>
		void serialize(Archive& ar, const unsigned int /* version */) {
			ar & _dummy;
		}
};

/*
 * A serializable structure containing all the parameters 
 * for the 'loadShape' and derived commands
 */
struct shapeParams {

   string _shapeFile;
    
    private:
    
	    friend class boost::serialization::access;
		template<class Archive>
		void serialize(Archive& ar, const unsigned int /* version */) {
			ar & _shapeFile;
		}
};

/*
 * A serializable structure containing all the parameters 
 * for the 'setGain' and derived commands
 */
struct gainParams {

   string _gainFile;
    
    private:
    
	    friend class boost::serialization::access;
		template<class Archive>
		void serialize(Archive& ar, const unsigned int /* version */) {
			ar & _gainFile;
		}
};

struct saveSlopesParams {

   string _filename;
   int    _nFrames;
    
    private:
    
	    friend class boost::serialization::access;
		template<class Archive>
		void serialize(Archive& ar, const unsigned int /* version */) {
			ar & _filename;
			ar & _nFrames;
		}
};

struct saveStatusParams {

   string _filename;
    
    private:
    
	    friend class boost::serialization::access;
		template<class Archive>
		void serialize(Archive& ar, const unsigned int /* version */) {
			ar & _filename;
		}
};

struct dataDecimationParams {

   int _decimation;

    private:
    
	    friend class boost::serialization::access;
		template<class Archive>
		void serialize(Archive& ar, const unsigned int /* version */) {
			ar & _decimation;
		}
};

struct selectFocalStationParams {

   string _focalStation;

    private:
    
	    friend class boost::serialization::access;
		template<class Archive>
		void serialize(Archive& ar, const unsigned int /* version */) {
			ar & _focalStation;
		}
};

struct ttOffloadParams {

   bool enable;

    private:
    
	    friend class boost::serialization::access;
		template<class Archive>
		void serialize(Archive& ar, const unsigned int /* version */) {
			ar & enable;
		}
};

struct runAoParams {

   int _decimation;
   float _loopFreq;
   float _ovsFreq;

    private:
    
	    friend class boost::serialization::access;
		template<class Archive>
		void serialize(Archive& ar, const unsigned int /* version */) {
			ar & _decimation;
         ar & _loopFreq;
         ar & _ovsFreq;
		}
};

struct setZernikesParams {

    double deltaM[N_ZERN];

    private:
    
	    friend class boost::serialization::access;
		template<class Archive>
		void serialize(Archive& ar, const unsigned int /* version */) {
			ar & deltaM;
		}
};

struct stopAoParams {

   bool _restoreShape;

    private:
    
	    friend class boost::serialization::access;
		template<class Archive>
		void serialize(Archive& ar, const unsigned int /* version */) {
			ar & _restoreShape;
		}
};

struct calibrateParams {
    string _procname;
    int    _param;
    string _result;
    
  private:
    friend class boost::serialization::access;
    template<class Archive>
    void serialize(Archive& ar, const unsigned int /* version */) {
        ar & _procname;
        ar & _param;
        ar & _result;
    }
};


}
}

#endif /*ADSECPTYPES_H_INCLUDE*/
