#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>

#include <cstdlib>
#include <string>
#include <sstream>
#include <time.h>

#include "Hexapod.h"


extern "C" {
#include "base/timelib.h"
}



using namespace std;
using namespace Arcetri;


pthread_mutex_t    Hexapod::_sendMutex;


Hexapod::Hexapod(string conffile) throw (Config_File_Exception, TcpCreationException) {

   _connected = false;
	try {
		Config_File config(conffile);
      _loglevel = Logger::stringToLevel((config)["LogLevel"]);
		_logger = Logger::get("HEXAPOD", _loglevel);
		_logger->log(Logger::LOG_LEV_INFO, "Creating Hexapod...");
		_logger->log(Logger::LOG_LEV_INFO, "Using configuration file %s", conffile.c_str());
	
        _pmac_port  = config["PMAC_PORT"];
        _pmac_ip    = (string)config["PMAC_IP"];
        _timeout_ms = config["TIMEOUT_TCP_MS"];
		
        // TODO this is useless: don't want to use P102x for safety reasons and can't do
        // a MoveTo after homing, because Home() is non-blocking. Maybe in the HexapodCtrl?
        //_init_position = HexaTuple( 
        //        (float)config["INIT_X"],(float)config["INIT_Y"],(float)config["INIT_Z"],
        //        (float)config["INIT_A"],(float)config["INIT_B"],(float)config["INIT_C"]
        //        );
        //ostringstream slog;
        //slog <<  "Hexapod created. When homed will go to absolute position: " << _init_position;
		//_logger->log(Logger::LOG_LEV_INFO, slog.str() );

        _last_abs_position=HexaTuple(0,0,0,0,0,0);
		
        // initialize mutex
        pthread_mutex_init(&_sendMutex, NULL);
        
        // alllowable region boundaries
        //_bound_min = HexaTuple((float)config["MIN_X"],(float)config["MIN_Y"], (float)config["MIN_Z"], (float)config["MIN_A"], (float)config["MIN_B"], (float)config["MIN_C"]); 
        //_bound_max = HexaTuple((float)config["MAX_X"],(float)config["MAX_Y"], (float)config["MAX_Z"], (float)config["MAX_A"], (float)config["MAX_B"], (float)config["MAX_C"]); 
		//_logger->log(Logger::LOG_LEV_INFO, "Hexapod created with max bound %s [%s:%d]", _bound_max.str().c_str(), __FILE__, __LINE__);
		_bound_radius = (float)config["BOUND_RADIUS"];
		_bound_z      = (float)config["BOUND_Z"];
		_bound_tilt   = (float)config["BOUND_TILT"];
        _logger->log(Logger::LOG_LEV_INFO, "Hexapod created with bound radius: %f, z: %f, tilt %f [%s:%d]", _bound_radius, _bound_z, _bound_tilt, __FILE__, __LINE__);

        // park is specially allowed position 
        _park   = HexaTuple((float)config["PARK_X"],(float)config["PARK_Y"], (float)config["PARK_Z"], 
                (float)config["PARK_A"], (float)config["PARK_B"], (float)config["PARK_C"]); 
		_logger->log(Logger::LOG_LEV_INFO, "Hexapod created with fully_up    %s [%s:%d]", _park.str().c_str(), __FILE__, __LINE__);
        
        // max speed
        _linear_speed  = (float)config["MAX_LINEAR_SPEED"];
        _rot_speed     = (float)config["MAX_ROTATIONAL_SPEED"];
		_logger->log(Logger::LOG_LEV_INFO, "Hexapod created with max speed [um/s, arcsec/s] %f %f",_linear_speed, _rot_speed );
        
        //_sphere_radius = (float)config["SPHERE_RADIUS"];
        //_logger->log(Logger::LOG_LEV_INFO, "Hexapod created with sphere radius [m] %f ",_sphere_radius );

        _last_move_time.start(); 
        
        _simulator = ((int)config["SIMULATOR"] == 1) ? true : false;
        _sim_open  = false;
        _sim_homed = false;
        if (_simulator)  return;
        
        // Will connect on the first command.
        //_tcpConn = new TcpConnection(_pmac_ip, _pmac_port, (int)config["LogLevel"]);
        //_logger->log(Logger::LOG_LEV_INFO, "Hexapod created with TCP timeout %d ms", _timeout_ms);
    
    }	
	catch(AOException& e) {
		Logger::get("HEXAPOD", Logger::LOG_LEV_ERROR)->log(Logger::LOG_LEV_ERROR, "Hexapod creation error: %s [%s:%d]", e.what().c_str(), __FILE__, __LINE__);
		throw;
	}
		
};

void Hexapod::Connect() {

   if (_connected)
      return;

   try {
        _tcpConn = new TcpConnection(_pmac_ip, _pmac_port, _loglevel);
        _logger->log(Logger::LOG_LEV_INFO, "Hexapod connected with TCP timeout %d ms", _timeout_ms);
        _connected = true;
   }
   catch (TcpException &e) {
		Logger::get("HEXAPOD", Logger::LOG_LEV_ERROR)->log(Logger::LOG_LEV_ERROR, "Error connecting to hexapod: %s [%s:%d]", e.what().c_str(), __FILE__, __LINE__);
      throw;
   }
}

Hexapod::~Hexapod() {
    // DeInit() is not suitable, 'cause requires a homing to restart. 
    CloseBrake();
}


string Hexapod::sendCmd(string cmd) throw (TcpSendException, TcpReceiveException, TcpTimeoutException, HexapodException) 
{
    ETHERNETCMD ecSend;
	int iRet;
    BYTE cRet[1400];

    Connect();    // Ensure connection is up
	

    // create the command to send to umac
    const char *cCommand=cmd.c_str();
    ecSend.RequestType = VR_DOWNLOAD;
    ecSend.Request = VR_PMAC_GETRESPONSE;
    ecSend.wValue = 0;
    ecSend.wIndex = 0;
    ecSend.wLength = htons((unsigned short)strlen(cCommand));
    strncpy((char*)&ecSend.bData[0], cCommand, (unsigned short)strlen(cCommand));


    // lock mutex
 	pthread_mutex_lock(&_sendMutex); 
    _logger->log(Logger::LOG_LEV_DEBUG, "Hexapod sending  %s [%s:%d]", cmd.c_str(),__FILE__, __LINE__);

    if (_simulator) {
        const struct timespec tm = {0, 3000000};
        nanosleep(&tm, NULL);
 	    pthread_mutex_unlock(&_sendMutex); 
       _logger->log(Logger::LOG_LEV_DEBUG, "Hexapod SIMULATOR received  0.0 [%s:%d]", cRet,__FILE__, __LINE__);
        return "0.0";
    }

    // Send data using the TCP connection.
    try {
        _tcpConn->sendTcp( (BYTE*)&ecSend,  EC_SIZE + strlen(cCommand) );
    } catch (TcpSendException &e) {
        _logger->log(Logger::LOG_LEV_ERROR, "Hexapod sendCmd error: %s [%s:%d]", e.what().c_str(),__FILE__, __LINE__);
 	    pthread_mutex_unlock(&_sendMutex); 
		throw;
    }
    
    // Receive data from TCP connection.
    try {
        iRet = _tcpConn->receiveTcp(cRet, 1400, _timeout_ms);
    } catch (TcpReceiveException  &e) {
       _logger->log(Logger::LOG_LEV_ERROR, "Hexapod sendCmd error: %s [%s:%d]", e.what().c_str(),__FILE__, __LINE__);
 	   pthread_mutex_unlock(&_sendMutex); 
       throw;
    } catch (TcpTimeoutException &e) {
       _logger->log(Logger::LOG_LEV_ERROR, "Hexapod sendCmd error: %s [%s:%d]", e.what().c_str(),__FILE__, __LINE__);
 	   pthread_mutex_unlock(&_sendMutex); 
       throw;
    }
 	pthread_mutex_unlock(&_sendMutex); 
	
    if (cRet[iRet-1] == 6)	// acknowledge
    {
       _logger->log(Logger::LOG_LEV_DEBUG, "Hexapod received  %s [%s:%d]", cRet,__FILE__, __LINE__);
       return string((char*)cRet, iRet-1);
    } else {
       // Tira eccezione. recv ok, ma UMAC ha risposto male o il comando era errato
       throw HexapodException("Hexapod error: wrong command or reply error");
    }
}

//
// Open the brakes immediately (P750 indicates the status of the brakes: 0 = CLOSED, 1 = OPEN).
// The method is blocking: brakes are opened very quickly
//
void Hexapod::OpenBrake()
{
     if ( isFollowingError() ) { 
        ostringstream sss;
        sss << "Hexapod::OpenBrake  Hexapod is in following error. OpenBrake rejected";
        _logger->log(Logger::LOG_LEV_ERROR, "%s [%s:%d]", sss.str().c_str(),__FILE__, __LINE__);
        throw HexapodException(sss.str());
    }
    if ( isAmplifierFault() ) { 
        ostringstream sss;
        sss << "Hexapod::OpenBrake  Hexapod is in amplifier fault. OpenBrake rejected";
        _logger->log(Logger::LOG_LEV_ERROR, "%s [%s:%d]", sss.str().c_str(),__FILE__, __LINE__);
        throw HexapodException(sss.str());
    }
    if ( isLimitsPositiveHW() ) { 
        ostringstream sss;
        sss << "Hexapod::OpenBrake  Hexapod reached positive HW limits. OpenBrake rejected";
        _logger->log(Logger::LOG_LEV_ERROR, "%s [%s:%d]", sss.str().c_str(),__FILE__, __LINE__);
        throw HexapodException(sss.str());
    }
    if ( isLimitsNegativeHW() ) { 
        ostringstream sss;
        sss << "Hexapod::OpenBrake  Hexapod reached negative HW limits. OpenBrake rejected";
        _logger->log(Logger::LOG_LEV_ERROR, "%s [%s:%d]", sss.str().c_str(),__FILE__, __LINE__);
        throw HexapodException(sss.str());
    }
    // LB 08 jun 09 loop is closed in plc2, so this check is not useful here.
    //if ( isOpenLoop() ) { 
    //    ostringstream sss;
    //    sss << "Hexapod::OpenBrake  Hexapod is in open loop. OpenBrake rejected";
    //    _logger->log(Logger::LOG_LEV_ERROR, "%s [%s:%d]", sss.str().c_str(),__FILE__, __LINE__);
    //    throw HexapodException(sss.str());
    //}
    if (_simulator) { msleep(10); _sim_open = true; return; }

    string ret = sendCmd( "enable plc 2");
    while(!isBrakeOpen()) {
      msleep(10);
    }
}


//
// Close the brakes immediately. The brakes are fully engaged when P751 changes from 1 to 0
// 
void Hexapod::CloseBrake()
{
    if (_simulator) { msleep(10); _sim_open = false; return; }
    string ret = sendCmd("enable plc 5");
}

//
// Query brakes status.  P750=0: close, P750=1:open
// 
bool  Hexapod::isBrakeOpen()
{
    if (_simulator) { msleep(10); return (_sim_open == true); }
    string ret = sendCmd("P750");
    if (atoi(ret.c_str()) == 0) return false;
    return true;
}

//
// Return true if the system has been properly initialized
// From HP2 200080829 it seems that p751 goes 1 at the end of plc7
//
bool  Hexapod::isInitialized()
{
    string ret = sendCmd("P751");
    if (atoi(ret.c_str()) == 0) return false;
    return true;
}

//
// From 610a046d:
// -------  BEGIN -------
// The movement is completed when the variable M5187 changes from 0 to 1. 
// Please note that variable M5187 is normally 1, indicating that the previous
// commanded motion as been completed. When starting a new movement, its value
// changes from 1 to 0 during the transient. A minimum delay, of the order of 200
// millisecond, shall be awaited before starting polling the end-of-motion.
// ---------- END ---------
//
// if last move was commanded less than 200ms ago return true.
// else read M5187 and return true if 0, false if 1
//
// From a mail of P.Fumi 15 dec 08:
// "M5187 is the AND of Mx37" 
// so this method is equivalent to actsRunning().or()=1 TODO ask confirmation
//
bool  Hexapod::isMoving()
{
    if (_simulator) return false;
    _last_move_time.stop();
    if (_last_move_time.msec() < 250) msleep(250);
    string ret = sendCmd("M5187");
    if (atoi(ret.c_str()) == 0) return true;
    return false;
}



//
// Homing procedure
//
// 1) set after-homing position to 0 to prevent potentially unsafe commands
// 2) open brakes
// 3) start homing
//
// This method returns before the homing procedure is completed. Use isHomed() to poll
// for completion.
// 
// From 610a046d:
// -------  BEGIN -------
// Launch the homing procedure.
// This procedure is completed when variable P751 changes from 0 to 1. 
// At the end of such a procedure, the system is in closed loop and ready to perform any motion.
// A further vertex offset (displacement and rotation) can be specified in order to modify 
// the final position of the homing procedure. This can be done by assigning values to the following variables
// before issuing the homing itself.
// P1021 vertex offset along X in micron
// P1022 vertex offset along Y in micron
// P1023 vertex offset along Z in micron
// P1024 vertex rotation about X in arcsec (a)
// P1025 vertex rotation about Y in arcsec (b)
// P1026 vertex rotation about Z in arcsec (g)
// ---------- END ---------
// 
//
void Hexapod::Home()
{
    if (_simulator) { msleep(300); _sim_homed=true; return;}

    // set homing position to 0
    // Think twice before modifying the homing position: it could be outside the safe range
    // and it will not be filtered 
    //
    ostringstream sCmd;
    sCmd << " P1021=0.0"; //<< _init_position._x;
    sCmd << " P1022=0.0"; //<< _init_position._y;
    sCmd << " P1023=0.0"; //<< _init_position._z;
    sCmd << " P1024=0.0"; //<< _init_position._a;
    sCmd << " P1025=0.0"; //<< _init_position._b;
    sCmd << " P1026=0.0"; //<< _init_position._c;
    sendCmd(sCmd.str());
    OpenBrake();
    sendCmd("enable plc 7");
}

//
// isHomed return 1 if all the legs are homed
//
// TODO ask Gigi if this is the correct procedure
bool Hexapod::isHomed() {
    HexaBool actsHomed = ActsHomed();
    return (actsHomed.And());
}


//
// Allowed boundaries are defined in the hexapod config file.
// 
bool Hexapod::commandOutOfBound(HexaTuple p) {

    // allow commands to [0,0, +/- 12e3,0,0,0] iif
    // given from pos = 0+/-10um and tilt = 0+/-10"
    if (p == _park) {
        HexaTuple now = GetPos(); 
        if ( now.distance() <= 10.0 && now.tilt() <= 10.0) return false;
        return true;
    }
    // 
    return ( (p._x*p._x+p._y*p._y >_bound_radius*_bound_radius) ||
             (p._z > _bound_z || p._z < -_bound_z) || 
             (p.tilt() > _bound_tilt) );
    //return !(_bound_min <= p && _bound_max >= p);
}

//
// 
// Close brakes, open loop and reset.
// After DeInit, the system requires a new homing procedure.
//
// From 610a046d:
// -------  BEGIN -------
// The de-initializing procedure is performed to close the brakes, kill the servo-loop
// and restore some important safety variables.
// At the end of this procedure, the system can be re-initialized if needed, or switched off.
// ---------- END --------- 
// 
//
void Hexapod::DeInit()
{
    if (_simulator) { msleep(300); _sim_homed=true; _sim_open=false; return;}
    string ret = sendCmd("enable plc 6");
}


//
// Get hexapod absolute position
//
HexaTuple Hexapod::GetPos() throw (HexapodException) 
{
    if (!isInitialized()) {
        ostringstream sss;
        sss << "Hexapod::GetPos  Hexapod has not been homed yet. GetPos refused";
        _logger->log(Logger::LOG_LEV_ERROR, "%s [%s:%d]", sss.str().c_str(),__FILE__, __LINE__);
        throw HexapodException(sss.str());
    }
    if (_simulator) return _last_abs_position +
        HexaTuple( rand()/RAND_MAX, rand()/RAND_MAX, rand()/RAND_MAX, rand()/RAND_MAX, rand()/RAND_MAX, rand()/RAND_MAX);

    // TODO maybe everything in a single cmd ot the uMAC?
    float32 x = atof( sendCmd("Q651").c_str() );
    float32 y = atof( sendCmd("Q652").c_str() );
    float32 z = atof( sendCmd("Q653").c_str() );
    float32 a = atof( sendCmd("Q654").c_str() );
    float32 b = atof( sendCmd("Q655").c_str() );
    float32 c = atof( sendCmd("Q656").c_str() );

    _last_abs_position = HexaTuple(x,y,z,a,b,c);
    return _last_abs_position;
}


//
// Corresponds to SET_POS_ABS in 610a046d
//
// From 610a046d:
// -------  BEGIN -------
//Set the absolute position of the vertex of the mirror with respect to the main reference system.
//The movement is defined by assign the new X, Y, Z coordinates and the relevant rotations about these axes by means
//of the following variables:
//p1001 is the X-coordinate of the vertex;
//p1002 is the Y-coordinate of the vertex;
//p1003 is the Z-coordinate of the vertex;
//p1004 is the rotation a about the X axis;
//p1005 is the rotation b about the Y axis;
//p1006 is the rotation g about the Z axis
//The movement is completed when the variable M5187 changes from 0 to 1.
//Please note that variable M5187 is normally 1, indicating that the previous commanded motion as been completed.
//When starting a new movement, its value changes from 1 to 0 during the transient.
//A minimum delay, of the order of 200 millisecond, shall be awaited before starting polling the end-of-motion
//
//
//% put here speeds for displacements and rotations.
//% P1007 controls the linear speed in mm/s and p1008 controls the rotational speed in arcsec/sec
//p1007 = {floating point value}
//p1008 = {floating point value}
//% put here X, Y, Z coordinates of the vertex [micron]
//p1001 = {floating point value}
//p1002 = {floating point value}
//p1003 = {floating point value}
//% put here a, b, g rotations of the axes [arcsec]
//p1004 = {floating point value}
//p1005 = {floating point value}
//p1006 = {floating point value}
//% end of user definitions
//p1010=0
//b9r
// ---------- END ---------
void Hexapod::MoveTo(HexaTuple p) throw (HexapodException) 
{
    if (commandOutOfBound(p) == true) {
        ostringstream sss;
        sss << "Hexapod::MoveTo  Requested position is out of the allowed band. Command "<< p << " rejected";
        _logger->log(Logger::LOG_LEV_ERROR, "%s [%s:%d]", sss.str().c_str(),__FILE__, __LINE__);
        throw HexapodException(sss.str());
    }
    if ( !isHomed()) {
        ostringstream sss;
        sss << "Hexapod::MoveTo  Hexapod has not been homed yet. Command "<< p << " rejected";
        _logger->log(Logger::LOG_LEV_ERROR, "%s [%s:%d]", sss.str().c_str(),__FILE__, __LINE__);
        throw HexapodException(sss.str());
    }
    if ( !isBrakeOpen() ) {
        ostringstream sss;
        sss << "Hexapod::MoveTo  Brakes are closed. Command "<< p << " rejected";
        _logger->log(Logger::LOG_LEV_ERROR, "%s [%s:%d]", sss.str().c_str(),__FILE__, __LINE__);
        throw HexapodException(sss.str());
    }
    if ( isMoving() ) {
        ostringstream sss;
        sss << "Hexapod::MoveTo  Hexapod is moving. Command "<< p << " rejected";
        _logger->log(Logger::LOG_LEV_ERROR, "%s [%s:%d]", sss.str().c_str(),__FILE__, __LINE__);
        throw HexapodException(sss.str());
    }
    //if ( isRunning()==true ) {
    //    ostringstream sss;
    //    sss << "Hexapod::MoveTo  Hexapod is running. Command "<< p << " rejected";
    //    _logger->log(Logger::LOG_LEV_ERROR, "%s [%s:%d]", sss.str().c_str(),__FILE__, __LINE__);
    //    throw HexapodException(sss.str());
    //}
    if ( !isInitialized()) { 
        ostringstream sss;
        sss << "Hexapod::MoveTo  Hexapod has not been initialized yet . Command "<< p << " rejected";
        _logger->log(Logger::LOG_LEV_ERROR, "%s [%s:%d]", sss.str().c_str(),__FILE__, __LINE__);
        throw HexapodException(sss.str());
    }
    if ( isFollowingError() ) { 
        ostringstream sss;
        sss << "Hexapod::MoveTo  Hexapod is in following error. Command "<< p << " rejected";
        _logger->log(Logger::LOG_LEV_ERROR, "%s [%s:%d]", sss.str().c_str(),__FILE__, __LINE__);
        throw HexapodException(sss.str());
    }
    if ( isAmplifierFault() ) { 
        ostringstream sss;
        sss << "Hexapod::MoveTo  Hexapod is in amplifier fault. Command "<< p << " rejected";
        _logger->log(Logger::LOG_LEV_ERROR, "%s [%s:%d]", sss.str().c_str(),__FILE__, __LINE__);
        throw HexapodException(sss.str());
    }
    if ( isLimitsPositiveHW() ) { 
        ostringstream sss;
        sss << "Hexapod::MoveTo  Hexapod reached positive HW limits. Command "<< p << " rejected";
        _logger->log(Logger::LOG_LEV_ERROR, "%s [%s:%d]", sss.str().c_str(),__FILE__, __LINE__);
        throw HexapodException(sss.str());
    }
    if ( isLimitsNegativeHW() ) { 
        ostringstream sss;
        sss << "Hexapod::MoveTo  Hexapod reached negative HW limits. Command "<< p << " rejected";
        _logger->log(Logger::LOG_LEV_ERROR, "%s [%s:%d]", sss.str().c_str(),__FILE__, __LINE__);
        throw HexapodException(sss.str());
    }
    if ( isOpenLoop() ) { 
        ostringstream sss;
        sss << "Hexapod::MoveTo  Hexapod is in open loop. Command "<< p << " rejected";
        _logger->log(Logger::LOG_LEV_ERROR, "%s [%s:%d]", sss.str().c_str(),__FILE__, __LINE__);
        throw HexapodException(sss.str());
    }

    
    if (_simulator) {
        _last_abs_position = p;
        return;
    }
    ostringstream sCmd;
    sCmd << " P1007=" << _linear_speed;
    sCmd << " P1008=" << _rot_speed;
    sCmd << " P1001=" << p._x;
    sCmd << " P1002=" << p._y;
    sCmd << " P1003=" << p._z;
    sCmd << " P1004=" << p._a;
    sCmd << " P1005=" << p._b;
    sCmd << " P1006=" << p._c;
    sCmd << " P1010=0";  //  toggle to tell the umac that this is abs move.

    sendCmd(sCmd.str());
    msleep(10);
    sendCmd("b9r");
    _last_move_time.start();

    msleep(10);
}



void Hexapod::MoveBy(HexaTuple p)
{
    MoveTo(GetPos() + p);
}


//
//
//
//void Hexapod::SetCurPosAsHome()
//{
//}

//
// theta and phi in [arcsec] radius in [m] 
//
//
#include <cmath>
void Hexapod::MoveOnSphere(float radius, float theta, float phi)
{
    //_sphere_radius = radius;
	//float q  = _sphere_radius * 1e6;

    // radius is [m], q is [um]
	float q  = radius * 1e6;

    // eTheta is [rad], theta is [arcsec]
    float eTheta = theta / 3600. * M_PI / 180.;
    float ePhi   = phi / 3600. * M_PI / 180.;

    HexaTuple currPos = GetPos();
    float xv = currPos._x;
    float yv = currPos._y;
    float zv = currPos._z;
    float a  = currPos._a / 3600. * M_PI / 180;
    float b  = currPos._b / 3600. * M_PI / 180;
    float c  = currPos._c / 3600. * M_PI / 180;

	double xv1 = xv 
			+ q * ( ( -1 + cos( eTheta ) ) * ( cos( a ) * cos( c ) * sin( b )
			+ sin( a ) * sin( c ) ) 
			+ cos( b ) * cos( c ) * cos( ePhi ) * sin( eTheta ) 
			+ ( cos( c ) * sin( a ) * sin( b ) 
			- cos( a ) * sin( c ) ) * sin( eTheta ) * sin( ePhi ) );
	
	double yv1 = yv 
			+ q * cos( c ) * ( sin( a ) - cos( eTheta ) * sin( a ) 
			+ cos( a ) * sin( eTheta ) * sin( ePhi ) ) 
			+ q * sin( c ) * ( cos( a ) * ( -1 + cos( eTheta )) * sin( b ) 
			+ sin( eTheta ) * ( cos( b ) * cos( ePhi ) 
			+ sin( a )*sin( b ) * sin( ePhi ) ) );
	
	double zv1 = zv + q 
			* ( cos( a ) * cos( b ) 
			* ( -1 + cos( eTheta ) ) + sin( eTheta ) 
			* ( - ( cos( ePhi ) * sin( b ) ) + cos( b ) 
			* sin( a ) * sin( ePhi ) ) );
	
	double r1_32 = 
			- ( ( -1 + cos( eTheta ) ) * cos( ePhi ) 
			* sin( b ) * sin( ePhi ) ) 
			+ cos( b ) * ( - ( cos( a ) 
			* sin( eTheta ) * sin( ePhi ) ) 
			+ sin( a ) * ( ( cos( ePhi ) * cos( ePhi ) ) 
			+ cos( eTheta ) * ( sin( ePhi ) * sin( ePhi ) ) ) );
			
	double r1_33 =
			cos( a ) * cos( b ) * cos( eTheta ) 
			+ sin( eTheta ) * ( - ( cos( ePhi ) * sin( b ) ) 
			+ cos( b ) * sin( a ) * sin( ePhi ) );
			
	double r1_31 = 
			- ( cos( a ) * cos( b ) * cos( ePhi ) * sin( eTheta ) ) 
			- 2 * cos( b ) * cos( ePhi )
			* sin( a ) * ( sin( eTheta/2.0 ) * sin( eTheta/2.0 ) ) * sin( ePhi ) 
			- sin( b ) * ( cos( eTheta ) * ( cos( ePhi ) * cos( ePhi ) ) 
			+ ( sin( ePhi ) * sin( ePhi ) ) );
	/*		
	double r1_22 =
			- 2 * cos( b ) * cos( ePhi )
			* sin( c ) * ( sin( eTheta / 2.0f ) * sin( eTheta / 2.0f ) ) * sin( ePhi ) 
			+ ( cos( c ) * sin( a ) 
			- cos( a ) * sin( b ) * sin( c ) ) * sin( eTheta ) * sin( ePhi ) 
			+ ( cos( a ) * cos( c ) 
			+ sin( a ) * sin( b ) * sin( c ) ) * ( ( cos( ePhi ) * cos( ePhi ) ) 
			+ cos( eTheta ) * ( sin( ePhi ) * sin( ePhi ) ) );	
*/
     double r1_21 = 
            cos(ePhi) * ( (cos(c)*sin(a) - cos(a)*sin(b)*sin(c) ) * sin(eTheta)
            + (cos(eTheta)-1) * (cos(a)*cos(c) + sin(a)*sin(b)*sin(c) ) * sin(ePhi) )
            + cos(b)*sin(c)*( cos(eTheta) * pow(cos(ePhi),2) + pow(sin(ePhi),2));
    
	double r1_11 =
			cos( ePhi ) * ( - ( ( cos( a ) * cos( c ) * sin( b ) 
			+ sin( a ) * sin( c ) ) * sin( eTheta )) 
			+ ( -1 + cos( eTheta ) ) * ( cos( c ) * sin( a ) * sin( b ) 
			- cos( a ) * sin( c ) ) * sin( ePhi) ) 
			+ cos( b ) * cos( c ) * ( cos( eTheta ) * ( cos( ePhi ) * cos( ePhi ) ) 
			+ ( sin( ePhi ) * sin( ePhi ) ) );
			
	float a1 = atan( r1_32 / r1_33 ) * 180/M_PI * 3600;
	
	float b1 = -asin( r1_31 ) * 180/M_PI * 3600;
	
	float c1 = atan( r1_21 / r1_11 ) * 180/M_PI * 3600;
		
	return MoveTo( HexaTuple(xv1, yv1, zv1, a1, b1, c1) );
}

HexaBool Hexapod::ActsHomed()
{
    if (_simulator) {
        if (_sim_homed) return HexaBool(1.,1.,1.,1.,1.,1.);
        else return HexaBool(0.,0.,0.,0.,0.,0.);
    }
    int x = atoi( sendCmd("M145").c_str() );
    int y = atoi( sendCmd("M245").c_str() );
    int z = atoi( sendCmd("M345").c_str() );
    int a = atoi( sendCmd("M445").c_str() );
    int b = atoi( sendCmd("M545").c_str() );
    int c = atoi( sendCmd("M645").c_str() );

    return  HexaBool(x,y,z,a,b,c);
}


//
// ActsLimit is 1 for the legs having reached the HW OR the SW limit.
//
HexaBool Hexapod::ActsLimit()
{
    if (_simulator) return HexaBool(0.,0.,0.,0.,0.,0.);

    int x = atoi( sendCmd("M130").c_str() );
    int y = atoi( sendCmd("M230").c_str() );
    int z = atoi( sendCmd("M330").c_str() );
    int a = atoi( sendCmd("M430").c_str() );
    int b = atoi( sendCmd("M530").c_str() );
    int c = atoi( sendCmd("M630").c_str() );

    return  HexaBool(x,y,z,a,b,c);
}

//
// ActsRunning is 1 for the legs that are in motion.
// Motion starts and ends simultaneously for all the legs
// so the possible return values should be only [0,0,0,0,0,0] or
// [1,1,1,1,1,1] (Gigi said) 
//
HexaBool Hexapod::ActsRunning()
{
    if (_simulator) return HexaBool(0.,0.,0.,0.,0.,0.);

    int x = atoi( sendCmd("M137").c_str() );
    int y = atoi( sendCmd("M237").c_str() );
    int z = atoi( sendCmd("M337").c_str() );
    int a = atoi( sendCmd("M437").c_str() );
    int b = atoi( sendCmd("M537").c_str() );
    int c = atoi( sendCmd("M637").c_str() );

    return  HexaBool(x,y,z,a,b,c);
}

//
// isRunning return 1 if at least one leg is moving
// This is equivalent to isMoving
//
//bool Hexapod::isRunning() {
//    HexaBool actsRunning = ActsRunning();
//    return (actsRunning.Or() );
//}


//
// ActsOpenLoop is 1 for the legs that are in open loop
// 
HexaBool Hexapod::ActsOpenLoop()
{
    if (_simulator) return HexaBool(0.,0.,0.,0.,0.,0.);

    int x = atoi( sendCmd("M138").c_str() );
    int y = atoi( sendCmd("M238").c_str() );
    int z = atoi( sendCmd("M338").c_str() );
    int a = atoi( sendCmd("M438").c_str() );
    int b = atoi( sendCmd("M538").c_str() );
    int c = atoi( sendCmd("M638").c_str() );

    return  HexaBool(x,y,z,a,b,c);
}

//
// isOpenLoop return 1 if 1+ legs are in open loop
//
bool Hexapod::isOpenLoop() {
    HexaBool ss   = ActsOpenLoop();
    return (ss.Or());
}


//
// ActsInPosition return 1 for the legs that have reached the required position.
// Beware of timing: when a new position is commanded, it may take a while before ActsInPosition 
// become 0 and the hexapod start moving. This may lead to believe that the hexapod has already concluded the command
// while, in fact, it has still to start the motion.
//
HexaBool Hexapod::ActsInPosition()
{
    if (_simulator) return HexaBool(1.,1.,1.,1.,1.,1.);

    int x = atoi( sendCmd("M140").c_str() );
    int y = atoi( sendCmd("M240").c_str() );
    int z = atoi( sendCmd("M340").c_str() );
    int a = atoi( sendCmd("M440").c_str() );
    int b = atoi( sendCmd("M540").c_str() );
    int c = atoi( sendCmd("M640").c_str() );

    return  HexaBool(x,y,z,a,b,c);
}

//
// ActsFollowingError is 1 for the legs that are in following error
// 
HexaBool Hexapod::ActsFollowingError()
{
    if (_simulator) return HexaBool(1.,1.,0.,0.,1.,1.);

    int x = atoi( sendCmd("M142").c_str() );
    int y = atoi( sendCmd("M242").c_str() );
    int z = atoi( sendCmd("M342").c_str() );
    int a = atoi( sendCmd("M442").c_str() );
    int b = atoi( sendCmd("M542").c_str() );
    int c = atoi( sendCmd("M642").c_str() );

    return  HexaBool(x,y,z,a,b,c);
}

//
// isFollowingError return 1 if 1+ legs signal a following error
//
bool Hexapod::isFollowingError() {
    HexaBool ss   = ActsFollowingError();
    return (ss.Or());
}

//
// ActsAmplifierFault is 1 for the legs that are in amplifier fault
// 
HexaBool Hexapod::ActsAmplifierFault()
{
    if (_simulator) return HexaBool(0.,0.,1.,1.,0.,1.);

    int x = atoi( sendCmd("M143").c_str() );
    int y = atoi( sendCmd("M243").c_str() );
    int z = atoi( sendCmd("M343").c_str() );
    int a = atoi( sendCmd("M443").c_str() );
    int b = atoi( sendCmd("M543").c_str() );
    int c = atoi( sendCmd("M643").c_str() );

    return  HexaBool(x,y,z,a,b,c);
}

//
// isAmplifierFault return 1 if 1+ legs signal an amplifier fault
//
bool Hexapod::isAmplifierFault() {
    HexaBool ss   = ActsAmplifierFault();
    return (ss.Or());
}

//
// TODO add description and relation with ActLimits
//
HexaBool Hexapod::LimitsPositiveHW()
{
    if (_simulator) return HexaBool(0.,1.,0.,1.,0.,1.);

    int x = atoi( sendCmd("M121").c_str() );
    int y = atoi( sendCmd("M221").c_str() );
    int z = atoi( sendCmd("M321").c_str() );
    int a = atoi( sendCmd("M421").c_str() );
    int b = atoi( sendCmd("M521").c_str() );
    int c = atoi( sendCmd("M621").c_str() );

    return  HexaBool(x,y,z,a,b,c);
}

//
// isLimitsPositiveHW return 1 if 1+ legs reached a positive HW limit
//
bool Hexapod::isLimitsPositiveHW() {
    HexaBool ss   = LimitsPositiveHW();
    return (ss.Or());
}

//
// TODO add description and relation with ActLimits
//
HexaBool Hexapod::LimitsNegativeHW()
{
    if (_simulator) return HexaBool(1.,0.,1.,0.,1.,0.);

    int x = atoi( sendCmd("M122").c_str() );
    int y = atoi( sendCmd("M222").c_str() );
    int z = atoi( sendCmd("M322").c_str() );
    int a = atoi( sendCmd("M422").c_str() );
    int b = atoi( sendCmd("M522").c_str() );
    int c = atoi( sendCmd("M622").c_str() );

    return  HexaBool(x,y,z,a,b,c);
}

//
// isLimitsNegativeHW return 1 if 1+ legs reached a negative HW limit
//
bool Hexapod::isLimitsNegativeHW() {
    HexaBool ss   = LimitsNegativeHW();
    return (ss.Or());
}


//
// Get hexapod status
//
HexaStatus Hexapod::GetStatus() throw (HexapodException) 
{

	HexaBool homed        = ActsHomed();
    HexaBool limit        = ActsLimit();
    HexaBool running      = ActsRunning();
    HexaBool openloop     = ActsOpenLoop();
    HexaBool inposition   = ActsInPosition();
    HexaBool followingerr = ActsFollowingError();
    HexaBool ampfault     = ActsAmplifierFault();
    HexaBool limitposhw   = LimitsPositiveHW();
    HexaBool limitneghw   = LimitsNegativeHW();
	bool     brakesopen   = isBrakeOpen(); 
	bool     initialized  = isInitialized(); 
	bool     ismoving     = isMoving(); 
    return HexaStatus(homed, limit, running, openloop, inposition, followingerr, ampfault, limitposhw, limitneghw, brakesopen, initialized, ismoving);
}


//
