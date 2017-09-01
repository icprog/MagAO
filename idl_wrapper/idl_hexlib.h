/*
 * @File idl_hexlib.h
 *
 * Idl library to handle the hexapod
 * @
 */

#ifndef IDL_HEXLIB_H_INCLUDE
#define IDL_HEXLIB_H_INCLUDE


extern "C"{
	#include "idl_export.h"
	IDL_VPTR IDL_hexhome(int Argc, IDL_VPTR Argv[], char *Argk);		// TODO ???
	IDL_VPTR IDL_hexgetpos(int Argc, IDL_VPTR Argv[], char *Argk);		// TODO ???
	IDL_VPTR IDL_hexgetstatus(int Argc, IDL_VPTR Argv[], char *Argk);	// TODO ???
	IDL_VPTR IDL_hexmoveto(int Argc, IDL_VPTR Argv[], char *Argk);
	IDL_VPTR IDL_hexmoveby(int Argc, IDL_VPTR Argv[], char *Argk);
	IDL_VPTR IDL_hexmovsph(int Argc, IDL_VPTR Argv[], char *Argk);
	IDL_VPTR IDL_hexbrake(int Argc, IDL_VPTR Argv[], char *Argk);
}


#endif /*IDL_HEXLIB_H_INCLUDE*/
