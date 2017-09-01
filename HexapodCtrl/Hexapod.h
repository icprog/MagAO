//@File: Hexapod.h
//
// General library for the hexapod
//
// Assumptions (based on 610a046d):
//
// 0) Positions and angles are in um and arcsec respectively
// 1) Hexapod has finished initialization after an INIT cmd when P751 passes from 0 to 1
// 2) Status of brakes is indicated by P750: 0=closed, 1=open
// 3) CLOSE_BRAKE: brakes are engaged when P751 changes from 1 to 0. (TODO GIGI, non e' forse P750???)
// 4) SET_POS_ABS: 200ms after the command, M5187 changes from 1 to 0. Movement completed when M5187 changes from 0 to 1.
// 5) HOME (enable plc 7) is completed when P751 changes from 0 to 1
// 6) 
//@

#ifndef HEXAPOD_H_INCLUDE
#define HEXAPOD_H_INCLUDE

#include "AOExcept.h"
#include "TcpConnection.h"
#include "aotypes.h"
#include "Logger.h"
#include "stdconfig.h"
#include "Timing.h"
#include <iomanip>

#include <pthread.h>
#include <cmath>

using namespace Arcetri;

namespace Arcetri {

typedef int    SOCKET;

class HexaBool {
    public:
        HexaBool(){};
        //HexaBool(float32 x, float32 y, float32 z, float32 a, float32 b, float32 c):
        // _x(x), _y(y), _z(z), _a(a), _b(b), _c(c) {};  
        HexaBool(bool x, bool y, bool z, bool a, bool b, bool c):
         _x(x), _y(y), _z(z), _a(a), _b(b), _c(c) {};  
        //HexaBool(int x, int y, int z, int a, int b, int c):
        // _x(x), _y(y), _z(z), _a(a), _b(b), _c(c) {};  
        ~HexaBool(){};

        bool And() { return _x && _y && _z && _a && _b && _c; }
        bool Or()  { return _x || _y || _z || _a || _b || _c; }

        friend std::ostream &operator<<(std::ostream & output, const HexaBool & p)
        {
            output  << boolalpha << "("<< setw(5) << p._x <<", "<< setw(5) << p._y <<", "<< setw(5)<< p._z<<", ";
            output  << setw(5)<< p._a <<", "<< setw(5)<< p._b <<", "<< setw(5)<< p._c<<") ";
            return output;   
        }

		string str() {
            std::ostringstream output;
            output  << *this;
            return output.str();
		}

        bool _x;
        bool _y;
        bool _z;
        bool _a;
        bool _b;
        bool _c;
};


class HexaTuple {

    public:
        HexaTuple(){_x=0; _y=0; _z=0; _a=0; _b=0; _c=0;};
        HexaTuple(float32 x, float32 y, float32 z, float32 a, float32 b, float32 c):
         _x(x), _y(y), _z(z), _a(a), _b(b), _c(c) {};  
        HexaTuple(double* p) : _x(p[0]), _y(p[1]), _z(p[2]), _a(p[3]), _b(p[4]), _c(p[5]) {};
        ~HexaTuple(){};
        
        /*
         * Return the hexapod position tuple as a C++ vector, useful
         * for using with RTDBvar::Set method
         */
        vector<double> asVector() {
        	vector<double> v(6,0);
        	v[0] =_x;// .push_back(_x);
        	v[1] =_y;// .push_back(_y);
        	v[2] =_z;// .push_back(_z);
        	v[3] =_a;// .push_back(_a);
        	v[4] =_b;// .push_back(_b);
        	v[5] =_c;// .push_back(_c);
        	return v;
        }

        friend std::ostream &operator<<(std::ostream & output, const HexaTuple & p)
        {
            output  << "("<< p._x <<", "<< p._y <<", "<< p._z<<", ";
            output  << p._a <<", "<< p._b <<", "<< p._c<<") ";
            return output;   
        }

        string str() {
            std::ostringstream output;
            output  << "("<< _x <<", "<< _y <<", "<< _z<<", ";
            output  << _a <<", "<< _b <<", "<< _c<<") ";
            return output.str();
        }

        HexaTuple operator+ (HexaTuple p) {
            HexaTuple temp;
            temp._x = _x + p._x;
            temp._y = _y + p._y;
            temp._z = _z + p._z;
            temp._a = _a + p._a;
            temp._b = _b + p._b;
            temp._c = _c + p._c;
            return (temp);
        }
        void operator+= (HexaTuple p) {
            _x += p._x;
            _y += p._y;
            _z += p._z;
            _a += p._a;
            _b += p._b;
            _c += p._c;
        }

        // _ALL_ elements of the Hexatuple are == of the corresponding elements of p
        bool operator ==(HexaTuple p) {
            return (  (_x == p._x) && (_y == p._y) && (_z == p._z) && (_a == p._a) && (_b == p._b) && (_c == p._c) ); 
        }

        // _ALL_ elements of the Hexatuple are <= of the corresponding elements of p
        bool operator <=(HexaTuple p) {
            return (  (_x <= p._x) && (_y <= p._y) && (_z <= p._z) && (_a <= p._a) && (_b <= p._b) && (_c <= p._c) ); 
        }

        // _ALL_ elements of the Hexatuple are >= of the corresponding elements of p
        bool operator >=(HexaTuple p) {
            return (  (_x >= p._x) && (_y >= p._y) && (_z >= p._z) && (_a >= p._a) && (_b >= p._b) && (_c >= p._c) ); 
        }
        
        // _ALL_ elements of the Hexatuple are < of the corresponding elements of p
        bool operator <(HexaTuple p) {
            return (  (_x < p._x) && (_y < p._y) && (_z < p._z) && (_a < p._a) && (_b < p._b) && (_c < p._c) ); 
        }

        // _ALL_ elements of the Hexatuple are > of the corresponding elements of p
        bool operator >(HexaTuple p) {
            return (  (_x > p._x) && (_y > p._y) && (_z > p._z) && (_a > p._a) && (_b > p._b) && (_c > p._c) ); 
        }

        float32 distance(HexaTuple p) {
            return sqrt( (_x-p._x)*(_x-p._x) + (_y-p._y)*(_y-p._y) + (_z-p._z)*(_z-p._z) );
        }

        float32 distance() {
            HexaTuple p;
            return distance(p);
        }

        float32 tilt(){
            return sqrt(_a*_a+_b*_b);
        }

		/*
		 * TODO This implementation could be changed to a vector<float32>
         * TODO Tosetti, ripensaci. _x, _y, _z sono pubblici. LB
		 */
        float32 _x;
        float32 _y;
        float32 _z;
        float32 _a;
        float32 _b;
        float32 _c;

    private:
};


/*
 * Defines the object returned by the GetStatus method
 */
 class HexaStatus {
	 public:
		HexaStatus() {};
		HexaStatus(
		HexaBool homed,
		HexaBool limit,
		HexaBool running,
		HexaBool openloop,
		HexaBool inposition,
		HexaBool followingerr,
		HexaBool ampfault,
		HexaBool limitposhw,
		HexaBool limitneghw,
		bool     brakesopen,
		bool     initialized,
		bool     ismoving
		):
		_homed(homed),
		_limit(limit),
		_running(running),
		_openloop(openloop),
		_inposition(inposition),
		_followingerr(followingerr),
		_ampfault(ampfault),
		_limitposhw(limitposhw),
		_limitneghw(limitneghw),
		_brakesopen(brakesopen),
		_initialized(initialized),
		_ismoving(ismoving)
		{};

	    ~HexaStatus() {};

        friend std::ostream &operator<<(std::ostream & output, const HexaStatus & p)
        {
            output  << " homed         (Mx45) " << p._homed << std::endl;  
            output  << " limit         (Mx30) " << p._limit << std::endl;  
            output  << " running       (Mx37) " << p._running << std::endl;  
            output  << " openloop      (Mx38) " << p._openloop << std::endl;  
            output  << " inposition    (Mx40) " << p._inposition << std::endl;  
            output  << " followingerr  (Mx42) " << p._followingerr << std::endl;  
            output  << " ampfault      (Mx43) " << p._ampfault << std::endl;  
            output  << " limitposhw    (Mx21) " << p._limitposhw << std::endl;  
            output  << " limitneghw    (Mx22) " << p._limitneghw << std::endl;  
            output  << " brakes open   (P750) " << boolalpha << p._brakesopen << std::endl;  
            output  << " initialized   (P751) " << boolalpha << p._initialized << std::endl;  
            output  << " moving       (M5187) " << boolalpha << p._ismoving << std::endl;  
            return output;   
        }

        string str() {
            std::ostringstream output;
			output << *this;
            return output.str();
        }

		void  log(Logger* logger, int level) {
			logger->log(level, " homed         (Mx45) %s", _homed.str().c_str());
            logger->log(level, " limit         (Mx30) %s", _limit.str().c_str());
            logger->log(level, " running       (Mx37) %s", _running.str().c_str());
            logger->log(level, " openloop      (Mx38) %s", _openloop.str().c_str());
            logger->log(level, " inposition    (Mx40) %s", _inposition.str().c_str());
            logger->log(level, " followingerr  (Mx42) %s", _followingerr.str().c_str());
            logger->log(level, " ampfault      (Mx43) %s", _ampfault.str().c_str());
            logger->log(level, " limitposhw    (Mx21) %s", _limitposhw.str().c_str());
            logger->log(level, " limitneghw    (Mx22) %s", _limitneghw.str().c_str());
			std::ostringstream brakesopenstring;
			brakesopenstring << boolalpha << _brakesopen;
			logger->log(level, " brakes open   (P750) %s", brakesopenstring.str().c_str());
			std::ostringstream initializedstring;
			initializedstring << boolalpha << _initialized;
			logger->log(level, " initialized   (P751) %s", initializedstring.str().c_str());
			std::ostringstream ismovingstring;
			ismovingstring << boolalpha << _ismoving;
			logger->log(level, " moving       (M5187) %s", ismovingstring.str().c_str());
		}

	 private:
		HexaBool _homed;
		HexaBool _limit;
		HexaBool _running;
		HexaBool _openloop;
		HexaBool _inposition;
		HexaBool _followingerr;
		HexaBool _ampfault;
		HexaBool _limitposhw;
		HexaBool _limitneghw;
		bool     _brakesopen;
		bool     _initialized;
		bool     _ismoving;
 };
 

/*
 * Generic exception for the Hexapod connection
 */
class HexapodException: public AOException {
        
        public:
                HexapodException(string m): AOException(m) { exception_id = "HexapodException"; }
                virtual ~HexapodException() {}
};


/*@Class: Hexapod
 * Hi-level communication with Hexapod
 * 
 * This class provides hi-level routines to command the Hexapod.
 * @
 */
class Hexapod {
	
	public:
	
        /*
         * Default constructor.
         * Reads the hexapod parameters from the default config file.
         */
        Hexapod(string conffile) throw (Config_File_Exception, TcpCreationException);
             
        ~Hexapod();

        /*
         * Default commands
         */
        void      DeInit();

		bool      isHomed();
        void      Home();

        void      OpenBrake();
        void      CloseBrake();
        bool      isBrakeOpen();

        bool      isInitialized();
        bool      isMoving();
        
        //bool      isRunning();
        bool      isOpenLoop();
        bool      isFollowingError();
        bool      isAmplifierFault();
        bool      isLimitsPositiveHW();
        bool      isLimitsNegativeHW();

        bool      commandOutOfBound(HexaTuple p);
        bool      isSimulator() {return _simulator;}

        void         MoveTo(HexaTuple) throw (HexapodException);
        void         MoveBy(HexaTuple);
        HexaTuple    GetPos() throw (HexapodException);
        HexaStatus	 GetStatus() throw (HexapodException);
        HexaBool     ActsHomed();
        HexaBool     ActsLimit();
        HexaBool     ActsRunning();
        HexaBool     ActsOpenLoop();
        HexaBool     ActsInPosition();
        HexaBool     ActsFollowingError();
        HexaBool     ActsAmplifierFault();
        HexaBool     LimitsPositiveHW();
        HexaBool     LimitsNegativeHW();

        void      MoveOnSphere(float, float, float);

	    //private: // TODO this should be private (now used by hexapodTest)

		string    sendCmd(string command)  throw (TcpSendException, TcpReceiveException, TcpTimeoutException, HexapodException);
	
      void Connect();   

	private:

		SOCKET  sUmac;

        static const int VR_UPLOAD = 0xC0;
        static const int VR_DOWNLOAD = 0x40;

        static const int VR_PMAC_SENDLINE = 0xB0;
        static const int VR_PMAC_GETLINE = 0xB1;
        static const int VR_PMAC_FLUSH = 0xB3;
        static const int VR_PMAC_GETMEM = 0xB4;
        static const int VR_PMAC_SETMEM = 0xB5;
        static const int VR_PMAC_SETBIT = 0xBA;
        static const int VR_PMAC_SETBITS = 0xBB;
        static const int VR_PMAC_PORT = 0xBE;
        static const int VR_PMAC_GETRESPONSE = 0xBF;
        static const int VR_PMAC_READREADY = 0xC2;
        static const int VR_CTRL_RESPONSE = 0xC4;
        static const int VR_PMAC_GETBUFFER = 0xC5;
        static const int VR_PMAC_WRITEBUFFER = 0xC6;
        static const int VR_PMAC_WRITEERROR = 0xC7;
        static const int VR_FWDOWNLOAD = 0xCB;
        static const int VR_IPADDRESS = 0xE0;

        typedef struct tagEthernetCmd
        {
            unsigned char  RequestType;
            unsigned char  Request;
            unsigned short wValue;
            unsigned short wIndex;
            unsigned short wLength;
            unsigned char  bData[1492];
        } ETHERNETCMD, *PETHERNETCMD;
        static const int EC_SIZE=8;     // length of the EthernetCmd structure without bData array

        int                 _pmac_port;
        string              _pmac_ip;		
        int                 _timeout_ms;
        TcpConnection*      _tcpConn;

        static pthread_mutex_t    _sendMutex; // to serialize sending

        Logger*             _logger;
        HexaTuple           _last_abs_position; 
        HexaTuple           _init_position;
        float               _linear_speed;
        float               _rot_speed;
        //float               _sphere_radius;

        Timing              _last_move_time;

        bool                _simulator;
        bool                _sim_open;
        bool                _sim_homed;

        float               _bound_radius; 
        float               _bound_z; 
        float               _bound_tilt; 
        HexaTuple           _park;

        bool                _connected;
        int                 _loglevel;
};

} //end namespace Arcetri


#endif

