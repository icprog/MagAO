#include <unistd.h>
#include <stdio.h>

extern "C"{
	#include "idl_export.h"
}

extern "C"{
	int IDL_Load(void);
}

#define NULL_VPTR ((IDL_VPTR) NULL)

IDL_VPTR GETPID(int /*argc*/, IDL_VPTR /*argv[]*/)
{
  IDL_VPTR tmp = IDL_Gettmp();

  if (tmp==NULL_VPTR) {
    IDL_Message(IDL_M_NAMED_GENERIC,IDL_MSG_LONGJMP,
		"Couldn't create temporary variable");
  }
  
  tmp->type = IDL_TYP_LONG;
  tmp->value.l = getpid();

  return tmp;
}


IDL_SYSFUN_DEF main_def[] = 
  {{(IDL_FUN_RET) GETPID, (char*)"GETPID",  0, 0, 0}};


int IDL_Load(void)
{
  return IDL_AddSystemRoutine(main_def,IDL_TRUE,1); /* Just add getpid */
}
