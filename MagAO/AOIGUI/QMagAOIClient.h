
#ifndef __QMagAOIClient_h__
#define __QMagAOIClient_h__


#include "MagAOIClient.h"
#include <qobject.h>

class QMagAOIClient : public QObject, public MagAOIClient
{
   Q_OBJECT

   public:
      QMagAOIClient( int argc, char **argv, QObject *parent = 0, const char *name = 0) throw(AOException);

      //Overridden from MagAOIClient, lets us emit signals when we want too
      virtual void post_update_DD_var(DD_RTDBVar &var);
   
      virtual void Run();

      pthread_t exec_thread;

      void thread_exec();

      void get_nudegeae_rot(double &ang, double &parity);

   signals:
      void some_signal(std::string msg);

};


#endif //__QMagAOIClient_h__
