
#include <iostream>
#include "RTDBvar.h"
#include "AOExcept.h"


int main()
{
   std::string varname;
   std::string device = "CCD";
   std::string name = "FRAMERATE";

   varname = RTDBvar::varname( device, CUR_VAR, name);
   std::cout << varname << std::endl;

   varname = RTDBvar::varname( device, REQ_VAR, name);
   std::cout << varname << std::endl;

   varname = RTDBvar::varname( device, NO_DIR, name);
   std::cout << varname << std::endl;

   varname = RTDBvar::varname( device, REQ_VAR, name);
   std::cout << varname << std::endl;

   std::cout << std::endl;

   return 0;
}
