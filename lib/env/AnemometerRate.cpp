#include "env/AnemometerRate.h"


namespace Arcetri {

AnemometerRate::exception::exception(const std::string &msg):std::runtime_error(msg) {
}

AnemometerRate::AnemometerRate() {
}

AnemometerRate::AnemometerRate(std::string rate) {
  static struct { 
    const std::string rate;
    const std::string rep;
    const float hz;
  } rate_map[] = {
    {"1/4", "P20", .25},
    {"1/2", "P21", .50}, 
    {"1", "P1", 1 },
    {"2", "P2", 2 },
    {"4", "P3", 4 },
    {"5", "P4", 5 },
    {"8", "P5", 8 },
    {"10", "P6", 10 },
    {"16", "P7", 16 },
    {"20", "P8", 20 },
    {"32", "P9", 32 },
    {"","",0},
  };

  for(int i = 0; rate_map[i].rate != ""; i++) 
    if(rate == rate_map[i].rate) {
      this->rate = &(rate_map[i].rate);
      this->rep = &(rate_map[i].rep);
      this->hz =  &(rate_map[i].hz);
      return;
    }
   
   throw AnemometerRate::exception("unknown rate");
}

const std::string AnemometerRate::getRate() {
  return *(this->rate);
}

const std::string AnemometerRate::getRep() {
  return *(this->rep);
}

float AnemometerRate::getHz() {
  return *(this->hz);
}

unsigned int AnemometerRate::getMilliSeconds() {
  return (unsigned int) (1000/getHz());
}

unsigned int AnemometerRate::getMicroSeconds() {
  return (unsigned int) (1000000/getHz());
}

AnemometerRate::~AnemometerRate() {
}

} /* end of namespace */

