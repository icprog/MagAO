/*
 * @File idl_wraplib.h
 * 
 * Idl mix library wrapper.
 * 
 * Include a set of routines callable from IDL, defined with
 * the method of DLM. Look at idl_wraplib.dlm to know the
 * callable routines.
 * @
 */

#ifndef IDL_WRAPLIB_H_INCLUDE
#define IDL_WRAPLIB_H_INCLUDE


extern "C"{
	
	/*
	 * Used to load IDL callable functions defined by the DLM file.
	 * To know the callable functions, see idl_wraplib.dlm
	 */
	int IDL_Load(void);
	
}


#endif /*IDL_WRAPLIB_H_INCLUDE*/
