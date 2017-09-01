
#ifndef IDL_CTRL_H_INCLUDED
#define IDL_CTRL_H_INCLUDED

#include <string>
#include <pthread.h>    // pthread_mutex*
#include "AOApp.h"

#include "arblib/base/ArbitratorInterface.h"

extern "C" {
	#include "idllib/idllib.h"
	#include "idl_export.h"
	#include "idl_rpc.h"
}

//@Class: IDLCtrl
class IDLCtrl: public AOApp {

	public:
	
	   	IDLCtrl( int argc, char **argv) throw (AOException);
	
	   	void Create();
	
		~IDLCtrl();

	protected:
	   
	   	void SetupVars();
	   	void InstallHandlers();
	   	void PostInit();
		void Run();

      void spawnIdlCtrl();
      void spawnIdlRpc();
      void connectIdlRpc();


      string expandVars( string s);

      pthread_mutex_t   cmd_mutex;
      pthread_t         license_thread;

      static void *licenseThread( void *ctrl);
      void notifyLicenseAlert();

      int demoModeLooptime();
	
	   	// VIRTUAL message handlers
	
	protected:
	
	   	static int IDLcmd_handler( MsgBuf *msgb, void *argp, int /*thQueueSize*/);
	   	static int IDLgetvar_handler( MsgBuf *msgb, void *argp, int /*thQueueSize*/);
	   	static int IDLlock_handler( MsgBuf *msgb, void *argp, int /*thQueueSize*/);
	   	static int IDLunlock_handler( MsgBuf *msgb, void *argp, int /*thQueueSize*/);
		static int IDLctrlc_handler(MsgBuf *msgb, void *argp, int /*thQueueSize*/);
		//static void ctrlc(int);
		static void OutFunc(int flags, char *buf, int n);
	
	protected:
	
		//void RPCFlushOutput();
	   	int SendIDLcmd( const char *cmd, int cmd_len, string *out_string, string *err_string, int *ret_code = NULL);
	   	int SendIDLcmdCtrl( const char *cmd, int cmd_len, string *out_string, string *err_string, int *ret_code = NULL);
	   	int SendIDLcmdRpc( const char *cmd, int cmd_len, string *out_string, string *err_string, int *ret_code = NULL);
	
		// Returns 1 when a command is valid, 0 otherwise
	   	int isValidCmd( const char *cmd, int cmd_len);
	
	public:
	
	   	std::string  _lockedBy;
	
	protected:

         int _errcode;
         int _demoModeLooptime;

         /* Old version */
	
		 std::string  _IDL_path;
		 std::string  _IDL_path_rpc;
		 std::string  _IDL_ExeCmd;
		 std::string  _IDL_ExeCmd2;
		 std::string  _IDL_EndCmd;
		 std::string  _IDL_EndMsg;
		 std::string  _IDL_StartCmd;
		 std::string  _IDL_StartMsg;
		 std::string  _IDL_ErrorCodeTrap;

       std::string escapePercent( const char *str);

       // 1 for an IDL RPC session
       // 0 for IDL session with stdin/out/err redirection
       int          _rpc;
       CLIENT *     _pClient;

	   	int _myPid;
	   	RTDBvar _pidVar;
         RTDBvar _errcodeVar;
         RTDBvar _rpcVar;

       AlertNotifier* _adSecArbNotifier; 


   public:
         RTDBvar _demoModeVar;
	
};

#endif // IDL_CTRL_H_INCLUDED
