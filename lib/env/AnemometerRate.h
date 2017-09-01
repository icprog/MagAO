#ifndef ANEMOMETERRATE_H
#define ANEMOMETERRATE_H

#include <string>
#include <stdexcept>

#define HZ_TO_USEC(x) 1000000/x

namespace Arcetri {


class AnemometerRate {
  private:
   
  const std::string *rate;
  const std::string *rep;
  const float *hz;


  public:

  class exception:public std::runtime_error {
    public:
    explicit exception(const std::string &);
  }; 

  static AnemometerRate default_rate() {
     return AnemometerRate("1");
  };

  AnemometerRate(); 
  AnemometerRate(std::string);
  const std::string getRep();
  const std::string getRate();
  float getHz();
  unsigned int getMilliSeconds();
  unsigned int getMicroSeconds();

  ~AnemometerRate();
};


} /* end of namespace */
#endif
