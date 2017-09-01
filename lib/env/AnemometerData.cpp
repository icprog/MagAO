#include "env/AnemometerData.h"
#include <cstdio>

namespace Arcetri {

AnemometerData::exception::exception(const std::string &msg,const std::string &frame):
std::runtime_error(msg) {
 this->_frame = frame;
}

std::string AnemometerData::exception::frame() {
  return this->_frame;
}

AnemometerData::exception::~exception() throw() {
}

AnemometerData::status_error::status_error(int status):
std::runtime_error(AnemometerData::status_error::status_error_lookup(status)) {
}

const std::string AnemometerData::status_error::status_error_lookup(int status) {
  static const std::string statstr[] = {
	"Functioning correctly",
	"Insufficient samples in average period from transducer pair 1",
	"Insufficient samples in average period from transducer pair 2",
	"Insufficient samples in average period from transducer pair 3",
	"Insufficient samples in average period from transducer pairs 1,2",
	"Insufficient samples in average period from transducer pairs 1,3",
	"Insufficient samples in average period from transducer pairs 2,3",
	"Insufficient samples in average period from all transducer pairs",
	"NVM Checksum failed",
	"ROM checksum failed",
	"System gain at Maximum Results OK, but marginal operation",
	"Retries used",
	"Unknown status"
  };

  status = (status>0xC)?0xC:status;
  return statstr[status];
}

AnemometerData::AnemometerData() {
}

AnemometerData::AnemometerData(const char *frame) {
    int checksum;
    char sum = 0;
    const char *start;

    if(sscanf(frame,"\002%c,%f,%f,%f,%c,%i,\003%x",
       &id,
       &u_velocity,
       &v_velocity,
       &w_velocity,
       &units,
       &status,
       &checksum
      ) < 7)
        throw AnemometerData::exception("missing parameters",std::string(frame));

     start = frame;
     while(*start != '\002') start++;
     start++;
     while(*start != '\003') 
       sum ^= *start++;

     if(sum != checksum) 
         throw AnemometerData::exception("invalid checksum",std::string(frame));

     if(status != 0) 
         throw AnemometerData::status_error(status);
}

char AnemometerData::getID() {
   return id;
}

float AnemometerData::getU() {
   return u_velocity;
}

float AnemometerData::getV() {
   return v_velocity;
}

unsigned int AnemometerData::getStatus() {
   return status;
}

float AnemometerData::getW() {
  return w_velocity;
}

} /* end of namespace */
