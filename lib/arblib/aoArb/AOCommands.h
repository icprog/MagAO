/*
 * This file store all commands to simplify the use by AOS.
 *
 * ATTENTION: always include "AOCommandsExport.h", not this file,
 * to use commands with an arbitrator interface
 */

#ifndef AOCOMMANDS_H_INCLUDED
#define AOCOMMANDS_H_INCLUDED

#include "arblib/base/Command.h"
#include "arblib/aoArb/AOOpCodes.h"
#include "arblib/aoArb/AOPTypes.h"
#include "arblib/base/Commands.h"

using namespace Arcetri::Arbitrator;

// ---------------- TYPES FOR ALL COMMAND'S PARAMETERS ------------- //
// ----------------------------------------------------------------- //

#include <vector>
#include <boost/serialization/vector.hpp>

using namespace std;

namespace Arcetri {
namespace Arbitrator {


// ------------------------------------- REQUESTS ------------------------------ //
// ----------------------------------------------------------------------------- //
// Handled in a concurrent way with respect to commands

class AdjustGain: public Command {

	public:

		AdjustGain(unsigned int timeout_ms, adjustGainParams params, string progressVar = ""): 
            Command(AOOpCodes::ADJUST_GAIN, "AdjustGain", timeout_ms, progressVar) { _params = params;  }
		void log() const;

		virtual ~AdjustGain() {}

      adjustGainParams getParams() { return _params; }


	protected:

		/*
		 * Default constructor needed for boost serialization.
		 */
		AdjustGain() {}

	private:

		bool validateImpl() throw(CommandValidationException);

		friend class boost::serialization::access;
	    template<class Archive>
		void serialize(Archive& ar, const unsigned int /*version*/) {
			// Serialize base class object
			ar & boost::serialization::base_object<Command>(*this);
         ar & _params;
		}

    adjustGainParams _params;
};


class AdjustIntTime: public Command {

	public:

		AdjustIntTime(unsigned int timeout_ms, adjustIntTimeParams params, string progressVar = ""): 
            Command(AOOpCodes::ADJUST_INT_TIME, "AdjustIntTime", timeout_ms, progressVar) { _params = params;  }
		void log() const;

		virtual ~AdjustIntTime() {}

      adjustIntTimeParams getParams() { return _params; }


	protected:

		/*
		 * Default constructor needed for boost serialization.
		 */
		AdjustIntTime() {}

	private:

		bool validateImpl() throw(CommandValidationException);

		friend class boost::serialization::access;
	    template<class Archive>
		void serialize(Archive& ar, const unsigned int /*version*/) {
			// Serialize base class object
			ar & boost::serialization::base_object<Command>(*this);
         ar & _params;
		}

    adjustIntTimeParams _params;
};


// ------------------------------- INITIAL COMMAND'S --------------------------- //
// ----------------------------------------------------------------------------- //


class PowerOnWfs: public Command {

	public:

		PowerOnWfs(unsigned int timeout_ms, wfsOnOffParams params, string progressVar = ""): 
            Command(AOOpCodes::POWER_ON_WFS, "Power on wfs", timeout_ms, progressVar) { _params = params; }

		virtual ~PowerOnWfs() {}

		void log() const;

      wfsOnOffParams getParams() { return _params; }

	protected:

		/*
		 * Default constructor needed for boost serialization.
		 */
		PowerOnWfs() {}

	private:

		bool validateImpl() throw(CommandValidationException);

		friend class boost::serialization::access;
	    template<class Archive>
		void serialize(Archive& ar, const unsigned int /*version*/) {
			// Serialize base class object
			ar & boost::serialization::base_object<Command>(*this);
			ar & _params;
		}

	private: // COMMAND PARAMETERS

       wfsOnOffParams _params;
};

class PowerOffWfs: public Command {

	public:

		PowerOffWfs(unsigned int timeout_ms, wfsOnOffParams params, string progressVar = ""): 
            Command(AOOpCodes::POWER_OFF_WFS, "Power off wfs", timeout_ms, progressVar) { _params = params; }

		virtual ~PowerOffWfs() {}

		void log() const;

       wfsOnOffParams getParams() { return _params; }

	protected:

		/*
		 * Default constructor needed for boost serialization.
		 */
		PowerOffWfs() {}

	private:

		bool validateImpl() throw(CommandValidationException);

		friend class boost::serialization::access;
      template<class Archive>
		void serialize(Archive& ar, const unsigned int /*version*/) {
			// Serialize base class object
			ar & boost::serialization::base_object<Command>(*this);
         ar & _params;
		}
	private: // COMMAND PARAMETERS

       wfsOnOffParams _params;

};

class PowerOnAdSec: public Command {

	public:

		PowerOnAdSec(unsigned int timeout_ms, string progressVar = ""): 
            Command(AOOpCodes::POWER_ON_ADSEC, "Power on adsec", timeout_ms, progressVar) { }

		virtual ~PowerOnAdSec() {}

	protected:

		/*
		 * Default constructor needed for boost serialization.
		 */
		PowerOnAdSec() {}

	private:

		bool validateImpl() throw(CommandValidationException);

		friend class boost::serialization::access;
	    template<class Archive>
		void serialize(Archive& ar, const unsigned int /*version*/) {
			// Serialize base class object
			ar & boost::serialization::base_object<Command>(*this);
		}

};

class PowerOffAdSec: public Command {

	public:

		PowerOffAdSec(unsigned int timeout_ms, string progressVar = ""): 
            Command(AOOpCodes::POWER_OFF_ADSEC, "Power off adsec", timeout_ms, progressVar) { }

		virtual ~PowerOffAdSec() {}

	protected:

		/*
		 * Default constructor needed for boost serialization.
		 */
		PowerOffAdSec() {}

	private:

		bool validateImpl() throw(CommandValidationException);

		friend class boost::serialization::access;
	    template<class Archive>
		void serialize(Archive& ar, const unsigned int /*version*/) {
			// Serialize base class object
			ar & boost::serialization::base_object<Command>(*this);
		}

};

class MirrorSet: public Command {

	public:

		MirrorSet(unsigned int timeout_ms, string progressVar = ""): 
            Command(AOOpCodes::MIRROR_SET, "MirrorSet", timeout_ms, progressVar) { }

		virtual ~MirrorSet() {}

	protected:

		/*
		 * Default constructor needed for boost serialization.
		 */
		MirrorSet() {}

	private:

		bool validateImpl() throw(CommandValidationException);

		friend class boost::serialization::access;
	    template<class Archive>
		void serialize(Archive& ar, const unsigned int /*version*/) {
			// Serialize base class object
			ar & boost::serialization::base_object<Command>(*this);
		}

};


class MirrorRest: public Command {
	public:
		MirrorRest(unsigned int timeout_ms, string progressVar = ""): 
            Command(AOOpCodes::MIRROR_REST, "MirrorRest", timeout_ms, progressVar) { }
		virtual ~MirrorRest() {}
	protected:
		/*
		 * Default constructor needed for boost serialization.
		 */
		MirrorRest() {}
	private:
		bool validateImpl() throw(CommandValidationException);
		friend class boost::serialization::access;
	    template<class Archive>
		void serialize(Archive& ar, const unsigned int /*version*/) {
			// Serialize base class object
			ar & boost::serialization::base_object<Command>(*this);
		}

};

class FaultRecovery: public Command {
	public:
		FaultRecovery(unsigned int timeout_ms, string progressVar = ""): 
            Command(AOOpCodes::FAULT_RECOVERY, "FaultRecovery", timeout_ms, progressVar) { }
		virtual ~FaultRecovery() {}
	protected:
		/*
		 * Default constructor needed for boost serialization.
		 */
		FaultRecovery() {}
	private:
		bool validateImpl() throw(CommandValidationException);
		friend class boost::serialization::access;
	    template<class Archive>
		void serialize(Archive& ar, const unsigned int /*version*/) {
			// Serialize base class object
			ar & boost::serialization::base_object<Command>(*this);
		}
};


class StandaloneFail: public Command {
	public:
		StandaloneFail(unsigned int timeout_ms, string progressVar = ""): 
            Command(AOOpCodes::STANDALONE_FAIL, "StandaloneFail", timeout_ms, progressVar) { }
		virtual ~StandaloneFail() {}
	protected:
		/*
		 * Default constructor needed for boost serialization.
		 */
		StandaloneFail() {}
	private:
		bool validateImpl() throw(CommandValidationException);
		friend class boost::serialization::access;
	    template<class Archive>
		void serialize(Archive& ar, const unsigned int /*version*/) {
			// Serialize base class object
			ar & boost::serialization::base_object<Command>(*this);
		}
};


class SeeingLmtdFail: public Command {
	public:
		SeeingLmtdFail(unsigned int timeout_ms, string progressVar = ""): 
            Command(AOOpCodes::SEEING_LMTD_FAIL, "SeeingLmtdFail", timeout_ms, progressVar) { }
		virtual ~SeeingLmtdFail() {}
	protected:
		/*
		 * Default constructor needed for boost serialization.
		 */
		SeeingLmtdFail() {}
	private:
		bool validateImpl() throw(CommandValidationException);
		friend class boost::serialization::access;
	    template<class Archive>
		void serialize(Archive& ar, const unsigned int /*version*/) {
			// Serialize base class object
			ar & boost::serialization::base_object<Command>(*this);
		}
};




class SetAOMode: public Command {

	public:

		SetAOMode(unsigned int timeout_ms, setAOModeParams params, string progressVar = ""): 
            Command(AOOpCodes::SET_AO_MODE, "Set AO Mode", timeout_ms, progressVar) { _params = params; }

		virtual ~SetAOMode() {}

		setAOModeParams getParams() { return _params; }

	protected:

		/*
		 * Default constructor needed for boost serialization.
		 */
		SetAOMode() {}

	private:

		bool validateImpl() throw(CommandValidationException);

		friend class boost::serialization::access;
	    template<class Archive>
		void serialize(Archive& ar, const unsigned int /*version*/) {
			// Serialize base class object
			ar & boost::serialization::base_object<Command>(*this);
			ar & _params;
		}

	   setAOModeParams _params;
};




// ----------------------------------- COMMANDS ------------------------------- //
// ----------------------------------------------------------------------------- //

class PresetFlat: public Command {

	public:

		PresetFlat(unsigned int timeout_ms, presetFlatParams params, string progressVar = "") : 
            Command(AOOpCodes::PRESET_FLAT, "PresetFlat", timeout_ms, progressVar) { _params = params; }


		virtual ~PresetFlat() {}

		void log() const;


		// ACCESSORS FOR INTERFACE CLIENT //
		presetFlatParams getParams() { return _params; }
      presetFlatParams getResult() { return _result; }

      void setResult( presetFlatParams result) { _result = result; }


	protected:

		/*
		 * Default constructor.
		 * Needed for boost serialization.
		 */
		PresetFlat() {}

	private:

		bool validateImpl() throw(CommandValidationException);

		friend class boost::serialization::access;
	    template<class Archive>
		void serialize(Archive& ar, const unsigned int /*version*/) {
			// Serialize base class object
			ar & boost::serialization::base_object<Command>(*this);
			ar & _params;
         ar & _result;
		}

	private: // COMMAND PARAMETERS

		presetFlatParams _params;
      presetFlatParams _result;

};


class PresetAO: public Command {

	public:

		PresetAO(unsigned int timeout_ms, presetAOParams params, string progressVar = "") : 
            Command(AOOpCodes::PRESET_AO, "PresetAO", timeout_ms, progressVar) { _params = params; }

		virtual ~PresetAO() {}

		void log() const;

		// ACCESSORS FOR INTERFACE CLIENT //
      presetAOParams getParams() { return _params; }

      acquireRefAOResult getResult() { return _result; }
      void setResult( acquireRefAOResult result) { _result = result; }

	protected:

		/*
		 * Default constructor needed for boost serialization.
		 */
		PresetAO() {}

	private:

		bool validateImpl() throw(CommandValidationException);

		friend class boost::serialization::access;
	    template<class Archive>
		void serialize(Archive& ar, const unsigned int /*version*/) {
			// Serialize base class object
			ar & boost::serialization::base_object<Command>(*this);
         ar & _params;
         ar & _result;
		}

	private: // COMMAND PARAMETERS

      presetAOParams _params;
      acquireRefAOResult _result;

};


class AcquireRefAO: public Command {

	public:

        AcquireRefAO(unsigned int timeout_ms, string progressVar = "") : 
            Command(AOOpCodes::ACQUIREREF_AO, "AcquireRefAO", timeout_ms, progressVar) {}


		virtual ~AcquireRefAO() {}

		void log() const;

      acquireRefAOResult getResult() { return _result; }
      void setResult( acquireRefAOResult result) { _result = result; }

	protected:

		/*
		 * Default constructor needed for boost serialization.
		 */
		AcquireRefAO() {}

	private:

		bool validateImpl() throw(CommandValidationException);

		friend class boost::serialization::access;
	    template<class Archive>
		void serialize(Archive& ar, const unsigned int /*version*/) {
			// Serialize base class object
			ar & boost::serialization::base_object<Command>(*this);
         ar & _result;
		}

	private: // COMMAND PARAMETERS

		// Output parameter
     acquireRefAOResult _result;
};


class CheckRefAO: public Command {

	public:

		CheckRefAO(unsigned int timeout_ms, string progressVar = "")  : 
            Command(AOOpCodes::CHECKREF_AO, "CheckRefAO", timeout_ms, progressVar) { }

		virtual ~CheckRefAO() {}

		void log() const;

      void setResult( checkRefAOResult result) { _result = result; }
      checkRefAOResult getResult() { return _result; }

	protected:

		/*
		 * Default constructor needed for boost serialization.
		 */
		 CheckRefAO() {}

	private:

		bool validateImpl() throw(CommandValidationException);

		friend class boost::serialization::access;
	    template<class Archive>
		void serialize(Archive& ar, const unsigned int /*version*/) {
			// Serialize base class object
			ar & boost::serialization::base_object<Command>(*this);
         ar & _result;
		}


	private:

   checkRefAOResult _result;

};


class CorrectModes: public Command {

	public:

		CorrectModes(unsigned int timeout_ms, correctModesParams params, string progressVar = "") : 
            Command(AOOpCodes::CORRECTMODES, "CorrectModes", timeout_ms, progressVar) { _params = params; }

		virtual ~CorrectModes() {}

		void log() const;

      correctModesParams getParams() { return _params; }

	protected:

		/*
		 * Default constructor.
		 * Needed for boost serialization.
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

	private: // COMMAND PARAMETERS

      correctModesParams _params;

};


class SetZernikes: public Command {

	public:

		SetZernikes(unsigned int timeout_ms, setZernikesParams params, string progressVar = "") : 
            Command(AOOpCodes::SETZERNIKES, "SetZernikes", timeout_ms, progressVar) { _params = params; }

		virtual ~SetZernikes() {}

		void log() const;

      setZernikesParams getParams() { return _params; }

	protected:

		/*
		 * Default constructor.
		 * Needed for boost serialization.
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

	private: // COMMAND PARAMETERS

      setZernikesParams _params;

};


class ModifyAO: public Command {

	public:

      ModifyAO(unsigned int timeout_ms, modifyAOParams params, string progressVar = "")  : 
          Command(AOOpCodes::MODIFY_AO, "ModifyAO", timeout_ms, progressVar) { _params = params; }


		virtual ~ModifyAO() {}

		void log() const;

      modifyAOParams getParams() { return _params; }
      acquireRefAOResult getResult() { return _result; }

      void setResult( acquireRefAOResult result) { _result = result; }

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
         ar & _result;
		}

	private: // COMMAND PARAMETERS

       modifyAOParams _params;
       acquireRefAOResult _result;

};


class OffsetXY: public Command {

	public:

		OffsetXY(unsigned int timeout_ms, offsetXYParams params, string progressVar = "")  : 
            Command(AOOpCodes::OFFSETXY, "OffsetXY", timeout_ms, progressVar) { _params = params; } 

		virtual ~OffsetXY() {}

		void log() const;

      offsetXYParams getParams() {  return _params; }


	protected:

		/*
		 * Default constructor.
		 * Needed for boost serialization.
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

	private: // COMMAND PARAMETERS

      offsetXYParams _params;

};


class OffsetZ: public Command {

	public:
		 
      OffsetZ(unsigned int timeout_ms, offsetZParams params, string progressVar = "")  : 
          Command(AOOpCodes::OFFSETZ, "OffsetZ", timeout_ms, progressVar) { _params = params; }

		virtual ~OffsetZ() {}

		void log() const;

      offsetZParams getParams() { return _params; }

	protected:

		/*
		 * Default constructor.
		 * Needed for boost serialization.
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

	private: // COMMAND PARAMETERS

      offsetZParams _params;
};


class Pause: public Command {

	public:

		Pause(unsigned int timeout_ms, string progressVar = ""): 
            Command(AOOpCodes::PAUSE, "Pause", timeout_ms, progressVar) {}

		virtual ~Pause() {}

	protected:

		/*
		 * Default constructor needed for boost serialization.
		 */
		 Pause() {}

	private:

		bool validateImpl() throw(CommandValidationException);

		friend class boost::serialization::access;
	    template<class Archive>
		void serialize(Archive& ar, const unsigned int /*version*/) {
			// Serialize base class object
			ar & boost::serialization::base_object<Command>(*this);
		}

};


class RefineAO: public Command {

	public:

		RefineAO(unsigned int timeout_ms, refineAOParams params, string progressVar = "") : 
            Command(AOOpCodes::REFINE_AO, "RefineAO", timeout_ms, progressVar) { _params = params; }

		virtual ~RefineAO() {}

		void log() const;

      refineAOParams getParams() { return _params; }
      acquireRefAOResult getResult() { return _result; }

      void setResult( acquireRefAOResult result) { _result = result; }

	protected:

		/*
		 * Default constructor.
		 * Needed for boost serialization.
		 */
		RefineAO() {}

	private:

		bool validateImpl() throw(CommandValidationException);

		friend class boost::serialization::access;
	    template<class Archive>
		void serialize(Archive& ar, const unsigned int /*version*/) {
			// Serialize base class object
			ar & boost::serialization::base_object<Command>(*this);
			ar & _params;
         ar & _result;
		}

	private: // COMMAND PARAMETERS

		refineAOParams _params;
      acquireRefAOResult _result;

};


class Resume: public Command {

	public:

		Resume(unsigned int timeout_ms, string progressVar = ""): 
            Command(AOOpCodes::RESUME, "Resume", timeout_ms, progressVar) {}

		virtual ~Resume() {}

	protected:

		/*
		 * Default constructor needed for boost serialization.
		 */
		 Resume() {}

	private:

		bool validateImpl() throw(CommandValidationException);

		friend class boost::serialization::access;
	    template<class Archive>
		void serialize(Archive& ar, const unsigned int /*version*/) {
			// Serialize base class object
			ar & boost::serialization::base_object<Command>(*this);
		}

};

class RecoverWfsFailure: public Command {

	public:

		RecoverWfsFailure(unsigned int timeout_ms, string progressVar = ""): 
            Command(AOOpCodes::RECOVER_WFS_FAILURE, "RecoverWfsFailure", timeout_ms, progressVar) {}

		virtual ~RecoverWfsFailure() {}

	protected:

		/*
		 * Default constructor needed for boost serialization.
		 */
		 RecoverWfsFailure() {}

	private:

		bool validateImpl() throw(CommandValidationException);

		friend class boost::serialization::access;
	    template<class Archive>
		void serialize(Archive& ar, const unsigned int /*version*/) {
			// Serialize base class object
			ar & boost::serialization::base_object<Command>(*this);
		}

};

class RecoverAdSecFailure: public Command {

	public:

		RecoverAdSecFailure(unsigned int timeout_ms, string progressVar = ""): 
            Command(AOOpCodes::RECOVER_ADSEC_FAILURE, "RecoverAdSecFailure", timeout_ms, progressVar) {}

		virtual ~RecoverAdSecFailure() {}

	protected:

		/*
		 * Default constructor needed for boost serialization.
		 */
		 RecoverAdSecFailure() {}

	private:

		bool validateImpl() throw(CommandValidationException);

		friend class boost::serialization::access;
	    template<class Archive>
		void serialize(Archive& ar, const unsigned int /*version*/) {
			// Serialize base class object
			ar & boost::serialization::base_object<Command>(*this);
		}

};


class StartAO: public Command {

	public:

		StartAO(unsigned int timeout_ms, string progressVar = ""): 
            Command(AOOpCodes::START_AO, "Start AO", timeout_ms, progressVar) {}

		virtual ~StartAO() {}

	protected:

		/*
		 * Default constructor needed for boost serialization.
		 */
		 StartAO() {}

	private:

		bool validateImpl() throw(CommandValidationException);

		friend class boost::serialization::access;
	    template<class Archive>
		void serialize(Archive& ar, const unsigned int /*version*/) {
			// Serialize base class object
			ar & boost::serialization::base_object<Command>(*this);
		}

};


class Stop: public Command {

	public:

		Stop(unsigned int timeout_ms, stopParams params, string progressVar = "") : 
            Command(AOOpCodes::STOP, "StopAO", timeout_ms, progressVar) { _params = params; }

		virtual ~Stop() {}

		void log() const;

      stopParams getParams() {  return _params; }

	protected:

		/*
		 * Default constructor.
		 * Needed for boost serialization.
		 */
		Stop() {}

	private:

		bool validateImpl() throw(CommandValidationException);

		friend class boost::serialization::access;
	    template<class Archive>
		void serialize(Archive& ar, const unsigned int /*version*/) {
			// Serialize base class object
			ar & boost::serialization::base_object<Command>(*this);
			ar & _params;
		}

	private: // COMMAND PARAMETERS

      stopParams _params;
};


class UserPanic: public Command {

	public:

		UserPanic(unsigned int timeout_ms, userPanicParams params, string progressVar = "") : 
            Command ( AOOpCodes::USER_PANIC, "UserPanic", timeout_ms, progressVar) { _params = params; }

		virtual ~UserPanic() {}

		void log() const;

      userPanicParams getParams() { return _params; }

	protected:

		/*
		 * Default constructor.
		 * Needed for boost serialization.
		 */
		UserPanic() {}

	private:

		bool validateImpl() throw(CommandValidationException);

		friend class boost::serialization::access;
	    template<class Archive>
		void serialize(Archive& ar, const unsigned int /*version*/) {
			// Serialize base class object
			ar & boost::serialization::base_object<Command>(*this);
			ar & _params;
		}

	private: // COMMAND PARAMETERS

       userPanicParams _params;

};

class RequestAOStatus: public RequestStatus {

   public:

      RequestAOStatus(unsigned int timeout_ms):
          RequestStatus(timeout_ms, AOOpCodes::REQUEST_AO_STATUS_CMD) {
             _wfsPower = false;
             _adsecPower = false;
             _adsecSet = false;
             _aoMode = "";
             _wfsStatus="";
             _adsecStatus="";
             _wfsClStatReady = false;
             _adsecClStatReady = false;
          }

      bool wfsPower() { return _wfsPower; }
      bool adsecPower() { return _adsecPower; }
      bool adsecSet() { return _adsecSet; }
      string aoMode() { return _aoMode; }
      string wfsStatus() { return _wfsStatus; }
      string adsecStatus() { return _adsecStatus; }
      bool wfsClStatReady() { return _wfsClStatReady; }
      bool adsecClStatReady() { return _adsecClStatReady; }

      void setWfsPower( bool power) { _wfsPower = power; }
      void setAdSecPower( bool power) { _adsecPower = power; }
      void setAdSecSet( bool set) { _adsecSet = set; }
      void setAOMode( string mode) { _aoMode = mode; }
      void setWfsStatus( string status) { _wfsStatus = status; }
      void setAdsecStatus( string status) { _adsecStatus = status; }
      void setWfsClStatReady( bool ready) { _wfsClStatReady = ready; }
      void setAdSecClStatReady( bool ready) { _adsecClStatReady = ready; }

      virtual ~RequestAOStatus() {}

   protected:

      /*
       * Default constructor needed for boost serialization.
       */
      RequestAOStatus() {}


   private:

      bool validateImpl() throw(CommandValidationException);

      bool _wfsPower;
      bool _adsecPower;
      bool _adsecSet;
      string _aoMode;
      string _wfsStatus;
      string _adsecStatus;
      bool _wfsClStatReady;
      bool _adsecClStatReady;

      friend class boost::serialization::access;
      template<class Archive>
      void serialize(Archive& ar, const unsigned int /*version*/) {
          // Serialize base class object
          ar & boost::serialization::base_object<RequestStatus>(*this);
          ar & _wfsPower;
          ar & _adsecPower;
          ar & _adsecSet;
          ar & _aoMode;
          ar & _wfsStatus;
          ar & _adsecStatus;
          ar & _wfsClStatReady;
          ar & _adsecClStatReady;
       }

};




}
}

#endif
