#include "env/Anemometer.h"
#include "TcpConnection.h"
#include "Utils.h"
using namespace Arcetri;

int main(int argc, char **argv) {

    Logger::get()->log(Logger::LOG_LEV_INFO, "Starting...");

    if (argc != 3) {
        printf("Usage: anemometertest <host> <port>\n\n");
        exit(0);
    }

    Anemometer *an = NULL;
    AnemometerData ad;
    chardev *dev;
    TcpConnection *tcp;
//  AnemometerRate rate = AnemometerRate::default_rate();
    AnemometerRate rate("5");

    string host = argv[1];
    int port = atoi(argv[2]);
    
    printf("Connecting to %s:%d\n", host.c_str(), port);
    
    try {
        dev = new chardev(host, port);
        an = new Anemometer(dev,rate);
    }
    catch (Anemometer::exception &e) {
        printf("Failed to connect to device (%s)\n", e.what());
        return -1;
    }
    catch (...) {
        printf("Failed to connect to device\n");
        return -1;
    }
    

    while (true) {
        ad = an->read_frame();
        printf("Anemometer data %c (status %d): %.2f %.2f %.2f\n", ad.getID(), ad.getStatus(),
               ad.getU(), ad.getV(), ad.getW());
    //    sleep(1);
    }

    return 0;
}
