#ifndef FASTDIAGNOSTIC_CONVERT_H_INCLUDED
#define FASTDIAGNOSTIC_CONVERT_H_INCLUDED

#include <iostream>


#include "Funct.h"
#include "Logger.h"

extern "C" {
#include "aotypes.h"        // float32
}

using namespace Arcetri;
/*
class FunctFloat32 : public Funct {
    public:
        FunctFloat32(float32 *x) : x_(x)
        {
            Logger::get()->log(Logger::LOG_LEV_TRACE, "FunctFloat32 constructor(float32*)");
        }
        virtual void doit(double *v)
        {
            //std::cout << "FunctFloat32 address: " <<  x_ << std::endl;
            *v = (double)(*x_);
        }
    private:
        float32  *x_;
};

class FunctUint32 : public Funct {
    public:
        FunctUint32(uint32 *x) : x_(x)
        {
            Logger::get()->log(Logger::LOG_LEV_TRACE, "FunctUint32 constructor(uint32*)");
        }
        virtual void doit(double *v)
        {
            *v = (double)(*x_);
        }
    private:
        uint32  *x_;
};

class FunctUint32Bit : public Funct {
    public:
        FunctUint32Bit(uint32 *x, unsigned int bit, bool neg=false) : _x(x), _bit(bit), _neg(neg)
        {
            Logger::get()->log(Logger::LOG_LEV_TRACE, "FunctUint32Bit constructor(uint32*, uint, bool=0)");
        }
        virtual void doit(double *v)
        {
            if (_neg==false) *v = (double)((*_x >> _bit) &1 );
            else *v = (double)(  (~(*_x) >> _bit) &1) ;
        }
    private:
        uint32  *_x;
        unsigned int  _bit;
        bool  _neg;
};
*/
class FunctSlopeNorm : public Funct {
    public:
        FunctSlopeNorm(uint32 *x, unsigned int bit0) : _x(x), _bit0(bit0)
        {
            Logger::get()->log(Logger::LOG_LEV_TRACE, "FunctSlopeNorm constructor(uint32*, uint)");
        }
        virtual void doit(double *v)
        {
            switch  ( (*_x >> _bit0)   & 0x7  )
            {
                case 1  : *v=1; break;
                case 2  : *v=2; break;
                case 4  : *v=3; break;
                default : *v=0;
            }
        }
    private:
        uint32  *_x;
        unsigned int  _bit0;
};

class FunctFramesCounter : public Funct {
    public:
        FunctFramesCounter(
                uint32 *FramesCounter,
                uint32 *FramesCounterCheck
                ) :
            _FramesCounter(FramesCounter),
            _FramesCounterCheck(FramesCounterCheck)
        {
            Logger::get()->log(Logger::LOG_LEV_TRACE, "FunctFramesCounter constructor(uint32*, uint32*)");
        }
        virtual void doit(double *v)
        {
            // Check Frame integrity: if FramesCnt != FramesCntCheck give back (double)-1;
            // Set AlarmMin to 0
            if (*_FramesCounter != *_FramesCounterCheck) *v = -1.0;
            else *v = (double)(*_FramesCounter);
        }
    private:
        uint32  *_FramesCounter;
        uint32  *_FramesCounterCheck;
};


class FunctModes : public Funct {
    public:
        FunctModes(float32 *x) : x_(x)
        {
            Logger::get()->log(Logger::LOG_LEV_TRACE, "FunctModes constructor(float32*)");
        }
        virtual void doit(double *v)
        {
            *v = (double)(*x_);
        }
    private:
        float32  *x_;
};


#include "gmpxx.h"
#include <math.h>
#include "DiagnVar.h"

// TODO See GMP manual sect 4.1 to convert to C++
class FunctParseAccumulator : public Funct {
    public:
        FunctParseAccumulator(
                uint32 *acc_zero,
                uint32 *acc_uno,
                uint32 *acc2_zero,
                uint32 *acc2_uno,
                uint32 *acc2_due,
                uint32 mask,
                double fixed_point,
                DiagnVarPtr varAverage) :
            acc_zero_(acc_zero),
            acc_uno_(acc_uno),
            acc2_zero_(acc2_zero),
            acc2_uno_(acc2_uno),
            acc2_due_(acc2_due),
            mask_(mask),
            fixed_point_(fixed_point),
            varAverage_(varAverage)
        {
            _num_samples_computed_in_this_loop = false;
            _num_samples = 0;
            mpz_init(ll);
            mpz_init(lm);
            mpz_init(ln);
            mpf_init(acc);
            mpf_init(acc2);
            mpf_init(rms);
            mpf_init(fact);
            mpf_init(tmp);
            mpf_init(tmp2);
            mpz_init(two32);
            mpz_init(two64);
            mpz_set_str(two32, "4294967296", 10) ;
            mpz_set_str(two64, "18446744073709551616", 10) ;
            Logger::get()->log(Logger::LOG_LEV_TRACE, "FunctParseAccumulator c'tor(uint32*[5], uint32, double, double)");
        }

        virtual ~FunctParseAccumulator()
        {
            mpz_clear(ll);
            mpz_clear(lm);
            mpz_clear(ln);
            mpf_clear(acc);
            mpf_clear(acc2);
            mpf_clear(rms);
            mpf_clear(fact);
            mpf_clear(tmp);
            mpf_clear(tmp2);
            mpz_clear(two32);
            mpz_clear(two64);
            Logger::get()->log(Logger::LOG_LEV_TRACE, "FunctParseAccumulator d'tor()");
        }

        long int GetNumSamples() { return _num_samples;}
        
        virtual void BeginLoop() {_num_samples_computed_in_this_loop=false;}

        virtual void doit(double *v)
        {
        	   if(!varAverage_->isEnabled()) {
        		   throw AOException("FunctParseAccumulator::doit()] Variable " + varAverage_->Name() + " is disabled",GENERIC_SYS_ERROR,__FILE__,__LINE__);
        	   }

            varAveLast_ = varAverage_->Last();
            // <x> = acc =  (acc[1] * _2to32 + acc[0]) * fixed * (1/num_samples);
            mpz_set_si(ll, *acc_uno_);
            mpz_mul(ln, ll, two32);
            mpz_set_ui(lm, *acc_zero_);
            mpz_add(ll, ln, lm);
            mpf_set_z(tmp, ll);

            if (_num_samples_computed_in_this_loop == false) {
               computeNumSamples();
               
            
            //_num_samples = round(   mpf_get_d(tmp)*fixed_point_ / varAveLast_  );
            //if (_num_samples > 1024) {
            //   int div = 1024;
            //    while (round(_num_samples / div) > 1 ) {
            //        div *= 2;
            //    }
            //    _num_samples = div;
            //}
               if(_num_samples==0) {
                   Logger::get()->log(Logger::LOG_LEV_WARNING, "FunctParseAccumulator: Variable %s gave num_samples ZERO", varAverage_->Name().c_str());
               }
               else {
                   _num_samples_computed_in_this_loop=true;
               }
            }

            if (_num_samples != 0) {

                 double inv_num_samples_ = 1./ _num_samples;

                mpf_set_d(fact, (double)(fixed_point_ * inv_num_samples_));
                mpf_mul(acc, tmp, fact);

                // <x^2> = acc2 =  (acc2[2] * _2to64 + acc2[1] * _2to32 + acc2[0]) * fixed2 / _num_samples
                mpz_set_ui(ll, (*acc2_due_) & mask_);
                mpz_mul(ln, ll, two64);
                mpz_set_ui(lm, *acc2_zero_);
                mpz_add(ll, ln, lm);
                mpz_set_ui(ln, *acc2_uno_);
                mpz_mul(lm, ln, two32);
                mpz_add(ln, ll, lm);
                mpf_set_z(tmp, ln);
                mpf_set_d(fact, (double)(fixed_point_ * fixed_point_ * inv_num_samples_));
                mpf_mul(acc2, tmp, fact);

                // tmp2 = <x>^2
                mpf_mul(tmp2, acc, acc);

                // tmp = <x^2> - <x>^2
                mpf_sub(tmp, acc2, tmp2);

                // Lower bound of tmp (rms^2) to zero to avoid sqrt floating exception
                if (mpf_sgn(tmp) < 0) mpf_set_ui (tmp,0);

                // rms = sqrt( <x^2> - <x>^2 )
                mpf_sqrt(rms,tmp);
                *v = mpf_get_d(rms);

                Logger::get()->log(Logger::LOG_LEV_TRACE,
                                   "FunctParseAccumulator: acc %lu %lu, acc2 %lu %lu %lu, varAve %g, _num_samples %d, rms = %g , <x>=%g, <x^2>=%g [%s:%d]",
                                   *acc_zero_, *acc_uno_, *acc2_zero_, *acc2_uno_, *acc2_due_, varAveLast_, _num_samples,
                                   *v, mpf_get_d(acc), mpf_get_d(acc2) , __FILE__, __LINE__);
            }
        }
    private:
        virtual void computeNumSamples()
        {
             _num_samples = (long int)round(   mpf_get_d(tmp)*fixed_point_ / varAveLast_  );
            if (_num_samples > 1024) {
                int div = 1024;
                while (round(_num_samples / div) > 1 ) {
                    div *= 2;
                }
                _num_samples = div;
            }
            Logger::get()->log(Logger::LOG_LEV_TRACE,
                    "FunctParseAccumulator: computed num_samples %d from varAverage %s [%s:%d]",
                    _num_samples, varAverage_->Name().c_str(), __FILE__, __LINE__);
        }
    private:
        uint32     *acc_zero_;
        uint32     *acc_uno_;
        uint32     *acc2_zero_;
        uint32     *acc2_uno_;
        uint32     *acc2_due_;
        uint32      mask_;
        double      fixed_point_;
        double      varAveLast_;
        mpz_t ll, lm, ln, two32, two64;         // multiple precision integer numbers
        mpf_t acc, acc2, rms, fact, tmp, tmp2;  // multiple precision float numbers
    protected:
        static long int    _num_samples;
        DiagnVarPtr varAverage_;
        static bool _num_samples_computed_in_this_loop;

};


#include <boost/serialization/shared_ptr.hpp>
#ifndef BOOST_SERIALIZATION_SHARED_PTR_HPP
    #include "boost/smart_ptr.hpp" // use it if boost/serialization/shared_ptr.hpp is not included
#endif
typedef boost::shared_ptr<FunctParseAccumulator> FunctParseAccumulatorPtr;

/*
#include "DiagnVarAccumulator.h"
class FunctParseAccumulatorCurr: public FunctParseAccumulator
{
    public:
        FunctParseAccumulatorCurr(
                uint32 *acc_zero,
                uint32 *acc_uno,
                uint32 *acc2_zero,
                uint32 *acc2_uno,
                uint32 *acc2_due,
                uint32 mask,
                double fixed_point,
                DiagnVarAccumulatorPtr varAverage) :
            FunctParseAccumulator(acc_zero, acc_uno, acc2_zero, acc2_uno, acc2_due, mask,
                    fixed_point, varAverage){;}

    private:
        void computeNumSamples() {
            //_num_samples = (long int)((static_cast<DiagnVarAccumulator*>(get_pointer(varAverage_)))->GetNumSamples());
            _num_samples = (long int) ( (boost::static_pointer_cast<DiagnVarAccumulator, DiagnVar>
                       (varAverage_))->GetNumSamples() );
            Logger::get()->log(Logger::LOG_LEV_TRACE,
                    "FunctParseAccumulatorCurr: retrieved num_samples %d from varAverage %s [%s:%d]",
                    _num_samples, varAverage_->Name().c_str(), __FILE__, __LINE__);
        }
};

#include <boost/serialization/shared_ptr.hpp>
#ifndef BOOST_SERIALIZATION_SHARED_PTR_HPP
    #include "boost/smart_ptr.hpp" // use it if boost/serialization/shared_ptr.hpp is not included
#endif
typedef boost::shared_ptr<FunctParseAccumulatorCurr> FunctParseAccumulatorCurrPtr;
*/

/**
 * Modal projecxtion: project difference between actual and flat shape onto the modal basis
 * meas_pos is the vector of positions measured by the capsensors.
 * ref_pos is the vector of positions of the flat mirror (external input).
 * M is the matrix that transform positions in modes  (modes = M ## positions)
 *
 * This function compute the projection for a single mode: M(n,*) ## (meas_pos - ref_pos)(n)
 * where M(n,*) is the n-th row of M and ref_pos(n) the n-th scalar value of ref_pos.
 * meas_pos is a vector of n_act elements.
 * Disabled actuators' values are zeroed before matrix multiplication.
 */
#include "DiagnSet.h"
#include <iomanip>
class FunctModalProj : public Funct {
    public:
        FunctModalProj(
                DiagnSetPtr pos,
                int idx_modo,
                int n_modes,
                vector<double>* ref_pos,
                double* Mn) :
            _pos(pos),
            _idx_modo(idx_modo),
            _n_modes(n_modes),
            _ref_pos(ref_pos),
            _Mn(Mn)

        {
            _n_act    = pos->size();
            if (_idx_modo == 0) _f_pos       =  new double[_n_act];
            if (_idx_modo == 0) _meas_modes  =  new double[_n_modes];
            Logger::get()->log(Logger::LOG_LEV_TRACE, "FunctModalProj c'tor()");
        }

        virtual ~FunctModalProj()
        {
            if (_idx_modo == _n_modes-1) delete[] _f_pos;
            if (_idx_modo == _n_modes-1) delete[] _meas_modes;
            Logger::get()->log(Logger::LOG_LEV_TRACE, "FunctModalProj d'tor()");
        }

        virtual void copy()
        {
            if (_idx_modo==0){
            	int ctr=0;
				for (DiagnSet::iterator it = _pos->begin();   it != _pos->end();    it++) {
//					double val = (**it).isEnabled() ? (**it).Value() : 0.0;  // remove NaN
					_f_pos[ctr] = (**it).isEnabled() ? (**it).Value() - (*_ref_pos)[ctr] : 0.0;
                    ctr++;
				}
            }
        }

        virtual void doit(double *v)
        {
            if (_idx_modo==0){
            	//if(!_targetVar->isSlow()()) {
            		copy(); // TODO here suppose the var is SLOW!!!
            	//}
                matrix_vect(_Mn, _f_pos, _meas_modes, _n_modes, _n_act);
            }
            *v = _meas_modes[_idx_modo]; // - _f_modes[_idx_modo];
        }
    private:
        /*void vect_vect (float* a, float* b, float* r, int n)
        {
            float *v1 = a;
            float *v2 = b;
            float *v2_end = v2 + n;
            *r = 0;
            for ( ; v2 < v2_end; )
                *r += *v1++ * *v2++;
        }*/
        void matrix_vect(double* _matr, double* _vect, double* _r, long _n_row, long _n_column)
        {
            //if(1) std::cout << "--- FunctModalProj matrix_vect()" << std::endl;
            memset(_r,0,_n_row*sizeof(*_r));
            double *a=_matr;
            double *b=_vect;
            double *b_end = b + _n_column;
            double *c=_r;
            double *c_end = c + _n_row;
            for ( ; c < c_end; c++){
                double *b = _vect;
                for ( ; b < b_end; )
                    *c += *a++ * *b++;
            }
        }

    private:
        DiagnSetPtr     _pos;
        int     _idx_modo;
        int     _n_modes;
        int     _n_act;
        vector<double>*  _ref_pos;
        double*  _Mn;
        static double*  _f_pos;       // _pos copied
        static double*  _meas_modes;// _Mn ## _f_pos
};

class FunctSkipFrameRate : public Funct {
  public:
        FunctSkipFrameRate(DiagnVarPtr sk, DiagnVarPtr fc) :
           _sk(sk),
           _fc(fc)
        {
           _lastsk=0;
           _lastfc=0;
           _init=false;
        }
        virtual void doit(double *v)
        {
           if(!_sk->isEnabled()) {
        		   throw AOException("FunctSkipFrameRate::doit()] Variable " + _sk->Name() + " is disabled",GENERIC_SYS_ERROR,__FILE__,__LINE__);
        	  }
           if(!_fc->isEnabled()) {
        		   throw AOException("FunctSkipFrameRate::doit()] Variable " + _fc->Name() + " is disabled",GENERIC_SYS_ERROR,__FILE__,__LINE__);
        	  }
           double nowsk  = _sk->Last();
           double nowfc  = _fc->Last();
           if (_init) {
              if ( nowfc == _lastfc) {
                  // set full skipping rate for frozen counter
                  *v = 1.0;
              }
              else {
                  double rate = (nowsk - _lastsk) / (nowfc - _lastfc);
                  *v = rate;
              }
           }
           _lastsk = nowsk;
           _lastfc = nowfc;
           _init = true;
        }
    private:
        bool   _init;
        double _lastsk;
        double _lastfc;
        DiagnVarPtr _sk;
        DiagnVarPtr _fc;
};



#endif //FASTDIAGNOSTIC_CONVERT_H_INCLUDED
