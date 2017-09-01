#include <string>
#include <iostream>

using namespace std;


class Base {

	public:
		Base() { cout << "Base constructor" << endl; }
		virtual ~Base() { cout << "Base destructor" << endl; };
};

class Derived: public Base {

	public:
		Derived() throw(int) { cout << "Derived constructor" << endl; }
		virtual ~Derived() { 
			cout << "Derived destructor throwing an exception..." << endl; 
			throw 0;
		};
};


int main() {

	try {
		Derived d;
	}
	catch(int& e) {
		cout << "Exception occurred in main !!!" << endl;
	}

}
