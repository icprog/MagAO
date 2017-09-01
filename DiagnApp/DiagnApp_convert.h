#ifndef DIAGNAPP_CONVERT_H_INCLUDED
#define DIAGNAPP_CONVERT_H_INCLUDED

#include <iostream>


#include "Funct.h"
#include "Logger.h"

extern "C" {
#include "aotypes.h"        // float32
}


namespace Arcetri {
    namespace Diagnostic {
        //TODO I'd like to do something like this: Logger * _logger = Logger::get("DIAGNAPPCONVERT");

        class FunctFloat32 : public Funct {
            public:
                FunctFloat32(float32 *x) : x_(x)
                {
                   _logger = Logger::get("DIAGNAPPCONVERT", Logger::stringToLevel(AOApp::ConfigDictionary()["DIAGNAPPCONVERT_LOG_LEV"]) );
                   _logger->log(Logger::LOG_LEV_TRACE, "FunctFloat32 constructor(float32*) [%s:%d]", __FILE__, __LINE__);
                }
                virtual void doit(double *v) 
                {
                    *v = (double)(*x_);
                    _logger->log(Logger::LOG_LEV_TRACE, "FunctFloat32  *v=%g (double*)=%p [%s:%d]", *v, x_, __FILE__, __LINE__);
                }
            private:
                float32  *x_;
                Logger* _logger;
        };

        class FunctDouble : public Funct {
            public:
                FunctDouble(double *x) : x_(x) {
                   _logger = Logger::get("DIAGNAPPCONVERT", Logger::stringToLevel(AOApp::ConfigDictionary()["DIAGNAPPCONVERT_LOG_LEV"])  );
                   _logger->log(Logger::LOG_LEV_TRACE, "FunctDouble constructor(double*)");
                }
                virtual void doit(double *v) 
                {
                    *v = (double)(*x_);
                    _logger->log(Logger::LOG_LEV_TRACE, "FunctDouble  *v=%g (double*) %p [%s:%d]", *v, x_, __FILE__, __LINE__);
                }
            private:
                double  *x_;
                Logger* _logger;
        };

        class FunctFloat : public Funct {
            public:
                FunctFloat(float *x) : x_(x) {
                   _logger = Logger::get("DIAGNAPPCONVERT", Logger::stringToLevel(AOApp::ConfigDictionary()["DIAGNAPPCONVERT_LOG_LEV"])  );
                   _logger->log(Logger::LOG_LEV_TRACE, "FunctFloat constructor(float*)");
                }
                virtual void doit(double *v) 
                {
                    *v = (double)(*x_);
                    _logger->log(Logger::LOG_LEV_TRACE, "FunctFloat  *v=%g (float*) %p [%s:%d]", *v, x_, __FILE__, __LINE__);
                }
            private:
                float  *x_;
                Logger* _logger;
        };

        class FunctUnsignedInt : public Funct {
            public:
                FunctUnsignedInt(unsigned int *x) : x_(x) {
                   _logger = Logger::get("DIAGNAPPCONVERT", Logger::stringToLevel(AOApp::ConfigDictionary()["DIAGNAPPCONVERT_LOG_LEV"])  );
                   _logger->log(Logger::LOG_LEV_TRACE, "FunctUnsignedInt constructor(unsigned int*)");
                }
                virtual void doit(double *v) 
                {
                    *v = (double)(*x_);
                    _logger->log(Logger::LOG_LEV_TRACE, "FunctUnsignedInt  *v=%g (unsigned int*) %p [%s:%d]", *v, x_, __FILE__, __LINE__);
                }
            private:
                unsigned int  *x_;
                Logger* _logger;
        };

        class FunctUint16 : public Funct {
            public:
                FunctUint16(uint16 *x) : x_(x)
                {
                   _logger = Logger::get("DIAGNAPPCONVERT",Logger::stringToLevel(AOApp::ConfigDictionary()["DIAGNAPPCONVERT_LOG_LEV"])   );
                   _logger->log(Logger::LOG_LEV_TRACE, "FunctUint16 constructor(uint16*)");
                }
                virtual void doit(double *v) 
                {
                    *v = (double)(*x_);
                    _logger->log(Logger::LOG_LEV_TRACE, "FunctUint16  *v=%g (uint16*) %p [%s:%d]", *v, x_, __FILE__, __LINE__);
                }
            private:
                uint16  *x_;
                Logger* _logger;
        };

        class FunctInt16 : public Funct {
            public:
                FunctInt16(int16 *x) : x_(x)
                {
                   _logger = Logger::get("DIAGNAPPCONVERT", Logger::stringToLevel(AOApp::ConfigDictionary()["DIAGNAPPCONVERT_LOG_LEV"])  );
                   _logger->log(Logger::LOG_LEV_TRACE, "FunctInt16 constructor(int16*)");
                }
                virtual void doit(double *v) 
                {
                    *v = (double)(*x_);
                    _logger->log(Logger::LOG_LEV_TRACE, "FunctInt16  *v=%g (int16*) %p [%s:%d]", *v, x_, __FILE__, __LINE__);
                }
            private:
                int16  *x_;
                Logger* _logger;
        };

        class FunctUint32 : public Funct {
            public:
                FunctUint32(uint32 *x) : x_(x)
                {
                   _logger = Logger::get("DIAGNAPPCONVERT",Logger::stringToLevel(AOApp::ConfigDictionary()["DIAGNAPPCONVERT_LOG_LEV"])   );
                   _logger->log(Logger::LOG_LEV_TRACE, "FunctUint32 constructor(uint32*)");
                }
                virtual void doit(double *v) 
                {
                    *v = (double)(*x_);
                    _logger->log(Logger::LOG_LEV_TRACE, "FunctUint32  *v=%g (uint32*) %p [%s:%d]", *v, x_, __FILE__, __LINE__);
                }
            private:
                uint32  *x_;
                Logger* _logger;
        };

        class FunctInt32 : public Funct {
            public:
                FunctInt32(int32 *x) : x_(x)
                {
                   _logger = Logger::get("DIAGNAPPCONVERT", Logger::stringToLevel(AOApp::ConfigDictionary()["DIAGNAPPCONVERT_LOG_LEV"])  );
                   _logger->log(Logger::LOG_LEV_TRACE, "FunctInt32 constructor(int32*)");
                }
                virtual void doit(double *v) 
                {
                    *v = (double)(*x_);
                    _logger->log(Logger::LOG_LEV_TRACE, "FunctInt32  *v=%g (int32*) %p [%s:%d]", *v, x_, __FILE__, __LINE__);
                }
            private:
                int32  *x_;
                Logger* _logger;
        };

        class FunctUint32Bit : public Funct {
            public:
                FunctUint32Bit(uint32 *x, unsigned int bit, bool neg=false) : _x(x), _bit(bit), _neg(neg) 
                {
                   _logger = Logger::get("DIAGNAPPCONVERT", Logger::stringToLevel(AOApp::ConfigDictionary()["DIAGNAPPCONVERT_LOG_LEV"])  );
                   _logger->log(Logger::LOG_LEV_TRACE, "FunctUint32Bit constructor(uint32*, uint, bool=false)");
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
                Logger* _logger;
        };

        //
        // Return (x & mask) >> shift
        //
        class FunctUint32Mask : public Funct {
            public:
                FunctUint32Mask(uint32 *x, unsigned int mask, unsigned int shift=0, bool neg=false) : _x(x), _mask(mask), _shift(shift), _neg(neg) 
                {
                   _logger = Logger::get("DIAGNAPPCONVERT", Logger::stringToLevel(AOApp::ConfigDictionary()["DIAGNAPPCONVERT_LOG_LEV"])  );
                   _logger->log(Logger::LOG_LEV_TRACE, "FunctUint32Mask constructor(uint32*, uint mask, uint mask=0, bool=false)");
                }
                virtual void doit(double *v) 
                {
                    if (_neg==false) *v = (double)( ( (*_x) & _mask ) >> _shift );
                    else *v = (double)( ( ~(*_x) & _mask)  >> _shift ) ;
                }
            private:
                uint32  *_x;
                unsigned int  _mask;
                unsigned int  _shift;
                bool  _neg;
                Logger* _logger;
        };

	//@Class: FunctAverageDiagnSet
	//
	// Return the average value of a DiagnSet
	//
	// Return the average value of a set of DiagnVar specified by a DiagnSetPtr.
	// Only the enabled DiagnVars are used to computed the average.
	//
	// TODO: If no DiagnVar in the set is enabled, the average value returned is 0
	// Qui si puo' vedere se e' furbo lanciare eccezioni qua dentro e farle raccattare all'Update
	//
	//@
	#include "DiagnSet.h"
	class FunctAverageDiagnSet : public Funct {
	    public:
		FunctAverageDiagnSet(DiagnSetPtr vars) : _vars(vars)
		{
		   _logger = Logger::get("FUNCTAVERAGEDIAGNSET",Logger::stringToLevel(AOApp::ConfigDictionary()["DIAGNAPPCONVERT_LOG_LEV"])   );
		   _logger->log(Logger::LOG_LEV_TRACE, "FunctAverageDiagnSet constructor(DiagnSetPtr)");
		}
		virtual void doit(double *v)
		{
		    *v = 0;
		    int cnt = 0;
		    for (DiagnSet::iterator it = _vars->begin(); it != _vars->end();  it++)
		    {
			if ( (*it)->isEnabled() ) {
			    *v += (*it)->Value();
			    cnt++;
			}
		    }
		    if (cnt>0) *v /= cnt;
		    else {
			throw AOException("FunctAverageDiagnSet::doit()] All variables are disabled",GENERIC_SYS_ERROR,__FILE__,__LINE__);
		    }
	            _logger->log(Logger::LOG_LEV_TRACE, "FunctAverageDiagnSet %g (on %d of %d items)",*v, cnt, _vars->size());
		}
	    private:
		DiagnSetPtr _vars;
		Logger* _logger;
	};

	//@Class: FunctMaxAbsDiagnSet
	//
	// Return the max absolute value value of a DiagnSet
	//
	// Return the max absolute value of a set of DiagnVar specified by a DiagnSetPtr.
	// Only the enabled DiagnVars are used to computed the average.
	//
	// TODO: If no DiagnVar in the set is enabled, the value returned is 0
	// Qui si puo' vedere se e' furbo lanciare eccezioni qua dentro e farle raccattare all'Update
	//
	//@
	class FunctMaxAbsDiagnSet : public Funct {
	    public:
		FunctMaxAbsDiagnSet(DiagnSetPtr vars) : _vars(vars) {
		   _logger = Logger::get("FUNCTMAXABSDIAGNSET",Logger::stringToLevel(AOApp::ConfigDictionary()["DIAGNAPPCONVERT_LOG_LEV"])   );
		   _logger->log(Logger::LOG_LEV_TRACE, "FunctMaxAbsDiagnSet constructor(DiagnSetPtr)");
		}
		virtual void doit(double *v) {
		    *v = 0;
		    int cnt = 0;
		    for (DiagnSet::iterator it = _vars->begin(); it != _vars->end();  it++) {
			    if ( (*it)->isEnabled() ) {
                    if (isfinite((*it)->Value())) {
                       cnt++;
                       if (fabs( (*it)->Value() ) > *v)
                          *v = fabs( (*it)->Value());
                    }

			    }
		    }
	        _logger->log(Logger::LOG_LEV_TRACE, "FunctMaxAbsDiagnSet %g (on %d of %d items)",*v, cnt, _vars->size());
        }
	    private:
		DiagnSetPtr _vars;
		Logger* _logger;
	};



    } // namespace Diagnostic
} // namespace Arcetri

#endif //DIAGNAPP_CONVERT_H_INCLUDED
