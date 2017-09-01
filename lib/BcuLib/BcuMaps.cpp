#include "BcuLib/BcuMaps.h"
#include "AdSecConstants.h"
#include "WfsConstants.h"
#include "stdconfig.h"
#include <arpa/inet.h>
#include <netdb.h>      // gethostbyname()

using namespace Arcetri;
using namespace Arcetri::Bcu;

// ------------------- BCU_ID_MAP ----------------- //

Bcu::BcuIdMap::BcuIdMap() {
	
	this->insert("BCU_39", WfsConstants::Bcu39::ID);
	this->insert("BCU_47", WfsConstants::Bcu47::ID);
	this->insert("BCU_SWITCH", AdSecConstants::BcuSwitch::ID);
	this->insert("BCU_MIRROR_0", AdSecConstants::BcuMirror::ID);
	this->insert("BCU_MIRROR_1", AdSecConstants::BcuMirror::ID + 1);
	this->insert("BCU_MIRROR_2", AdSecConstants::BcuMirror::ID + 2);
	this->insert("BCU_MIRROR_3", AdSecConstants::BcuMirror::ID + 3);
	this->insert("BCU_MIRROR_4", AdSecConstants::BcuMirror::ID + 4);
	this->insert("BCU_MIRROR_5", AdSecConstants::BcuMirror::ID + 5);
	this->insert("BCU_MIRROR_ALL", 50);
	this->insert("BCU_ALL", 51);
}


int& Bcu::BcuIdMap::operator[](const string& key) throw (BcuIdMapException) {
	iterator it = this->find(key);
	if (it != this->end())
    	return it->second;
	else {
		throw BcuIdMapException("Trying to access a not existing key: '" + key + "'. To add a new element, use insert(string,Conf_Value) instead");
	}
}


bool Bcu::BcuIdMap::insert(string key, int id) {
	bool inserted = insert(make_pair(key,id)).second; 
	return inserted;
}


// ------------------- BCU_MASTER_UDP_PORT_MAP ----------------- //

Bcu::BcuMasterUdpMap::BcuMasterUdpMap(string masterdiagnosticconffile) {
	Config_File masterDiagnosticConfig = Config_File(masterdiagnosticconffile);
	
	this->insert("BCU_39", masterDiagnosticConfig["Bcu39Port"]);
	this->insert("BCU_47", masterDiagnosticConfig["Bcu47Port"]);
	this->insert("BCU_SWITCH",  masterDiagnosticConfig["BcuSwitchPort"]);
	this->insert("BCU_MIRROR_0", masterDiagnosticConfig["BcuMirror0Port"]);
	this->insert("BCU_MIRROR_1", masterDiagnosticConfig["BcuMirror1Port"]);
	this->insert("BCU_MIRROR_2", masterDiagnosticConfig["BcuMirror2Port"]);
	this->insert("BCU_MIRROR_3", masterDiagnosticConfig["BcuMirror3Port"]);
	this->insert("BCU_MIRROR_4", masterDiagnosticConfig["BcuMirror4Port"]);
	this->insert("BCU_MIRROR_5", masterDiagnosticConfig["BcuMirror5Port"]);
}


int& Bcu::BcuMasterUdpMap::operator[](const string& key) throw (BcuUdpMapException) {
	iterator it = this->find(key);
	if (it != this->end())
    	return it->second;
	else {
		throw BcuUdpMapException("Trying to access a not existing key: '" + key + "'. To add a new element, use insert(string,Conf_Value) instead");
	}
}


bool Bcu::BcuMasterUdpMap::insert(string key, int id) {
	bool inserted = insert(make_pair(key,id)).second; 
	return inserted;
}

// ------------------- BCU_IP_PORT_MAP ----------------- //
// Used only in the idl_wrapper and adsec_wrapper from getbcuip.pro

Bcu::BcuIpMap::BcuIpMap() {
	
   string suffix="";
   struct hostent *hostent;

   char *side = getenv("ADOPT_SIDE");
   if (strcmp(side, "R") == 0)
      suffix = "dx";
   else if (strcmp(side, "L") == 0)
      suffix = "sx";

   const char *hostnames[] = { "bcu39", "bcu47", "swbcu", "mbcu0", "mbcu1", "mbcu2", "mbcu3", "mbcu4", "mbcu5" };
   const char *names[] = { "BCU_39", "BCU_47", "BCU_SWITCH", "BCU_MIRROR_0", "BCU_MIRROR_1", "BCU_MIRROR_2",
                     "BCU_MIRROR_3", "BCU_MIRROR_4", "BCU_MIRROR_5" };

   // Fill this[names] with IP addresses
   for (unsigned int i=0; i<sizeof(hostnames)/sizeof(char*); i++)
      if ((hostent = gethostbyname((hostnames[i]+suffix).c_str())) != NULL)
	      this->insert(names[i], string(inet_ntoa(*(struct in_addr*)hostent->h_addr)));

}


string& Bcu::BcuIpMap::operator[](const string& key) throw (BcuIpMapException) {
	iterator it = this->find(key);
	if (it != this->end())
    	return it->second;
	else {
		throw BcuIpMapException("Trying to access a not existing key: '" + key + "'. To add a new element, use insert(string,Conf_Value) instead");
	}
}


bool Bcu::BcuIpMap::insert(string key, string ip) {
	bool inserted = insert(make_pair(key,ip)).second; 
	return inserted;
}

