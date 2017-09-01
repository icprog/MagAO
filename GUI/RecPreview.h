#ifndef RECPREVIEW_H_INCLUDED
#define RECPREVIEW_H_INCLUDED

#include <kpreviewwidgetbase.h>
#include <kurl.h>
#include <qpainter.h>

#include <string>

using namespace std;

extern "C" {
#include "math.h"  // round()
}

class RecPreview: public KPreviewWidgetBase
{
   virtual void showPreview( const KURL& url);
   virtual void clearPreview() {}
   virtual void paintEvent( QPaintEvent *);

public:
   RecPreview( QWidget *parent);

protected:
   int _x1, _x2, _x3, _y1;
   string _binning;
   string _modes;
   string _cut_modes;
   string _pupils;
   string _m2c;
   string _imacq;
   string _mod;

};

#endif // RECPREVIEW_H_INCLUDED



