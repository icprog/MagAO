
extern "C"{
	#include "base/thrdlib.h"
}

#include "aotypes.h"

#include <string>
#include <iostream>
using namespace std;

class Base {

	public:
		
		Base(int value, string name) { 
			_value = value; 
			_name = name;
		}
		
		void print() { cout << _name << ":" << _value << endl; }
	
	private:
		
		int _value;
		string _name;
	
};

class DerivedUsingBufInfo: public Base {
	
	public:
	
		DerivedUsingBufInfo(int value, string name):Base(value, name) { 
			_value2 = value + 1; 
			_name2 = name + "2";
		}
		
		void print() { 
			Base::print();
			cout << _name2 << ":" << _value2 << endl; 
		}
	
	private:
	
		BufInfo _shmBufInfo; 	
		int _value2;
		string _name2;
};

int main() {

	DerivedUsingBufInfo dObj(10, "pippo");
	dObj.print();
	
	int x = 1;
	uint8 x8 = 1;
	printf("%d\n", x);
	printf("%d\n", x8);
	if(x && x8) {
		printf("UGUALI !!!\n");
	}
	

}
