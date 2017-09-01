/****************************************************************************
** Form interface generated from reading ui file 'P672.ui'
**
** Created: lun lug 14 10:40:30 2008
**
** WARNING! All changes made in this file will be lost!
****************************************************************************/

#ifndef ACTUATORSMAP_H
#define ACTUATORSMAP_H

#include <qvariant.h>
#include <qdialog.h>
#include <qtimer.h> 
#include <qframe.h>

class QVBoxLayout;
class QHBoxLayout;
class QGridLayout;
class QSpacerItem;

class ActuatorsMap : public QFrame
{
    Q_OBJECT

public:
    ActuatorsMap( QWidget* parent = 0, const char* name = 0, WFlags fl = 0 );
    virtual ~ActuatorsMap();

    float _attuatori[900];
    
    QTimer *_timer;
    
public slots:
    void updateLeds();

protected:
    virtual void paintEvent (QPaintEvent *);

};

#endif // ACTUATORSMAP_H
