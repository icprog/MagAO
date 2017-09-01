#ifndef BCUMAPS_H_INCLUDE
#define BCUMAPS_H_INCLUDE

#include <map>

#include "AOExcept.h"

using namespace std;

namespace Arcetri {

	namespace Bcu {
		
		/*
		 * @Class{API}: BcuIdMapException
		 * Defines an exception for wrong BcuIdMap access.
		 * @
		 */
		class BcuIdMapException: public AOException {
			
			public:
				explicit BcuIdMapException(string m): AOException(m) { exception_id = "BcuIdMapException"; }
				virtual ~BcuIdMapException() throw() {}
		};
		
		/*
		 * @Class{API}: BcuUdpMapException
		 * Defines an exception for wrong BcuUdpMap access.
		 * @
		 */
		class BcuUdpMapException: public AOException {
			
			public:
				explicit BcuUdpMapException(string m): AOException(m) { exception_id = "BcuUdpMapException"; }
				virtual ~BcuUdpMapException() throw() {}
		};
		
		/*
		 * @Class{API}: BcuIpMapException
		 * Defines an exception for wrong BcuIpMap access.
		 * @
		 */
		class BcuIpMapException: public AOException {
			
			public:
				explicit BcuIpMapException(string m): AOException(m) { exception_id = "BcuIpMapException"; }
				virtual ~BcuIpMapException() throw() {}
		};
		
		
	
		/*
		 * @Class{API}: BcuIdMap
		 * Retrieve a Bcu id by name.
		 * 
		 * Defined Bcu names: BCU_39, BCU_47, BCU_MIRROR_0, ..., BCU_MIRROR_5,
		 * Defined special names for broadcast: BCU_ALL, BCU_MIRROR_ALL
		 * 
		 * Use this to obtain Bcu id when calling:
		 *  - CommandSender::sendMultiBcuCommand(..., firstBcu, lastBcu,...)
		 *  - CommandSender::sendMultiBcuCommand(..., bcuId, ...)
		 */
		class BcuIdMap: public map<string, int> {
			
			public:
			
				BcuIdMap();
			
				//@Member{API}: operator[]
				//
				// Return the element corresponding to given key, or raise an exception 
				// if key not found.
				//
				// ATTENTION: use it only when extracting an element ! When 
				// inserting an element you have to use use insert(key,value)
				//@
				int& operator[](const string& key) throw (BcuIdMapException);		
				
				//@Member{API}: Insert
				//
				// Utility overload to replace operator[] in the left side of assignment.
				// Return true only if element have been inserted, that is if it wasn't 
				// already present
				//@
				bool insert(string key, int id);
				using map<string, int>::insert; //Guarantee visibility for base 
												//class insert methods
		};
		
		
		/*
		 * @Class{API}: BcuMasterUdpMap
		 * Retrieve a Bcu UDP port by name.
		 * 
		 * Defined Bcu names: BCU_39, BCU_47, BCU_MIRROR_0, ..., BCU_MIRROR_5,
		 */
		class BcuMasterUdpMap: public map<string, int> {
			
			public:
			
				BcuMasterUdpMap(string masterdiagnosticconffile);
			
				//@Member{API}: operator[]
				//
				// Return the element corresponding to given key, or raise an exception 
				// if key not found.
				//
				// ATTENTION: use it only when extracting an element ! When 
				// inserting an element you have to use use insert(key,value)
				//@
				int& operator[](const string& key) throw (BcuUdpMapException);		
				
				//@Member{API}: Insert
				//
				// Utility overload to replace operator[] in the left side of assignment.
				// Return true only if element have been inserted, that is if it wasn't 
				// already present
				//@
				bool insert(string key, int port);
				using map<string, int>::insert; //Guarantee visibility for base 
												//class insert methods
		};
		
		/*
		 * @Class{API}: BcuIpMap
		 * Retrieve a Bcu Ip by name.
		 * 
		 * Defined Bcu names: BCU_39, BCU_47, BCU_MIRROR_0, ..., BCU_MIRROR_5,
		 */
		class BcuIpMap: public map<string, string> {
			
			public:
			
				BcuIpMap();
			
				//@Member{API}: operator[]
				//
				// Return the element corresponding to given key, or raise an exception 
				// if key not found.
				//
				// ATTENTION: use it only when extracting an element ! When 
				// inserting an element you have to use use insert(key,value)
				//@
				string& operator[](const string& key) throw (BcuIpMapException);		
				
				//@Member{API}: Insert
				//
				// Utility overload to replace operator[] in the left side of assignment.
				// Return true only if element have been inserted, that is if it wasn't 
				// already present
				//@
				bool insert(string key, string ip);
				using map<string, string>::insert; //Guarantee visibility for base 
												//class insert methods
		};
	
	}
	
}

#endif /*BCUMAPS_H_INCLUDE*/
