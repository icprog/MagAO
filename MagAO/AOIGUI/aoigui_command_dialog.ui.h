/****************************************************************************
** ui.h extension file, included from the uic-generated form implementation.
**
** If you want to add, delete, or rename functions or slots, use
** Qt Designer to update this file, preserving your code.
**
** You should not define a constructor or destructor in this file.
** Instead, write your code in functions called init() and destroy().
** These will automatically be called by the form's constructor and
** destructor.
*****************************************************************************/

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include "aoigui_warning_dialog.uic.h"

#include <qvalidator.h>
#include <qmessagebox.h>
#include <cmath>
//#include <tcs/networkconfig/NetworkConfig.hpp>


// AO Supervisor dependencies
extern "C" {
#include "base/globals.h"
}

// helper class to convert numerical values to text in a QString
#include "myqstring.h"
//#include "aos/AOScmdstat.hpp"

// AOS command timeouts
//#include "aos/AOSTimeouts.hpp"

#include "QMagAOIClient.h"

static QDoubleValidator *doubleValidator;
static QIntValidator *intValidator;

//using namespace lbto;

double nudgeae_rot;
double nudgeae_par;

void AOIGUICommandDialog::init()
{
   lastUpdate = -1;

   setModal(false);

   presetVisAOButton->hide();
   presetVisAO_status->hide();

   wfsSpecLabel->hide();
   presetAO_slNull_Edit->hide();
   colorIdLabel->hide();
   presetAO_colIndex_Edit->hide();  
   presetAO_ra_Edit->hide();
   presetAO_decl_Edit->hide();
   textLabel3_2_2_4->hide();
   textLabel3_2_3_2_2->hide();
   textLabel3_2_3_2_3->hide();
   //textLabel3_2_2_2->hide();
   textLabel1->hide();
   textLabel1_2->hide();

   presetAO_ClioFOV->insertItem(QString("Wide / Full"),0);
   presetAO_ClioFOV->insertItem(QString("Rotation Center"),1);
   presetAO_ClioFOV->insertItem(QString("Wide / Stamp"),2);
   presetAO_ClioFOV->insertItem(QString("Wide / Sub-stamp"),3);
   presetAO_ClioFOV->insertItem(QString("Narrow / Full"),4);
   presetAO_ClioFOV->insertItem(QString("Narrow / Strip"),5);
   presetAO_ClioFOV->insertItem(QString("Narrow / Stamp"),6);
   presetAO_ClioFOV->insertItem(QString("Narrow / Sub-stamp"),7);

   presetAO_ClioFOV->setCurrentItem(0);

   presetAOSeeingCombo->insertItem(QString("very bad"),0);
   presetAOSeeingCombo->insertItem(QString("bad (>0.7\")"),1);
   presetAOSeeingCombo->insertItem(QString("good (0.5\"-0.7\")"),2);
   presetAOSeeingCombo->insertItem(QString("damn good (<0.5\")"),3);
   presetAOSeeingCombo->setCurrentItem(2);

   processing = false;
    
   setNewInstrument_Edit_CBox->insertItem(QString("50/50 (bright stars)"),0);
   setNewInstrument_Edit_CBox->insertItem(QString("Window (faint stars)"),1);
   setNewInstrument_Edit_CBox->insertItem(QString("800 Dichroic (very bright stars)"),2);
   setNewInstrument_Edit_CBox->insertItem(QString("950 Dicrhoic (very bright stars)"),3);
   setNewInstrument_Edit_CBox->setCurrentItem(0);

    // Create the input validators
    doubleValidator = new QDoubleValidator(this);
    intValidator = new QIntValidator(this);
    
    // Assign the validators to the edit boxes
    presetAO_ra_Edit->setValidator(doubleValidator);
    presetAO_decl_Edit->setValidator(doubleValidator);
    presetAO_appMag_Edit->setValidator(doubleValidator);
    presetAO_colIndex_Edit->setValidator(doubleValidator);
    
    nudgeAmount->insertItem(QString("0.25"),0);
    nudgeAmount->insertItem(QString("0.50"),1);
    nudgeAmount->insertItem(QString("1.00"),2);
    nudgeAmount->setCurrentItem(1);


    offsetXY_X_Edit->setValidator(doubleValidator);
    offsetXY_Y_Edit->setValidator(doubleValidator);
    offsetZ_Z_Edit->setValidator(doubleValidator);
    
    // Update cycle for timeout progress bar
    timeoutNSteps = 0;
    timeoutCounter = -1;
    timeoutTimer = new QTimer(this);
    connect (timeoutTimer, SIGNAL(timeout ()), this, SLOT(updateProgressBar ()));

   guiderAcqDone = 0;
}

void AOIGUICommandDialog::destroy()
{
    delete timeoutTimer;
    delete doubleValidator;
    delete intValidator;
}

void AOIGUICommandDialog::updateValues()
{
   MyQString numericValue;
   double dval;
   int ival;
   
   // Initialize the various parameter boxes
   if(aoi->side.cmds.updateargs)
   {
      aoi->side.cmds.updateargs = 0;
      
      presetFlat_Edit_CBox->setCurrentText(QString(aoi->side.cmds.presetflat.flatspec));
      
      
      
      presetAO_slNull_Edit->setText(QString(aoi->side.cmds.presetao.wfsSpec));
      
      dval=aoi->side.cmds.presetao.cindex;
      numericValue.setValue(dval);
      presetAO_colIndex_Edit->setText(numericValue);
      
      //dval=aoi->side.cmds.presetao.mag;
      //numericValue.setValue(dval);
      //presetAO_appMag_Edit->setText(numericValue);
      
      dval=aoi->side.cmds.presetao.rocoordx;
      numericValue.setValue(dval,'f',2);
      presetAO_ra_Edit->setText(numericValue);
      
      dval=aoi->side.cmds.presetao.rocoordy;
      numericValue.setValue(dval,'f',2);
      presetAO_decl_Edit->setText(numericValue);
      
      
      dval=aoi->side.cmds.offsetxy.x;
      numericValue.setValue(dval);
      offsetXY_X_Edit->setText(numericValue);
      
      dval=aoi->side.cmds.offsetxy.y;
      numericValue.setValue(dval);
      offsetXY_Y_Edit->setText(numericValue);
      
      dval=aoi->side.cmds.offsetz.z;
      numericValue.setValue(dval);
      offsetZ_Z_Edit->setText(numericValue);
      
      
      // Update command status
      setResult(presetFlat_status, aoi->side.cmds.presetflat.stat, aoi->side.cmds.presetflat.errMsg);
      setResult(presetAO_status, aoi->side.cmds.presetao.stat, aoi->side.cmds.presetao.errMsg);
      setResult(presetVisAO_status, aoi->side.cmds.presetvisao.stat, aoi->side.cmds.presetvisao.errMsg);

      setResult(acquireGuider_status, aoi->side.cmds.acqguider.stat, aoi->side.cmds.acqguider.errMsg);
   
      setResult(startAO_status, aoi->side.cmds.startao.stat, aoi->side.cmds.startao.errMsg);
      setResult(offsetXY_status, aoi->side.cmds.offsetxy.stat, aoi->side.cmds.offsetxy.errMsg);
      setResult(offsetZ_status, aoi->side.cmds.offsetz.stat, aoi->side.cmds.offsetz.errMsg);
      setResult(pause_status, aoi->side.cmds.pause.stat, aoi->side.cmds.pause.errMsg);
      setResult(resume_status, aoi->side.cmds.resume.stat, aoi->side.cmds.resume.errMsg);
      setResult(stop_status, aoi->side.cmds.stop.stat, aoi->side.cmds.stop.errMsg);
   }

   //Display seeing
   // -- use DIMM if it's current
   // -- otherwise use Baade if current
   // -- display nothing otherwise

   float fw = aoi->environ.dimmfwhm;
   if(fw <= 0) fw = aoi->environ.mag1fwhm;

   
   char dimmstr[20];

   if(fw <= 0)
   {
      snprintf(dimmstr, 20, "[---]");
   }
   else
   {
      //Correct for elevation
      fw /= pow( cos( (90 - aoi->el.position)*3.14159/180.), 3./5.);

      snprintf(dimmstr, 20, "[%0.2f\"]", fw);
   }
   dimmSeeing->setText(dimmstr);

}

void AOIGUICommandDialog::setResult( QLabel* label, int result, std::string errMsg )
{
   time_t ltime;
   struct tm * tptr;
   char msg[1024], tm[512];
   switch (result)
   {
      case CMD_RUNNING:
         label->setPaletteBackgroundColor(QColor(0,0,0));
         label->setPaletteForegroundColor(QColor(85, 255, 255));
         label->setText(CMD_RUNNING_STR);
         break;
         
      case CMD_FAILURE:
         label->setPaletteBackgroundColor(QColor(0,0,0));
         label->setPaletteForegroundColor(QColor(255, 0, 0));

         ltime = time(0);
         tptr = localtime(&ltime);
         strftime(tm, 512, "%H:%M:%S", tptr);
         //tm[8] = 0;
         snprintf(msg, 1024, "%s %s", tm, errMsg.c_str());
         resultText->append(msg);
         label->setText(CMD_FAILURE_STR);
         break;
         
      case CMD_SUCCESS:
         label->setPaletteBackgroundColor(QColor(0,0,0));
         label->setPaletteForegroundColor(QColor(100, 255, 100));
         
         label->setText(CMD_SUCCESS_STR);

         ltime = time(0);
         tptr = localtime(&ltime);
         strftime(tm, 512, "%H:%M:%S", tptr);
         //tm[8] = 0;
         snprintf(msg, 1024, "%s %s", tm, errMsg.c_str());
         resultText->append(msg);

         break;
         
      default:
         label->setPaletteBackgroundColor(QColor(0,0,0));
         label->setPaletteForegroundColor(QColor(100, 255, 100));
         label->setText("");
   }
}

void AOIGUICommandDialog::setValue( QLabel* label, double result )
{
   MyQString numRes;
   label->setPaletteForegroundColor(QColor(255, 0, 0));
   label->setPaletteBackgroundColor(QColor(0, 0, 0));
   
   numRes.setValue(result);
   label->setText(numRes);
}

void AOIGUICommandDialog::setValue( QLabel * label, int result )
{
   MyQString numRes;
   label->setPaletteForegroundColor(QColor(255, 0, 0));
   label->setPaletteBackgroundColor(QColor(0, 0, 0));
   
   numRes.setValue(result);
   label->setText(numRes);
   
}

void AOIGUICommandDialog::setValue( QLabel * label, string value )
{
   label->setPaletteForegroundColor(QColor(255, 0, 0));
   label->setPaletteBackgroundColor(QColor(0, 0, 0));
   
   label->setText(QString(value));
}


// void AOIGUICommandDialog::CloseButton_clicked()
// {
//    hide();
// }
// 
// void AOIGUICommandDialog::done( int r )
// {
//    hide();
// }


void AOIGUICommandDialog::PresetFlatAction()
{
   if (getRunningStatus())
   {
      return;
   }
   setRunning();
   
   //AOSClient      aoClient(side);
   //CSQHandle      handle;
   //CommandReturn  cmdReturn;
   
   presetFlat_status->setPaletteForegroundColor(QColor(85, 255, 255));
   presetFlat_status->setPaletteBackgroundColor(QColor(0, 0, 0));
   
   magaoic->PresetFlat(presetFlat_Edit_CBox->currentText().ascii());
   
   startTimer(aoi->side.cmds.presetflat.tmout);
   presetFlat_status->setText(CMD_RUNNING_STR);
}


void AOIGUICommandDialog::PresetAOAction()
{
   if (getRunningStatus())
      return;
   setRunning();
   
   //AOSClient    aoClient(side);
   //CSQHandle    handle;
   //CommandReturn   cmdReturn;
   
   presetAO_status->setPaletteForegroundColor(QColor(85, 255, 255));
   presetAO_status->setPaletteBackgroundColor(QColor(0, 0, 0));

   QString cliofov;
   switch(presetAO_ClioFOV->currentItem())
   {
      case 0:
         cliofov = "wide_full";
         break;
      case 1:
         cliofov = "wide_strip";
         break;
      case 2:
         cliofov = "wide_stamp";
         break;
      case 3:
         cliofov = "wide_substamp";
         break;
      case 4:
         cliofov = "narrow_full";
         break;
      case 5:
         cliofov = "narrow_strip";
         break;
      case 6:
         cliofov = "narrow_stamp";
         break;
      case 7:
         cliofov = "narrow_substamp";
         break;
      default:
         cliofov = "wide_full";
   }

   int badseeing;
   switch(presetAOSeeingCombo->currentItem())
   {
      case 0:
         badseeing = -1;
         break;
      case 1:
         badseeing = 0;
         break;
      case 2:
         badseeing = 1;
         break;
      case 3:
         badseeing = 2;
         break;
      default:
         badseeing = 1;
   }

   bool adcTracking = adcTrackingCheck->isChecked();

   //Make sure current selection is set.
   setNewInstrument_Edit_CBox_activated(setNewInstrument_Edit_CBox->currentItem());

   magaoic->PresetAO( cliofov,
                       presetAO_slNull_Edit->text().ascii(),
                       presetAO_ra_Edit->text().toDouble(),
                       presetAO_decl_Edit->text().toDouble(),
                       presetAO_appMag_Edit->text().toDouble(),
                       badseeing,
                       adcTracking);

   startTimer(aoi->side.cmds.presetao.tmout);
   presetAO_status->setText(CMD_RUNNING_STR);

   guiderAcqDone = 1;
}

void AOIGUICommandDialog::acquireGuiderButton_clicked()
{

   if(guiderAcqDone == 0)
   {
      if(QMessageBox::warning(this, "Acquire From Guider Request",
             "You have not Preset AO -- the bayside stages may not be in the right place.\n\n!!! THIS WILL MOVE THE TELESCOPE !!!\n\nDo you want to continue?",
             "Continue","Cancel","",1) != 0)
         return;
   }

   if(guiderAcqDone == 2)
   {
      if(QMessageBox::warning(this, "Acquire From Guider Request",
             "You have not Preset AO and/or have already acquired from the guider\n\n!!! THIS WILL MOVE THE TELESCOPE !!!\n\nDo you want to continue?",
             "Continue","Cancel","",1) != 0)
         return;
   }

   magaoic->AcquireFromGuider();
   
   startTimer(aoi->side.cmds.acqguider.tmout);
   aoi->side.cmds.acqguider.stat = CMD_RUNNING;
   acquireGuider_status->setText(CMD_RUNNING_STR);

   guiderAcqDone = 2;
}

void AOIGUICommandDialog::nudge(int xsign, int ysign)
{
   

   double amount = 0.5;
   int amountIndex = nudgeAmount->currentItem();

   double rotang = aoi->rotator.angle;
   double rerot = aoi->side.wfs1.rerotator_angle;
   double ang;
   switch(amountIndex)
   {
      case 0:
         amount = 0.25;
         break;
      case 1:
         amount = 0.5;
         break;
      case 2:
         amount = 1.00;
         break;
      default:
         amount = 0.5;
   }

   std::cout << "Nudge: " << xsign*amount << " " << ysign*amount << "\n";

   ang = (rotang + 2*rerot-90);   

   std::cout << nudgeae_rot << " " << nudgeae_par << "\n";

   double x = xsign*cos(ang*3.14159/180.) - nudgeae_par*ysign*sin(ang*3.14159/180.);
   double y = xsign*sin(ang*3.14159/180.) + nudgeae_par*ysign*cos(ang*3.14159/180.);

   magaoic->NudgeAE(x*amount, y*amount);
}

void AOIGUICommandDialog::nudgeUp_clicked()
{
   nudge(0,1);
}


void AOIGUICommandDialog::nudgeDown_clicked()
{
   nudge(0,-1);
}


void AOIGUICommandDialog::nudgeLeft_clicked()
{
   nudge(1,0);
}


void AOIGUICommandDialog::nudgeRight_clicked()
{
   nudge(-1,0);
}


void AOIGUICommandDialog::StartAOAction()
{
   if (getRunningStatus())
      return;
   setRunning();
   
   //AOSClient    aoClient(side);
   //CSQHandle    handle;
   //CommandReturn   cmdReturn;
   
   startAO_status->setPaletteForegroundColor(QColor(85, 255, 255));
   startAO_status->setPaletteBackgroundColor(QColor(0, 0, 0));
   
   magaoic->StartAO();
   
   
   startTimer(aoi->side.cmds.startao.tmout);
   startAO_status->setText(CMD_RUNNING_STR);
}


void AOIGUICommandDialog::OffestXYAction()
{
   if (getRunningStatus())
      return;
   setRunning();
   
   //AOSClient    aoClient(side);
   //CSQHandle    handle;
   //CommandReturn   cmdReturn;
   
   offsetXY_status->setPaletteForegroundColor(QColor(85, 255, 255));
   offsetXY_status->setPaletteBackgroundColor(QColor(0, 0, 0));
   
   magaoic->OffsetXY( offsetXY_X_Edit->text().toDouble(),
                      offsetXY_Y_Edit->text().toDouble() );
   startTimer(aoi->side.cmds.offsetxy.tmout);
   offsetXY_status->setText(CMD_RUNNING_STR);
}


void AOIGUICommandDialog::OffsetZAction()
{
   if (getRunningStatus())
      return;
   setRunning();
   
   //AOSClient    aoClient(side);
   //CSQHandle    handle;
   //CommandReturn   cmdReturn;
   
   offsetZ_status->setPaletteForegroundColor(QColor(85, 255, 255));
   offsetZ_status->setPaletteBackgroundColor(QColor(0, 0, 0));
   
   magaoic->OffsetZ(offsetZ_Z_Edit->text().toDouble());
   startTimer(aoi->side.cmds.offsetz.tmout);
   offsetZ_status->setText(CMD_RUNNING_STR);
}


void AOIGUICommandDialog::StopAction()
{
   if (getRunningStatus())
      return;
   setRunning();
      
   stop_status->setPaletteForegroundColor(QColor(85, 255, 255));
   stop_status->setPaletteBackgroundColor(QColor(0, 0, 0));
   
   magaoic->Stop("Stopped from operator console");
   startTimer(aoi->side.cmds.stop.tmout);
   stop_status->setText(CMD_RUNNING_STR);
}


void AOIGUICommandDialog::PauseAction()
{
   if (getRunningStatus())
      return;
   setRunning();
   
   //AOSClient    aoClient(side);
   //CSQHandle    handle;
   //CommandReturn   cmdReturn;
   
   pause_status->setPaletteForegroundColor(QColor(85, 255, 255));
   pause_status->setPaletteBackgroundColor(QColor(0, 0, 0));
   
   magaoic->Pause();
   startTimer(aoi->side.cmds.pause.tmout);
   pause_status->setText(CMD_RUNNING_STR);
}


void AOIGUICommandDialog::ResumeAction()
{
   if (getRunningStatus())
      return;
   setRunning();
   
   //AOSClient    aoClient(side);
   //CSQHandle    handle;
   //CommandReturn   cmdReturn;
   
   resume_status->setPaletteForegroundColor(QColor(85, 255, 255));
   resume_status->setPaletteBackgroundColor(QColor(0, 0, 0));
   
   magaoic->Resume();
   startTimer(aoi->side.cmds.resume.tmout);
   resume_status->setText(CMD_RUNNING_STR);
}

void AOIGUICommandDialog::AbortAction()
{
   //Issue the stop command, let the resulting errors propagate to GUI.
   magaoic->AOArbStop();
}


void AOIGUICommandDialog::checkRunningStatus()
{
    // Update processing status
    aoi->side.cmds.busy = (aoi->side.cmds.presetflat.stat == CMD_RUNNING || aoi->side.cmds.presetao.stat == CMD_RUNNING || aoi->side.cmds.acqguider.stat == CMD_RUNNING
                            || aoi->side.cmds.checkref.stat == CMD_RUNNING|| aoi->side.cmds.refine.stat == CMD_RUNNING ||
                             aoi->side.cmds.modify.stat == CMD_RUNNING || aoi->side.cmds.startao.stat == CMD_RUNNING ||
                              aoi->side.cmds.offsetxy.stat == CMD_RUNNING || aoi->side.cmds.offsetz.stat == CMD_RUNNING ||
                               aoi->side.cmds.pause.stat == CMD_RUNNING || aoi->side.cmds.resume.stat == CMD_RUNNING ||
                                aoi->side.cmds.stop.stat == CMD_RUNNING || aoi->side.cmds.wfson.stat == CMD_RUNNING ||
                                aoi->side.cmds.wfsoff.stat == CMD_RUNNING);

   
    processing = aoi->side.cmds.busy;

}


bool AOIGUICommandDialog::getRunningStatus()
{
   checkRunningStatus();
   return processing;
}


void AOIGUICommandDialog::setRunning()
{
   processing = true;
}


void AOIGUICommandDialog::updateProgressBar()
{
   timeoutCounter ++;
   progressBar->setProgress(timeoutCounter);


   if (!getRunningStatus() || timeoutCounter >= timeoutNSteps)
      stopTimer();
}


void AOIGUICommandDialog::startTimer(int timeout)
{
   timeoutCounter = 0;
   timeoutNSteps = timeout / 100;
   progressBar->setProgress(0);
   progressBar->setTotalSteps(timeoutNSteps);
   timeoutTimer->start(100);
   
}


void AOIGUICommandDialog::stopTimer()
{
   timeoutTimer->stop();
   timeoutCounter = -1;
   progressBar->setProgress(timeoutNSteps);
   progressBar->setTotalSteps(1);
}

void AOIGUICommandDialog::setSide( int cSide )
{
   side = cSide;
   
   setCaption(QString("MagAO Commands"));
   sideLabel->setText("");
   
   
   string flatnameListFileName;
   
   flatnameListFileName = aoi->side.adsec.fl_filename;
   
   FILE *fd = fopen(flatnameListFileName.c_str(), "r");
   
   
   if (fd != NULL)
   {
      char flatname[1024];
      
      ///-///SysLog::log(string("Reading flat names from file: ")+flatnameListFileName);
      while (fscanf(fd, "%s", flatname) > 0)
         presetFlat_Edit_CBox->insertItem(QString(flatname));
      
      fclose(fd);
   }
   else
   {
      ///-///SysLog::log(string("Cannot open flat file: ")+flatnameListFileName);
   }
}


void AOIGUICommandDialog::setQMagAOIClient( QMagAOIClient * qaoic )
{
   magaoic = qaoic;
   aoi = &magaoic->aoi;

   qaoic->get_nudegeae_rot(nudgeae_rot, nudgeae_par);

}


void AOIGUICommandDialog::PresetVisAOAction()
{
   if (getRunningStatus())
      return;
   setRunning();
   
   //AOSClient    aoClient(side);
   //CSQHandle    handle;
   //CommandReturn   cmdReturn;
   
   resume_status->setPaletteForegroundColor(QColor(85, 255, 255));
   resume_status->setPaletteBackgroundColor(QColor(0, 0, 0));
   
   magaoic->PresetVisAO();
   startTimer(aoi->side.cmds.presetvisao.tmout);
   presetVisAO_status->setText(CMD_RUNNING_STR);


}

void AOIGUICommandDialog::setNewInstrument_Edit_CBox_activated( int k )
{

   std::string instr;


   switch(k)
   {
      case 0:
         instr = "MagAO_bs5050";
         break;
      case 1:
         instr = "MagAO_bsWindow";
         break;
      case 2:
         instr = "MagAO_bs800";
         break;
      case 3:
         instr = "MagAO_bs950";
         break;
      default:
         instr = "MagAO_bs5050";
   }

   magaoic->SetNewInstrument(instr, "");
   

}


void AOIGUICommandDialog::presetAOSeeingCombo_activated( int k )
{
   /*if(k == 0)
   {
      AOSGUIWarningDialog *warningDialog;

      warningDialog = new AOSGUIWarningDialog(this, "WARNING", TRUE);
      warningDialog->warningMessage->setText(QString("You are in LBT conditions.\nThe dome will be closed."));
      warningDialog->exec();
      delete warningDialog;

      warningDialog = new AOSGUIWarningDialog(this, "ha ha", TRUE);
      warningDialog->warningMessage->setText(QString("Just kidding.\nSetting bad seeing."));
      warningDialog->exec();
      delete warningDialog;

      presetAOSeeingCombo->setCurrentItem(1);
   }*/
}





