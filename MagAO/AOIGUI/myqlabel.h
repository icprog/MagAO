#ifndef MYQLABEL_H
#define MYQLABEL_H

#include <qlabel.h>

class MyQLabel : public QLabel
{
public:
	MyQLabel( QWidget *parent, const char* name=0, WFlags f=0 );

	void setAutoEraseBkg ( bool AutoErase );
};

#endif
