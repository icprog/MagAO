/****************************************************************************
** Form interface generated from reading ui file 'PlotForm.ui'
**
** Created by User Interface Compiler
**
** WARNING! All changes made in this file will be lost!
****************************************************************************/

#ifndef PLOTFORM_H
#define PLOTFORM_H

#include <string>
using namespace std;

#include <qvariant.h>
#include <qpen.h>
#include <qdialog.h>
#include "qwt_plot.h"
#include "qwt_text.h"
#include "qwt_plot_zoomer.h"

class QVBoxLayout;
class QHBoxLayout;
class QGridLayout;
class QSpacerItem;
class QwtPlot;
class QwtPlotZoomer;
class QwtText;
class QPushButton;
class QPen;

class PlotForm : public QDialog
{
    Q_OBJECT

public:
    PlotForm( QWidget* parent = 0, const char* name = 0, bool modal = FALSE, WFlags fl = 0 );
    ~PlotForm();

    QwtPlot* qwtPlot1;
    QwtPlotZoomer* qwtZoom;
//    QPushButton* buttonHelp;
    QPushButton* buttonOk;
    QPushButton* buttonCancel;

    QColor backgr;
    int Nx; //number of points to plot
    int iy; //which array of Y to plot
    bool   Refresh;
    int    inVector;
    string Panel;
    int    IndexP;
    QwtText Title;
    QwtText xtitle;
    double x[500];
    double xmin;
    double xmax;
    double y[10][500];
    double ymin[10];
    double ymax[10];
    double wmin[10];
    double wmax[10];
    double amin[10];
    double amax[10];
    double mean[10];
    double rms[10];
    QPen   pen[10];
    QwtText ytitle[10];

    virtual void plotxy();

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

#endif // PLOTFORM_H
