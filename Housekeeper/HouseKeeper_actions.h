#ifndef HOUSEKEEPER_ACTIONS_H_INCLUDED
#define HOUSEKEEPER_ACTIONS_H_INCLUDED

#include <iostream>


#include "Funct.h"
#include "AdamLib.h"
#include "Logger.h"

extern "C" {
#include "aotypes.h"        // float32
#include <pthread.h>
}
#include "BcuLib/bcucommand.h"
#include "AdSecConstants.h"

using namespace Arcetri;

/*
class FunctEmergencyStop: public Funct 
{
    public:
    
      	FunctEmergencyStop() {isOn=false;}
        
        virtual void doit(double *f)
        {
            Logger::get()->log(Logger::LOG_LEV_WARNING, "FunctEmergencyStop doit(double*) %p should not be called ",f);
        }
        
        virtual void doit(const std::string &y, double f)
        {
            Logger::get()->log(Logger::LOG_LEV_WARNING, "[FunctEmergencyStop] %s = %g  [%s:%d]",y.c_str(),f,__FILE__,__LINE__);
            if(!isOn) {
                isOn = true;
                Adam* adam = new Adam();
                adam->disableCoils();
                delete adam;
                spawn_siren();
            }
        }
    private:
    
        bool isOn;

        ///////////// SUONA LA FANFARA //////////////
        void spawn_siren(){
            pthread_attr_t attribute;
            pthread_t thread_id;

            pthread_attr_init(&attribute);
            pthread_attr_setdetachstate(&attribute, PTHREAD_CREATE_DETACHED); // dies at end
            Logger::get()->log(Logger::LOG_LEV_TRACE, "[FunctEmergencyStop] Spawning thread [%s:%d]",__FILE__,__LINE__);
            if(pthread_create(&thread_id,&attribute, PlaySiren,this)!=0) {
                Logger::get()->log(Logger::LOG_LEV_ERROR, "[FunctEmergencyStop] CAN'T START PLAYING SIREN [%s:%d]",__FILE__,__LINE__);
            }
        }

        static void* PlaySiren(void* argp){
            FunctEmergencyStop *pt = (FunctEmergencyStop*)argp;

            system("play siren.wav");
            pt->isOn=false;
            return 0;
        }

};*/
#endif //HOUSEKEEPER_ACTIONS_H_INCLUDED
