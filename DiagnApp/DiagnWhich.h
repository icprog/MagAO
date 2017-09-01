//@File: DiagnWhich.h
//
//Definition of the class DiagnWhich.
//
//author: Lorenzo Busoni.
//
//version 1.0
//
//Created on: 1 Sep 2006
//
//Modification history:
//@ 

#ifndef DIAGNWHICH_H_INCLUDED
#define DIAGNWHICH_H_INCLUDED


#include <string>
#include <sstream>
#include <iostream>
#include <stdlib.h> 
#include <fnmatch.h>
#include <algorithm>
#include <cctype> // std::toupper

#include <boost/archive/binary_oarchive.hpp> //serialize

#include "DiagnVar.h"


//@Class: DiagnWhich
//
// Identifies a DiagnVar or a set of DiagnVars.
//
// This class is used in two way.
//
// 1) To retrieve or set properties for a set of DiagnVars. In this case, a DiagnWhich can match more than one DiagnVar.
// For instance, *[0/5] matches all the DiagnVars with index between 0 and 5 included. 
// 
// 2) As key to build dictionary of DiagnVar's properties. In this case, a DiagnWhich identifies only one DiagnVar.
// The matched DiagnVar's name and index can  be retrieved by the Family() and From() method respectively.
// The To() and From() methods should return the same value. So "Distance[4/4]" univocally identifies the 
// DiagnVar where family="Distance" and index=4.  
//@

class DiagnWhich {
    public:
        //DiagnWhich() {};
        //DiagnWhich(const std::string & family, int index) : _family(family), _from(index), _to(index) {} 
        //DiagnWhich(const std::string & family, int from, int to) : _family(family), _from(from), _to(to) {} 
        DiagnWhich(
                const std::string &family="*", 
                int from    = All, 
                int to      = All, 
                int slow    = All, 
                int enabled = All
                ) :
            _family(family),
            _from(from),
            _to(to),
            _slow(slow),
            _ena(enabled) {
                // explicit cast needed to resolve ambiguity of toupper function
                std::transform(_family.begin(), _family.end(), _family.begin(), (int(*)(int)) std::toupper);
            };

        const std::string &             Family() {return _family;}
        int                             From() {return _from;}
        int                             To() {return _to;}
        int                             isSlow() {return _slow;}
        int                             isEnabled() {return _ena;}
         
        void                            SetFamily(const std::string & family) {_family = family;}
        void                            SetFrom(int from) {_from = from;}
        void                            SetTo(int to) {_to = to;}
        void                            SetSlow(int slow) {_slow = slow;}
        void                            SetEnabled(int ena) {_ena = ena; }
        std::string                     str() {std::ostringstream oss; oss << *this; return oss.str();}
        
        friend int operator<(const DiagnWhich& l, const DiagnWhich& r) {
            if ( l._family <  r._family ) return true;
            if ( l._family == r._family && l._from <  r._from ) return true;
            if ( l._family == r._family && l._from == r._from && l._to < r._to ) return true;
            return false;
        }

        //@Method: operator()
        //Compare a DiagnWhich filter with a DiagnVar
        //
        //Return true if the DiagnVar matches the DiagnWhich object,
        //false otherwise.
        //@
        bool operator()(const DiagnVarPtr &obj)
        {
            if (_from        != All  && _from        > (int)obj->Index())     return false;
            if (_to          != All  && _to          < (int)obj->Index())     return false;
            if (_slow        != All  && _slow        != obj->isSlow())        return false;
            if (_ena         != All  && _ena         != obj->isEnabled())     return false;
			
            if (_family == "*") return true;

            if (_family.find_first_of("* ") == string::npos) return _family == obj->Family();  

            // if we are here either there is a white-space in the filter and maybe some *
            if (_family != "*") {
                std::string               buf;
                stringstream              ss(_family);  // Insert the string into a stream
                bool                      gotit = false;
                while (ss >> buf){  //tokenize the string passed with the filter
                    //std::cout << buf << " --- " << obj->Family() << std::endl;
                    // check if the filter string match the Family name of the DiagnVar passed as obj 
                    if (fnmatch( buf.c_str(), obj->Family().c_str(), FNM_CASEFOLD)==0) gotit=true ;
                }
                if (!gotit) return false;
            }
 
            return true;
        }


    public:
        static const int   All = -1;
        static const bool  Slow = true;
        static const bool  Fast = false;
        static const bool  Enabled  = true;
        static const bool  Disabled = false;

    private:
        friend class boost::serialization::access;
        template<class Archive>
        void  serialize(Archive & ar, const unsigned int /* file_version */) {
            ar & _family & _from & _to & _slow & _ena;
        }

        friend std::ostream &operator<<(std::ostream & output, const DiagnWhich & p)
        {
            output << setw(25) << std::left << p._family << " " ;
			output << setw(6); //output << "[";
            if (p._from == All) output << "All"; else output << p._from;
            output << setw(6); //output << "/"; 
            if (p._to == All) output << "All"; else output << p._to;
            //output << "]";
			return output;   
        }
        
        friend std::istream &operator>>(std::istream & input, DiagnWhich & p)
        {
            std::string idx;
            input >>  p._family;
            input >> idx;
            if (!idx.compare("All") || !idx.compare("all")) p._from=All;
            else p._from = atoi(idx.c_str());
            input >> idx;
            if (!idx.compare("All") || !idx.compare("all")) p._to=All;
            else p._to = atoi(idx.c_str());
            return input;   
        }

       
    protected:
        std::string     _family;
        int             _from;
        int             _to;
        int             _slow;
        int             _ena;
};
#include <boost/serialization/tracking.hpp>
BOOST_CLASS_TRACKING(DiagnWhich, boost::serialization::track_never);


//@Class: ParamDict
//
//@
#include "DiagnParam.h"
#include <boost/serialization/map.hpp>
typedef std::map<DiagnWhich, DiagnParam> ParamDict;
#include <boost/serialization/tracking.hpp>
BOOST_CLASS_TRACKING(ParamDict, boost::serialization::track_never);


//@Class: WhichParam
//
//@
#include "DiagnParam.h"
struct WhichParam{
    DiagnWhich wh;
    DiagnParam pa;
};

//@Class: ValueDict
//
//@
#include "DiagnValue.h"
#include <boost/serialization/map.hpp>
typedef std::map<DiagnWhich, DiagnValue> ValueDict;
#include <boost/serialization/tracking.hpp>
BOOST_CLASS_TRACKING(ValueDict, boost::serialization::track_never);


//@Class: BufferDict
//
//@
#include "DiagnBuffer.h"
#include <boost/serialization/map.hpp>
//typedef std::map<DiagnWhich, DiagnBuffer > BufferDict;
typedef std::map<DiagnWhich, DiagnBufferPtr > BufferDict;
#include <boost/serialization/tracking.hpp>
BOOST_CLASS_TRACKING(BufferDict, boost::serialization::track_never);


#endif

