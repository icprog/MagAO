#ifndef ADSEC_COMMANDS_H_INCLUDE
#define ADSEC_COMMANDS_H_INCLUDE

#include "arblib/adSecArb/AdSecOpCodes.h"
#include "arblib/adSecArb/AdSecPTypes.h"

#include "arblib/base/Commands.h"

//using namespace Arcetri::Arbitrator;


namespace Arcetri {
namespace AdSec_Arbitrator {

/*
 * Base class for:
 * - SetFlatChop, SetFlatAo command classes
 * - XToY command classes
 * because they have the same parameters.
 *
 * This class can't be instatiated, because there aren't public
 * constructors and the class is abstract (pure virtual method).
 */
class SetMirror: public Command {

	public:

		virtual ~SetMirror() {}

		flatParams getParams() { return _params; }

	protected:

		/*
		 * Default constructor needed for boost serialization.
		 */
		SetMirror() {}

		/*
		 *
		 */
		SetMirror(unsigned int timeout_ms, OpCode code, string description, flatParams params, string progressVar = "");

		void log() const;


	private:

		bool validateImpl() throw(CommandValidationException) = 0;

		friend class boost::serialization::access;
	    template<class Archive>
		void serialize(Archive& ar, const unsigned int /*version*/) {
			// Serialize base class object
			ar & boost::serialization::base_object<Command>(*this);
			ar & _params;
		}

		flatParams _params;

};

class RequestAdSecStatus: public RequestStatus {

	public:

		// Command parameters: [none]
		RequestAdSecStatus(unsigned int timeout_ms):
				 RequestStatus(timeout_ms, AdSecOpCodes::REQUEST_ADSEC_STATUS_CMD) {
                _safeSkipFramesCounter = -1;
                _safeSkipPercent = 0;
                _isSkippingFrames = false;
                _clStatReady = false;
                _coilsEnabled = false;
                _tssEnabled = false;
                _tipOffload=0;
                _tiltOffload=0;
                _focusOffload=0;
             }

		virtual ~RequestAdSecStatus() {}

      int safeSkipFramesCounter() { return _safeSkipFramesCounter; }
      float safeSkipPercent() { return _safeSkipPercent; }
      bool isSkippingFrames() { return _isSkippingFrames; }
      bool getClStatReady() { return _clStatReady; }
      bool coilsEnabled() { return _coilsEnabled; }
      bool tssEnabled() { return _tssEnabled; }
      double tipOffload() { return _tipOffload; }
      double tiltOffload() { return _tiltOffload; }
      double focusOffload() { return _focusOffload; }


      void setClStatReady( bool ready) { _clStatReady = ready; }

      void setSkipping( int counter, bool skipping, float percent) {
         _safeSkipFramesCounter = counter;
         _isSkippingFrames = skipping;
         _safeSkipPercent = percent;
      }

      void setCoilsEnabled( bool enabled) { _coilsEnabled = enabled; }
      void setTssEnabled( bool enabled) { _tssEnabled = enabled; }
      void setTipOffload( double value) { _tipOffload = value; }
      void setTiltOffload( double value) { _tiltOffload = value; }
      void setFocusOffload( double value) { _focusOffload = value; }

	protected:

		 // Default constructor needed for boost serialization.
		 RequestAdSecStatus() {}

       int _safeSkipFramesCounter;
       float _safeSkipPercent;
       bool _isSkippingFrames;
       bool _clStatReady;
       bool _coilsEnabled;
       bool _tssEnabled;
       double _tipOffload;
       double _tiltOffload;
       double _focusOffload;

	private:

		bool validateImpl() throw(CommandValidationException);

		friend class boost::serialization::access;
	    template<class Archive>
		void serialize(Archive& ar, const unsigned int /* version */) {
			// Serialize base class object
			ar & boost::serialization::base_object<RequestStatus>(*this);
         ar & _safeSkipFramesCounter;
         ar & _safeSkipPercent;
         ar & _isSkippingFrames;
         ar & _clStatReady;
         ar & _coilsEnabled;
         ar & _tssEnabled;
         ar & _tipOffload;
         ar & _tiltOffload;
         ar & _focusOffload;
		}

};

class AoToChop: public SetMirror {

	public:

		/*
		 * Command parameters: [none]
		 */
		AoToChop(unsigned int timeout_ms, flatParams params, string progressVar = ""):
            SetMirror(timeout_ms, AdSecOpCodes::AO_TO_CHOP_CMD, "Ao to Chop", params, progressVar) {}

		virtual ~AoToChop() {}

	protected:

		/*
		 * Default constructor needed for boost serialization.
		 */
		 AoToChop() {}

	private:

		bool validateImpl() throw(CommandValidationException);

		friend class boost::serialization::access;
	    template<class Archive>
		void serialize(Archive& ar, const unsigned int /*version*/) {
			// Serialize base class object
			ar & boost::serialization::base_object<SetMirror>(*this);
		}

};

/* L.F. removed
class AoToSl: public SetMirror {

	public:

		// Command parameters: [none]
		AoToSl(unsigned int timeout_ms, flatParams params):
			   SetMirror(timeout_ms, AdSecOpCodes::AO_TO_SL_CMD, "Ao to Sl", params) {}

		virtual ~AoToSl() {}

	protected:

		 // Default constructor needed for boost serialization.
		 AoToSl() {}

	private:

		bool validateImpl() throw(CommandValidationException);

		friend class boost::serialization::access;
	    template<class Archive>
		void serialize(Archive& ar, const unsigned int version) {
			// Serialize base class object
			ar & boost::serialization::base_object<SetMirror>(*this);
		}

};
  end of remove */

class ApplyCommands: public Command {

	public:

		// Command parameters: [none]
		ApplyCommands(unsigned int timeout_ms, applyCommandParams params, string progressVar = ""): 
            Command(AdSecOpCodes::APPLY_COMMANDS_CMD, "Apply commands", timeout_ms, progressVar) { _params = params; }

		virtual ~ApplyCommands() {}

		void log() const;

	   applyCommandParams getParams() { return _params; }

	protected:

		 // Default constructor needed for boost serialization.
		 ApplyCommands() {}

	private:

		bool validateImpl() throw(CommandValidationException);

		friend class boost::serialization::access;
	    template<class Archive>
		void serialize(Archive& ar, const unsigned int /*version*/) {
			// Serialize base class object
			ar & boost::serialization::base_object<Command>(*this);
			ar & _params;
		}

		applyCommandParams _params;

};


class RecoverSkipFrame: public Command {

	public:

		/*
		 * Command parameters: [none]
		 */
		RecoverSkipFrame(unsigned int timeout_ms, string progressVar = ""): 
            Command(AdSecOpCodes::RECOVER_SKIP_FRAME_CMD, "RecoverSkipFrame", timeout_ms, progressVar) {}

		virtual ~RecoverSkipFrame() {}

	protected:

		/*
		 * Default constructor needed for boost serialization.
		 */
		 RecoverSkipFrame() {}

	private:

		bool validateImpl() throw(CommandValidationException);

		friend class boost::serialization::access;
	    template<class Archive>
		void serialize(Archive& ar, const unsigned int /*version*/) {
			// Serialize base class object
			ar & boost::serialization::base_object<Command>(*this);
		}

};


class ChopToAo: public SetMirror {

	public:
		 // Command parameters: [none]
		ChopToAo(unsigned int timeout_ms, flatParams params, string progressVar = ""):
            SetMirror(timeout_ms, AdSecOpCodes::CHOP_TO_AO_CMD, "Chop to Ao", params, progressVar) {}

		virtual ~ChopToAo() {}

	protected:

		 // Default constructor needed for boost serialization.
		 ChopToAo() {}

	private:

		bool validateImpl() throw(CommandValidationException);

		friend class boost::serialization::access;
	    template<class Archive>
		void serialize(Archive& ar, const unsigned int /*version*/) {
			// Serialize base class object
			ar & boost::serialization::base_object<SetMirror>(*this);
		}

};

/* L.F. removed 
class ChopToSl: public SetMirror {

	public:

		 // Command parameters: [none]
		ChopToSl(unsigned int timeout_ms, flatParams params):
				 SetMirror(timeout_ms, AdSecOpCodes::CHOP_TO_SL_CMD, "Chop to Sl", params) {}

		virtual ~ChopToSl() {}

	protected:

		 // Default constructor needed for boost serialization.
		 ChopToSl() {}

	private:

		bool validateImpl() throw(CommandValidationException);

		friend class boost::serialization::access;
	    template<class Archive>
		void serialize(Archive& ar, const unsigned int version) {
			// Serialize base class object
			ar & boost::serialization::base_object<SetMirror>(*this);
		}

};
end of remove */

class LoadProgram: public Command {

	public:

		 // Command parameters: [none]
		LoadProgram(unsigned int timeout_ms, string progressVar = ""): 
            Command(AdSecOpCodes::LOAD_PROGRAM_CMD, "Load program", timeout_ms, progressVar) {}

		virtual ~LoadProgram() {}

	protected:

		/*
		 * Default constructor needed for boost serialization.
		 */
		 LoadProgram() {}

	private:

		bool validateImpl() throw(CommandValidationException);

		friend class boost::serialization::access;
	    template<class Archive>
		void serialize(Archive& ar, const unsigned int /*version*/) {
			// Serialize base class object
			ar & boost::serialization::base_object<Command>(*this);
		}

};


class Off: public Command {

	public:

		 // Command parameters: [none]
		Off(unsigned int timeout_ms, string progressVar = ""): 
            Command(AdSecOpCodes::OFF_CMD, "Power off", timeout_ms, progressVar) {}

		virtual ~Off() {}

	protected:

		 // Default constructor needed for boost serialization.
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


class On: public Command {

	public:

		 // Command parameters: [none]
		On(unsigned int timeout_ms, string progressVar = ""): 
            Command(AdSecOpCodes::ON_CMD, "Power on", timeout_ms, progressVar) {}

		virtual ~On() {}

	protected:

		 // Default constructor needed for boost serialization.
		 On() {}

	private:

		bool validateImpl() throw(CommandValidationException);

		friend class boost::serialization::access;
	    template<class Archive>
		void serialize(Archive& ar, const unsigned int /*version*/) {
			// Serialize base class object
			ar & boost::serialization::base_object<Command>(*this);
		}

};


class Reset: public Command {

	public:

		/*
		 * Command parameters: [none]
		 */
		Reset(unsigned int timeout_ms, string progressVar = ""): 
            Command(AdSecOpCodes::RESET_CMD, "Reset", timeout_ms, progressVar) {}

		virtual ~Reset() {}

	protected:

		/*
		 * Default constructor needed for boost serialization.
		 */
		 Reset() {}

	private:

		bool validateImpl() throw(CommandValidationException);

		friend class boost::serialization::access;
	    template<class Archive>
		void serialize(Archive& ar, const unsigned int /*version*/) {
			// Serialize base class object
			ar & boost::serialization::base_object<Command>(*this);
		}

};


class Rest: public Command {

	public:

		/*
		 * Command parameters: [none]
		 */
		Rest(unsigned int timeout_ms, string progressVar = ""): 
            Command(AdSecOpCodes::REST_CMD, "Rest", timeout_ms, progressVar) {}

		virtual ~Rest() {}

	protected:

		/*
		 * Default constructor needed for boost serialization.
		 */
		 Rest() {}

	private:

		bool validateImpl() throw(CommandValidationException);

		friend class boost::serialization::access;
	    template<class Archive>
		void serialize(Archive& ar, const unsigned int /*version*/) {
			// Serialize base class object
			ar & boost::serialization::base_object<Command>(*this);
		}

};


class SetZernikes: public Command {

	public:

		/*
		 * Command parameters: [none]
		 */
		SetZernikes(unsigned int timeout_ms, setZernikesParams params, string progressVar = ""): 
            Command(AdSecOpCodes::SET_ZERNIKES_CMD, "Set zernikes", timeout_ms, progressVar) { _params = params; }

		virtual ~SetZernikes() {}

      setZernikesParams getParams() { return _params; }

	protected:

		/*
		 * Default constructor needed for boost serialization.
		 */
		 SetZernikes() {}

	private:

		bool validateImpl() throw(CommandValidationException);

		friend class boost::serialization::access;
	    template<class Archive>
		void serialize(Archive& ar, const unsigned int /*version*/) {
			// Serialize base class object
			ar & boost::serialization::base_object<Command>(*this);
         ar & _params;
		}

    setZernikesParams _params;

};


class RunAo: public Command {

	public:

		/*
		 * Command parameters: [none]
		 */
		RunAo(unsigned int timeout_ms, runAoParams params, string progressVar = ""): 
            Command(AdSecOpCodes::RUN_AO_CMD, "Run Ao", timeout_ms, progressVar) { _params = params; }

		virtual ~RunAo() {}

      runAoParams getParams() { return _params; }

	protected:

		/*
		 * Default constructor needed for boost serialization.
		 */
		 RunAo() {}

	private:

		bool validateImpl() throw(CommandValidationException);

		friend class boost::serialization::access;
	    template<class Archive>
		void serialize(Archive& ar, const unsigned int /*version*/) {
			// Serialize base class object
			ar & boost::serialization::base_object<Command>(*this);
         ar & _params;
		}

    runAoParams _params;

};

class PauseAo: public Command {

	public:

		/*
		 * Command parameters: [none]
		 */
		PauseAo(unsigned int timeout_ms, string progressVar = ""): 
            Command(AdSecOpCodes::PAUSE_AO_CMD, "Pause Ao", timeout_ms, progressVar) { }

		virtual ~PauseAo() {}

	protected:

		/*
		 * Default constructor needed for boost serialization.
		 */
		 PauseAo() {}

	private:

		bool validateImpl() throw(CommandValidationException);

		friend class boost::serialization::access;
	    template<class Archive>
		void serialize(Archive& ar, const unsigned int /*version*/) {
			// Serialize base class object
			ar & boost::serialization::base_object<Command>(*this);
		}


};


class ResumeAo: public Command {

	public:

		/*
		 * Command parameters: [none]
		 */
		ResumeAo(unsigned int timeout_ms, string progressVar = ""): 
            Command(AdSecOpCodes::RESUME_AO_CMD, "ResumeAo Ao", timeout_ms, progressVar) { }

		virtual ~ResumeAo() {}

	protected:

		/*
		 * Default constructor needed for boost serialization.
		 */
		 ResumeAo() {}

	private:

		bool validateImpl() throw(CommandValidationException);

		friend class boost::serialization::access;
	    template<class Archive>
		void serialize(Archive& ar, const unsigned int /*version*/) {
			// Serialize base class object
			ar & boost::serialization::base_object<Command>(*this);
		}


};


class RunChop: public Command {

	public:

		/*
		 * Command parameters: [none]
		 */
		RunChop(unsigned int timeout_ms, string progressVar = ""): 
            Command(AdSecOpCodes::RUN_CHOP_CMD, "Run chop", timeout_ms, progressVar) {}

		virtual ~RunChop() {}

	protected:

		/*
		 * Default constructor needed for boost serialization.
		 */
		 RunChop() {}

	private:

		bool validateImpl() throw(CommandValidationException);

		friend class boost::serialization::access;
	    template<class Archive>
		void serialize(Archive& ar, const unsigned int /*version*/) {
			// Serialize base class object
			ar & boost::serialization::base_object<Command>(*this);
		}

};


class SetFlatAo: public Command {

	public:

		/*
		 * Command parameters: [none]
		 */

      SetFlatAo(unsigned int timeout_ms, string progressVar = ""):
          Command(AdSecOpCodes::SET_FLAT_AO_CMD, "Set Flat AO", timeout_ms, progressVar) {}

		virtual ~SetFlatAo() {}


	protected:

		/*
		 * Default constructor needed for boost serialization.
		 */
		 SetFlatAo() {}

	private:

		bool validateImpl() throw(CommandValidationException);

		friend class boost::serialization::access;
	    template<class Archive>
		void serialize(Archive& ar, const unsigned int /*version*/) {
			// Serialize base class object
			ar & boost::serialization::base_object<Command>(*this);
		}

};


class SetFlatChop: public SetMirror {

	public:

		/*
		 *
		 */
		SetFlatChop(unsigned int timeout_ms, flatParams params, string progressVar = ""):
            SetMirror(timeout_ms, AdSecOpCodes::SET_FLAT_CHOP_CMD, "Set Flat Chop", params, progressVar) {}

		virtual ~SetFlatChop() {}

	protected:

		/*
		 * Default constructor needed for boost serialization.
		 */
		 SetFlatChop() {}

	private:

		bool validateImpl() throw(CommandValidationException);

		friend class boost::serialization::access;
	    template<class Archive>
		void serialize(Archive& ar, const unsigned int /*version*/) {
			// Serialize base class object
			ar & boost::serialization::base_object<SetMirror>(*this);
		}

};

/* L.F. removed
class SetFlatSl: public SetMirror {

	public:

		 // Command parameters: [none]
		SetFlatSl(unsigned int timeout_ms, flatParams params):
				  SetMirror(timeout_ms, AdSecOpCodes::SET_FLAT_SL_CMD, "Set Flat Sl", params) { }

		virtual ~SetFlatSl() {}

	protected:

		 // Default constructor needed for boost serialization.
		 SetFlatSl() {}

	private:

		bool validateImpl() throw(CommandValidationException);

		friend class boost::serialization::access;
	    template<class Archive>
		void serialize(Archive& ar, const unsigned int version) {
			// Serialize base class object
			ar & boost::serialization::base_object<SetMirror>(*this);
		}

};
  end of remove */

class SetGain: public Command {

	public:

		/*
		 * Command parameters: [none]
		 */
		SetGain(unsigned int timeout_ms, gainParams params, string progressVar = ""): 
            Command(AdSecOpCodes::SET_GAIN_CMD, "SetGain", timeout_ms, progressVar) { _params = params; }

		virtual ~SetGain() {}

      gainParams getParams() { return _params; }

	protected:

		/*
		 * Default constructor needed for boost serialization.
		 */
		 SetGain() {}

		 void log() const;

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
       gainParams _params;

};


class SaveSlopes: public Command {

	public:

		/*
		 * Command parameters: [none]
		 */
		SaveSlopes(unsigned int timeout_ms, saveSlopesParams params, string progressVar = ""): 
            Command(AdSecOpCodes::SAVE_SLOPES_CMD, "Save slopes", timeout_ms, progressVar) { _params = params; }

		virtual ~SaveSlopes() {}

      saveSlopesParams getParams() { return _params; }

	protected:

		/*
		 * Default constructor needed for boost serialization.
		 */
		 SaveSlopes() {}

		 void log() const;

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
       saveSlopesParams _params;

};

class SaveStatus: public Command {

	public:

		/*
		 * Command parameters: [none]
		 */
		SaveStatus(unsigned int timeout_ms, saveStatusParams params, string progressVar = ""): 
            Command(AdSecOpCodes::SAVE_STATUS_CMD, "Save status", timeout_ms, progressVar) { _params = params; }

		virtual ~SaveStatus() {}

      saveStatusParams getParams() { return _params; }

	protected:

		/*
		 * Default constructor needed for boost serialization.
		 */
		 SaveStatus() {}

		 void log() const;

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
       saveStatusParams _params;

};

class SetDisturb: public Command {

	public:

		/*
		 * Command parameters: [none]
		 */
		SetDisturb(unsigned int timeout_ms, disturbParams params, string progressVar = ""): 
            Command(AdSecOpCodes::SET_DISTURB_CMD, "Set disturb", timeout_ms, progressVar) { _params = params; }

		virtual ~SetDisturb() {}

      disturbParams getParams() { return _params; }

		void log() const;

	protected:

		/*
		 * Default constructor needed for boost serialization.
		 */
		 SetDisturb() {}

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
       disturbParams _params;

};

class LoadShape: public Command {

	public:

		/*
		 * Command parameters: [none]
		 */
		LoadShape(unsigned int timeout_ms, shapeParams params, string progressVar = ""): 
            Command(AdSecOpCodes::LOAD_SHAPE_CMD, "Load shape", timeout_ms, progressVar) { _params = params;}

		virtual ~LoadShape() {}

      shapeParams getParams() { return _params; }

		void log() const;

	protected:

		/*
		 * Default constructor needed for boost serialization.
		 */
		 LoadShape() {}

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
       shapeParams _params;

};

class SaveShape: public Command {

	public:

		/*
		 * Command parameters: [none]
		 */
		SaveShape(unsigned int timeout_ms, shapeParams params, string progressVar = ""): 
            Command(AdSecOpCodes::SAVE_SHAPE_CMD, "Save shape", timeout_ms, progressVar) { _params = params;}

		virtual ~SaveShape() {}

      shapeParams getParams() { return _params; }

		void log() const;

	protected:

		/*
		 * Default constructor needed for boost serialization.
		 */
		 SaveShape() {}

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
       shapeParams _params;

};


class SetDataDecimation: public Command {

	public:

		/*
		 * Command parameters: [none]
		 */
		SetDataDecimation(unsigned int timeout_ms, dataDecimationParams params, string progressVar = ""): 
            Command(AdSecOpCodes::DATA_DECIMATION_CMD, "Set data decimation", timeout_ms, progressVar) { _params = params;}

		virtual ~SetDataDecimation() {}

      dataDecimationParams getParams() { return _params; }

		void log() const;

	protected:

		/*
		 * Default constructor needed for boost serialization.
		 */
		 SetDataDecimation() {}

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
       dataDecimationParams _params;

};


class SelectFocalStation: public Command {

	public:

		/*
		 * Command parameters: [none]
		 */
		SelectFocalStation(unsigned int timeout_ms, selectFocalStationParams params, string progressVar = ""): 
            Command(AdSecOpCodes::SELECT_FOCAL_STATION_CMD, "Set focal station", timeout_ms, progressVar) { _params = params;}

		virtual ~SelectFocalStation() {}

      selectFocalStationParams getParams() { return _params; }

		void log() const;

	protected:

		/*
		 * Default constructor needed for boost serialization.
		 */
		 SelectFocalStation() {}

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
       selectFocalStationParams _params;

};





class SetRecMat: public Command {

	public:

		/*
		 * Command parameters: [none]
		 */
		SetRecMat(unsigned int timeout_ms, recMatParams params, string progressVar = ""): 
            Command(AdSecOpCodes::SET_REC_MAT_CMD, "SetRecMat", timeout_ms, progressVar) { _params = params;}

		virtual ~SetRecMat() {}

      recMatParams getParams() { return _params; }

		void log() const;

	protected:

		/*
		 * Default constructor needed for boost serialization.
		 */
		 SetRecMat() {}

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
       recMatParams _params;

};

class TTOffload: public Command {

	public:

		/*
		 * Command parameters: [none]
		 */
		TTOffload(unsigned int timeout_ms, ttOffloadParams params, string progressVar = ""): 
            Command(AdSecOpCodes::TT_OFFLOAD_CMD, "TTOffload", timeout_ms, progressVar) { _params = params;}

		virtual ~TTOffload() {}

      ttOffloadParams getParams() { return _params; }

		void log() const;

	protected:

		/*
		 * Default constructor needed for boost serialization.
		 */
		 TTOffload() {}

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
       ttOffloadParams _params;

};

/* L.F. removed
class SlToAo: public SetMirror {

	public:

		 // Command parameters: [none]
		SlToAo(unsigned int timeout_ms, flatParams params):
			   SetMirror(timeout_ms, AdSecOpCodes::SL_TO_AO_CMD, "Sl to Ao", params) {}

		virtual ~SlToAo() {}

	protected:

		 // Default constructor needed for boost serialization.
		 SlToAo() {}

	private:

		bool validateImpl() throw(CommandValidationException);

		friend class boost::serialization::access;
	    template<class Archive>
		void serialize(Archive& ar, const unsigned int version) {
			// Serialize base class object
			ar & boost::serialization::base_object<SetMirror>(*this);
		}

};


class SlToChop: public SetMirror {

	public:

		 // Command parameters: [none]
		SlToChop(unsigned int timeout_ms, flatParams params):
				SetMirror(timeout_ms, AdSecOpCodes::SL_TO_CHOP_CMD, "Sl to Chop", params) {}

		virtual ~SlToChop() {}

	protected:

		 // Default constructor needed for boost serialization.
		 SlToChop() {}

	private:

		bool validateImpl() throw(CommandValidationException);

		friend class boost::serialization::access;
	    template<class Archive>
		void serialize(Archive& ar, const unsigned int version) {
			// Serialize base class object
			ar & boost::serialization::base_object<SetMirror>(*this);
		}

};
  end of remove */

class StopAo: public Command {

	public:

		/*
		 * Command parameters: [none]
		 */
		StopAo(unsigned int timeout_ms, stopAoParams params, string progressVar = ""): 
            Command(AdSecOpCodes::STOP_AO_CMD, "Stop Ao", timeout_ms, progressVar) { _params = params; }

		virtual ~StopAo() {}

        stopAoParams getParams() { return _params; }
		void log() const;

	protected:

		/*
		 * Default constructor needed for boost serialization.
		 */
		 StopAo() {}

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
        stopAoParams _params;

};


class StopChop: public Command {

	public:

		/*
		 * Command parameters: [none]
		 */
		StopChop(unsigned int timeout_ms, string progressVar = ""): 
            Command(AdSecOpCodes::STOP_CHOP_CMD, "Stop Chop", timeout_ms, progressVar) {}

		virtual ~StopChop() {}

	protected:

		/*
		 * Default constructor needed for boost serialization.
		 */
		 StopChop() {}

	private:

		bool validateImpl() throw(CommandValidationException);

		friend class boost::serialization::access;
	    template<class Archive>
		void serialize(Archive& ar, const unsigned int /*version*/) {
			// Serialize base class object
			ar & boost::serialization::base_object<Command>(*this);
		}

};

class RecoverFailure: public Command {

	public:

		/*
		 * Command parameters: [none]
		 */
		RecoverFailure(unsigned int timeout_ms, string progressVar = ""): 
            Command(BasicOpCodes::RECOVER_FAILURE, "Recover failure", timeout_ms, progressVar) {}

		virtual ~RecoverFailure() {}

	protected:

		/*
		 * Default constructor needed for boost serialization.
		 */
		 RecoverFailure() {}

	private:

		bool validateImpl() throw(CommandValidationException);

		friend class boost::serialization::access;
	    template<class Archive>
		void serialize(Archive& ar, const unsigned int /*version*/) {
			// Serialize base class object
			ar & boost::serialization::base_object<Command>(*this);
		}

};

/* L.F. removed
class SetFlatFF: public Command {

	public:

		 // Command parameters: [none]
		SetFlatFF(unsigned int timeout_ms): Command(AdSecOpCodes::SET_FLAT_FF_CMD, "Set Flat FF", timeout_ms) {}

		virtual ~SetFlatFF() {}

	protected:

		 // Default constructor needed for boost serialization.
		 SetFlatFF() {}

	private:

		bool validateImpl() throw(CommandValidationException);

		friend class boost::serialization::access;
	    template<class Archive>
		void serialize(Archive& ar, const unsigned int version) {
			// Serialize base class object
			ar & boost::serialization::base_object<Command>(*this);
		}

};

class IfAcquire: public Command {

	public:

		 // Command parameters: [none]
		IfAcquire(unsigned int timeout_ms): Command(AdSecOpCodes::IF_ACQUIRE_CMD, "IF Acquire", timeout_ms) {}

		virtual ~IfAcquire() {}

	protected:

		 // Default constructor needed for boost serialization.
		 IfAcquire() {}

	private:

		bool validateImpl() throw(CommandValidationException);

		friend class boost::serialization::access;
	    template<class Archive>
		void serialize(Archive& ar, const unsigned int version) {
			// Serialize base class object
			ar & boost::serialization::base_object<Command>(*this);
		}

};

class IfReduce: public Command {

	public:

		 // Command parameters: [none]
		IfReduce(unsigned int timeout_ms): Command(AdSecOpCodes::IF_REDUCE_CMD, "IF Reduce", timeout_ms) {}

		virtual ~IfReduce() {}

	protected:

		 // Default constructor needed for boost serialization.
		 IfReduce() {}

	private:

		bool validateImpl() throw(CommandValidationException);

		friend class boost::serialization::access;
	    template<class Archive>
		void serialize(Archive& ar, const unsigned int version) {
			// Serialize base class object
			ar & boost::serialization::base_object<Command>(*this);
		}

};

class DustTest: public Command {

	public:

		 // Command parameters: [none]
		DustTest(unsigned int timeout_ms): Command(AdSecOpCodes::DUST_TEST_CMD, "Dust test", timeout_ms) {}

		virtual ~DustTest() {}

	protected:

		 // Default constructor needed for boost serialization.
		 DustTest() {}

	private:

		bool validateImpl() throw(CommandValidationException);

		friend class boost::serialization::access;
	    template<class Archive>
		void serialize(Archive& ar, const unsigned int version) {
			// Serialize base class object
			ar & boost::serialization::base_object<Command>(*this);
		}

};

class CoilTest: public Command {

	public:

		 // Command parameters: [none]
		CoilTest(unsigned int timeout_ms): Command(AdSecOpCodes::COIL_TEST_CMD, "Coil test", timeout_ms) {}

		virtual ~CoilTest() {}

	protected:

		 // Default constructor needed for boost serialization.
		 CoilTest() {}

	private:

		bool validateImpl() throw(CommandValidationException);

		friend class boost::serialization::access;
	    template<class Archive>
		void serialize(Archive& ar, const unsigned int version) {
			// Serialize base class object
			ar & boost::serialization::base_object<Command>(*this);
		}

};

class CapsensTest: public Command {

	public:

		 // Command parameters: [none]
		CapsensTest(unsigned int timeout_ms): Command(AdSecOpCodes::CAPSENS_TEST_CMD, "Capsens test", timeout_ms) {}

		virtual ~CapsensTest() {}

	protected:

		 // Default constructor needed for boost serialization.
		 CapsensTest() {}

	private:

		bool validateImpl() throw(CommandValidationException);

		friend class boost::serialization::access;
	    template<class Archive>
		void serialize(Archive& ar, const unsigned int version) {
			// Serialize base class object
			ar & boost::serialization::base_object<Command>(*this);
		}

};

class FFAcquire: public Command {

	public:

		 // Command parameters: [none]
		FFAcquire(unsigned int timeout_ms): Command(AdSecOpCodes::FF_ACQUIRE_CMD, "FF Acquire", timeout_ms) {}

		virtual ~FFAcquire() {}

	protected:

		 // Default constructor needed for boost serialization.
		 FFAcquire() {}

	private:

		bool validateImpl() throw(CommandValidationException);

		friend class boost::serialization::access;
	    template<class Archive>
		void serialize(Archive& ar, const unsigned int version) {
			// Serialize base class object
			ar & boost::serialization::base_object<Command>(*this);
		}

};

class FFReduce: public Command {

	public:

		 // Command parameters: [none]
		FFReduce(unsigned int timeout_ms): Command(AdSecOpCodes::FF_REDUCE_CMD, "FF Reduce", timeout_ms) {}

		virtual ~FFReduce() {}

	protected:

		 // Default constructor needed for boost serialization.
		 FFReduce() {}

	private:

		bool validateImpl() throw(CommandValidationException);

		friend class boost::serialization::access;
	    template<class Archive>
		void serialize(Archive& ar, const unsigned int version) {
			// Serialize base class object
			ar & boost::serialization::base_object<Command>(*this);
		}

};

class WdTest: public Command {

	public:

		 // Command parameters: [none]
		WdTest(unsigned int timeout_ms): Command(AdSecOpCodes::WD_TEST_CMD, "WD Test", timeout_ms) {}

		virtual ~WdTest() {}

	protected:

		 // Default constructor needed for boost serialization.
		 WdTest() {}

	private:

		bool validateImpl() throw(CommandValidationException);

		friend class boost::serialization::access;
	    template<class Archive>
		void serialize(Archive& ar, const unsigned int version) {
			// Serialize base class object
			ar & boost::serialization::base_object<Command>(*this);
		}

};

class FlTest: public Command {

	public:

		 // Command parameters: [none]
		FlTest(unsigned int timeout_ms): Command(AdSecOpCodes::FL_TEST_CMD, "Fl Test", timeout_ms) {}

		virtual ~FlTest() {}

	protected:

		 // Default constructor needed for boost serialization.
		 FlTest() {}

	private:

		bool validateImpl() throw(CommandValidationException);

		friend class boost::serialization::access;
	    template<class Archive>
		void serialize(Archive& ar, const unsigned int version) {
			// Serialize base class object
			ar & boost::serialization::base_object<Command>(*this);
		}

};

class PsTest: public Command {

	public:

		 // Command parameters: [none]
		PsTest(unsigned int timeout_ms): Command(AdSecOpCodes::PS_TEST_CMD, "Ps Test", timeout_ms) {}

		virtual ~PsTest() {}

	protected:

		 // Default constructor needed for boost serialization.
		 PsTest() {}

	private:

		bool validateImpl() throw(CommandValidationException);

		friend class boost::serialization::access;
	    template<class Archive>
		void serialize(Archive& ar, const unsigned int version) {
			// Serialize base class object
			ar & boost::serialization::base_object<Command>(*this);
		}

};
  end of remove */

class CalibrateCmd: public Command {
  public:
    /*
     * Command parameters: [calibration procedure name]
     */
    CalibrateCmd(unsigned int timeout_ms, calibrateParams params, string progressVar = ""): 
        Command(AdSecOpCodes::CALIBRATE_CMD, "Calibrate", timeout_ms, progressVar), _params(params) { }

    ~CalibrateCmd() {}

    calibrateParams getParams() { return _params; }
    
  protected:
    CalibrateCmd() {}

  private:
    bool validateImpl() throw(CommandValidationException) { return true; };
    friend class boost::serialization::access;
    template<class Archive>
    void serialize(Archive& ar, const unsigned int ) {
        ar & boost::serialization::base_object<Command>(*this);
        ar & _params;
    }
    
  protected:
    calibrateParams _params;
};



}
}

#endif //ADSEC_COMMANDS_H_INCLUDE
