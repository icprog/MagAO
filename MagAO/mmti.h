#ifndef __mmti_h__
#define __mmti_h__

#include "MagAOIClient/MagAOIClient.h"

struct mythread
{
   pthread_t th;
   int connected;
};

class mmti : public MagAOIClient
{
   public:
      mmti(int argc, char **argv) throw (AOException);
      mmti(std::string name, const std::string &conffile) throw(AOException);

   protected:
      void Create(void) throw(AOException);

      int LoadConfig();

      virtual void SetupVars();

      virtual void Run();

      virtual void post_update_DD_var(DD_RTDBVar &var);

      int port; ///<The port used for connections.

      int n_th; ///<The number of tcp connection threads.
      std::vector<mythread> tcpth_v; ///<Contains the thread status for the tcp server

      pthread_mutex_t thmutex;///<Mutex fot the tcp threads

      int s00; ///< Socket for the MMT server connections.

      ///Process mmt style requests
      int mmt_request(std::string &_req, FILE *fp);

      ///Respond to the all request
      void send_all(FILE *fp);

      int offset_params_expected;///<Flag to indicate that next request is an offset parameter
      double offset_az; ///<Offset in AZ
      double offset_el; ///<Offset in EL

      double offset_az_total; ///<Total applied az offset, used to go_home
      double offset_el_total; ///<Total applied el offset, used to go_home

      ///Apply the offset by sending the NodXY MagAOI Command.
      void offset(double ora, double odec, FILE *fp);

      bool focus_param_expected;
      double focus_val;

      void focus(double fv, FILE *fp);
      
      std::ostringstream logss;
   public:
      ///Listens on the MMT server port and sends incoming requests to mmt_request.
      /** This is the worker for the tcp server threads.
        */
      void listen_on_socket();  

      ///Log and print a message to stdout
      void log_msg(std::string msg, int lvl=Logger::LOG_LEV_INFO);
};

/// set up a listen socket
/** \param port is the port to open the socket on
  * \retval -1 on error
  * \retval 0 on success
  */ 
int lsocket (int port);


///Launch the listening thread
/** Calls mmti::liston_on_socket()
  */
void * __listen_on_socket(void *vmmti);

///Strip the newline character from a string.
std::string strip_newl(std::string &nl);

///Switch a string to all lower case
std::string str_to_lower(std::string &up);

///Write a string to a FILE pointer
int mmti_fputs(char *string,FILE *fp);

///Get a string from a FILE pointer
int mmti_fgets(char *string,int length,FILE *fp);

#endif





