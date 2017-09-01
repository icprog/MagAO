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


/////Don't know what these do:
#define WFS_ON 1
#define ADSEC_SET 2
#define ADSEC_SAFE 1

///Don't know where this is orginally defined
#ifndef AO_WFS_MAGAO
#define AO_WFS_MAGAO "MAGWFS"
#endif

// AO Supervisor dependencies
extern "C"
{
#include "base/globals.h"
}

// Qwt widget include
#include <qwt.h>
#include <qwt_plot.h>
#include <qwt_plot_curve.h>
#include <qwt_plot_canvas.h>
#include <qwt_compass.h>
#include <qwt_compass_rose.h>
#include <qwt_dial_needle.h>

// helper class to convert numerical values to text in a QString
#include "myqstring.h"

#include "version.h"


// QTimer update cycle in milliseconds
const int CYCLE = 1000; 

// Version 
static int GUIVmajor = 0;
static int GUIVminor = 0;
static int side;

// Plots variables
static QwtPlotCurve *countsCurveWfs1;
static QwtPlotCurve *strehlCurve;
static double countsBufferWfs1[61], strehlBuffer[61], secondsBuffer[61], integrationStepsBuffer[61];

// Image variables
#define IMAGE_X_SIZE 256
#define IMAGE_Y_SIZE 256

static QRgb colorMap[256];
static uchar fieldImageData1[IMAGE_X_SIZE * IMAGE_Y_SIZE];
static time_t fieldImageFileModTime1;
static int refreshCount1;
static int imageTypeWfs1;

// Error events management 
//#include "EventCallbackAOS.hpp"

// Manual commands window
#include "aoigui_command_dialog.uic.h"
static AOIGUICommandDialog *commandDialog;

// Warning dialogs
#include "aoigui_warning_dialog.uic.h"
static AOSGUIWarningDialog *warningIDLDialog = NULL;
static AOSGUIWarningDialog *warningDustDialog = NULL;
static AOSGUIWarningDialog *warningActFailDialog = NULL;
static AOSGUIWarningDialog *warningCheckPower = NULL;

// Info windows
#include "aoigui_about.uic.h"

static bool wfs1Enabled;
static QColor frameBackgroundColor;
static int currentLabmode;

// these variables are for counts integration
#define INTEGRATION_TIME 10
static int integrationCounterWfs1;
static int countSumWfs1;
static int maxScaleWfs1;

void AOIGUIForm::fileExit()
{
   //*///*/SysLog::log ("Exiting.\n");
   delete countsCurveWfs1;
   
   delete strehlCurve;
   delete commandDialog;
   
   QApplication::exit(0);
}

void AOIGUIForm::helpIndex()
{
    
}

void AOIGUIForm::helpContents()
{
    
}

void AOIGUIForm::helpAbout()
{
   AOSGUIAbout aboutDlg;
   
   QString Caption;
   
   Caption = "Magellan AO GUI Info";
   aboutDlg.setCaption(Caption);
   aboutDlg.sideLabel->setText("");
   
   QString vMaj, vMin, text;
   
   aboutDlg.aosVersion->setText(vMaj.setNum((int)(aoi->side.vmajor)) + "." + vMin.setNum((int)(aoi->side.vminor)));
   aboutDlg.aosGUIVersion->setText(vMaj.setNum(GUIVmajor) + "." + vMin.setNum(GUIVminor));
   
   aboutDlg.aosStartedAt->setText(QString(aoi->side.starttime));
   aboutDlg.aosSupervisorIP->setText(QString(aoi->side.msgdip));
   aboutDlg.msgDIdent->setText(QString(aoi->side.msgdident));
   aboutDlg.msgDStatus->setText(QString(aoi->side.msgdstat));
   aboutDlg.lastConnected->setText(QString(aoi->side.conntime));
   
   if (aoi->side.tel_enabled)
      aboutDlg.telemetryCollection->setText(QString("Enabled"));
   else
      aboutDlg.telemetryCollection->setText(QString("Disabled"));
   
   aboutDlg.logdir->setText(QString(aoi->side.logdir));
   
   aboutDlg.setModal(TRUE);
   aboutDlg.exec();
}

void AOIGUIForm::init()
{
   int i;
   lastUpdate = -1;
   integrationCounterWfs1 = 0;
   maxScaleWfs1 = 1;
   
   //*///*/SysLog::setPrefix ("AOSGUI");
   //*/ ("Version %d.%d\n", GUIVmajor, GUIVminor);
   
   frameBackgroundColor = paletteBackgroundColor();
   QString GUICaption;
   GUICaption = "Magellan AO";
   
   setCaption(GUICaption);
   
   magaoic = 0;
   //First initialize QMagAOIClient
   try
   {
      int argc = 5;
      time_t tmt = time(0);
      
      char tstmp[128];

      snprintf(tstmp, 128, "%s:%lu", "aoigui", (unsigned long) tmt);
      char *argv[] = {"QMagAOIClient", "-i", tstmp, "-f", "conf/adsec/current/processConf/aoigui.conf"};
      
      magaoic = new QMagAOIClient(argc, argv, this);
      
   }
   catch (AOException &e)
   {
      fprintf(stderr, "%s:%d: %s", __FILE__, __LINE__, e.what().c_str());
      magaoic = 0;
      throw(e);
   }
   catch(...)
   {
      std::cerr << "Problem creating QMagAOIClient.\n";
      magaoic = 0;
      throw;
   }
   
   magaoic->thread_exec();
   aoi = &magaoic->aoi;
   AOI_conn_status = 0;
   aoi->side.cmds.updateargs = 1; //force a GUI update.
   
   currentLabmode = aoi->side.labmode;
   
   // Initalize a grayscale colormop and set the image file modification time to 0
   for (i = 0; i < 256; i++)
      colorMap[i] = 0xff000000 | (i << 16) | (i << 8) | i;
   
   fieldImageFileModTime1 = 0;
   refreshCount1 = 0;
   
   // set image type to "Current"
   imageTypeWfs1 = 1;
   
   // initialize the compass for the two WFSs
   //wfs1FieldViewerCompass->setFrameShadow(QwtCompass::Sunken);
   //wfs1FieldViewerCompass->setMode(QwtDial::RotateScale);
   QwtSimpleCompassRose *rose1 = new QwtSimpleCompassRose(4, 1);
   //wfs1FieldViewerCompass->setRose(rose1);
   //wfs1FieldViewerCompass->setValue(0.0);
   //setFramePalette((QFrame *)wfs1FieldViewerCompass, backgroundColor());
   
   // Set the TextEdit widget to the LogText format so text
   // can be written in different colors.
   messageBox->setTextFormat (Qt::LogText);
   
   // Enable Tooltips - can be enabled/disabled via menu item
   QToolTip::setGloballyEnabled (TRUE);
   
   // Attach the curve and data classes to the plot widgets.
   for (i = 0; i < 60; i++)
      integrationStepsBuffer[i] = -6.0 + 0.1 * i;
   for (i = 0; i < 60; i++)
      secondsBuffer[i] = i;
   
   // Counts
      countsCurveWfs1 = new QwtPlotCurve("Counts Curve" );
      memset(countsBufferWfs1, 0, 61 * sizeof(int));
      countsCurveWfs1->setData(integrationStepsBuffer, countsBufferWfs1, 61);
      wfs1CCDCountsPlot->setAxisScale(QwtPlot::yLeft, 0, 100);
      wfs1CCDCountsPlot->setAxisMaxMajor(QwtPlot::yLeft, 4);
      wfs1CCDCountsPlot->setAxisMaxMajor(QwtPlot::xBottom, 7);
      wfs1CCDCountsPlot->setAxisMaxMinor(QwtPlot::xBottom, 3);
      countsCurveWfs1->attach(wfs1CCDCountsPlot);
      
      // Strehl
      strehlCurve = new QwtPlotCurve("Strehl Curve" );
      memset(strehlBuffer, 0, 61 * sizeof(int));
      strehlCurve->setData(secondsBuffer, strehlBuffer, 61);
      //aoStrehlPlot->setAxisScale(QwtPlot::yLeft, 0, 100);
      //aoStrehlPlot->setAxisMaxMajor(QwtPlot::yLeft, 4);
      //strehlCurve->attach(aoStrehlPlot);
      aoStrehlPlot->hide();

      // Update cycle for AOS GUI
      QTimer *counter = new QTimer(this);
      connect (counter, SIGNAL(timeout ()), this, SLOT(updateAOIGUIForm ()));
      counter->start (CYCLE);
      //*/SysLog::log ("Counter started");
      
      // Set maximum lines in the messages window
      messageBox->setMaxLogLines(100);
      
      // Create the manual commands window
      commandDialog = new AOIGUICommandDialog();
      
      wfs1Enabled = TRUE;
      
      // Define the error message handler
      //  these are the events we will catch here
      
}

void AOIGUIForm::updateAOIGUIForm()
{
   int i;

   //First we check if the AOApp thread is still running
   if(pthread_tryjoin_np(magaoic->exec_thread, 0) != EBUSY)
   {
      //It must not be running.
      aoi->side.wfs1.enabled = false;
      aoi->side.wfs1.active = 0;
      aoi->side.updateargs = 1;
   }
   else
   {
      aoi->side.wfs1.enabled = true;
      aoi->side.wfs1.active = 1;
   }
   
   if (!aoi->side.wfs1.enabled)
   {
      // not running --> red background
      
      setPaletteBackgroundColor(QColor(255, 0, 0));
      //operativeStatusBox->setText(QString("<font color=#ff0000>Not running</font>"));
      //wfs1FieldViewerPixmap->setAutoEraseBkg(TRUE);
      //wfs1FieldViewerPixmap->setText(QString("NO IMAGE"));
      wfs1Led->clear();
      wfs1Status->clear();
      wfs1CCDBinningBox->clear();
      wfs1CCDFrequencyBox->clear();
      //wfs1Active->setText(QString("Idle"));
      
      adsecLed->clear();
      adsecStatus->clear();
      adsecShape->clear();
      mainPowerStatus->clear();
      tssStatus->clear();
      coilStatus->clear();
      aoStatus->clear();
      //aoMode->clear();
      aoCorrectedModes->clear();
      aoSkipFrame->clear();
      aoOffloadBox_1->clear();
      aoOffloadBox_2->clear();
      aoOffloadBox_3->clear();
      
      //setFramePalette((QFrame *)wfs1FieldViewerCompass, backgroundColor());
      wfs1CCDCountsPlot->setCanvasBackground(backgroundColor());
      wfs1CCDCountsPlot->repaint();
      wfs1CCDCountsPlot->replot();
      
      //aoStrehlPlot->setCanvasBackground(backgroundColor());
      //aoStrehlPlot->repaint();
      //aoStrehlPlot->replot();
      
      commandDialog->setPaletteBackgroundColor(QColor(255, 0, 0));
      
      commandsButton->setEnabled(false);
      adsecOnButton->setEnabled(false);
      adsecOffButton->setEnabled(false);
      adsecSetButton->setEnabled(false);
      adsecRestButton->setEnabled(false);
      //wfsTab->setTabEnabled(wfsTab->page(1), false);
      //wfsTab->setTabEnabled(wfsTab->page(0), false);
      aoSLReady->setPaletteBackgroundColor(paletteBackgroundColor());
      aoAOReady->setPaletteBackgroundColor(paletteBackgroundColor());
      aoLoopClosed->setPaletteBackgroundColor(paletteBackgroundColor());
      adsecHealth->setPaletteBackgroundColor(paletteBackgroundColor());
      adsecElevUpd->setPaletteBackgroundColor(paletteBackgroundColor());
      adsecAnemUpd->setPaletteBackgroundColor(paletteBackgroundColor());
      wfs1Health->setPaletteBackgroundColor(paletteBackgroundColor());
      
      //Attempt to reconnect to MsgD
      if(magaoic) delete magaoic;
      try
      {
         int argc = 3;
         char *argv[] = {"QMagAOIClient", "-i", "aoigui"};
         
         magaoic = new QMagAOIClient(argc, argv, this);   
      }
      catch (AOException &e)
      {
         fprintf(stderr, "%s:%d: %s", __FILE__, __LINE__, e.what().c_str());
         magaoic = 0;
         throw(e);
      }
      catch(...)
      {
         std::cerr << "Problem creating QMagAOIClient.\n";
         magaoic = 0;
         throw;
      }
      magaoic->thread_exec();
   
      aoi = &magaoic->aoi;
      aoi->side.cmds.updateargs = 1; //force a GUI update.
      return;
   }
   else if (frameBackgroundColor != paletteBackgroundColor())
   {
      // reset the background to the default color
      setPaletteBackgroundColor(frameBackgroundColor);
      //setFramePalette((QFrame *)wfs1FieldViewerCompass, frameBackgroundColor);
      commandDialog->setPaletteBackgroundColor(frameBackgroundColor);
      wfs1CCDCountsPlot->setCanvasBackground(frameBackgroundColor);
      wfs1CCDCountsPlot->repaint();
      wfs1CCDCountsPlot->replot();
      
      //aoStrehlPlot->setCanvasBackground(frameBackgroundColor);
      //aoStrehlPlot->repaint();
      //aoStrehlPlot->replot();
      commandsButton->setEnabled(true);
      //wfsTab->setTabEnabled(wfsTab->page(0), true);
      //wfsTab->setTabEnabled(wfsTab->page(1), true);
   }
    
   if (aoi->side.updateargs  != lastUpdate)
   {
      lastUpdate = aoi->side.updateargs;
      
      //if (aoi->side.wfs1.active)
         //wfs1Active->setText(QString("Active"));
     // else
         //wfs1Active->setText(QString("Idle"));
   }
   
   if (aoi->side.wfs1.enabled != wfs1Enabled)
   {
      wfs1Enabled = aoi->side.wfs1.enabled;
      if (!wfs1Enabled)
      {
         //wfs1FieldViewerPixmap->setAutoEraseBkg(true);
         //wfs1FieldViewerPixmap->setText(QString(""));
         //wfs1FieldViewerPixmap->setPaletteBackgroundColor(frameBackgroundColor);
         //wfs1FieldViewerPixmap->setEnabled(false);
         
         wfs1Status->clear();
         wfs1Status->setPaletteBackgroundColor(frameBackgroundColor);
         wfs1Status->setEnabled(false);
         
         wfs1CCDBinningBox->clear();
         wfs1CCDBinningBox->setPaletteBackgroundColor(frameBackgroundColor);
         wfs1CCDBinningBox->setEnabled(false);
         wfs1CCDFrequencyBox->clear();
         wfs1CCDFrequencyBox->setPaletteBackgroundColor(frameBackgroundColor);              wfs1CCDFrequencyBox->setEnabled(false);
         
         for (i = 0; i < 60; countsBufferWfs1[i++] = 0);
         countsCurveWfs1->setData(integrationStepsBuffer, countsBufferWfs1, 61);
         wfs1CCDCountsPlot->setAxisScale(QwtPlot::yLeft, 0, 100);
         wfs1CCDCountsPlot->replot();
         wfs1CCDCountsPlot->setEnabled(false);
         
         //radioCurrentWfs1->setEnabled(false);
         //radioAcquisitionWfs1->setEnabled(false);
      }
      else
      {
         //wfs1Active->setText(QString("Idle"));
         //wfs1FieldViewerPixmap->setAutoEraseBkg(true);
         //wfs1FieldViewerPixmap->setText(QString("NO IMAGE"));
         //wfs1FieldViewerPixmap->setEnabled(true);

         //wfs1FieldViewerPixmap->setPaletteBackgroundColor(QColor(0,0,0));
         //wfs1FieldViewerPixmap->setEnabled(true);

         wfs1Status->setPaletteBackgroundColor(QColor(0,0,0));
         wfs1Status->setEnabled(true);

         wfs1CCDBinningBox->setPaletteBackgroundColor(QColor(0,0,0));
         wfs1CCDBinningBox->setEnabled(true);

         wfs1CCDFrequencyBox->setPaletteBackgroundColor(QColor(0,0,0));
         wfs1CCDFrequencyBox->setEnabled(true);

         wfs1CCDCountsPlot->setEnabled(true);

         //radioCurrentWfs1->setEnabled(true);
         //radioAcquisitionWfs1->setEnabled(false);
      }
   }

   if (aoi->side.idlstat == 0L && aoi->side.connected)
   {
      // IDL license manager is not running
      if (warningIDLDialog == NULL)
      {
         warningIDLDialog = new AOSGUIWarningDialog(this, "WARNING", TRUE);
         warningIDLDialog->warningMessage->setText(QString("IDL license missing"));
         warningIDLDialog->exec();
      }
   }
   else
   {
      if (warningIDLDialog != NULL)
      {
         warningIDLDialog->close();
         delete warningIDLDialog;
         warningIDLDialog = NULL;
      }
   }

   if (aoi->side.adsec.contamination)
   {
      // Dust contamination detected
      if (warningDustDialog == NULL)
      {
         warningDustDialog = new AOSGUIWarningDialog(this, "WARNING", TRUE);
         warningDustDialog->warningMessage->setText(QString("Dust contamination"));
         warningDustDialog->warningMessage2->setText(QString(aoi->side.adsec.popmsg));
         warningDustDialog->exec();
      }
   }
   else
   {
      if (warningDustDialog != NULL)
      {
         warningDustDialog->close();
         delete warningDustDialog;
         warningDustDialog = NULL;
      }
   }
    
   if (aoi->side.adsec.nwact)
   {
      // Not working actuator detected
      if (warningActFailDialog == NULL)
      {
         warningActFailDialog = new AOSGUIWarningDialog(this, "WARNING", TRUE);
         warningActFailDialog->warningMessage->setText(QString("Actuator Failure"));
         warningActFailDialog->warningMessage2->setText(QString(aoi->side.adsec.popmsg));
         warningActFailDialog->exec();
      }
   }
   else
   {
      if (warningActFailDialog != NULL)
      {
         warningActFailDialog->close();
         delete warningActFailDialog;
         warningActFailDialog = NULL;
      }
   }

//********************
//Here is where the AOS code handles events and presents messages to the user.
//Need to decide if we need something similar here.
//**********************

   if (aoi->side.labmode != currentLabmode)
   {
      if (aoi->side.labmode)
      {
         // AOS in lab mode
         
         //pname->setPaletteBackgroundColor(QColor(255,0,0));
         //pname->setPaletteForegroundColor(QColor(255,255,0));
         //pname->setText(QString("  WARNING: Lab mode!  "));
         commandDialog->pname->setPaletteBackgroundColor(QColor(255,0,0));
         commandDialog->pname->setPaletteForegroundColor(QColor(255,255,0));
         commandDialog->pname->setText(QString("  WARNING Lab mode!  "));
         currentLabmode = aoi->side.labmode;
      }
      else
      {
         //pname->setPaletteBackgroundColor(backgroundColor());
         //pname->setPaletteForegroundColor(QColor(0,0,0));
         //pname->setText(QString("AdOpt Subsystem"));
         commandDialog->pname->setPaletteBackgroundColor(backgroundColor());
         commandDialog->pname->setPaletteForegroundColor(QColor(0,0,0));
         commandDialog->pname->setText(QString("AdOpt Subsystem"));
         currentLabmode = aoi->side.labmode;
      }
   }
   
   QString ServerStatus;
   switch(aoi->side.sstat_color)
   {
      case 1:
         ServerStatus = "<font color=#ff0000>";
         break;
      case 2:
         ServerStatus = "<font color=#ffff00>";
         break;
      case 3:
         ServerStatus = "<font color=#64ff64>";
         break;
      case 4:
         ServerStatus = "<font color=#55ffff>";
         break;
      case 5:
      default:
         ServerStatus = "<font color=#ffffff>";
         break;
   }
   ServerStatus +=  string(aoi->side.servstat) + "</font>";
   //operativeStatusBox->setText(ServerStatus);
    
    // Wavefront sensor

   //std::cout << "aoi->side.wfs1.led: " << aoi->side.wfs1.led << std::endl;
    // Wavefront sensor status
   if (aoi->side.wfs1.led == (long)WFS_ON)
   {
      wfs1Led->setPaletteForegroundColor(QColor(0,255,0));
      wfs1Led->setText(QString("On"));
      wfs1OnButton->setEnabled(false);
      wfs1OffButton->setEnabled(TRUE);
      wfs1Health->setPaletteBackgroundColor(QColor(0,255,0));
   }
   else
   {
      wfs1Led->setPaletteForegroundColor(QColor(255,0,0));
      wfs1Led->setText(QString("Off"));
      wfs1OnButton->setEnabled(true);
      wfs1OffButton->setEnabled(false);
      wfs1Health->setPaletteBackgroundColor(QColor(255,0,0));
   }
/*   if (aoi->side.wfs1.health == (long)1)
      wfs1Health->setPaletteBackgroundColor(QColor(0,255,0));
   else
      wfs1Health->setPaletteBackgroundColor(QColor(255,0,0));*/
   

   if(!obsnameText->hasFocus())
   {
      if(magaoic->aoi.cat.obsname == "")
      {
         obsnameText->setPaletteBackgroundColor(QColor(255,0,0));
      }
      else
      {
         obsnameText->setPaletteBackgroundColor(QColor(239,239,239));
      }
      obsnameText->setText(magaoic->aoi.cat.obsname);
   }

   if(!obsinstText->hasFocus())
   {
      if(magaoic->aoi.cat.obsinst == "")
      {
         obsinstText->setPaletteBackgroundColor(QColor(255,0,0));
      }
      else
      {
         obsinstText->setPaletteBackgroundColor(QColor(239,239,239));
      }
      obsinstText->setText(magaoic->aoi.cat.obsinst);
   }


   if(wfs1Enabled)
   {
      // Field viewer
      struct stat fieldImageFileStatus;
      QString fieldImageError;
      QString fieldImageFileName;
      
      // Select file name
      if (imageTypeWfs1 == 1)
         fieldImageFileName = aoi->side.wfs1.tv_filename0;
      else
         fieldImageFileName = aoi->side.wfs1.tv_filename1;
      
      bool toClear;
      bool toDraw;
      
      if (stat(fieldImageFileName, &fieldImageFileStatus)==0) {
         toDraw = true;
         toClear = false;
      }
      else
      {
         toDraw = false;
         toClear = true;
      }
      if (fieldImageFileStatus.st_mtime <= fieldImageFileModTime1)
         toDraw = false;
      else
         if (imageTypeWfs1 == 1) refreshCount1 = 0;
         
      if (imageTypeWfs1 == 1) 
      {
         if (refreshCount1 > 10) 
         {
            toDraw = false;
            toClear = true;
         }
         else refreshCount1++;
      }
         
      if (toDraw)
      {
         // save the modification time of the image
         fieldImageFileModTime1 = fieldImageFileStatus.st_mtime;

         // reset the count of seconds without an image update
         refreshCount1 = 0;
 
         FILE *fd = fopen(fieldImageFileName, "r");
         if (fd == NULL)
         {
            fieldImageError = "CANNOT READ\nIMAGE DATA";
            //wfs1FieldViewerPixmap->setText(fieldImageError);
         }
         else
         {
            // read the binary data into a grayscale image
            //fread (fieldImageData1, IMAGE_X_SIZE * IMAGE_Y_SIZE, 1, fd);
            fclose(fd);

            // create a grayscale image from the binary data
            QImage fieldImage(fieldImageData1, IMAGE_X_SIZE, IMAGE_Y_SIZE, 8, colorMap, 256, QImage::LittleEndian);

            // convert the image color depth to 32 bits to allow the drawing of a yellow mark  box
            QImage tcFieldImage = fieldImage.convertDepth(32);

            // create a pixmap from the image
            QPixmap fieldPixmap(tcFieldImage);

            // load the pixmap into the widget
            //wfs1FieldViewerPixmap->setAutoEraseBkg(false);
            //wfs1FieldViewerPixmap->setPixmap(fieldPixmap);
         }
      }

      if (toClear)
      {
         //wfs1FieldViewerPixmap->setAutoEraseBkg(TRUE);
         if (imageTypeWfs1 == 1)
            fieldImageError = "NO CURRENT IMAGE";
         else
            fieldImageError = "NO ACQUISITION IMAGE";
         //wfs1FieldViewerPixmap->setText(fieldImageError);
      }
         
      // set the orientation of the compass needle
     // wfs1FieldViewerCompass->setValue(aoi->side.wfs1.tv_angle);
         
      // WFS status
      wfs1Status->setText(QString(aoi->side.wfs1.status));
         
      // Integrate the counts and rotates the buffer with the counts data and add the newest value
      if (aoi->side.ao.ao_ready)
      {
         if (integrationCounterWfs1 == INTEGRATION_TIME)
         {
            for (i = 0; i < 60; i++)
               countsBufferWfs1[i] = countsBufferWfs1[i + 1];
            countsBufferWfs1[60] = countSumWfs1 / INTEGRATION_TIME;
            countsCurveWfs1->setData(integrationStepsBuffer, countsBufferWfs1, 61);
            if (countsBufferWfs1[60] > maxScaleWfs1)
            {
               maxScaleWfs1 = (int)countsBufferWfs1[60];
               wfs1CCDCountsPlot->setAxisScale(QwtPlot::yLeft, 0, maxScaleWfs1);
            }
            wfs1CCDCountsPlot->replot();
            integrationCounterWfs1 = 0;
            countSumWfs1 = 0;
            countSumWfs1 += aoi->side.wfs1.counts;
         }
         else
         {
            countSumWfs1 += aoi->side.wfs1.counts;
            integrationCounterWfs1++;
         }
      }
      else if (maxScaleWfs1 != 1)
      {
         maxScaleWfs1 = 1;
         for (i = 0; i < 60; countsBufferWfs1[i++] = 0);
         countsCurveWfs1->setData(integrationStepsBuffer, countsBufferWfs1, 61);
         wfs1CCDCountsPlot->setAxisScale(QwtPlot::yLeft, 0, 100);
         wfs1CCDCountsPlot->replot();
      }
         
      // CCD binning
      QString CCDBinning;
         
      CCDBinning.setNum( (int)aoi->side.wfs1.ccdbin);
      wfs1CCDBinningBox->setText(CCDBinning);
         
      // CCD current frequence value
      QString CCDFrequency;
      CCDFrequency.setNum( (int)(aoi->side.wfs1.ccdfreq));
      wfs1CCDFrequencyBox->setText(CCDFrequency);
   }
   
   // Adaptive Secondary
   if (aoi->side.adsec.led == (long)ADSEC_SET)
   {
      adsecLed->setPaletteForegroundColor(QColor(0,255,0));
      adsecLed->setText(QString("Set"));
      adsecOnButton->setEnabled(false);
      adsecOffButton->setEnabled(true);
      adsecRestButton->setEnabled(true);
      adsecSetButton->setEnabled(false);
   }
   else if (aoi->side.adsec.led == (long)ADSEC_SAFE)
   {
      adsecLed->setPaletteForegroundColor(QColor(255,255,0));
      adsecLed->setText(QString("Safe"));
      adsecOnButton->setEnabled(false);
      adsecOffButton->setEnabled(true);
      adsecRestButton->setEnabled(false);
      adsecSetButton->setEnabled(true);
   }
   else
   {
      adsecLed->setPaletteForegroundColor(QColor(255,0,0));
      adsecLed->setText(QString("Off"));
      adsecOnButton->setEnabled(true);
      adsecOffButton->setEnabled(false);
      adsecRestButton->setEnabled(false);
      adsecSetButton->setEnabled(false);
   }  
    
    adsecShape->setText(QString(aoi->side.adsec.shape));
    adsecStatus->setText(QString(aoi->side.adsec.status));
    
    if (aoi->side.adsec.health == (long)1)
       adsecHealth->setPaletteBackgroundColor(QColor(0,255,0));
    else
       adsecHealth->setPaletteBackgroundColor(QColor(255,0,0));
    
    if (aoi->side.adsec.pwr_status == 0L)
    {
       mainPowerStatus->setPaletteForegroundColor(QColor(255, 255, 0));
       mainPowerStatus->setText(QString("Off"));
    }
    else
    {
       mainPowerStatus->setPaletteForegroundColor(QColor(100, 255, 100));
       mainPowerStatus->setText(QString("On"));
    }
    
    switch(aoi->side.adsec.tss_status)
    {
       case 0:
          tssStatus->setPaletteForegroundColor(QColor(100, 255, 100));
          tssStatus->setText(QString("Disabled"));
       break;
       case 1:
          tssStatus->setPaletteForegroundColor(QColor(100, 255, 100));
          tssStatus->setText(QString("Enabled"));
       break;
       case 2:
          tssStatus->setPaletteForegroundColor(QColor(255, 255, 0));
          tssStatus->setText(QString("Activated"));
          break;
       case 3:
          tssStatus->setPaletteForegroundColor(QColor(255, 0, 0));
          tssStatus->setText(QString("Error"));
          break;
    }
    
    if (aoi->side.adsec.coil_status == 0L)
    {
       coilStatus->setPaletteForegroundColor(QColor(255, 0, 0));
       coilStatus->setText(QString("Disabled"));
    }
    else
    {
       coilStatus->setPaletteForegroundColor(QColor(100, 255, 100));
       coilStatus->setText(QString("Enabled"));
    }
    
    // Leds
    if (aoi->side.adsec.elev_upd)
       adsecElevUpd->setPaletteBackgroundColor(QColor(0,255,0));
    else
       adsecElevUpd->setPaletteBackgroundColor(backgroundColor());
    
    if (aoi->side.adsec.anem_upd)
       adsecAnemUpd->setPaletteBackgroundColor(QColor(0,255,0));
    else
       adsecAnemUpd->setPaletteBackgroundColor(backgroundColor());
    
    // AO
       
   // AO source
   //aoSource->setText(QString(aoi->side.ao.wfs_source));
       
   // AO status
   aoStatus->setText(QString(aoi->side.ao.status));
   
   //Observicn mode
   //aoMode->setText(QString(aoi->side.ao.mode));
   
   // Corrected modes
   QString AdOptCorrectedModes;
   AdOptCorrectedModes.setNum ((int)(aoi->side.ao.correctedmodes));
   aoCorrectedModes->setText(AdOptCorrectedModes);
   
   // Skip frame percentage
   QString AdOptSkipFrame;
   double skf = aoi->side.adsec.safeskip_perc;
   int iskf =(int)(skf*100.0+.5);
   AdOptSkipFrame.setNum(iskf);
   if (iskf < SAFESKIP_PERC_WARN)
      aoSkipFrame->setText("<font color=#64ff64>"+AdOptSkipFrame+"</font>");
   else if (iskf <= SAFESKIP_PERC_ERROR)
      aoSkipFrame->setText("<font color=#ffff00>"+AdOptSkipFrame+"</font>");
   else
      aoSkipFrame->setText("<font color=#ff0000>"+AdOptSkipFrame+"</font>");
   
   // Gains

   double tt, ho1, ho2;
   magaoic->parseLoopGains(tt,ho1,ho2);
   QString AdOptOffload;
//   double iv=aoi->side.adsec.offload[1];
   AdOptOffload.setNum(tt, 'f', 2);
   aoOffloadBox_1->setText(AdOptOffload);
   
   //iv=aoi->side.adsec.offload[2];
   AdOptOffload.setNum(ho1, 'f', 2);
   aoOffloadBox_2->setText(AdOptOffload);
   
   //iv=aoi->side.adsec.offload[3];
   AdOptOffload.setNum(ho2, 'f', 2);
   aoOffloadBox_3->setText(AdOptOffload);
   
   // Leds
   if (aoi->side.ao.sl_ready)
      aoSLReady->setPaletteBackgroundColor(QColor(0,255,0));
   else
      aoSLReady->setPaletteBackgroundColor(backgroundColor());
   
   if (aoi->side.ao.ao_ready)
      aoAOReady->setPaletteBackgroundColor(QColor(0,255,0));
   else
      aoAOReady->setPaletteBackgroundColor(backgroundColor());
   
   if (aoi->side.ao.loopon == 1l)
      aoLoopClosed->setPaletteBackgroundColor(QColor(0,255,0));
   else if (aoi->side.ao.loopon == 2l)
      aoLoopClosed->setPaletteBackgroundColor(QColor(255,255,0));
   else
      aoLoopClosed->setPaletteBackgroundColor(backgroundColor());
   
   // Rotates the buffer with the Strehl data and add the newest value
   if (aoi->side.ao.loopon)
   {
      for (i = 0; i < 60; i++)
         strehlBuffer[i] = strehlBuffer[i + 1];
      strehlBuffer[60] = aoi->side.ao.strehl;
      strehlCurve->setData(secondsBuffer, strehlBuffer, 61);
      //aoStrehlPlot->replot();
   }
   else if (strehlBuffer[0] != 0)
   {
      memset(strehlBuffer, 0, 61 * sizeof(int));
      strehlCurve->setData(secondsBuffer, strehlBuffer, 61);
      //aoStrehlPlot->replot();
   }
   // updates variables and parameters into the Commands dialog
   commandDialog->checkRunningStatus();
   
   if (commandDialog->isVisible())
   {
      commandDialog->updateValues();
   }




   /*******************************************************/
   //Update telescope data

   char vals[512];

   catObjText->setText(magaoic->aoi.cat.obj.c_str());

   snprintf(vals, 512, "%0.2f", magaoic->aoi.el.position);
   //std::cout << vals << "\n";
   elevationText->setText(vals);
   
   snprintf(vals, 512, "%0.2f", magaoic->aoi.am);
   //std::cout << vals << "\n";
   airmassText->setText(vals);

   snprintf(vals, 512, "%0.2f", magaoic->aoi.ha);
   //std::cout << vals << "\n";
   hourangleText->setText(vals);

   snprintf(vals, 512, "%0.2f", magaoic->aoi.rotator.angle);
   //std::cout << vals << "\n";
   rotatorText->setText(vals);

}

void AOIGUIForm::setSide( int cSide )
{
   
   side = (char) cSide;
   
   
   //textLabel_Side->setText("");
   
   setCaption(QString("Magellan AO System"));
   
   commandDialog->setQMagAOIClient( magaoic );
   commandDialog->setSide(side);
   
}

void AOIGUIForm::viewCommand_window()
{
   commandDialog->show();
}


void AOIGUIForm::toggleImageTypeWfs1()
{
/*   if (radioCurrentWfs1->isChecked())
      imageTypeWfs1 = 1;
   else
      imageTypeWfs1 = 2;
   fieldImageFileModTime1 = 0;*/
}

void AOIGUIForm::setFramePalette( QFrame *widget, const QColor & color)
{
   int c;
   QColorGroup colorGroup;
   
   for ( c = 0; c < QColorGroup::NColorRoles; c++ )
      colorGroup.setColor((QColorGroup::ColorRole)c, QColor());
   colorGroup.setColor(QColorGroup::Base, color);
   colorGroup.setColor(QColorGroup::Background, color);
   colorGroup.setColor(QColorGroup::Foreground, color);
   QPalette newPalette = widget->palette();
   for ( c = 0; c < QColorGroup::NColorRoles; c++ )
   {
      if ( colorGroup.color((QColorGroup::ColorRole)c).isValid() )
      {
         for ( int cg = 0; cg < QPalette::NColorGroups; cg++ )
         {
            newPalette.setColor(
               (QPalette::ColorGroup)cg,
                                (QColorGroup::ColorRole)c,
                                colorGroup.color((QColorGroup::ColorRole)c));
         }
      }
   }
   
   for ( int i = 0; i < QPalette::NColorGroups; i++ )
   {
      QPalette::ColorGroup cg = (QPalette::ColorGroup)i;
      
      const QColor light = newPalette.color(cg, QColorGroup::Base).light(170);
      const QColor dark = newPalette.color(cg, QColorGroup::Base).dark(170);
      const QColor mid = widget->frameShadow() == QFrame::Raised ? newPalette.color(cg, QColorGroup::Base).dark(110)  : newPalette.color(cg, QColorGroup::Base).light(110);
      newPalette.setColor(cg, QColorGroup::Dark, dark);
      newPalette.setColor(cg, QColorGroup::Mid, mid);
      newPalette.setColor(cg, QColorGroup::Light, light);
   }
   
   widget->setPalette(newPalette);
}

void AOIGUIForm::wfs1SensorOn()
{
   if(QMessageBox::warning(this, "WFS ON Request",
      "About to turn ON the MagAO WFS. Do you want to continue?",
      "Continue","Cancel","",1) != 0)
   {
      return;
   }
   std::cout << "Turning on\n";
   
   magaoic->WfsOn(AO_WFS_MAGAO);
}


void AOIGUIForm::wfs1SensorOff()
{
   if(QMessageBox::warning(this, "WFS OFF Request",
      "About to switch OFF the MagAO WFS. Do you want to continue?",
      "Continue","Cancel","",1) != 0)
      return;
   magaoic->WfsOff(AO_WFS_MAGAO);
}


void AOIGUIForm::adsecOn()
{
    if(QMessageBox::warning(this, "Adaptive Secondary ON Request",
			    "You are about to turn ON the adaptive secondary.\n\n!!! YOU MUST ENSURE COOLING IS RUNNING !!! \n\nDo you want to continue?",
			    "Continue","Cancel","",1) != 0)
	return;
   
   magaoic->AdsecOn();
}


void AOIGUIForm::adsecOff()
{
    if(QMessageBox::warning(this, "Adaptive Secondary OFF Request",
			    "About to switch OFF the adaptive secondary. Do you want to continue?",
			    "Continue","Cancel","",1) != 0)
	return;
   
   magaoic->AdsecOff();
}


void AOIGUIForm::adsecRest()
{
   magaoic->AdsecRest();
}


void AOIGUIForm::adsecSet()
{
   magaoic->AdsecSet();
}

/*void AOIGUIForm::customEvent( QCustomEvent *event )
{
    AOSAppendEvent *pevent = (AOSAppendEvent *)event;
    if (pevent->type() == AOSGUImsgType)
	messageBox->append(pevent->line());
}*/


void AOIGUIForm::editObsNameInst()
{
   magaoic->updateObservers(obsnameText->text(), obsinstText->text());


}
