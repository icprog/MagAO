//@File: Bcu39Item.cpp
//
// Class for a single power-controlling item.
//
// The class knows how to switch on and off the item power, owns
// a status variable and a request variable in the MsgD-RTBD and knows 
// how to answer to power switch requests.
//@

#include <stdio.h>
#include <string.h>
#include <memory.h>


extern "C" {
#include "aotypes.h"
#include "commlib.h"
#include "base/errlib.h"
}

#include "stdconfig.h"
#include "BcuLib/bcucommand.h"

#include "Bcu39Item.h"


//@Bcu39Item (default constructor)
//
//@

Bcu39Item::Bcu39Item( AOApp &app, Config_File &cfg) : AbstractPowerItem( app, cfg)
{
   bit_pos   = cfg["bitnum"];
   bcunum    = cfg["bcunum"];
	seqnum=0;
   clientname = app.MyFullName();
}

//@~Bcu39Item
//
// Standard deconstructor
//@

Bcu39Item::~Bcu39Item()
{
}

void Bcu39Item::SwitchOn()
{
	uint32 buffer[2];

	buffer[0] = RELAY_OR;
	buffer[1] = 0x01 << bit_pos;

   Logger::get()->log( Logger::LOG_LEV_INFO, "Switching ON %s", item_name.c_str());


   Logger::get()->log( Logger::LOG_LEV_TRACE, "Writing 0x%08X to RELAY 0x%08X (OR action)\n", buffer[1], 0);
   thSendBCUcommand( (char *)clientname.c_str(), bcunum, BCU_DSP, BCU_DSP, MGP_OP_WRRD_RELAIS_BOARD,
            0, (unsigned char *)buffer, sizeof(uint32)*2, 1000, seqnum++, 0);
	var_status.Set( (int)PWRSTATUS_ON);
}

void Bcu39Item::SwitchOff()
{
	uint32 buffer[2];

	buffer[0] = RELAY_AND;
	buffer[1] = ~(0x01 << bit_pos);

   Logger::get()->log( Logger::LOG_LEV_INFO, "Switching OFF %s", item_name.c_str());

   Logger::get()->log( Logger::LOG_LEV_TRACE, "Writing 0x%08X to RELAY 0x%08X (AND action)\n", buffer[1], 0);
   thSendBCUcommand( (char *)clientname.c_str(), bcunum, BCU_DSP, BCU_DSP, MGP_OP_WRRD_RELAIS_BOARD,
            0, (unsigned char *)buffer, sizeof(uint32)*2, 1000, seqnum++, 0); 
	var_status.Set( (int)PWRSTATUS_OFF);
}



