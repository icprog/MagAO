//@File: Bcu39Item.h
//
// Class for a single power-controlling item.
//
// The class knows how to switch on and off the item power, owns
// a status variable and a request variable in the MsgD-RTBD and knows
// how to answer to power switch requests.
//@

#ifndef POWERITEMBCU39_H_INCLUDED
#define POWERITEMBCU39_H_INCLUDED

#include "AbstractPowerItem.h"

class Bcu39Item : public AbstractPowerItem
{
public:
	Bcu39Item( AOApp &app, Config_File &cfg);
	~Bcu39Item();

public:
   //void UpdateStatus( void *ptr);

	void SwitchOn();
	void SwitchOff();

protected:

protected:
	int bit_pos;
	int bcunum;
	int seqnum;
};

#define RELAY_NONE 0x00
#define RELAY_SET  0x01
#define RELAY_OR   0x02
#define RELAY_AND  0x03

#endif // POWERITEMBCU39_H_INCLUDED

		
