#include "TimeToDie.h"
 
bool* TimeToDie::_timeToDie = NULL;

bool TimeToDie::check() throw (MirrorControllerShutdownException) { 
	if(*_timeToDie) {
		throw MirrorControllerShutdownException();
	} 
	else {
		return (*_timeToDie); 
	}
}
