/****************************************************************************
** Form implementation generated from reading ui file 'P672.ui'
**
** Created: lun lug 14 10:41:38 2008
**
** WARNING! All changes made in this file will be lost!
****************************************************************************/

#include "P672.h"

#include <qvariant.h>
#include <kled.h>
#include <qlayout.h>
#include <qtooltip.h>
#include <qwhatsthis.h>
#include <qapplication.h>
#include <qpainter.h>

#define ACTUATOR_SPACING (20)
#define ACTUATOR_SIZE    (10)

/*
 *  Constructs a ActuatorsForm as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 *  The dialog will by default be modeless, unless you set 'modal' to
 *  TRUE to construct a modal dialog.
 */
ActuatorsMap::ActuatorsMap( QWidget* parent, const char* name,  WFlags fl )
    : QFrame( parent, name, fl )
{
    if ( !name )
	setName( "ActuatorsMap" );
    setPaletteForegroundColor( QColor( 255, 255, 127 ) );
    QPalette pal;
    QColorGroup cg;
    cg.setColor( QColorGroup::Foreground, QColor( 255, 255, 127) );
    cg.setColor( QColorGroup::Button, QColor( 221, 223, 228) );
    cg.setColor( QColorGroup::Light, white );
    cg.setColor( QColorGroup::Midlight, white );
    cg.setColor( QColorGroup::Dark, QColor( 85, 85, 85) );
    cg.setColor( QColorGroup::Mid, QColor( 199, 199, 199) );
    cg.setColor( QColorGroup::Text, black );
    cg.setColor( QColorGroup::BrightText, white );
    cg.setColor( QColorGroup::ButtonText, black );
    cg.setColor( QColorGroup::Base, white );
    cg.setColor( QColorGroup::Background, QColor( 239, 239, 239) );
    cg.setColor( QColorGroup::Shadow, black );
    cg.setColor( QColorGroup::Highlight, QColor( 103, 141, 178) );
    cg.setColor( QColorGroup::HighlightedText, white );
    cg.setColor( QColorGroup::Link, QColor( 0, 0, 238) );
    cg.setColor( QColorGroup::LinkVisited, QColor( 82, 24, 139) );
    pal.setActive( cg );
    cg.setColor( QColorGroup::Foreground, QColor( 255, 255, 127) );
    cg.setColor( QColorGroup::Button, QColor( 221, 223, 228) );
    cg.setColor( QColorGroup::Light, white );
    cg.setColor( QColorGroup::Midlight, white );
    cg.setColor( QColorGroup::Dark, QColor( 85, 85, 85) );
    cg.setColor( QColorGroup::Mid, QColor( 199, 199, 199) );
    cg.setColor( QColorGroup::Text, black );
    cg.setColor( QColorGroup::BrightText, white );
    cg.setColor( QColorGroup::ButtonText, black );
    cg.setColor( QColorGroup::Base, white );
    cg.setColor( QColorGroup::Background, QColor( 239, 239, 239) );
    cg.setColor( QColorGroup::Shadow, black );
    cg.setColor( QColorGroup::Highlight, QColor( 103, 141, 178) );
    cg.setColor( QColorGroup::HighlightedText, white );
    cg.setColor( QColorGroup::Link, QColor( 0, 0, 238) );
    cg.setColor( QColorGroup::LinkVisited, QColor( 82, 24, 139) );
    pal.setInactive( cg );
    cg.setColor( QColorGroup::Foreground, QColor( 255, 255, 127) );
    cg.setColor( QColorGroup::Button, QColor( 221, 223, 228) );
    cg.setColor( QColorGroup::Light, white );
    cg.setColor( QColorGroup::Midlight, white );
    cg.setColor( QColorGroup::Dark, QColor( 85, 85, 85) );
    cg.setColor( QColorGroup::Mid, QColor( 199, 199, 199) );
    cg.setColor( QColorGroup::Text, QColor( 199, 199, 199) );
    cg.setColor( QColorGroup::BrightText, white );
    cg.setColor( QColorGroup::ButtonText, QColor( 128, 128, 128) );
    cg.setColor( QColorGroup::Base, white );
    cg.setColor( QColorGroup::Background, QColor( 239, 239, 239) );
    cg.setColor( QColorGroup::Shadow, black );
    cg.setColor( QColorGroup::Highlight, QColor( 86, 117, 148) );
    cg.setColor( QColorGroup::HighlightedText, white );
    cg.setColor( QColorGroup::Link, QColor( 0, 0, 238) );
    cg.setColor( QColorGroup::LinkVisited, QColor( 82, 24, 139) );
    pal.setDisabled( cg );
    setPalette( pal );

    setMinimumSize( 30*ACTUATOR_SPACING, 30*ACTUATOR_SPACING);
    setMaximumSize( 30*ACTUATOR_SPACING, 30*ACTUATOR_SPACING);

    setBackgroundColor(QColor(0,0,0));

	for(int i=0; i<30; i++) {
		for(int j=0; j<30; j++) {
         _attuatori[30*i+j] = 0;
		}
	}
	
    _timer = new QTimer(this);
    connect( _timer, SIGNAL(timeout()), SLOT(updateLeds()) );
    _timer->start(50);
}

/*
 *  Destroys the object and frees any allocated resources
 */
ActuatorsMap::~ActuatorsMap()
{
    // no need to delete child widgets, Qt does it all for us
}

void ActuatorsMap::updateLeds() {
	
	// Loop di modifica
	QColor red(255,0,0);
	QColor green(0,255,0);
	
	for(int i=0; i<30; i++)
		for(int j=0; j<30; j++)
			_attuatori[30*i+j] = ((float)rand()) / RAND_MAX;

   repaint(false);
}

void ActuatorsMap::paintEvent( QPaintEvent *e)
{
   QPainter painter(this);
	for(int i=0; i<30; i++)
		for(int j=0; j<30; j++)
         {
         int value = (int)(_attuatori[30*i+j] * 255);
         QColor color(value, value, value);
         painter.fillRect( i*ACTUATOR_SPACING, j*ACTUATOR_SPACING, ACTUATOR_SIZE, ACTUATOR_SIZE, color);
         }
//   drawFrame(&painter);
}


int main(int argc, char* argv[]) {
	
	QApplication a( argc, argv );
    ActuatorsMap actuators;
    a.setMainWidget(&actuators);
    actuators.show();
    
    
    return a.exec();
	
	return 0;
}

