/****************************************************************************
** plot modes data
****************************************************************************/

#include "PlotModes.h"

#include <qvariant.h>
#include <qwt_plot.h>
#include <qpushbutton.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qtooltip.h>
#include <qwhatsthis.h>
#include <qimage.h>
#include <qpixmap.h>
#include <qpainter.h>
#include <qcheckbox.h>
#include <qwt_scale_engine.h>
#include <qwt_plot_zoomer.h>

#include "qwt_plot.h"
#include "qwt_plot_curve.h"
#include "qwt_scale_widget.h"
#include "qwt_text.h"
#include "qwt_text_label.h"

//int   ModesBand[14] ={9,15,21,27,33,39,45,51,57,63,69,75,81,87};
//int Red[14]={200,0,  200,10, 254,50, 150,0,  0,  200,0,  250,200,250};
//int Green[14]={0,200,180,255,0,  50, 250,100,250,0,  200,100,200,40};
//int Blue[14]={0, 200,0,  255,100,250,150,250,0,  200,200,0,  0,  40};
int NModesBand=7;
int   ModesBand[7] ={100,100,100,100,100,100,72};
int Red[7]={200,0,  200,10, 254, 0, 150};
int Green[7]={0,200,180,255,0, 200, 250};
int Blue[7]={0, 200,0,  255,100,250,150};
/*
 *  Constructs a PlotModes as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 *  The dialog will by default be modeless, unless you set 'modal' to
 *  TRUE to construct a modal dialog.
 */
PlotModes::PlotModes( QWidget* parent, const char* name, bool modal, WFlags fl )
    : QDialog( parent, name, modal, fl )
{
    if ( !name )
	setName( "PlotModes" );
    setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)7, (QSizePolicy::SizeType)7, 0, 0, sizePolicy().hasHeightForWidth() ) );
    setSizeGripEnabled( TRUE );
    totLayout =new QVBoxLayout (this,11,6,"PlotModesLayout");

    qwtPlot1 = new QwtPlot( this, "qwtPlot1" );
    qwtPlot1->setGeometry( QRect( 0, 10, 500, 220 ) );
    qwtPlot1->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)7, (QSizePolicy::SizeType)7, 0, 0, qwtPlot1->sizePolicy().hasHeightForWidth() ) );
    QWhatsThis::add(qwtPlot1->canvas(),"use left mouse click&drag to zoom, middle to unzoom\n");
    totLayout->addWidget(qwtPlot1);

    Layout1 = new QHBoxLayout( 0, 0, 6, "Layout1");

    for (int im=0; im<NModesBand; im++)  {
        c[im] = new QwtPlotCurve ("0");
        c[im]->attach(qwtPlot1);
    }

    //buttonHelp = new QPushButton( this, "buttonHelp" );
    //buttonHelp->setAutoDefault( TRUE );
    //Layout1->addWidget( buttonHelp );
    //Horizontal_Spacing2 = new QSpacerItem( 20, 20, QSizePolicy::Expanding, QSizePolicy::Minimum );
    //Layout1->addItem( Horizontal_Spacing2 );

    buttonOk = new QPushButton( this, "Stop" );
    buttonOk->setAutoDefault( TRUE );
    buttonOk->setDefault( TRUE );
    buttonOk->setPaletteBackgroundColor ( QColor (255,255,127));
    QWhatsThis::add(buttonOk,"use to Stop/Start refreshing\n");
    Layout1->addWidget( buttonOk );
    TimeR = new QLabel (this,"TimeLabel");
    QFont TimeRFont(TimeR->font());
    TimeRFont.setPointSize(8);
    TimeR->setFont( TimeRFont );
    Layout1->addWidget( TimeR );

    Horizontal_Spacing2 = new QSpacerItem( 20, 20, QSizePolicy::Expanding, QSizePolicy::Minimum );
    Layout1->addItem( Horizontal_Spacing2 );

    checkBox1 = new QCheckBox( this, "checkBox1" );
    QWhatsThis::add(checkBox1,"use to change x scale between linear&log\n");
    Layout1->addWidget( checkBox1 );

    checkBox2 = new QCheckBox( this, "checkBox2" );
    QWhatsThis::add(checkBox2,"use to change y scale between linear&log\n");
    Layout1->addWidget( checkBox2 );

    buttonCancel = new QPushButton( this, "buttonCancel" );
    buttonCancel->setAutoDefault( TRUE );
    Layout1->addWidget( buttonCancel );

    totLayout->addLayout(Layout1);

    //initialization

    qwtPlot1->setAxisScaleEngine(QwtPlot::xBottom, new QwtLinearScaleEngine () );
    for (int i=0; i<NX; i++) { x[i]=(double)i; }
    xmin = 0;
    xmax = NX-1;
    for (int i=0; i<NX; i++) {
        x[i]=(double)i;
    }
    qwtPlot1->setAxisScale(QwtPlot::xBottom,  xmin,xmax );
    //xmin=-5.;
    //xmax=(double)(NX+5);
    //xtitle=(QwtText)"Actuator";
    ytitle=(QwtText)"Amplitude";
    ylim0 = 0.;
    ylog = false;


    languageChange();
    resize( QSize(515, 271).expandedTo(minimumSizeHint()) );
    clearWState( WState_Polished );

    // signals and slots connections
    connect( buttonOk, SIGNAL( clicked() ), this, SLOT( StopStart() ) );
    connect( buttonCancel, SIGNAL( clicked() ), this, SLOT( quit() ) );
    connect( checkBox1, SIGNAL( toggled(bool) ), this, SLOT( xlog_toggled(bool) ) );
    connect( checkBox2, SIGNAL( toggled(bool) ), this, SLOT( ylog_toggled(bool) ) );


}

/*
 *  Destroys the object and frees any allocated resources
 */
PlotModes::~PlotModes()
{
    // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void PlotModes::languageChange()
{
    setCaption( tr( "Plot Modes" ) );
    //buttonHelp->setText( tr( "&Help" ) );
    //buttonHelp->setAccel( QKeySequence( tr( "F1" ) ) );
    buttonOk->setText( tr( "&Stop Refresh" ) );
    buttonOk->setAccel( QKeySequence( tr( "Alt+S" ) ) );
    buttonCancel->setText( tr( "&Quit" ) );
    buttonCancel->setAccel( QKeySequence( tr( "Alt+Q" ) ) );
    checkBox1->setText( tr( "xlog" ) );
    checkBox2->setText( tr( "ylog" ) );

}

void PlotModes::xlog_toggled(bool tmp)
{
   if (tmp == true) {
      qwtPlot1->setAxisScaleEngine(QwtPlot::xBottom, new QwtLog10ScaleEngine () );
      //for (int i=1; i<NX+1; i++) { x[i]=(double)i; }
      xmin = 1;
      xmax = NX;
   } else {
      qwtPlot1->setAxisScaleEngine(QwtPlot::xBottom, new QwtLinearScaleEngine () );
      //for (int i=0; i<NX; i++) { x[i]=(double)i; }
      xmin = 0;
      xmax = NX-1;
   }
   qwtPlot1->setAxisScale(QwtPlot::xBottom,  xmin,xmax );
   qwtPlot1->replot();
}

void PlotModes::ylog_toggled(bool tmp) 
{
   double ymin1,ymax1;

   if (tmp == true) {
      for (int i=0; i<NX; i++) { ytmp[i]=y[i]; }
      for (int i=0; i<NX; i++) { y[i]=fabs(y[i])+1e-9; }
      qwtPlot1->setAxisScaleEngine(QwtPlot::yLeft, new QwtLog10ScaleEngine () );
      ymin1 = 1e-9;
      ymax1 = ylim;
      ylog = true;

      
   } else {
      qwtPlot1->setAxisScaleEngine(QwtPlot::yLeft, new QwtLinearScaleEngine () );
      for (int i=0; i<NX; i++) { y[i]=ytmp[i]; }
      ymin1 = -ylim;
      ymax1 = ylim;
      ylog = false;
   }
   qwtPlot1->setAxisScale(QwtPlot::yLeft,  ymin1,ymax1 );
   qwtPlot1->replot();
}


void PlotModes::StopStart()
{
   if (QString::compare(buttonOk->text(),"&Stop Refresh") == 0 ) {
      Refresh = FALSE;
      buttonOk->setText( tr ("&Start Refresh"));
      buttonOk->setPaletteBackgroundColor ( QColor (200,0,0));
   }  else {
      Refresh = TRUE;
      buttonOk->setText( tr ("&Stop Refresh"));
      buttonOk->setPaletteBackgroundColor ( QColor (255,255,127));
   } 
}


void PlotModes::plotxy()
{
    char temp[100];
    QPen wapen;
    int im,im0;
    double yymin,yymax;
    double ytemp;
    QwtText title;

    if ( NX <= 0 )  return;

    // axis titles
    xtitle.setColor (QColor (0,0,255));
    ytitle.setColor (QColor (0,0,255));
    qwtPlot1->setAxisTitle(QwtPlot::xBottom, xtitle);
    //qwtPlot1->setAxisScale(QwtPlot::xBottom,  xmin,xmax );
    qwtPlot1->setAxisTitle(QwtPlot::yLeft, ytitle);
    //qwtPlot1->setAxisScale(QwtPlot::yLeft, ymin,ymax);
    // axis min-max
    //qwtPlot1->enableAxis(QwtPlot::xTop);
    //qwtPlot1->setAxisScale(QwtPlot::xTop,  xmin0,xmax0 );
    // drownig area background
        
    qwtPlot1->setCanvasBackground( QColor(255,255,255) );

    if (ylog == true) {
       
      for (int i=0; i<NX; i++) {
          ytemp=y[i]; 
          ytmp[i]=ytemp;
          y[i]=fabs(ytemp)+1e-9;
      }

    }


    ymin=1.e20;
    ymax=0.;
    im0=0;
    for (im=0; im<NModesBand; im++) {
        c[im]->setData( &x[im0], &y[im0], ModesBand[im] );
        c[im]->setStyle((QwtPlotCurve::CurveStyle) 2);
        pen.setColor ( QColor (Red[im],Green[im],Blue[im]));
        c[im]->setPen( pen );
        c[im]->attach(qwtPlot1);
        yymin = c[im]->minYValue();
        if (ymin>=yymin) ymin=yymin;
        yymax = c[im]->maxYValue();
        if (ymax<=yymax) ymax=yymax;
        im0 += ModesBand[im];
    }
    //printf("ymax=%f ymin=%f \n",ymax,ymin);
    ylim = fabs(ymax);
    double yyy= fabs(ymin);
    if (yyy > ylim )  ylim =yyy;
    ylim=1.1*ylim;
    if (ylim>=ylim0)  ylim0=ylim;
    //printf("ylim=%f ylim0=%f \n",ylim,ylim0);

    // plot title
    sprintf(temp,"%s      min=%4.2e  max=%4.2e",Title,ymin,ymax);
    title =(QwtText)temp;
    title.setColor (QColor (200,0,0));
    QFont title_font (title.font());
    title_font.setFamily("Bitstream Charter");
    title_font.setItalic(TRUE);
    title.setFont(title_font);
    qwtPlot1->setTitle(title);

    if (ymax==0.) ymax=1.;
    if (type<2) qwtPlot1->setAxisScale(QwtPlot::yLeft, -ylim0,ylim0);
    else        qwtPlot1->setAxisScale(QwtPlot::yLeft, 0.,ymax);
    //draw mean&rms values
    /*


    wapen.setColor ( QColor (200,200,0));
    cw1->setPen(wapen);


    int xmean =qwtPlot1->transform(QwtPlot::xBottom, 5.);
    int ymean =qwtPlot1->transform(QwtPlot::yLeft, mean[iy]);
    sprintf (temp,"mean = %f +- %f",(float)mean[iy],(float)rms[iy]);
    QwtText MeanLabel( (QString) temp );
    QwtTextLabel meanLabel(MeanLabel,qwtPlot1 );

    //QPainter paint (qwtPlot1);
    QPainter* paint = new QPainter(qwtPlot1);
    paint->setFont(title_font);
    paint->setPen( pen[iy] );
    //paint.setFont(title_font);
    //paint.setPen( pen[iy] );
    QRect r(xmean,ymean, 100,10);
    meanLabel.draw (paint,r);
    //paint.drawText( xmean,ymean, (QString) temp );
    //QPainter* paint = new QPainter(this);
    ////qwtPlot1->QwtPlainTextEngine::draw(paint,r,Qt::AlignCenter,(QString) temp );
    //QwtPlainTextEngine::draw(paint,r,Qt::AlignCenter,(QString) temp );
      */

    qwtPlot1->replot();

    qwtZoom = new QwtPlotZoomer(QwtPlot::xBottom,QwtPlot::yLeft, qwtPlot1->canvas() );

    TimeR->setPaletteForegroundColor ( timeCol );
    TimeR->setText(TimeRefr);
}


void PlotModes::quit()
{
 emit removeMyPlot (inVector);
 close();
}
