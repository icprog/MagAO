#include "adsecarbitrator/AdamSystem.h"
#include "adsecarbitrator/AdSecArbitrator.h"


using namespace Arcetri::AdSec_Arbitrator;

//
//
AdamSystem::AdamSystem(bool simulation): AbstractSystem(ADAMSYSTEM_NAME),
                                         _simulation(simulation),
                                         _adam(NULL)
{
    try {
        if (!simulation) _adam = new AdamModbus("ADAM-MODBUS", false);
    } catch (AOException& e) {
        _logger->log(Logger::LOG_LEV_FATAL, "AdamSystem connection to ADAM failed %s", e.what().c_str());
        throw(e);
    }
    _arbitrator = AbstractArbitrator::getInstance();

}

//
//
AdamSystem::~AdamSystem() {
	
}


//
//
bool AdamSystem::disableCoils() 
{
    if (_simulation) {
        ((AdSecArbitrator *)_arbitrator)->setCoilsEnabled(false);
        return true;
    }

    if (_adam != NULL) {
        bool ret= _adam->disableCoils();
        if (ret) ((AdSecArbitrator *)_arbitrator)->setCoilsEnabled(false);
        return ret;
    }
    
    return false;
}

//
//
bool AdamSystem::enableCoils()
{
    if (_simulation) {
        ((AdSecArbitrator *)_arbitrator)->setCoilsEnabled(true);
        return true;
    }

    if (_adam != NULL) {
        bool ret= _adam->enableCoils();
        if (ret) ((AdSecArbitrator *)_arbitrator)->setCoilsEnabled(true);
        return ret;
    }
    
    return false;
}

//
//
bool AdamSystem::disableTss()
{
    if (_simulation)
        return true;

    if (_adam != NULL)
        return _adam->disableTss();
    
    return false;
}

//
//
bool AdamSystem::enableTss()
{
    if (_simulation)
        return true;

    if (_adam != NULL)
        return _adam->enableTss();
    
    return false;
}
