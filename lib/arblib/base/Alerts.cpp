#include "arblib/base/Alerts.h"
#include "base/msgcodes.h"

using namespace Arcetri::Arbitrator;

Alert::Alert(OpCode code, string shortDescription, string textMessage): SerializableMessage(ARB_ALERT, code, false, 0) {

	setShortDescription(shortDescription);
	setTextMessage(textMessage);
}

Alert::~Alert() {

}

void Alert::log() const {
	SerializableMessage::log();
	_logger->log(Logger::LOG_LEV_TRACE, " >> Alert: shortDescr=%s, textMsg=%s", getShortDescription().c_str(), getTextMessage().c_str());
}
