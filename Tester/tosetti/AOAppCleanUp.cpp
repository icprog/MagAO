#include "AOApp.h"

using namespace Arcetri;

int main() {
	
	AOApp* app;
	for(int i=0; i<100; i++) {
		app = new AOApp("AOAPP", 
				   	    "L", 
				 	    "127.0.0.1", 
				 	    i,
				 	    Logger::LOG_LEV_INFO);
		app->Exec();
		delete app;	 		  
	}
	          
}
