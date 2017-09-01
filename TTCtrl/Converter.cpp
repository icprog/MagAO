#include "Converter.h"
#include "Logger.h"
#include "AOStates.h"
using namespace Arcetri;

#include <cmath>
using namespace std;


#define PI (3.141592653589793116)
#define DEG_PER_RAD (57.29577951308232286)

//--------------------------- PUBLIC METHODS ---------------------------//
//----------------------------------------------------------------------//


Converter::Converter(TTCtrl* myCtrl, Config_File cfg) {

    
    _myCtrl = myCtrl;
    
    try {
		_n_act = cfg["ACT_NUM"];
	} 
	catch(Config_File_Exception& e) {
		_n_act = 3;
	}

	//Load conversion parameters
	try {
		_zV_to_xV = cfg["ZV_TO_XV"]; 
		_zV_to_xV = _zV_to_xV * (-1);	// Correct the sign: when actuator increase voltage,
										// the corresponding translation along its axis 
										// is negative.
		_rot_ang = cfg["DEFAULT_ROT_ANG"];
	}
	catch(Config_File_Exception e) {
		Logger::get()->log(Logger::LOG_LEV_FATAL, "%s", e.what().c_str());
		throw Config_File_Exception("Incomplete config file: please specify all conversion parameters !"); 
	}

	//Load min/max default values
	try { 
		_minFreq = cfg["MIN_FREQ"]; 	
		_maxFreq = cfg["MAX_FREQ"]; 

		_minVolt = cfg["MIN_VOLT"]; 	
		_maxVolt = cfg["MAX_VOLT"]; 
	} 
	catch(Config_File_Exception e) {
		Logger::get()->log(Logger::LOG_LEV_FATAL, "%s", e.what().c_str());
		throw Config_File_Exception("Incomplete config file: please specify all range limits !"); 
	}

	//Load default settings
	try { 
		_defaultPhase1 = cfg["DEFAULT_LL_PHASE_1"];
		_defaultPhase2 = cfg["DEFAULT_LL_PHASE_2"];
		_defaultPhase3 = cfg["DEFAULT_LL_PHASE_3"];
	} 
	catch(Config_File_Exception e) {
		Logger::get()->log(Logger::LOG_LEV_FATAL, "%s", e.what().c_str());
		throw Config_File_Exception("Incomplete config file: please specify default phase parameters !"); 
	}

	// Compute the max high-level amplitude when rotation center=(0,0),
	// because it depends only on low-level bounds and mirror phisical parameters
	double act_max_amp = (_maxVolt-_minVolt) / 2;
	// Convert it in x movement: take abs because 
	// a positive dZ corresponds to a negative dX, but
	// radius must be always positive
	_hl_max_amp_zero = abs(getDX(act_max_amp));
}


void Converter::convert(TipTilt *tiptilt, Waves *waves) {

	Logger::get()->log(Logger::LOG_LEV_INFO, "----------------------------- HI->LOW LEV CONVERSION --------------------------------");

	// Get TipTilt parameters
	double freq = tiptilt->Freq();
	double amp = tiptilt->Amp();
	double offsetx = tiptilt->Offset_x();
	double offsety = tiptilt->Offset_y();

	double waves_freqs[_n_act];
	double waves_phases[_n_act];
	double waves_amps[_n_act];
	double waves_offsets[_n_act];	

	// Frequence
	for(int i=0; i<_n_act; i++) { 
		waves_freqs[i]= freq;	
	}
	
	//Default phases (for 3 actuators) in degree
	waves_phases[0] = _defaultPhase1;
	waves_phases[1] = _defaultPhase2;
	waves_phases[2] = _defaultPhase3;

	//Amplitude
	for(int i=0; i<_n_act; i++) { 
		waves_amps[i] = abs(getDZ(amp));
	}
	
	//Offsets 
   	double offset1, offset2, offset3;
	offset_XY_to_3Z(offsetx, offsety, offset1, offset2, offset3);

	waves_offsets[0] = offset1;
	waves_offsets[1] = offset2;
	waves_offsets[2] = offset3;

	//--- Set this object ---//
	waves->setFreqs(waves_freqs);
	waves->setAmps(waves_amps);
	waves->setPhases(waves_phases);
	waves->setOffsets(waves_offsets); 

	// Do some stupid print...
	Logger::get()->log(Logger::LOG_LEV_INFO, "Computing Waves from TipTilt done:");
	for(int i=0; i<_n_act; i++) {
		Logger::get()->log(Logger::LOG_LEV_INFO, "<Low-Level> Freq = %f | Amp = %f | Offset = %f | Phase = %f", waves_freqs[i], waves_amps[i], waves_offsets[i], waves_phases[i]);
	}
	Logger::get()->log(Logger::LOG_LEV_INFO, "Max low-level amplitude allowed: %f", LL_MaxAmp(offset1,offset2,offset3));
	Logger::get()->log(Logger::LOG_LEV_INFO, "Max high-level amplitude allowed: %f", HL_MaxAmp(offsetx,offsety));
}


void Converter::checkAndFix(TipTilt *tiptilt) {

	Logger::get()->log(Logger::LOG_LEV_INFO, "------------------------------- HI-LEV CHECK&FIX ----------------------------------");

	double freq = tiptilt->Freq();
	double amp = tiptilt->Amp();
	double offsetx = tiptilt->Offset_x();
	double offsety = tiptilt->Offset_y();

	Logger::get()->log(Logger::LOG_LEV_INFO, "<High-Level> Freq = %f | Amp = %f | Off_x = %f | Off_y = %f", freq, amp, offsetx, offsety);
	
	// Frequence
	if(freq>_maxFreq) {
		freq = _maxFreq;;
	}
	if(freq<_minFreq) {
		freq = _minFreq;
	}
	
	// If frequency is zero (attached to CCD39) check that CCD39 is OPERATING
	if(freq == 0) {
		if(!(_myCtrl->isCcd39CtrlReady()) || !(_myCtrl->isCcd39Operating())) {
			freq = _myCtrl->ConfigDictionary()["DEFAULT_FREQ"];
			amp = 0;
			Logger::get()->log(Logger::LOG_LEV_WARNING, "Frequency is ZERO but CCD39 isn't operating (using default frequency)");
		}
		else {
			Logger::get()->log(Logger::LOG_LEV_INFO, "CCD39 is operating: attaching the CCD frequency");
		}
	}
	
	// Offset: if invalid, set it in circle border
	double radius = sqrt(pow(offsetx,2) + pow(offsety,2));
	if(radius > _hl_max_amp_zero) {
		double ang = atan2(offsety,offsetx);
		offsetx = _hl_max_amp_zero*cos(ang);
		offsety = _hl_max_amp_zero*sin(ang);
	}

	// Amplitude 
	double ampExcess = amp - HL_MaxAmp(offsetx, offsety);
	if(ampExcess > 0) {
		amp = amp - ampExcess;
	}

        // Amplitude * frequency
        amp = _myCtrl->checkAmp( freq, amp);

	Logger::get()->log(Logger::LOG_LEV_INFO, "<Fixed High-Level> Freq = %f | Amp = %f | Off_x = %f | Off_y = %f", freq, amp, offsetx, offsety);

	//Set fixed parames
	tiptilt->setFreq(freq);
	tiptilt->setAmp(amp);
	tiptilt->setOffset_x(offsetx);
	tiptilt->setOffset_y(offsety);
}


void Converter::checkAndFix(Waves *waves) {

	Logger::get()->log(Logger::LOG_LEV_INFO, "------------------------------- LOW-LEV CHECK&FIX ----------------------------------");
	
	double freqs[_n_act], amps[_n_act], offsets[_n_act], phases[_n_act];

	waves->Freqs(freqs);
	waves->Amps(amps);
	waves->Offsets(offsets);
	waves->Phases(phases);

	for(int i=0; i<_n_act; i++) {

		Logger::get()->log(Logger::LOG_LEV_INFO, "<Low-Level> Freq = %f | Amp = %f | Offset = %f | Phase = %f", freqs[i], amps[i], offsets[i], phases[i]);

      // Zero frequence means synch with ccd39
		if(freqs[i] == 0) {
			if(!(_myCtrl->isCcd39CtrlReady()) || !(_myCtrl->isCcd39Operating())) {
				freqs[i] = _myCtrl->ConfigDictionary()["DEFAULT_FREQ"];
				Logger::get()->log(Logger::LOG_LEV_WARNING, "Frequency is ZERO but CCD39 isn't operating (using default frequency)");
			}
			else {
				Logger::get()->log(Logger::LOG_LEV_INFO, "CCD39 is operating: attaching the CCD frequency");
			}
		}
      else {

		   // Otherwise, check the frequency safety range
		   if(freqs[i]>_maxFreq) {
			   freqs[i] = _maxFreq;;
		   }
		   else if(freqs[i]<_minFreq) {
			   freqs[i] = _minFreq;
		   }
      }
		
		//Offset
		if(offsets[i]>_maxVolt) {
			offsets[i] = _maxVolt;
		}
		else if(offsets[i]<_minVolt) {
			offsets[i] = _minVolt;
		}

		// Amplitude (not negative)
		if(amps[i] < 0) {
			amps[i] = 0;
		}

                // Amplitude * frequency
                amps[i] = _myCtrl->checkAmp( freqs[i], amps[i]);
		
	}

	// Compute max amplitude allowed by fixed offsets
	for(int i=0; i<_n_act; i++) {
	    double maxAmpl = min(_maxVolt-offsets[i], offsets[i]-_minVolt);
		if(amps[i] > maxAmpl) {
			amps[i] = maxAmpl;
		}
	}
	
	for(int i=0; i<_n_act; i++) {
		Logger::get()->log(Logger::LOG_LEV_INFO, "<Fixed Low-Level> Freq: %f - Amp: %f - Offset: %f - Phase: %f", freqs[i], amps[i], offsets[i], phases[i]);
	}

	//Set fixed parames
	waves->setFreqs(freqs);
	waves->setAmps(amps);
	waves->setOffsets(offsets);
	waves->setPhases(phases);
}



//-------------------------- PRIVATE METHODS ---------------------------//
//----------------------------------------------------------------------//

double Converter::LL_MaxAmp(double off_z1, double off_z2, double off_z3) {
	
	double a1_max = min(_maxVolt-off_z1, off_z1-_minVolt);
	double a2_max = min(_maxVolt-off_z2, off_z2-_minVolt);
	double a3_max = min(_maxVolt-off_z3, off_z3-_minVolt);
	
	double maxAmp = min(a1_max, min (a2_max, a3_max));
	if(maxAmp > 0) {
		return maxAmp;
	}
	// This mean that one or more actuators offsets are out
	// of bounds !!!
	return 0;
}

double Converter::HL_MaxAmp(double off_x, double off_y) { 

	// Rotation center is not offset (actuators are an their zero): 
	// max amplitude have been computed once in the constructor
	if (off_x==0 && off_y==0) {
		return _hl_max_amp_zero;
	}
	// Rotation center have an offset
	else {
		// R is the max Hi-Level amplitude when offset is zero
		double R = _hl_max_amp_zero;
		// Compute de distance from P(off_x,aff_y) to center C = P(0,0)
		double d = sqrt(pow(off_x,2) + pow(off_y,2));
		// Max amplitude from offset center is simply R-d
		double r = R-d;
		return r;
	}
} 	
		
double Converter::getDX(double dz) {
	return (dz * _zV_to_xV);
}

double Converter::getDZ(double dx) { 
	return dx / _zV_to_xV; 
}

double Converter::getX(double z) { 
	double zCenter = (_maxVolt - _minVolt) / 2;
	return getDX(z - zCenter);
} 
		
double Converter::getZ(double x) { 
	double zCenter = (_maxVolt - _minVolt) / 2;
	return getDZ(x) + zCenter; 
}


void Converter::offset_XY_to_3Z(const double x, const double y, 
							   	double &z1, double &z2, double &z3) {

        // Since we only have to axis, use the first two
        z1 = x;
        z2 = y;
        z3 = 0;
        return;
        
	
	// Consider ax1, ax2, ax3 as three 120° axis on (x,y) plane, with:
	//  - ax1 rotated of a_1 = _rot_ang
	//	- ax2 rotated of a_2 = _rot_ang + 120
	//	- ax3 rotated of a_3 = _rot_ang + 240
	// degree respect to y axis, where _rot_angle is in hourly direction.

	// Rotates z1, z2, z3 to fix z1 on imaginary y axis
	double a_1 = _rot_ang / DEG_PER_RAD;
	double a_2 = a_1 + PI*2.0/3.0;
	double a_3 = a_2 + PI*2.0/3.0;

	// Compute P(x,y) point on (r,b) coordinates
	double r = sqrt(pow(x,2)+pow(y,2)); // Radius of P(x,y) from center 

	if(r!=0) {
		double b = acos(y/r);    // Angle (in radiants) beetwen  P(x,y) and z1: acos is periodic,
							     // that is acos(y/r) maps to 0°-180°
		if(x<0) {			     // Increments angle if needed: if x<0 =>  180°<b<360° 
			b = PI*2.0-b;
		}			
		Logger::get()->log(Logger::LOG_LEV_DEBUG, "Offset point (r,ang_b) = (%f,%f)", r, b*DEG_PER_RAD);
		
		// Project P(r,b) on ax1, ax2, ax2
		double x1, x2, x3;
		x1 = r*cos(a_1-b);
		x2 = r*cos(a_2-b);
		x3 = r*cos(a_3-b);
	
		// Translate z1, z2, z3 on "vertical" actuators movements 
		z1 = getZ(x1);
		z2 = getZ(x2);
		z3 = getZ(x3);
	}
	else {
		z1 = z2 = z3 = getZ(0);
	}
}



