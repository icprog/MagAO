//@File: idl_ctrl.c
//
// The {\tt echoer} program echoes every message it receives back to 
// the sender.
//@

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/types.h>

#include "stdconfig.h"
#include "AOStates.h"

#include <string>

#include "arblib/base/AlertsExport.h"
#include "arblib/base/Alerts.h" // Needed for Alert serialization

extern "C" {
   #include "idllib/idllib.h"
   #include "idl_rpc.h"
}

#include "idl_ctrl.h"

const char *MyName = "IDLCTRL";
int VersMajor = 2;
int VersMinor = 0;
const char *Date = "September 2007";

#define BUFLEN (2047)
char buffer[BUFLEN+1];


//@Function help
//
//  help prints an usage message
//@
// Rimosso perche' non usato (L.F.)
//static void help()
//{
//    printf("\nIDLCTRL  - Vers. %d.%d.   A.Puglisi, %s\n\n",
//            VersMajor,VersMinor,Date);
//    printf("Usage: idlctrl [-v] [<Server Address>]\n\n");
//    printf("       idlctrl -h\n\n");
//    printf("   -f    configuration file (defaults to config)\n\n");
//    printf("   -v    verbose mode\n\n");
//    printf("   -h    prints help page and exit\n\n");
//    printf("<Server Address> defaults to 127.0.0.1\n\n");
//}

int writepipe[2],   /* parent -> child */
    readpipe [2],   /* child -> parent */
    errpipe  [2];   /* child -> parent */

#define PARENT_READ     readpipe[0]
#define CHILD_WRITE     readpipe[1]
#define CHILD_READ      writepipe[0]
#define PARENT_WRITE    writepipe[1]
#define PARENT_READERR  errpipe[0]
#define CHILD_WRITEERR  errpipe[1]

void ctrlc( int signal);
int idlPid;
IDLCtrl *instance=NULL;

//@Main: 
//
// idl_ctrl   IDL controller program
//
// Sends incoming messages to IDL's standard input
// and sends back IDL's standard output as answers.
//@

IDLCtrl::IDLCtrl( int argc, char **argv) throw (AOException): AOApp( argc, argv)
{
   instance=this;
   Create();
}

IDLCtrl::~IDLCtrl()
{
}

void IDLCtrl::Create()
{
    signal( SIGUSR1, ctrlc);

    // Determine whether we are going to use RPC mode or not
    _rpc=0;
    try {
      _rpc = ConfigDictionary()["use_rpc"];
    } catch (Config_File_Exception &e) {
      _logger->log(Logger::LOG_LEV_INFO, "Warning: keyword 'use_rpc' not defined in config file, defaulting to normal IdlCtrl mode.");
    }

    // Check conf. file keywords based on rpc value

    RequireKeyword("IDL_ErrorCodeTrap");
    RequireKeyword("AdSecArbName");
    RequireKeyword("IDL_SetDLMPathCmd");
    RequireKeyword("IDL_SetPathCmd");
    RequireKeyword("IDL_StartupCmd");

    if (_rpc) {
       RequireKeyword("IDL_path_rpc");
    } else {
       RequireKeyword("IDL_path");
       RequireKeyword("IDL_EndCmd");
       RequireKeyword("IDL_EndMsg");
       RequireKeyword("IDL_StartCmd");
       RequireKeyword("IDL_StartMsg");
    }
    CheckKeywords(true, true, true);

    // Load keywords
    _IDL_ErrorCodeTrap = (std::string) ConfigDictionary()["IDL_ErrorCodeTrap"];
         _IDL_path_rpc = expandVars( (std::string) ConfigDictionary()["IDL_path_rpc"]);
        _IDL_path = expandVars( (std::string) ConfigDictionary()["IDL_path"]);
        _IDL_EndCmd = (std::string) ConfigDictionary()["IDL_EndCmd"];
        _IDL_EndMsg = (std::string) ConfigDictionary()["IDL_EndMsg"];
        _IDL_StartCmd = (std::string) ConfigDictionary()["IDL_StartCmd"];
        _IDL_StartMsg = (std::string) ConfigDictionary()["IDL_StartMsg"];
        
    pthread_mutex_init(&cmd_mutex, NULL);

    
    _adSecArbNotifier = new AlertNotifier((string)(AOApp::ConfigDictionary()["AdSecArbName"])+"."+Side());

    _demoModeLooptime = 60;
    try {
       _demoModeLooptime = ConfigDictionary()["demo_mode_looptime"];
       if (_demoModeLooptime > 0) 
         _logger->log(Logger::LOG_LEV_INFO, "Demo mode looptime set to %d seconds", _demoModeLooptime);
       else
         _logger->log(Logger::LOG_LEV_WARNING, "Demo mode looptime is <=0. License check is DISABLED");
    } catch (Config_File_Exception &e) {
       _logger->log(Logger::LOG_LEV_WARNING, "No keyword 'demo_mode_looptime' in config file. Set to 60 seconds");
    }

    _errcode=0;

    // Spawn a first IDL RPC to set the DLM also in the first IDL run
    _logger->log(Logger::LOG_LEV_INFO, "Spawn IDL process to set the DLM path preferencies...");

    spawnIdlRpc();

    connectIdlRpc();
  
    std::string cmdDLMpath = expandVars( (std::string) ConfigDictionary()["IDL_SetDLMPathCmd"]);
    Logger::get()->log(Logger::LOG_LEV_DEBUG, "Executing %s [%s:%d]", cmdDLMpath.c_str(),  __FILE__, __LINE__);
    SendIDLcmdRpc( cmdDLMpath.c_str(), cmdDLMpath.size(), NULL, NULL, NULL);

    if(!IDL_RPCCleanup(_pClient, 1)) {
       _logger->log(Logger::LOG_LEV_FATAL, "RPC IDL DLM path cleanup failed. Exiting");
        exit(0);
    }

    if (_rpc)
      spawnIdlRpc();
    else
      spawnIdlCtrl();


    _logger->log(Logger::LOG_LEV_INFO, "Startup done");
}

int IDLCtrl::demoModeLooptime() {
   return _demoModeLooptime;
}

void *IDLCtrl::licenseThread( void *arg) {

   string out_string, err_string;
   int stat;
   char cmd[] = "print,test_demo_mode()";
   IDLCtrl *ctrl = (IDLCtrl *)arg;

   // See if the check is disabled
   if (ctrl->demoModeLooptime() <= 0)
      return NULL;

   while (1) {

      stat = ctrl->SendIDLcmd( cmd, strlen(cmd), &out_string, &err_string);
      if (IS_ERROR(stat) || (atoi(out_string.c_str()) != 0)) {
         _logger->log(Logger::LOG_LEV_WARNING, "IDL in demo mode");
         ctrl->_demoModeVar.Set(0,0,FORCE_SEND);
         ctrl->notifyLicenseAlert();
      } else {
         ctrl->_demoModeVar.Set(1,0,FORCE_SEND);
      }

      sleep(ctrl->demoModeLooptime());
 }
  return NULL;
}



void IDLCtrl::spawnIdlRpc() {

    _logger->log(Logger::LOG_LEV_INFO, "Starting IDL... %s [%s:%d]", 
            _IDL_path_rpc.c_str(), __FILE__, __LINE__);

    idlPid = fork();
    if (idlPid == 0)
        {
        // Start IDL
        execlp( _IDL_path_rpc.c_str(), "idlrpc", "-server=20500000", NULL);

        _logger->log(Logger::LOG_LEV_FATAL, "RPC IDL start failed with error '%s'. Exiting",strerror(errno));
        exit(errno);
        // If we arrive here, there was a problem in starting IDL
        }
}


void IDLCtrl::connectIdlRpc() {

    for (int i=0; 30; i++) {
        _pClient = IDL_RPCInit(0x20500000, NULL);
       if (_pClient == NULL) {
         Logger::get()->log(Logger::LOG_LEV_INFO, "Cannot initialize RPC communication, waiting...[%s:%d]",  __FILE__, __LINE__);
         usleep(100000);
       } else {
         break;
       }
       if (i==30) {

         Logger::get()->log(Logger::LOG_LEV_FATAL, "Cannot initialize RPC communication after %d  attempts. Exiting.[%s:%d]", i,  __FILE__, __LINE__);
         SetTimeToDie(true);
       }
    }

    if (!IDL_RPCOutputCapture( _pClient, 10000)) {
      Logger::get()->log(Logger::LOG_LEV_FATAL, "Cannot setup RPC output buffer [%s:%d]",  __FILE__, __LINE__);
      SetTimeToDie(true);
    }
    if (!IDL_RPCTimeout(500)) {
      Logger::get()->log(Logger::LOG_LEV_FATAL, "Cannot set RPC timeout [%s:%d]",  __FILE__, __LINE__);
      SetTimeToDie(true);
    }
    Logger::get()->log(Logger::LOG_LEV_INFO, "RPC communication initialized [%s:%d]",  __FILE__, __LINE__);

}

void IDLCtrl::spawnIdlCtrl() {

    // Start IDL
    //
    // Creates two pipes for read/write with IDL and forks

    _logger->log(Logger::LOG_LEV_INFO, "Starting IDL... %s [%s:%d]", 
            _IDL_path.c_str(), __FILE__, __LINE__);

    if ( pipe(readpipe) < 0  ||
         pipe(writepipe) < 0 ||
         pipe(errpipe) <0)
        {
        /* FATAL: cannot create pipe */
        _logger->log(Logger::LOG_LEV_FATAL, "Cannot create pipe: %s", strerror(errno));
        SetTimeToDie(true);
        }

    idlPid = fork();
    if (idlPid ==0)
        {
        // Adjust child's file descriptors
        close( PARENT_WRITE);
        close( PARENT_READ);
        close( PARENT_READERR);

        dup2(CHILD_READ,  0);  close(CHILD_READ);
        dup2(CHILD_WRITE, 1);  close(CHILD_WRITE);
        dup2(CHILD_WRITEERR, 2);  close(CHILD_WRITEERR);

        // Start IDL
        execlp( _IDL_path.c_str(), "idl", NULL);

        _logger->log(Logger::LOG_LEV_FATAL, "Child IDL start failed with error '%s'. Exiting",strerror(errno));
        exit(errno);
        // If we arrive here, there was a problem in starting IDL
        // but filehandles are already closed. What to we do??
        }

    // Adjust parent's file descriptors
    close(CHILD_READ);
    close(CHILD_WRITE);
    close(CHILD_WRITEERR);
    fcntl( PARENT_READERR, F_SETFL, O_NONBLOCK);  // IDL's stderr not blocking


    _logger->log(Logger::LOG_LEV_INFO, "Flushing IDL stderr...");
    // Flush IDL's stderr before starting
    msleep(1000);
    while( read( PARENT_READERR, buffer, BUFLEN-1) >0)
        ;

}

void IDLCtrl::SetupVars()
{
   try {
    _pidVar = RTDBvar( MyFullName(), "pid", NO_DIR, INT_VARIABLE);
    _errcodeVar = RTDBvar( MyFullName(), "errcode", NO_DIR, INT_VARIABLE);
    _demoModeVar  = RTDBvar( Side()+".IDL_STAT",INT_VARIABLE,1);
    _demoModeVar.Set(1);
    _rpcVar = RTDBvar( MyFullName(), "rpc", NO_DIR, INT_VARIABLE);
    _rpcVar.Set(_rpc);
   } catch (AOVarException &e) {
        Logger::get()->log(Logger::LOG_LEV_FATAL, "%s:%d: %s", __FILE__, __LINE__, e.what().c_str());
        throw AOException("Error creating RTDB variables");
   }
}

string IDLCtrl::expandVars( string s)
{
   size_t pos;
   if ((pos = s.find("$ADOPT_ROOT")) != string::npos)
      s.replace( pos, 11, getenv("ADOPT_ROOT"));
   return s;
}

void IDLCtrl::PostInit()
{
    // Initialize RPC communication
    if (_rpc) connectIdlRpc();

    // Set our PID in the message daemon - to receive Ctrl-C signals
    _pidVar.Set(getpid());

    // Start license check thread
    pthread_attr_t attribute;
    pthread_attr_init(&attribute);
    pthread_attr_setdetachstate(&attribute,               // Declare thread "detached"
                                 PTHREAD_CREATE_DETACHED); // So that it dies at end

    int stat;
    if((stat=pthread_create(&license_thread,&attribute,licenseThread, (void*)this))!=0) {
        _logger->log(Logger::LOG_LEV_ERROR, "Error: cannot start license check thread (%s)", strerror(stat));
    } 
   
    std::string cmdpath = expandVars( (std::string) ConfigDictionary()["IDL_SetPathCmd"]);
    Logger::get()->log(Logger::LOG_LEV_DEBUG, "Executing %s [%s:%d]", cmdpath.c_str(),  __FILE__, __LINE__);
    SendIDLcmd( cmdpath.c_str(), cmdpath.size(), NULL, NULL, NULL);

    std::string cmd = expandVars( (std::string) ConfigDictionary()["IDL_StartupCmd"]);
    Logger::get()->log(Logger::LOG_LEV_DEBUG, "Executing %s [%s:%d]", cmd.c_str(),  __FILE__, __LINE__);
    SendIDLcmd( cmd.c_str(), cmd.size(), NULL, NULL, NULL);

    cmd = "print, '"+_IDL_ErrorCodeTrap+"',errcode";
    Logger::get()->log(Logger::LOG_LEV_DEBUG, "Executing %s [%s:%d]", cmd.c_str(),  __FILE__, __LINE__);
    SendIDLcmd( cmd.c_str(), cmd.size(), NULL, NULL, &_errcode);

    _errcodeVar.Set(_errcode, 0, FORCE_SEND);


    setCurState(STATE_READY);
}


void IDLCtrl::InstallHandlers()
{
    int stat;

   _logger->log(Logger::LOG_LEV_INFO, "Installing IDL handlers ...");

    stat = thHandler( IDLCMD, "*", 0, IDLcmd_handler, "IDLcmd_handler", this);
    if (IS_ERROR(stat))
         {
         _logger->log(Logger::LOG_LEV_FATAL, "Error %d (%s) from thHandler()", stat, lao_strerror(stat));
         SetTimeToDie(true);
         }
    stat = thHandler( IDLLOCK, "*", 0, IDLlock_handler, "IDLlock", this);
    if (IS_ERROR(stat))
         {
         _logger->log(Logger::LOG_LEV_FATAL, "Error %d (%s) from thHandler()", stat, lao_strerror(stat));
         SetTimeToDie(true);
         }
    stat = thHandler( IDLUNLOCK, "*", 0, IDLunlock_handler, "IDLunlock", this);
    if (IS_ERROR(stat))
         {
         _logger->log(Logger::LOG_LEV_FATAL, "Error %d (%s) from thHandler()", stat, lao_strerror(stat));
         SetTimeToDie(true);
         }
}




// IDLLOCK message: locks the IDL session so that only this process can send commands
// returns back a confirmation message with the same code

int IDLCtrl::IDLlock_handler( MsgBuf *msgb, void *argp, int /* thQueueSize */)
{
   IDLCtrl *ctrl = (IDLCtrl *)argp;

   ctrl->_logger->log(Logger::LOG_LEV_INFO, "Message %d", HDR_CODE(msgb));

   if ((ctrl->_lockedBy != "") && (ctrl->_lockedBy != HDR_FROM(msgb)))
        {
        ctrl->_logger->log(Logger::LOG_LEV_WARNING, "IDL session locked by %s, command from %s was ignored", ctrl->_lockedBy.c_str(), HDR_FROM(msgb));
        thRelease(msgb);
        return NO_ERROR;
        }

   ctrl->_lockedBy = HDR_FROM(msgb);
   int stat=thReplyMsg(IDLLOCK,0,NULL,msgb);
   if(IS_ERROR(stat))  
      Logger::get()->log(Logger::LOG_LEV_ERROR, "Error in IDLCtrl::IDLlock_handler(): %s [%s:%d]",lao_strerror(stat), __FILE__, __LINE__);
   else
      Logger::get()->log(Logger::LOG_LEV_INFO, "IDL session locked by %s", ctrl->_lockedBy.c_str());

   thRelease(msgb);
   return NO_ERROR;
}


// IDLUNLOCK message: unlocks the previously locked IDL session
// returns back a confirmation message with the same code

int IDLCtrl::IDLunlock_handler( MsgBuf *msgb, void *argp, int /* thQueueSize */)
{
   IDLCtrl *ctrl = (IDLCtrl *)argp;
   ctrl->_lockedBy = "";
   int stat=thReplyMsg(IDLUNLOCK,0,NULL,msgb);
   if(IS_ERROR(stat))  
      Logger::get()->log(Logger::LOG_LEV_ERROR, "Error in IDLCtrl::IDLunlock_handler(): %s [%s:%d]",lao_strerror(stat), __FILE__, __LINE__);
   else
      Logger::get()->log(Logger::LOG_LEV_INFO, "IDL session unlocked by %s", HDR_FROM(msgb));

   thRelease(msgb);
   return NO_ERROR;
}


int IDLCtrl::IDLcmd_handler( MsgBuf *msgb, void *argp, int /* thQueueSize */)
{   
   IDLCtrl *ctrl = (IDLCtrl *)argp;
   ctrl->_logger->log(Logger::LOG_LEV_DEBUG, "Message %d", HDR_CODE(msgb));

    // If in locked state, refuse all messages except from the locking process
    if ((ctrl->_lockedBy != "") && (ctrl->_lockedBy != HDR_FROM(msgb)))
        {
        _logger->log(Logger::LOG_LEV_WARNING, "IDL session locked by %s, command from %s was ignored", ctrl->_lockedBy.c_str(), HDR_FROM(msgb));
        thRelease(msgb);
        return NO_ERROR;
        }

    _logger->log( Logger::LOG_LEV_TRACE, "%d bytes received from %s - seq:%d", 
            HDR_LEN(msgb),            // Use msglib macros to
            HDR_FROM(msgb),           // extract header fields
            HDR_SEQN(msgb));       
    snprintf(buffer, BUFLEN, "%s", (char *)MSG_BODY(msgb));
    buffer[HDR_LEN(msgb)]=0;

    string out_string, err_string;
    int stat;

    if (ctrl->isValidCmd((const char *)buffer, HDR_LEN(msgb)))
      stat = ctrl->SendIDLcmd( (const char *)buffer, HDR_LEN(msgb), &out_string, &err_string);
    else
      {
      out_string = "";
      err_string = "IDLCtrl error: invalid command";
      }
                                
    // Fill the body with the two consecutive strings, both zero-terminated 
    stat = SendIDLAnswer( msgb, (char*)out_string.c_str(), (char*)err_string.c_str());

    if (IS_ERROR(stat))
      _logger->log( Logger::LOG_LEV_ERROR, "Error in SendIDLCmd(): (%d) %s [%s:%d]", stat, lao_strerror(stat), __FILE__, __LINE__);

     //thRelease(msgb);
     return NO_ERROR;
}

//@Function: SendIDLcmd
//
// Sends a command to IDL and waits for the answer
//-

int IDLCtrl::SendIDLcmd( const char *cmd, int cmd_len, string *out_string, string *err_string, int *ret_code)
{
   int stat;

   // Allow serialization between threads
   pthread_mutex_lock(&cmd_mutex);

   _logger->log( Logger::LOG_LEV_INFO, "Executing command: %s", cmd);

   try {
      if (_rpc) 
         stat= SendIDLcmdRpc( cmd, cmd_len, out_string, err_string, ret_code);
      else
         stat= SendIDLcmdCtrl( cmd, cmd_len, out_string, err_string, ret_code);
   } catch(...) {
      _logger->log( Logger::LOG_LEV_ERROR, "Exception in SendIDLCmd() [%s:%d]", __FILE__, __LINE__);
   }

   pthread_mutex_unlock(&cmd_mutex);
   return stat;
}


int IDLCtrl::SendIDLcmdCtrl( const char *cmd, int cmd_len, string *out_string, string *err_string, int *ret_code)
{
        char lf = 10;
        int n;
        unsigned int pos;

        // Write start marker 
        write( PARENT_WRITE, _IDL_StartCmd.c_str(), _IDL_StartCmd.size());
        write( PARENT_WRITE, &lf, 1);

        // Write client's command
        write( PARENT_WRITE, cmd, cmd_len);
        write( PARENT_WRITE, &lf, 1);

        // Write end marker
        write( PARENT_WRITE, _IDL_EndCmd.c_str(), _IDL_EndCmd.size());
        write( PARENT_WRITE, &lf, 1);

        // Read stdout and stderr in parallel (!) to avoid deadlocks
        string idlout = "";
        string idlerr = "";

        int idlout_done = 0;
        int idlerr_done = 0;
        while((idlout_done == 0) || (idlerr_done == 0))
                {
                if (!idlout_done)
                        {
                        if ((n = read( PARENT_READ, buffer, BUFLEN-1)) > 0)
                                {
                                buffer[n]=0;
                                string tolog = escapePercent(buffer);

                                // Remove terminating newline (the logger will take care of that)
                                while ( (tolog.size()>0) && (tolog[ tolog.size()-1]) == '\n')
                                   tolog = tolog.substr(0, tolog.size()-1);

                                _logger->log( Logger::LOG_LEV_INFO, tolog.c_str());
                                idlout += std::string(buffer, n);
                                if ((pos = idlout.find( _IDL_StartMsg)) != std::string::npos)
                                        idlout = idlout.substr(pos+ _IDL_StartMsg.size());
        
                                if ((pos = idlout.rfind( _IDL_EndMsg)) != std::string::npos)
                                        {
                                        idlout_done = 1;
                                        idlout = idlout.substr(0, pos);
                                        }
                                }
                        }

                if ((n = read( PARENT_READERR, buffer, BUFLEN-1)) >0)
                        {
                        buffer[n]=0;
                        idlerr += std::string(buffer, n);
                        _logger->log( Logger::LOG_LEV_INFO, escapePercent(buffer).c_str());
                        printf("Error: %s\n", idlerr.c_str());
                        }
                else if (idlout_done == 1)
                        idlerr_done = 1;
                }


    if (out_string) 
       *out_string = idlout;
    if (err_string) 
       *err_string = idlerr;

    // Trap error code if requested
    if (ret_code)
        {
        pos = idlout.find( _IDL_ErrorCodeTrap);
        if (pos != std::string::npos)
                *ret_code = atoi( idlout.substr(pos+ _IDL_ErrorCodeTrap.size()).c_str());
        }
                
                
    return NO_ERROR;
}


int IDLCtrl::SendIDLcmdRpc( const char *cmd, int cmd_len, string *out_string, string *err_string, int *ret_code)
{
        string idlout = "";
        string idlerr = "";
        char newline = '\n';

        int ret = IDL_RPCExecuteStr( _pClient, (char*)cmd);

        if (ret==0) {
            _logger->log( Logger::LOG_LEV_ERROR, "Error in IDLRPCExecuteStr() [%s:%d], cmd = %s", __FILE__, __LINE__, cmd);
            if (out_string)
               *out_string = "";
            if (err_string)
               *err_string = "";
            return IDL_GENERIC_ERROR;
        }

        // Read stdout and stderr
        IDL_RPC_LINE_S pLine;
        char outbuffer[IDL_RPC_MAX_STRLEN];
        pLine.buf = outbuffer;

        while (IDL_RPCOutputGetStr( _pClient, &pLine, 0)) {

           // Log any output
           string tolog = escapePercent(pLine.buf);

           // Remove terminating newline (the logger will take care of that)
           while ( (tolog.size()>0) && (tolog[ tolog.size()-1]) == '\n')
             tolog = tolog.substr(0, tolog.size()-1);
           _logger->log( Logger::LOG_LEV_INFO, tolog.c_str());

           string append;
           if (pLine.flags & IDL_TOUT_F_NLPOST)
              append = newline;
           else
              append = "";

           if (pLine.flags & IDL_TOUT_F_STDERR)
              idlerr += pLine.buf + append;
           else
              idlout += pLine.buf + append;
        }

    if (out_string)
       *out_string = idlout;
    if (err_string)
       *err_string = idlerr;

    // Trap error code if requested
    if (ret_code)
        {
        unsigned int pos = idlout.find( _IDL_ErrorCodeTrap);
        if (pos != std::string::npos)
                *ret_code = atoi( idlout.substr(pos+ _IDL_ErrorCodeTrap.size()).c_str());
        }
                
                
    return NO_ERROR;
}

void IDLCtrl::Run() {

   while (!TimeToDie())
      msleep(1000);

   if (_rpc)
      IDL_RPCCleanup(_pClient,1);
}

void ctrlc( int /* signal */)
{
   printf("Ctrl-C received\n");
   if (idlPid >0)
      kill( idlPid, SIGINT);
}

std::string IDLCtrl::escapePercent( const char *str)
{
   std::string s= "";
   int n  =strlen(str);

   for (int i=0; i<n; i++)
      if (str[i] == '%')
         s += "%%";
      else
         s += str[i];

   return s;
}


int IDLCtrl::isValidCmd( const char *cmd, int cmd_len)
{  
   char spaces[] = " \n\r\t";

   std::string c(cmd, cmd_len);

   int start = c.find_first_not_of(spaces);
   int end   = c.find_last_not_of(spaces);
   if (start== -1)
      return 0;

   std::string real_cmd( cmd, start, end-start+1);

   if (real_cmd == ".r")
      return 0;

   return 1;
}

void IDLCtrl::notifyLicenseAlert() {

    // Send Notify to Arbitrator
    try {
        Error alert( "IDL license not found", true);
        _adSecArbNotifier->notifyAlert((Alert*)(&alert));
        _logger->log(Logger::LOG_LEV_INFO,"Alert for IDL license to adsec arbitrator successfully sent");
    } catch (AOException &e) {
       _logger->log(Logger::LOG_LEV_ERROR,"Alert for IDL license to adsec arbitrator failed: %s", e.what().c_str());
    }
}


int main( int argc, char **argv) {

   SetVersion(VersMajor,VersMinor);

   try {
       IDLCtrl *c;

       c = new IDLCtrl( argc, argv);

       c->Exec();

       delete c;
   } catch (AOException &e) {
       Logger::get()->log( Logger::LOG_LEV_FATAL, "%s:%d: %s", __FILE__, __LINE__, e.what().c_str());
   }
}

