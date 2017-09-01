//@File: DiagnValue.h
//
//Definition of the class DiagnValue.
//
//author: Lorenzo Busoni.
//
//version 1.0
//
//Created on: 21 Sep 2006
//
//Modification history:
//@ 
#ifndef DIAGNVALUE_H_INCLUDED
#define DIAGNVALUE_H_INCLUDED

#include <string>
#include <iostream>
#include <iomanip>  //setw
#include <boost/archive/binary_oarchive.hpp> //serialize



namespace Arcetri {
	namespace Diagnostic {

        //@Class Status
        //Define the meaning of status for DiagnValue (and DiagnVar)
        //@
        class Status {
            public:
                static const int OK           = 0;      // Value is in allowed range
                static const int WARNING      = 1;      // Value is in the warning range
                static const int ALARM        = 2;      // Value is in the alarm range
        };


        //@Class: DiagnValue
        //
        // Value object for a DiagnVar.
        //
        // This class contains all the quantities useful to describe the
        // present value of a DiagnVar object.
        //  
        //@
        class DiagnValue
        {
            ////////////// methods   ////////////
            public:
            DiagnValue(
                    double mean=0, 
                    double stddev=0,
                    double last=0,
                    double time=0,
                    int    status=Status::OK
                    );

            double              Mean() {return _mean;}
            double              Stddev() {return _stddev;}
            double              Last() {return _last;}
            double              Time() {return _time;}
            int                 Status() {return _status;}
            std::string         str() { std::ostringstream out; out << *this; return out.str();}

            private:
            friend class boost::serialization::access;
            template<class Archive>
            void  serialize(Archive & ar, const unsigned int /* file_version */){
                ar & _mean & _stddev & _last & _time & _status;
            }

            friend std::ostream &operator<<(std::ostream & output, const DiagnValue & p)
            {
                output.precision(3);
                output <<  " Mean: "  << p._mean;
                output <<  " StdDev: "  << p._stddev;
                output <<  " Last: "  << p._last;
                output <<  " Time: "  << p._time;
                output <<  " Status: "  << p._status;
                return output;   
            }
            
            ////////////// variables ////////////
            private:
                double                   _mean;
                double                   _stddev;
                double                   _last;
                double                   _time;
                int                      _status;
        };

    } // namespace Diagnostic
} // namespace Arcetri

using namespace Arcetri::Diagnostic;
#include <boost/serialization/tracking.hpp>
BOOST_CLASS_TRACKING(DiagnValue, boost::serialization::track_never);

#endif

