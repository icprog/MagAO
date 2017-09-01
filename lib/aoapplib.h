//@File: aoapplib.h
//
//@

#ifndef AOAPPLIB_H_INCLUDED
#define AOAPPLIB_H_INCLUDED

#include "AOExcept.h"
#include <boost/archive/binary_oarchive.hpp> //serialize
#include <boost/archive/binary_iarchive.hpp> //serialize

using namespace std;

namespace Arcetri {

class LogLevelModifier {
    public:
        LogLevelModifier(string logger="", int level=0) : 
            _logger(logger),
            _level(level) {};
        string       Logger(){return _logger;}
        int          Level() {return _level;}

    private:
        friend class boost::serialization::access;
        template<class Archive>
        void  serialize(Archive & ar, const unsigned int /* file_version */) {
            ar & _logger & _level;
        }

    private:
        string  _logger;
        int     _level;
};
}
#include <boost/serialization/tracking.hpp>
BOOST_CLASS_TRACKING(Arcetri::LogLevelModifier, boost::serialization::track_never);

#include <boost/serialization/map.hpp>
typedef map<string,int> NameLevelDict;
BOOST_CLASS_TRACKING(NameLevelDict, boost::serialization::track_never);

namespace Arcetri {
class AOAppLib {

    public:
  
        /*
         * Destroy the interface.
         */
        ~AOAppLib();


        /*
         * Change the level of a logger of an aoapp
         */
        static void setLoggerLevel(string aoapp, string logger, int level) throw (AOException);


        /*
         * Get the dictionary of loggers' names and levels from an aoapp
         * timeout in ms to wait for the answer
         */
        static NameLevelDict getLoggerNamesAndLevels(string aoapp, int timeout=1000) throw (AOException);

};


}

#endif



