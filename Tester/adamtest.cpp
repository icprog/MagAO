#include "AdamLib.h"
#include "Utils.h"

int main(int argc, char **argv) {

    AdamModbus *adam = new AdamModbus();
    int bitmask;
    int adamvec[8];
    string time0;
    string time1;
   

    adam->setLogLevel(Logger::LOG_LEV_TRACE); 

    while(1) {

        adam->ReadInputStatus(1, 0, 8, &bitmask);
        for (int i=0; i<=7; i++) {
            int bit = 1 << i;
            adamvec[i]= (bitmask & bit) ? 1:0;
        }
        //time0 = Utils::asciiDateAndTime();
        printf("Adam in status: %d:%d:%d:%d:%d:%d:%d:%d\n", adamvec[7], adamvec[6], adamvec[5], adamvec[4], adamvec[3], adamvec[2], adamvec[1], adamvec[0]);


        adam->ReadCoilStatus(1, 16, 8, &bitmask);
        for (int i=0; i<=7; i++) {
            int bit = 1 << i;
            adamvec[i]=(bitmask & bit) ? 1 :0;
        }
//        time1 = Utils::asciiDateAndTime();
        printf("Adam out status: %d:%d:%d:%d:%d:%d:%d:%d\n", adamvec[7], adamvec[6], adamvec[5], adamvec[4], adamvec[3], adamvec[2], adamvec[1], adamvec[0]);

        sleep(1);
    }

    return 0;
}
