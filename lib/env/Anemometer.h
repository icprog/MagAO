#ifndef ANEMOMETER_H
#define ANEMOMETER_H

#include "AnemometerData.h"
#include "AnemometerRate.h"
#include "chardev.h"

#include <stdexcept>
#include <string>

namespace Arcetri {

    class Anemometer {
      private:

        chardev *device;
        AnemometerRate rate;

      public:

        class exception: public std::runtime_error {
          public:
            explicit exception(const std::string &);
        };

        Anemometer(chardev *, AnemometerRate &, bool dosetup = true);
        virtual ~Anemometer();

        virtual AnemometerData read_frame();
        virtual void setup();
    };

} /* end of namespace */

#endif
