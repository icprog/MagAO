/****************************************************************************
** GUIadSecM meta object code from reading C++ file 'FastAdSecMir_gui.h'
**
** Created by Qt MOC ($Id: qt/moc_yacc.cpp   3.3.6)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#undef QT_NO_COMPAT
#include "FastAdSecMir_gui.h"
#include <qmetaobject.h>
#include <qapplication.h>

#include <private/qucomextra_p.h>
#if !defined(Q_MOC_OUTPUT_REVISION) || (Q_MOC_OUTPUT_REVISION != 26)
#error "This file was generated using the moc from 3.3.6. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

const char *GUIadSecM::className() const
{
    return "GUIadSecM";
}

QMetaObject *GUIadSecM::metaObj = 0;
static QMetaObjectCleanUp cleanUp_GUIadSecM( "GUIadSecM", &GUIadSecM::staticMetaObject );

#ifndef QT_NO_TRANSLATION
QString GUIadSecM::tr( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "GUIadSecM", s, c, QApplication::DefaultCodec );
    else
	return QString::fromLatin1( s );
}
#ifndef QT_NO_TRANSLATION_UTF8
QString GUIadSecM::trUtf8( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "GUIadSecM", s, c, QApplication::UnicodeUTF8 );
    else
	return QString::fromUtf8( s );
}
#endif // QT_NO_TRANSLATION_UTF8

#endif // QT_NO_TRANSLATION

QMetaObject* GUIadSecM::staticMetaObject()
{
    if ( metaObj )
	return metaObj;
    QMetaObject* parentObject = QDialog::staticMetaObject();
    static const QUMethod slot_0 = {"Run", 0, 0 };
    static const QUMethod slot_1 = {"quit", 0, 0 };
    static const QUMethod slot_2 = {"StopStartTimer", 0, 0 };
    static const QUMethod slot_3 = {"TakeOffset", 0, 0 };
    static const QUMethod slot_4 = {"RefreshRate", 0, 0 };
    static const QUMethod slot_5 = {"Refresh", 0, 0 };
    static const QUMethod slot_6 = {"GetAll", 0, 0 };
    static const QUMethod slot_7 = {"SetAll", 0, 0 };
    static const QUMethod slot_8 = {"RunningMean", 0, 0 };
    static const QUMethod slot_9 = {"changeNsamples", 0, 0 };
    static const QUMethod slot_10 = {"GetAllDummy", 0, 0 };
    static const QUMethod slot_11 = {"CreatePlotModes", 0, 0 };
    static const QUMethod slot_12 = {"CreatePlotModesMean", 0, 0 };
    static const QUMethod slot_13 = {"CreatePlotModesRMS", 0, 0 };
    static const QUParameter param_slot_14[] = {
	{ 0, &static_QUType_charstar, 0, QUParameter::In },
	{ 0, &static_QUType_int, 0, QUParameter::In }
    };
    static const QUMethod slot_14 = {"newPlot", 2, param_slot_14 };
    static const QUParameter param_slot_15[] = {
	{ 0, &static_QUType_ptr, "PlotModes", QUParameter::In }
    };
    static const QUMethod slot_15 = {"fillPlot", 1, param_slot_15 };
    static const QUParameter param_slot_16[] = {
	{ 0, &static_QUType_int, 0, QUParameter::In }
    };
    static const QUMethod slot_16 = {"remove_plot", 1, param_slot_16 };
    static const QUMethod slot_17 = {"refreshPlots", 0, 0 };
    static const QUMethod slot_18 = {"languageChange", 0, 0 };
    static const QMetaData slot_tbl[] = {
	{ "Run()", &slot_0, QMetaData::Public },
	{ "quit()", &slot_1, QMetaData::Public },
	{ "StopStartTimer()", &slot_2, QMetaData::Public },
	{ "TakeOffset()", &slot_3, QMetaData::Public },
	{ "RefreshRate()", &slot_4, QMetaData::Public },
	{ "Refresh()", &slot_5, QMetaData::Public },
	{ "GetAll()", &slot_6, QMetaData::Public },
	{ "SetAll()", &slot_7, QMetaData::Public },
	{ "RunningMean()", &slot_8, QMetaData::Public },
	{ "changeNsamples()", &slot_9, QMetaData::Public },
	{ "GetAllDummy()", &slot_10, QMetaData::Public },
	{ "CreatePlotModes()", &slot_11, QMetaData::Public },
	{ "CreatePlotModesMean()", &slot_12, QMetaData::Public },
	{ "CreatePlotModesRMS()", &slot_13, QMetaData::Public },
	{ "newPlot(char*,int)", &slot_14, QMetaData::Public },
	{ "fillPlot(PlotModes*)", &slot_15, QMetaData::Public },
	{ "remove_plot(int)", &slot_16, QMetaData::Public },
	{ "refreshPlots()", &slot_17, QMetaData::Public },
	{ "languageChange()", &slot_18, QMetaData::Protected }
    };
    metaObj = QMetaObject::new_metaobject(
	"GUIadSecM", parentObject,
	slot_tbl, 19,
	0, 0,
#ifndef QT_NO_PROPERTIES
	0, 0,
	0, 0,
#endif // QT_NO_PROPERTIES
	0, 0 );
    cleanUp_GUIadSecM.setMetaObject( metaObj );
    return metaObj;
}

void* GUIadSecM::qt_cast( const char* clname )
{
    if ( !qstrcmp( clname, "GUIadSecM" ) )
	return this;
    if ( !qstrcmp( clname, "AOApp" ) )
	return (AOApp*)this;
    return QDialog::qt_cast( clname );
}

bool GUIadSecM::qt_invoke( int _id, QUObject* _o )
{
    switch ( _id - staticMetaObject()->slotOffset() ) {
    case 0: Run(); break;
    case 1: quit(); break;
    case 2: StopStartTimer(); break;
    case 3: TakeOffset(); break;
    case 4: RefreshRate(); break;
    case 5: Refresh(); break;
    case 6: GetAll(); break;
    case 7: SetAll(); break;
    case 8: RunningMean(); break;
    case 9: changeNsamples(); break;
    case 10: GetAllDummy(); break;
    case 11: CreatePlotModes(); break;
    case 12: CreatePlotModesMean(); break;
    case 13: CreatePlotModesRMS(); break;
    case 14: newPlot((char*)static_QUType_charstar.get(_o+1),(int)static_QUType_int.get(_o+2)); break;
    case 15: fillPlot((PlotModes*)static_QUType_ptr.get(_o+1)); break;
    case 16: remove_plot((int)static_QUType_int.get(_o+1)); break;
    case 17: refreshPlots(); break;
    case 18: languageChange(); break;
    default:
	return QDialog::qt_invoke( _id, _o );
    }
    return TRUE;
}

bool GUIadSecM::qt_emit( int _id, QUObject* _o )
{
    return QDialog::qt_emit(_id,_o);
}
#ifndef QT_NO_PROPERTIES

bool GUIadSecM::qt_property( int id, int f, QVariant* v)
{
    return QDialog::qt_property( id, f, v);
}

bool GUIadSecM::qt_static_property( QObject* , int , int , QVariant* ){ return FALSE; }
#endif // QT_NO_PROPERTIES
