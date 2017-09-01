#include <boost/shared_ptr.hpp>
#include <boost/pool/object_pool.hpp>
#include <string>
#include <iostream>

using namespace boost;
using namespace std;

class Foo {
	
	public:
	
		explicit Foo(string name) { _name = name; cout << "Foo " << _name << " created" << endl; }
		virtual ~Foo() { cout << "Foo " << _name << " destroyed" << endl; }
		
		void setName(string name) { _name = name; }
		
		void print() { cout << "My name is "<< _name << endl; }
	
	private:
		string _name;
};

void allocateFoo() {
	Foo* myFoo = new Foo("AutoPippo"); 
	boost::shared_ptr<Foo> myShFoo(myFoo);
}


int main() {
	Foo* myFoo = new Foo("Pippo"); 
	shared_ptr<Foo> myShFoo(myFoo);
	myShFoo->print();
	boost::shared_ptr<Foo> myShFooCopy = myShFoo;
	myShFooCopy->print();
	
	allocateFoo();
	boost::pool<> intPool(sizeof(int));
	int* myInt = (int*)(intPool.malloc());
	(*myInt) = 32;
	cout << "My int is " << *myInt << endl;
	
//	boost::object_pool<Foo> fooPool(sizeof(Foo));
//	Foo* myFooFromPool = fooPool.malloc(); // Doesn't call the Foo constructor !!! 
//	myFooFromPool->print();				   // Segmentation fault !!!
}
