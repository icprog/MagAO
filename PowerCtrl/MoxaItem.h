//@File: MoxaItem.h
//
// Class for a single power-controlling item.
//
// The class knows how to switch on and off the item power, owns
// a status variable and a request variable in the MsgD-RTBD and knows
// how to answer to power switch requests.
//@

#ifndef MOXAITEM_H_INCLUDED
#define MOXAITEM_H_INCLUDED

#include "AbstractPowerItem.h"

class MoxaItem : public AbstractPowerItem
{
public:
	MoxaItem( AOApp &app, Config_File &cfg);
	~MoxaItem();

public:
   //void UpdateStatus( void *ptr);

	void SwitchOn();
	void SwitchOff();
        int  Status();

protected:
   int num;

   AOApp *app;

   bool checkTT;

};


#endif // MOXAITEM_H_INCLUDED

		
