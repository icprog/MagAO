//@File: DiagnBuffer.h
//
//Definition of the class DiagnBuffer.
//
//author: Lorenzo Busoni.
//
//version 1.0
//
//Created on: 1 Dec 2006
//
//Modification history:
//@ 
#ifndef DIAGNBUFFER_H_INCLUDED
#define DIAGNBUFFER_H_INCLUDED

#include <string>
#include <iostream>
#include <iomanip>  //setw

#include <boost/archive/binary_oarchive.hpp> //serialize

#include "RunningMean.h"

//@Class: DiagnBuffer
//
// Buffer object for exporting DiagnVar running mean buffers.
//
// This class contains all data from the running mean buffers 
// (both the one containing values and the one containing timestamp)
// of a DiagnVar object. 
//@
class DiagnBuffer
{
    ////////////// methods   ////////////
    public:
		DiagnBuffer() {};
        DiagnBuffer(
				RunningMean<double>& times,
				RunningMean<double>& values
                ):
            _times(times),
            _values(values) {};

        RunningMean<double>&            Times()  {return _times;}
        RunningMean<double>&            Values() {return _values;}
        std::string         str() { std::ostringstream out; out << *this; return out.str();}

    private:
        friend class boost::serialization::access;
        template<class Archive>
        void  serialize(Archive & ar, const unsigned int /* file_version */){
            ar & _times & _values;
        }

    friend std::ostream &operator<<(std::ostream & output, const DiagnBuffer & p)
    {
        output.precision(3);
        output <<  " Times: "  << const_cast<DiagnBuffer&>(p)._times;
        output <<  " Values: "  << const_cast<DiagnBuffer&>(p)._values;
        return output;   
    }
    
    ////////////// variables ////////////
    private:
		RunningMean<double> 	_times;
		RunningMean<double>  	_values;
};


// TODO DOC cosa vuol dire?? con quelle 5 righe sotto devo levare queste 2. Perche'?
//#include <boost/serialization/tracking.hpp>
//BOOST_CLASS_TRACKING(DiagnBuffer, boost::serialization::track_never);


#include <boost/serialization/shared_ptr.hpp>
#ifndef BOOST_SERIALIZATION_SHARED_PTR_HPP
    #include "boost/smart_ptr.hpp"
#endif
typedef boost::shared_ptr<DiagnBuffer> DiagnBufferPtr;


#endif

