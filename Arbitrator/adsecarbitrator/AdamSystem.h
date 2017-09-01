#ifndef ADAMSYSTEM_H_INCLUDE
#define ADAMSYSTEM_H_INCLUDE

#include "framework/AbstractSystem.h"
#include "AdamLib.h"

#define ADAMSYSTEM_NAME "ADAM"

using namespace Arcetri::Arbitrator;


namespace Arcetri{
    namespace AdSec_Arbitrator {
	
        class AdamSystem: public AbstractSystem {
	
          private:
            bool _simulation;

          public:
	
            AdamSystem(bool simulation=false);
            virtual ~AdamSystem();

            bool disableCoils();
            bool enableCoils();
            bool disableTss();
            bool enableTss();

          private:
            AdamModbus * _adam;
            AbstractArbitrator* _arbitrator;

		
        };

    }
}

#endif /*ADAMSYSTEM_H_INCLUDE*/
