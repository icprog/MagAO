#ifndef ANEMOMETERDATA_H
#define ANEMOMETERDATA_H

#include <stdexcept>
#include <string>

namespace Arcetri {

class AnemometerData {
  private:

  char id;
  float u_velocity;
  float v_velocity;
  float w_velocity;
  unsigned int status;
  char units;

  public:

  class exception: public std::runtime_error {
     private:
     std::string _frame; 

     public:
     exception(const std::string &, const std::string &);
     std::string frame();
     ~exception() throw();
  };

  class status_error: public std::runtime_error {
    private:
    static const std::string status_error_lookup(int status);

    public:
    status_error(int status);
  };

  AnemometerData();
  AnemometerData(const char *);
  char getID();
  float getU();
  float getV();
  float getW();
  unsigned int getStatus();
  char getUnits();
};

} /* End of namespace */
#endif
