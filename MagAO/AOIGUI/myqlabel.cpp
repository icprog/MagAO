#include "myqlabel.h"

MyQLabel::MyQLabel( QWidget *parent, const char* name, WFlags f )
        : QLabel( parent, name, f )
{
    setAlignment( int( QLabel::AlignVCenter | QLabel::AlignCenter ) );
};

void MyQLabel::setAutoEraseBkg ( bool AutoErase )
{
	if (AutoErase)
		clearWFlags (Qt::WNoAutoErase);
	else
		setWFlags (Qt::WNoAutoErase);
};
