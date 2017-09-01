
#include <qapplication.h>
#include <qlabel.h>
#include <qlineedit.h>
#include <qmessagebox.h>
#include <Utils.h>
#include <stdconfig.h>
#include <cstdio>
#include <AOApp.h>
#include <dirent.h>
#include <stddef.h>
#include <iostream>
#include <vector>
#include <qstringlist.h>
#include <qcombobox.h>
#include <iostream>
#include <sstream>

using namespace Arcetri;
using namespace std;

extern "C" {
#include "base/timelib.h"
#include "iolib.h"
}

#include "AdSecConfGui.h"
#include <qpushbutton.h>
#include <qlayout.h>


AdSecConfGui::AdSecConfGui( int argc, char *argv[]) : QThread() {

    char staux[31], stnum[31];

   pthread_mutex_init( &_displayMutex, NULL);

   // add actuator buttons
   QFont font;
   QColor color( 85, 255, 0 );
   QSize size(20, 20);

   for (int i = 0; i < NUM_ACTUATORS; i++) {
       snprintf(stnum, 30, "%3d", i);
       // actPos
       if (i % NUM_ACTPERCOL == 0) {
           snprintf(staux, 30, "actposlayout%02d", i / NUM_ACTPERCOL);
           actPosLayout[i/NUM_ACTPERCOL] = new QHBoxLayout( 0, 0, 6, staux);
       }
       snprintf(staux, 30, "act%03dpos", i);
       actPos[i] =  new QPushButton( (QWidget *)buttonGroupActPos, staux);
       actPos[i]->setMaximumSize( size );
       actPos[i]->setPaletteBackgroundColor( color );
       font = actPos[i]->font();
       font.setPointSize( 6 );
       actPos[i]->setFont( font );
       actPos[i]->setText( stnum );
       actPosLayout[i/NUM_ACTPERCOL]->addWidget(actPos[i]);
       if (i % NUM_ACTPERCOL == NUM_ACTPERCOL-1) {
           buttonGroupActPosLayout->addLayout(actPosLayout[i/NUM_ACTPERCOL]);
       }

       // actCurr
       if (i % NUM_ACTPERCOL == 0) {
           snprintf(staux, 30, "actcurrlayout%02d", i / NUM_ACTPERCOL);
           actCurrLayout[i/NUM_ACTPERCOL] = new QHBoxLayout( 0, 0, 6, staux);
       }
       snprintf(staux, 30, "act%03dcurr", i);
       actCurr[i] =  new QPushButton( (QWidget *)buttonGroupActCurr, staux);
       actCurr[i]->setMaximumSize( size );
       actCurr[i]->setPaletteBackgroundColor( color );
       font = actCurr[i]->font();
       font.setPointSize( 6 );
       actCurr[i]->setFont( font );
       actCurr[i]->setText( stnum );
       actCurrLayout[i/NUM_ACTPERCOL]->addWidget(actCurr[i]);
       if (i % NUM_ACTPERCOL == NUM_ACTPERCOL-1) {
           buttonGroupActCurrLayout->addLayout(actCurrLayout[i/NUM_ACTPERCOL]);
       }

       // actIcc
       if (i % NUM_ACTPERCOL == 0) {
           snprintf(staux, 30, "acticclayout%02d", i / NUM_ACTPERCOL);
           actIccLayout[i/NUM_ACTPERCOL] = new QHBoxLayout( 0, 0, 6, staux);
       }
       snprintf(staux, 30, "act%03dicc", i);
       actIcc[i] =  new QPushButton( (QWidget *)buttonGroupActIcc, staux);
       actIcc[i]->setMaximumSize( QSize( 20, 20 ) );
       actIcc[i]->setPaletteBackgroundColor( QColor( 85, 255, 0 ) );
       font = actIcc[i]->font();
       font.setPointSize( 6 );
       actIcc[i]->setFont( font );
       actIcc[i]->setText( stnum );
       actIccLayout[i/NUM_ACTPERCOL]->addWidget(actIcc[i]);
       if (i % NUM_ACTPERCOL == NUM_ACTPERCOL-1) {
           buttonGroupActIccLayout->addLayout(actIccLayout[i/NUM_ACTPERCOL]);
       }
   }

   //UNIT DEFINITION AND GUI FILL
   string conflink = Utils::getAdoptRoot()+"/conf/"+Utils::lowercase(Utils::getAdoptSubsystem())+"/current";
   char proc[512];
   int ch = readlink(conflink.c_str(),proc,512);
   if (ch != -1) {
      proc[ch]=0;
      _unit = string(proc);
      _unit.substr(0, ch);
      unitCurr->setText(_unit);
   }

   string unitpath = Utils::getAdoptRoot()+"/conf/"+Utils::lowercase(Utils::getAdoptSubsystem());
   std::vector<std::string> list = parseDir(unitpath, "672");
  //vector<string>::iterator it;
  //cout << "myvector contains:";
  //for ( it=unitlist.begin() ; it < unitlist.end(); it++ ) {
  //   puts((*it).c_str());
  //}

   int curridx;
   vector<string>::iterator iter;
   for (iter = list.begin(); iter != list.end(); iter++) {
       unitList->insertItem( *iter);
       if ((*iter).compare(_unit) == 0) curridx = unitList->currentItem();
   }
   unitList->setCurrentItem(curridx);
   emit unitList_activated(QString(_unit));



   string confFname = conflink+"/configuration.txt";
   _cfgElec         = new Config_File(confFname);
   _shell = (std::string)(*_cfgElec)["shell"];
   shellCurr->setText(_shell);
   redisplay();


  /*  std::string _shell;
    std::string _unit;
    std::string _dspver;
    std::string _cbcuver;
    std::string _sbcuver;
    std::string _accver;
    Config_File * _cfgElec;
    std::string _currElecTN;
    std::string _currFFmatrixTN;
    std::string _currOptDataTN;
    std::string _currFlatTN;
    std::string _zernTN;
    std::string _hoTN;
    std::string _loZernTN;
    std::string _hoZernTN;
    std::string _fastParamTN;
    std::string _hkprParamTN;
*/
//   int iU, iS;

//   setupUi(this); 
//   connect( unitList_textChanged, SIGNAL( activated() ), this, SLOT( updateShellList() )); 
//   connect( shellList_textChanged, SIGNAL( activated() ), this, SLOT( updateAllList() )); 
}


AdSecConfGui::~AdSecConfGui() {

    for (int i = 0; i < NUM_ACTUATORS; i++) {
        delete actPos[i];
        delete actCurr[i];
        delete actIcc[i];
        if (i % NUM_ACTPERCOL == NUM_ACTPERCOL-1) {
            delete actPosLayout[i/NUM_ACTPERCOL];
            delete actCurrLayout[i/NUM_ACTPERCOL];
            delete actIccLayout[i/NUM_ACTPERCOL];
        }
    }
}

void AdSecConfGui::run() {
}

void AdSecConfGui::redisplay() {
   puts("redisplay");
   pthread_mutex_lock( &_displayMutex);
   updateFitsTN();
   updateTxtTN();
   pthread_mutex_unlock( &_displayMutex);
}

std::vector<std::string>  AdSecConfGui::parseDir(string dir2parse, string pattern) {
   static DIR *dp;
   struct dirent *ep;
   std::vector<std::string>  out;
   dp = opendir (dir2parse.c_str());
   string currdir;
   if (dp != NULL) {
      while (ep = readdir (dp)) {
         currdir = string(ep->d_name);
         if (pattern.compare("") != 0 ) {
            if (currdir.find(pattern) != string::npos ) out.push_back(ep->d_name);
         } else {
            out.push_back(ep->d_name);
         }
      }
      (void) closedir (dp);
   }
   return out;
}

void AdSecConfGui::unitList_activated(const QString& unitSelected) {

   
   _unit = unitSelected.latin1();

//   static DIR *dp;
//   struct dirent *ep;
//   std::vector<std::string> shelllist;

   string shellpath = Utils::getAdoptRoot()+"/conf/"+Utils::lowercase(Utils::getAdoptSubsystem())+"/"+_unit;
   std::vector<std::string> list = parseDir(shellpath, "TS");

   /*dp = opendir (shellpath.c_str());
   string currdir;
   if (dp != NULL) {
      while (ep = readdir (dp)) {
         currdir = string(ep->d_name);
         if (currdir.find("TS") != string::npos ) shelllist.push_back(ep->d_name);
      }
      (void) closedir (dp);
   }*/

   shellList->clear();
   int curridx;
   vector<string>::iterator iter;
   for (iter = list.begin(); iter != list.end(); iter++) {
       shellList->insertItem( *iter);
       if ((*iter).compare(_unit) == 0) curridx = shellList->currentItem();
   }
   shellList->setCurrentItem(curridx);
   emit shellList_activated(QString(_shell));

}

void AdSecConfGui::shellList_activated(const QString& shellSelected) {
   
   string shellpath = "/towerdata/adsec_calib/ff_matrix/";
   std::vector<std::string> list = parseDir(shellpath, "2");
   ffReqTN->clear();
   vector<string>::iterator iter;
   for (iter = list.begin(); iter != list.end(); iter++)
       ffReqTN->insertItem( *iter);
}

void AdSecConfGui::updateFitsTN() {

   char buf[256];
   string path = Utils::getAdoptRoot()+"/calib/"+Utils::lowercase(Utils::getAdoptSubsystem())+"/"+_unit+"/data/ff_matrix.fits";
   puts(path.c_str());
   ReadFitsKeyword( (char*)path.c_str(), "TRACKNUM", TSTRING, buf);
   _currFFmatrixTN = string(buf);
   ffCurrTN->setText(_currFFmatrixTN);
   
   path = Utils::getAdoptRoot()+"/calib/"+Utils::lowercase(Utils::getAdoptSubsystem())+"/"+_unit+"/data/pmz.fits";
   puts(path.c_str());
   ReadFitsKeyword( (char*)path.c_str(), "TRACKNUM", TSTRING, buf);
   _zernTN = string(buf);
   zernCurr->setText(_zernTN);
   
   path = Utils::getAdoptRoot()+"/calib/"+Utils::lowercase(Utils::getAdoptSubsystem())+"/"+_unit+"/data/pmohofor.fits";
   puts(path.c_str());
   ReadFitsKeyword( (char*)path.c_str(), "TRACKNUM", TSTRING, buf);
   _currFFmatrixTN = string(buf);
   hoCurr->setText(_hoTN);
   
   path = Utils::getAdoptRoot()+"/conf/"+Utils::lowercase(Utils::getAdoptSubsystem())+"/"+_unit+"/processConf/fastdiagn/pos2modes.fits";
   ReadFitsKeyword( (char*)path.c_str(), "TRACKNUM", TSTRING, buf);
   _loZernTN = string(buf);
   loZernCurr->setText(_loZernTN);
   
   path = Utils::getAdoptRoot()+"/conf/"+Utils::lowercase(Utils::getAdoptSubsystem())+"/"+_unit+"/processConf/fastdiagn/curr2modes.fits";
   puts(path.c_str());
   ReadFitsKeyword( (char*)path.c_str(), "TRACKNUM", TSTRING, buf);
   _hoZernTN = string(buf);
   hoZernCurr->setText(_hoZernTN);
   
}

void AdSecConfGui::updateTxtTN() {

   string path = Utils::getAdoptRoot()+"/conf/"+Utils::lowercase(Utils::getAdoptSubsystem())+"/"+_unit+"/"+_shell+"/elec.txt";
   string tmp,tn;
   FILE * pFile;
   char mystring [100];

   pFile = fopen (path.c_str() , "r");
   if (pFile == NULL) perror ("Error opening file");
   else {
     fgets (mystring , 100 , pFile);
     tmp = string(mystring);
     tn = tmp.substr(tmp.find_first_of("+& ")+3, 15);
     puts(tn.c_str());
     fclose (pFile);
   }
   elecCurr->setText(tn);
   elecCurr_2->setText(tn);
   elecCurr_3->setText(tn);

}

int main( int argc, char *argv[]) {


   QApplication qApp( argc, argv);
   AdSecConfGui *cfggui = NULL;

      cfggui = new AdSecConfGui( argc, argv);

      qApp.setMainWidget(cfggui);
//      hxpgui->Exec();
      cfggui->show();
      qApp.exec();

      delete cfggui;

   return 0;
}
  

