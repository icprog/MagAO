
#define VERS_MAJOR   1
#define VERS_MINOR   0


#include <stdio.h>
#include <stdlib.h>
#include <readline/readline.h>
#include <readline/history.h>
#include <signal.h>

#include "idl_term.h"

// Awful global due to sigaction !!!!
IdlCtrlInterface*   _idlCtrlInterface;

//@Main: 
//
// idl_term   IDL terminal program
//
//@

IDLTerm::IDLTerm( const std::string &conffile) throw (AOException) : AOApp(conffile)
{
   Create();
}

IDLTerm::IDLTerm( int argc, char **argv) throw (AOException): AOApp( argc, argv)
{
   Create();
}

void IDLTerm::Create()
{
//    _idlCmdTimeout = (int) ConfigDictionary()["idlCmdTimeout"]; 
//	int _server_id = (int) ConfigDictionary()["IDLRPCServerId"]; // 0x2010CAFA = 537971450 ;
 string idlCtrlName; 
  try { 
   idlCtrlName = (string)ConfigDictionary()["idlCtrlName"];  
  }
  catch(Config_File_Exception &e) {
     _logger->log(Logger::LOG_LEV_ERROR, "Error reading config file: %s [%s:%d]", e.what().c_str(), __FILE__, __LINE__);
     throw;
  }


    /*
     * Catch SIGINT signal and send message to IDLCtrl 
     */
    struct sigaction act;
    act.sa_handler = ctrlc;
    sigemptyset (&act.sa_mask);
    act.sa_flags = 0;
    sigaction(SIGINT, &act, NULL);

    // create a client interface for the IDL RPC
    //_logger->log(Logger::LOG_LEV_INFO, "Creating IDLCtrlInterface to server id %X  [%s:%d]",
                        //_server_id, __FILE__, __LINE__);
    _logger->log(Logger::LOG_LEV_INFO, "Creating IDLCtrlInterface to server %s  [%s:%d]",
                        idlCtrlName.c_str(), __FILE__, __LINE__);
    _idlCtrlInterface = new IdlCtrlInterface( idlCtrlName, Logger::LOG_LEV_TRACE);

    /* New version 
    _logger->log(Logger::LOG_LEV_INFO, "Creating IDLCtrlInterface to server id %X  [%s:%d]",
                        _server_id, __FILE__, __LINE__);
    //_idlCtrlInterface = new IdlCtrlInterface(_server_id, Logger::LOG_LEV_INFO);
    printf("TOFIX: IdlCtrlInterface() with RPC interface\n");
    _idlCtrlInterface = new IdlCtrlInterface( "", Logger::LOG_LEV_INFO);
    */
}

IDLTerm::~IDLTerm()
{
    delete _idlCtrlInterface;
}


void IDLTerm::Run()
{
    char *idlCmd;
	while( (idlCmd = readline("AdOpt>")))
	{
        if (strlen(idlCmd) == 0) continue;
        
        string s_idlCmd = string(idlCmd);
		
        add_history(idlCmd);
		free(idlCmd);

        int found=s_idlCmd.find("%get");
        if (found!=string::npos) {
            try {
                int value = _idlCtrlInterface->getIntVariable(s_idlCmd.substr(5), _idlCmdTimeout);
                std::cout << value << std::endl;
            } catch(IdlCtrlInterfaceException &e) {
                _logger->log(Logger::LOG_LEV_ERROR, "Error getting variable from IDL: %s  [%s:%d]",
                        e.what().c_str(), __FILE__, __LINE__);
              std::cout << e.what() << std::endl;
            }
        } else {
            try {
                _logger->log(Logger::LOG_LEV_INFO, "Command %s sending to IDL [%s:%d]",
                        s_idlCmd.c_str(),  __FILE__, __LINE__);
                IdlCommandReply reply = _idlCtrlInterface->sendCommand(s_idlCmd, _idlCmdTimeout);
                std::cout << reply.getOutput();
                std::cout << reply.getError();
            } catch(IdlCtrlInterfaceException &e) {
                    _logger->log(Logger::LOG_LEV_ERROR, "Error sending command to IDL: %s  [%s:%d]",
                            e.what().c_str(), __FILE__, __LINE__);
                std::cout << e.what() << std::endl;
            }
        }
    }

}


// Ctrl-C handler
void  IDLTerm::ctrlc( int signal)
{ 
     _logger->log(Logger::LOG_LEV_ERROR, "Got Ctrl-C signal  [%s:%d]", __FILE__, __LINE__);
     _idlCtrlInterface->sendCtrlC();
}

int main( int argc, char **argv) {

   SetVersion(VERS_MAJOR,VERS_MINOR);

   try {
       IDLTerm *c;

       if (argc>1)
          c = new IDLTerm( argc, argv);
       else {
         c = new IDLTerm( AOApp::getConffile("idlterm.conf"));
       }

       c->Exec();

       delete c;
   } catch (AOException &e) {
       Logger::get()->log( Logger::LOG_LEV_FATAL, "%s:%d: %s", __FILE__, __LINE__, e.what().c_str());
   }
}

