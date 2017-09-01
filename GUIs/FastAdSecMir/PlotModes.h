/****************************************************************************
** plot modes data
****************************************************************************/

#ifndef PLOTMODES_H
#define PLOTMODES_H

#include <string>
#include <stdio.h>
#include <stdlib.h>

using namespace std;

#include <qvariant.h>
#include <qpen.h>
#include <qdialog.h>
#include "qwt_plot.h"
#include "qwt_plot_curve.h"
#include "qwt_picker.h"
#include "qwt_plot_zoomer.h"
#include "qwt_text.h"

class QCheckBox;
class QVBoxLayout;
class QHBoxLayout;
class QGridLayout;
class QSpacerItem;
class QwtPlot;
class QwtText;
class QwtPlotZoomer;
class QPushButton;
class QLabel;
class QPen;


#define NMIRS 14
#define NX 672  //number of points to plot



class PlotModes : public QDialog
{
    Q_OBJECT

public:
    PlotModes( QWidget* parent = 0, const char* name = 0, bool modal = FALSE, WFlags fl = 0 );
    ~PlotModes();

    QwtPlot* qwtPlot1;
    QwtPlotZoomer* qwtZoom;
    QwtPlotCurve *c[NMIRS];
//    QPushButton* buttonHelp;
    QCheckBox* checkBox1;
    QCheckBox* checkBox2;

    QPushButton* buttonOk;
    QPushButton* buttonCancel;
    QLabel* TimeR;

    QColor backgr;

    int Nx; //number of points to plot
    bool   Refresh;
    int    inVector;
    int    type;
    QColor timeCol;
    char   Title[50];
    QwtText xtitle;
    QwtText ytitle;
    char TimeRefr[9];
    double x[NX];
    double y[NX];
    double yl[NX];
    double ytmp[NX];
    int    en[NX];
    double xmin;
    double xmax;
    double ymin;
    double ymax;
    double ylim0;
    double ylim;
    bool xlog;
    bool ylog;
    QPen   pen;


    virtual void plotxy();

public slots:
    virtual void xlog_toggled( bool);
    virtual void ylog_toggled( bool);

signals:
    void removeMyPlot (int);

protected:
    QVBoxLayout* totLayout;
    QHBoxLayout* Layout1;
    QSpacerItem* Horizontal_Spacing2;

protected slots:
    virtual void quit();
    virtual void StopStart();
    virtual void languageChange();

};

#endif // PLOTMODES_H
