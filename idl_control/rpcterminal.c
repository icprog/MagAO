
#include "idl_rpc.h"  
int main()  
{   
   CLIENT *pClient;   
   char    cmdBuffer[512];  
   int     result;  

   /* Connect to the server */  
   if( (pClient = IDL_RPCInit(0, (char*)NULL)) == (CLIENT*)NULL){   
      fprintf(stderr, "Can't register with IDL server\n");   
      exit(1);  
   }  

   if (!(IDL_RPCTimeout((long)120L))){   
      fprintf(stderr, "Can't set timeout for the IDL RPCs\n");   
      exit(1);  
   }  

   /* Start a loop that will read commands and then send them to idl */  
   for(;;){  
      printf("RMTIDL> ");  
      cmdBuffer[0]='\0';  
      fgets(cmdBuffer, 1000, stdin);  
      if( cmdBuffer[0] == '\n' || cmdBuffer[0] == '\0')  
      break;   
      result = IDL_RPCExecuteStr(pClient, cmdBuffer);  
   }  

   /* Now disconnect from the server and kill it. */  
   if(!IDL_RPCCleanup(pClient, 1))  
   fprintf(stderr, "IDL_RPCCleanup: failed\n");  
   exit(0);  
}  
