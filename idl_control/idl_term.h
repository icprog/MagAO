
#ifndef IDL_TERM_H_INCLUDED
#define IDL_TERM_H_INCLUDED

#include <string>
#include "AOApp.h"
#include "idllib/IdlCtrlInterface.h"


//@Class: IDLTerm

class IDLTerm: public AOApp {

	public:
	
		IDLTerm( const std::string &conffile) throw (AOException);
	
	   	IDLTerm( int argc, char **argv) throw (AOException);
	
	   	void Create();

        virtual ~IDLTerm();

	
	protected:
	   
	   	void Run();
	
	
	   	// VIRTUAL message handlers
	protected:
	   static void ctrlc(int);

	
	protected:
       std::string         _idlCtrlId;
       int                 _idlCmdTimeout;
};

#endif // IDL_TERM_H_INCLUDED
