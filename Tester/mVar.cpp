
#include "mVar.h"

extern "C" {
#include "base/thrdlib.h"
}



mVar::mVar( int argc, char *argv[]) : AOApp(argc, argv) {

   _target = "";

}

void mVar::InstallHandlers() {

   _logger->log(Logger::LOG_LEV_DEBUG, "Installing handler to receive variable messages.");
   int stat = NO_ERROR;

   if(!IS_ERROR(stat = thHandler(MVAR, "*", 0, mVar_incoming, "mVar_incoming", this))) {
      _logger->log(Logger::LOG_LEV_DEBUG, "Handler for incoming variables succesfully installed!");
   }
   else {
      _logger->log(Logger::LOG_LEV_ERROR, "Error (code %d) while installing incoming variables handler handler.", stat);
      throw AOException("Impossible to install incoming variables handler", NOT_INIT_ERROR);
   }

}


void mVar::PostInit() {

   _target = (string)ConfigDictionary()["Target"];
   _toExport = ConfigDictionary()["Variables"];

   // Do not catch errors - everything here is fatal
   for (vector<string>::iterator iter= _toExport.begin(); iter != _toExport.end(); iter++) {

      // First value
      int stat;
      MsgBuf *msgb = thReadVar( iter->c_str(), 0, &stat);
      while (msgb) {
         Variable *var = thValue(msgb);
         mVar_notif( this, var);
         Notify( var->H.Name, mVar_notif);
         msgb = thNextVar(msgb);
      }

   }
}


void mVar::Run() {

   while (!TimeToDie())
      msleep(1000);
}


int mVar::mVar_notif( void *thisPtr, Variable *var)
{
   mVar *me = (mVar *)thisPtr;

   printf("%s %s\n", me->_target.c_str(), var->H.Name);
   
   if (me->_target == "")
      return NO_ERROR;

   int size = sizeof(VHeader) + VarSize( var->H.Type, var->H.NItems);

   Logger::get()->log(Logger::LOG_LEV_DEBUG, "Sending %s to %s", var->H.Name, me->_target.c_str());
   return thSendMsg( size, me->_target.c_str(), MVAR, 0, var);
}

int mVar::mVar_incoming( MsgBuf* requestMsgBuf, void *argp, int /* queueSize */) {

   mVar *me = (mVar *)argp;
   Variable *var = (Variable*)requestMsgBuf->Msg->Body;

   // Discard messages for variables that we are exporting ourselves!
   for (vector<string>::iterator iter= me->_toExport.begin(); iter != me->_toExport.end(); iter++) 
      if ( iter->compare( var->H.Name) == 0)
         return NO_ERROR;

   Logger::get()->log(Logger::LOG_LEV_DEBUG, "Mirroring %s", var->H.Name);
   int stat= thWriteVar( var->H.Name, var->H.Type, var->H.NItems, var->Value.Sv, 2000);
   thRelease(requestMsgBuf);
   return stat;
}


int main( int argc, char *argv[]) {

   mVar *mvar = NULL;

   try {
      mvar = new mVar( argc, argv);
      mvar->Exec();

      delete mvar;
      }
   catch (LoggerFatalException &e) {
      // In this case the logger can't log!!!
      printf("%s\n", e.what().c_str());
   }
   catch (AOException &e) {
      Logger::get()->log(Logger::LOG_LEV_FATAL, "Error: %s", e.what().c_str());
   }

   return 0;
}
  

