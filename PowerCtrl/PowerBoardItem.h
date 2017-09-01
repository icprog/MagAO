//@File: PowerBoardItem.h
//
// Class for a single power-controlling item.
//
// The class knows how to switch on and off the item power, owns
// a status variable and a request variable in the MsgD-RTBD and knows
// how to answer to power switch requests.
//@

#ifndef POWERITEMPOWERBOARD_H_INCLUDED
#define POWERITEMPOWERBOARD_H_INCLUDED

#include "AbstractPowerItem.h"

class PowerBoardItem : public AbstractPowerItem
{
public:
	PowerBoardItem( AOApp *app, Config_File &cfg);
	~PowerBoardItem();

public:
   //void UpdateStatus( void *ptr);

	void SwitchOn();
	void SwitchOff();

protected:

   std::string ProcessEscapes( std::string str);

protected:
   std::string cmd_on;
   std::string cmd_off;
   std::string cmd_reply_on;
   std::string cmd_reply_off;

   AOApp *app;

};


#endif // POWERITEMPOWERBOARD_H_INCLUDED

		
