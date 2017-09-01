
#include "QMagAOIClient.h"

QMagAOIClient::QMagAOIClient( int argc, char **argv, QObject *parent, const char *name) throw(AOException) : QObject(parent, name), MagAOIClient(argc, argv)

{
   return ;
}

void QMagAOIClient::post_update_DD_var(DD_RTDBVar &var)
{
   return;
}

void QMagAOIClient::Run()
{
   _logger->log( Logger::LOG_LEV_INFO, "AOApp Running...");

   init_DD();

   while(!TimeToDie())
   {
      sleep(1); //We just wait for TimeToDie from msgD
   }
}

void * __thread_exec(void *ptr)
{
   QMagAOIClient * aoic = (QMagAOIClient *) ptr;

   aoic->Exec();
}

void QMagAOIClient::thread_exec()
{
   pthread_create(&exec_thread, 0, &__thread_exec, (void *) this);
}


void QMagAOIClient::get_nudegeae_rot(double &ang, double &parity)
{
   Config_File *cfg = &ConfigDictionary();
   try 
   {

      ang = (int)(*cfg)["nudgeae_rot_ang"];
      parity = (int)(*cfg)["nudgeae_rot_par"];
   }
   catch (Config_File_Exception &e)
   {
      Logger::get()->log( Logger::LOG_LEV_FATAL, "Missing NudgeAE configuration: %s", e.what().c_str());
      throw(e);
   }
}


