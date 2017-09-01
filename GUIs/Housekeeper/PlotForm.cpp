/****************************************************************************
** Form implementation generated from reading ui file '.ui/PlotForm.ui'
**
** Created by User Interface Compiler
**
** WARNING! All changes made in this file will be lost!
****************************************************************************/

#include "PlotForm.h"

#include <qvariant.h>
#include <qwt_plot.h>
#include <qwt_plot_zoomer.h>
#include <qpushbutton.h>
#include <qlayout.h>
#include <qtooltip.h>
#include <qwhatsthis.h>
#include <qimage.h>
#include <qpixmap.h>
#include <qpainter.h>

#include "qwt_plot.h"
#include "qwt_plot_curve.h"
#include "qwt_scale_widget.h"
#include "qwt_text.h"
#include "qwt_text_label.h"
/*
 *  Constructs a PlotForm as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 *  The dialog will by default be modeless, unless you set 'modal' to
 *  TRUE to construct a modal dialog.
 */
PlotForm::PlotForm( QWidget* parent, const char* name, bool modal, WFlags fl )
    : QDialog( parent, name, modal, fl )
{
    if ( !name )
	setName( "PlotForm" );
    setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)7, (QSizePolicy::SizeType)7, 0, 0, sizePolicy().hasHeightForWidth() ) );
    setSizeGripEnabled( TRUE );
    totLayout =new QVBoxLayout (this,11,6,"PlotFormLayout");

    qwtPlot1 = new QwtPlot( this, "qwtPlot1" );
    qwtPlot1->setGeometry( QRect( 0, 10, 500, 220 ) );
    qwtPlot1->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)7, (QSizePolicy::SizeType)7, 0, 0, qwtPlot1->sizePolicy().hasHeightForWidth() ) );
    totLayout->addWidget(qwtPlot1);

    Layout1 = new QHBoxLayout( 0, 0, 6, "Layout1");

    //buttonHelp = new QPushButton( this, "buttonHelp" );
    //buttonHelp->setAutoDefault( TRUE );
    //Layout1->addWidget( buttonHelp );
    //Horizontal_Spacing2 = new QSpacerItem( 20, 20, QSizePolicy::Expanding, QSizePolicy::Minimum );
    //Layout1->addItem( Horizontal_Spacing2 );

    buttonOk = new QPushButton( this, "Stop" );
    buttonOk->setAutoDefault( TRUE );
    buttonOk->setDefault( TRUE );
    buttonOk->setPaletteBackgroundColor ( QColor (255,255,127));
    Layout1->addWidget( buttonOk );

    Horizontal_Spacing2 = new QSpacerItem( 20, 20, QSizePolicy::Expanding, QSizePolicy::Minimum );
    Layout1->addItem( Horizontal_Spacing2 );

    buttonCancel = new QPushButton( this, "buttonCancel" );
    buttonCancel->setAutoDefault( TRUE );
    Layout1->addWidget( buttonCancel );

    totLayout->addLayout(Layout1);

    languageChange();
    resize( QSize(515, 271).expandedTo(minimumSizeHint()) );
    clearWState( WState_Polished );

    // signals and slots connections
    connect( buttonOk, SIGNAL( clicked() ), this, SLOT( StopStart() ) );
    connect( buttonCancel, SIGNAL( clicked() ), this, SLOT( quit() ) );

    Nx=0;
}

/*
 *  Destroys the object and frees any allocated resources
 */
PlotForm::~PlotForm()
{
    // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void PlotForm::languageChange()
{
    setCaption( tr( "Plot" ) );
    //buttonHelp->setText( tr( "&Help" ) );
    //buttonHelp->setAccel( QKeySequence( tr( "F1" ) ) );
    buttonOk->setText( tr( "&Stop Refresh" ) );
    buttonOk->setAccel( QKeySequence( tr( "Alt+S" ) ) );
    buttonCancel->setText( tr( "&Quit" ) );
    buttonCancel->setAccel( QKeySequence( tr( "Alt+Q" ) ) );
}


void PlotForm::StopStart()
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


void PlotForm::plotxy()
{
    char temp[100];
    QPen wapen;
    double x1[2],y1[2];
    double xmin0,xmax0;

    if ( Nx <= 0 )  return;

    qwtPlot1->clear();
    // plot title
    Title.setColor (QColor (200,0,0));
    QFont title_font (Title.font());
    title_font.setFamily("Bitstream Charter");
    title_font.setItalic(TRUE);
    Title.setFont(title_font);
    qwtPlot1->setTitle(Title);
    // axis titles
    xtitle.setColor (QColor (0,0,255));
    ytitle[iy].setColor (QColor (0,0,255));
    qwtPlot1->setAxisTitle(QwtPlot::xBottom, xtitle);
    qwtPlot1->setAxisTitle(QwtPlot::yLeft, ytitle[iy]);
    // axis min-max
    //xmin =  x[0]-x[Nx-1]-10.;
    //xmax =  10.;
    xmin =  x[0]-x[Nx-1];
    xmax =  0.;
    xmin0 = xmin+x[Nx-1];
    xmax0 = xmax+x[Nx-1];
    qwtPlot1->enableAxis(QwtPlot::xTop);
    qwtPlot1->setAxisScale(QwtPlot::xTop,  xmin0,xmax0 );
    qwtPlot1->setAxisScale(QwtPlot::xBottom,  xmin,xmax );
    qwtPlot1->setAxisScale(QwtPlot::yLeft, ymin[iy],ymax[iy]);
    // drownig area background
    qwtPlot1->setCanvasBackground( backgr );

    sprintf (temp,"%d",iy);

    QwtPlotCurve *c = new QwtPlotCurve (temp);
    //for (int i=0; i<Nx; i++) printf ("%d x=%f y=%f\n",i,x[i],y[iy][i]);
    for (int i=0; i<Nx; i++) x[i] -= x[Nx-1];
    c->setData( x, y[iy], Nx );
    c->setPen( pen[iy] );
    c->attach(qwtPlot1);


    //warning & Alarm min&max lines
    x1[0]=x[0];
    x1[1]=x[Nx-1];

    wapen.setColor ( QColor (255,0,0));
    y1[0]=amin[iy];
    y1[1]=amin[iy];
    QwtPlotCurve *ca1 = new QwtPlotCurve ("amin");
    ca1->setData( x1, y1, 2 );
    ca1->setPen(wapen);
    ca1->attach(qwtPlot1);
    y1[0]=amax[iy];
    y1[1]=amax[iy];
    QwtPlotCurve *ca2 = new QwtPlotCurve ("amax");
    ca2->setData( x1, y1, 2 );
    ca2->setPen(wapen);
    ca2->attach(qwtPlot1);

    wapen.setColor ( QColor (200,200,0));
    y1[0]=wmin[iy];
    y1[1]=wmin[iy];
    QwtPlotCurve *cw1 = new QwtPlotCurve ("wmin");
    cw1->setData( x1, y1, 2 );
    cw1->setPen(wapen);
    cw1->attach(qwtPlot1);
    y1[0]=wmax[iy];
    y1[1]=wmax[iy];
    QwtPlotCurve *cw2 = new QwtPlotCurve ("wmax");
    cw2->setData( x1, y1, 2 );
    cw2->setPen(wapen);
    cw2->attach(qwtPlot1);

    //draw mean&rms values
    /*
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
    qwtZoom = new QwtPlotZoomer ( QwtPlot::xBottom,QwtPlot::yLeft, qwtPlot1->canvas() );
}


void PlotForm::quit()
{
 emit removeMyPlot (inVector);
 close();
}
