/****************************************************************************
****************************************************************************/

#ifndef MIRROR_H
#define MIRROR_H


#include <qwidget.h>
#include <qframe.h>
#include <qcolor.h>
//#include <qcolordialog.h>
#include <qpointarray.h>



#include <qvariant.h>

class QVBoxLayout;
class QHBoxLayout;
class QGridLayout;
class QSpacerItem;
//class QColorDialog;
class QSlider;
class QScrollBar;

class QLabel;
class QPushButton;
class QLineEdit;
class QPopupMenu;
class QColorDialog;

//class QColor;
//class QPointArray;

class QFrame;
class QPainter;
class QPaintEvent;



#define NPOLIG 8       // number of points in polygon, must be EVEN
#define NSMALL 100      // max number of small mirrors in the ring
#define NRINGS 14       // max number of rings
#define PI 3.141592653

#define NVAL 14         // number of data values for each actuator
                        // the name of each value is written in array Names defined in Mirror.cpp


struct smir {                //structure for small mirror
 int id;                     // identifier
 int np;                     // number of pairs of points in polygon (i.e. NPOLYG/2)
 QPointArray p;              // array of polygon points (x,y)
 double angle0;                 // angle0 of the actuator
 double angle1;                 // angle1 of the actuator
 double data[NVAL];           // array with associated data
 QColor pen_c[NVAL];          // pen color
 QColor br_c[NVAL];           // brush color
 int bad[NVAL];
 double offset[NVAL];         // array with offset data
};

struct ring {                //structure for each ring
 int id;                     // identifier
 int nmir;                   // number of small mirrors in the ring
 int radiusN;                // min radius of the ring
 int radiusX;                // max radius of the ring
 smir mir[NSMALL];           // each mirror structure
 float angle;                // angle to rotate a ring as a whole from vertical/up to right direction
};



//class Mirror: public QWidget      //class for compound mirror

class Mirror: public QFrame      //class for compound mirror
{
    Q_OBJECT

public:
    Mirror(QWidget* parent = 0, const char* name = 0 );
    ~Mirror();

    QPushButton* MTitleButton;
    int MTitleX,MTitleY;
    QLabel* MTitle;

    QPushButton* Default;

    QLabel* From;
    QLineEdit* FromLine;
    QScrollBar* ScrollBarFrom;
    QLabel* To;
    QLineEdit* ToLine;
    QScrollBar* ScrollBarTo;

    QLabel* Offset;
    QLineEdit* OffsetLine;
    QPushButton* OffsetBm;
    QPushButton* OffsetBp;
    QPushButton* OffsetB0;

    QLabel* TimeL;
    QPushButton* TakeOffset;
    QPushButton* OffsetOnOff;

    //QPushButton* ReflectH;
    //QPushButton* ReflectV;

    QPushButton* ColorTable;
    QPushButton* Mapping;
    QPushButton* MinColor;
    QPushButton* MaxColor;


    int xM0,yM0,widthM,hightM;

    int ival;  // wich value to show (index in  data[]
    double dmin[NVAL];    // physical min & max in the current pattern
    double dmax[NVAL];
    double sdefmin[NVAL]; // default min & max in the panel
    double sdefmax[NVAL];
    // normally =0.999*dmin & 1.001*dmax
    // where dmin & dmax are most min & max from all arrived data to the moment
    double offmin[NVAL]; // default min & max in the panel
    double offmax[NVAL]; // with offset subtracted
    // normally =0.999*dmin & 1.001*dmax   with offset  subtracted
    // where dmin & dmax are most min & max from all arrived data to the moment

    int    offset[NVAL];    // offset On=1 Off=0
    int    usedVal[NVAL];   // values used in current realisation of the class

    int    limFlag;        // =0 setLmin,Lmax; =1 do not it
    double smin[NVAL];     // current min & max in the panel
    double smax[NVAL];
    double sminc[NVAL];    // current EXTREME min & max in the panel
    double smaxc[NVAL];    // by default are sdef or offmin&max
    int    first[NVAL];
    double xx[NVAL];      // smax-smin            >=.01*smin || 0.01

    int    sliderNsteps;         //max number of steps in slider
    int    sliderMinV[NVAL];     //position of the min slider
    int    sliderMaxV[NVAL];     //position of the max slider
    double sliderFstep[NVAL];    // value per step in slider

    int    colortable[NVAL];
    int    colTableMax;
    int    userdefined[NVAL];
    int    indMap[NVAL];
    int    indMapMax;
    QColor minColor[NVAL];       // starting color in palette
    QColor maxColor[NVAL];       // last color in palette
    QRgb   cc[NVAL];             // last - start  >=100

public slots:
    
    void setMinMax();
    void setMLimits();
    void mtitle();
    void setColorScale();
    void setMBrush(int);
    void init(int,int,int,int,int,int,int);        //(ival,id,nrings,xc,yc,rmin,rmax)
    void updateM();              // update Mirror picture
    //void updateRings(int,int);   // (first ring,nrings)
    void fillM(int*,double*,int);  //(ibuf,buf,ival)  fill r0 for ival from ibuf & buf

    void getLmin();   // get smin value from lineEdit  FromLine
    void getLmax();   // get smax value from lineEdit  ToLine
    void setLmin();   // set smin value for lineEdit   FromLine
    void setLmax();   // set smax value for lineEdit   ToLine
    void getSliderMin();   // get value from scrollBarFrom
    void getSliderMax();   // get value from scrollBarTo
    void setSliderMin();   // set value for scrollBarFrom
    void setSliderMax();   // set value for scrollBarTo
    void text2slider();  
    void slider2text();
    void fromLChanged();
    void fromSChanged(int);
    void toLChanged();
    void toSChanged(int);

    void setDefaultLim();

    void takeOffset();
    void setOffset();

    void changeColorTable();

    void showUserDefined();
    void hideUserDefined();
    void changeMap();
    void changeMinColor();
    void setMinColor();
    void changeMaxColor();
    void setMaxColor();

    QColor value2color(double, int);


protected:
    void paintEvent ( QPaintEvent * );

protected slots:
    virtual void languageChange();
    void mouseDoubleClickEvent(QMouseEvent *);  //handler to provide information from a single actuator
    void changeMTitle();
    void helpMTitle(int);
    void helpMTitleClose(int);
    void setMTitle(int);
    void setMap(int);
    void setColTable(int);

private:
    int UpdateMC; //flag to prevent double chain execution of updateM() from internal parasite calls
    int Init[NVAL];
    int FirstFill[NVAL];
    int id;       // identifier
    int nrings;   // number of rings in compound mirror
    int xc,yc;    // center of the whole mirror (in pixels)
    int rmin;     // inner radius of the first ring  (in pixels)
    int rmax;     // outer radius of the last ring   (in pixels)
    float angle;  // angle to rotate everything as a whole from vertical/up to right direction
                  // all angles in radian
    ring r0[NRINGS];   // basic structure (without any rotation/reflection, etc.)
    //ring r1[NRINGS];   // structure to be really painted
};

#endif // MIRROR_H
