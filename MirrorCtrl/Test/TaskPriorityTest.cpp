#include "Logger.h"
using namespace Arcetri;

#include <unistd.h>
#include <sys/time.h>
#include <sys/resource.h>

int main() {
	
	Logger* logger = Logger::get(Logger::LOG_LEV_INFO);
	
	int ret;
	struct sched_param param;
    param.sched_priority = 99;
    if((ret = sched_setscheduler(0, SCHED_RR, &param)) != 0) {
        logger->log(Logger::LOG_LEV_FATAL, "Real-time priority not set !");
    }
    else {
        logger->log(Logger::LOG_LEV_INFO, "Real-time priority succesfully set !");
    }
    logger->log(Logger::LOG_LEV_INFO, "Min priority: %d", sched_get_priority_min(SCHED_RR));
  	logger->log(Logger::LOG_LEV_INFO, "Max priority: %d", sched_get_priority_max(SCHED_RR));
  	
  	struct sched_param my_sched_param;
  	sched_getparam(0, &my_sched_param);
  	logger->log(Logger::LOG_LEV_INFO, "My priority: %d", my_sched_param.sched_priority);
 
 	
 
}
