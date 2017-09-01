#include "NaNutils.h"

class MyQString: public QString
{
public:
    void setValue(double value, char f='g', int prec=6) {
        if(Arcetri::NaNutils::isNaN(value))
            QString::setAscii("");
        else
            QString::setNum(value,f,prec);
    }
    void setValue(float value, char f='g', int prec=6) {
        if(Arcetri::NaNutils::isNaN(value))
            QString::setAscii("");
        else
            QString::setNum(value,f,prec);
    }
    void setValue(int value) {
        if(Arcetri::NaNutils::isNaN(value))
            QString::setAscii("");
        else
            QString::setNum(value);
    }
    void setValue(std::string value) {
        QString::setAscii(value.c_str());
    }
};

