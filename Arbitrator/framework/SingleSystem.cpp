#include "framework/SingleSystem.h"
#include "framework/AbstractArbitrator.h"

using namespace Arcetri::Arbitrator;

int SingleSystem::_counter=0;

SingleSystem::SingleSystem(string systemName) : AbstractSystem(systemName) {

   _counter++;
   if (_counter>1) {
      _logger->log( Logger::LOG_LEV_FATAL, "Single System %s: no more instances allowed.", systemName.c_str());
      throw AOException( "SingleSystem double instantiation", TBD_ERROR, __FILE__, __LINE__);
   }

}

SingleSystem::~SingleSystem() {
}
