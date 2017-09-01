#include "DiagnosticManagerPool.h"


DiagnosticManagerPool::DiagnosticManagerPool(int logLevel) {
	_logger = Logger::get("DIAGNMAN-POOL", logLevel);
}


DiagnosticManagerPool::~DiagnosticManagerPool() {
	for(unsigned int i=0; i<_diagnManagers.size(); i++) {
    	delete _diagnManagers[i];
    }
}


bool DiagnosticManagerPool::add(DiagnosticManager* dm) { 
	if(size() < _MAX_DIAGN_MAN_NUM) {
		_diagnManagers.push_back(dm); 
		return true;
	}
	else {
		return false;
	}
}	


void DiagnosticManagerPool::startDiagnosticManagerInternal(unsigned int index) throw (UnexistingDiagnosticManagerException) {
	// Check if the index is out of range (unexisting diagn manager)
	if(index >= size()) {
		throw UnexistingDiagnosticManagerException();
	}
	else {
		// Stop others diagnostic managers of the same group
		unsigned int groupId = _diagnManagers[index]->getGroupId();
		stopDiagnosticManagersByGroup(groupId);
		
		// Start the new one
		_logger->log(Logger::LOG_LEV_INFO, "Starting diagnostic manager [assigned id: %d] (SHM BUF: %s)...", index, _diagnManagers[index]->getShmBufName().c_str());
		_threads[index] = _diagnManagers[index]->start();
	}
}


unsigned int DiagnosticManagerPool::startDiagnosticManager(unsigned int groupId, unsigned int frameSizePx) throw (UnexistingDiagnosticManagerException, DiagnosticManagerPoolStartException) {
	_logger->log(Logger::LOG_LEV_INFO, "Start diagnostic manager group %d and frame size %d pixels (0 px = don't care)...", groupId, frameSizePx);
	_logger->log(Logger::LOG_LEV_INFO, "Searching diagnostic manager... found %d diagnostic managers !", size());
	
	// Search and start the diagnostic manager
	for(unsigned int id=0; id<size(); id++) {
		if( _diagnManagers[id]->getGroupId() == groupId && 
		    (frameSizePx == 0  || (frameSizePx != 0 && _diagnManagers[id]->getFrameSizePx() == frameSizePx))
		  ) {
		  	_logger->log(Logger::LOG_LEV_INFO, " -> SUCCESS: found diagnostic manager group %d and frame size %d px", _diagnManagers[id]->getGroupId(), _diagnManagers[id]->getFrameSizePx());
			startDiagnosticManagerInternal(id);
			return id;
		}	
		else {
			_logger->log(Logger::LOG_LEV_INFO, " -> FAIL: found diagnostic manager group %d and frame size %d px", _diagnManagers[id]->getGroupId(), _diagnManagers[id]->getFrameSizePx());
		}
	}
	// None started :-(
	throw DiagnosticManagerPoolStartException();
}

void DiagnosticManagerPool::stopDiagnosticManager(unsigned int id) {
	_logger->log(Logger::LOG_LEV_INFO, "Stopping diagnostic manager [id: %d] (SHM BUF: %s)...", id, _diagnManagers[id]->getShmBufName().c_str());
	_diagnManagers[id]->stop();
	// [todo] Must sync with the diagnostic manager to be sure it's stopped !!!
	// waitDiagnosticManager(id); // This isn't ok: if thread already terminated ?!
	msleep(2000);
	_logger->log(Logger::LOG_LEV_INFO, "Succesfully stopped diagnostic manager [id: %d] (SHM BUF: %s)", id, _diagnManagers[id]->getShmBufName().c_str());
}


void DiagnosticManagerPool::stopDiagnosticManagersByGroup(unsigned int groupId) {
	_logger->log(Logger::LOG_LEV_INFO, "Stopping diagnostic managers belonging to group %d...", groupId);
	for(unsigned int i=0; i<size(); i++) {
		if(_diagnManagers[i]->getGroupId() == groupId && _diagnManagers[i]->isStarted()) {
			stopDiagnosticManager(i);
		}
	}
}


// [todo] Not safe if thread with this id doesn't exists...
void DiagnosticManagerPool::waitDiagnosticManager(unsigned int id) {
	_logger->log(Logger::LOG_LEV_INFO, "Waiting diagnostic manager [id: %d] (SHM BUF: %s)...", id, _diagnManagers[id]->getShmBufName().c_str());
	pthread_join(_threads[id], NULL);
}

