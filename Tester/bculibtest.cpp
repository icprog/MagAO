#include <iostream>
#include <cstdio>
#include "BcuLib/BcuCommon.h"
#include "stdconfig.h"
#include "Utils.h"


#define N_HOSTS 9
#define N_BCUIDS 11


int main(int argc, char *argv[])
{
    unsigned int i;

   const char *ip_names[N_HOSTS] = { "BCU_39", "BCU_47", "BCU_SWITCH", "BCU_MIRROR_0", "BCU_MIRROR_1", "BCU_MIRROR_2",
			    "BCU_MIRROR_3", "BCU_MIRROR_4", "BCU_MIRROR_5" };

   const char *id_names[N_BCUIDS] = {	"BCU_39", "BCU_47", "BCU_SWITCH", "BCU_MIRROR_0", "BCU_MIRROR_1", "BCU_MIRROR_2",
				"BCU_MIRROR_3", "BCU_MIRROR_4", "BCU_MIRROR_5", "BCU_MIRROR_ALL", "BCU_ALL" };

    static BcuIpMap _bcuIpMap;
    static BcuIdMap _bcuIdMap;
    string bcuIp;
    int bcuId;

    // get ips
    printf("\nGetting IPs:\n");
    for (i=0; i<N_HOSTS; i++) {
	try {
	    bcuIp = _bcuIpMap[ip_names[i]];
	    printf("IP for %s is %s\n", ip_names[i], bcuIp.c_str());
	}
	catch (BcuIpMapException& e) {
	    printf("IP for %s not defined\n", ip_names[i]);
	}
    }	  
    
    // get ids
    printf("\nGetting IDs:\n");
    for (i=0; i<N_BCUIDS; i++) {
	try {
	    bcuId = _bcuIdMap[id_names[i]];
	    printf("ID for %s is %d\n", id_names[i], bcuId);
	}
	catch (BcuIdMapException& e) {
	    printf("ID for %s not defined\n", id_names[i]);
	}
    }	    
	
    

    return 0;
}
