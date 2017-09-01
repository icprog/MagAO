#define VERS_MAJOR   1
#define VERS_MINOR   0

#include "BoardGui.h"
#include "math.h"

extern "C" {
#include "base/timelib.h"
}

#include <qapplication.h>
#include <qlabel.h>
#include <qlineedit.h>
#include <qmessagebox.h>
#include <qcombobox.h>
#include <qcheckbox.h>
#include <kapplication.h>
#include <kcmdlineargs.h>
#include <kled.h>
#include <qdial.h>

#include <iomanip>


BoardGui::BoardGui( string name, string configFile, KApplication &kApp) : AOAppGui(name, configFile, kApp) {
    init();
}

BoardGui::BoardGui( int argc, char *argv[], KApplication &kApp) : AOAppGui(argc, argv, kApp) {
    init();

}

void BoardGui::init() {
    active.clear();
 
}

void BoardGui::SetupVars() {

   v_curPosX = new RTDBvar( "baysidex."+Side()+".POS.CUR", REAL_VARIABLE, 1, false);
   v_curPosY = new RTDBvar( "baysidey."+Side()+".POS.CUR", REAL_VARIABLE, 1, false);
   v_curPosZ = new RTDBvar( "baysidez."+Side()+".POS.CUR", REAL_VARIABLE, 1, false);

   v_tgtPosX = new RTDBvar( "baysidex."+Side()+".POS.REQ", REAL_VARIABLE, 1, false);
   v_tgtPosY = new RTDBvar( "baysidey."+Side()+".POS.REQ", REAL_VARIABLE, 1, false);
   v_tgtPosZ = new RTDBvar( "baysidez."+Side()+".POS.REQ", REAL_VARIABLE, 1, false);

   v_ccd39Bin = new RTDBvar( "ccd39."+Side()+".XBIN.CUR", INT_VARIABLE, 1, false);
   v_ccd39Freq= new RTDBvar( "ccd39."+Side()+".FRMRT.CUR", REAL_VARIABLE, 1, false);
   v_ccd39Temp= new RTDBvar( "ccd39."+Side()+".TEMPS", INT_VARIABLE, 3, false);

   v_ccd47Bin = new RTDBvar( "ccd47."+Side()+".XBIN.CUR", INT_VARIABLE, 1, false);
   v_ccd47Freq= new RTDBvar( "ccd47."+Side()+".FRMRT.CUR", REAL_VARIABLE, 1, false);
   v_ccd47Temp= new RTDBvar( "ccd47."+Side()+".TEMPS", INT_VARIABLE, 3, false);

   v_rerotPos = new RTDBvar( "rerotator."+Side()+".POS.CUR", REAL_VARIABLE, 1, false);
   //v_rerotSync= new RTDBvar( "rerotator."+Side()+".TRACKING.CUR", INT_VARIABLE, 1, false);

   v_ttAmp = new RTDBvar( "ttctrl."+Side()+".AMP.REQ", REAL_VARIABLE, 1, false);
   v_ttFreq= new RTDBvar( "ttctrl."+Side()+".FREQ.CUR", REAL_VARIABLE, 1, false);

   v_fw1Pos = new RTDBvar( "filterwheel1."+Side()+".POSNAME.CUR", CHAR_VARIABLE, 50, false);
   v_fw2Pos = new RTDBvar( "filterwheel2."+Side()+".POSNAME.CUR", CHAR_VARIABLE, 50, false);

   v_clPosX = new RTDBvar( "ttctrl."+Side()+".POSX.CUR", REAL_VARIABLE, 1, false);
   v_clPosY = new RTDBvar( "ttctrl."+Side()+".POSY.CUR", REAL_VARIABLE, 1, false);

   v_adcPos1 = new RTDBvar( "adc1."+Side()+".POS.CUR", REAL_VARIABLE, 1, false);
   v_adcPos2 = new RTDBvar( "adc2."+Side()+".POS.CUR", REAL_VARIABLE, 1, false);
   //v_adcSync = new RTDBvar( "adc."+Side()+".TRACKING.CUR", INT_VARIABLE, 50, false);

   v_derotPos = new RTDBvar("AOS."+Side()+".ROTATOR.ANGLE@M_ADSEC", REAL_VARIABLE, 1, false);
   v_telEl = new RTDBvar("AOS.TEL.EL@M_ADSEC", REAL_VARIABLE, 1, false);
   v_telAz = new RTDBvar("AOS.TEL.AZ@M_ADSEC", REAL_VARIABLE, 1, false);

   v_cubePos = new RTDBvar("cubestage."+Side()+".POS.CUR", REAL_VARIABLE, 1, false);

   Notify( *v_curPosX, stages_notify, true);
   Notify( *v_curPosY, stages_notify, true);
   Notify( *v_curPosZ, stages_notify, true);

   Notify( *v_tgtPosX, stages_notify, true);
   Notify( *v_tgtPosY, stages_notify, true);
   Notify( *v_tgtPosZ, stages_notify, true);

   Notify( *v_ccd39Bin, ccd39_notify, true);
   Notify( *v_ccd39Freq, ccd39_notify, true);
   Notify( *v_ccd39Temp, ccd39_notify, true);

   Notify( *v_ccd47Bin, ccd47_notify, true);
   Notify( *v_ccd47Freq, ccd47_notify, true);
   Notify( *v_ccd47Temp, ccd47_notify, true);

   Notify( *v_rerotPos, rerot_notify, true);
   //Notify( *v_rerotSync, rerot_notify, true);

   Notify( *v_ttAmp, tt_notify, true);
   Notify( *v_ttFreq, tt_notify, true);

   Notify( *v_fw1Pos, fw1_notify, true);
   Notify( *v_fw2Pos, fw2_notify, true);
   
   Notify( *v_clPosX, cl_notify, true);
   Notify( *v_clPosY, cl_notify, true);
   
   Notify( *v_adcPos1, adc_notify, true);
   Notify( *v_adcPos2, adc_notify, true);
   //Notify( *v_adcSync, adc_notify, true);

   Notify( *v_derotPos, tel_notify, true);
   Notify( *v_telEl, tel_notify, true);
   Notify( *v_telAz, tel_notify, true);

   Notify( *v_cubePos, cube_notify, true);
   
}

void BoardGui::run() {

   while (!TimeToDie()) {
      Periodic();
      msleep(1000);
   }

}


void BoardGui::customEvent( QCustomEvent *e) {

   string s;
   StagesEvent *stagesEvent;
   Ccd39Event *ccd39Event;
   Ccd47Event *ccd47Event;
   CameraLensEvent *cameraLensEvent;
   TTEvent *ttEvent;
   RerotEvent *rerotEvent;
   CubeEvent *cubeEvent;
   Fw1Event *fw1Event;
   Fw2Event *fw2Event;
   AdcEvent *adcEvent;
   TelEvent *telEvent;
   DeactivateEvent *deactivateEvent;
   bool changed;
   int pos;
   QWidget *widget;

   ostringstream oss1, oss2, oss3;

   //printf("Custom event %d\n", e->type());
   
   switch(e->type()) {

      case DEACTIVATE_EVENT_ID:
         deactivateEvent = (DeactivateEvent *)e;
         widget = deactivateEvent->widget();
         widget->setPaletteForegroundColor( Qt::black);
         break;

      case STAGES_EVENT_ID:
         stagesEvent = (StagesEvent *)e;
         kLedStagesPosition->move( roundf(stagesEvent->curx()), roundf(stagesEvent->cury()));
         labelStagesTarget->move( roundf(stagesEvent->tgtx()), roundf(stagesEvent->tgty()));

         oss1 << setw(6) << setprecision(3) << stagesEvent->x();
         oss2 << setw(6) << setprecision(3) << stagesEvent->y();
         oss3 << setw(6) << setprecision(3) << stagesEvent->z();

         if (oss1.str() != editStageX->text()) { 
             editStageX->setText( oss1.str());
             activate(editStageX);
         }

         if (oss2.str() != editStageY->text()) { 
             editStageY->setText( oss2.str());
             activate(editStageY);
         }

         if (oss3.str() != editStageZ->text()) { 
             editStageZ->setText( oss3.str());
             activate(editStageZ);
         }

         break; 
   
      case CCD39_EVENT_ID:
         ccd39Event = (Ccd39Event *)e;
         oss1 << (int) roundf(ccd39Event->freq());
         editCcd39Freq->setText(oss1.str());

         oss2 << ccd39Event->bin() << "x" << ccd39Event->bin();
         comboCcd39Bin->setCurrentText(oss2.str());

         activate(editCcd39Freq);
         activate(comboCcd39Bin);
         break;

      case CCD47_EVENT_ID:
         ccd47Event = (Ccd47Event *)e;
         oss1 << (int) roundf(ccd47Event->freq());
         editCcd47Freq->setText(oss1.str());

         oss2 << ccd47Event->bin() << "x" << ccd47Event->bin();
         comboCcd47Bin->setCurrentText(oss2.str());

         activate(editCcd47Freq);
         activate(comboCcd47Bin);
         break;

      case REROT_EVENT_ID:
         rerotEvent = (RerotEvent *)e;
	 oss1 << fixed << setw(6) << setprecision(1) << rerotEvent->pos();
         if (oss1.str() != editRerotPos->text()) {
             editRerotPos->setText(oss1.str());
             activate(editRerotPos);
         }

         checkRerotSync->setChecked( rerotEvent->sync());
         break;

      case CUBE_EVENT_ID:
         cubeEvent = (CubeEvent *)e;
         pos = (int)(380+ cubeEvent->pos()*(-6));
         cubePix->move( pos, cubePix->y());

	 // Cube IN!
	 if (cubeEvent->pos() > -10)
		cubePix->setFrameShape( QFrame::StyledPanel);
	 else
                cubePix->setFrameShape( QFrame::NoFrame);
         break;

      case FW1_EVENT_ID:
         fw1Event = (Fw1Event *)e;
         editFW1->setText(fw1Event->posname());
         activate(editFW1);
         break;

      case FW2_EVENT_ID:
         fw2Event = (Fw2Event *)e;
         editFW2->setText(fw2Event->posname());
         activate(editFW2);
         break;

      case TEL_EVENT_ID:
         telEvent = (TelEvent *)e;
         oss1 << fixed << setw(6) << setprecision(1) << telEvent->derotPos();
         oss2 << fixed << setw(6) << setprecision(1) << telEvent->telAz();
         oss3 << fixed << setw(6) << setprecision(1) << telEvent->telEl();
         if (oss1.str() != labelDerotator->text()) {
             labelDerotator->setText(oss1.str());
             activate(labelDerotator);
         }
         dialDerotator->setValue((int)telEvent->derotPos()+180);

         if (oss2.str() != labelAzimuth->text()) {
             labelAzimuth->setText(oss2.str());
             activate(labelAzimuth);
         }
         if (oss3.str() != labelElevation->text()) {
             labelElevation->setText(oss3.str());
             activate(labelElevation);
         }

         break;

      case TT_EVENT_ID:
         ttEvent = (TTEvent *)e;
         oss1 << fixed << setw(4) << setprecision(2) << (float) ttEvent->amp();
         editTTAmpl->setText(oss1.str());

         if ( ttEvent->freq() == 0)
		oss2 << "Sync";
         else
		oss2 << (int) roundf(ttEvent->freq());

	 editTTFreq->setText(oss2.str());
         activate(editTTAmpl);
         activate(editTTFreq);
         break;

      case ADC_EVENT_ID:
         changed=false;
         adcEvent = (AdcEvent *)e;
         oss1 << (int) roundf(adcEvent->angle());
         if (oss1.str() != editADCangle->text()) {
             editADCangle->setText(oss1.str());
             changed=true;
         }

         oss2 << (int) roundf(adcEvent->disp())+65;
         if (oss2.str() != editADCdisp->text()) {
             editADCdisp->setText(oss2.str());
             changed=true;
         }

         if (changed) {
             checkADCSync->setChecked( adcEvent->sync());
             activate(editADCangle);
             activate(editADCdisp);
             activate(checkADCSync);
         }
	 break;

      case CAMERALENS_EVENT_ID:
         cameraLensEvent = (CameraLensEvent *)e;
         oss1 << (int) (cameraLensEvent->xpos()*1e6);
         editCLX->setText(oss1.str());

         oss2 << (int) (cameraLensEvent->ypos()*1e6);
         editCLY->setText(oss2.str());

         activate(editCLX);
         activate(editCLY);
	 break;

      case CLEAN_EVENT_ID:
	 break;

      default:
         break;
      }
}


int BoardGui::ccd39_notify( void *thisPtr, Variable *var)
{
   BoardGui *ptr = dynamic_cast<BoardGui*>((AOApp*)thisPtr);

   if (var) {
      ptr->v_ccd39Bin->MatchAndSet(var);
      ptr->v_ccd39Freq->MatchAndSet(var);
      ptr->v_ccd39Temp->MatchAndSet(var);
   }

   int bin, temp1, temp2, temp3;
   double framerate;
   ptr->v_ccd39Bin->Get(&bin);
   ptr->v_ccd39Freq->Get(&framerate);
   ptr->v_ccd39Temp->Get(0, &temp1);
   ptr->v_ccd39Temp->Get(1, &temp2);
   ptr->v_ccd39Temp->Get(2, &temp3);

   Ccd39Event *e = new Ccd39Event( bin, framerate, temp1, temp2, temp3);
   postEvent(ptr, e);
   return 0;
}


int BoardGui::ccd47_notify( void *thisPtr, Variable *var) {
   BoardGui *ptr = dynamic_cast<BoardGui*>((AOApp*)thisPtr);

   if (var) {
      ptr->v_ccd47Bin->MatchAndSet(var);
      ptr->v_ccd47Freq->MatchAndSet(var);
      ptr->v_ccd47Temp->MatchAndSet(var);
   }

   int bin, temp1, temp2, temp3;
   double framerate;
   ptr->v_ccd47Bin->Get(&bin);
   ptr->v_ccd47Freq->Get(&framerate);
   ptr->v_ccd47Temp->Get(0, &temp1);
   ptr->v_ccd47Temp->Get(1, &temp2);
   ptr->v_ccd47Temp->Get(2, &temp3);

   Ccd47Event *e = new Ccd47Event( bin, framerate, temp1, temp2, temp3);
   postEvent(ptr, e);
   return 0;
}


int BoardGui::stages_notify( void *thisPtr, Variable *var)
{
   BoardGui *ptr = dynamic_cast<BoardGui*>((AOApp*)thisPtr);

   if (var) {
      ptr->v_curPosX->MatchAndSet(var);
      ptr->v_curPosY->MatchAndSet(var);
      ptr->v_curPosZ->MatchAndSet(var);

      ptr->v_tgtPosX->MatchAndSet(var);
      ptr->v_tgtPosY->MatchAndSet(var);
      ptr->v_tgtPosZ->MatchAndSet(var);
   }

   float xfield[2] = { 0, 36 };
   float yfield[2] = { 0, 20 };

   QSize size = ptr->stagesFrame->size();

   double curX, curY, curZ, tgtX, tgtY, tgtZ;
   ptr->v_curPosX->Get(&curX);
   ptr->v_curPosY->Get(&curY);
   ptr->v_curPosZ->Get(&curZ);

   ptr->v_tgtPosX->Get(&tgtX);
   ptr->v_tgtPosY->Get(&tgtY);
   ptr->v_tgtPosZ->Get(&tgtZ);

   float xcurpos = size.width() - (size.width() / (xfield[1]-xfield[0])) * (curX - xfield[0]);
   float ycurpos = size.height() - (size.height()/ (yfield[1]-yfield[0])) * (curY - yfield[0]);

   float xtgtpos = size.width() - (size.width() / (float(xfield[1])-xfield[0])) * (tgtX - xfield[0]);
   float ytgtpos = size.height() - (size.height()/ (float(yfield[1])-yfield[0])) * (tgtY - yfield[0]);

   StagesEvent *e = new StagesEvent( curX, curY, curZ, xcurpos, ycurpos, xtgtpos, ytgtpos);
   postEvent(ptr, e);

   return 0;
}

int BoardGui::fw1_notify( void *thisPtr, Variable *var) {
   BoardGui *ptr = dynamic_cast<BoardGui*>((AOApp*)thisPtr);

   if (var) {
      ptr->v_fw1Pos->MatchAndSet(var);
   }

   string pos = ptr->v_fw1Pos->Get();

   Fw1Event *e = new Fw1Event( pos);
   postEvent(ptr, e);
   return 0;
}


int BoardGui::fw2_notify( void *thisPtr, Variable *var) {
   BoardGui *ptr = dynamic_cast<BoardGui*>((AOApp*)thisPtr);

   if (var) {
      ptr->v_fw2Pos->MatchAndSet(var);
   }

   string pos = ptr->v_fw2Pos->Get();

   Fw2Event *e = new Fw2Event( pos);
   postEvent(ptr, e);
   return 0;
}


int BoardGui::tel_notify( void *thisPtr, Variable *var) {
   BoardGui *ptr = dynamic_cast<BoardGui*>((AOApp*)thisPtr);

   if (var) {
      ptr->v_derotPos->MatchAndSet(var);
      ptr->v_telAz->MatchAndSet(var);
      ptr->v_telEl->MatchAndSet(var);
   }

   double pos, az, el;
   ptr->v_derotPos->Get(&pos);
   ptr->v_telAz->Get(&az);
   ptr->v_telEl->Get(&el);
   TelEvent *e = new TelEvent( pos, az, el);
   postEvent(ptr, e);
   return 0;
}


int BoardGui::tt_notify( void *thisPtr, Variable *var) {
   BoardGui *ptr = dynamic_cast<BoardGui*>((AOApp*)thisPtr);

   if (var) {
      ptr->v_ttAmp->MatchAndSet(var);
      ptr->v_ttFreq->MatchAndSet(var);
   }

   double amp, freq;
   ptr->v_ttAmp->Get(&amp);
   ptr->v_ttFreq->Get(&freq);

   TTEvent *e = new TTEvent( amp*1e3, freq);
   postEvent(ptr, e);
   return 0;
}


int BoardGui::rerot_notify( void *thisPtr, Variable *var) {
   BoardGui *ptr = dynamic_cast<BoardGui*>((AOApp*)thisPtr);

   if (var) {
      ptr->v_rerotPos->MatchAndSet(var);
      //ptr->v_rerotSync->MatchAndSet(var);
   }

   double pos;
   int sync=0;
   ptr->v_rerotPos->Get(&pos);
   //ptr->v_rerotSync->Get(&sync);

   RerotEvent *e = new RerotEvent( pos, sync);
   postEvent(ptr, e);
   return 0;
}

int BoardGui::cube_notify( void *thisPtr, Variable *var) {
   BoardGui *ptr = dynamic_cast<BoardGui*>((AOApp*)thisPtr);

   if (var) {
      ptr->v_cubePos->MatchAndSet(var);
   }

   double pos;
   ptr->v_cubePos->Get(&pos);

   CubeEvent *e = new CubeEvent( pos);
   postEvent(ptr, e);
   return 0;
}


int BoardGui::adc_notify( void *thisPtr, Variable *var) {
   BoardGui *ptr = dynamic_cast<BoardGui*>((AOApp*)thisPtr);

   if (var) {
      ptr->v_adcPos1->MatchAndSet(var);
      ptr->v_adcPos2->MatchAndSet(var);
      //ptr->v_adcSync->MatchAndSet(var);
   }

   double pos1, pos2;
   int sync=0;
   ptr->v_adcPos1->Get(&pos1);
   ptr->v_adcPos2->Get(&pos2);
   //ptr->v_adcSync->Get(&sync);

   float angle = (pos1+pos2)/2.0;
   float disp = pos1-angle;

   AdcEvent *e = new AdcEvent( angle, disp, sync);
   postEvent(ptr, e);
   return 0;
}

int BoardGui::cl_notify( void *thisPtr, Variable *var) {
   BoardGui *ptr = dynamic_cast<BoardGui*>((AOApp*)thisPtr);

   if (var) {
      ptr->v_clPosX->MatchAndSet(var);
      ptr->v_clPosY->MatchAndSet(var);
   }

   double posx, posy;
   ptr->v_clPosX->Get(&posx);
   ptr->v_clPosY->Get(&posy);

   CameraLensEvent *e = new CameraLensEvent( posx, posy);
   postEvent(ptr, e);
   return 0;
}


void BoardGui::activate( QWidget *widget) {
    widget->setPaletteForegroundColor( Qt::red);
    active.push_back(widget);
}


void BoardGui::Periodic() {

    vector<QWidget *>::iterator iter;
    for ( iter = active.begin(); iter != active.end(); iter++) {
        DeactivateEvent *e = new DeactivateEvent( *iter);
        postEvent( this, e);
    }
    active.clear();
    
}

static const KCmdLineOptions options[] =
{
      {"f <file>", ("Configuration file"), 0 },
      {"i <identity>", ("MsgD identity"), 0 },
      KCmdLineLastOption
};

int main( int argc, char *argv[]) {

   SetVersion(VERS_MAJOR,VERS_MINOR);

   KCmdLineArgs::init( argc, argv, "BoardGui", "BoardGui", "BoardGui", "1.0" );
   KCmdLineArgs::addCmdLineOptions(options);
   KApplication kApp;

   BoardGui *gui = NULL;

   try {
      if (argc>1)
         gui = new BoardGui( argc, argv, kApp);
      else {
         gui = new BoardGui("boardgui", AOApp::getConffile("boardgui"), kApp);
      }

      gui->doGui(gui, true, true);

      delete gui;
      }
   catch (LoggerFatalException &e) {
      // In this case the logger can't log!!!
      printf("%s\n", e.what().c_str());
   }
   catch (AOException &e) {
      printf("Error: %s\n", e.what().c_str());
      Logger::get(Logger::LOG_LEV_ERROR)->log(Logger::LOG_LEV_ERROR, "Error: %s", e.what().c_str());
   }

   return 0;
}
  


