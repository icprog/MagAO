
#include "nettest.h"
using namespace Arcetri;

extern "C" {
#include "base/thrdlib.h"
}


#define LINESTODO (500000)
#define DELAY 

nettest::nettest( int argc, char *argv[]) : AOApp(argc, argv) {

}

void nettest::Run() {


  string ifName = "eth0";
  string mac, ip, netmask;

  Utils::getMacAddr( ifName, mac, ip, netmask);

  printf("Interface: %s\n", ifName.c_str());
  printf("MAC      : %s\n", mac.c_str());
  printf("Ip       : %s\n", ip.c_str());
  printf("Netmask  : %s\n", netmask.c_str());

}


int main( int argc, char *argv[]) {

    nettest *test = NULL;

  string ifName = "eth1";
  string mac, ip, netmask;

  Utils::getMacAddr( ifName, mac, ip, netmask);

  printf("Interface: %s\n", ifName.c_str());
  printf("MAC      : %s\n", mac.c_str());
  printf("Ip       : %s\n", ip.c_str());
  printf("Netmask  : %s\n", netmask.c_str());


    try {
	test = new nettest( argc, argv);

	test->Exec();

	delete test;
    }
    catch (LoggerFatalException &e) {
	// In this case the logger can't log!!!
	printf("%s\n", e.what().c_str());
    }
    catch (AOException &e) {
	Logger::get()->log(Logger::LOG_LEV_FATAL, "Error: %s", e.what().c_str());
    }

    return 0;
}
  

