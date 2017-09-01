
// Uncomment the following line to have debug printout
//#define DEBUG 

#include "env/AnemometerData.h"
#include "env/AnemometerRate.h"
#include "env/Anemometer.h"

#include <cerrno>
#include <cstdlib>

namespace Arcetri {

    Anemometer::exception::exception(const std::string &msg): std::runtime_error(msg) {
    }

    Anemometer::~Anemometer() {
    }

    Anemometer::Anemometer(chardev *device,AnemometerRate &rate, bool dosetup) {
        this->device = device;
        this->rate = rate;
        device->set_terminator("\r\n"); // terminator string
        device->set_echo(true); 
        device->set_check_echo(false); // Doesn't work with Anemometer in "continuous" mode

        if (dosetup)
            setup();
    }

    void Anemometer::setup() {
        const char *init[] = {
            "M1","U1","O1","L1","NQ","T1",
            "S1","C2","A1","J1","X1","G0",
            "K50",
            "\0"
        };

        const char *line;

#ifdef DEBUG
        printf("Sending two Q lines\n");   // DBG
#endif
        device->write_line("Q");          // Be sure we are in polling mode
        device->write_line("Q");
	msleep(100);
	
#ifdef DEBUG
        printf("Sending one * char\n");   // DBG
#endif
        device->putch('*');
        bool found=false;
        for(int i=0;i<30;i++) {       // Throw away lines in input buffer
            line=device->read_line();
#ifdef DEBUG
            printf("Read line %d: %s\n",i,line);   // DBG
#endif
            if(strcmp(line,"CONFIGURATION MODE") == 0) {
                found=true;
#ifdef DEBUG
                printf("Got CONFIG. MODE\n");   // DBG
#endif
                break;
            }
        }
        if(!found)
	  throw AOException("could not enter configuration mode", COMMUNICATION_ERROR);
  
        for(int i = 0 ; init[i][0] != '\0'; i++) {

#ifdef DEBUG
            printf("Sending: %s\n",init[i]);   // DBG
#endif
            device->write_line(init[i]);
            line=device->read_line();
#ifdef DEBUG
            printf("Got line 1: %s\n",line);   // DBG
#endif
            line=device->read_line();
#ifdef DEBUG
            printf("Got line 2: %s\n",line);   // DBG
#endif
            if(strcmp(line,init[i]) != 0) 
	      throw AOException("configuration error", COMMUNICATION_ERROR);
        }

        string rep=rate.getRep();

#ifdef DEBUG
        printf("Setting rate: %s\n",rep.c_str());      // DBG
#endif
        device->write_line(rate.getRep().c_str());
        device->read_line();
        line=device->read_line();
#ifdef DEBUG
        printf("read line: %s\n",line);               // DBG
#endif

        if(strcmp(line,rate.getRep().c_str()) != 0)
            throw AOException("set rate config error", COMMUNICATION_ERROR);

        /* after we exit from config mode, the first few frames may be garbage */
#ifdef DEBUG
        printf("Sending final Q\n");   // DBG
#endif
        device->write_line("Q");
        while(1) {
            try {
                read_frame();
            } catch(AnemometerData::exception &e) {
                continue;
            } catch(...) {
                continue;
            }
            break;
        }
    }

    AnemometerData Anemometer::read_frame() {
    
      int timeout = max(int(( rate.getMicroSeconds() + 1) / 1000), 1000);
    
        const char *line = device->read_line(timeout);
        while(*line != '\002' && *line != '\0') line++;
        return AnemometerData(line);
    }


}  /* end of namespace */

