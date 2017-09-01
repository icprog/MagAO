#include "VisAOPwrCtrl.h"

using namespace Arcetri;

// Mutex to lock out other threads during complex operations
pthread_mutex_t threadMutex;


VisAOPwrCtrl::VisAOPwrCtrl( int argc, char **argv) throw (AOException) : AOApp( argc, argv)
{
   Create();
}

void VisAOPwrCtrl::Create() throw (AOException)
{
   Logger::get()->log( Logger::LOG_LEV_TRACE, "VisAOPwrCtrl Create");
   pthread_mutex_init( &mutex, NULL);

   
   LoadConfig();
   
   network_ok = 0;
   errmsg = "";
   
   ERRMSG_LEN = 20;
   
   pthread_mutex_init(&threadMutex, NULL);
      
}      

int VisAOPwrCtrl::LoadConfig()
{
   Config_File *cfg = &ConfigDictionary();
   
   try 
   {
      PSaddr  = (char *) ((string)(*cfg)["IPaddr"]).c_str();
      PSport = (*cfg)["IPport"];
   } 
   catch (Config_File_Exception &e)
   {
      Logger::get()->log( Logger::LOG_LEV_FATAL, e.what().c_str());
      throw(e);
   }


   item_codes.resize(8);
   item_names.resize(8);
   char item_code[32];
   char item_name[32];
   
   for(int i=0; i< 8; i++)
   {

      snprintf( item_code, 32, "item%d.code", i);
      snprintf( item_name, 32, "item%d.name", i);

      try
      {
         item_codes[i]  = (std::string)(*cfg)[item_code];
      }
      catch (Config_File_Exception &e)
      {
         snprintf( item_code, 32, "OUTLET%d", i+1);
         item_codes[i]  = item_code;
      }  

      try
      {
         item_names[i]  = (std::string)(*cfg)[item_name];
      }
      catch (Config_File_Exception &e)
      {
         snprintf( item_name, 32, "OUTLET%d", i+1);
         item_names[i]  = item_name;
      }
   }
   
   return NO_ERROR;
}

void VisAOPwrCtrl::SetupVars()
{
   try 
   {
      var_ctype_cur = RTDBvar( this->MyFullName(), "ConMode", CUR_VAR, INT_VARIABLE, 1,1);
      var_ctype_req = RTDBvar( this->MyFullName(), "ConMode", REQ_VAR, INT_VARIABLE, 1,1);
      
      var_outlet1_cur = RTDBvar( this->MyFullName(), item_codes[0], CUR_VAR, INT_VARIABLE, 1,1);
      var_outlet1_req = RTDBvar( this->MyFullName(), item_codes[0], REQ_VAR, INT_VARIABLE, 1,1);
      var_outlet1_name = RTDBvar( this->MyFullName(), item_codes[0]+".NAME" , NO_DIR, CHAR_VARIABLE, item_names[0].size());
      var_outlet1_name.Set(item_names[0], FORCE_SEND);
      
      var_outlet2_cur = RTDBvar( this->MyFullName(), item_codes[1], CUR_VAR, INT_VARIABLE, 1,1);
      var_outlet2_req = RTDBvar( this->MyFullName(), item_codes[1], REQ_VAR, INT_VARIABLE, 1,1);
      var_outlet2_name = RTDBvar( this->MyFullName(), item_codes[1]+".NAME" , NO_DIR, CHAR_VARIABLE, item_names[1].size());
      var_outlet2_name.Set(item_names[1], FORCE_SEND);
      
      var_outlet3_cur = RTDBvar( this->MyFullName(), item_codes[2], CUR_VAR, INT_VARIABLE, 1,1);
      var_outlet3_req = RTDBvar( this->MyFullName(), item_codes[2], REQ_VAR, INT_VARIABLE, 1,1);
      var_outlet3_name = RTDBvar( this->MyFullName(), item_codes[2]+".NAME" , NO_DIR, CHAR_VARIABLE, item_names[2].size());
      var_outlet3_name.Set(item_names[2], FORCE_SEND);
      
      var_outlet4_cur = RTDBvar( this->MyFullName(), item_codes[3], CUR_VAR, INT_VARIABLE, 1,1);
      var_outlet4_req = RTDBvar( this->MyFullName(), item_codes[3], REQ_VAR, INT_VARIABLE, 1,1);
      var_outlet4_name = RTDBvar( this->MyFullName(), item_codes[3]+".NAME" , NO_DIR, CHAR_VARIABLE, item_names[3].size());
      var_outlet4_name.Set(item_names[3], FORCE_SEND);
      
      var_outlet5_cur = RTDBvar( this->MyFullName(), item_codes[4], CUR_VAR, INT_VARIABLE, 1,1);
      var_outlet5_req = RTDBvar( this->MyFullName(), item_codes[4], REQ_VAR, INT_VARIABLE, 1,1);
      var_outlet5_name = RTDBvar( this->MyFullName(), item_codes[4]+".NAME" , NO_DIR, CHAR_VARIABLE, item_names[4].size());
      var_outlet5_name.Set(item_names[4], FORCE_SEND);
      
      var_outlet6_cur = RTDBvar( this->MyFullName(), item_codes[5], CUR_VAR, INT_VARIABLE, 1,1);
      var_outlet6_req = RTDBvar( this->MyFullName(), item_codes[5], REQ_VAR, INT_VARIABLE, 1,1);
      var_outlet6_name = RTDBvar( this->MyFullName(), item_codes[5]+".NAME" , NO_DIR, CHAR_VARIABLE, item_names[5].size());
      var_outlet6_name.Set(item_names[5], FORCE_SEND);
      
      var_outlet7_cur = RTDBvar( this->MyFullName(), item_codes[6], CUR_VAR, INT_VARIABLE, 1,1);
      var_outlet7_req = RTDBvar( this->MyFullName(), item_codes[6], REQ_VAR, INT_VARIABLE, 1,1);
      var_outlet7_name = RTDBvar( this->MyFullName(), item_codes[6]+".NAME" , NO_DIR, CHAR_VARIABLE, item_names[6].size());
      var_outlet7_name.Set(item_names[6], FORCE_SEND);
      
      var_outlet8_cur = RTDBvar( this->MyFullName(), item_codes[7], CUR_VAR, INT_VARIABLE, 1,1);
      var_outlet8_req = RTDBvar( this->MyFullName(), item_codes[7], REQ_VAR, INT_VARIABLE, 1,1);
      var_outlet8_name = RTDBvar( this->MyFullName(), item_codes[7]+".NAME" , NO_DIR, CHAR_VARIABLE, item_names[7].size());
      var_outlet8_name.Set(item_names[7], FORCE_SEND);
   } 
   catch (AOVarException &e)  
   {
      Logger::get()->log(Logger::LOG_LEV_FATAL, "%s:%s: %s", __FILE__, __LINE__, e.what().c_str());
      throw AOException("Error creating RTDB variables");
   }
   
   this->Notify( var_ctype_req, ((VisAOPwrCtrl *)this)->CtypeReqChanged);
   this->Notify( var_outlet1_req, ((VisAOPwrCtrl *)this)->outlet1ReqChanged);
   this->Notify( var_outlet2_req, ((VisAOPwrCtrl *)this)->outlet2ReqChanged);
   this->Notify( var_outlet3_req, ((VisAOPwrCtrl *)this)->outlet3ReqChanged);
   this->Notify( var_outlet4_req, ((VisAOPwrCtrl *)this)->outlet4ReqChanged);
   this->Notify( var_outlet5_req, ((VisAOPwrCtrl *)this)->outlet5ReqChanged);
   this->Notify( var_outlet6_req, ((VisAOPwrCtrl *)this)->outlet6ReqChanged);
   this->Notify( var_outlet7_req, ((VisAOPwrCtrl *)this)->outlet7ReqChanged);
   this->Notify( var_outlet8_req, ((VisAOPwrCtrl *)this)->outlet8ReqChanged);
}
      

        
int VisAOPwrCtrl::SetupNetwork( int force)
{
   int stat;
   force = 2;
   char answer[512];
         
   ShutdownNetwork();
            
   Logger::get()->log( Logger::LOG_LEV_INFO, "Connecting to %s:%d", PSaddr, PSport);

    // Setup serial/network interface
   pthread_mutex_lock(&mutex);
   
   stat = SerialInit( PSaddr, PSport );	// This locks if network is down or host unreachable

   pthread_mutex_unlock(&mutex);
   
   if (stat != NO_ERROR)
      Logger::get()->log( Logger::LOG_LEV_ERROR, "Connect result: %d - Errno: %s", stat, strerror(errno));
   CHECK_SUCCESS(stat);
                                                                                                                                      
    // Set OK flag
   network_ok =1;
   errmsg = "Connected";

   Logger::get()->log( Logger::LOG_LEV_INFO, "Network connect OK");

   //Now clear the startup messages if necessary.
   EmptySerial(1, 2000);

   pthread_mutex_lock(&mutex);
   //Now login
   stat = sendCommand("@@@@\r\n");
   pthread_mutex_unlock(&mutex);
   
   if (stat < 0)
   {
      Logger::get()->log( Logger::LOG_LEV_TRACE, "Error logging in to Eaton PS");
      
      return stat;
   }
   
   answer[0] = 0;
   
   stat = SerialIn( answer, 26, 1000);
   
  
   CHECK_SUCCESS(stat);

   #ifdef _debug
      std::cout << "Answer to @@@@:\n" << answer+6 << "=================\n";
   #endif
    // Check if there actually was an answer
   if (stat <= 0)
   {
      Logger::get()->log( Logger::LOG_LEV_TRACE, "No answer to log on");

      return COMMUNICATION_ERROR;
   }
  
   //+6 to get past the start of message \r\n\0 stuff
   if(strstr(answer+6, "IPC ONLINE!") == 0)
   {
      Logger::get()->log( Logger::LOG_LEV_TRACE, "Incorrect answer to log on");
      return COMMUNICATION_ERROR;
   }
   
   return NO_ERROR;
}

int VisAOPwrCtrl::ShutdownNetwork()
{
   int stat = NO_ERROR;

   if ( network_ok)
   {
      pthread_mutex_lock(&mutex);
      Logger::get()->log( Logger::LOG_LEV_DEBUG, "VisAO Power: closing communication");
      stat = SerialClose();
      pthread_mutex_unlock(&mutex);
      CHECK_SUCCESS(stat);

      network_ok = 0;
      errmsg = "Not connected";
   }
   
   return stat;
}

int VisAOPwrCtrl::EmptySerial( int avoidLock, int timeout)
{
   char buffer[1024];

   if (!avoidLock)
      pthread_mutex_lock( &mutex);

   int n;
   while( (n= SerialInString( buffer, 1024-1, timeout, 0x0A)) > 0)
      Logger::get()->log( Logger::LOG_LEV_TRACE, "EmptySerial(): received string: %s", buffer);

   Logger::get()->log( Logger::LOG_LEV_TRACE, "EmptySerial(): exiting");

   if (!avoidLock)
      pthread_mutex_unlock( &mutex);

   return NO_ERROR;
}



int VisAOPwrCtrl::parseOutletNumber(const std::string &stStr)
{
   int on;
   
   if(stStr.length() < 8) 
   {
      #ifdef _debug  
         std::cerr << "Too Short\n";
      #endif
      return -1;
   }
   
   on = stStr[7]-48;
   
   if(on < 1 || on > 8) 
   {
      #ifdef _debug 
         std::cerr << "Bad number\n";
         std::cerr << on << " " << stStr[7] << "\n";
      #endif
      return -1;
   }
   
   return on;
}

int VisAOPwrCtrl::parseOutletStatus(const std::string &stStr)
{
   if(stStr.length() < 11) return -1;
   
   if(stStr[10] == 'F') return 0;
   if(stStr[10] == 'N') return 1;

   return -1;
}


int VisAOPwrCtrl::parseStatus(const std::string &ansstr)
{
   int st, en;
   int on,os;
   st = ansstr.find_first_not_of("\r\n\0");
   
   for(int i=0; i < 8; i++)
   {
      if(st < 0)
      {
         #ifdef _debug 
            std::cerr << "Parser error (st), can't read status\n";
         #endif
         return -1;
      }
      
      en = ansstr.find_first_of("\n", st+1);
      
      if(en < 0)
      {
         #ifdef _debug 
            std::cerr << "Parser error (en), can't read status\n";
         #endif
         return -1;
      }
      
      if(en - st < 15) 
      {
         #ifdef _debug 
            std::cerr << "Bad parse, string not long enough\n";
         #endif
         return -1;
      }
      else
      {
         on = parseOutletNumber(ansstr.substr(st, en));
          
         os = parseOutletStatus(ansstr.substr(st, en));
         
         if(on != i+1)
         {
            #ifdef _debug 
               std::cerr << "Bad outlet number\n";
            #endif
            return -1;
         }
         else outletState[i] = os;
      }
      
      st = ansstr.find_first_not_of("\r\n\0", en);
   }
   return 0;
}

int VisAOPwrCtrl::sendOutletState(int on, int os)
{
   switch(on)
   {
      case 1:
         var_outlet1_cur.Set(os, 0, CHECK_SEND);
         break;
      case 2:
         var_outlet2_cur.Set(os, 0, CHECK_SEND);
         break;
      case 3:
         var_outlet3_cur.Set(os, 0, CHECK_SEND);
         break;
      case 4:
         var_outlet4_cur.Set(os, 0, CHECK_SEND);
         break;
      case 5:
         var_outlet5_cur.Set(os, 0, CHECK_SEND);
         break;
      case 6:
         var_outlet6_cur.Set(os, 0, CHECK_SEND);
         break;
      case 7:
         var_outlet7_cur.Set(os, 0, CHECK_SEND);
         break;
      case 8:
         var_outlet8_cur.Set(os, 0, CHECK_SEND);
         break;
      default:
         return -1;
   }
   return 0;
}

         
int VisAOPwrCtrl::TestNetwork(void)
{
   int stat;
   char answer[512], *tans;
   
   Logger::get()->log( Logger::LOG_LEV_TRACE, "VisAO Power: Testing network, network_ok=%d", network_ok);
   if (!network_ok)
      SetupNetwork();
   
   pthread_mutex_lock(&mutex);
   
   EmptySerial( 1, 10);
   stat = sendCommand("DX0\r\n");
   
   if (stat<0)
   {
      Logger::get()->log( Logger::LOG_LEV_TRACE, "VisAO Power: Error sending test command");
      pthread_mutex_unlock(&mutex);
      return stat;
   }
   
   //Eaton IPC3401-NET responds in several \r\0 terminated lines.  Have to read each individually.
   int totres = 0;
   stat = 0;
   for(int i=0; i < 9; i++)
   {
      stat = SerialInString(answer + totres, 512-totres, 5000, '\r');
      if(stat <= 0) break;
                         totres += stat;
   }
   
   
   pthread_mutex_unlock(&mutex);
   
   
   CHECK_SUCCESS(stat);
   
   // Check if there actually was an answer
   if (stat <= 0)
   {
      Logger::get()->log( Logger::LOG_LEV_TRACE, "No answer to test command");
      return COMMUNICATION_ERROR;
   }
   
   //Now clear out all the \0 in the buffer
   tans = answer + 6;
   for(int k = 0; k < 230; k++) if(tans[k] == '\0') tans[k] = '\n';
                         
   std::string ansstr = tans;
   if(parseStatus(ansstr) < 0)
   {
      std::cerr << "VisAO Power: parse failure in network test\n";
      Logger::get()->log( Logger::LOG_LEV_TRACE, "parse failure in network test");
      return COMMUNICATION_ERROR;
   }
   
   #ifdef _debug
   for(int k=0;k < 8; k++) std::cout << k+1 << " " << outletState[k] << "\n";
   #endif
   
   for(int k=0;k < 8; k++) sendOutletState(k+1, outletState[k]);

   Logger::get()->log( Logger::LOG_LEV_TRACE, "Network test OK", tans);
   return NO_ERROR;
}

void VisAOPwrCtrl::Run()
{
   _logger->log( Logger::LOG_LEV_INFO, "Running...");

   while(!TimeToDie()) 
   {
      try 
      {
         DoFSM();
      } 
      catch (AOException &e) 
      {
         _logger->log( Logger::LOG_LEV_ERROR, "Caught exception (at %s:%d): %s", __FILE__, __LINE__, e.what().c_str());

         // When the exception is thrown, the mutex was held!
         pthread_mutex_unlock(&threadMutex);
      }
   }
}
      
    
int VisAOPwrCtrl::DoFSM()
{
   // [todo] This contains blocking methods, like TestNetwork(),
   // and doesn't allow the AOApp to die !
   
   int status;//, stat;
   static float delay=1.0;
   
   // Lock out everyone else!!
   pthread_mutex_lock(&threadMutex);
   
   status = getCurState();
   
   // Always check the network
   if (status != STATE_NOCONNECTION)
   {
      if(TestNetwork() != NO_ERROR)
      {
         setCurState(STATE_NOCONNECTION);
         for(int k=0;k<8;k++) sendOutletState(k, 2); //Sets state to unreachable
         status = STATE_NOCONNECTION; //so we aren't STATE_CONNECTED in switch statement.
      }
      else setCurState(STATE_OPERATING);
   }

   
   switch(status)
   {
      case STATE_NOCONNECTION:
         if (SetupNetwork() == NO_ERROR)
         {
            if (TestNetwork() == NO_ERROR) setCurState(STATE_CONNECTED);
            else
            {
               for(int k=0;k<8;k++) sendOutletState(k, 2); //Sets state to unreachable
               ShutdownNetwork();
            }
         }
         else for(int k=0;k<8;k++) sendOutletState(k, 2); //Sets state to unreachable
         break;
      case STATE_CONNECTED:
         //Add a ping here.
         setCurState(STATE_OPERATING);
         break;
   }
   // Always set current status (for external watchdog)
   setCurState( getCurState());
   pthread_mutex_unlock(&threadMutex);
   nusleep( (unsigned int)(delay * 1e6));
   
   return NO_ERROR;
}
    


int VisAOPwrCtrl::CtypeReqChanged(void *pt, Variable *msgb)
{
   int newstate;
   pthread_mutex_lock(&threadMutex);
   
   newstate = msgb->Value.Lv[0];
   
   pthread_mutex_unlock(&threadMutex);

   pt = 0;
   return 0;
}


int VisAOPwrCtrl::outlet1ReqChanged(void *pt, Variable *msgb)
{
   int stat, newstate;
   
   pthread_mutex_lock(&threadMutex);
   
   newstate = msgb->Value.Lv[0];

   if(newstate == 1) 
   {
      stat = ((VisAOPwrCtrl *)pt)->sendCommand("N01\r\n"); //This isn't really possible if this is the VisAO switch
      ((VisAOPwrCtrl *)pt)->sendOutletState(1, 1);
   }
   if(newstate == 0) 
   {
      stat = ((VisAOPwrCtrl *)pt)->sendCommand("SR0\r\n"); //Outlet 1 has switch, so this is different
      ((VisAOPwrCtrl *)pt)->sendOutletState(1, 0);
   }
   pthread_mutex_unlock(&threadMutex);
   
   return stat;
}

int VisAOPwrCtrl::outlet2ReqChanged(void *pt, Variable *msgb)
{
   int newstate;
   
   newstate = msgb->Value.Lv[0];

   return ((VisAOPwrCtrl *)pt)->setOutletState(2, newstate);
   
}

int VisAOPwrCtrl::outlet3ReqChanged(void *pt, Variable *msgb)
{
   int newstate;
   
   newstate = msgb->Value.Lv[0];

   return ((VisAOPwrCtrl *)pt)->setOutletState(3, newstate);
   
}

int VisAOPwrCtrl::outlet4ReqChanged(void *pt, Variable *msgb)
{
   int newstate;
   
   newstate = msgb->Value.Lv[0];

   return ((VisAOPwrCtrl *)pt)->setOutletState(4, newstate);
   
}

int VisAOPwrCtrl::outlet5ReqChanged(void *pt, Variable *msgb)
{
   int newstate;
   
   newstate = msgb->Value.Lv[0];

   return ((VisAOPwrCtrl *)pt)->setOutletState(5, newstate);
   
}

int VisAOPwrCtrl::outlet6ReqChanged(void *pt, Variable *msgb)
{
   int newstate;
   
   newstate = msgb->Value.Lv[0];

   return ((VisAOPwrCtrl *)pt)->setOutletState(6, newstate);
   
}

int VisAOPwrCtrl::outlet7ReqChanged(void *pt, Variable *msgb)
{
   int newstate;
   
   newstate = msgb->Value.Lv[0];

   return ((VisAOPwrCtrl *)pt)->setOutletState(7, newstate);
   
}

int VisAOPwrCtrl::outlet8ReqChanged(void *pt, Variable *msgb)
{
   int newstate;
      
   newstate = msgb->Value.Lv[0];

   return ((VisAOPwrCtrl *)pt)->setOutletState(8, newstate);
   
}

int VisAOPwrCtrl::sendCommand(const char * com)
{
   #ifdef _debug
      std::cout << "Sending " << com << "\n";
   #endif
   
   return SerialOut(com, strlen(com));
   
}

int VisAOPwrCtrl::setOutletState(int outnum, int outstate)
{
   int stat;
   
   char com[6], answer[512];
   
   if(outnum == 1) return -1;
   
   if(outstate == 0) com[0] = 'F';
   else com[0] = 'N';
   
   com[1] = '0';
   
   com[2] = 48 + outnum;
   
   com[3] = '\r';
   com[4] = '\n';
   com[5] = '\0';
   
   pthread_mutex_lock(&mutex);
   
   EmptySerial( 1, 1);
   stat = sendCommand(com);
   
   usleep(100);
     
   if (stat<0)
   {
      Logger::get()->log( Logger::LOG_LEV_TRACE, "VisAO Power: Error outlet state change");
      pthread_mutex_unlock(&mutex);
      return stat;
   }
  
   stat = SerialInString( answer, 512, 1000, 10);
   pthread_mutex_unlock(&mutex);
   CHECK_SUCCESS(stat);

   //Check if there actually was an answer
   if (stat <= 0)
   {
      Logger::get()->log( Logger::LOG_LEV_ERROR, "No answer to outlet state change request");

      return COMMUNICATION_ERROR;
   }
      
   if(strstr(answer+6, "DONE") == 0)
   {
      Logger::get()->log( Logger::LOG_LEV_ERROR, "Unexpected response to outlet state change request.");
      return COMMUNICATION_ERROR;
   }
   
   sendOutletState(outnum, outstate);
   return NO_ERROR;
};
   



