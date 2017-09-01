
/****************************************************************************
** 
**
** 
**
** 
****************************************************************************/

extern "C"{
	#include <stdlib.h>
	#include <math.h>
	#include <time.h>
}


#include "Mirror.h"

#include <qpainter.h>


#include <qvariant.h>
//#include <kcolordialog.h>
//#include <kcolorbutton.h>
#include <qslider.h>
#include <qscrollbar.h>
//#include <knuminput.h>
//#include <kselect.h>
#include <qlayout.h>
#include <qtooltip.h>
#include <qmessagebox.h>
#include <qwhatsthis.h>
//#include "kcolordialog.h"
//#include "kcolorbutton.h"
//#include "knuminput.h"
//#include "kselect.h"


#include <qlabel.h>
#include <qlineedit.h>
#include <qpushbutton.h>
#include <qpopupmenu.h>
#include <qcolordialog.h>


int   mirring[14] ={9,15,21,27,33,39,45,51,57,63,69,75,81,87};
double a0ring[14] ={
   0.698132,
   0.837758,
   0.897598,
   0.930843,
   0.951998,
   0.966644,
   0.977384,
   0.985597,
   0.992082,
   0.997331,
    1.00167,
    1.00531,
    1.00841,
    1.01109
};
/*float angring[14] ={
1.2615,
2.1498,
3.0382,
3.9266,
4.8150,
5.7034,
6.5918,
7.4802,
8.3686,
9.2570,
10.1454,
11.0338,
11.9222,
12.8105 };*/

float angring[14] ={
0.872664,
0.733038,
0.673198,
0.639954,
0.618799,
0.604152,
0.593412,
0.585199,
0.578714,
0.573465,
0.569129,
0.565487,
0.562384,
0.559709
};

//===========================================================================
// Definition of the mirror parameters for the LBT672a unit.
/*
mag_mass           float    2.8 ;[g] mass of one magnet+glass_puck+Al_ring (0.12 extra mass aluminum ring wrt spheric shell)
glass_mass         float   2318.4 ;[g] mass of the glass (thin mirror). 4200g-672*mag_mass, 4200g: estimated by ADS
membrane_ring_mass float      0.0 ;[g] mass of the outer ring of the central membrane
curv_radius        float   1955.0 ;[mm] back radius of curvature (RoC) of thin shell
n_rings            int         14 ; # of actuator rings (AR)
in_radius          float     27.5 ;[mm] inner radius of the mirror
out_radius         float    455.5 ;[mm] outer radius of the mirror
vertex_thickness   float      1.6 ;[mm] thickness at the vertex
n_act_ring         array          ;number of actuators per ring
    int 14
9
15
21
27
33
39
45
51
57
63
69
75
81
87
end

angle_ring         array          ;[deg] angle between AO and AR with vertex in CoC
    float 14
1.2615
2.1498
3.0382
3.9266
4.8150
5.7034
6.5918
7.4802
8.3686
9.2570
10.1454
11.0338
11.9222
12.8105
end

angle_act0         array          ;[deg] azimuthal angle of the first act
    float 14
0
0
0
0
0
0
0
0
0
0
0
0
0
0
end

in_cap_radius      float   8.0                ;[mm] inner radius of capacitive sensor area
out_cap_radius     float   12.5               ;[mm] outer radius of capacitive sensor area
*/
//==================================== End of parameters =======================================

int miract2act[672]={
104,216,217,328,440,441,552,664,665,105,106,218,219,220,329,330,442,443,444,553,554,666,667,668,107,108,109,221,
222,223,208,331,332,333,445,446,447,432,555,556,557,669,670,671,656,110,111,96,97,98,209,210,211,212,334,335,
320,321,322,433,434,435,436,558,559,544,545,546,657,658,659,660,88,99,100,101,102,103,200,201,213,214,215,312,
323,324,325,326,327,424,425,437,438,439,536,547,548,549,550,551,648,649,661,662,663,89,90,91,92,93,94,95,
202,203,204,205,206,207,313,314,315,316,317,318,319,426,427,428,429,430,431,537,538,539,540,541,542,543,650,651,
652,653,654,655,72,73,74,75,80,81,82,83,192,193,194,184,185,186,187,296,297,298,299,304,305,306,307,416,
417,418,408,409,410,411,520,521,522,523,528,529,530,531,640,641,642,632,633,634,635,64,76,77,78,79,84,85,
86,87,195,196,197,188,189,190,191,176,288,300,301,302,303,308,309,310,311,419,420,421,412,413,414,415,400,512,
524,525,526,527,532,533,534,535,643,644,645,636,637,638,639,624,65,66,67,68,69,70,48,49,50,51,160,198,
199,177,178,179,180,181,182,289,290,291,292,293,294,272,273,274,275,384,422,423,401,402,403,404,405,406,513,514,
515,516,517,518,496,497,498,499,608,646,647,625,626,627,628,629,630,71,56,57,58,59,60,61,52,53,54,55,
161,162,163,168,169,170,171,172,173,183,295,280,281,282,283,284,285,276,277,278,279,385,386,387,392,393,394,395,
396,397,407,519,504,505,506,507,508,509,500,501,502,503,609,610,611,616,617,618,619,620,621,631,8,9,10,11,
24,62,63,40,41,42,43,152,164,165,166,167,174,175,136,120,121,122,123,232,233,234,235,248,286,287,264,265,
266,267,376,388,389,390,391,398,399,360,344,345,346,347,456,457,458,459,472,510,511,488,489,490,491,600,612,613,
614,615,622,623,584,568,569,570,571,12,13,14,15,25,26,27,28,44,45,46,47,153,154,155,156,157,137,138,
139,140,124,125,126,127,236,237,238,239,249,250,251,252,268,269,270,271,377,378,379,380,381,361,362,363,364,348,
349,350,351,460,461,462,463,473,474,475,476,492,493,494,495,601,602,603,604,605,585,586,587,588,572,573,574,575,
0,1,2,3,29,16,17,18,30,32,33,34,35,158,144,145,146,159,141,128,129,130,142,112,113,114,115,224,
225,226,227,253,240,241,242,254,256,257,258,259,382,368,369,370,383,365,352,353,354,366,336,337,338,339,448,449,
450,451,477,464,465,466,478,480,481,482,483,606,592,593,594,607,589,576,577,578,590,560,561,562,563,4,5,6,
7,31,19,20,21,22,23,36,37,38,39,147,148,149,150,151,131,132,133,134,135,143,116,117,118,119,228,229,
230,231,255,243,244,245,246,247,260,261,262,263,371,372,373,374,375,355,356,357,358,359,367,340,341,342,343,452,
453,454,455,479,467,468,469,470,471,484,485,486,487,595,596,597,598,599,579,580,581,582,583,591,564,565,566
};




char* Name[13] = {
(char *)"Position, um",
(char *)"Position\nRMS, nm",
(char *)"Current, N",
(char *)"Current\nRms, N",
(char *)"Current\nInt Control, N",
(char *)"Current\nFF Pure, N",
(char *)"DSP Bias\n Current, N",
(char *)"DSP Fmol, N",
(char *)"Command\nNew Delta, um",
(char *)"Command\nFF, um",
(char *)"DSP Bias\n Command, um",
(char *)"DSP Cmol, um",
(char *)"Modes"
};
char* NameHelp[13] = {
(char *)"help for Position, um",
(char *)"help for Position\nRMS, nm",
(char *)"help for Current, N",
(char *)"help for Current\nRms, N",
(char *)"help for Current\nInt Control, N",
(char *)"help for Current\nFF Pure, N",
(char *)"help for DSP Bias\n Current, N",
(char *)"help for DSP Fmol, N",
(char *)"help for Command\nNew Delta, um",
(char *)"help for Command\nFF, um",
(char *)"help for DSP Bias\n Command, um",
(char *)"help for DSP Cmol, um",
(char *)"help for Modes"  //this really is not used
};

char* Map[5] = {
(char *)"Linear Map",
(char *)"Square Map",
(char *)"Sqrt   Map",
(char *)"Exp    Map",
(char *)"Ln     Map"
};

char* CTable[3] = {
(char *)"temperature",
(char *)"16 colors",
(char *)"user defined"
};



/*
 *  Constructs a Mirror widget as a child of 'parent', with the
 *  name 'name'.
 */
Mirror::Mirror(QWidget* parent, const char* name)
  : QFrame( parent, name )
//  : QWidget( parent, name )
{

//    if ( !name ) 	setName( "Mirror" );
//    setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)7, (QSizePolicy::SizeType)7, 0, 0, sizePolicy().hasHeightForWidth() ) );
//    setSizeGripEnabled( TRUE );
//    GUIadSecMLayout = new QVBoxLayout( this, 11, 6, "GUIadSecMLayout");

    QWhatsThis::add(this,"panel to show values for each actuator as colored poligons\nuse left mouse double-click on actuator to see values\n");

    MTitleButton = new QPushButton (this,"titleButton");  // button to choose value to show
    QWhatsThis::add(MTitleButton,"choose from the list the value to show in this panel\n");
    MTitleX=90;
    MTitleY=47;
    MTitleButton->setGeometry( QRect( 2, 0, MTitleX, MTitleY ) );
    MTitleButton->setPaletteBackgroundColor( QColor( 200, 200, 200 ) );
    QFont TitleFont(MTitleButton->font());
    TitleFont.setFamily("Bitstream Charter");
    TitleFont.setPointSize(9);
    TitleFont.setBold(TRUE);
    TitleFont.setItalic(TRUE);
    MTitleButton->setFont( TitleFont );

    MTitle = new QLabel (this,"titleLabel");  // Name of the value shown + min&max of it
    QWhatsThis::add(MTitle,"current min&max of the chosen value\n");
    QFont MTitleFont(MTitle->font());
    //MTitleFont.setFamily("Bitstream Charter");
    MTitleFont.setPointSize(8);
    MTitle->setFont( MTitleFont );
    MTitle->setGeometry( QRect( 92, 0, 180, 30 ) );

    Default = new QPushButton (this,"default");  // button to set default limits
    QWhatsThis::add(Default,"restores the default  value limits to show in panel\nto  min&max or min&max-offset if on\n");
    Default->setGeometry( QRect( 95, 32, 175, 15 ) );
    Default->setText( "optimize limits" );

    int ypos =MTitleY+2;
    From = new QLabel (this,"fromLabel");  // legend for from(min) limit
    QWhatsThis::add(From,"MIN value to show\nchoose in window at the right  MIN value to show & press RETURN\n");
    From->setGeometry( QRect( 2, ypos, 30, 15 ) );
    From->setText("from");
    FromLine = new QLineEdit (this,"fromLine");  // text input  for from(min) limit
    FromLine->setGeometry( QRect( 32, ypos, 80, 15 ) );

    ScrollBarFrom = new QScrollBar( this, "scrollBarFrom" );
    QWhatsThis::add(ScrollBarFrom,"choose here MIN value to show using slider\n");
    ScrollBarFrom->setGeometry( QRect( 115, ypos, 155, 15 ) );
    ScrollBarFrom->setPaletteBackgroundColor( QColor( 255, 170, 255 ) );
    ScrollBarFrom->setOrientation( QScrollBar::Horizontal );

    ypos +=17;
    To = new QLabel (this,"toLabel");  // legend for to(max) limit
    QWhatsThis::add(To,"MAX value to show\nchoose in window at the right  MAX value to show & press RETURN\n");
    To->setGeometry( QRect( 2, ypos, 30, 15 ) );
    To->setText("to  ");
    ToLine = new QLineEdit (this,"toLine");  // text input for to(max) limit
    QWhatsThis::add(FromLine,"chos here MAX value to show & press RETURN\n");
    ToLine->setGeometry( QRect( 32, ypos, 80, 15 ) );
    ScrollBarTo = new QScrollBar( this, "scrollBarTo" );
    QWhatsThis::add(ScrollBarTo,"choose here MAX value to show using slider\n");
    ScrollBarTo->setGeometry( QRect( 115, ypos, 155, 15 ) );
    ScrollBarTo->setPaletteBackgroundColor( QColor( 255, 170, 255 ) );
    ScrollBarTo->setOrientation( QScrollBar::Horizontal );


    TimeL = new QLabel (this,"TimeLabel");
    QWhatsThis::add(TimeL,"time when offset has been taken\n");
    TimeL->setGeometry( QRect( 2, 330, 50, 10 ) );
    QFont TimeLFont(TimeL->font());
    TimeLFont.setPointSize(8);
    TimeL->setFont( TimeLFont );
    TakeOffset = new QPushButton (this,"takeOffset");  // button to take offset pattern
    QWhatsThis::add(TakeOffset,"use to take Offset ONLY for THIS panel\n");
    TakeOffset->setGeometry( QRect( 2, 345, 80, 18 ) );
    TakeOffset->setText("Take Offset");
    OffsetOnOff = new QPushButton (this,"offsetOnOff");  // button to set  offset On/Off
    QWhatsThis::add(OffsetOnOff,"use to show value with/without Offset\n");
    OffsetOnOff->setGeometry( QRect( 90, 345, 80, 18 ) );
    OffsetOnOff->setText( "Offset OFF" );
    ColorTable = new QPushButton (this,"colorTable");  // button to changet  color table
    QWhatsThis::add(ColorTable,"change color table for data-to-color mapping\n");
    ColorTable->setGeometry( QRect( 180, 345, 80, 18 ) );
    ColorTable->setText( CTable[0] );

/*
    ReflectH = new QPushButton (this,"reflectH");  // button to make left/right reflection
    ReflectH->setGeometry( QRect( 2, 300, 65, 18 ) );
    ReflectH->setText( "Reflect<>" );
    ReflectH->setPaletteBackgroundColor( QColor( 255, 255, 60 ) );
    ReflectH->setPaletteForegroundColor( QColor( 0, 0, 255 ) );
*/

    Mapping = new QPushButton (this,"mapping");  // button to choose data/color mapping
    QWhatsThis::add(Mapping,"change data-to-color mapping algorithm\n pop-ups the available list\n");
    Mapping->setGeometry( QRect( 2, 365, 80, 18 ) );
    Mapping->setText( "Linear Map" );

    MinColor = new QPushButton (this,"minColor");  // button to choose color of Min for  data/color mapping
    QWhatsThis::add(MinColor,"choose MIN color index for data-to-color mapping\n");
    MinColor->setGeometry( QRect( 90, 365, 80, 18 ) );
    MinColor->setText( "from Color" );

    MaxColor = new QPushButton (this,"maxColor");  // button to choose color of Max for  data/color mapping
    QWhatsThis::add(MaxColor,"choose MAX color index for data-to-color mapping\n");
    MaxColor->setGeometry( QRect( 180, 365, 80, 18 ) );
    MaxColor->setText( "to Color" );

//    languageChange();


    // signals and slots connections


      connect( FromLine, SIGNAL( returnPressed() ), this, SLOT( fromLChanged() ) );

      connect( ToLine, SIGNAL( returnPressed() ), this, SLOT( toLChanged() ) );

      connect( ScrollBarFrom, SIGNAL( valueChanged(int) ), this, SLOT( fromSChanged(int) ) );
      connect( ScrollBarTo, SIGNAL( valueChanged(int) ), this, SLOT( toSChanged(int) ) );

      connect( TakeOffset, SIGNAL( clicked() ), this, SLOT( takeOffset() ) );
      connect( OffsetOnOff, SIGNAL( clicked() ), this, SLOT( setOffset() ) );

      connect( Default, SIGNAL( clicked() ), this, SLOT( setDefaultLim() ) );
      connect( ColorTable, SIGNAL( clicked() ), this, SLOT( changeColorTable() ) );

      connect( Mapping, SIGNAL( clicked() ), this, SLOT( changeMap() ) );
      connect( MinColor, SIGNAL( clicked() ), this, SLOT( changeMinColor() ) );
      connect( MaxColor, SIGNAL( clicked() ), this, SLOT( changeMaxColor() ) );


      connect( MTitleButton, SIGNAL( clicked() ), this, SLOT( changeMTitle() ) );
}

/*
 *  Destroys the object and frees any allocated resources
 */
Mirror::~Mirror()
{
    // no need to delete child widgets, Qt does it all for us
}
/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void Mirror::languageChange()
{
    //kColorButton1->setText( QString::null );
    /*
    kGradientSelector1->setFirstText( tr( "text1" ) );
    kGradientSelector1->setSecondText( tr( "text2" ) );
    QToolTip::add( kGradientSelector1, QString::null );
    */
}


void Mirror::paintEvent(QPaintEvent *)
{
    int ir,im;
    QPainter pnt(this);

    for (ir=0; ir<nrings; ir++) {
        int nmir = r0[ir].nmir;
        for (im=0; im<nmir; im++) {
            pnt.setPen( r0[ir].mir[im].pen_c[ival] );
            pnt.setBrush( r0[ir].mir[im].br_c[ival] );
            pnt.drawPolygon( r0[ir].mir[im].p );
        }
    }
}

void Mirror::updateM()
{
//    printf("updateM: init=%d ival=%d id=%d dmin=%f dmax=%f C=%d\n",Init[ival],ival,id,dmin[ival],dmax[ival],UpdateMC);
//    printf("       : smin=%f smax=%f sdefmin=%f sdefmax=%f\n",smin[ival],smax[ival],sdefmin[ival],sdefmax[ival]);
//    if (ival==0)
//    printf("updateM: ival=%d smin=%f smax=%f\n",ival,smin[ival],smax[ival]);
//    if (smin[0]!=0. && ival==0)
//    printf("updateM1: ival=%d smin=%f smax=%f\n",ival,smin[ival],smax[ival]);
    if (UpdateMC==1) return;
    UpdateMC=1;
    mtitle();
    if (Init[ival]==0 || (smin[ival]==0.&&smax[ival]==0.)) {
       setMinMax();
       Init[ival]=1;
    }
    setMLimits();
    setColorScale();
    setMBrush(ival);
    //updateRings(0,14);   // for the case when some rotation, reflection, etc needed
    bool erase=FALSE;
    repaint(xM0,yM0,widthM,hightM, erase);
    UpdateMC=0;
   // printf("updateM2: ival=%d smin=%f smax=%f\n",ival,smin[ival],smax[ival]);
}


void Mirror::mouseDoubleClickEvent(QMouseEvent * e)
{
  int ir,im;
  char temp[100];

  int x = e->x() - xc;
  int y = e->y() - yc;
  int r = (int)sqrt(x*x+y*y);
  double angle = atan2((double)y,(double)x);
  if (angle > 0.) angle -= PI*2.;
  //printf("angle=%f\n",angle);

    for (ir=0; ir<nrings; ir++) {
        int rn =r0[ir].radiusN;
        int rx =r0[ir].radiusX;
        if (r>=rn && r<=rx) goto radiusok;
    }
    return;
radiusok:
    int nmir = r0[ir].nmir;
    for (im=0; im<nmir; im++) { 
        double a0 = r0[ir].mir[im].angle0;
        double a1 = r0[ir].mir[im].angle1;
        if (angle>=a0 && angle<=a1) goto ok;
        if (a1<a0) {
           if (angle<=a1 && angle<=0.) goto ok;
           if (angle>=a0 && angle<=0.) goto ok;
        }
    }

//    printf("== ival=%d id=%d x=%d y=%d r=%d angle=%f ir=%d im=%d\n",ival,id,x,y,r,angle,ir,im);
     return;
ok:
     QPopupMenu *SMirInfo = new QPopupMenu (this,"info");
     int itot=im;
     for (int ir1=0; ir1<ir; ir1++)    itot+=mirring[ir1];
     int iDSP=miract2act[itot];
     sprintf (temp,"ACTUATOR number %d in the ring %d  DSP=%d\n",im,ir+1,iDSP);
     SMirInfo->insertItem(temp,0,0,0);
     //sprintf (temp,"a0=%f a1=%f",r0[ir].mir[im].angle0,r0[ir].mir[im].angle1);
     //SMirInfo->insertItem(temp,0,1,1);
     if (r0[ir].mir[im].bad[ival]==0) {
        //sprintf (temp,"ival=%d id=%d\n",ival,id);
        for (int iv=0;iv<NVAL;iv++) {
        //int iv=ival;
          if (usedVal[iv]==1) {
            sprintf (temp,"%s: value=%12.8f offset=%12.8f",Name[iv],r0[ir].mir[im].data[iv],r0[ir].mir[im].offset[iv]);
            SMirInfo->insertItem(temp,0,1+iv*1,1+iv*1);
          }
        }
     } else {
        SMirInfo->insertItem("bad actuator",0,1,1);
     }
     SMirInfo->exec(QCursor::pos());
}


void Mirror::setColorScale()
{
    xx[ival] = smax[ival] - smin[ival];
    if (xx[ival] == 0. && smin[ival] != 0. ) xx[ival] = .01*smin[ival];
    if (xx[ival] == 0. && smin[ival] == 0. ) xx[ival] = .01;
}

void Mirror::setMLimits()     
{    // here we define all limits in the panel using current smin&smax

 /*
 if (ival==0){
    printf(" : smin=%e smax=%e sdefmin=%e sdefmax=%e\n",smin[ival],smax[ival],sdefmin[ival],sdefmax[ival]);
    printf(" : offmin=%e offmax=%e\n",offmin[ival],offmax[ival]);
 }
 */
    if (limFlag==0) {
       setLmin();
       setLmax();
    }
    limFlag=1;
    text2slider();
    setSliderMin();
    setSliderMax();
}

void Mirror::getLmin()
{    //here we permit to set min BY HANDS
    QString t = FromLine->text();
    double v =t.toDouble();
    smin[ival] = v;
}
void Mirror::setLmin()
{
    char temp[100];
    sprintf (temp,"%11.5f",smin[ival]);
    FromLine->setText(temp);
}
void Mirror::getLmax()
{    //here we permit to set max BY HANDS
    QString t = ToLine->text();
    double v =t.toDouble();
    smax[ival] = v;
    //printf("getLmax:  v=%f\n",v);
}
void Mirror::setLmax()
{
    char temp[100];
    sprintf (temp,"%11.5f",smax[ival]);
    ToLine->setText(temp);
    //printf("setLmax:  v=%f\n",smax[ival]);
}

void Mirror::text2slider()
{   
    sliderMinV[ival] =(int)((smin[ival]-sminc[ival])/sliderFstep[ival]);
    sliderMaxV[ival] =(int)((smax[ival]-sminc[ival])/sliderFstep[ival]);
}

void Mirror::fromLChanged()
{    //here we permit to set arbitrary min
    getLmin();
    if (smin[ival]<sminc[ival])   sminc[ival]=smin[ival];
    sliderFstep[ival] = (smaxc[ival]-sminc[ival])/sliderNsteps;
    limFlag=0;
    updateM();
}
void Mirror::toLChanged()
{    //here we permit to set arbitrary max
    getLmax();
    if (smax[ival]>smaxc[ival])   smaxc[ival]=smax[ival];
    sliderFstep[ival] = (smaxc[ival]-sminc[ival])/sliderNsteps;
    limFlag=0;
    updateM();
}


void Mirror::slider2text()
{    //with slider we can  set min&max only  INSIDE available limits
    smin[ival] = sliderMinV[ival]*sliderFstep[ival]+sminc[ival] ;
    smax[ival] = sliderMaxV[ival]*sliderFstep[ival]+sminc[ival] ;
}

void Mirror::getSliderMin()
{    
    sliderMinV[ival] = ScrollBarFrom->value();
}
void Mirror::setSliderMin()
{
    ScrollBarFrom->setValue( sliderMinV[ival] );
}
void Mirror::getSliderMax()
{
    sliderMaxV[ival] = ScrollBarTo->value();
}
void Mirror::setSliderMax()
{
    ScrollBarTo->setValue( sliderMaxV[ival] );
}

void Mirror::fromSChanged(int /* v */)
{    //with slider we can  set min ONLY INSIDE available limits
    getSliderMin();
    slider2text();
    limFlag=0;
    updateM();
}
void Mirror::toSChanged(int /* v */)
{    //with slider we can  set max ONLY INSIDE available limits
    getSliderMax();
    slider2text();
    limFlag=0;
    updateM();
}

void Mirror::setMinMax()  //only change the values
{
    if (offset[ival] == 0) {
       smin[ival]=sdefmin[ival];
       smax[ival]=sdefmax[ival];
    } else {
       smin[ival]=offmin[ival];
       smax[ival]=offmax[ival];
    } 
    sminc[ival]=smin[ival];
    smaxc[ival]=smax[ival];
    sliderFstep[ival] = (smaxc[ival]-sminc[ival])/sliderNsteps;
}

void Mirror::setDefaultLim()
{
   sdefmin[ival]=dmin[ival];
   sdefmax[ival]=dmax[ival];

    setMinMax();
    limFlag=0;
    updateM();
}


void Mirror::takeOffset()
{
 long time_d;
 char time_arr[27];
 char time_h[9];
 char *time_str;
 int ir,im,i;
 for (int iv=0;iv<NVAL;iv++) {
    if (usedVal[iv]==1) {
       for (ir=0; ir<nrings; ir++) {
           int nmir = r0[ir].nmir;
           for (im=0; im<nmir; im++) {
               r0[ir].mir[im].offset[iv]=r0[ir].mir[im].data[iv];
           }
       }
    }
 }
 time(&time_d);
 time_str = ctime(&time_d);
 sprintf(time_arr,"%s",time_str);
 for (i=0;i<8;i++) time_h[i]=time_arr[i+11];
 time_h[8]='\0';
 TimeL->setText(time_h);
}


void Mirror::setOffset()
{

    if (offset[ival]==0) {
       OffsetOnOff->setText( "Offset ON" );
       offset[ival] = 1;
       //OffsetOnOff->setPaletteBackgroundColor( QColor( 255, 0, 255) );
       //OffsetOnOff->setPaletteForegroundColor( QColor( 0, 255, 0) );
       //printf("1 sdefmin,x=%f %f offmin,x=%f %f\n",sdefmin[ival],sdefmax[ival],offmin[ival],offmax[ival]);
    }  else {
       OffsetOnOff->setText( "Offset OFF" );
       offset[ival] = 0;
       //OffsetOnOff->setPaletteBackgroundColor( QColor( 100, 100, 100) );
       //OffsetOnOff->setPaletteForegroundColor( QColor( 255, 0, 255) );
    }
    setDefaultLim();
}

void Mirror::changeColorTable()
{
     int i;

     QPopupMenu *ColTable = new QPopupMenu (ColorTable,"colIndex");
     for (i=0; i<colTableMax; i++) {
         ColTable->insertItem(CTable[i],this,SLOT(setColTable(int)),0,i,i);
     }
     ColTable->exec(QCursor::pos());
}
void Mirror::setColTable(int ind)
{
    colortable[ival] = ind;
    ColorTable->setText(CTable[colortable[ival]]);
    //ColorTable->setPaletteBackgroundColor( QColor( 100, 0, 0) );
    //ColorTable->setPaletteForegroundColor( QColor( 0, 255, 255) );
    if (ind==2) {
       userdefined[ival] = 1;
       //ColorTable->setPaletteBackgroundColor( QColor( 240, 200, 200 ) );
       //ColorTable->setPaletteForegroundColor( QColor( 0, 0, 0) );
       showUserDefined();
    }  else {
       userdefined[ival] = 0;
       hideUserDefined();
    }
    updateM();
}


void Mirror::showUserDefined()
{
    Mapping->show();
    MinColor->show();
    MaxColor->show();
}
void Mirror::hideUserDefined()
{
    Mapping->hide();
    MinColor->hide();
    MaxColor->hide();
}


void Mirror::changeMap()
{
     int i;

     QPopupMenu *MapIndex = new QPopupMenu (Mapping,"mapIndex");
     for (i=0; i<indMapMax; i++) {
         MapIndex->insertItem(Map[i],this,SLOT(setMap(int)),0,i,i);
     }
     MapIndex->exec(QCursor::pos());
     //MapIndex->setActiveItem(indMap[ival]);
}
void Mirror::setMap(int ind)
{
    indMap[ival] = ind;
    Mapping->setText(Map[indMap[ival]]);
    updateM();
}

void Mirror::changeMinColor()
{
    minColor[ival] = QColorDialog::getColor (minColor[ival],this,"colorDialog");
   // if ( maxColor[ival].rgb()< minColor[ival].rgb()) minColor[ival].setRgb(maxColor[ival].rgb()-100);
    setMinColor();
    updateM();
}
void Mirror::setMinColor()
{
    MinColor->setPaletteBackgroundColor( minColor[ival] );
    int r=minColor[ival].red();
    int g=minColor[ival].green();
    int b=minColor[ival].blue();
    MinColor->setPaletteForegroundColor( QColor( 255-r, 255-g, 255-b) );
    cc[ival] = maxColor[ival].rgb() - minColor[ival].rgb();
    if ( cc[ival] <10 ) cc[ival]= (QRgb) 10;
//    printf ("rgb: %d\n", (unsigned int) minColor[ival].rgb());
}
void Mirror::changeMaxColor()
{
    maxColor[ival] = QColorDialog::getColor (maxColor[ival],this,"colorDialog");
    //if ( maxColor[ival].rgb()< minColor[ival].rgb()) maxColor[ival].setRgb(minColor[ival].rgb()+100);
    setMaxColor();
    updateM();
}
void Mirror::setMaxColor()
{
    MaxColor->setPaletteBackgroundColor( maxColor[ival] );
    int r=maxColor[ival].red();
    int g=maxColor[ival].green();
    int b=maxColor[ival].blue();
    MaxColor->setPaletteForegroundColor( QColor( 255-r, 255-g, 255-b) );
    cc[ival] = maxColor[ival].rgb() - minColor[ival].rgb();
    if (cc[ival] <10 ) cc[ival]= (QRgb) 10;
}


QColor Mirror::value2color(double v, int bad)
{
   int red,green,blue;
   double y;
   QRgb rgb;
   QColor c;

//    printf("value2color: ival=%d  smin=%f smax=%f\n",ival,smin[ival],smax[ival]);
   if (bad == 1) {
      //c = ????? background ();
      c = QColor (170,170,170);
      return c;
   }
   if (v>=smax[ival]) {
      c = QColor (255,255,255);
      return c;
   }
   if (v<smin[ival]) {
      c = QColor (0,0,0);
      return c;
   }

   y = (v-smin[ival])/xx[ival];

   if (userdefined[ival]==1) {
      switch (indMap[ival]) {
         case 0:
           rgb = y*cc[ival] + minColor[ival].rgb();
           break;
         case 1:
           rgb = y*y*cc[ival] + minColor[ival].rgb();
           break;
         case 2:
           rgb = sqrt(y)*cc[ival] + minColor[ival].rgb();
           break;
         case 3:
           rgb = exp(y)*cc[ival] + minColor[ival].rgb();
           break;
         case 4:
           rgb = log(y)*cc[ival] + minColor[ival].rgb();
      }
      c.setRgb (rgb);
   } else {
     if (colortable[ival]==0) {     //black-red
        //int red = y*200;
        // a-la IDL red-temperatures
         int col_ind=(int)(y*255);
         if (col_ind <=180)  red=(int)(col_ind/180.*255);
         else red=255;
         if (col_ind >=120)  green=(int)((col_ind-120.)/135.*255);
         else green=0;
         if (col_ind >=190)  blue=(int)((col_ind-190.)/65.*255);
         else blue=0;

   //if (ival==0)
   //printf("--ival=%d v=%f y=%f red=%d green=%d blue=%d col_ind=%d\n",ival,v,y,red,green,blue,col_ind);
   //printf("--ival=%d v=%f y=%f red=%d smin=%f smax=%f xx=%f\n",ival,v,y,red,smin[ival],smax[ival],xx[ival]);
        //c = QColor (red+55,0,0);
        c = QColor (red,green,blue);
     }
     if (colortable[ival]==1) {     // 16 colors
        float step=1./16.;
        //if (y>=0. && y<=step) c = QColor (125,0,0);
        //if (y>step && y<=step*2.) c = QColor (200,50,0);
        //if (y>step*2. && y<=step*3.) c = QColor (255,85,0);
        //if (y>step*3. && y<=step*4.) c = QColor (255,170,0);
        //if (y>step*4. && y<=step*5.) c = QColor (255,255,0);
        //if (y>step*5. && y<=step*6.) c = QColor (0,90,0);
        //if (y>step*6. && y<=step*7.) c = QColor (0,170,0);
        //if (y>step*7. && y<=step*8.) c = QColor (0,255,0);
        //if (y>step*8. && y<=step*9.) c = QColor (0,255,255);
        //if (y>step*9. && y<=step*10.) c = QColor (0,170,255);
        //if (y>step*10. && y<=step*11.) c = QColor (0,0,255);
        //if (y>step*11. && y<=step*12.) c = QColor (85,0,127);
        //if (y>step*12. && y<=step*13.) c = QColor (170,0,127);
        //if (y>step*13. && y<=step*14.) c = QColor (170,0,255);
        //if (y>step*14. && y<=step*15.) c = QColor (255,0,127);
        //if (y>step*15. && y<=step*16.) c = QColor (255,85,255);
        // a-la idl 16-color table
        if (y>=0. && y<=step) c = QColor (0,84,0);
        if (y>step && y<=step*2.) c = QColor (0,168,0);
        if (y>step*2. && y<=step*3.) c = QColor (0,255,0);
        if (y>step*3. && y<=step*4.) c = QColor (0,255,84);
        if (y>step*4. && y<=step*5.) c = QColor (0,255,168);
        if (y>step*5. && y<=step*6.) c = QColor (0,255,255);
        if (y>step*6. && y<=step*7.) c = QColor (0,0,255);
        if (y>step*7. && y<=step*8.) c = QColor (128,0,255);
        if (y>step*8. && y<=step*9.) c = QColor (255,0,220);
        if (y>step*9. && y<=step*10.) c = QColor (255,0,180);
        if (y>step*10. && y<=step*11.) c = QColor (255,0,128);
        if (y>step*11. && y<=step*12.) c = QColor (255,0,64);
        if (y>step*12. && y<=step*13.) c = QColor (255,0,0);
        if (y>step*13. && y<=step*14.) c = QColor (220,190,190);
        if (y>step*14. && y<=step*15.) c = QColor (220,220,220);
        if (y>step*15. && y<=step*16.) c = QColor (255,255,255);
     }
   }
//   printf("--ival=%d v=%f y=%f rgb=%d\n",ival,v,y,rgb);
   return c;
   
}


// title of this mirror widget
void Mirror::mtitle()
{
    double mean,rms,v;
    int ir,im;

    char temp[100];
    int c1,c2,c3;
    double v1,v2;

    sprintf (temp,"%s",Name[ival]);
    MTitleButton->setText(temp);
    if (offset[ival] == 0) {
       v1=dmin[ival];
       v2=dmax[ival];
    } else {
       v1=offmin[ival];
       v2=offmax[ival];
    } 

    int Ntot=0;
    mean = 0.;
    rms  = 0.;
    for (ir=0; ir<NRINGS; ir++) {
        for (im=0; im<r0[ir].nmir; im++) {             
            if (r0[ir].mir[im].bad[ival] == 0 ) {
               v = r0[ir].mir[im].data[ival];
               if (offset[ival]==1)   v = v - r0[ir].mir[im].offset[ival];
               mean += v;
               Ntot++;
            }
        }
    }
    mean = mean/Ntot;
    for (ir=0; ir<NRINGS; ir++) {
        for (im=0; im<r0[ir].nmir; im++) {
            if (r0[ir].mir[im].bad[ival] == 0 ) {
               v = r0[ir].mir[im].data[ival];
               if (offset[ival]==1)   v = v - r0[ir].mir[im].offset[ival];
               rms += (v-mean)*(v-mean);
            }
        }
    }
    rms = sqrt(rms/Ntot);

    //sprintf (temp,"min=%9.5f max=%9.5f",dmin[ival],dmax[ival]);
    //sprintf (temp," min=%4.2e max=%4.2e\n mean=%3.1e  rms=%3.1e",v1,v2,mean,rms);
    //sprintf (temp,"   min=%9.5f\n   max=%9.5f\n   mean=%9.5f\n    rms=%9.5f",v1,v2,mean,rms);
    sprintf (temp,"   min=%9.5f   max=%9.5f\n   mean=%9.5f rms=%9.5f",v1,v2,mean,rms);
    MTitle->setText(temp);

    //set different color for title line
    switch (ival) {
     case 0:
     case 1:
       c1=180;
       c2=0;
       c3=0;
       break;
     case 2:
     case 3:
     case 4:
     case 5:
     case 6:
     case 7:
       c1=0;
       c2=100;
       c3=0;
       break;
     case 8:
     case 9:
     case 10:
     case 11:
       c1=0;
       c2=0;
       c3=180;
       break;
     default:
       c1=180;
       c2=180;
       c3=180;
    }
    MTitleButton->setPaletteForegroundColor( QColor( c1,c2,c3 ) );
    MTitle->setPaletteForegroundColor( QColor( c1,c2,c3 ) );
    Default->setPaletteBackgroundColor( QColor( c1,c2,c3 ) );
    Default->setPaletteForegroundColor( QColor( 255-c1,255-c2,255-c3 ) );
    TakeOffset->setPaletteForegroundColor( QColor( c1,c2,c3 ) );
    OffsetOnOff->setPaletteForegroundColor( QColor( c1, c2, c3) );
    ColorTable->setPaletteForegroundColor( QColor( c1, c2, c3) );

}

void Mirror::changeMTitle()
{
     int i,i0,imax;

     QPoint p = mapFromGlobal(QCursor::pos());
     if (p.x()>MTitleX ||p.y()>MTitleY) return; //to prevent the parasite calls!!!
//    printf("changeMtitle: ival=%d i=%d  smin=%f smax=%f\n",ival,i,smin[ival],smax[ival]);

     QPopupMenu *MTitlePopup = new QPopupMenu (MTitleButton,"mtitlePopup");
     MTitlePopup = new QPopupMenu (MTitleButton,"mtitlePopup");
     if (ival<2) {
        i0=0;
        imax=2;
     }
     if (ival>1 && ival<8) {
        i0=2;
        imax=8;
     }
     if (ival>7 && ival<12) {
        i0=8;
        imax=12;
     }
     for (i=i0; i<imax; i++) {
         MTitlePopup->insertItem(Name[i],this,SLOT(setMTitle(int)),0,i,i);
         //MTitlePopup->setWhatsThis(i,NameHelp[i]);
         // this service does not work(why?) so I use 2 tricky connections below
     }
     connect( MTitlePopup, SIGNAL( highlighted(int) ), this, SLOT( helpMTitle(int) ) );
     connect( MTitlePopup, SIGNAL( activated(int) ), this, SLOT( helpMTitleClose(int) ) );
     MTitlePopup->exec(QCursor::pos());
}

QPopupMenu *MTitleHelp;
void Mirror::helpMTitle(int i)
{
     QPoint pp = QCursor::pos();
     int x=pp.x()+100;
     int y=pp.y()+30;
     pp.setX(x);
     pp.setY(y);
     if (MTitleHelp!=(QPopupMenu *)NULL) MTitleHelp->close();
        MTitleHelp = new QPopupMenu (MTitleButton,"mtitleHelp");
        MTitleHelp->setPaletteBackgroundColor( QColor( 255, 255, 255 ) );
        MTitleHelp->insertItem(NameHelp[i],0,0);
        //MTitleHelp->exec(QCursor::pos());
        MTitleHelp->exec(pp);
}

void Mirror::helpMTitleClose(int /* i */)
{
     if (MTitleHelp!=(QPopupMenu *)NULL) MTitleHelp->close();
}

void Mirror::setMTitle(int i)
{
    char temp[100];
//    printf("setMtitle: ival=%d i=%d smin=%f smax=%f\n",ival,i,smin[ival],smax[ival]);

    if( ival==i) return;
    userdefined[i]=userdefined[ival];
    colortable[i]=colortable[ival];
    ival=i;
    sprintf (temp,"%s",Name[ival]);
    MTitleButton->setText(temp);

    offset[ival]=1-offset[ival];
    setOffset();
    setMinMax();
    setMLimits();
    limFlag=0;
    updateM();
}

void Mirror::setMBrush(int iv)
{
// if (iv==0)
// printf("setMBrush: ival=%d id=%d smin=%f smax=%f xx=%f cc=%d minColor=%d\n",iv,id,smin[ival],smax[ival],xx[ival],cc[ival],minColor[ival].rgb());
    for (int ir=0; ir<NRINGS; ir++) {
        for (int im=0; im<r0[ir].nmir; im++) {
// if (ir==10 && im==8) printf("setBrush: ival=%d v=%f bad=%d smin=%f smax=%f xx=%f\n",iv,r0[ir].mir[im].data[iv],r0[ir].mir[im].bad[iv],smin[ival],smax[ival],xx[ival]);
            double v = r0[ir].mir[im].data[iv];
            if (offset[iv]==1)   v = v - r0[ir].mir[im].offset[iv];
            r0[ir].mir[im].br_c[iv]=value2color(v,r0[ir].mir[im].bad[iv]);
        }
    }
}


void Mirror::init(int iv, int ident, int nr, int x, int y, int rmn, int rmx)
{
    int ir,im,i,i1;
    double a,da1,da2;
    double sa,ca;
    double a00;
    int xp,yp,r;

    ival = iv;
    id = ident;
//printf("init: ival=%d id=%d\n",ival,id);

    Init[ival] =0;
    first[ival]=0;
    usedVal[ival]=0;
    UpdateMC=0;
    limFlag=0;

    //initialization of sliders properties
    sliderNsteps = 2000;              //number of steps in sliders
    sliderMinV[ival] = 0;             //initial settings
    sliderMaxV[ival] = sliderNsteps;
    ScrollBarFrom->setMinValue( 0 );
    ScrollBarFrom->setMaxValue( sliderNsteps);
    ScrollBarFrom->setLineStep( 1 );
    ScrollBarFrom->setPageStep( 5 );
    ScrollBarTo->setMinValue( 0 );
    ScrollBarTo->setMaxValue( sliderNsteps);
    ScrollBarTo->setLineStep( 1 );
    ScrollBarTo->setPageStep( 5 );

    offset[ival]=0;
//    OffsetOnOff->setPaletteBackgroundColor( QColor( 100, 100, 100) );
//    OffsetOnOff->setPaletteForegroundColor( QColor( 255, 0, 255) );

    colTableMax=3;
    colortable[ival]=0;
//    ColorTable->setPaletteBackgroundColor( QColor( 100, 0, 0) );
//    ColorTable->setPaletteForegroundColor( QColor( 0, 255, 255) );
    indMapMax=5;
    indMap[ival]=0;
    Mapping->setText(Map[indMap[ival]]);
    Mapping->setPaletteBackgroundColor( QColor( 240, 200, 200 ) );
    Mapping->setPaletteForegroundColor( QColor( 0,0,0 ) );

    minColor[ival] = QColor( 0, 0, 0 );
    maxColor[ival] = QColor( 255, 255, 255 );
    setMinColor();
    setMaxColor();

    hideUserDefined();

    //intialization of mirror properties
    nrings  = nr;
    xc = x;
    yc = y;
    rmin = rmn;
    rmax = rmx;
    xM0=xc-rmax-5;
    yM0=yc-rmax-5;
    widthM =2*rmax+10;
    hightM =2*rmax+10;

    angle = 0;
    double pi2=2.*PI;
    double pip=.5*PI;

    int dr = (rmax - rmin -nrings*2)/nrings;   //width of each ring in pixels
    int np = NPOLIG/2;

    r = rmin;
    for (ir=0; ir<nrings; ir++) {
        int nmir = mirring[ir];
        int idr = id + ir*10;
        r0[ir].id = idr;
        r0[ir].nmir = nmir;

        da1 = pi2/nmir;
        da2 = da1/(np-1);
        a = +a0ring[ir]+da1/2.-pip ;  // we fill ring from x axis (right 2 left)
                              // with initial rotation a0ring[] of the attuator CENTER
        for (im=0; im<nmir; im++) { 
            r0[ir].mir[im].id = idr + im*1000;
            r0[ir].mir[im].np = np;                   // number of pairs of points in polygon
            r0[ir].mir[im].pen_c[ival] = Qt::blue;    //pen color
            r0[ir].mir[im].br_c[ival] = Qt::yellow;   //brush color (default)

            a += da2;
            r0[ir].mir[im].p = QPointArray(NPOLIG);
            a00=a-da2;
            if (a00<=-pi2) a00=a00+pi2;
            r0[ir].mir[im].angle1 = a00;           //starting angle of the poligon (sector)
                                                   // we set here the SECOND limit because the angle
                                                   // is going into NEGATIVE direction
            for (i=0; i<np; i++) {    //  points coordinates definition
                a -= da2;
                sa = sin(a);
                ca = cos(a);
                xp = xc + (int) (r*ca);
                yp = yc + (int) (r*sa);
                if (i==0) i1=0; else i1=NPOLIG-i;
                r0[ir].mir[im].p.setPoint(i1,xp,yp);
                xp = xp + (int) (dr*ca);
                yp = yp + (int) (dr*sa);
                r0[ir].mir[im].p.setPoint(i+1,xp,yp);
            }
            a00=a;
            if (a00<=-pi2) a00=a00+pi2;
            r0[ir].mir[im].angle0 = a00;              //ending angle of the poligon (sector)
 
//        if (ir==0) printf("im=%d a0=%f a1=%f\n",im,r0[ir].mir[im].angle0,r0[ir].mir[im].angle1);
        }
        r0[ir].radiusN = r;                         // min radius of the sector
        r = r+dr;
        r0[ir].radiusX = r;                         // max radius of the sector
    }
}


void Mirror::fillM(int *ibuf,double *buf,int iv)  //ibuf[i] =1 if attuator "i" is disabled
{    // here we place data into the structure & find min/max

    double v,vo;
//    static int num=0;

//    num++;
//    printf("fillM: num=%d iv=%d smin=%f smax=%f\n",num,iv,smin[iv],smax[iv]);
//    if (iv==0)
//    printf("fillM0: num=%d iv=%d smin=%f smax=%f\n",num,iv,smin[iv],smax[iv]);
   // if (smin[0]==0. && iv==0)
   // printf("fillM1: num=%d iv=%d smin=%f smax=%f\n",num,iv,smin[iv],smax[iv]);
    int i=-1;
    int bad=0;
    double amin=1.e20;
    double amax=-1.e20;
    double amino=1.e20;
    double amaxo=-1.e20;
    double t1,t2,t3,t4;

    for (int ir=0; ir<NRINGS; ir++) {
        for (int im=0; im<r0[ir].nmir; im++) {
            i++;
            bad = ibuf[i];
            r0[ir].mir[im].bad[iv]=bad;
            v = buf[i];
            if (bad==0) {
               if (v<amin) amin=v;
               if (v>amax) amax=v;
            }  else {
               v=0.;
            }
            if (first[iv]==0) r0[ir].mir[im].offset[iv]=0.;
            if (offset[ival]==1) {
               vo=v-r0[ir].mir[im].offset[iv]; //if offset is not yet taken, this will be normal min&max
               if (bad==0) {
                  if (vo<amino) amino=vo;
                  if (vo>amaxo) amaxo=vo;
               }
            } else {
              amino=0.;
              amaxo=0.;
            }
            r0[ir].mir[im].data[iv] = v;
        }
    }
    dmin[iv]= amin;    // current min&max  we save to show
    dmax[iv]= amax;

    //default min&max we take as MIN & MAX of all already obtained to the moment
    t1=dmin[iv]-0.001*fabs(dmin[iv]);
    t2=dmax[iv]+0.001*fabs(dmax[iv]);
    t3=amino-0.001*fabs(amino);
    t4=amaxo+0.001*fabs(amaxo);
    if (first[iv]==0) {  //it is a first data set arrived
       sdefmin[iv]=t1;
       sdefmax[iv]=t2;
       offmin[iv] =t1;   // certainly there is NO offset taken yet
       offmax[iv] =t2;
    } else {             // all next
      if (t1< sdefmin[iv]) sdefmin[iv]=t1;
      if (t2> sdefmax[iv]) sdefmax[iv]=t2;
      offmin[iv] =t3;
      offmax[iv] =t4;
    }
    first[iv]=1;    // first comes only ones
    usedVal[iv]=1;  // mark the ival position as used
}

/*
void Mirror::updateRings(int first,int nrings) //if we will need to rotate, etc.
{
    for (int ir=first; ir<first+nrings; ir++) {
        r1[ir]=r0[ir];
    }
}
*/
