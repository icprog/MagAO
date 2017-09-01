#include "gainSetter.h"



//#include "arblib/base/Commands.h"
// #include "AOGlobals.h"
// #include "arblib/base/CommandsExport.h"
// #include "arblib/base/AlertsExport.h"
//#include "arblib/adSecArb/AdSecCommands.h"
#include "arblib/adSecArb/AdSecCommandsExport.h"


using namespace Arcetri;
using namespace Arcetri::Arbitrator;
using namespace Arcetri::AdSec_Arbitrator;


#include <boost/thread/thread.hpp>
#include <boost/bind.hpp>


using namespace Arcetri;

//#define _debug


//#include "arblib/aoArb/AOCommandsExport.h"

gainSetter::gainSetter( int argc, char **argv) throw (AOException) : AOApp( argc, argv)
{
   Create();
}

gainSetter::~gainSetter()
{
   if(_arbIntf) delete _arbIntf;

}



void gainSetter::Create() throw (AOException)
{   
   _arbName = "adsecarb";
   _arbMsgdId = _arbName + "." + Side();

   _arbIntf = NULL;

   LoadConfig();
   
}//void gainSetter::Create()

int gainSetter::LoadConfig()
{
   //Config_File *cfg = &ConfigDictionary();
   try 
   {
      return 0;
   }
   catch (Config_File_Exception &e)
   {
      Logger::get()->log( Logger::LOG_LEV_FATAL, "Missing config data: %s", e.what().c_str());
      throw(e);
   }

   return 0;
}//int gainSetter::LoadConfig()

void gainSetter::SetupVars()
{

   //Setup the RTDB variables
   try
   {
      var_gain_tt_cur = RTDBvar( MyFullName(), "gain_tt", CUR_VAR, REAL_VARIABLE, 1, 1);
      var_gain_tt_req = RTDBvar( MyFullName(), "gain_tt", REQ_VAR, REAL_VARIABLE, 1, 1);
      var_gain_ho1_cur = RTDBvar( MyFullName(), "gain_ho1", CUR_VAR, REAL_VARIABLE, 1, 1);
      var_gain_ho1_req = RTDBvar( MyFullName(), "gain_ho1", REQ_VAR, REAL_VARIABLE, 1, 1);
      var_gain_ho2_cur = RTDBvar( MyFullName(), "gain_ho2", CUR_VAR, REAL_VARIABLE, 1, 1);
      var_gain_ho2_req = RTDBvar( MyFullName(), "gain_ho2", REQ_VAR, REAL_VARIABLE, 1, 1);

      var_gain_tt0_cur = RTDBvar( MyFullName(), "gain_tt0", CUR_VAR, REAL_VARIABLE, 1, 1);
      var_gain_tt0_req = RTDBvar( MyFullName(), "gain_tt0", REQ_VAR, REAL_VARIABLE, 1, 1);

      var_gain_tt1_cur = RTDBvar( MyFullName(), "gain_tt1", CUR_VAR, REAL_VARIABLE, 1, 1);
      var_gain_tt1_req = RTDBvar( MyFullName(), "gain_tt1", REQ_VAR, REAL_VARIABLE, 1, 1);


      var_nmodes = RTDBvar( MyFullName(), "nmodes", NO_DIR, INT_VARIABLE, 1, 1);
      var_homiddle = RTDBvar( MyFullName(), "homiddle", NO_DIR, INT_VARIABLE, 1, 1);

      var_gain_file = RTDBvar("ADSEC.L", "G_GAIN_A", NO_DIR, CHAR_VARIABLE, 512);

      var_recmat_file = RTDBvar("ADSEC.L", "B0_A", NO_DIR, CHAR_VARIABLE, 512);

      Notify(var_gain_tt_req, gain_tt_req_changed);
      Notify(var_gain_ho1_req, gain_ho1_req_changed);
      Notify(var_gain_ho2_req, gain_ho2_req_changed);
      Notify(var_gain_file, gain_file_changed);
      Notify(var_recmat_file, recmat_file_changed);

      Notify(var_gain_tt0_req, gain_tt0_req_changed);
      Notify(var_gain_tt1_req, gain_tt1_req_changed);

   }
   catch (AOVarException &e)  
   {
      Logger::get()->log(Logger::LOG_LEV_FATAL, "%s:%i: %s", __FILE__, __LINE__, e.what().c_str());
      throw AOException("Error creating RTDB variables");
   }
}//void gainSetter::SetupVars()

void gainSetter::PostInit()
{
   //_aoArb = "AOARB.L";
   _arbIntf = new ArbitratorInterface(_arbMsgdId);

   std::string file;
  
   var_recmat_file.Update();
   file = var_recmat_file.Get();
   updateRecmat(file);

   var_gain_file.Update();
   file = var_gain_file.Get();
   parseModalBasis(file);
   parseGains(file, true);

   

}//void gainSetter::PostInit()

void gainSetter::Run()
{
   while(!TimeToDie()) 
   {
      try 
      {
         DoFSM();
      } 
      catch (AOException &e) 
      {
         _logger->log( Logger::LOG_LEV_ERROR, "Caught exception (at %s:%d): %s", __FILE__, __LINE__, e.what().c_str());
      }
   }
}//void gainHunger::Run()


int gainSetter::DoFSM()
{
   int status;

   status = STATE_OPERATING;
   


   setCurState(status);


   usleep( (unsigned int)(1.0 * 1e6));

   return NO_ERROR;

}// int gainSetter::DoFSM()

int gainSetter::gain_tt_req_changed(void *pt, Variable *msgb __attribute__((unused)))
{
   gainSetter * gh = (gainSetter *) pt;

   double tt;

   gh->var_gain_tt_req.Update();
   gh->var_gain_tt_req.Get(&tt);

   return gh->setGains(tt, -1, -1);
}

int gainSetter::gain_ho1_req_changed(void *pt, Variable *msgb __attribute__((unused)))
{
   gainSetter * gh = (gainSetter *) pt;

   double ho1;

   gh->var_gain_ho1_req.Update();
   gh->var_gain_ho1_req.Get(&ho1);

   return gh->setGains(-1, ho1, -1);
}

int gainSetter::gain_ho2_req_changed(void *pt, Variable *msgb __attribute__((unused)))
{
   gainSetter * gh = (gainSetter *) pt;

   double ho2;

   gh->var_gain_ho2_req.Update();
   gh->var_gain_ho2_req.Get(&ho2);

   return gh->setGains(-1, -1, ho2);
}

int gainSetter::gain_tt0_req_changed(void *pt, Variable *msgb __attribute__((unused)))
{
   gainSetter * gh = (gainSetter *) pt;

   double tt0;

   gh->var_gain_tt0_req.Update();
   gh->var_gain_tt0_req.Get(&tt0);

   return gh->setGains(tt0, -1, -1, -1);
}

int gainSetter::gain_tt1_req_changed(void *pt, Variable *msgb __attribute__((unused)))
{
   gainSetter * gh = (gainSetter *) pt;

   double tt1;

   gh->var_gain_tt1_req.Update();
   gh->var_gain_tt1_req.Get(&tt1);

   return gh->setGains(-1, tt1, -1, -1);
}

int gainSetter::gain_file_changed(void *pt, Variable *msgb __attribute__((unused)))
{
   gainSetter * gh = (gainSetter *) pt;

   std::string file;

   gh->var_gain_file.Update();
   file = gh->var_gain_file.Get();

   gh->parseModalBasis(file);
   return gh->parseGains(file, true);
}  

int gainSetter::recmat_file_changed(void *pt, Variable *msgb __attribute__((unused)))
{
   gainSetter * gh = (gainSetter *) pt;
   

   std::string file;

   gh->var_recmat_file.Update();
   file = gh->var_recmat_file.Get();

   return gh->updateRecmat(file);
}

int gainSetter::setGains(double tt, double ho1, double ho2)
{
   if(tt < 0)
   {
      var_gain_tt_cur.Update();
      var_gain_tt_cur.Get(&tt);
   }

   if(ho1 < 0)
   {
      var_gain_ho1_cur.Update();
      var_gain_ho1_cur.Get(&ho1);
   }

   if(ho2 < 0)
   {
      var_gain_ho2_cur.Update();
      var_gain_ho2_cur.Get(&ho2);
   }

   std::string newGainFile = writeGains(tt, ho1, ho2);
   
   gainParams params;
   params._gainFile = newGainFile;
   Command *cmd = new SetGain(200*1000, params, "");


   sendCommandThread(cmd);

   return 0;
}

int gainSetter::setGains(double tt0, double tt1, double ho1, double ho2)
{
   if(tt0 < 0)
   {
      var_gain_tt0_cur.Update();
      var_gain_tt0_cur.Get(&tt0);
   }

   if(tt1 < 0)
   {
      var_gain_tt1_cur.Update();
      var_gain_tt1_cur.Get(&tt1);
   }

   if(ho1 < 0)
   {
      var_gain_ho1_cur.Update();
      var_gain_ho1_cur.Get(&ho1);
   }

   if(ho2 < 0)
   {
      var_gain_ho2_cur.Update();
      var_gain_ho2_cur.Get(&ho2);
   }

   std::string newGainFile = writeGains(tt0, tt1, ho1, ho2);
   
   std::cout << "newGainFile :" << newGainFile << "\n";

   gainParams params;
   params._gainFile = newGainFile;
   Command *cmd = new SetGain(200*1000, params, "");


   sendCommandThread(cmd);

   return 0;
}

void gainSetter::sendCommandWithReply(Command* cmd, Command **reply) {

   Command* cmdReply;
   try
   {
      //postSetMessage("Executing " + cmd->getDescription() + " command...");
      _logger->log(Logger::LOG_LEV_INFO, " >> Sending command %s request...", cmd->getDescription().c_str());
      cmdReply = _arbIntf->requestCommand(cmd);
      _logger->log(Logger::LOG_LEV_INFO, " >> Command reply (%s) received!", cmdReply->getStatusAsString().c_str());
      //postSetMessage(cmd->getDescription() + " command reply received");
      //processReply( cmdReply); // Virtual
      cmdReply->log();
      if (reply)
         *reply = cmdReply;
      else
         delete cmdReply;
   }
   catch(...)
   {
      //setMessage(cmd->getDescription() + " command has timed out");
      //postAlert(cmd->getDescription() + " command has timed out");
   }

   delete cmd;

}

void gainSetter::sendCommand(Command* cmd)
{
   sendCommandWithReply( cmd, NULL);
}

void gainSetter::sendCommandThread(Command* cmd) {

   // Launch the new command in a thread: the command is deleted by called method!!!
   boost::thread* thCommand = new boost::thread(boost::bind(&gainSetter::sendCommand, this, cmd));

   delete thCommand; // The thread is not interrupted (see Boost documentation!)
   thCommand = NULL;
}



int gainSetter::parseGains(std::string & file, bool updateCur)
{
   size_t pos;

   double tt=0;
   double ho1=0;
   double ho2=0;
   double tt0 = 0;
   double tt1 = 0;

   pos = file.find("tt0_");

   if(pos != std::string::npos)
   {
      tt0 = atof(file.substr(pos+4,4).c_str());

      pos = file.find("tt1_");
      if (pos != std::string::npos)
         tt1 = atof(file.substr(pos+4,4).c_str());
      
      tt = 0.5*(tt0+tt1);
   }
   else
   {     
      pos = file.find("tt_");
      if (pos != std::string::npos)
         tt = atof(file.substr(pos+3,4).c_str());

      tt0 = tt;
      tt1 = tt;
   }

   pos = file.find("ho1_");
   if (pos != std::string::npos)
       ho1 = atof(file.substr(pos+4,4).c_str());
   pos = file.find("ho2_");
   if (pos != std::string::npos)
       ho2 = atof(file.substr(pos+4,4).c_str());

   if(updateCur)
   {
      var_gain_tt_cur.Set(tt, 0);
      var_gain_ho1_cur.Set(ho1, 0);
      var_gain_ho2_cur.Set(ho2, 0);

      var_gain_tt0_cur.Set(tt, 0);
      var_gain_tt1_cur.Set(tt, 0);
   }
      


   return 0;
}




int gainSetter::parseModalBasis(std::string & file)
{
   size_t pos1, pos2;

   pos1 = file.rfind("/gain/");
   if (pos1 == std::string::npos)
   {
      modalBasis = "";
      return 0;
   }

   pos2 = file.rfind("/", pos1-1);
   if (pos2 == std::string::npos)
   {
      modalBasis = "";
      return 0;
   }
      
   modalBasis = file.substr(pos2+1, pos1-pos2 - 1);
   

   return 0;
}

int gainSetter::updateRecmat(std::string & file)
{
   char buf[256];
   ostringstream oss;
   int stat;
 
   if (file == recMat)
       return 0;

   recMat = file;

   if(recMat == "")
   {
      nModes = 0;
      hoMiddle = 0;
   }
   else
   {

      stat = ReadFitsKeyword( (char*)recMat.c_str(), "IM_MODES", TSTRING, buf);
      nModes = atoi(buf);
      //printf("Updating gain ranges for %d modes\n", nmodes);

      hoMiddle = 200;
      if (nModes<=300)
         hoMiddle = 200;
      if (nModes<=200)
         hoMiddle = 100;
      if (nModes<=100)
          hoMiddle = 66;
   }

   std::cout << nModes << "\n";

   var_nmodes.Set(nModes,0, FORCE_SEND);
   var_homiddle.Set(hoMiddle,0, FORCE_SEND);

   return 0;
    
}

std::string gainSetter::writeGains( double tt, double ho1, double ho2)
{
   if(modalBasis == "")
   {
      return "";
   }

   std::string gainDir = Paths::GainDir( modalBasis, true);
   char buf[128];
   if (tt<0) tt=0;
   if (ho1<0) ho1=0;
   if (ho2<0) ho2=0;

   sprintf(buf, "g_tt_%4.2f_ho1_%4.2f_ho2_%4.2f.fits", tt, ho1, ho2);

   float gainVector[672];

   gainVector[0] = tt;
   gainVector[1] = tt;

   for (int i=2; i< hoMiddle; i++)
      gainVector[i] = ho1;
   for (int i=hoMiddle; i<672; i++)
      gainVector[i] = ho2;

   long dims[] = {672};
   std::string outfile = gainDir + buf;
   std::cout << "WriteFitsFile: " << outfile << "\n";
   //int stat = 
   WriteFitsFile( (char*)outfile.c_str(), (unsigned char *)gainVector, TFLOAT, dims, 1);
   return outfile;
}

std::string gainSetter::writeGains( double tt0, double tt1, double ho1, double ho2)
{
   if(modalBasis == "")
   {
      return "";
   }

   std::string gainDir = Paths::GainDir( modalBasis, true);
   char buf[128];
   if (tt0<0) tt0=0;
   if (tt1<0) tt1=0;
   if (ho1<0) ho1=0;
   if (ho2<0) ho2=0;

   sprintf(buf, "g_tt0_%4.2f_tt1_%4.2f_ho1_%4.2f_ho2_%4.2f.fits", tt0, tt1, ho1, ho2);

   float gainVector[672];

   gainVector[0] = tt0;
   gainVector[1] = tt1;

   for (int i=2; i< hoMiddle; i++)
      gainVector[i] = ho1;
   for (int i=hoMiddle; i<672; i++)
      gainVector[i] = ho2;

   long dims[] = {672};
   std::string outfile = gainDir + buf;
   std::cout << "WriteFitsFile: " << outfile << "\n";
   //int stat = 
   WriteFitsFile( (char*)outfile.c_str(), (unsigned char *)gainVector, TFLOAT, dims, 1);
   return outfile;
}

