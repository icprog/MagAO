#include "Waves.h"
#include "Logger.h"
using namespace Arcetri;

extern "C" {
	#include "commlib.h"
	#include "base/thrdlib.h"
        #include "tt_matlab.h"
}


// Parameters for BCU communication	
int TT_Dsp 	= 0xFA;
int TT_BaseAddress  = 0x2000;

int TT_enableOutputsAddress = 0x3FFF;
int TT_enableOutputsValue   = 0x00010000;	

Waves::Waves(TTCtrl *ctrl, int direction) throw(WavesException) {

	_n_act = ctrl->getActNum();
	_myCtrl = ctrl;
	_direction = direction;
	
	_comSender = NULL;
	
	// Only a CUR object will send the command to the BCU
	if(_direction == REQ_VAR) {
		// Get the command sender timeout
		try {
			_timeout_ms = _myCtrl->ConfigDictionary()["TIMEOUT_MS"];
		} 
		catch(Config_File_Exception& e) {
			_timeout_ms = 500;
			Logger::get()->log(Logger::LOG_LEV_WARNING, "Using default timeout (500 ms)");
		}
		
		_comSender = new CommandSender();
	}
	Logger::get()->log(Logger::LOG_LEV_DEBUG, "Waves created");	
} 

Waves::~Waves() {
	delete _comSender;
}

void Waves::setupVars() throw (AOVarException) {
    try {
	   _freqs = RTDBvar(_myCtrl->MyFullName(), "LOWFREQ", _direction, REAL_VARIABLE, _n_act);
	   _amps = RTDBvar(_myCtrl->MyFullName(), "LOWAMP", _direction, REAL_VARIABLE, _n_act);
	   _offsets = RTDBvar(_myCtrl->MyFullName(), "LOWOFF", _direction, REAL_VARIABLE, _n_act);
	   _phases = RTDBvar(_myCtrl->MyFullName(), "LOWPHASE", _direction, REAL_VARIABLE, _n_act);
    }
    catch (AOVarException &e) {
 		Logger::get()->log(Logger::LOG_LEV_FATAL, e.what().c_str());	
        throw AOException("Error creating RTDB variables");
    }


	Logger::get()->log(Logger::LOG_LEV_DEBUG, "Waves vars setup completed");
}

void Waves::notifyVars() {
	if(_direction == REQ_VAR) 
	{
		_myCtrl->Notify(_freqs, _myCtrl->WavesReqChanged);
		_myCtrl->Notify(_amps, _myCtrl->WavesReqChanged);
		_myCtrl->Notify(_offsets, _myCtrl->WavesReqChanged);
		_myCtrl->Notify(_phases, _myCtrl->WavesReqChanged);	
	}
	
	Logger::get()->log(Logger::LOG_LEV_DEBUG, "Waves vars notify request completed");
}

void Waves::initVars() throw (Config_File_Exception) {
	Config_File cfg = _myCtrl->ConfigDictionary();

	//Load req default values and save them to RTDB
	try { 
		double defFreqs[_n_act];
		double defAmps[_n_act];
		double defOffs[_n_act];
		double defPhas[_n_act];
		
		for(int i=0; i<_n_act; i++) {
			defFreqs[i] = cfg["DEFAULT_LL_FREQ"];
		}
		for(int i=0; i<_n_act; i++) {
			defAmps[i] = cfg["DEFAULT_LL_AMP"];
		}
		for(int i=0; i<_n_act; i++) {
			defOffs[i] = cfg["DEFAULT_LL_OFFSET"];
		}
		defPhas[0] = cfg["DEFAULT_LL_PHASE_1"];
		defPhas[1] = cfg["DEFAULT_LL_PHASE_2"];
		defPhas[2] = cfg["DEFAULT_LL_PHASE_3"];
		
		setFreqs(defFreqs); 
		setAmps(defAmps); 
		setOffsets(defOffs);
		setPhases(defPhas); 
	} 
	catch(Config_File_Exception e) { 
		Logger::get()->log(Logger::LOG_LEV_DEBUG, "%s", e.what().c_str());
		throw Config_File_Exception("Incomplete config file: please specify default TipTilt parameters !"); 
	}
	save();

	Logger::get()->log(Logger::LOG_LEV_DEBUG, "TipTilt vars initialization completed");
}

void Waves::copy(Waves *another) throw (AOVarException) {
	double freqs_req[_n_act], amps_req[_n_act], offsets_req[_n_act], phases_req[_n_act];
	
	another->Freqs(freqs_req);
	another->Amps(amps_req);
	another->Offsets(offsets_req);
	another->Phases(phases_req);
	
	setFreqs(freqs_req);
	setAmps(amps_req);
	setOffsets(offsets_req);
	setPhases(phases_req);
    
	Logger::get()->log(Logger::LOG_LEV_TRACE, "Object succesfully copied");
}

void Waves::Freqs(double freqs[]) throw (AOVarException) {
   for(int i=0; i<_n_act; i++) {
	  _freqs.Get(i, &freqs[i]);
   }
}

void Waves::Amps(double amps[]) throw (AOVarException) {
   for(int i=0; i<_n_act; i++) {
	  _amps.Get(i, &amps[i]);
   }
}

void Waves::Offsets(double offsets[]) throw (AOVarException) {
   for(int i=0; i<_n_act; i++) {
	  _offsets.Get(i, &offsets[i]);
   }
}

void Waves::Phases(double phases[]) throw (AOVarException) {
    for(int i=0; i<_n_act; i++) {
	  _phases.Get(i, &phases[i]);
   }
}

void Waves::setFreqs(const double freqs[]) throw (AOVarException) {
	_freqs.Set(freqs, NO_SEND);
}

void Waves::setAmps(const double amps[]) throw (AOVarException) {
   _amps.Set(amps, NO_SEND);
}

void Waves::setOffsets(const double offsets[]) throw (AOVarException) {
    _offsets.Set(offsets, NO_SEND);
}

void Waves::setPhases(const double phases[]) throw (AOVarException) {
   _phases.Set(phases, NO_SEND);
}

bool Waves::update(Variable *var) throw (AOVarException) {
   return (_freqs.MatchAndSet(var) || 
		   _amps.MatchAndSet(var) ||
		   _offsets.MatchAndSet(var) || 
		   _phases.MatchAndSet(var));
}

void Waves::apply() throw (AOVarException, WavesException) {

	Logger::get()->log(Logger::LOG_LEV_INFO, "------------------------------- LOW-LEV APPLY ----------------------------------");
	
	struct wave_params BCU_waves[_n_act];
        static double lastFreq = -1;
        static double lastAmp = -1;

	// Fill BCU_waves with req Waves values
	double freq, phase, offset, amp;	

   //int remap_waves[] = {1,2,0};
   int remap_waves[] = {0,1,2};

	for(int i=0; i<_n_act; i++) {
		// Get parameters from RTDB vars
		try {
            _freqs.Get(i, &freq);
            _phases.Get(i, &phase); 
            _offsets.Get(i, &offset); 
            _amps.Get(i, &amp);
		} catch (AOVarException &e) { 
			Logger::get()->log(Logger::LOG_LEV_FATAL, "Rtdb Get: %s ", e.what().c_str()); 
    		throw WavesException("Unexpected failure");
		}
		
		// Translate values to actuators parameters
                // These values are OK for LBT but will need to change for Magellan
		//freq = freq * 65536.0 / 118582.6;
		//amp = amp * 6.4 * 2;	
		//offset = (0x4000 * offset) / 5.0;
		//phase = phase * 182.0416666666666667;

                double fixedAmp = _myCtrl->checkAmp( freq, amp);

		BCU_waves[ remap_waves[i]].freq = freq;
		BCU_waves[ remap_waves[i]].offset = offset;
		BCU_waves[ remap_waves[i]].amp = fixedAmp;
      BCU_waves[ remap_waves[i]].phase = phase;
	}

    Logger::get()->log(Logger::LOG_LEV_INFO, "Default phases (0-120-240) in TT Mirror units: %d %d %d", 0, 21845, 43691);
	for(int i=0; i<_n_act; i++) {
		Logger::get()->log(Logger::LOG_LEV_INFO, "Freq: %d - Amp: %d - Offset: %d - Phase: %d", BCU_waves[i].freq, BCU_waves[i].amp, BCU_waves[i].offset, BCU_waves[i].phase);  
	}


	//Send to BCU
        matlab_setTT(_myCtrl->engine(), BCU_waves[0].offset, BCU_waves[1].offset);

        if ((BCU_waves[0].freq != lastFreq) || (BCU_waves[0].amp != lastAmp)) {
            printf("Applying freq = %g Hz, amp = %g rad\n", BCU_waves[0].freq, BCU_waves[0].amp);
            matlab_setMod(_myCtrl->engine(),BCU_waves[0].freq, BCU_waves[0].amp);
            lastFreq = BCU_waves[0].freq;
            lastAmp  = BCU_waves[0].amp;
        }
        
    	Logger::get()->log(Logger::LOG_LEV_INFO, "Applying Waves done !");
    
}

void Waves::save() throw (AOVarException) {
    _freqs.Send();
    _amps.Send();
    _offsets.Send();
    _phases.Send();
	Logger::get()->log(Logger::LOG_LEV_INFO, "Saving to RTDB of Waves done !");
}
