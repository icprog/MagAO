#include <iostream>
#include <cstdio>
#include "float.h"
using namespace std;

#include "stdconfig.h"


int main() {
	printf("\n*** CONFIG FILE PARSER TEST *** \n");

	Config_File myConf; 
	try {
		myConf = Config_File("conf/left/test_conf_1"); // Change to unexisting to test exception
	}
	catch(Config_File_Exception cfe) {
		terminate();
	}	

	string s;
	int32 i32;
	int64 i64;
	uint32 ui32;
	uint64 ui64; 
	float32 f32;
	float64 f64;
	
	printf("\n");
	printf("Testing reading dictionary...\n");
	printf("Dictionary of total size %d.\n", myConf.size());
	printf("Read some keys:\n");
	
	//Overload of operator[] throw exception when key not found
	s = (string)myConf["par0"];
	i32 = myConf["par1"];
	i64 = myConf["par2"];
	ui32 = myConf["inner1.par5"];
	ui64 = myConf["inner1.par6"];
	f32 = myConf["inner1.inner2.par11"];
	f64 = myConf["inner1.inner2.par12"];
	 
	cout << "string: " << s << "\n";
	cout << "int32: " << i32 << "\n";
	cout << "int64: " << i64 << "\n";
	cout << "uint32: " << ui32 << "\n";
	cout << "uint64: " << ui64 << "\n";
	cout << "float32: " << f32 << "\n"; 
	cout << "float64: " << f64 << "\n";  
	 

	printf("\n*** VALUE_TYPE CLASS TEST *** \n");
	Config_Value v1, v2;
	
	printf("---String---\n");
	v1 = "pippo";
	v2 = "pluto";
	if (v1 == v2) 
		printf("(%s,%s) -> UGUALI!\n", ((string)v1).c_str(), ((string)v2).c_str());
	else 
		printf("(%s,%s) -> DIVERSI!\n", ((string)v1).c_str(), ((string)v2).c_str());
	v1 = "pippo";
	v2 = "pippo";
	if (v1 == v2) 
		printf("(%s,%s) -> UGUALI!\n", ((string)v1).c_str(), ((string)v2).c_str());
	else 
		printf("(%s,%s) -> DIVERSI!\n", ((string)v1).c_str(), ((string)v2).c_str());
	
	printf("---int32---\n");
	v1 = 10;
	v2 = 10;
	if (v1 == v2) 
		printf("(%d,%d) -> UGUALI!\n", (int32)v1, (int32)v2);
	else 
		printf("(%d,%d) -> DIVERSI!\n", (int32)v1, (int32)v2);
	v1 = 10;
	v2 = 14;
	if (v1 == v2) 
		printf("(%d,%d) -> UGUALI!\n", (int32)v1, (int32)v2);
	else 
		printf("(%d,%d) -> DIVERSI!\n", (int32)v1, (int32)v2);
		
	printf("---uint32---\n");	
	v1 = (uint32)30;
	v2 = (uint32)30;
	if (v1 == v2) 
		printf("(%d,%d) -> UGUALI!\n", (uint32)v1, (uint32)v2);
	else 
		printf("(%d,%d) -> DIVERSI!\n", (uint32)v1, (uint32)v2);
	v1 = (uint32)20;
	v2 = (uint32)24;
	if (v1 == v2) 
		printf("(%d,%d) -> UGUALI!\n", (uint32)v1, (uint32)v2);
	else 
		printf("(%d,%d) -> DIVERSI!\n", (uint32)v1, (uint32)v2);
	
	printf("---int64---\n");		
	v1 = (int64)347438647;
	v2 = (int64)347438647;
	if (v1 == v2) 
		printf("(%lld,%lld) -> UGUALI!\n", (int64)v1, (int64)v2);
	else 
		printf("(%lld,%lld) -> DIVERSI!\n", (int64)v1, (int64)v2);
	v1 = (int64)7438647;
	v2 = (int64)474347;
	if (v1 == v2) 
		printf("(%lld,%lld) -> UGUALI!\n", (int64)v1, (int64)v2);
	else 
		printf("(%lld,%lld) -> DIVERSI!\n", (int64)v1, (int64)v2);
	
	printf("---float32---\n");	
	Config_Value v3 = (float32)10.5; //This calls constructor, not assignment !!!
	Config_Value v4 = (float32)10.5; 
	if (v3 == v4) 
		printf("(%f,%f) -> UGUALI!\n", (float32)v3, (float32)v4);
	else 
		printf("(%f,%f) -> DIVERSI!\n", (float32)v3, (float32)v4);
	v3 = (float32)130.5; 
	v4 = (float32)10.4; 
	if (v3 == v4) 
		printf("(%f,%f) -> UGUALI!\n", (float32)v3, (float32)v4);
	else 
		printf("(%f,%f) -> DIVERSI!\n", (float32)v3, (float32)v4);
	
	printf("---float64---\n");	
	v3 = (float64)155650000.5667686; 
	v4 = (float64)155650000.5667686; 
	if (v3 == v4) 
		printf("(%f,%f) -> UGUALI!\n", (float64)v3, (float64)v4);
	else 
		printf("(%f,%f) -> DIVERSI!\n", (float64)v3, (float64)v4);
	v3 = (float64)1556000.5667686; 
	v4 = (float64)355655000.5667686; 
	if (v3 == v4) 
		printf("(%f,%f) -> UGUALI!\n", (float64)v3, (float64)v4);
	else 
		printf("(%f,%f) -> DIVERSI!\n", (float64)v3, (float64)v4);
	
	
}
