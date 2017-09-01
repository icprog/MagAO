/* 
 * @File: RTDBvar.h
 * Defines the RTDBvar class.
 * @
 */ 

#ifndef RTDBVAR_H_INCLUDED
#define RTDBVAR_H_INCLUDED

#include <vector>
#include <iostream>

using namespace std;

extern "C" {
#include "base/common.h"
#include "base/thrdlib.h"
}

#include "aotypes.h"
#include "NaNutils.h"


/*
 * @enum{API}: send_actions 
 * Action for RTDB var synchronization with 
 * the repository.
 * @
 */
enum send_actions
{
   CHECK_SEND = -1,
   NO_SEND    =  0,
   FORCE_SEND = 1
};


/*
 * @enum{API}: vardirection 
 * Available var directions.
 * @
 */
enum vardirection
{
   NO_DIR = 0,
   CUR_VAR = 1,
   REQ_VAR = 2
};

/*
   @Function: getStringRTDB

   Return a string extracted from an RTDB variable
   @

string getStringRTDB(Variable *var); //@P{var}: RTDB Variable structure

*/
/*
   @Function: getIntRTDB

   Return a Int value (type: int) extracted from an RTDB variable
   @

int getIntRTDB(Variable *var, int ix=0); //@P{var}: RTDB Variable structure
                                          //@P{ix}: Variable index (default:0)
*/

/*
   @Function: getIntRTDB

   Return a Real value (type: double) extracted from an RTDB variable
   @

double getRealRTDB(Variable *var, int ix=0); //@P{var}: RTDB Variable structure
                                             //@P{ix}: Variable index (default:0)
*/




/* @Class: RTDBvar
 * 
 * Class to manage MsgD-RTDB variables.
 * 
 * This class manages creation, modification and deletion of MsgD-RTDB values.
 * 
 * The class mantains a local copy of the variable contents which is automatically mirrored
 * to the repository (MsgD-RTDB); if necessary, the programmer can configure a different
 * behaviour: in any case the mirroring is one-way, that is from the local copy to MsgD-RTDB.
 * 
 * To update the value of the local object with the current remote value, call the method Update()
 * or use the notification mechanism provided by thrdlib.
 * 
 * The class can have variable ownership or can be attached to a variable owned by other programs.
 * Standard global/private access mechanism apply in all cases.
 */
class RTDBvar {
	
	public:
  
/* 
   @Method{API}: RTDBvar

   Constructor 1. 
  
   Create an empty object variable without any binding with the MsgD-RTDB.
 @
*/
		RTDBvar();
		
/* 
@Method{API}: RTDBvar

   Constructor 2. 

Creates a fully initialized object and (optionally, and if not exisiting) the 
corresponding variable in the MsgD-RTDB. 

If the variable in the MSGD-RTDB already exists, it is attached to local object.

If called with create=false and doesn't exist, the variable in RTDB must be 
created by an explicit call to method Create().
@
*/
/*
		RTDBvar(string fullName, 
              bool global=false, 
              int type=INT_VARIABLE, 
              int len=1, 
              bool create=true) __attribute__ ((deprecated));
*/

		RTDBvar(string fullName,          // @P{fullName}: Complete name of variable
              int type=INT_VARIABLE,    // @P{type}: Variable type (INT_VARIABLE, REAL_VARIABLE, etc.)
              int len=1,                // @P{len}:  Number of elements
              bool create=true);        // @P{create}: Create corresponding RTDB variable
	   
/* 
 @Method{API}: RTDBvar

 Constructor 3. 

 Creates a fully initialized object and (optionally, and if not existing) the 
 corresponding variable in the MsgD-RTDB. 
 If the variable in the MSGD-RTDB already exists, it is attached to local object.
 
 If called with create=false and doesn't exist, the variable in RTDB must be 
 created by an explicit call to method Create().

 Variable name is specified in the form prefix, name. The corresponding name
becomes: prefix.name
 @
*/
/*
   	RTDBvar( string prefix, 
               string name, 
               int dir=NO_DIR, 
               bool global=false,
               int type=INT_VARIABLE, 
               int len=1, 
               bool create=true) __attribute__ ((deprecated));
*/

   	RTDBvar( string prefix,          // Variable name prefix
               string name,            // name
               int dir=NO_DIR,         // Either NO_DIR, CUR_VAR, REQ_VAR
               int type=INT_VARIABLE, 
               int len=1, 
               bool create=true);

		/* 
		 * @Method{API}: RTDBvar
		 * Creates a fully initialized object from an existing variable 
		 * in the MsgD-RTDB.
		 * @
		 */
	   	RTDBvar(Variable *TheVar);
	   	
		/*
		 * @Method{API}: RTDBvar
		 * Copy constructor
		 * @
		 */
	   	RTDBvar(const RTDBvar &from);
	   	
	   	/*
	     * Query methods
	     */
/*                 L.F. removed
	   	string prefix()         { return _prefix; }
	   	string name()           { return _name; }
*/
	   	int    dir()            { return _dir; }
	   	bool   global() __attribute__ ((deprecated)) { return true; }
	   	int    type()           { return _type; }
	   	int    len()            { return _len; }
	   	int    buflen()         { return _len*_bpe; }
	   	string complete_name()  { return _complete_name; }
	   	
	   	/*
	   	 * @Method{API}: ~RTDBvar
		 * Destructor
		 * 
		 * This doesn't delete (see Delete()) the variable in the
		 * MSGD (current policy).
		 * @
	   	 */
	   	 ~RTDBvar();
	   	
	   	/*
	   	 * @Method{API}: operator=
		 * Assignement operator
		 * @
	   	 */
	   	RTDBvar& operator=(const RTDBvar &other);

        /*
         * Create a variable constructed with the flag "create" = false.
         * (see constructors)        
         */
       	void Create();
       	
       	/*
       	 * Update the object, retrieving the value form RTDB var
       	 */
		void Update();

		/*
		 * @Method{API}: Get
		 * Get the first (or the only) array element (from local copy, not from the MsgD-RTDB)
		 * @
		 */
		template <class T> void Get(T *value);
		
		/*
		 * @Method{API}: Get
		 * Get one array element value (from local copy, not from the MsgD-RTDB)
		 * @
		 */
	    template <class T> void Get(int index, T *value);
	    
	    /*
		 * @Method{API}: Get
		 * Get one array element value (Overload for C strings)
		 * @
		 */
	    template <class T> void Get(T **value);
	    
	    /*
		 * @Method{API}: Get
		 * Overload for C++ strings (whole variable array at once)
		 * @
		 */
	    string Get();


		/* 
		 * @Method{API}: MatchAndSet
		 * Checks and updates the variable.
		 * 
		 * Set this variable if the variable given as argument has the same name
		 * and type.
		 * 
		 * Use this function to process MsgD-RTDB notification messages.
		 * Returns one if the variable was set, zero if not, or an error code
		 * if the variable had the right name but couldn't be set (i.e. wrong type)
		 * @
		 */
	   	int MatchAndSet(Variable *TheVar); // Get the variable value if name and type match
	   	
	   	/*
	   	 * @Method{API}: Set
	   	 * Sets this variable
	   	 * 
	   	 * Use this overload to set the local variable value from a MsgD-RTDB message
	   	 * This routine does NOT update the remote value.
	   	 */
	   	void Set(Variable *TheVar);     
  
	  	/*
	  	 * @Method{API}: Set
	  	 * Set one value of the array and optionally send to MsgD-RTDB
	  	 * @
	  	 */
      void Set(float value, int index, send_actions send);

		template <class T> void Set(T value, int index = 0, send_actions send = CHECK_SEND);
		
		/*
	  	 * @Method{API}: Set
	  	 * Overload for C vectors (whole variable array at once)
	  	 * @
	  	 */
		template <class T> void Set(T *value, send_actions send = CHECK_SEND); 
		
		/*
	  	 * @Method{API}: Set
	  	 * Overload for C++ vectors (whole variable array at once)
	  	 * @
	  	 */
		template <class T> void Set(vector<T> &value, send_actions send = CHECK_SEND);   
	
		/*
	  	 * @Method{API}: Set
	  	 * Overload for C++ strings (whole variable array at once)
	  	 * @
	  	 */
		void Set( string value, send_actions send = CHECK_SEND);
	   
	   	/*
	  	 * @Method{API}: Set
	  	 * Wrapper to redirect char * to C++ strings
	  	 * @
	  	 */
	    void Set( char *value, send_actions send = CHECK_SEND);
    
		/*
	  	 * @Method{API}: Send
	  	 * Send the current variable status to MsgD-RTDB
	  	 * @
	  	 */ 
	    void Send();       
    
	    /*
		 * @Method: varname
		 * Returns a properly formatted variable name
		 * @
		 */
		static string varname(string device_type, int direction, string name);
	   
	   	/*
		 * @Method: split_varname
		 * Splits a variable name into its components
		 * An exception is thrown if the variable name is not well-formed
		 * @
		 */
	    static void split_varname(string varname, string &device_type, int &direction, string &name);
	    
	    /*
		 * @Method: strtype
		 * @
		 */
	    static string strtype(int type);


       /*
        * @Method: buffer
        * Internal buffer access
        */
       unsigned char *getBuffer() { return _buffer; }

       /*
        * @Method: mTime()
        * Get last modification time (reference to struct timeval)
        */

       struct timeval &mTime() { return _mTime; }


	protected:

			/*
			 * Functions to check for types, access range, etc...
			 */
		   	void check_type( uint8 value);
		    void check_type( uint16 value);
		    void check_type( uint32 value);
		    void check_type( uint64 value);
        void check_type( long value);
		    void check_type( double value);
		    void check_type( char value);
		    void check_type( string value);
		    void check_type( int value);
		    template<class T> void check_type(vector<T> v) {}

			/*
			 * @Method: check_element
			 * Check if an element index is available
			 * @
			 */
		    void check_element(int wanted_element);
		    
		    /*
			 * @Method: check_len
			 * Check variable length
			 * @
			 */
		    void check_len(int wanted_len);
		    
		    /*
			 * @Method: check_pointer
			 * Check for a valid pointer
			 * @
			 */
		    void check_pointer(void *pointer);
		
			/*
			 * @Method: CreateInternal
			 * @
			 */
		    void CreateInternal();
		    
		    /*
			 * @Method: Update_contents     
			 * Function to update the variable contents copying the specified array
			 * @
			 */
		    void Update_contents( void *value, int send);
		    
		    /*
			 * @Method: Set_internal   
			 * Function to send the variable value to MsgD-RTDB
			 * 
			 * NOTE: send every change to MsgD-RTDB
			 * @
			 */
		    void Set_internal();
		    
		    /*
			 * @Method: Delete    
			 * Delete the internal buffer storage but NOT the corresponding
			 * MsgD-RTDB variable (see note)
			 * 
			 * --- NOTE ---
			 * Removing the RTDB variable cause an wrong behaviour when a constructor 
			 * is called this way:
			 * 
			 * 		RTDBvar myVar = RTDBvar(string prefix,  ...)
			 * 
			 * A temp RTDBvar is created by the constructor, then copied to myVar 
			 * (by the assignement operator), and at the end the temp var is destroyed,
			 * causing the deletion of the var in the MSGD-RTDB !!!
			 * 
			 * Because the vars in the MSGD-RTDB don't have to be removed (current policy), 
			 * this delete is unnecessary! 
			 * An alternative solution (change to policy!) is to disable the assignement operator, 
			 * that probably is unuseful.
			 * @
			 */
		    void Delete();       
		    
		    int TypeToBpe( int type);
		
		    int    _type;
		    int    _bpe;    	// number of bytes necessary to store on element
		    int    _len;    	// number of elements in the variable
		    bool   _own;    	// variable ownership flag
		    int    _dir;
          struct timeval _mTime;
		
			string _complete_name;
	        bool _created;
                pthread_mutex_t _mutex;
		
		    unsigned char *_buffer;          // storage
   
	private:
		
		void CopyConstructor(const RTDBvar &from);   	
		
		/*
		 * Creates the object from a variable existing in the RTDB.
		 * Throw an AOVarException if the variable doesn't exist.
		 */
		void CreateFrom(Variable *TheVar);
   		void CreateFrom(string varname);

      /*
       * Does the actual constructor job, called by the various constructors
       */

        void init(string fullName, int dir, int type, int len, bool create);

};

//@API
//
// Set    set the variable value
//
// Various overloads are defined to set a particular value of the variable array,
// the first value, or all of them from a C array or a STL vector.
//
// Methods are templated to allow for any valid variable type to be passed. The type
// used to pass values must match the one in the variable, or an exception will be thrown.
//
// By default, the methods will send a message to MsgD-RTDB updating the variable value only
// if the value has actually changed from the previous one. To have a message for each Set(),
// regardless of the actual value, set <send> to FORCE_SEND. Set <send> to NO_SEND to avoid
// updating MsgD-RTDB in any case.

template <class T>
void RTDBvar::Set(T value, int index, send_actions send)
{
   bool modified;

   check_type(value);
   check_element(index);

   if (((T *)_buffer)[index] != value)
      modified = true;
   else
      modified = false;

   pthread_mutex_lock(&_mutex);
   try {
       ((T *)_buffer)[index] = value;
   }
   catch (...) {
       pthread_mutex_unlock(&_mutex);
       throw;
   }
   pthread_mutex_unlock(&_mutex);


   if ((send == FORCE_SEND) || ((modified) && (send == CHECK_SEND)))
       Set_internal();
}


template <class T>
void RTDBvar::Set(T *value, send_actions send)
{
    check_pointer((void *)value);
    check_type(value[0]);
    Update_contents( (void *)value, send);
}

template <class T>
void RTDBvar::Set(vector<T> &value, send_actions send)     // Overload for C++ vectors (whole variable array at once)
{
   if (value.size()<1)
       return;

   check_type(value[0]);
   check_len(value.size());
   Update_contents( (void *)&value[0], send);   // This is the correct way to get the buffer address
}

// Special case for float: explicit promotion to double
template <>
void RTDBvar::Set( float value, int index, send_actions send);

template <>
void RTDBvar::Set( float *value, int index, send_actions send);

template <>
void RTDBvar::Set( vector<float> value, int index, send_actions send);

//+API
//
// Get    get a variable value (from the local copy, not from MsgD-RTDB)
//
// Various overloads are defined to get a particular value of the variable array,
// the first value, or all of them to a C array.
//
// Methods are templated to allow for any valid variable type to be passed. The type
// used to pass values must match the one in the variable, or an exception will be thrown.
template <class T>
void RTDBvar::Get(T *value)
{
   Get( 0, value);
}

template <class T>
void RTDBvar::Get( int index, T *value)
{
   check_element(index);
   check_pointer(value);
   check_type(*value);

   pthread_mutex_lock(&_mutex);
   try {
       if (!_buffer)
           return;
       *value = (( T*)_buffer)[index];
   }
   catch (...) {
       pthread_mutex_unlock(&_mutex);
       throw;
   }
   pthread_mutex_unlock(&_mutex);
}

template <class T>
void RTDBvar::Get(T **value)
{
   check_pointer(value);
   check_type((*value)[0]);

   pthread_mutex_lock(&_mutex);
   try {
       if (!_buffer)
           return;
       *value = (T *)_buffer;
   }
   catch (...) {
       pthread_mutex_unlock(&_mutex);
       throw;
   }
   pthread_mutex_unlock(&_mutex);
}

#endif // RTDBVAR_H_INCLUDED

