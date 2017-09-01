#define VERS_MAJOR   1
#define VERS_MINOR   0

#include <iostream>
#include <sstream>

extern "C" {
#include "iolib.h" //ReadBinaryFile
}


#include "RecPreview.h"


void RecPreview::showPreview( const KURL& url)
{
      char buf[256];
      ostringstream oss;
      string path = url.path().latin1();

      ReadFitsKeyword( (char*)path.c_str(), "BINNING", TSTRING, buf);
      _binning = buf;

      ReadFitsKeyword( (char*)path.c_str(), "IM_MODES", TSTRING, buf);
      oss.str("");
      oss << atoi(buf);
      _modes = oss.str();

      ReadFitsKeyword( (char*)path.c_str(), "CT_MODES", TSTRING, buf);
      _cut_modes = buf;

      ReadFitsKeyword( (char*)path.c_str(), "PUPILS", TSTRING, buf);
      _pupils = buf;

      ReadFitsKeyword( (char*)path.c_str(), "M2C", TSTRING, buf);
      _m2c = buf;

      ReadFitsKeyword( (char*)path.c_str(), "IM_ACQ_D", TSTRING, buf);
      _imacq = buf;
      size_t pos = _imacq.find("intmatAcq");
      if (pos != string::npos)
          _imacq = _imacq.substr(pos+10, 15);
      else
          _imacq = "";

      ReadFitsKeyword( (char*)path.c_str(), "TT.AMPLITUDE", TSTRING, buf);
      oss.str("");
      oss.precision(2);
      oss << std::fixed << atof(buf)*1000;
      _mod = oss.str();
      update();
   }

     
void RecPreview::paintEvent( QPaintEvent *)
{

      QPainter painter(this);
      painter.drawText(_x1, _y1+0*fontMetrics().height(), "Binning");
      painter.drawText(_x1, _y1+1*fontMetrics().height(), "# modes");
      painter.drawText(_x1, _y1+2*fontMetrics().height(), "Modulation");
      painter.drawText(_x1, _y1+3*fontMetrics().height(), "Cut modes");
      painter.drawText(_x1, _y1+4*fontMetrics().height(), "M2C");
      painter.drawText(_x1, _y1+5*fontMetrics().height(), "IM acq");
      painter.drawText(_x1, _y1+6*fontMetrics().height(), "Pupils");

      painter.drawText(_x2, _y1+0*fontMetrics().height(), _binning);
      painter.drawText(_x2, _y1+1*fontMetrics().height(), _modes);
      painter.drawText(_x2, _y1+2*fontMetrics().height(), _mod);
      painter.drawText(_x2, _y1+3*fontMetrics().height(), _cut_modes);
      painter.drawText(_x2, _y1+4*fontMetrics().height(), _m2c);
      painter.drawText(_x2, _y1+5*fontMetrics().height(), _imacq);
      painter.drawText(_x2, _y1+6*fontMetrics().height(), _pupils);
} 
       
RecPreview::RecPreview( QWidget *parent) : KPreviewWidgetBase( parent)
{
      _x1 = 10;
      _x2 = _x1 + fontMetrics().width("Modulation ");
      _x3 = _x2 + fontMetrics().boundingRect("/bin1/00000000-000000").width();
      _y1 = 10;

      setMinimumWidth( _x3);
      _binning = "";
      _modes = "";
      _cut_modes = "";
      _pupils = "";
      _m2c = "";
      _mod = "";
      _imacq = "";
}




