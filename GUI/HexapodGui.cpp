#define VERS_MAJOR   1
#define VERS_MINOR   0

#include "HexapodGui.h"

extern "C" {
#include "aoslib/aoslib.h"
#include "base/timelib.h"
}

#include <qapplication.h>
#include <qlabel.h>
#include <qlineedit.h>
#include <qmessagebox.h>

#include <boost/thread/thread.hpp>
#include <boost/bind.hpp>


HexapodGui::HexapodGui( string name, string configFile) : QThread(), AOApp(name, configFile) {

   _aosInit = false;
}

HexapodGui::HexapodGui( int argc, char *argv[]) : QThread(), AOApp(argc, argv) {

   _aosInit = false;
}

void HexapodGui::PostInit() {
   init();

   v_curPos = new RTDBvar("AOS."+Side()+".HEXAPOD.ABS_POS", REAL_VARIABLE, 6, false);
   int errCode;
   MsgBuf *msgb = thGetVar(("AOS."+Side()+".HEXAPOD.ABS_POS").c_str(), 1000, &errCode);
   if (msgb)
      curpos_changed(dynamic_cast<AOApp*>(this), (Variable*) MSG_BODY(msgb));
   Notify( *v_curPos, curpos_changed);

   for (int i=0; i<6; i++)
      _curPos[i] = 0;

}

void HexapodGui::init() {

   if (!_aosInit) {
      int ret = aos_init( (char *)Side().c_str());
      if (!IS_ERROR(ret)) {
         _aosInit = true;
      }
   }

   timer = new QTimer( this);
   connect( timer, SIGNAL(timeout()), SLOT(Periodic()));
   timer->start(1000);
}

void HexapodGui::Run() {
   /* Does nothing but does not block! */

}

void HexapodGui::run() {

   while (!TimeToDie()) {
      Periodic();
      msleep(1000);
   }

}



int HexapodGui::curpos_changed( void *thisPtr, Variable *var)
{
   HexapodGui *ptr = dynamic_cast<HexapodGui*>((AOApp*)thisPtr);
   memcpy( ptr->_curPos, var->Value.Dv, sizeof(double)*6);
   return 0;
}

void HexapodGui::setCurPos( double *pos)
{
   char buf[32];
   sprintf(buf, "%5.3f", pos[0]);
   curX->setText(buf);
   sprintf(buf, "%5.3f", pos[1]);
   curY->setText(buf);
   sprintf(buf, "%5.3f", pos[2]);
   curZ->setText(buf);
   sprintf(buf, "%5.3f", pos[3]);
   curTip->setText(buf);
   sprintf(buf, "%5.3f", pos[4]);
   curTilt->setText(buf);
   sprintf(buf, "%5.3f", pos[5]);
   curRot->setText(buf);
}


int HexapodGui::moveBy( float x, float y, float z, float a1, float a2, float a3)
{
   return aos_hxpmoveby( x, y, z, a1, a2, a3,50000);
}


void HexapodGui::moveX_clicked()
{
   float x = atof( editX->text().latin1());
   int ret = moveBy( x, 0, 0, 0, 0, 0);
   if (ret != NO_ERROR)
      QMessageBox::warning( this, "Error", lao_strerror(ret));
}

void HexapodGui::setupButton_clicked()
{
   boost::thread* thSetup = new boost::thread(boost::bind(&HexapodGui::thSetup, this));
   delete thSetup;
}

void HexapodGui::thSetup()
{
   int                 ret;
   int hometimeout = 600;
   int movetimeout = 120;
   int sleepd = 1000;
   int elapsed=0;
   
   // home hexapod
   _logger->log(Logger::LOG_LEV_INFO, "Homing hexapod [%s:%d]", __FILE__,__LINE__);
   if (IS_ERROR(ret=aos_hxpinit(0,0,0,0,0,0,50000))) {
      _logger->log(Logger::LOG_LEV_FATAL, "Error homing hexapod: %d [%s:%d]", ret,  __FILE__,__LINE__);
      QMessageBox::warning(this, "Error", "Error homing hexapod");
      return;
   }
   
   // TODO write on a status line that the homing procedure can take a long time (5min+)
   elapsed=0;
   while (aos_hxpisinitialized(2000) == 0) {
      msleep(sleepd);
      elapsed += sleepd;
      if (elapsed > hometimeout)  {
         _logger->log(Logger::LOG_LEV_FATAL, "Timeout homing hexapod: %d [%s:%d]", ret,  __FILE__,__LINE__);
         QMessageBox::warning(this, "Error", "Timeout homing hexapod");
         return;
      }
   }

   Config_File config =  ConfigDictionary();
   _init_x = (float)config["INIT_X"];
   _init_y = (float)config["INIT_Y"];
   _init_z = (float)config["INIT_Z"];
   _init_a = (float)config["INIT_A"];
   _init_b = (float)config["INIT_B"];
   _init_c = (float)config["INIT_C"];
 
	// Move hexapod to 
   _logger->log(Logger::LOG_LEV_INFO, "Moving hexapod to [%g,%g,%g,%g,%g,%g] [%s:%d]",
      _init_x, _init_y, _init_z, _init_a, _init_b, _init_c, __FILE__,__LINE__);
	if (IS_ERROR(ret =  aos_hxpmoveto(_init_x, _init_y, _init_z, _init_a, _init_b, _init_c,50000))) { 
      _logger->log(Logger::LOG_LEV_FATAL, "Error moving hexapod: %d [%s:%d]", ret,  __FILE__,__LINE__);
      QMessageBox::warning(this, "Error", "Error moving hexapod");
      return;
   }
   
   elapsed=0;
   while (aos_hxpismoving(2000) == 1) {
      msleep(sleepd);
      elapsed += sleepd;
      if (elapsed > movetimeout)  {
         _logger->log(Logger::LOG_LEV_FATAL, "Timeout moving hexapod: %d [%s:%d]", ret,  __FILE__,__LINE__);
         QMessageBox::warning(this, "Error", "Timeout moving hexapod");
         return;
      }
   }

}

void HexapodGui::stopButton_clicked()
{
   int    ret;
   
   // close hexapod brakes
   _logger->log(Logger::LOG_LEV_INFO, "Closing hexapod brakes [%s:%d]", __FILE__,__LINE__);
   if (IS_ERROR(ret=aos_hxpbrake(1,20))) {
      _logger->log(Logger::LOG_LEV_FATAL, "Error closing hexapod brakes: %d [%s:%d]", ret,  __FILE__,__LINE__);
      QMessageBox::warning(this, "Error", "Error closing hexapod brakes");
      return;
   }
   
}


void HexapodGui::parkButton_clicked()
{
   boost::thread* thPark = new boost::thread(boost::bind(&HexapodGui::thPark, this));
   delete thPark;
}

void HexapodGui::thPark()
{
   int                 ret;
   int movetimeout = 120;
   int sleepd = 1;
   int elapsed=0;
   
	// Move hexapod to 0 before parking
   _logger->log(Logger::LOG_LEV_INFO, "Moving hexapod to [0,0,0,0,0,0] [%s:%d]", __FILE__,__LINE__);
	if (IS_ERROR(ret =  aos_hxpmoveto(0,0,0,0,0,0,(-1)))) { 
      _logger->log(Logger::LOG_LEV_FATAL, "Error moving hexapod: %d [%s:%d]", ret,  __FILE__,__LINE__);
      QMessageBox::warning(this, "Error", "Error moving hexapod");
      return;
   }
   
   elapsed=0;
   while (aos_hxpismoving(2000) == 1) {
      msleep(sleepd);
      elapsed += sleepd;
      if (elapsed > movetimeout)  {
         _logger->log(Logger::LOG_LEV_FATAL, "Timeout moving hexapod: %d [%s:%d]", ret,  __FILE__,__LINE__);
         QMessageBox::warning(this, "Error", "Timeout moving hexapod");
         return;
      }
   }

   double park_x, park_y, park_z, park_a, park_b, park_c;
   //TODO read from hexapod.conf 
   //Config_File & config =  ConfigDictionary();
   //park_x = (double)config["PARK_X"];
   //park_y = (double)config["PARK_Y"];
   //park_z = (double)config["PARK_Z"];
   //park_a = (double)config["PARK_A"];
   //park_b = (double)config["PARK_B"];
   //park_c = (double)config["PARK_C"];

   park_x = park_y = park_a = park_b = park_c = 0;
   park_z = -12000;
  	// Move hexapod to park position
   _logger->log(Logger::LOG_LEV_INFO, "Moving hexapod to [%g,%g,%g,%g,%g,%g] [%s:%d]",
      park_x, park_y, park_z, park_a, park_b, park_c, __FILE__,__LINE__);
	if (IS_ERROR(ret =  aos_hxpmoveto(park_x, park_y, park_z, park_a, park_b, park_c,(-1)))) { 
      _logger->log(Logger::LOG_LEV_FATAL, "Error moving hexapod: %d [%s:%d]", ret,  __FILE__,__LINE__);
      QMessageBox::warning(this, "Error", "Error moving hexapod");
      return;
   }

   
   elapsed=0;
   while (aos_hxpismoving(2000) == 1) {
      msleep(sleepd);
      elapsed += sleepd;
      if (elapsed > movetimeout)  {
         _logger->log(Logger::LOG_LEV_FATAL, "Timeout moving hexapod: %d [%s:%d]", ret,  __FILE__,__LINE__);
         QMessageBox::warning(this, "Error", "Timeout moving hexapod");
         return;
      }
   }


}

void HexapodGui::Periodic() {

   int ismoving = aos_hxpismoving(2000);
   int isinit   = aos_hxpisinitialized(2000);

   if (isinit == 0) {
      labelStatus->setText("Not initialized");
      labelStatus->setPaletteForegroundColor( qRgb( 128, 0, 0));
   }
   else {
      if (ismoving ==1) {
         labelStatus->setText("Moving");
         labelStatus->setPaletteForegroundColor( qRgb( 128, 128, 128));
      }
      else {
         labelStatus->setText("Ready");
         labelStatus->setPaletteForegroundColor( qRgb( 0, 128, 0));
      }
   }

   /* Questo non funziona (timeout -5001) */
   /*
   double *pos = aos_hxpgetpos();
   if (pos)
      setCurPos(pos);
  */

   setCurPos(_curPos);

   if (TimeToDie())
       close();
}

int main( int argc, char *argv[]) {

   SetVersion(VERS_MAJOR,VERS_MINOR);

   QApplication qApp( argc, argv);
   HexapodGui *hxpgui = NULL;

   try {
      if (argc>1)
         hxpgui = new HexapodGui( argc, argv);
      else
         hxpgui = new HexapodGui("hexapodgui", AOApp::getConffile("hexapodgui"));


      qApp.setMainWidget(hxpgui);
      hxpgui->Exec();

      hxpgui->show();
      qApp.exec();

      delete hxpgui;
      }
   catch (LoggerFatalException &e) {
      // In this case the logger can't log!!!
      printf("%s\n", e.what().c_str());
   }
   catch (AOException &e) {
      Logger::get(Logger::LOG_LEV_ERROR)->log(Logger::LOG_LEV_ERROR, "Error: %s", e.what().c_str());
   }

   return 0;
}
  

