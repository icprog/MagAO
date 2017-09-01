/****************************************************************************
** PlotModes meta object code from reading C++ file 'PlotModes.h'
**
** Created: Wed Jun 15 11:30:53 2011
**      by: The Qt MOC ($Id: qt/moc_yacc.cpp   3.3.6   edited Mar 8 17:43 $)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#undef QT_NO_COMPAT
#include "PlotModes.h"
#include <qmetaobject.h>
#include <qapplication.h>

#include <private/qucomextra_p.h>
#if !defined(Q_MOC_OUTPUT_REVISION) || (Q_MOC_OUTPUT_REVISION != 26)
#error "This file was generated using the moc from 3.3.6. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

const char *PlotModes::className() const
{
    return "PlotModes";
}

QMetaObject *PlotModes::metaObj = 0;
static QMetaObjectCleanUp cleanUp_PlotModes( "PlotModes", &PlotModes::staticMetaObject );

#ifndef QT_NO_TRANSLATION
QString PlotModes::tr( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "PlotModes", s, c, QApplication::DefaultCodec );
    else
	return QString::fromLatin1( s );
}
#ifndef QT_NO_TRANSLATION_UTF8
QString PlotModes::trUtf8( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "PlotModes", s, c, QApplication::UnicodeUTF8 );
    else
	return QString::fromUtf8( s );
}
#endif // QT_NO_TRANSLATION_UTF8

#endif // QT_NO_TRANSLATION

QMetaObject* PlotModes::staticMetaObject()
{
    if ( metaObj )
	return metaObj;
    QMetaObject* parentObject = QDialog::staticMetaObject();
    static const QUParameter param_slot_0[] = {
	{ 0, &static_QUType_bool, 0, QUParameter::In }
    };
    static const QUMethod slot_0 = {"xlog_toggled", 1, param_slot_0 };
    static const QUParameter param_slot_1[] = {
	{ 0, &static_QUType_bool, 0, QUParameter::In }
    };
    static const QUMethod slot_1 = {"ylog_toggled", 1, param_slot_1 };
    static const QUMethod slot_2 = {"quit", 0, 0 };
    static const QUMethod slot_3 = {"StopStart", 0, 0 };
    static const QUMethod slot_4 = {"languageChange", 0, 0 };
    static const QMetaData slot_tbl[] = {
	{ "xlog_toggled(bool)", &slot_0, QMetaData::Public },
	{ "ylog_toggled(bool)", &slot_1, QMetaData::Public },
	{ "quit()", &slot_2, QMetaData::Protected },
	{ "StopStart()", &slot_3, QMetaData::Protected },
	{ "languageChange()", &slot_4, QMetaData::Protected }
    };
    static const QUParameter param_signal_0[] = {
	{ 0, &static_QUType_int, 0, QUParameter::In }
    };
    static const QUMethod signal_0 = {"removeMyPlot", 1, param_signal_0 };
    static const QMetaData signal_tbl[] = {
	{ "removeMyPlot(int)", &signal_0, QMetaData::Public }
    };
    metaObj = QMetaObject::new_metaobject(
	"PlotModes", parentObject,
	slot_tbl, 5,
	signal_tbl, 1,
#ifndef QT_NO_PROPERTIES
	0, 0,
	0, 0,
#endif // QT_NO_PROPERTIES
	0, 0 );
    cleanUp_PlotModes.setMetaObject( metaObj );
    return metaObj;
}

void* PlotModes::qt_cast( const char* clname )
{
    if ( !qstrcmp( clname, "PlotModes" ) )
	return this;
    return QDialog::qt_cast( clname );
}

// SIGNAL removeMyPlot
void PlotModes::removeMyPlot( int t0 )
{
    activate_signal( staticMetaObject()->signalOffset() + 0, t0 );
}

bool PlotModes::qt_invoke( int _id, QUObject* _o )
{
    switch ( _id - staticMetaObject()->slotOffset() ) {
    case 0: xlog_toggled((bool)static_QUType_bool.get(_o+1)); break;
    case 1: ylog_toggled((bool)static_QUType_bool.get(_o+1)); break;
    case 2: quit(); break;
    case 3: StopStart(); break;
    case 4: languageChange(); break;
    default:
	return QDialog::qt_invoke( _id, _o );
    }
    return TRUE;
}

bool PlotModes::qt_emit( int _id, QUObject* _o )
{
    switch ( _id - staticMetaObject()->signalOffset() ) {
    case 0: removeMyPlot((int)static_QUType_int.get(_o+1)); break;
    default:
	return QDialog::qt_emit(_id,_o);
    }
    return TRUE;
}
#ifndef QT_NO_PROPERTIES

bool PlotModes::qt_property( int id, int f, QVariant* v)
{
    return QDialog::qt_property( id, f, v);
}

bool PlotModes::qt_static_property( QObject* , int , int , QVariant* ){ return FALSE; }
#endif // QT_NO_PROPERTIES
