
#include "StatusTable.h"

StatusTable::StatusTable(string name, uint8 size, uint8 expectedFrames) {
	
	_logger = Logger::get(name);
	_logger->log(Logger::LOG_LEV_INFO, "> Constructing StatusTable %s of size %d, expectedFrames %d...", name.c_str(), size, expectedFrames);
	
	_name = name;
	_size = size;
	
	pthread_mutex_init(&_mutexUpdating, NULL);
	pthread_mutex_init(&_mutexFrameReady, NULL);
	pthread_cond_init(&_condFrameReady, NULL);
	
	// Init expectedFrames value and entries
	_entryReady = -1;
	_entries = new StatusEntry[_size];
	for(int i=0; i<_size; i++) {
		_entries[i] = StatusEntry(i, expectedFrames);
	}
}

StatusTable::~StatusTable() {
	delete [] _entries;
}
