#include "mastdiagnlib.h"

using namespace Arcetri;

namespace Arcetri {

MastDiagnInterface::MastDiagnInterface(string masterdiagnosticidentity, int logLevel) {
	_mastDiagnMsgDIdnty = masterdiagnosticidentity;
	_logger = Logger::get("MASTDIAGN-INTF", logLevel);
	_logger->log(Logger::LOG_LEV_INFO, "Created interface to MasterDiagnostic %s", _mastDiagnMsgDIdnty.c_str());
}

MastDiagnInterface::~MastDiagnInterface() {

}

void MastDiagnInterface::sendMessage(int msgCode) throw (MastDiagnInterfaceException) {
	_logger->log(Logger::LOG_LEV_DEBUG, "Sending message %d to %s", msgCode, _mastDiagnMsgDIdnty.c_str());
	int seqNum = thSendMsg(0, (char*)_mastDiagnMsgDIdnty.c_str(), msgCode, 0, NULL);
	if (IS_ERROR(seqNum)) {
		int stat = PLAIN_ERROR(seqNum);
		_logger->log(Logger::LOG_LEV_ERROR, "Error in thSendMsg: %d (%s)", stat, lao_strerror(stat));
      	throw MastDiagnInterfaceException("Failed to send message");
	}
}

void MastDiagnInterface::dumpAdSecDiagnHistory() throw (MastDiagnInterfaceException) {
	_logger->log(Logger::LOG_LEV_INFO, "Requesting MASTDIAGN_DUMP_ADSEC_FRAMES to %s",_mastDiagnMsgDIdnty.c_str());
	sendMessage(MASTDIAGN_DUMP_ADSEC_FRAMES);
}

void MastDiagnInterface::dumpOptLoopDiagnHistory() throw (MastDiagnInterfaceException) {
	_logger->log(Logger::LOG_LEV_INFO, "Requesting MASTDIAGN_DUMP_OPTLOOP_FRAMES to %s",_mastDiagnMsgDIdnty.c_str());
	sendMessage(MASTDIAGN_DUMP_OPTLOOP_FRAMES);
}

}
