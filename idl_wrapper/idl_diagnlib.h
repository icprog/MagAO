/*
 * @File idl_diagnlib.h
 *
 * Idl library to request diagnostic tasks, including:
 * 	- DiagnApp (and derived classes)
 * 	- MasterDiagnostic
 * @
 */

#ifndef IDL_DIAGNLIB_H_INCLUDE
#define IDL_DIAGNLIB_H_INCLUDE

extern "C"{
	IDL_VPTR IDLGetDiagnParam(int Argc, IDL_VPTR Argv[], char *Argk);
	IDL_VPTR IDLSetDiagnParam(int Argc, IDL_VPTR Argv[], char *Argk);
	IDL_VPTR IDLGetDiagnValue(int Argc, IDL_VPTR Argv[], char *Argk);
	IDL_VPTR IDLGetDiagnBuffer(int Argc, IDL_VPTR Argv[], char *Argk);
	IDL_VPTR IDLDumpDiagnParams(int Argc, IDL_VPTR Argv[], char *Argk);

	IDL_VPTR IDLMastdiagnDumpHistory(int Argc, IDL_VPTR Argv[]);
	IDL_VPTR IDLLoadOfflineDiagnFrames(int Argc, IDL_VPTR Argv[]);
}


#endif /*IDL_DIAGNLIB_H_INCLUDE*/
