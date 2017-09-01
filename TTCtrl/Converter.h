#ifndef CONVERTER_H
#define CONVERTER_H

#include "TTCtrl.h"
#include "TipTilt.h"
#include "Waves.h"
#include <cmath>
using namespace std;

//@Class: Converter
//
// Defines a converter from TipTilt to Waves.
// (Note: this version supports only 3 actuators !!!)
//
// Utility class to convert TipTilt parameters from
// high-level to low-level and vice versa.
// Also stores setting ranges read from Configuration file.
//
//@
class Converter {
	
	public:
		Converter(TTCtrl* myCtrl, Config_File cfg);
		
		double getRotationAngle() { return _rot_ang; };
		
		// Change the axys rotation angle defined in config file
		void setRotationAngle(double rotAngleDegree) { _rot_ang = rotAngleDegree; }
		
		void convert(TipTilt *tiptilt, Waves *waves);

		void checkAndFix(TipTilt *tiptilt);
		void checkAndFix(Waves *waves);			

	private:
	
		TTCtrl* _myCtrl;

		int _n_act;

		//--- Config file interesting values are copied here ---//
		// This allow to check config file completenessConverter 

		// Conversion parameters
		double _zV_to_xV;
		double _rot_ang;

		// Valid ranges of the actuators (low-level)
		double _minFreq;	// Correspond to high-level
		double _maxFreq;
		double _minVolt;	// Are translated on high-level ranges using
		double _maxVolt;	// MinOffset() and MaxOffset()

		double _hl_max_amp_zero;

      // Phases in degrees
      double _defaultPhase1;
      double _defaultPhase2;
      double _defaultPhase3;

		// Low-level computed bounds
		double LL_MaxAmp(double off_z1, double off_z2, double off_z3);

		// High-level computed bounds
		double HL_MaxAmp(double off_x=0, double off_y=0);

		// Compute a deltaX shift along the direction 
		// center-actuator obtained from an actuator 
		// deltaZ movement.
		double getDX(double dz);

		// Compute the deltaZ actuator movement needed
		// to obtain a deltaX shift along the direction 
		// center-actuator.
		double getDZ(double dx); 
		
		// Compute the x offset of the light reflected 
		// when actuator is set to z-voltage. (x is along
		// the direction center-actuator)
		double getX(double z); 
		
		// Compute the actuator z-voltage needed to obtain
		// an offset x of the light reflected (x is along
		// the direction center-actuator)
		double getZ(double x);

		// Compute the three Z actuators movements needed 
		// to obtain a transaltion x,y of the mirror rotation 
		// center
		void offset_XY_to_3Z(const double x, const double y, 
							 double &z1, double &z2, double &z3);

};

#endif
