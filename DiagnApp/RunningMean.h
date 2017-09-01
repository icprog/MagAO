#ifndef RUNNINGMEAN_H_INCLUDED
#define RUNNINGMEAN_H_INCLUDED

#include <math.h>
//#include <deque> //not needed included by boost/serialization
#include <iostream>
#include <limits>
#include "Timing.h"

#include <boost/archive/binary_oarchive.hpp>
#include <boost/serialization/access.hpp> //serialize
#include <boost/serialization/base_object.hpp>
#include <boost/serialization/deque.hpp>



typedef size_t size_type;

//@Class: RunningMean
//
// TODO: specializzabile solo per tipi float
//
//@
template<typename T>
class RunningMean: public std::deque<T> 
{
    //////////// construction ///////////
    public:
        RunningMean(size_type sz=1);
        ~RunningMean();

    //////////////  methods ////////////
    public:
        void              Resize(size_type); // change the interval on which the mean is computed eventually discarding oldest values
        void              Update(T const&); // 
        T                 Mean() {
                                    if (_n_elem<1) return std::numeric_limits<T>::quiet_NaN();
                                    return (fabs(_mean) < std::numeric_limits<T>::epsilon()) ? 0 : _mean;
                                 }
        T                 StdDev() {
                                    if (_n_elem<2) return std::numeric_limits<T>::quiet_NaN();
                                    return (fabs(_variance) < std::numeric_limits<T>::epsilon()) ? 0 : sqrt(_variance/(_n_elem-1));
                                   }
        T                 Variance() {
                                    if (_n_elem<2) return std::numeric_limits<T>::quiet_NaN();
                                    return (fabs(_variance) < std::numeric_limits<T>::epsilon()) ? 0 : _variance/(_n_elem-1);
                                     }
        T                 Last() {return (_n_elem > 0) ? this->back() : std::numeric_limits<T>::quiet_NaN();}
        unsigned int      Size()  {return _req_sz;}    // BEWARE Size() != size() == NElem()
        unsigned int      NElem()  {return _n_elem;}
        float             UpdateMin() {return  _update_period_min;}
        float             UpdateMax() {return  _update_period_max;}
        float             UpdateAve() {return  _update_period_ave;}
        unsigned long     Ctr() {return  _ctr;}
        void              Dump();
        void              PushOne(T const&);    // Push one element into the queue and update _mean and _variance
        void              PopOne();  // Pop one element from the queue and update  _mean and _variance

    private:
        friend class boost::serialization::access;
        template<class Archive>
        void  serialize(Archive & ar, const unsigned int /* file_version */){
            ar & _req_sz & _n_elem & _mean & _variance & _ctr;
            ar & _update_period_ave & _update_period_min & _update_period_max & _last_update;
            ar & boost::serialization::base_object<std::deque<T> >(*this);
        }

        friend std::ostream &operator<<(std::ostream & output, RunningMean<T> & p)
        {
            output << " ";
            typename std::deque<T>::iterator it;
            for(it = p.begin(); it != p.end(); it++)
                output << *it << " ";
   
            return output;
        }

    ////////////// variables ////////////
    private:
        size_type         _req_sz;   // requested size of the buffer (in samples)
        size_type         _n_elem;   // n elem (<=size) on which the mean is computed.   
        T                 _mean;     // average value;
        T                 _variance; // variance;
        unsigned long     _ctr;                     // how many times the variable has been updated
        float             _update_period_ave;       // mean time between 2 updates  [s] averaged on last _req_sz cycles
        float             _update_period_min;       // minimum of update_period_ave [s]
        float             _update_period_max;       // maximum of update_period_ave [s] 
        Timing            _t_update;                // Timer to measure update period
        unsigned long     _last_update;             // =_ctr at the last update
        unsigned long     _update_period_countdown; // how often should I compute the update_period values

       // bool              _verbose; 
};

#endif // RUNNINGMEAN_H_INCLUDED

