/*
 * @File idl_4dlib.h
 * 
 * Idl library to handle the 4D PhaseCam
 * @
 */

#ifndef IDL_4DLIB_H_INCLUDE
#define IDL_4DLIB_H_INCLUDE

extern "C"{
	IDL_VPTR idl_4D_init(int lArgc, IDL_VPTR Argv[]);
	IDL_VPTR idl_4D_finalize(int lArgc, IDL_VPTR Argv[]);
	IDL_VPTR idl_4D_setTrigger(int lArgc, IDL_VPTR Argv[]);
	IDL_VPTR idl_4D_setPostProc(int lArgc, IDL_VPTR Argv[]);
	IDL_VPTR idl_4D_getMeasure(int lArgc, IDL_VPTR Argv[]);
	IDL_VPTR idl_4D_getBurst(int lArgc, IDL_VPTR Argv[]);
	IDL_VPTR idl_4D_trigger(int lArgc, IDL_VPTR Argv[]);
	IDL_VPTR idl_4D_postProcess(int lArgc, IDL_VPTR Argv[]);
	IDL_VPTR idl_4D_capture(int lArgc, IDL_VPTR Argv[]);
	IDL_VPTR idl_4D_produce(int lArgc, IDL_VPTR Argv[]);
}


#endif /*IDL_4DLIB_H_INCLUDE*/
