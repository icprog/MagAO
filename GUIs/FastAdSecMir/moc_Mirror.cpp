/****************************************************************************
** Mirror meta object code from reading C++ file 'Mirror.h'
**
** Created: Wed Jun 15 11:30:52 2011
**      by: The Qt MOC ($Id: qt/moc_yacc.cpp   3.3.6   edited Mar 8 17:43 $)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#undef QT_NO_COMPAT
#include "Mirror.h"
#include <qmetaobject.h>
#include <qapplication.h>

#include <private/qucomextra_p.h>
#if !defined(Q_MOC_OUTPUT_REVISION) || (Q_MOC_OUTPUT_REVISION != 26)
#error "This file was generated using the moc from 3.3.6. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

#include <qvariant.h>
const char *Mirror::className() const
{
    return "Mirror";
}

QMetaObject *Mirror::metaObj = 0;
static QMetaObjectCleanUp cleanUp_Mirror( "Mirror", &Mirror::staticMetaObject );

#ifndef QT_NO_TRANSLATION
QString Mirror::tr( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "Mirror", s, c, QApplication::DefaultCodec );
    else
	return QString::fromLatin1( s );
}
#ifndef QT_NO_TRANSLATION_UTF8
QString Mirror::trUtf8( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "Mirror", s, c, QApplication::UnicodeUTF8 );
    else
	return QString::fromUtf8( s );
}
#endif // QT_NO_TRANSLATION_UTF8

#endif // QT_NO_TRANSLATION

QMetaObject* Mirror::staticMetaObject()
{
    if ( metaObj )
	return metaObj;
    QMetaObject* parentObject = QFrame::staticMetaObject();
    static const QUMethod slot_0 = {"setMinMax", 0, 0 };
    static const QUMethod slot_1 = {"setMLimits", 0, 0 };
    static const QUMethod slot_2 = {"mtitle", 0, 0 };
    static const QUMethod slot_3 = {"setColorScale", 0, 0 };
    static const QUParameter param_slot_4[] = {
	{ 0, &static_QUType_int, 0, QUParameter::In }
    };
    static const QUMethod slot_4 = {"setMBrush", 1, param_slot_4 };
    static const QUParameter param_slot_5[] = {
	{ 0, &static_QUType_int, 0, QUParameter::In },
	{ 0, &static_QUType_int, 0, QUParameter::In },
	{ 0, &static_QUType_int, 0, QUParameter::In },
	{ 0, &static_QUType_int, 0, QUParameter::In },
	{ 0, &static_QUType_int, 0, QUParameter::In },
	{ 0, &static_QUType_int, 0, QUParameter::In },
	{ 0, &static_QUType_int, 0, QUParameter::In }
    };
    static const QUMethod slot_5 = {"init", 7, param_slot_5 };
    static const QUMethod slot_6 = {"updateM", 0, 0 };
    static const QUParameter param_slot_7[] = {
	{ 0, &static_QUType_varptr, "\x10", QUParameter::In },
	{ 0, &static_QUType_varptr, "\x13", QUParameter::In },
	{ 0, &static_QUType_int, 0, QUParameter::In }
    };
    static const QUMethod slot_7 = {"fillM", 3, param_slot_7 };
    static const QUMethod slot_8 = {"getLmin", 0, 0 };
    static const QUMethod slot_9 = {"getLmax", 0, 0 };
    static const QUMethod slot_10 = {"setLmin", 0, 0 };
    static const QUMethod slot_11 = {"setLmax", 0, 0 };
    static const QUMethod slot_12 = {"getSliderMin", 0, 0 };
    static const QUMethod slot_13 = {"getSliderMax", 0, 0 };
    static const QUMethod slot_14 = {"setSliderMin", 0, 0 };
    static const QUMethod slot_15 = {"setSliderMax", 0, 0 };
    static const QUMethod slot_16 = {"text2slider", 0, 0 };
    static const QUMethod slot_17 = {"slider2text", 0, 0 };
    static const QUMethod slot_18 = {"fromLChanged", 0, 0 };
    static const QUParameter param_slot_19[] = {
	{ 0, &static_QUType_int, 0, QUParameter::In }
    };
    static const QUMethod slot_19 = {"fromSChanged", 1, param_slot_19 };
    static const QUMethod slot_20 = {"toLChanged", 0, 0 };
    static const QUParameter param_slot_21[] = {
	{ 0, &static_QUType_int, 0, QUParameter::In }
    };
    static const QUMethod slot_21 = {"toSChanged", 1, param_slot_21 };
    static const QUMethod slot_22 = {"setDefaultLim", 0, 0 };
    static const QUMethod slot_23 = {"takeOffset", 0, 0 };
    static const QUMethod slot_24 = {"setOffset", 0, 0 };
    static const QUMethod slot_25 = {"changeColorTable", 0, 0 };
    static const QUMethod slot_26 = {"showUserDefined", 0, 0 };
    static const QUMethod slot_27 = {"hideUserDefined", 0, 0 };
    static const QUMethod slot_28 = {"changeMap", 0, 0 };
    static const QUMethod slot_29 = {"changeMinColor", 0, 0 };
    static const QUMethod slot_30 = {"setMinColor", 0, 0 };
    static const QUMethod slot_31 = {"changeMaxColor", 0, 0 };
    static const QUMethod slot_32 = {"setMaxColor", 0, 0 };
    static const QUParameter param_slot_33[] = {
	{ 0, &static_QUType_QVariant, "\x0a", QUParameter::Out },
	{ 0, &static_QUType_double, 0, QUParameter::In },
	{ 0, &static_QUType_int, 0, QUParameter::In }
    };
    static const QUMethod slot_33 = {"value2color", 3, param_slot_33 };
    static const QUMethod slot_34 = {"languageChange", 0, 0 };
    static const QUParameter param_slot_35[] = {
	{ 0, &static_QUType_ptr, "QMouseEvent", QUParameter::In }
    };
    static const QUMethod slot_35 = {"mouseDoubleClickEvent", 1, param_slot_35 };
    static const QUMethod slot_36 = {"changeMTitle", 0, 0 };
    static const QUParameter param_slot_37[] = {
	{ 0, &static_QUType_int, 0, QUParameter::In }
    };
    static const QUMethod slot_37 = {"helpMTitle", 1, param_slot_37 };
    static const QUParameter param_slot_38[] = {
	{ 0, &static_QUType_int, 0, QUParameter::In }
    };
    static const QUMethod slot_38 = {"helpMTitleClose", 1, param_slot_38 };
    static const QUParameter param_slot_39[] = {
	{ 0, &static_QUType_int, 0, QUParameter::In }
    };
    static const QUMethod slot_39 = {"setMTitle", 1, param_slot_39 };
    static const QUParameter param_slot_40[] = {
	{ 0, &static_QUType_int, 0, QUParameter::In }
    };
    static const QUMethod slot_40 = {"setMap", 1, param_slot_40 };
    static const QUParameter param_slot_41[] = {
	{ 0, &static_QUType_int, 0, QUParameter::In }
    };
    static const QUMethod slot_41 = {"setColTable", 1, param_slot_41 };
    static const QMetaData slot_tbl[] = {
	{ "setMinMax()", &slot_0, QMetaData::Public },
	{ "setMLimits()", &slot_1, QMetaData::Public },
	{ "mtitle()", &slot_2, QMetaData::Public },
	{ "setColorScale()", &slot_3, QMetaData::Public },
	{ "setMBrush(int)", &slot_4, QMetaData::Public },
	{ "init(int,int,int,int,int,int,int)", &slot_5, QMetaData::Public },
	{ "updateM()", &slot_6, QMetaData::Public },
	{ "fillM(int*,double*,int)", &slot_7, QMetaData::Public },
	{ "getLmin()", &slot_8, QMetaData::Public },
	{ "getLmax()", &slot_9, QMetaData::Public },
	{ "setLmin()", &slot_10, QMetaData::Public },
	{ "setLmax()", &slot_11, QMetaData::Public },
	{ "getSliderMin()", &slot_12, QMetaData::Public },
	{ "getSliderMax()", &slot_13, QMetaData::Public },
	{ "setSliderMin()", &slot_14, QMetaData::Public },
	{ "setSliderMax()", &slot_15, QMetaData::Public },
	{ "text2slider()", &slot_16, QMetaData::Public },
	{ "slider2text()", &slot_17, QMetaData::Public },
	{ "fromLChanged()", &slot_18, QMetaData::Public },
	{ "fromSChanged(int)", &slot_19, QMetaData::Public },
	{ "toLChanged()", &slot_20, QMetaData::Public },
	{ "toSChanged(int)", &slot_21, QMetaData::Public },
	{ "setDefaultLim()", &slot_22, QMetaData::Public },
	{ "takeOffset()", &slot_23, QMetaData::Public },
	{ "setOffset()", &slot_24, QMetaData::Public },
	{ "changeColorTable()", &slot_25, QMetaData::Public },
	{ "showUserDefined()", &slot_26, QMetaData::Public },
	{ "hideUserDefined()", &slot_27, QMetaData::Public },
	{ "changeMap()", &slot_28, QMetaData::Public },
	{ "changeMinColor()", &slot_29, QMetaData::Public },
	{ "setMinColor()", &slot_30, QMetaData::Public },
	{ "changeMaxColor()", &slot_31, QMetaData::Public },
	{ "setMaxColor()", &slot_32, QMetaData::Public },
	{ "value2color(double,int)", &slot_33, QMetaData::Public },
	{ "languageChange()", &slot_34, QMetaData::Protected },
	{ "mouseDoubleClickEvent(QMouseEvent*)", &slot_35, QMetaData::Protected },
	{ "changeMTitle()", &slot_36, QMetaData::Protected },
	{ "helpMTitle(int)", &slot_37, QMetaData::Protected },
	{ "helpMTitleClose(int)", &slot_38, QMetaData::Protected },
	{ "setMTitle(int)", &slot_39, QMetaData::Protected },
	{ "setMap(int)", &slot_40, QMetaData::Protected },
	{ "setColTable(int)", &slot_41, QMetaData::Protected }
    };
    metaObj = QMetaObject::new_metaobject(
	"Mirror", parentObject,
	slot_tbl, 42,
	0, 0,
#ifndef QT_NO_PROPERTIES
	0, 0,
	0, 0,
#endif // QT_NO_PROPERTIES
	0, 0 );
    cleanUp_Mirror.setMetaObject( metaObj );
    return metaObj;
}

void* Mirror::qt_cast( const char* clname )
{
    if ( !qstrcmp( clname, "Mirror" ) )
	return this;
    return QFrame::qt_cast( clname );
}

bool Mirror::qt_invoke( int _id, QUObject* _o )
{
    switch ( _id - staticMetaObject()->slotOffset() ) {
    case 0: setMinMax(); break;
    case 1: setMLimits(); break;
    case 2: mtitle(); break;
    case 3: setColorScale(); break;
    case 4: setMBrush((int)static_QUType_int.get(_o+1)); break;
    case 5: init((int)static_QUType_int.get(_o+1),(int)static_QUType_int.get(_o+2),(int)static_QUType_int.get(_o+3),(int)static_QUType_int.get(_o+4),(int)static_QUType_int.get(_o+5),(int)static_QUType_int.get(_o+6),(int)static_QUType_int.get(_o+7)); break;
    case 6: updateM(); break;
    case 7: fillM((int*)static_QUType_varptr.get(_o+1),(double*)static_QUType_varptr.get(_o+2),(int)static_QUType_int.get(_o+3)); break;
    case 8: getLmin(); break;
    case 9: getLmax(); break;
    case 10: setLmin(); break;
    case 11: setLmax(); break;
    case 12: getSliderMin(); break;
    case 13: getSliderMax(); break;
    case 14: setSliderMin(); break;
    case 15: setSliderMax(); break;
    case 16: text2slider(); break;
    case 17: slider2text(); break;
    case 18: fromLChanged(); break;
    case 19: fromSChanged((int)static_QUType_int.get(_o+1)); break;
    case 20: toLChanged(); break;
    case 21: toSChanged((int)static_QUType_int.get(_o+1)); break;
    case 22: setDefaultLim(); break;
    case 23: takeOffset(); break;
    case 24: setOffset(); break;
    case 25: changeColorTable(); break;
    case 26: showUserDefined(); break;
    case 27: hideUserDefined(); break;
    case 28: changeMap(); break;
    case 29: changeMinColor(); break;
    case 30: setMinColor(); break;
    case 31: changeMaxColor(); break;
    case 32: setMaxColor(); break;
    case 33: static_QUType_QVariant.set(_o,QVariant(value2color((double)static_QUType_double.get(_o+1),(int)static_QUType_int.get(_o+2)))); break;
    case 34: languageChange(); break;
    case 35: mouseDoubleClickEvent((QMouseEvent*)static_QUType_ptr.get(_o+1)); break;
    case 36: changeMTitle(); break;
    case 37: helpMTitle((int)static_QUType_int.get(_o+1)); break;
    case 38: helpMTitleClose((int)static_QUType_int.get(_o+1)); break;
    case 39: setMTitle((int)static_QUType_int.get(_o+1)); break;
    case 40: setMap((int)static_QUType_int.get(_o+1)); break;
    case 41: setColTable((int)static_QUType_int.get(_o+1)); break;
    default:
	return QFrame::qt_invoke( _id, _o );
    }
    return TRUE;
}

bool Mirror::qt_emit( int _id, QUObject* _o )
{
    return QFrame::qt_emit(_id,_o);
}
#ifndef QT_NO_PROPERTIES

bool Mirror::qt_property( int id, int f, QVariant* v)
{
    return QFrame::qt_property( id, f, v);
}

bool Mirror::qt_static_property( QObject* , int , int , QVariant* ){ return FALSE; }
#endif // QT_NO_PROPERTIES
