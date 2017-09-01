//@File: DiagnSet.h
//@
#ifndef DIAGNSET_H_INCLUDED
#define DIAGNSET_H_INCLUDED


#include <iostream>
#include <iterator>
#include <stdexcept>
#include <sstream>

#include <boost/serialization/tracking.hpp>
#include <boost/serialization/shared_ptr.hpp>
#include <boost/serialization/vector.hpp>
#ifndef BOOST_SERIALIZATION_SHARED_PTR_HPP
    #include "boost/smart_ptr.hpp"
#endif

#include "DiagnVar.h"
#include "DiagnWhich.h"

using namespace Arcetri;

class DiagnSet;
typedef boost::shared_ptr<DiagnSet> DiagnSetPtr;
typedef boost::shared_ptr<DiagnVar> DiagnVarPtr;

/*
 * Generic exception for the DiagnSet
 */
class DiagnSetException: public AOException {
	
	public:
		DiagnSetException(
                std::string message,
                int errcode = NO_ERROR,
                std::string file="",
                int line = 0
                ): AOException(message, errcode, file, line) 
        { 
            exception_id = "DiagnSetException"; 
        }
		
        virtual ~DiagnSetException() throw() {}
        
        std::string str()
        {
            std::ostringstream s;
            //s << _rng << std::endl;
            return s.str();
        }         
};


class DiagnSet : public  std::vector<DiagnVarPtr>
{
    public:
        DiagnSet() 
        {
            if (DEBUG) std::cout << "DiagnSet c'tor " << ++_ctr  << std::endl; 
        }
        ~DiagnSet()
        {
            if (DEBUG) std::cout << "DiagnSet d'tor " << --_ctr  << std::endl; 
        }
        DiagnSet(const DiagnSet& p) : std::vector<DiagnVarPtr>(p)
        {
            if (DEBUG) std::cout << "DiagnSet copyc'tor " << ++_ctr  << std::endl;
        }
        DiagnSet& operator=(const DiagnSet& p)
        {
            std::vector<DiagnVarPtr>::operator=(p);
            if (DEBUG) std::cout << "DiagnSet operator= " << ++_ctr  << std::endl;
            return *this;
        }

        //@Member: operator|
        //
        // Union of 2 DiagnSet //TODO
        //@
        friend DiagnSetPtr  operator| (DiagnSetPtr uno, DiagnSetPtr due);

        //@Member: operator&
        //
        // Intersection of 2 DiagnSet 
        //@
        friend DiagnSetPtr  operator& (DiagnSetPtr uno, DiagnSetPtr due);


        //@Member: Add
        //
        //Insert a new item in the set.
        //@
        void Add (DiagnVarPtr varptr);

        //@Member: Filter
        //Return a filtered subset of the DiagnSet
        //
        //This method allocates a DiagnSet, fills it with the elements matching 
        //the specified FilterDiagnVar and returns it.
        //
        //Everything is done with pointers to the objects and no real data copying is done.
        //@
        DiagnSetPtr Filter(const DiagnWhich& filter_var);

        //@Member: Find
        //
        //ritorna l'item o tira un'eccezione se non la trova;
        //@
        DiagnVarPtr Find(const MatchDiagnVar& matchvar );

        //@Member: GetValue
        //Return a vector containing the values of the DiagnVars in the DiagnSet
        //
        // WARNING: you usually need to act on DiagnSet in an atomic way protected by mutex.
        //@
        vector<double> GetValues();

    private:
        friend class boost::serialization::access;
        template<class Archive>
        void  serialize(Archive & ar, const unsigned int /* file_version */){
            ar & boost::serialization::base_object<std::vector<DiagnVarPtr> >(*this);
        }

    public:
        static int _ctr; 
};


BOOST_CLASS_TRACKING(DiagnSet, boost::serialization::track_never);


//inspiegabilmente copy_if non e' parte degli algoritmi standard
#include <algorithm>
using namespace std;

namespace std { 
    template <typename InIter, typename OutIter, typename Pred>
    OutIter copy_if(InIter first, InIter last, OutIter result, Pred pred)
    {
        for (; first != last; ++first)
            if(pred(*first)){
                *result = *first;
                ++result;
            }
        return result;
    }
}

#endif
