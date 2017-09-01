
extern "C"{
	#include "errlib.h"
	#include "errordb.h"
	#include "thrdlib.h"
}

#include "Logger.h"

using namespace Arcetri;

int main() {
	
	Logger* _logger = Logger::get(Logger::LOG_LEV_TRACE);
	
	char* MyName = "TestThLib";
	char* server_ip  = "127.0.0.1";
	
	int ErrC = 0;
	
	
	// First try !!!
	
	ErrC = thInit(MyName);
    if (IS_ERROR(ErrC)){ 
        _logger->log(Logger::LOG_LEV_WARNING, "thInit failed: %s (errcode %d)",lao_strerror(ErrC), ErrC);
        return 0;
    }
    _logger->log(Logger::LOG_LEV_INFO, "thInit succesfully completed");
    
    ErrC = thStart(server_ip, 5);
    if (IS_ERROR(ErrC)){ 
        _logger->log(Logger::LOG_LEV_WARNING, "thStart failed: %s (errcode %d)");
        return 0;
    }
    _logger->log(Logger::LOG_LEV_INFO, "thInit succesfully completed");
    
    ErrC = thClose();
    if (IS_ERROR(ErrC)) {
    	_logger->log(Logger::LOG_LEV_WARNING, "thClose failed: %s (errcode %d)",lao_strerror(ErrC), ErrC);
        return 0;
    }
    _logger->log(Logger::LOG_LEV_INFO ,"thClose succesfully completed");
    
    
    // Second try !!!
    
    ErrC = thInit(MyName);
    if (IS_ERROR(ErrC)){ 
        _logger->log(Logger::LOG_LEV_WARNING, "thInit failed: %s (errcode %d)",lao_strerror(ErrC), ErrC);
        return 0;
    }
    _logger->log(Logger::LOG_LEV_INFO, "thInit succesfully completed");
    
    ErrC = thStart(server_ip, 5);
    if (IS_ERROR(ErrC)){ 
        _logger->log(Logger::LOG_LEV_WARNING, "thStart failed: %s (errcode %d)");
        return 0;
    }
    _logger->log(Logger::LOG_LEV_INFO, "thInit succesfully completed");
    
    ErrC = thClose();
    if (IS_ERROR(ErrC)) {
    	_logger->log(Logger::LOG_LEV_WARNING, "thClose failed: %s (errcode %d)",lao_strerror(ErrC), ErrC);
        return 0;
    }
    _logger->log(Logger::LOG_LEV_INFO ,"thClose succesfully completed");
}