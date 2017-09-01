
#include "Logger.h"

#include "Utils.h"
using namespace Arcetri;


int main( int argc, char **argv)
{
   Logger *logger = Logger::get();

   int n = 250000;

   printf("Subsystem: --%s--\n", Utils::getAdoptSubsystem().c_str());


   return 0;
}
