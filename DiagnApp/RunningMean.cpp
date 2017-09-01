//@File: RunningMean.cpp
//
// Implementation of the RunningMean class
//@


#include <string>
#include "RunningMean.h"
#include "AOExcept.h"
#include <iostream>


/*template<typename T>
RunningMean<T>::RunningMean()
{
    _req_sz=1;
    _mean=0;
    _variance=0;
}*/

template<typename T>
RunningMean<T>::RunningMean(size_type sz)
{
    if (sz == 0) throw "Zero size";
    _req_sz = sz;
    _n_elem = 0;
    _mean = 0;          //TODO std::numeric_limits<T>::quiet_NaN(); or  0 ?
    _variance = 0;      //TODO std::numeric_limits<T>::quiet_NaN();  or 0 ?
    _update_period_ave=0.0;
    _update_period_min=std::numeric_limits<float>::infinity(); //1.0e30;
    _update_period_max=0.0;
    _ctr=0;
    _last_update=0;
    _update_period_countdown=100;
}

template<typename T>
RunningMean<T>::~RunningMean() {};

template<typename T>
void RunningMean<T>::Update(T const& dato)
{

    PushOne(dato);
    if(_n_elem == _req_sz+1) //keep it here: minimize variance rounding error
        PopOne();
}

template<typename T>
void RunningMean<T>::Resize(size_type sz)
{
    if (sz == 0) throw AOException("Cannot resize a running mean to zero length", NO_ERROR, __FILE__, __LINE__);
    while (sz<_n_elem)
        PopOne();
    _req_sz=sz;
}

template<typename T>
void RunningMean<T>::PushOne(T const& dato)
{
    T delta;
    this->push_back(dato);
    _n_elem++;

    delta       = dato - _mean;
	_mean      += delta/_n_elem;
	_variance  += delta * (dato - _mean); //see Knuth vol 2

    // update period
    if ( _ctr % _update_period_countdown == 0){
        if ( _ctr == 0) _t_update.start();
        else {
            _t_update.stop();
            _update_period_ave = _t_update.sec()/(_ctr-_last_update);
            _t_update.start();
            _last_update = _ctr;
            if ( _update_period_ave < _update_period_min) _update_period_min = _update_period_ave ;
            if ( _update_period_ave > _update_period_max) _update_period_max = _update_period_ave ;
        }
    }
    _ctr++;
}

template<typename T>
void RunningMean<T>::PopOne()
{
    if (_n_elem < 2) throw AOException("RunningMean cannot be empty",NO_ERROR, __FILE__, __LINE__);
    T delta;
    T poppedValue = this->front();

    this->pop_front();
    _n_elem--;

    if(!isfinite(poppedValue)) {
    	typename std::deque<T>::iterator iter;
    	_mean = 0;
    	_variance = 0;
    	for(iter = this->begin(); iter != this->end(); iter++) {
    		_mean += *iter;
    		_variance += (*iter)*(*iter);
    	}
    	_mean /= _n_elem;
    	_variance = _variance/_n_elem - _mean*_mean;
    }
    else {
    	delta      = poppedValue - _mean;
		_mean     -= delta/(_n_elem);
		_variance -= delta * (poppedValue - _mean);
    }



    if (_variance < 0) _variance=0; //could be -epsilon //TODO can't be moved to Variance()?
}

template<typename T>
void RunningMean<T>::Dump()
{
    std::cout << "Buffer: ";
    typename std::deque<T>::iterator it;
    for(it = this->begin(); it != this->end(); it++)
        std::cout << *it << " ";

    //std::copy(_coda.begin(), _coda.end(),
    //    std::ostream_iterator<T>(std::cout, " "));
    std::cout << std::endl;
}

// see http://www.parashift.com/c++-faq-lite/templates.html#faq-35.13

template RunningMean<float>::RunningMean(size_type);
template RunningMean<float>::~RunningMean();
template void RunningMean<float>::Resize(size_type);
template void RunningMean<float>::Update(float const&);
template void RunningMean<float>::Dump();
template void RunningMean<float>::PushOne(float const&);
template void RunningMean<float>::PopOne();

template RunningMean<double>::RunningMean(size_type);
template RunningMean<double>::~RunningMean();
template void RunningMean<double>::Resize(size_type);
template void RunningMean<double>::Update(double const&);
template void RunningMean<double>::Dump();
template void RunningMean<double>::PushOne(double const&);
template void RunningMean<double>::PopOne();
