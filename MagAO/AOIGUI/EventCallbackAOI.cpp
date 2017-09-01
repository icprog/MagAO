#include <qtextedit.h>
#include <qapplication.h>

#include <tcs/eventhandler/Event.hpp>
#include <tcs/core/SysLog.hpp>

#include "aosgui.uic.hpp"
#include "EventCallbackAOS.hpp"

using namespace lbto;

////////////////////////////////////////////////////////////////////////////////
///  constructor.

EventCallbackAOS::EventCallbackAOS( AOSGUIForm *mainWindow)
{
     mw = mainWindow;
     sideFilter = "left";
}

////////////////////////////////////////////////////////////////////////////////
///  default destructor.

EventCallbackAOS::~EventCallbackAOS()
{
}

////////////////////////////////////////////////////////////////////////////////
///  update the text box to show what event was received.

string EventCallbackAOS::eventHandle( string eventData )
{
    lbto::Event event;
	  
    if(event.setEvent(eventData))
    {
	if (event.getParameterValue("side") == sideFilter)
	{
	    string line;
	    switch(event.getPriority())
	    {
	    case 1:
		line = "<font color=#ff0000>";
		break;
	    case 2:
		line = "<font color=#ffff00>";
		break;
	    case 3:
		line = "<font color=#64ff64>";
		break;
	    case 4:
		line = "<font color=#55ffff>";
		break;
	    case 5:
	    default:
		line = "<font color=#ffffff>";
		break;
	    }
	    line += event.getDate() + " " + event.getName() + " - " + event.getConvertedDescription() + "</font>"; 
//	    printf("Writing event: %s\n", line.c_str());
	    AOSAppendEvent *pevent = new AOSAppendEvent(QString(line));
	    QApplication::postEvent(mw, pevent);
	}
    }
    return string ("Ok");
}

////////////////////////////////////////////////////////////////////////////////
