#ifndef WFS_COMMANDS_H_INCLUDE
#define WFS_COMMANDS_H_INCLUDE

#include "arblib/base/Commands.h"
#include "arblib/wfsArb/WfsArbitratorOpCodes.h"
#include "arblib/wfsArb/WfsPTypes.h"

using namespace Arcetri::Arbitrator;

namespace Arcetri {
namespace Wfs_Arbitrator {


class Operate: public Command {

	public:

		/*
		 * Command parameters: [none]
		 */
        Operate(unsigned int timeout_ms, powerOnParams params, string progressVar = ""): 
            Command(WfsOpCodes::OPERATE_CMD, "Operate", timeout_ms, progressVar) { _params = params;}
    
		virtual ~Operate() {}

		void log() const;

		powerOnParams getParams() const { return _params; }

	protected:

		/*
		 * Default constructor needed for boost serialization.
		 */
		 Operate() {}

	private:

		bool validateImpl() throw(CommandValidationException);

		friend class boost::serialization::access;
	    template<class Archive>
		void serialize(Archive& ar, const unsigned int /*version*/) {
			// Serialize base class object
			ar & boost::serialization::base_object<Command>(*this);
			ar & _params;
		}

	    powerOnParams _params;

};


class Off: public Command {

	public:

		/*
		 * Command parameters: [none]
		 */
        Off(unsigned int timeout_ms, string progressVar = ""): 
            Command(WfsOpCodes::OFF_CMD, "Off", timeout_ms, progressVar) {}

		virtual ~Off() {}

	protected:

		/*
		 * Default constructor needed for boost serialization.
		 */
		 Off() {}

	private:

		bool validateImpl() throw(CommandValidationException);

		friend class boost::serialization::access;
	    template<class Archive>
		void serialize(Archive& ar, const unsigned int /*version*/) {
			// Serialize base class object
			ar & boost::serialization::base_object<Command>(*this);
		}

};


class PrepareAcquireRef: public Command {

	public:

		/*
		 * Command parameters: star description
		 */
        PrepareAcquireRef(unsigned int timeout_ms, prepareAcquireRefParams params, string progressVar = ""): 
            Command(WfsOpCodes::PREPARE_ACQUIRE_REF_CMD, "PrepareAcquireRef", timeout_ms, progressVar) {
			_params = params;
		}

		virtual ~PrepareAcquireRef() {}

		void log() const;

		prepareAcquireRefParams getParams() const { return _params; }
      prepareAcquireRefResult *getOutputParams() { return &_outputParams; }

	protected:

		/*
		 * Default constructor needed for boost serialization.
		 */
		PrepareAcquireRef() {}

	private:

		bool validateImpl() throw(CommandValidationException);

		friend class boost::serialization::access;
	    template<class Archive>
		void serialize(Archive& ar, const unsigned int /*version*/) {
			// Serialize base class object
			ar & boost::serialization::base_object<Command>(*this);
			ar & _params;
         ar & _outputParams;
		}

		prepareAcquireRefParams _params;
      prepareAcquireRefResult _outputParams;
};


class AcquireRef: public Command {

	public:

		/*
		 * Command parameters: star description
		 */
        AcquireRef(unsigned int timeout_ms, string progressVar = ""): 
            Command(WfsOpCodes::ACQUIRE_REF_CMD, "AcquireRef", timeout_ms, progressVar) {}

		virtual ~AcquireRef() {}

		void log() const;

      acquireRefResult *getOutputParams()  { return &_outputParams; }

	protected:

		/*
		 * Default constructor needed for boost serialization.
		 */
		 AcquireRef() {}

	private:

		bool validateImpl() throw(CommandValidationException);

		friend class boost::serialization::access;
	    template<class Archive>
		void serialize(Archive& ar, const unsigned int /*version*/) {
			// Serialize base class object
			ar & boost::serialization::base_object<Command>(*this);
         ar & _outputParams;
		}

      acquireRefResult _outputParams;
};


class EnableDisturb: public Command {

	public:

		/*
		 * Command parameters: [none]
		 */
        EnableDisturb(unsigned int timeout_ms, enableDisturbParams params, string progressVar = ""): 
            Command(WfsOpCodes::ENABLE_DISTURB_CMD, "EnableDisturb", timeout_ms, progressVar) { _params = params; }

		virtual ~EnableDisturb() {}

		void log() const;

		enableDisturbParams getParams() { return _params; }

	protected:

		/*
		 * Default constructor needed for boost serialization.
		 */
		 EnableDisturb() {}

	private:

		bool validateImpl() throw(CommandValidationException);

		friend class boost::serialization::access;
	    template<class Archive>
		void serialize(Archive& ar, const unsigned int /*version*/) {
			// Serialize base class object
			ar & boost::serialization::base_object<Command>(*this);
			ar & _params;
		}

		enableDisturbParams _params;

};

class ModifyAO: public Command {

	public:

		/*
		 * Command parameters: [none]
		 */
        ModifyAO(unsigned int timeout_ms, modifyAOparams params, string progressVar = ""): 
            Command(WfsOpCodes::MODIFY_AO_CMD, "ModifyAO", timeout_ms, progressVar) { _params = params; }

		virtual ~ModifyAO() {}

		void log() const;

		modifyAOparams getParams() { return _params; }
		modifyAOparams *getOutputParams() { return &_outputParams; }

	protected:

		/*
		 * Default constructor needed for boost serialization.
		 */
		 ModifyAO() {}

	private:

		bool validateImpl() throw(CommandValidationException);

		friend class boost::serialization::access;
	    template<class Archive>
		void serialize(Archive& ar, const unsigned int /*version*/) {
			// Serialize base class object
			ar & boost::serialization::base_object<Command>(*this);
			ar & _params;
			ar & _outputParams;
		}

		modifyAOparams _params;
		modifyAOparams _outputParams;

};

class SaveStatus: public Command {

	public:

		/*
		 * Command parameters: [none]
		 */
        SaveStatus(unsigned int timeout_ms, saveStatusParams params, string progressVar = ""): 
            Command(WfsOpCodes::SAVE_STATUS_CMD, "Save status", timeout_ms, progressVar) { _params = params; }

		virtual ~SaveStatus() {}

		void log() const;

		saveStatusParams getParams() { return _params; }

	protected:

		/*
		 * Default constructor needed for boost serialization.
		 */
		 SaveStatus() {}

	private:

		bool validateImpl() throw(CommandValidationException);

		friend class boost::serialization::access;
	    template<class Archive>
		void serialize(Archive& ar, const unsigned int /*version*/) {
			// Serialize base class object
			ar & boost::serialization::base_object<Command>(*this);
			ar & _params;
		}

		saveStatusParams _params;

};

class SaveOptLoopData: public Command {

	public:

		/*
		 * Note that the <fsm> parameter to Command() is false.
		 */
        SaveOptLoopData(unsigned int timeout_ms, saveOptLoopDataParams params, string progressVar = ""): 
            Command(WfsOpCodes::SAVE_OPTLOOPDATA_CMD, "Save opt loop data", timeout_ms, progressVar, false) { _params = params; }

		virtual ~SaveOptLoopData() {}

		void log() const;

		saveOptLoopDataParams getParams() { return _params; }

      void setTrackingNum( string trackingNum) { _params._trackNum = trackingNum; }

	protected:

		/*
		 * Default constructor needed for boost serialization.
		 */
		 SaveOptLoopData() {}

	private:

		bool validateImpl() throw(CommandValidationException);

		friend class boost::serialization::access;
	    template<class Archive>
		void serialize(Archive& ar, const unsigned int /*version*/) {
			// Serialize base class object
			ar & boost::serialization::base_object<Command>(*this);
			ar & _params;
		}

		saveOptLoopDataParams _params;

};

class SetSource: public Command {

	public:

		/*
		 * Command parameters: 
		 */
		SetSource(unsigned int timeout_ms, setSourceParams params, string progressVar = ""): 
            Command(WfsOpCodes::SETSOURCE_CMD, "Set source", timeout_ms, progressVar) { _params = params; }

		virtual ~SetSource() {}

		void log() const;

		setSourceParams getParams() { return _params; }

	protected:

		/*
		 * Default constructor needed for boost serialization.
		 */
		 SetSource() {}

	private:

		bool validateImpl() throw(CommandValidationException);

		friend class boost::serialization::access;
	    template<class Archive>
		void serialize(Archive& ar, const unsigned int /*version*/) {
			// Serialize base class object
			ar & boost::serialization::base_object<Command>(*this);
			ar & _params;
		}

		setSourceParams _params;

};


class CloseLoop: public Command {

	public:

        CloseLoop(unsigned int timeout_ms, string progressVar = ""): 
            Command(WfsOpCodes::CLOSE_LOOP_CMD, "CloseLoop", timeout_ms, progressVar) {}

		virtual ~CloseLoop() {}

	protected:

		/*
		 * Default constructor needed for boost serialization.
		 */
		 CloseLoop() {}

	private:

		bool validateImpl() throw(CommandValidationException);

		friend class boost::serialization::access;
	    template<class Archive>
		void serialize(Archive& ar, const unsigned int /*version*/) {
			// Serialize base class object
			ar & boost::serialization::base_object<Command>(*this);
		}

};

class PrepareAdsec: public Command {

	public:

		/*
		 * Command parameters: [none]
		 */
		PrepareAdsec(unsigned int timeout_ms, string progressVar = ""): 
            Command(WfsOpCodes::PREPARE_ADSEC_CMD, "PrepareAdsec", timeout_ms, progressVar) {}

		virtual ~PrepareAdsec() {}

	protected:

		/*
		 * Default constructor needed for boost serialization.
		 */
		 PrepareAdsec() {}

	private:

		bool validateImpl() throw(CommandValidationException);

		friend class boost::serialization::access;
	    template<class Archive>
		void serialize(Archive& ar, const unsigned int /*version*/) {
			// Serialize base class object
			ar & boost::serialization::base_object<Command>(*this);
		}

};


class PauseLoop: public Command {

	public:

		/*
		 * Command parameters: [none]
		 */
		PauseLoop(unsigned int timeout_ms, string progressVar = ""): 
            Command(WfsOpCodes::PAUSE_LOOP_CMD, "PauseLoop", timeout_ms, progressVar) {}

		virtual ~PauseLoop() {}

	protected:

		/*
		 * Default constructor needed for boost serialization.
		 */
		 PauseLoop() {}

	private:

		bool validateImpl() throw(CommandValidationException);

		friend class boost::serialization::access;
	    template<class Archive>
		void serialize(Archive& ar, const unsigned int /*version*/) {
			// Serialize base class object
			ar & boost::serialization::base_object<Command>(*this);
		}

};


class RefineLoop: public Command {

	public:

		/*
		 * Command parameters: [none]
		 */
		RefineLoop(unsigned int timeout_ms, wfsLoopParams wfsLoopPar, string progressVar = ""): 
            Command(WfsOpCodes::REFINE_LOOP_CMD, "RefineLoop", timeout_ms, progressVar) { wfsLoopPar = _wfsLoopPar; }

		virtual ~RefineLoop() {}

		void log() const;

		wfsLoopParams getWfsLoopParameters() { return _wfsLoopPar; }

	protected:

		/*
		 * Default constructor needed for boost serialization.
		 */
		 RefineLoop() {}

	private:

		bool validateImpl() throw(CommandValidationException);

		friend class boost::serialization::access;
	    template<class Archive>
		void serialize(Archive& ar, const unsigned int /*version*/) {
			// Serialize base class object
			ar & boost::serialization::base_object<Command>(*this);
			ar & _wfsLoopPar;
		}

		wfsLoopParams _wfsLoopPar;

};


class ResumeLoop: public Command {

	public:

		/*
		 * Command parameters: [none]
		 */
		ResumeLoop(unsigned int timeout_ms, string progressVar = ""): 
            Command(WfsOpCodes::RESUME_LOOP_CMD, "ResumeLoop", timeout_ms, progressVar) {}

		virtual ~ResumeLoop() {}

	protected:

		/*
		 * Default constructor needed for boost serialization.
		 */
		 ResumeLoop() {}

	private:

		bool validateImpl() throw(CommandValidationException);

		friend class boost::serialization::access;
	    template<class Archive>
		void serialize(Archive& ar, const unsigned int /*version*/) {
			// Serialize base class object
			ar & boost::serialization::base_object<Command>(*this);
		}

};


class StopLoop: public Command {

	public:

		/*
		 * Command parameters: [none]
		 */
		StopLoop(unsigned int timeout_ms, string progressVar = ""): 
            Command(WfsOpCodes::STOP_LOOP_CMD, "StopLoop", timeout_ms, progressVar) {}

		virtual ~StopLoop() {}

	protected:

		/*
		 * Default constructor needed for boost serialization.
		 */
		 StopLoop() {}

	private:

		bool validateImpl() throw(CommandValidationException);

		friend class boost::serialization::access;
	    template<class Archive>
		void serialize(Archive& ar, const unsigned int /*version*/) {
			// Serialize base class object
			ar & boost::serialization::base_object<Command>(*this);
		}

};

class StopLoopAdsec: public Command {

	public:

		/*
		 * Command parameters: [none]
		 */
		StopLoopAdsec(unsigned int timeout_ms, stopLoopAdsecParams params, string progressVar = ""): 
            Command(WfsOpCodes::STOPLOOP_ADSEC_CMD, "StopLoopAdsec", timeout_ms, progressVar) { _params = params;}

		virtual ~StopLoopAdsec() {}

        stopLoopAdsecParams getParams() { return _params; }

	protected:

		/*
		 * Default constructor needed for boost serialization.
		 */
		 StopLoopAdsec() {}

	private:

		bool validateImpl() throw(CommandValidationException);

		friend class boost::serialization::access;
	    template<class Archive>
		void serialize(Archive& ar, const unsigned int /*version*/) {
			// Serialize base class object
			ar & boost::serialization::base_object<Command>(*this);
            ar & _params;
		}

    protected:
        stopLoopAdsecParams _params;
};

class CalibrateHODark: public Command {

	public:

		/*
		 * Command parameters: [none]
		 */
		CalibrateHODark(unsigned int timeout_ms, int nframes=100, string progressVar = ""): 
            Command(WfsOpCodes::CALIBRATE_HODARK_CMD, "CalibrateHODark", timeout_ms, progressVar)
             { _nframes = nframes; }

		virtual ~CalibrateHODark() {}

      int getNframes() { return _nframes; }

	protected:

		/*
		 * Default constructor needed for boost serialization.
		 */
		 CalibrateHODark() {}

       int _nframes;

	private:

		bool validateImpl() throw(CommandValidationException);

		friend class boost::serialization::access;
	    template<class Archive>
		void serialize(Archive& ar, const unsigned int /*version*/) {
			// Serialize base class object
			ar & boost::serialization::base_object<Command>(*this);
         ar & _nframes;
		}

};

class CalibrateIRTCDark: public Command {

	public:

		/*
		 * Command parameters: [none]
		 */
		CalibrateIRTCDark(unsigned int timeout_ms, int nframes=100, string progressVar = ""): 
            Command(WfsOpCodes::CALIBRATE_IRTCDARK_CMD, "CalibrateIRTCDark", timeout_ms, progressVar)
             { _nframes = nframes; }

		virtual ~CalibrateIRTCDark() {}

      int getNframes() { return _nframes; }

	protected:

		/*
		 * Default constructor needed for boost serialization.
		 */
		 CalibrateIRTCDark() {}

       int _nframes;

	private:

		bool validateImpl() throw(CommandValidationException);

		friend class boost::serialization::access;
	    template<class Archive>
		void serialize(Archive& ar, const unsigned int /*version*/) {
			// Serialize base class object
			ar & boost::serialization::base_object<Command>(*this);
         ar & _nframes;
		}

};

class CalibratePISCESDark: public Command {

	public:

		/*
		 * Command parameters: [none]
		 */
		CalibratePISCESDark(unsigned int timeout_ms, int nframes=100, string progressVar = ""): 
            Command(WfsOpCodes::CALIBRATE_PISCESDARK_CMD, "CalibratePISCESDark", timeout_ms, progressVar)
             { _nframes = nframes; }

		virtual ~CalibratePISCESDark() {}

      int getNframes() { return _nframes; }

	protected:

		/*
		 * Default constructor needed for boost serialization.
		 */
		 CalibratePISCESDark() {}

       int _nframes;

	private:

		bool validateImpl() throw(CommandValidationException);

		friend class boost::serialization::access;
	    template<class Archive>
		void serialize(Archive& ar, const unsigned int /*version*/) {
			// Serialize base class object
			ar & boost::serialization::base_object<Command>(*this);
         ar & _nframes;
		}

};

class CalibrateTVDark: public Command {

	public:

		/*
		 * Command parameters: [none]
		 */
		CalibrateTVDark(unsigned int timeout_ms, int nframes=10, string progressVar = ""): 
            Command(WfsOpCodes::CALIBRATE_TVDARK_CMD, "CalibrateTVDark", timeout_ms, progressVar)
             { _nframes = nframes; }

		virtual ~CalibrateTVDark() {}

      int getNframes() { return _nframes; }

	protected:

		/*
		 * Default constructor needed for boost serialization.
		 */
		 CalibrateTVDark() {}

       int _nframes;

	private:

		bool validateImpl() throw(CommandValidationException);

		friend class boost::serialization::access;
	    template<class Archive>
		void serialize(Archive& ar, const unsigned int /*version*/) {
			// Serialize base class object
			ar & boost::serialization::base_object<Command>(*this);
         ar & _nframes;
		}

};

class CalibrateMovements: public Command {

	public:

		/*
		 * Command parameters: [none]
		 */
		CalibrateMovements(unsigned int timeout_ms, string progressVar = ""): 
            Command(WfsOpCodes::CALIBRATE_MOVEMENTS_CMD, "CalibrateMovements", timeout_ms, progressVar) {}

		virtual ~CalibrateMovements() {}

	protected:

		/*
		 * Default constructor needed for boost serialization.
		 */
		 CalibrateMovements() {}

	private:

		bool validateImpl() throw(CommandValidationException);

		friend class boost::serialization::access;
	    template<class Archive>
		void serialize(Archive& ar, const unsigned int /*version*/) {
			// Serialize base class object
			ar & boost::serialization::base_object<Command>(*this);
		}

};

class CalibrateSlopenull: public Command {

	public:

		/*
		 * Command parameters: [none]
		 */
		CalibrateSlopenull(unsigned int timeout_ms, int nframes=100, string progressVar = ""): 
            Command(WfsOpCodes::CALIBRATE_SLOPENULL_CMD, "CalibrateSlopenull", timeout_ms, progressVar)
           { _nframes = nframes; }

		virtual ~CalibrateSlopenull() {}

      int getNframes() {  return _nframes; }

	protected:

		/*
		 * Default constructor needed for boost serialization.
		 */
		 CalibrateSlopenull() {}

       int _nframes;

	private:

		bool validateImpl() throw(CommandValidationException);

		friend class boost::serialization::access;
	    template<class Archive>
		void serialize(Archive& ar, const unsigned int /*version*/) {
			// Serialize base class object
			ar & boost::serialization::base_object<Command>(*this);
         ar & _nframes;
		}

};

class GetTVSnap: public Command {

	public:

		/*
		 * Command parameters: [none]
		 */
		GetTVSnap(unsigned int timeout_ms, string progressVar = ""): 
            Command(WfsOpCodes::GET_TV_SNAP_CMD, "GetTVSnap", timeout_ms, progressVar) {}

		virtual ~GetTVSnap() {}

      snapParams *getOutputParams() { return &_outputParams; }


	protected:

		/*
		 * Default constructor needed for boost serialization.
		 */
		 GetTVSnap() {}

	private:

		bool validateImpl() throw(CommandValidationException);

		friend class boost::serialization::access;
	    template<class Archive>
		void serialize(Archive& ar, const unsigned int /*version*/) {
			// Serialize base class object
			ar & boost::serialization::base_object<Command>(*this);
         ar & _outputParams;
		}

   snapParams _outputParams;

};

class CheckRef: public Command {

	public:

		CheckRef(unsigned int timeout_ms, string progressVar = ""): 
            Command(WfsOpCodes::CHECK_REF_CMD, "CheckRef", timeout_ms, progressVar) {}

		virtual ~CheckRef() {}

      checkRefResult *getOutputParams() { return &_outputParams; }


	protected:

		/*
		 * Default constructor needed for boost serialization.
		 */
		 CheckRef() {}

	private:

		bool validateImpl() throw(CommandValidationException);

		friend class boost::serialization::access;
	    template<class Archive>
		void serialize(Archive& ar, const unsigned int /*version*/) {
			// Serialize base class object
			ar & boost::serialization::base_object<Command>(*this);
         ar & _outputParams;
		}

   checkRefResult _outputParams;

};

class OffsetXY: public Command {

	public:

		/*
		 * Command parameters: [none]
		 */
		OffsetXY(unsigned int timeout_ms, offsetXYparams params, string progressVar = ""): 
            Command(WfsOpCodes::OFFSET_XY, "OffsetXY", timeout_ms, progressVar) { _params = params; }

		virtual ~OffsetXY() {}

		void log() const;

		offsetXYparams getParams() { return _params; }

	protected:

		/*
		 * Default constructor needed for boost serialization.
		 */
		 OffsetXY() {}

	private:

		bool validateImpl() throw(CommandValidationException);

		friend class boost::serialization::access;
	    template<class Archive>
		void serialize(Archive& ar, const unsigned int /*version*/) {
			// Serialize base class object
			ar & boost::serialization::base_object<Command>(*this);
			ar & _params;
		}

		offsetXYparams _params;

};

class OffsetZ: public Command {

	public:

		/*
		 * Command parameters: [none]
		 */
		OffsetZ(unsigned int timeout_ms, offsetZparams params, string progressVar = ""): 
            Command(WfsOpCodes::OFFSET_Z, "OffsetZ", timeout_ms, progressVar) { _params = params; }

		virtual ~OffsetZ() {}

		void log() const;

		offsetZparams getParams() { return _params; }

	protected:

		/*
		 * Default constructor needed for boost serialization.
		 */
		 OffsetZ() {}

	private:

		bool validateImpl() throw(CommandValidationException);

		friend class boost::serialization::access;
	    template<class Archive>
		void serialize(Archive& ar, const unsigned int /*version*/) {
			// Serialize base class object
			ar & boost::serialization::base_object<Command>(*this);
			ar & _params;
		}

		offsetZparams _params;

};

class OptimizeGain: public Command {

	public:

		/*
		 * Command parameters: [none]
		 */
		OptimizeGain(unsigned int timeout_ms, string progressVar = ""): 
            Command(WfsOpCodes::OPTIMIZE_GAIN_CMD, "OptimizeGain", timeout_ms, progressVar) { }

		virtual ~OptimizeGain() {}

		void log() const;


	protected:

		/*
		 * Default constructor needed for boost serialization.
		 */
		 OptimizeGain() {}

	private:

		bool validateImpl() throw(CommandValidationException);

		friend class boost::serialization::access;
	    template<class Archive>
		void serialize(Archive& ar, const unsigned int /*version*/) {
			// Serialize base class object
			ar & boost::serialization::base_object<Command>(*this);
		}


};

class CorrectModes: public Command {

	public:

		/*
		 * Command parameters: [none]
		 */
		CorrectModes(unsigned int timeout_ms, correctModesParams params, string progressVar = ""): 
            Command(WfsOpCodes::CORRECT_MODES, "CorrectModes", timeout_ms, progressVar) { _params = params; }

		virtual ~CorrectModes() {}


		correctModesParams getParams() { return _params; }

	protected:

		/*
		 * Default constructor needed for boost serialization.
		 */
		 CorrectModes() {}

	private:

		bool validateImpl() throw(CommandValidationException);

		friend class boost::serialization::access;
	    template<class Archive>
		void serialize(Archive& ar, const unsigned int /*version*/) {
			// Serialize base class object
			ar & boost::serialization::base_object<Command>(*this);
			ar & _params;
		}

		correctModesParams _params;

};

class AntiDrift: public Command {

	public:

		/*
		 * Command parameters: [none]
		 */
		AntiDrift(unsigned int timeout_ms, antiDriftParams params, string progressVar = ""): 
            Command(WfsOpCodes::ANTIDRIFT_CMD, "AntiDrift", timeout_ms, progressVar) { _params = params; }

		virtual ~AntiDrift() {}


		antiDriftParams getParams() { return _params; }

	protected:

		/*
		 * Default constructor needed for boost serialization.
		 */
		 AntiDrift() {}

	private:

		bool validateImpl() throw(CommandValidationException);

		friend class boost::serialization::access;
	    template<class Archive>
		void serialize(Archive& ar, const unsigned int /*version*/) {
			// Serialize base class object
			ar & boost::serialization::base_object<Command>(*this);
			ar & _params;
		}

		antiDriftParams _params;

};

class AutoTrack: public Command {

	public:

		/*
		 * Command parameters: [none]
		 */
		AutoTrack(unsigned int timeout_ms, autoTrackParams params, string progressVar = ""): 
            Command(WfsOpCodes::AUTOTRACK_CMD, "AutoTrack", timeout_ms, progressVar) { _params = params; }

		virtual ~AutoTrack() {}
		void log() const;

		autoTrackParams getParams() { return _params; }

	protected:

		/*
		 * Default constructor needed for boost serialization.
		 */
		 AutoTrack() {}

	private:

		bool validateImpl() throw(CommandValidationException);

		friend class boost::serialization::access;
	    template<class Archive>
		void serialize(Archive& ar, const unsigned int /*version*/) {
			// Serialize base class object
			ar & boost::serialization::base_object<Command>(*this);
			ar & _params;
		}

		autoTrackParams _params;

};

class EmergencyOff: public Command {

	public:

		/*
		 * Command parameters: [none]
		 */
		EmergencyOff(unsigned int timeout_ms, string progressVar = ""): 
            Command(WfsOpCodes::EMERGENCY_OFF, "EmergencyOff", timeout_ms, progressVar) { }

		virtual ~EmergencyOff() {}


	protected:

		/*
		 * Default constructor needed for boost serialization.
		 */
		 EmergencyOff() {}

	private:

		bool validateImpl() throw(CommandValidationException);

		friend class boost::serialization::access;
	    template<class Archive>
		void serialize(Archive& ar, const unsigned int /*version*/) {
			// Serialize base class object
			ar & boost::serialization::base_object<Command>(*this);
		}


};


class RequestWfsStatus: public RequestStatus {

   public:

      /*
       * Command parameters: [none]
       */
      RequestWfsStatus(unsigned int timeout_ms):
         RequestStatus(timeout_ms, WfsOpCodes::REQUEST_WFS_STATUS_CMD) {
            _sourceName = "";
            _clStatReady = false;
            _rerotTracking = false;
            _adcTracking = false;
            _lensTracking = false;
            _lensTrackingTarget = false;
            _lensTrackingOutOfRange = false;
            _lensTrackingAlert = false;
         }

      virtual ~RequestWfsStatus() {}

      void setSourceName( string sourceName) { _sourceName = sourceName; }
      void setClStatReady( bool ready) { _clStatReady = ready; }
      void setRerotTracking( bool tracking) { _rerotTracking = tracking; }
      void setAdcTracking( bool tracking) { _adcTracking = tracking; }
      void setLensTracking( bool tracking) { _lensTracking = tracking; }
      void setLensTrackingTarget( bool target) { _lensTrackingTarget = target; }
      void setLensTrackingOutOfRange( bool outofrange) { _lensTrackingOutOfRange = outofrange; }
      void setLensTrackingAlert( bool alert) { _lensTrackingAlert = alert; }

      string getSourceName() { return _sourceName; }
      bool getClStatReady() { return _clStatReady; }
      bool getRerotTracking() { return _rerotTracking; }
      bool getAdcTracking() { return _adcTracking; }
      bool getLensTracking() { return _lensTracking; }
      bool getLensTrackingTarget() { return _lensTrackingTarget; }
      bool getLensTrackingOutOfRange() { return _lensTrackingOutOfRange; }
      bool getLensTrackingAlert() { return _lensTrackingAlert; }

   protected:

      /*
       * Default constructor needed for boost serialization.
       */
    RequestWfsStatus() {}

    string _sourceName;
    bool _clStatReady;
    bool _rerotTracking;
    bool _adcTracking;
    bool _lensTracking;
    bool _lensTrackingTarget;
    bool _lensTrackingOutOfRange;
    bool _lensTrackingAlert;

   private:

      bool validateImpl() throw(CommandValidationException);

      friend class boost::serialization::access;
       template<class Archive>
      void serialize(Archive& ar, const unsigned int /*version*/) {
          // Serialize base class object
          ar & boost::serialization::base_object<RequestStatus>(*this);
          ar & _sourceName;
          ar & _clStatReady;
          ar & _rerotTracking;
          ar & _adcTracking;
          ar & _lensTracking;
          ar & _lensTrackingTarget;
          ar & _lensTrackingOutOfRange;
          ar & _lensTrackingAlert;
    }
                                                                                                                            

};





}
}


#endif /*WFS_COMMANDS_H_INCLUDE*/
