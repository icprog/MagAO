#ifndef DIAGNVARACCUMULATOR_H_INCLUDED
#define DIAGNVARACCUMULATOR_H_INCLUDED


//class FunctParseAccumulatorPtr ; //
#include "FastDiagnostic_convert.h"

using namespace Arcetri;
using namespace Arcetri::Diagnostic;



class DiagnVarAccumulator: public DiagnVar
{
	public:
		DiagnVarAccumulator(
        	const std::string  &family_name,
			unsigned int        index,
			FunctParseAccumulatorPtr  fConv,
			FunctActionPtr            warning_hndl,
			FunctActionPtr            alarm_hndl
        ):
		DiagnVar(family_name, index, fConv, warning_hndl, alarm_hndl){;}

		//long int GetNumSamples() { return (static_cast<FunctParseAccumulator*>(get_pointer(_fConv)))->GetNumSamples();}
		long int GetNumSamples() { return (boost::static_pointer_cast<FunctParseAccumulator, Funct>
                       (_fConv))->GetNumSamples();}
	private:
		//long int _num_samples;
};
#include <boost/serialization/shared_ptr.hpp>
#ifndef BOOST_SERIALIZATION_SHARED_PTR_HPP
    #include "boost/smart_ptr.hpp" 
#endif
typedef boost::shared_ptr<DiagnVarAccumulator> DiagnVarAccumulatorPtr;


#endif
