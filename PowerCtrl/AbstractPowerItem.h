//@File: PowerItem.h
//
// Class for a single power-controlling item.
//
// The class knows how to switch on and off the item power, owns
// a status variable and a request variable in the MsgD-RTBD and knows
// how to answer to power switch requests.
//@

#ifndef ABSTRACT_POWERITEM_H_INCLUDED
#define ABSTRACT_POWERITEM_H_INCLUDED

#include "AOApp.h"
#include "RTDBvar.h"
#include "base/common.h"
#include "stdconfig.h"

#include <string>

enum
{
   PWRSTATUS_OFF =0,
   PWRSTATUS_ON  =1,
   PWRSTATUS_NOTREACHABLE=2
};

class AbstractPowerItem
{
public:
	AbstractPowerItem();
	AbstractPowerItem( AOApp &app, Config_File &cfg);
	virtual ~AbstractPowerItem();

public:
	void HandleRequest( Variable *TheVar);
	//virtual void UpdateStatus( void *ptr)=0;

   void Retry();
   void forceStatus( int status);
   int getStatus() { int stat; var_status.Get(&stat); return stat; }

   std::string name() { return item_name; }

	virtual void SwitchOn() = 0;
    virtual void SwitchOff() = 0;

protected:
	void CreateVars( AOApp &app, std::string prefix, std::string item_code, std::string item_name);
	void DeleteVars();


protected:
	RTDBvar var_status;     // 0=off, 1=on, 2=not reachable
	RTDBvar var_request;
   RTDBvar var_name;
   std::string item_name;
   std::string clientname;
};


#endif // ABSTRACT_POWERITEM_H_INCLUDED

		
