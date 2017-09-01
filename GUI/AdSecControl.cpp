#define VERS_MAJOR   1
#define VERS_MINOR   0

#include "AdSecControl.h"
#include "AdSecCalibration.h"
#include "arblib/base/CommandsExport.h"
#include "arblib/base/AlertsExport.h"
#include "arblib/adSecArb/AdSecCommandsExport.h"


#include "Paths.h"
#include "NaNutils.h"
#include <kapplication.h>
#include <kcmdlineargs.h>
#include <qlcdnumber.h>
#include <qlabel.h>
#include <qpushbutton.h>
#include <qcheckbox.h>
#include <qtextedit.h>
#include <qfiledialog.h>
#include <qlineedit.h>
#include <qradiobutton.h>
#include <qcombobox.h>
#include <qmessagebox.h>
#include <qprogressbar.h>
#include <kurlrequester.h>
#include <kfiledialog.h>

#include "RecPreview.h"

#include <libgen.h>  // basename()

extern "C" {
#include "iolib.h" //ReadBinaryFile
}

#include <cstdlib>
#include <iostream>
#include <dirent.h>


using namespace Arcetri;
using namespace Arcetri::Arbitrator;
using namespace Arcetri::AdSec_Arbitrator;

extern resultColor colorTable[];

RTDBvar AdSecControl::_adsecSHAPE;
RTDBvar AdSecControl::_adsecDISTURB;
RTDBvar AdSecControl::_adsecG_GAIN_A;
RTDBvar AdSecControl::_adsecA_DELAY;
RTDBvar AdSecControl::_adsecM2C;
RTDBvar AdSecControl::_adsecB0_A;
RTDBvar AdSecControl::_ttOffloadENABLE;
RTDBvar AdSecControl::_ttOffloadACTIVE;
RTDBvar AdSecControl::_hoOffloadENABLE;
RTDBvar AdSecControl::_hoOffloadTIME;
RTDBvar AdSecControl::_windVALUE;
RTDBvar AdSecControl::_elevVALUE;
RTDBvar AdSecControl::_swaDEPLOYED;
RTDBvar AdSecControl::_tssSTATUS;
RTDBvar AdSecControl::_coilSTATUS;
RTDBvar AdSecControl::_mainpowerSTATUS;
RTDBvar AdSecControl::_labModeStatus;
RTDBvar AdSecControl::_focalStation;
RTDBvar AdSecControl::_wfsPupils;
RTDBvar AdSecControl::_zernikeTotal;
RTDBvar AdSecControl::_processDumpText;
RTDBvar AdSecControl::_globalProgress;
RTDBvar AdSecControl::_forceTSS;

AdSecControl::AdSecControl(string name, string configFile, KApplication &a): AdSecControlGui(), AbstractArbControl(name, configFile, a) {
	init();
}


AdSecControl::AdSecControl(int argc, char* argv[], KApplication &a): AdSecControlGui(), AbstractArbControl(argc, argv, a) {
	init();
}


AdSecControl::~AdSecControl() {

}

void AdSecControl::init() {

   qobject=this;
   alertsText->setMaxLogLines(500);
   string side = Utils::uppercase(Side());

   string logfile1 = Utils::getAdoptLog() + "/adsec."+side+".log";
   followLogFile(logfile1, this);

   string logfile2 = Utils::getAdoptLog() + "/adsecarb."+side+".log";
   followLogFile2(logfile2, this);

   string path = Paths::M2CDir( "", true);
   vector<string> M2Cs = Paths::readDir(path, DT_DIR);
   vector<string>::iterator iter;

   comboModalBasis->clear();

   sort( M2Cs.begin(), M2Cs.end());
   for (iter = M2Cs.begin(); iter != M2Cs.end(); iter++)
      comboModalBasis->insertItem( *iter);

   string s = comboModalBasis->currentText();
   comboModalBasis_activated(s);

   path = Paths::M2CDir( "Kalman_OV", true);
   vector<string> Kalmans = Paths::readDir(path, DT_DIR);

   comboKalman->clear();

   for (iter = Kalmans.begin(); iter != Kalmans.end(); iter++)
      comboKalman->insertItem( iter->substr(0,50));

   calibDialog = NULL;

   pthread_mutex_init( &_displayMutex, NULL);


   adam = new Adam();

   _minElevationSet=20;
   try {
       arbcfg = new Config_File( getConffile("adsecarb"));
       _minElevationSet = (*arbcfg)["minElevationSet"];
       try {
           _anemometerFromAOS = (*arbcfg)["anemometerFromAOS"];
       }
       catch (Config_File_Exception &e) {
           _anemometerFromAOS = 1;
       }

       // get focal stations
       Config_File::iterator it;
       comboFocalStations->clear();

       for (it = arbcfg->begin(); it != arbcfg->end(); it++) {
           int pos;
           if ((pos = it->first.find(ADSECCONTROL_FS_STR)) == 0) {
               comboFocalStations->insertItem(it->first.substr(strlen(ADSECCONTROL_FS_STR)));
           }
       }

   } catch (Config_File_Exception &e) {
   }

    _elevOK = false;
    _windOK = false;
    wfsPupils = "";

   readAutogainConf();
   _ho_middle = 100;

   processDumpText->setMaxLogLines(500);

   ForceTssOn->setBackgroundColor( QColor( qRgb(255,0,0)));
   ForceTssOn->setToggleButton(true);

}

void AdSecControl::slotDataAvail() {
   if (!_tailProc)
      return;

   while (_tailProc->canReadLineStdout())
      logText( _tailProc->readLineStdout().latin1());
}


void AdSecControl::slotDataAvail2() {
   if (!_tailProc2)
      return;

   while (_tailProc2->canReadLineStdout())
      logText( _tailProc2->readLineStdout().latin1());
}



// Called by AOApp::Exec()
void AdSecControl::SetupVarsChild() {
   _logger->log(Logger::LOG_LEV_INFO, "Setting up custom rtdb variables ...");
    try {
       _adsecM2C        = RTDBvar("ADSEC."+Side()+".M2C", CHAR_VARIABLE, 256);
       _adsecB0_A       = RTDBvar("ADSEC."+Side()+".B0_A", CHAR_VARIABLE, 256);
       _adsecA_DELAY    = RTDBvar("ADSEC."+Side()+".A_DELAY", CHAR_VARIABLE, 256);
       _adsecG_GAIN_A   = RTDBvar("ADSEC."+Side()+".G_GAIN_A", CHAR_VARIABLE, 256);
       _adsecDISTURB    = RTDBvar("ADSEC."+Side()+".DISTURB", CHAR_VARIABLE, 256);
       _adsecSHAPE      = RTDBvar("ADSEC."+Side()+".SHAPE", CHAR_VARIABLE, 256);
       _ttOffloadENABLE = RTDBvar("adsecarb."+Side()+".DOOFFLOADTT_ENABLED", INT_VARIABLE, 1);
       _ttOffloadACTIVE = RTDBvar("fastdiagn."+Side()+".DO_OFFLOAD_MODES.CUR", INT_VARIABLE, 1);
       _hoOffloadENABLE = RTDBvar("adsecarb."+Side()+".DOOFFLOADHIGHORDER", INT_VARIABLE, 1);
       _hoOffloadTIME   = RTDBvar("adsecarb."+Side()+".DOOFFLOADHO_LASTTIME", CHAR_VARIABLE, 20);

       if (_anemometerFromAOS)
           _windVALUE       = RTDBvar("AOS.AMB.WINDSPEED", REAL_VARIABLE, 1);
       else
           _windVALUE       = RTDBvar("ANEM."+Side()+".CKSPEED",REAL_VARIABLE,1);

       _elevVALUE       = RTDBvar("AOS.TEL.EL", REAL_VARIABLE, 1);
       _swaDEPLOYED     = RTDBvar("AOS."+Side()+".SWA.DEPLOYED", INT_VARIABLE, 1);
       _tssSTATUS       = RTDBvar("adamhousekeeper."+Side()+".TSS_STATUS", INT_VARIABLE, 1);
       _coilSTATUS      = RTDBvar("adamhousekeeper."+Side()+".COIL_STATUS", INT_VARIABLE, 1);
       _mainpowerSTATUS = RTDBvar("adamhousekeeper."+Side()+".MAIN_POWER_STATUS",INT_VARIABLE, 1);
       _labModeStatus   = RTDBvar(Side()+".LAB.MODE",INT_VARIABLE, 1);
       _focalStation    = RTDBvar("adsecarb."+Side()+".FOCAL_STATION",CHAR_VARIABLE, 50);
       _zernikeTotal    = RTDBvar("ADSEC."+Side()+".ZERN_APPLIED", REAL_VARIABLE, 22);
       _processDumpText = RTDBvar("ADSEC."+Side()+".LAST_FAULT_ANALYSIS", CHAR_VARIABLE, 1048);
       _globalProgress  = RTDBvar("adsecarb."+Side()+ ".PROGRESS",CHAR_VARIABLE, 50);
       _forceTSS  = RTDBvar("AOS.SHELL.TSS_OVERRIDE", INT_VARIABLE, 1);

       _ccd39bin = RTDBvar("ccd39.L.XBIN.CUR@M_MAGWFS", INT_VARIABLE, 1);

       // Ask to be notified for changes
       Notify(_adsecM2C, updateDisplay);
       Notify(_adsecB0_A, updateDisplay);
       Notify(_adsecA_DELAY, updateDisplay);
       Notify(_adsecG_GAIN_A, updateDisplay);
       Notify(_adsecDISTURB, updateDisplay);
       Notify(_adsecSHAPE, updateDisplay);
       Notify(_ttOffloadENABLE, updateDisplay);
       Notify(_ttOffloadACTIVE, updateDisplay);
       Notify(_hoOffloadENABLE, updateDisplay);
       Notify(_hoOffloadTIME, updateDisplay);
       Notify(_windVALUE, updateDisplay);
       Notify(_elevVALUE, updateDisplay);
       Notify(_swaDEPLOYED, updateDisplay);
       Notify(_tssSTATUS, updateDisplay);
       Notify(_coilSTATUS, updateDisplay);
       Notify(_mainpowerSTATUS, updateDisplay);
       Notify(_labModeStatus, updateDisplay);
       Notify(_focalStation, updateDisplay, 1);
       Notify(_zernikeTotal, updateDisplay, 1);
       Notify(_processDumpText, updateDisplay, 1);
       Notify(_globalProgress, updateDisplay, 1);

       // Initial update
       updateDisplay(dynamic_cast<AOApp *>(this), NULL);

     } catch (AOException &e) {
         _logger->log(Logger::LOG_LEV_ERROR, "%s", e.what().c_str());
       throw(e);
  }

   try {
       _wfsPupils       = RTDBvar("wfsarb."+Side()+".PUPILS@M_FLAOWFS",CHAR_VARIABLE, 200);
       Notify(_wfsPupils, updatePupils, 1);
     } catch (AOException &e) {
         _logger->log(Logger::LOG_LEV_ERROR, "SetupVars(): %s", e.what().c_str());
         _logger->log(Logger::LOG_LEV_ERROR, "Pupil check when loading reconstructors will not work");
     }

}

int AdSecControl::updatePupils(void* thisPtr, Variable* var) {

   AdSecControl* mySelf = dynamic_cast<AdSecControl*>((AOApp*) thisPtr);
   mySelf->wfsPupils = string( var->Value.Sv, var->H.NItems);
   return NO_ERROR;
}

int AdSecControl::updateDisplay(void* thisPtr, Variable* var) {


   AdSecControl* mySelf = dynamic_cast<AdSecControl*>((AOApp*) thisPtr);
   string s1, s2, s3, s4, s5, s6, s7, s8;

   pthread_mutex_lock( &mySelf->_displayMutex);

   if (var) {
      _adsecSHAPE.MatchAndSet(var);
      _adsecDISTURB.MatchAndSet(var);
      _adsecG_GAIN_A.MatchAndSet(var);
      _adsecA_DELAY.MatchAndSet(var);
      _adsecB0_A.MatchAndSet(var);
      _adsecM2C.MatchAndSet(var);
      _ttOffloadENABLE.MatchAndSet(var);
      _ttOffloadACTIVE.MatchAndSet(var);
      _hoOffloadENABLE.MatchAndSet(var);
      _hoOffloadTIME.MatchAndSet(var);
      _windVALUE.MatchAndSet(var);
      _elevVALUE.MatchAndSet(var);
      _swaDEPLOYED.MatchAndSet(var);
      _coilSTATUS.MatchAndSet(var);
      _tssSTATUS.MatchAndSet(var);
      _mainpowerSTATUS.MatchAndSet(var);
      _labModeStatus.MatchAndSet(var);
      _focalStation.MatchAndSet(var);
      _zernikeTotal.MatchAndSet(var);
      _processDumpText.MatchAndSet(var);
      _globalProgress.MatchAndSet(var);
   }


   s1 = _adsecSHAPE.Get();

   if (s1.find_last_of("/") != std::string::npos)
      mySelf->_shape = s1.substr( s1.find_last_of("/")+1);
   else
      mySelf->_shape = "";

   s2 = _adsecM2C.Get();
   if (s2.find("M2C") != std::string::npos) {
      string m2c = s2.substr( s2.find("M2C") + 4);
      mySelf->_m2c = m2c.substr(0, m2c.find_first_of("/"));
   } else
      mySelf->_m2c = "";

   s3 = _adsecB0_A.Get();
   if (s3.find_last_of("/") != std::string::npos) {
      mySelf->_b0_a = s3.substr( s3.find_last_of("/")+1);
      mySelf->updateGainRanges(s3);
   }
   else
      mySelf->_b0_a = "";

   s4 = _adsecA_DELAY.Get();
   if (s4.find("filtering") != std::string::npos) {
      string filtering = s4.substr( s4.find("filtering") + 10);
      mySelf->_a_delay = filtering.substr(0, filtering.find_first_of("/"));
   } else
      mySelf->_a_delay ="";

   s5 = _adsecG_GAIN_A.Get();
   if (s5.find_last_of("/") != std::string::npos)
      mySelf->_g_gain_a = s5.substr( s5.find_last_of("/")+1);
   else
      mySelf->_g_gain_a = "";

   s6 = _adsecDISTURB.Get();
   if (s6.find_last_of("/") != std::string::npos)
      mySelf->_disturb = s6.substr( s6.find_last_of("/")+1);
   else
      mySelf->_disturb = "";

   int v;
   _ttOffloadENABLE.Get(&v);
   mySelf->_ttOffload = (v!=0) ? true : false;

   _ttOffloadACTIVE.Get(&v);
   mySelf->_ttOffloadActive = (v!=0) ? true : false;

   _hoOffloadENABLE.Get(&v);
   mySelf->_hoOffload = (v!=0) ? true : false;

   mySelf->_hoOffloadTime = Utils::timeFromIsoString(_hoOffloadTIME.Get());
   
   double vv;
   _windVALUE.Get(&vv);
   mySelf->_windValue = vv;

   _elevVALUE.Get(&vv);
   mySelf->_elevValue = vv;

   _swaDEPLOYED.Get(&v);
   mySelf->_swaDeployed = v;

   _coilSTATUS.Get(&v);
   mySelf->_coilStatus =  (v!=0) ? true : false;

   _tssSTATUS.Get(&v);
   mySelf->_tssStatus = v;

   _mainpowerSTATUS.Get(&v);
   mySelf->_mainpowerStatus =  (v!=0) ? true : false;

   _labModeStatus.Get(&v);
   mySelf->_labMode=(v==1) ? true : false;

   _zernikeTotal.Get(1, &vv);
   mySelf->_z2 = vv*1e6;
   _zernikeTotal.Get(2, &vv);
   mySelf->_z3 = vv*1e6;
   _zernikeTotal.Get(4, &vv);
   mySelf->_z5 = vv*1e6;
   _zernikeTotal.Get(5, &vv);
   mySelf->_z6 = vv*1e6;
   _zernikeTotal.Get(6, &vv);
   mySelf->_z7 = vv*1e6;
   _zernikeTotal.Get(7, &vv);
   mySelf->_z8 = vv*1e6;
   _zernikeTotal.Get(8, &vv);
   mySelf->_z9 = vv*1e6;

   string s = _focalStation.Get();
   mySelf->_focalStationString =(string)"Focal station: "+ s;

   s = _processDumpText.Get();
   mySelf->_processDumpTextString =s;

   mySelf->_progress = atoi((char *)_globalProgress.getBuffer());

   mySelf->postRedisplayEvent();

   pthread_mutex_unlock( &mySelf->_displayMutex);
   return NO_ERROR;
}

void AdSecControl::Periodic() {

    struct timeval vtime, now;
    double vv;

    // Limit update rate
    static int counter=0;
    if ((counter++ %4)!= 0)
        return;
    
    gettimeofday(&now, NULL);
    _elevVALUE.Get(&vv);
    vtime = _elevVALUE.mTime();
    if (Utils::timediff(&now, &vtime) > EXPIRE_TIME || NaNutils::isNaN(vv)) _elevOK=false;
    else _elevOK = true;

    _windVALUE.Get(&vv);
    vtime = _windVALUE.mTime();
    if (Utils::timediff(&now, &vtime) > EXPIRE_TIME || NaNutils::isNaN(vv)) _windOK=false;
    else _windOK = true;

    postRedisplayEvent();

    if (TimeToDie())
        close();
}


void AdSecControl::redisplay() {

   // Lock this because updateDisplay() may be running in other AOApp threads.
   pthread_mutex_lock( &_displayMutex);

  curShape->setText(_shape);
  curModalBasis->setText(_m2c);
  curRecMatrix->setText(_b0_a);
  curFiltering->setText(_a_delay);
  curGainVector->setText(_g_gain_a);
  curDisturb->setText(_disturb);
  focalStationLabel->setText(_focalStationString);
  processDumpText->setText(_processDumpTextString);
  if (!_windOK) WindSpeedLCD->display("----");
  else {
     if (_windValue<0.1) //LCD problem with numbers 0.0000n
        WindSpeedLCD->display(0);
     else
        WindSpeedLCD->display(_windValue);
  }

  if (!_elevOK) ElevationLCD->display("----");
  else {
    if (_elevValue<0.1)     // LCD problem with numbers 0.0000n
        ElevationLCD->display(0);
    else
        ElevationLCD->display(_elevValue);
  }

  SwaLCD->display(_swaDeployed);

  if (_elevValue < _minElevationSet)
    ElevationLCD->setBackgroundColor( QColor( qRgb(255,0,0)));
  else
    ElevationLCD->setBackgroundColor( QColor( qRgb(0,255,0)));

  if (_swaDeployed == 0)
    SwaLCD->setBackgroundColor( QColor( qRgb(255,0,0)));
  else
    SwaLCD->setBackgroundColor( QColor( qRgb(0,255,0)));


  if (_ttOffload) {
      if (_ttOffloadActive) {
          labelOffloadStatus->setText( "ACTIVE");
          labelOffloadStatus->setEraseColor( QColor( qRgb(0,255,0)));
      }
      else {
          labelOffloadStatus->setText( "ENABLED");
          labelOffloadStatus->setEraseColor( QColor( qRgb(255,255,0)));
      }
  } else {
      labelOffloadStatus->setText( "DISABLED");
      //labelOffloadStatus->setEraseColor( QColor( qRgb(255,255,0)));
      labelOffloadStatus->setEraseColor( QColor( qRgb(192,192,192)));
  }

  if (_hoOffload) {
      time_t curTime;
      time(&curTime);
      
      if (curTime - _hoOffloadTime <= ADSECCONTROL_OFFLOAD_ACTDIFF) {
          labelHOOffloadStatus->setText( "ACTIVE");
          labelHOOffloadStatus->setEraseColor( QColor( qRgb(0,255,0)));
      }
      else {
          labelHOOffloadStatus->setText( "ENABLED");
          labelHOOffloadStatus->setEraseColor( QColor( qRgb(255,255,0)));
      }

  } else {
   labelHOOffloadStatus->setText( "DISABLED");
   labelHOOffloadStatus->setEraseColor( QColor( qRgb(192,192,192)));
   //labelHOOffloadStatus->setEraseColor( QColor( qRgb(255,255,0)));
  }

  
  switch (_tssStatus) 
      {
      case 0:
          TssStatus->setText( "<p align=""center"">TSS<br>DISABLED</p>");
          TssStatus->setEraseColor( QColor( qRgb(0,255,0)));
          break;
      case 1:
          TssStatus->setText( "<p align=""center"">TSS<br>READY</p>");
          TssStatus->setEraseColor( QColor( qRgb(0,255,0)));
          break;
      case 2:
          TssStatus->setText( "<p align=""center"">TSS<br>ACTIVATED</p>");
          TssStatus->setEraseColor( QColor( qRgb(255,255,0)));
          break;
      case 3:
          TssStatus->setText( "<p align=""center"">TSS<br>MALFUNCTION</p>");
          TssStatus->setEraseColor( QColor( qRgb(255,0,0)));
      default:
          TssStatus->setText( "<p align=""center"">TSS<br>UNKOWN</p>");
          TssStatus->setEraseColor( QColor( qRgb(255,0,0)));
      } 

  if (_coilStatus) {
   ShellRipStatus->setText( "<p align=""center"">SHELL<br>SET</p>");
   ShellRipStatus->setEraseColor( QColor( qRgb(0,255,0)));
  } else {
   ShellRipStatus->setText( "<p align=""center"">SHELL<br>RIP</p>");
   //labelHOOffloadStatus->setEraseColor( QColor( qRgb(192,192,192)));
   ShellRipStatus->setEraseColor( QColor( qRgb(255,255,0)));
  }

  if (_mainpowerStatus) {
   PowerStatus->setText( "<p align=""center"">POWER<br>ON</p>");
   PowerStatus->setEraseColor( QColor( qRgb(0,255,0)));
  } else {
   PowerStatus->setText( "<p align=""center"">POWER<br>OFF</p>");
   //labelHOOffloadStatus->setEraseColor( QColor( qRgb(192,192,192)));
   PowerStatus->setEraseColor( QColor( qRgb(255,255,0)));
  }

  if (_labMode) 
     labModeLabel->show();
   else 
     labModeLabel->hide();
   
   char buf[32];
   sprintf(buf, "%4.1f", _z2);
   z2Total->setText(buf);
   sprintf(buf, "%4.1f", _z3);
   z3Total->setText(buf);
   sprintf(buf, "%4.1f", _z5);
   z5Total->setText(buf);
   sprintf(buf, "%4.1f", _z6);
   z6Total->setText(buf);
   sprintf(buf, "%4.1f", _z7);
   z7Total->setText(buf);
   sprintf(buf, "%4.1f", _z8);
   z8Total->setText(buf);
   sprintf(buf, "%4.1f", _z9);
   z9Total->setText(buf);

   commandProgressBar->setProgress(_progress);

   sprintf(buf, "HO1 gain\n(<%d)", _ho_middle);
   labelHO1->setText(buf);
   sprintf(buf, "HO2 gain\n(>=%d)", _ho_middle);
   labelHO2->setText(buf);
   
  pthread_mutex_unlock( &_displayMutex);

}

void AdSecControl::updateGainRanges( string recpath) {

   //Changed by Jared 2014.11.29:
   //Now uses CCD 39 binning to match autogain.

    char buf[256];
    ostringstream oss;
    int nmodes, stat;
    static string lastrec="";
    static int lastbin = -1;

    int bin;
    _ccd39bin.Update();
    _ccd39bin.Get(&bin);


    if (recpath.compare(lastrec)==0 && lastbin == bin)
        return; 

//     stat = ReadFitsKeyword( (char*)recpath.c_str(), "IM_MODES", TSTRING, buf);
//     nmodes = atoi(buf);
//     printf("Updating gain ranges for %d modes\n", nmodes);
// 
//     if (nmodes<=300)
//         _ho_middle = 200;
//     if (nmodes<=200)
//         _ho_middle = 100;
//     if (nmodes<=100)
//         _ho_middle = 66;

    printf("Updating gain ranges for bin %d \n", bin);

   switch(bin)
   {
      case 1:
         _ho_middle = bin1_ho_middle;
         break;
      case 2:
         _ho_middle = bin2_ho_middle;
         break;
      case 3:
         _ho_middle = bin3_ho_middle;
         break;
      case 4:
         _ho_middle = bin4_ho_middle;
         break;
      default:
         _ho_middle = bin1_ho_middle;
         break;
   }

   lastrec = recpath;
   lastbin = bin;
}



void AdSecControl::customEvent(QCustomEvent *e) {

   SkipStatusEvent *skipStatusEvent;
   ostringstream oss;

   switch(e->type()) {

       case SKIPSTATUS_EVENT_ID:
          skipStatusEvent = (SkipStatusEvent *)e;

          progressBarSkip->setProgress ((int)(skipStatusEvent->percent()*100));
          if (skipStatusEvent->percent() > 0.1) {
              labelSkipStatus->setText("Safe skip ON");
              labelSkipStatus->setEraseColor( Qt::red);
          } else {
            labelSkipStatus->setText("Safe skip OFF");
            labelSkipStatus->setEraseColor( QColor( qRgb(192,192,192)));
          }

         break;

       default:
       break;
  }

  // Call base class method
  AbstractArbControl::customEvent(e);
}

void AdSecControl::readAutogainConf()
{
   try
   {
      Config_File autogainCfg = Utils::getConffile("autogain");

      bin1_ho_middle = (int)autogainCfg["bin1_ho_middle"];
      bin2_ho_middle = (int)autogainCfg["bin2_ho_middle"];
      bin3_ho_middle = (int)autogainCfg["bin3_ho_middle"];
      bin4_ho_middle = (int)autogainCfg["bin4_ho_middle"];
      std::cerr << "loaded autogain config.\n";
   }
   catch (Config_File_Exception &e) 
   {
      _logger->log( Logger::LOG_LEV_FATAL, "%s", e.what().c_str());
      _logger->log( Logger::LOG_LEV_FATAL, "failed to load autogain config. setting defaults");
      std::cerr << "failed to load autogain config. setting defaults" << "\n";

      bin1_ho_middle = 120;
      bin2_ho_middle = 33;
      bin3_ho_middle = 22;
      bin4_ho_middle = 6;
   }

}

// Reload filtering combobox
void AdSecControl::comboModalBasis_activated( const QString &str ) {

   string path = Paths::M2CDir( str, true) + "filtering";
   printf("%s\n", path.c_str());
   vector<string> filters = Paths::readDir(path, DT_DIR);
   vector<string>::iterator iter;

   comboFiltering->clear();

   for (iter = filters.begin(); iter != filters.end(); iter++)
      comboFiltering->insertItem( *iter);

   comboFiltering->setCurrentText("pureIntegrator");
}

void AdSecControl::kURLrecMatrix_openFileDialog( KURLRequester *kurl ) {
   string s = comboModalBasis->currentText();
   string path = Paths::M2CDir( s, true)+"RECs";
   kurl->setURL( path);
   kurl->setFilter("Rec_*");
   KFileDialog *dlg = kurl->fileDialog();
   RecPreview *preview = new RecPreview(NULL);
   dlg->setPreviewWidget(preview);

}

void AdSecControl::kURLshapeLoad_openFileDialog( KURLRequester *kurl ) {
   kurl->setURL( Paths::FlatsDir());
}

void AdSecControl::kURLshapeSave_openFileDialog( KURLRequester *kurl ) {
   kurl->setURL( Paths::FlatsDir());
}

void AdSecControl::kURLcommandVector_openFileDialog( KURLRequester *kurl ) {
   kurl->setURL( Paths::AdSecCalibDir( true)+"CMD");
}

void AdSecControl::kURLgainVector_openFileDialog( KURLRequester *kurl ) {
   string s = comboModalBasis->currentText();
   string path = Paths::M2CDir( s, true)+"gain";
   kurl->setURL( path);
}

void AdSecControl::kURLdisturbSelect_openFileDialog( KURLRequester *kurl ) {
   string s = comboModalBasis->currentText();
   string path = Paths::AdSecCalibDir( true)+"CMD/disturb";
   kurl->setURL( path);
}

void AdSecControl::updateArbStatus() {

   RequestAdSecStatus  arbStatus(150);
   RequestAdSecStatus  *arbStatusReply = NULL;

   updateArbStatusInternal( &arbStatus, (RequestStatus **)&arbStatusReply);

   if (arbStatusReply) {
      SkipStatusEvent *e = new SkipStatusEvent( arbStatusReply->isSkippingFrames(), arbStatusReply->safeSkipFramesCounter(), arbStatusReply->safeSkipPercent());
      postEvent( this, e);
      delete arbStatusReply;
   }

}



void AdSecControl::setArbitratorStatus(string arbStatus, string lastCmdDescr, string lastCmdResult) {
	adSecArbitratorStatusLabel->setText(arbStatus);
	lastCommandLabel->setText(lastCmdDescr);
	lastCommandResultLabel->setText(lastCmdResult);

   for (int i=0; colorTable[i].text[0] != '\0'; i++)
       if (colorTable[i].text == lastCmdResult)
          lastCommandResultLabel->setPaletteForegroundColor( QColor(colorTable[i].rgb));

   enableGui( (lastCmdResult.size()>0) && (lastCmdResult != "EXECUTING"));
}


void AdSecControl::logAlert(string log) {
        string s = Utils::loggerAsciiDateAndTime()+" Alert: "+log;
        alertsText->append(s);
}

void AdSecControl::logText(string log) {
	alertsText->append(log);
}


void AdSecControl::enableGui(bool enable) {

   // Always disable unused buttons
	setFlatChopButton->setEnabled(false);
	setFlatSlButton->setEnabled(false);
	resetChopButton->setEnabled(false);
	resetSlButton->setEnabled(false);
	chopToSlButton->setEnabled(false);
	slToChopButton->setEnabled(false);
	aoToSl->setEnabled(false);;
	slToAoButton->setEnabled(false);
	chopToAoButton->setEnabled(false);
	aoToChopButton->setEnabled(false);
	runChopButton->setEnabled(false);
	stopChopButton->setEnabled(false);

   onButton->setEnabled(  (_arbStatus == "PowerOff") ? enable : false);
   offButton->setEnabled( ((_arbStatus == "PowerOn") ||  (_arbStatus == "Ready")) ? enable : false);
	loadProgramButton->setEnabled( (_arbStatus == "PowerOn") ? enable : false);
	//resetButton->setEnabled( (_arbStatus == "Ready") ? enable : false);
	setFlatAoButton->setEnabled( (_arbStatus == "Ready") ? enable : false);
   restButton->setEnabled( ((_arbStatus == "AOSet") || (_arbStatus == "Failure")) ? enable : false);
   buttonApply->setEnabled( (_arbStatus == "AOSet") ? enable : false);
   gainApplyButton->setEnabled( ((_arbStatus == "AOSet") || (_arbStatus == "AORunning") || (_arbStatus == "AOPause")) ? enable : false);
   commandApplyVector->setEnabled( (_arbStatus == "AOSet") ? enable : false);
   buttonDisturbSend->setEnabled( ((_arbStatus == "AOSet") || (_arbStatus == "AORunning")  || (_arbStatus == "AOPause")) ? enable : false);
	stopAoButton->setEnabled( (_arbStatus != "Ready") ? enable : false);
//	stopAoButton->setEnabled( ((_arbStatus == "AORunning") || (_arbStatus == "AOPause")) ? enable : false);
//	runAoButton->setEnabled( (_arbStatus == "AOSet") ? enable : false);
	runAoButton->setEnabled(false);
	pauseAoButton->setEnabled( (_arbStatus == "AORunning") ? enable : false);
	resumeAoButton->setEnabled( (_arbStatus == "AOPause") ? enable : false);
	calibrateButton->setEnabled(true);

   shapeLoad->setEnabled( (_arbStatus == "AOSet") ? enable : false);
   shapeSaveButton->setEnabled( ((_arbStatus == "AORunning") || (_arbStatus == "AOSet")  || (_arbStatus == "AOPause")) ? enable : false);
  
   buttonGainZero->setEnabled( (_m2c != "") || (_arbStatus == "AOSet"));
   buttonGain10modes->setEnabled( _m2c != "");
}

void AdSecControl::saveSlopesButton_clicked() {

   QString qs = QFileDialog::getSaveFileName();
   if (qs != QString::null) {
//	   Command* cmd = new SaveSlopes(getTimeout(), qs.latin1(), _globalProgress.complete_name());
//	   sendCommandThread(cmd);
   }
}

void AdSecControl::processReply( Command *reply) {

   if (reply->getStatus() != Command::CMD_EXECUTION_SUCCESS) {
      ostringstream oss;
      oss << "Command " << reply->getDescription() << " failed with status " << reply->getStatusAsString()  << "\n" << reply->getErrorString();
      postAlert( oss.str());
   }

}
      

void AdSecControl::buttonDisturbSend_clicked() {
   disturbParams params;
   params._on = true;
   params._disturbFile = kURLdisturbSelect->url().latin1(); 
	Command* cmd = new SetDisturb(getTimeout(), params, _globalProgress.complete_name());
	sendCommandThread(cmd);
}

void AdSecControl::buttonOffloadEnable_clicked() {
   ttOffloadParams params;
   params.enable = true;
	Command* cmd = new TTOffload(getTimeout(), params, _globalProgress.complete_name());
	sendCommandThread(cmd);
}

void AdSecControl::buttonOffloadDisable_clicked() {
   ttOffloadParams params;
   params.enable = false;
	Command* cmd = new TTOffload(getTimeout(), params, _globalProgress.complete_name());
	sendCommandThread(cmd);
}

void AdSecControl::kURLrecMatrix_urlSelected( const QString &s) {
   string copy = Utils::deepCopy(s.latin1());
   kURLrecMatrix->setURL( basename((char*)copy.c_str()));
}

void AdSecControl::buttonKalmanApply_clicked() {
   recMatParams params;
   
   string kalman = comboKalman->currentText();
   string path = Paths::M2CDir( "Kalman_OV", true) + kalman +"/";

   params._m2cFile = path+ "M2C.fits";
   params._recMatFile = path+"B0.fits";
   params._aDelayFile = path+"A1.fits";
   params._bDelayFile = Paths::M2CDir( "Kalman_OV", true) + "slope_delay.fits";

   // Pupil check
   char buf[256];
   int stat = ReadFitsKeyword( (char*)params._recMatFile.c_str(), "PUPILS", TSTRING, buf);

   if (! IS_ERROR(stat)) {
       if (! isCurrentPupil(buf)) {
           ostringstream oss;
           oss << "Pupil mismatch:\n\nRec: " << buf << "\nWFS: " << wfsPupils << "\n\nApply anyway?";
           int ret = QMessageBox::question( NULL, "Warning", oss.str(), QMessageBox::Yes, QMessageBox::No);
           if (ret  == QMessageBox::No)
               return;
       }
   }


   Command* cmd = new SetRecMat(getTimeout(), params, _globalProgress.complete_name());
   sendCommandThread(cmd);
}

bool AdSecControl::isCurrentPupil( string recPup) {

    if ((wfsPupils.size()>0) && (wfsPupils.compare(recPup))) {

       // Patch per le pupille modificate dal digital shift
       char *rec = "/bin1/20100525-171742";
       char *pup = "/bin1/20110601-224135";
       if (recPup.compare(0, strlen(rec), rec)==0)
           if (wfsPupils.compare(0, strlen(pup), pup)==0)
               return true;

        return false;
        }
    return true;
}

void AdSecControl::buttonApply_clicked() {
   recMatParams params;

   string modalBasis = comboModalBasis->currentText();
   string path = Paths::M2CDir( modalBasis, true);
   string filterPath = path + "filtering/"+ comboFiltering->currentText()+"/";


   params._m2cFile = path + "m2c.fits";
   params._recMatFile = kURLrecMatrix->url().latin1(); 
   params._aDelayFile = filterPath + "mode_delay.fits";
   params._bDelayFile = filterPath + "slope_delay.fits";

   // Pupil check
   char buf[256];
   int stat = ReadFitsKeyword( (char*)params._recMatFile.c_str(), "PUPILS", TSTRING, buf);

   if (! IS_ERROR(stat)) {
       if (! isCurrentPupil(buf)) {
           ostringstream oss;
           oss << "Pupil mismatch:\n\nRec: " << buf << "\nWFS: " << wfsPupils << "\n\nApply anyway?";
           int ret = QMessageBox::question( NULL, "Warning", oss.str(), QMessageBox::Yes, QMessageBox::No);
           if (ret  == QMessageBox::No)
               return;
       }
   }

	Command* cmd = new SetRecMat(getTimeout(), params, _globalProgress.complete_name());
	sendCommandThread(cmd);
}

void AdSecControl::gainApplyButton_clicked() {
   gainParams params;
   string s = addExtension( kURLgainVector->url(), ".fits");
   applyGain(s);
}

void AdSecControl::buttonGainZero_clicked() {
   gainParams params;
   string s = Paths::GainDir( _m2c, true) + "gain0.fits";
   applyGain(s);
}

void AdSecControl::buttonGain10modes_clicked() {
   gainParams params;
   string s = Paths::GainDir( _m2c, true) + "gain0.1_10modi.fits";
   applyGain(s);
}

void AdSecControl::recoverFailButton_clicked() {
    Command *cmd = new Arcetri::AdSec_Arbitrator::RecoverFailure(getTimeout(), _globalProgress.complete_name());
   sendCommandThread(cmd);
}


void AdSecControl::shapeLoad_clicked() {

   shapeParams params;
   string s = addExtension( kURLshapeLoad->url(), ".sav");

   params._shapeFile = s;
   Command *cmd = new LoadShape(getTimeout(), params, _globalProgress.complete_name());
   sendCommandThread(cmd);
}

void AdSecControl::shapeSaveButton_clicked() {

    shapeParams params;
    string s = basename( (char*) addExtension( kURLshapeSave->url(), ".sav").c_str());

    params._shapeFile = s;
    Command *cmd = new SaveShape(getTimeout(), params, _globalProgress.complete_name());
    sendCommandThread(cmd);

    // Create default.sav link
    if (checkBoxDefault->isChecked()) {
        string path = Paths::FlatsDir()+"/";
        string fullpath = path + s;
        string deflink = path+"default.sav";
        unlink(deflink.c_str());
        symlink(fullpath.c_str(), deflink.c_str());
    }
}

void AdSecControl::commandApplyVector_clicked() {

   applyCommandParams params;
   string s = addExtension( kURLcommandVector->url(), ".fits");

   params.commandsFile = s;
   Command *cmd = new ApplyCommands(getTimeout(), params, _globalProgress.complete_name());
   sendCommandThread(cmd);
}




void AdSecControl::setMessage(string text) {
	messageLabel->setText(text);
}


void AdSecControl::setAsynchMessage(string /* text */) {
	//asynchMessageLabel->setText(text);
}


void AdSecControl::onButton_clicked() {
	Command* cmd = new On(getTimeout(), _globalProgress.complete_name());
	sendCommandThread(cmd);
}

void AdSecControl::offButton_clicked() {
	Command* cmd = new Off(getTimeout(), _globalProgress.complete_name());
	sendCommandThread(cmd);
}

void AdSecControl::loadProgramButton_clicked() {
	Command* cmd = new LoadProgram(getTimeout(), _globalProgress.complete_name());
	sendCommandThread(cmd);
}

void AdSecControl::calibrateButton_clicked() {

   if (calibDialog == NULL ) {
      calibDialog = new AdSecCalibration(this);
   }
   calibDialog->show();
//	Command* cmd = new Calibrate(getTimeout(), _globalProgress.complete_name());
//	sendCommandThread(cmd);
}

void AdSecControl::resetButton_clicked() {
	Command* cmd = new Reset(getTimeout(), _globalProgress.complete_name());
	sendCommandThread(cmd);
}

void AdSecControl::setFlatChopButton_clicked() {

}

void AdSecControl::setFlatSlButton_clicked() {

}

void AdSecControl::setFlatAoButton_clicked() {
	Command* cmd = new SetFlatAo(getTimeout(), _globalProgress.complete_name());
	sendCommandThread(cmd);

    selectFocalStationParams params;
    params._focalStation = "MAGAO_NASE";
    Command *cmd2 = new SelectFocalStation(getTimeout(), params, _globalProgress.complete_name());
    sendCommandThread(cmd2);
}

void AdSecControl::restButton_clicked() {
	Command* cmd = new Rest(getTimeout(), _globalProgress.complete_name());
	sendCommandThread(cmd);
}

void AdSecControl::forceDisable_clicked() {
   //ADAM DISABLE COIL
   try{
		if ( !adam->disableCoils() )  {
		    _logger->log(Logger::LOG_LEV_ERROR, "ADAM CAN'T DISABLE COILS: [%s:%d]",__FILE__,__LINE__);
		}
	} catch (AOException &e) {
		_logger->log(Logger::LOG_LEV_ERROR, "ADAM CAN'T DISABLE COILS: %s [%s:%d]", e.what().c_str(), __FILE__,__LINE__);
	}

}

void AdSecControl::ForceTssOn_toggled( bool on) {

   if (on){
      _forceTSS.Set(1);
      ForceTssOn->setText("Un-Force enable TSS");
   }
   else {
      _forceTSS.Set(0);
      ForceTssOn->setText("Force enable TSS");
   }

}

void AdSecControl::ForcePowerOff_clicked() {
   //ADAM POWER OFF
   try{
		if ( !adam->disableMainPower() )  {
		    _logger->log(Logger::LOG_LEV_ERROR, "ADAM CAN'T POWER OFF THE SYSTEM: [%s:%d]",__FILE__,__LINE__);
		}
	} catch (AOException &e) {
		_logger->log(Logger::LOG_LEV_ERROR, "ADAM CAN'T POWER OFF THE SYSTEM: %s [%s:%d]", e.what().c_str(), __FILE__,__LINE__);
	}

}

void AdSecControl::zernApply_clicked() {

      setZernikesParams params;
      int i;
      for ( i=0;i<22;i++) params.deltaM[i] =0.0;
      params.deltaM[8] = atof(z9Applied->text())*1e-6;
      params.deltaM[7] = atof(z8Applied->text())*1e-6;
      params.deltaM[6] = atof(z7Applied->text())*1e-6;
      params.deltaM[5] = atof(z6Applied->text())*1e-6;
      params.deltaM[4] = atof(z5Applied->text())*1e-6;
      params.deltaM[2] = atof(z3Applied->text())*1e-6;
      params.deltaM[1] = atof(z2Applied->text())*1e-6;

      // WriteFitsFile
//      long dims[] = {22};
//      string ModesCommandFile = "/tmp/modes2apply.fits";
//      unlink((char*)ModesCommandFile.c_str()); // cfitsio does not overwrite files!! 
//      int stat = WriteFitsFile( (char*)ModesCommandFile.c_str(), (unsigned char *)zz, TFLOAT, dims, 1);
//     if (stat != NO_ERROR) {
 //        _logger->log(Logger::LOG_LEV_ERROR, "Correct modes fits file not written [%s:%d]", __FILE__, __LINE__);
 //     }
     Command *cmd = new SetZernikes(getTimeout(), params, _globalProgress.complete_name());
     sendCommandThread(cmd);

      z9Applied->setEraseColor( QColor( qRgb(0,255,0)));
      z8Applied->setEraseColor( QColor( qRgb(0,255,0)));
      z7Applied->setEraseColor( QColor( qRgb(0,255,0)));
      z6Applied->setEraseColor( QColor( qRgb(0,255,0)));
      z5Applied->setEraseColor( QColor( qRgb(0,255,0)));
      z3Applied->setEraseColor( QColor( qRgb(0,255,0)));
      z2Applied->setEraseColor( QColor( qRgb(0,255,0)));

}

void AdSecControl::buttonSetFocalStation_clicked() {

    if (comboFocalStations->count() > 0) {
        selectFocalStationParams params;
        params._focalStation = (const char *)comboFocalStations->currentText();

        Command *cmd = new SelectFocalStation(getTimeout(), params, _globalProgress.complete_name());
        sendCommandThread(cmd);
    }
}

void AdSecControl::resetChopButton_clicked() {

}

void AdSecControl::resetSlButton_clicked() {

}

void AdSecControl::resetAoButton_clicked() {

}

void AdSecControl::chopToSlButton_clicked() {

}

void AdSecControl::slToChopButton_clicked() {

}

void AdSecControl::aoToSl_clicked() {

}

void AdSecControl::slToAoButton_clicked() {

}

void AdSecControl::chopToAoButton_clicked() {

}

void AdSecControl::aoToChopButton_clicked() {

}

void AdSecControl::runChopButton_clicked() {

}

void AdSecControl::stopChopButton_clicked() {

}

void AdSecControl::runAoButton_clicked() {
}

void AdSecControl::stopAoButton_clicked() {
    stopAoParams params;
    params._restoreShape = true;
	Command* cmd = new StopAo(getTimeout(), params, _globalProgress.complete_name());
	sendCommandThread(cmd);
}

void AdSecControl::pauseAoButton_clicked() {
	Command* cmd = new PauseAo(getTimeout(), _globalProgress.complete_name());
	sendCommandThread(cmd);
}

void AdSecControl::resumeAoButton_clicked() {
	Command* cmd = new ResumeAo(getTimeout(), _globalProgress.complete_name());
	sendCommandThread(cmd);
}

void AdSecControl::setGainButton_clicked() {

}

void AdSecControl::z9Input_returnPressed() {
     z9Applied->setText(z9Input->text());
     z9Applied->setEraseColor( QColor( qRgb(192,192,192)));
}

void AdSecControl::z8Input_returnPressed() {
     z8Applied->setText(z8Input->text());
     z8Applied->setEraseColor( QColor( qRgb(192,192,192)));
}

void AdSecControl::z7Input_returnPressed() {
     z7Applied->setText(z7Input->text());
     z7Applied->setEraseColor( QColor( qRgb(192,192,192)));
}

void AdSecControl::z6Input_returnPressed() {
     z6Applied->setText(z6Input->text());
     z6Applied->setEraseColor( QColor( qRgb(192,192,192)));
}

void AdSecControl::z5Input_returnPressed() {
     z5Applied->setText(z5Input->text());
     z5Applied->setEraseColor( QColor( qRgb(192,192,192)));
}

void AdSecControl::z3Input_returnPressed() {
     z3Applied->setText(z3Input->text());
     z3Applied->setEraseColor( QColor( qRgb(192,192,192)));
}

void AdSecControl::z2Input_returnPressed() {
     z2Applied->setText(z2Input->text());
     z2Applied->setEraseColor( QColor( qRgb(192,192,192)));
}

void AdSecControl::buttonHOOOffloadEnable_clicked() {
    _hoOffloadENABLE.Set(1);
}

void AdSecControl::buttonHOOffloadDisable_clicked() {
    _hoOffloadENABLE.Set(0);
}

void AdSecControl::buttonGainTTUp_clicked() {
    double tt, ho1, ho2;
    getCurGain( tt, ho1, ho2);
    tt += 0.05;
    applyGain( writeGain( tt, ho1, ho2), true);
}

void AdSecControl::buttonGainTTDown_clicked() {
    double tt, ho1, ho2;
    getCurGain( tt, ho1, ho2);
    tt -= 0.05;
    applyGain( writeGain( tt, ho1, ho2), true);
}

void AdSecControl::buttonGainHO1Up_clicked() {
    double tt, ho1, ho2;
    getCurGain( tt, ho1, ho2);
    ho1 += 0.05;
    applyGain( writeGain( tt, ho1, ho2), true);
}

void AdSecControl::buttonGainHO1Down_clicked() {
    double tt, ho1, ho2;
    getCurGain( tt, ho1, ho2);
    ho1 -= 0.05;
    applyGain( writeGain( tt, ho1, ho2), true);
}

void AdSecControl::buttonGainHO2Up_clicked() {
    double tt, ho1, ho2;
    getCurGain( tt, ho1, ho2);
    ho2 += 0.05;
    applyGain( writeGain( tt, ho1, ho2), true);
}

void AdSecControl::buttonGainHO2Down_clicked() {
    double tt, ho1, ho2;
    getCurGain( tt, ho1, ho2);
    ho2 -= 0.05;
    applyGain( writeGain( tt, ho1, ho2), true);
}

void AdSecControl::applyGain( string filename, bool updateEditBox) {

   gainParams params;
   params._gainFile = filename;
   Command *cmd = new SetGain(getTimeout(), params, _globalProgress.complete_name());

   if (updateEditBox)
      kURLgainVector->setURL(filename);

   sendCommandThread(cmd);
}

string AdSecControl::writeGain( double tt, double ho1, double ho2) {

    string m2c = comboModalBasis->currentText();
    string gainDir = Paths::GainDir( m2c, true);
    char buf[128];
    if (tt<0) tt=0;
    if (ho1<0) ho1=0;
    if (ho2<0) ho2=0;
    sprintf(buf, "g_tt_%4.2f_ho1_%4.2f_ho2_%4.2f.fits", tt, ho1, ho2);
    float gainVector[672];
    gainVector[0] = tt;
    gainVector[1] = tt;
    for (int i=2; i<_ho_middle; i++)
        gainVector[i] = ho1;
    for (int i=_ho_middle; i<672; i++)
        gainVector[i] = ho2;

    long dims[] = {672};
    string outfile = gainDir + buf;
    int stat = WriteFitsFile( (char*)outfile.c_str(), (unsigned char *)gainVector, TFLOAT, dims, 1);
    return outfile;
}

void AdSecControl::getCurGain( double& tt, double& ho1, double &ho2) {

    size_t pos;
    string gain =_g_gain_a;
    tt=0;
    ho1=0;
    ho2=0;

    pos = gain.find("tt_");
    if (pos != string::npos)
        tt = atof(gain.substr(pos+3,4).c_str());
    pos = gain.find("ho1_");
    if (pos != string::npos)
        ho1 = atof(gain.substr(pos+4,4).c_str());
    pos = gain.find("ho2_");
    if (pos != string::npos)
        ho2 = atof(gain.substr(pos+4,4).c_str());
}


static const KCmdLineOptions options[] =
{
   {"f <file>", ("Configuration file"), 0 },
   {"i <identity>", ("MsgD identity"), 0 },
   KCmdLineLastOption
};

int main(int argc, char* argv[]) {

   SetVersion(VERS_MAJOR,VERS_MINOR);

   KCmdLineArgs::init( argc, argv, "AdSecControl", "AdSecControl", "AdSecControl", "1.0" );
   KCmdLineArgs::addCmdLineOptions(options);
   KApplication a;

	AdSecControl* adsecControl = NULL;

	try {
		if(argc > 1) {
			adsecControl = new AdSecControl(argc, argv, a);
		}
		else {
			adsecControl = new AdSecControl("adseccontrol", AOApp::getConffile("adseccontrol"), a);
		}

		adsecControl->doGui( adsecControl, true, true);

		// Stop the AOApp 
		adsecControl->die();

		delete adsecControl;
		return 0;
	}
	catch (LoggerFatalException &e) {
		// In this case the logger can't log!!!
		printf("%s\n", e.what().c_str());
	}
	catch (AOException &e) {
		Logger::get(Logger::LOG_LEV_ERROR)->log(Logger::LOG_LEV_ERROR, "Error in AdSecArbitrator: %s", e.what().c_str());
	}

	return -1;
}




