/* dsec_comm.c++: Defines the entry point for the Idl Wrapper Routines
for the LBT672 Adaptive Secondary Mirror control amd diagnostic */

/* err = CALL_EXTERNAL(sc.dll_name, 'read_seq',               $
                       bcu_num, first_dsp, last_dsp, dsp_address, data_length,   $
                       set_PD
   read_seq.pro IDL call_external function example */

#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include "idl_export.h"
#include "msglib.h"
#include "commlib.h"
#include "BCUmsg.h"
#include "bcucommand.h"
#include "errlib.h"
#include "errordb.h"
#include "buflib.h"
#include "rtdblib.h" 
#include "adsec_comm.h"
#include "common.h"

//Global variables definition
unsigned char idl_seqnum = 0;
int idl_setup_ok = 0;
IDL_LONG timeout = 0;
char *server_ip    = "127.0.0.1";
char MyName[PROC_NAME_LEN];
int ErrC = 0;
int flags = 0;


//debugg variable
int debugg = 0;
int verb = 0;

// Shared memory variables
BufInfo *Info = NULL;
int info_size=0;

/* Function to check if we are already attached to a buffer.
 * Returns a pointer to the buffer Info struct or NULL if we are
 * not attached
 */
BufInfo *is_attached( char *bufname)
{
    char *pos, *onlyname;
    int i;

    pos = strchr(bufname, ':');
    *pos = 0;   // Zero-terminate after the producer
    onlyname = pos+1; // points to the buffer name

    if ((info_size>0) && (Info != NULL))
        for (i=0; i< info_size; i++)
            {
//            printf("name: %s - onlyname: %s - prod: %s - bufname; %s\n", Info[i].com.name, onlyname, Info[i].com.producer, bufname);
            if (strncmp( Info[i].com.name, onlyname, VAR_NAME_LEN) == 0)
                if (strncmp( Info[i].com.producer, bufname, PROC_NAME_LEN) ==0)
                    {
                    *pos = ':';    // Restore the corret name
                    return Info+i;
                    }
            }

    *pos = ':';    // Restore the corret name
    return NULL;
}
    
/* Function to resize the Info vector destroying the 'bufname' element */
void resize_Info( char *bufname)
{
    char *pos, *onlyname;
    int i;

    pos = strchr(bufname, ':');
    *pos = 0;   // Zero-terminate after the producer
    onlyname = pos+1; // points to the buffer name
    if (verb) printf("Resizing buffer queue...\n");

    if ((info_size>0) && (Info != NULL))
        for (i=0; i< info_size; i++)
            {
            if (verb) printf("name: %s - onlyname: %s - prod: %s - bufname; %s\n", Info[i].com.name, onlyname, Info[i].com.producer, bufname);
            if (strncmp( Info[i].com.name, onlyname, VAR_NAME_LEN) == 0)
                if (strncmp( Info[i].com.producer, bufname, PROC_NAME_LEN) ==0)
                    {
                    *pos = ':';    // Restore the corret name
                    memmove(Info+i,Info+i+1,(info_size-i-1)*sizeof(BufInfo));
                    info_size--;
                    if (verb) printf("Resized Info array, deleted elements %d, new info_size %d\n", i, info_size);
                    return;
                    }
            }

    if (verb) printf("Non ho trovato il buffer! sono Resize_Info\n");
    *pos = ':';    // Restore the corret name
    return;
}



/* Function to attach to a new shared buffer */

int idl_attach( char *bufname, int type, BufInfo **extern_info)
{
   BufInfo *local_info;

   /* Allocate a new vector element */
   if (!Info)
        {
        Info = (BufInfo *) malloc( sizeof(BufInfo));
        info_size =1;
        }
   else
        {
        info_size++;
        Info = (BufInfo *) realloc( Info, sizeof(BufInfo) * info_size);
        }

    if (!Info)
        {
        printf("Malloc error\n");
        info_size=0;
        return MEMORY_ALLOC_ERROR;
        }

    local_info = &(Info[info_size-1]);

    if (verb) printf("Mi attacco a %s come %s...\n", bufname, MyName);
    ErrC = bufRequest(MyName, bufname, type, local_info);
    if (verb) printf("Nome del shm: %s, e del producer %s\n", Info[info_size-1].com.name, Info[info_size-1].com.producer );
    if (PLAIN_ERROR(ErrC) != 0) {
        info_size--;
        Info = (BufInfo *) realloc( Info, sizeof(BufInfo) * info_size);
        if (verb) printf("BufRequest failed, realloc to infosize: %d\n", info_size);
        if (verb) printf("Errno e' %d\n",errno);
        return PLAIN_ERROR(ErrC);
    }
    if (verb) printf("ErrC1 from BufRequest: %d\n", PLAIN_ERROR(ErrC));
    if (verb) printf("Data readable length: %d\n", local_info->com.lng);
    if (verb) printf(" Attaccato con info_size: %d\n", info_size);

    if (extern_info)
        *extern_info = local_info;

    return NO_ERROR;
}


/* Function to call first of all to initialize the client IDL in order to connect with the Supervisor */
IDL_LONG idl_setup( int lArgc, void *Argv[])
{
   char *pchClientname;
      
   ErrC = 0;

   // lArgc is the number of arguments
      if (lArgc !=2 )
         return(IDL_PAR_NUM_ERROR);
     
      pchClientname =  ((IDL_STRING *)Argv[0])->s;
      timeout = *((IDL_LONG *)Argv[1]);
      memset( MyName, 0, PROC_NAME_LEN);
      strncpy( MyName, pchClientname , PROC_NAME_LEN-1);

      // Connnect to  server
      ErrC = ConnectToServer( server_ip, MyName, 1);
      if (ErrC >=0)
         {
         idl_setup_ok=1;
         return NO_ERROR;
         }
      else
         return PLAIN_ERROR(ErrC);
}


IDL_LONG smreadbuf(int Argc, void *Argv[])
{
   char *ptrBufName;
   ErrC = 0;
   int ErrC1 = 0;
   int ErrC2 = 0;
   int ErrC3 = 0;
   int retry, n_retry=1;
   int TypeBuf;
   IDL_INT type;
   unsigned char *dataptr;
   BufInfo *local_info;
   
//   printf("Argc,%d\n", Argc);

   if (Argc !=4) return(IDL_PAR_NUM_ERROR);
   
   printf("stringa = %d \n", (int)(Argv[0]));
   ptrBufName = ((IDL_STRING *)Argv[0])->s;
   type = *((IDL_INT *)Argv[1]);
   timeout = *((IDL_LONG *)Argv[2]);
   dataptr = (UCHAR *)Argv[3];

//   printf("%s %d %d\n", ptrBufName, type, timeout);

    if (type <= 0 || type >=3 ) return (IDL_DATATYPE_ERROR);
   switch (type) 
   {
      case 1 : TypeBuf=BUFCONT;
      break;
      case 2 : TypeBuf=BUFSNAP;
      break;
    }

   for (retry=0; retry <= n_retry; retry++)
   {
      if (! (local_info = is_attached(ptrBufName)))
      {
         if (verb) printf("I'm attaching with idl_attach...\n");
         if (verb) printf("Number retry: %d\n", retry);
         ErrC1 = idl_attach(ptrBufName, TypeBuf, &local_info);
       
         if (verb) printf("ErrC1 from BufRequest %d\n", PLAIN_ERROR(ErrC1));
         if (PLAIN_ERROR(ErrC1) < 0) return PLAIN_ERROR(ErrC1);
      }
    
      ErrC2 = bufRead (local_info, dataptr, timeout);
      if (verb) {
         if (ErrC2 > 0) printf("Length from BufRead %d\n", ErrC2);
         printf("Errno e' %d\n",errno);
      }

      if (PLAIN_ERROR(ErrC2) == SEM_LOCK_ERROR) {

         ErrC3 = bufRelease(MyName, local_info);
         if (ErrC3 != NO_ERROR) return PLAIN_ERROR(ErrC3);

         memset(local_info, 0, sizeof(local_info));
         if (verb) printf("Memset done, old shared buffer cleared.\n");
         resize_Info(ptrBufName);

      } else {
           if (verb) printf("Memset not done!\n");
           break;
      }
   }

   if (PLAIN_ERROR(ErrC2) < 0) return PLAIN_ERROR(ErrC2);
   //ErrC3 = bufRelease (MyName, &Info);
   //printf("ErrC3 from BufRelease %d\n", PLAIN_ERROR(ErrC3));
   return PLAIN_ERROR(ErrC2) ;
 
}
   

IDL_LONG idl_close( int lArgc, void *lArgv[])
   {
     ErrC = 0;
     int i;
      int socket = GetServerSocket();
      close(socket);

      /* Detach from shared memory buffers */
      if ((Info != NULL) && (info_size>0))
          {
//          for (i=0; i<info_size; i++)
 //             bufRelease( MyName, Info+i);
          free(Info);
          Info = NULL;
          info_size=0;
          }

      idl_setup_ok =0;
      return NO_ERROR;
   }


   IDL_LONG read_seq(int Argc, void *Argv[])
   {

     memset(&flags, 0, sizeof(flags));
     flags = FLAG_WANTREPLY;
     
     IDL_INT BCU_num, firstDSP, lastDSP;
     IDL_LONG dspAddress, dataLength;
     UCHAR *ReceiveBuffer, setPD;
     IDL_LONG time;
     int opcode,i;
     
     ErrC=0;
     // Check if the number of argument is correct
     if (Argc != 8) return IDL_PAR_NUM_ERROR;
     
     // Check if we are connected with the Supervisor
     if (! idl_setup_ok)
       return MISSING_CLIENTNAME_ERROR;
     
     BCU_num    = *((IDL_INT *)(Argv[0]));
     firstDSP  = *((IDL_INT *)(Argv[1]));
     lastDSP   = *((IDL_INT *)(Argv[2]));
     dspAddress= *((IDL_LONG *)(Argv[3]));
     dataLength= *((IDL_LONG *)(Argv[4]));
     ReceiveBuffer = (UCHAR  *)(Argv[5]);
     time = *((IDL_LONG *)(Argv[6]));
     setPD = *((UCHAR *)(Argv[7]));
     
     // Data compatibility check
     if (setPD >= 6 || setPD == 1) return IDL_PAR_NUM_ERROR;
     
     // verifica il valore di data length
     if (dataLength<=0)
       return IDL_DATALENGTH_ERROR;
     
     // verifica la congruenza dei dati
     // last DSP > first DSP
     if (lastDSP<firstDSP)
       return IDL_WRONG_DSP_ERROR;
     
     /* Opcode definition
        setPD = 0 (OL from IDL) -> DSP MEMORY
        setPD = 1 (1L from IDL) -> EMPTY MEMORY
        setPD = 2 (2L from IDL) -> SDRAM MEMORY 
        setPD = 3 (3L from IDL) -> SRAM MEMORY 
        setPD = 4 (4L from IDL) -> FLASH MEMORY 
        setPD = 5 (5L from IDL) -> FPGA MEMORY
     */
     switch (setPD) 
       {
       case 0 : opcode = MGP_OP_RDSEQ_DSP;
         break;
       case 2 : opcode = MGP_OP_RDSEQ_SDRAM;
         break;
       case 3 : opcode = MGP_OP_RDSEQ_SRAM;
         break;
       case 4 : opcode = MGP_OP_RDSEQ_FLASH;
         break;
       case 5 : opcode = MGP_OP_RDSEQ_DIAGBUFF;
         break;
       }
     
     if (debugg)
       {
         printf("OPCODE  : %d\n", opcode);
         printf("BCU_num : %d\n", BCU_num);
         printf("firstDSP: %d\n", firstDSP);
         printf("lastDSP : %d\n", lastDSP);
         printf("dspAddress : %d\n", dspAddress);
         printf("dataLength : %d\n", dataLength);
         printf("timeout : %d\n", time);
        int ii = 0;
         int j = 0;
         for (j=firstDSP;j<=lastDSP;j++)
         {
            for (i=0;i<dataLength;i++)
            {
               printf("%08X(%g),", ((int *)(ReceiveBuffer))[ii], ((float *)(ReceiveBuffer))[ii]);
               ii++;
            }
         printf("\n");
         }
      } 
     
     ErrC = sendBCUcommand( MyName, BCU_num, firstDSP, lastDSP, opcode, dspAddress, ReceiveBuffer, dataLength*(sizeof(float32)), time, ++idl_seqnum, flags);
     

     if (debugg) printf("Error from Supervisor %d\n",ErrC);

     //data e' *uchar   
     //tutti sono int
     //myname *char
     
     
     return ErrC;
   }


   IDL_LONG write_seq(int Argc, void *Argv[])
   {
     
     memset(&flags, 0, sizeof(flags));

     IDL_INT BCU_num, firstDSP, lastDSP;
     IDL_LONG dspAddress, dataLength;
     UCHAR *ReceiveBuffer, setPD;
     IDL_LONG time, flags_tmp;
     int opcode;
     ErrC=0;
     // Check if the number of argument is correct
     if (Argc != 9) return IDL_PAR_NUM_ERROR;
     
     // Check if we are connected with the Supervisor
     if (! idl_setup_ok)
       return MISSING_CLIENTNAME_ERROR;
     
     BCU_num    = *((IDL_INT *)(Argv[0]));
     firstDSP  = *((IDL_INT *)(Argv[1]));
     lastDSP   = *((IDL_INT *)(Argv[2]));
     dspAddress = *((IDL_LONG *)(Argv[3]));
     dataLength= *((IDL_LONG *)(Argv[4]));
     ReceiveBuffer = (UCHAR  *)(Argv[5]) ;
     time = *((IDL_LONG *)(Argv[6]));
     setPD = *((UCHAR *)(Argv[7]));
     flags_tmp = *((IDL_LONG *)(Argv[8]));
     
     // Data compatibility check
     if (setPD >= 7 || setPD == 1) return IDL_PAR_NUM_ERROR;
     
     // verifica il valore di data length
     if (dataLength<=0){
       printf("dataLength : %d\n", dataLength);
       return IDL_DATALENGTH_ERROR;
     }
     // verifica la congruenza dei dati
     // last DSP > first DSP
     if (lastDSP<firstDSP)
       return IDL_WRONG_DSP_ERROR;
     
     /* Opcode definition
        setPD = 0 (OL from IDL) -> DSP MEMORY
        setPD = 1 (1L from IDL) -> NOT USED       // Modified by AR
        setPD = 2 (2L from IDL) -> SDRAM MEMORY 
        setPD = 3 (3L from IDL) -> SRAM MEMORY 
        setPD = 4 (4L from IDL) -> FLASH MEMORY 
        setPD = 5 (5L from IDL) -> FPGA MEMORY 
     */
     switch (setPD) 
       {
       case 0 : opcode =  MGP_OP_WRSEQ_DSP;
         break;
       case 1 : // return an error code; Note by AR
         break;
       case 2 : opcode = MGP_OP_WRSEQ_SDRAM;
         break;
       case 3 : opcode = MGP_OP_WRSEQ_SRAM;
         break;
       case 4 : opcode = MGP_OP_WRITE_FLASH;
         break;
       case 5 : opcode = MGP_OP_WRSEQ_DIAGBUFF;
         break;
       case 6 : opcode = MGP_OP_WRITE_SIGGEN_RAM;
         break;
       }
     
     if (debugg)
       {
         printf("OPCODE  : %d\n", opcode);
         printf("BCU_num : %d\n", BCU_num);
         printf("firstDSP: %d\n", firstDSP);
         printf("lastDSP : %d\n", lastDSP);
         printf("dspAddress : %d\n", dspAddress);
         printf("dataLength : %d\n", dataLength);
         int ii = 0;
         int i,j;
         for (j=firstDSP;j<=lastDSP;j++)
         {
            for (i=0;i<dataLength;i++)
            {
               printf("%08X(%g),", ((int *)(ReceiveBuffer))[ii], ((float *)(ReceiveBuffer))[ii]);
               ii++;
            }
         printf("\n");
         }
         printf("timeout : %d\n", time);
         }
     

     
     ErrC = sendBCUcommand( MyName, BCU_num, firstDSP, lastDSP, opcode, dspAddress, ReceiveBuffer, dataLength*(sizeof(float32)), time, ++idl_seqnum, flags_tmp);
      
     if (debugg) printf("Error from Supervisor %d\n",ErrC);
     
     //data e' *uchar   
     //tutti sono int
     //myname *char
     
     
     return PLAIN_ERROR(ErrC);
   }



   IDL_LONG write_same(int Argc, void *Argv[])
   {
      
     memset(&flags, 0, sizeof(flags));
    
      IDL_INT BCU_num, firstDSP, lastDSP;
      IDL_LONG dspAddress, dataLength;
      UCHAR *ReceiveBuffer, setPD;
     IDL_LONG time, flags_tmp;
      int opcode,i;
      ErrC=0;
   // Check if the number of argument is correct
      if (Argc != 9) return IDL_PAR_NUM_ERROR;

   // Check if we are connected with the Supervisor
      if (! idl_setup_ok)
         return MISSING_CLIENTNAME_ERROR;

      BCU_num   = *((IDL_INT *)(Argv[0]));
      firstDSP  = *((IDL_INT *)(Argv[1]));
      lastDSP   = *((IDL_INT *)(Argv[2]));
      dspAddress = *((IDL_LONG *)(Argv[3]));
      dataLength= *((IDL_LONG *)(Argv[4]));
      ReceiveBuffer = (UCHAR  *)(Argv[5]) ;
     time = *((IDL_LONG *)(Argv[6]));
      setPD = *((UCHAR *)(Argv[7]));
     flags_tmp = *((IDL_LONG *)(Argv[8]));

   // Data compatibility check
      if (setPD >= 7 || setPD == 1 || setPD == 4) return IDL_PAR_NUM_ERROR;

   // verifica il valore di data length
      if (dataLength<=0)
         return IDL_DATALENGTH_ERROR;

   // verifica la congruenza dei dati
   // last DSP > first DSP
      if (lastDSP<firstDSP)
         return IDL_WRONG_DSP_ERROR;

   /* Opcode definition
         setPD = 0 (OL from IDL) -> DSP MEMORY
         setPD = 1 (1L from IDL) -> EMPTY MEMORY
         setPD = 2 (2L from IDL) -> SDRAM MEMORY 
         setPD = 3 (3L from IDL) -> SRAM MEMORY 
         setPD = 4 (4L from IDL) -> EMPTY MEMORY 
         setPD = 5 (5L from IDL) -> FPGA MEMORY
   */
      switch (setPD) 
      {
      case 0 : opcode = MGP_OP_WRSAME_DSP;
               break;
      case 1 : //error 
               break;
      case 2 : opcode = MGP_OP_WRSAME_SDRAM;
               break;
      case 3 : opcode = MGP_OP_WRSAME_SRAM;
               break;
      case 4 : //error 
               break;
      case 5 : opcode = MGP_OP_WRSAME_DIAGBUFF;
               break;
      case 6 : opcode = MGP_OP_WRITE_SIGGEN_RAM;
               break;
      }

   if (debugg)
   {
      printf("OPCODE  : %d\n", opcode);
      printf("BCU_num : %d\n", BCU_num);
      printf("firstDSP: %d\n", firstDSP);
      printf("lastDSP : %d\n", lastDSP);
      printf("dspAddress : %d\n", dspAddress);
      printf("dataLength : %d\n", dataLength);
         printf("timeout : %d\n", time);
      for (i=0;i<dataLength*4;i++)
         printf("%08X", ((int *)(ReceiveBuffer))[i]);
      printf("\n");
   }  


      ErrC = sendBCUcommand( MyName, BCU_num, firstDSP, lastDSP, opcode, dspAddress, ReceiveBuffer, dataLength*(sizeof(float32)), time, ++idl_seqnum, flags_tmp);
     if (debugg) printf("Error from Supervisor %d\n",ErrC);
   //data e' *uchar  
   //tutti sono int
   //myname *char

      
      return PLAIN_ERROR(ErrC);
   }


   IDL_LONG clear_mem(int Argc, void *Argv[])
   {
      
     memset(&flags, 0, sizeof(flags));
     flags = FLAG_WANTREPLY;
      IDL_INT BCU_num, firstDSP, lastDSP;
      IDL_LONG dspAddress, dataLength;

      UCHAR *ReceiveBuffer, setPD;
     IDL_LONG time;
      int opcode;
      ErrC=0;
   // Check if the number of argument is correct
      if (Argc != 7) return IDL_PAR_NUM_ERROR;

   // Check if we are connected with the Supervisor
      if (! idl_setup_ok)
         return MISSING_CLIENTNAME_ERROR;

      BCU_num   = *((IDL_INT *)(Argv[0]));
      firstDSP  = *((IDL_INT *)(Argv[1]));
      lastDSP   = *((IDL_INT *)(Argv[2]));
      dspAddress = *((IDL_LONG *)(Argv[3]));
      dataLength= *((IDL_LONG *)(Argv[4]));
      ReceiveBuffer = (UCHAR  *)(Argv[5]) ;
      setPD = *((UCHAR *)(Argv[6]));
      time = timeout;
      
   // Data compatibility check
      if (setPD >= 5 || setPD <= 0 || setPD == 1) return IDL_DATATYPE_ERROR;

   // verifica il valore di data length
      if (dataLength<=0)
         return IDL_DATALENGTH_ERROR;

   // verifica la congruenza dei dati
   // last DSP > first DSP
      if (lastDSP<firstDSP)
         return IDL_WRONG_DSP_ERROR;

   /* Opcode definition
         setPD = 0 (OL from IDL) -> not used 
         setPD = 1 (OL from IDL) -> EMPTY MEMORY 
         setPD = 2 (1L from IDL) -> SDRAM MEMORY 
         setPD = 3 (1L from IDL) -> SRAM MEMORY 
         setPD = 4 (1L from IDL) -> FLASH MEMORY 
   */
      switch (setPD) 
      {
      case 2 : opcode = 140;
            break;
   case 3 : opcode = 145;
            break;
   case 4 : opcode = 130;
            break;
   }

if (debugg)
{
   printf("OPCODE  : %d\n", opcode);
   printf("BCU_num : %d\n", BCU_num);
   printf("firstDSP: %d\n", firstDSP);
   printf("lastDSP : %d\n", lastDSP);
   printf("dspAddress to clear: %d\n", dspAddress);
   printf("dataLength : %d\n", dataLength);
   printf(" and length : %d\n", *((int *)ReceiveBuffer));
      printf("timeout : %d\n", time);
}  

   ErrC = sendBCUcommand( MyName, BCU_num, firstDSP, lastDSP, opcode, dspAddress, ReceiveBuffer, dataLength*(sizeof(float32)), time, ++idl_seqnum, flags);
  if (debugg) printf("Error from Supervisor %d\n",ErrC);
//data e' *uchar  
//tutti sono int
//myname *char

   
   return PLAIN_ERROR(ErrC);
}


IDL_LONG lock_flash(int Argc, void *Argv[])
{
   
  memset(&flags, 0, sizeof(flags));
  flags = FLAG_WANTREPLY;
   IDL_INT BCU_num, firstDSP, lastDSP;
   IDL_LONG dspAddress, dataLength;    
   UCHAR *ReceiveBuffer;
  IDL_LONG time;
   int opcode;
   ErrC=0;
// Check if the number of argument is correct
   if (Argc != 5) return IDL_PAR_NUM_ERROR;

// Check if we are connected with the Supervisor
   if (! idl_setup_ok)
      return MISSING_CLIENTNAME_ERROR;

   BCU_num   = *((IDL_INT *)(Argv[0]));
   firstDSP  = *((IDL_INT *)(Argv[1]));
   lastDSP   = *((IDL_INT *)(Argv[2]));
   dspAddress = *((IDL_LONG *)(Argv[3]));
   dataLength= *((IDL_LONG *)(Argv[4]));
   ReceiveBuffer = (UCHAR  *)(Argv[5]) ;
   time=timeout;
// verifica il valore di data length
   if (dataLength<=0)
      return IDL_DATALENGTH_ERROR;

// verifica la congruenza dei dati
// last DSP > first DSP
   if (lastDSP<firstDSP)
      return IDL_WRONG_DSP_ERROR;

   opcode = 128;

if (debugg)
{
   printf("OPCODE : %d\n", opcode);
   printf("Locking FLASH Memory in BCU number %d\n", BCU_num);
   printf(" from DSP: %d", firstDSP);
   printf(" to DSP %d\n", lastDSP);
   printf(" from address %d", dspAddress);
   printf(" and length : %d\n", *((IDL_LONG *)(Argv[5])));
      printf("timeout : %d\n", time);
}  

   ErrC = sendBCUcommand( MyName, BCU_num, firstDSP, lastDSP, opcode, dspAddress, ReceiveBuffer, dataLength*(sizeof(float32)), time, ++idl_seqnum, flags);
  if (debugg) printf("Error from Supervisor %d\n",ErrC);
//data e' *uchar  
//tutti sono int
//myname *char

   
   return PLAIN_ERROR(ErrC);
}

IDL_LONG unlock_flash(int Argc, void *Argv[])
{
   
  memset(&flags, 0, sizeof(flags));
  flags = FLAG_WANTREPLY;
   IDL_INT BCU_num, firstDSP, lastDSP;
   IDL_LONG dspAddress, dataLength;
   UCHAR *ReceiveBuffer;
  IDL_LONG time;
   int opcode;
   ErrC=0;
// Check if the number of argument is correct
   if (Argc != 5) return IDL_PAR_NUM_ERROR;

// Check if we are connected with the Supervisor
   if (! idl_setup_ok)
      return MISSING_CLIENTNAME_ERROR;

   BCU_num   = *((IDL_INT *)(Argv[0]));
   firstDSP  = *((IDL_INT *)(Argv[1]));
   lastDSP   = *((IDL_INT *)(Argv[2]));
   dspAddress = *((IDL_LONG *)(Argv[3]));
   dataLength= *((IDL_LONG *)(Argv[4]));
   ReceiveBuffer = (UCHAR  *)(Argv[5]) ;
   time=timeout;

// verifica il valore di data length
   if (dataLength<=0)
      return IDL_DATALENGTH_ERROR;

// verifica la congruenza dei dati
// last DSP > first DSP
   if (lastDSP<firstDSP)
      return IDL_WRONG_DSP_ERROR;

   opcode = 129;

if (debugg)
{
   printf("OPCODE : %d\n", opcode);
   printf("Unlocking FLASH Memory in BCU number %d\n", BCU_num);
   printf(" from DSP: %d", firstDSP);
   printf(" to DSP %d\n", lastDSP);
   printf(" from address %d", dspAddress);
   printf(" and length : %d\n", *((IDL_LONG *)(Argv[5])));
      printf("timeout : %d\n", time);
}  

   ErrC = sendBCUcommand( MyName, BCU_num, firstDSP, lastDSP, opcode, dspAddress, ReceiveBuffer, dataLength*(sizeof(float32)), time, ++idl_seqnum, flags);
  if (debugg) printf("Error from Supervisor %d\n",ErrC);
//data e' *uchar  
//tutti sono int
//myname *char

   
   return PLAIN_ERROR(ErrC);
}


IDL_LONG reset_devices(int Argc, void *Argv[])
{
   
  memset(&flags, 0, sizeof(flags));
  flags = FLAG_WANTREPLY;
   IDL_INT BCU_num, firstDSP, lastDSP;
   IDL_LONG dspAddress, dataLength;
   UCHAR *ReceiveBuffer, setPD;
  IDL_LONG time;
   int i,opcode;
   ErrC=0;
// Check if the number of argument is correct
   if (Argc != 6) return IDL_PAR_NUM_ERROR;

// Check if we are connected with the Supervisor
   if (! idl_setup_ok)
      return MISSING_CLIENTNAME_ERROR;

   BCU_num   = *((IDL_INT *)(Argv[0]));
   firstDSP  = *((IDL_INT *)(Argv[1]));
   lastDSP   = *((IDL_INT *)(Argv[2]));
   dspAddress = *((IDL_LONG *)(Argv[3]));
   dataLength= *((IDL_LONG *)(Argv[4]));
   ReceiveBuffer = (UCHAR  *)(Argv[5]) ;
   time=timeout;
// verifica la congruenza dei dati
// last DSP > first DSP
   if (lastDSP < firstDSP )
      return IDL_WRONG_DSP_ERROR;

   opcode = 10;

if (debugg)
{
   printf("OPCODE : %d\n", opcode);
   printf("Reset devices:\n");
   printf("BCU num: %d\n", BCU_num);
   printf(" from DSP: %d", firstDSP);
   printf(" to DSP %d\n", lastDSP);
      printf("timeout : %d\n", time);
   }  

   ErrC = sendBCUcommand( MyName, BCU_num, firstDSP, lastDSP, opcode, dspAddress, ReceiveBuffer , dataLength * sizeof(float32), time, ++idl_seqnum, flags);
  if (debugg) printf("Error from Supervisor %d\n",ErrC);

//data e' *uchar  
//tutti sono int
//myname *char
// ErrC = NO_ERROR;
   

return ErrC;
}


IDL_LONG powerup(int Argc, void *Argv[])
{
   
  memset(&flags, 0, sizeof(flags));
  flags = FLAG_WANTREPLY;
   IDL_INT BCU_num, firstDSP, lastDSP;
   IDL_LONG dspAddress, dataLength;
   UCHAR *ReceiveBuffer, setPD;
  IDL_LONG time;
   int i,opcode;
   ErrC=0;
// Check if the number of argument is correct
   if (Argc != 7) return IDL_PAR_NUM_ERROR;

// Check if we are connected with the Supervisor
   if (! idl_setup_ok)
      return MISSING_CLIENTNAME_ERROR;

   BCU_num   = *((IDL_INT *)(Argv[0]));
   firstDSP  = *((IDL_INT *)(Argv[1]));
   lastDSP   = *((IDL_INT *)(Argv[2]));
   dspAddress = *((IDL_LONG *)(Argv[3]));
   dataLength= *((IDL_LONG *)(Argv[4]));
   ReceiveBuffer = (UCHAR  *)(Argv[5]) ;
   time = *((IDL_LONG *)(Argv[6]));
// verifica la congruenza dei dati
// last DSP > first DSP
   if (lastDSP < firstDSP )
      return IDL_WRONG_DSP_ERROR;

   opcode = 11;

if (debugg)
{   printf("OPCODE : %d\n", opcode);
    printf("Powerup:\n");
    printf("BCU num: %d\n", BCU_num);
    printf("timeout : %d\n", time);
   }  

   ErrC = sendBCUcommand( MyName, BCU_num, firstDSP, lastDSP, opcode, dspAddress, ReceiveBuffer , dataLength * sizeof(float32), time, ++idl_seqnum, flags);
  if (debugg) printf("Error from Supervisor %d\n",ErrC);

//data e' *uchar  
//tutti sono int
//myname *char
// ErrC = NO_ERROR;
   

return PLAIN_ERROR(ErrC);
}


//@Function: setvar
//
// Wrapper for the SetVar() rtdblib function. Required arguments:
//
// 1) variable name (string)
// 2) variable value (array pointer or string)
// 3) variable type (long idl, int c)
// 4) array length (long idl, int c, use 1 for scalars)
//
// Returns an integer error code.
//@


IDL_LONG setvar(int Argc, void *Argv[])
{
    char *varname;
    void *varvalue;
    int ErrC;
    IDL_LONG vartype, varlen;


    // Check if the number of argument is correct
    if (Argc != 4) return IDL_PAR_NUM_ERROR;


    // Check if we are connected with the Supervisor
    if (! idl_setup_ok)
        return MISSING_CLIENTNAME_ERROR;


    varname =  ((IDL_STRING *)Argv[0])->s;
    vartype  = *((IDL_LONG *)(Argv[2]));


    // Get pointer to either array or string depending on type
    if (vartype == CHAR_VARIABLE)
        varvalue = ((IDL_STRING *)Argv[1])->s;
    else
        varvalue = Argv[1];


    varlen  = *((IDL_LONG *)(Argv[3]));


    ErrC = SetVar( MyName, varname, vartype, varlen, varvalue, NULL);
    return PLAIN_ERROR(ErrC);
}


//@Function: createvar to create variable into the MsgRTDB

IDL_LONG createvar(int Argc, void *Argv[])
{
    char *VarName, *VarMode;
    int VarType, NElem;
    int ErrC;

    // Check if the number of argument is correct
    if (Argc != 4) return IDL_PAR_NUM_ERROR;

    VarName = ((IDL_STRING *)Argv[0])->s;
    if (strlen(VarName) >= VAR_NAME_LEN) return IDL_DATATYPE_ERROR;

    VarMode = ((IDL_STRING *)Argv[1])->s;
    if (strlen(VarMode) > 1) return IDL_DATATYPE_ERROR;

    VarType = *(IDL_LONG *)Argv[2];
    NElem   = *(IDL_LONG *)Argv[3];
    

    if ((VarMode[0] != 'P') && (VarMode[0] != 'G'))  
        return IDL_DATATYPE_ERROR;
    
    ErrC = CreaVar( MyName, VarName, VarType, VarMode, NElem, NULL);

    return PLAIN_ERROR(ErrC);
}

//@Function: getvar
//
// Wrapper for the GetVar() rtdblib function. Required arguments:
//
// 1) variable name (string)
// 2) variable value (array or scalar)
// 3) variable length (long)
// 4) timeout in milliseconds (long)
//
// Returns an integer error code.
//
// The array or scalar passed as second argument will be filled with the variable value
//@


IDL_LONG getvar( int Argc, void *Argv[])
{
    char *varname;
    UCHAR *varvalue;
    IDL_LONG timeout, varlen;


    int ErrC =0;


    // Check if the number of arguments is correct
    if (Argc != 4) return IDL_PAR_NUM_ERROR;


    // Check if we are connected with the Supervisor
    if (! idl_setup_ok)
        return MISSING_CLIENTNAME_ERROR;


    varname = ((IDL_STRING *)Argv[0])->s;
    varvalue = (UCHAR *)(Argv[1]);
    varlen = *((IDL_LONG *)(Argv[2]));
    timeout  = *((IDL_LONG *)(Argv[3]));


    //printf("varname: %s - varlen: %d\n", varname, varlen);

    if ((ErrC = ReadVar( MyName, varname, varvalue, varlen, timeout, NULL, NULL)) <0)
        return PLAIN_ERROR(ErrC);


    return NO_ERROR;
} 

/*
//@Function: writevar
//
// Wrapper for the WriteVar() rtdblib function. Required arguments:
//
// 1) variable name (string)
// 2) variable type (long)
// 3) variable length (long)
// 4) variable value (array or scalar)
// 5) timeout in milliseconds (long)
//
// Returns an integer error code.
//
//@


IDL_LONG writevar( int Argc, void *Argv[])
{
    char *varname; 
    UCHAR *varvalue;
    IDL_LONG timeout, varlen, vartype;

    int ErrC =0;


    // Check if the number of arguments is correct
    if (Argc != 4) return IDL_PAR_NUM_ERROR;


    // Check if we are connected with the Supervisor
    if (! idl_setup_ok)
        return MISSING_CLIENTNAME_ERROR;


    varname = ((IDL_STRING *)Argv[0])->s;
    vartype = *((IDL_LONG *)(Argv[2]));
    varlen = *((IDL_LONG *)(Argv[3]));
    varvalue = (UCHAR *)(Argv[4]);
    timeout  = *((IDL_LONG *)(Argv[3]));


    //printf("varname: %s - varlen: %d\n", varname, varlen);

    if ((ErrC = WriteVar( MyName, varname, vartype, varlen, varvalue, timeout, NULL, NULL)) <0)
        return PLAIN_ERROR(ErrC);

    return NO_ERROR;
}

*/
// Funciton to get MsgRTDB variable type

IDL_LONG get_vartypes(int Argc, void *Argv[])
{
    IDL_LONG datalen;
    int *buffer;

    datalen = *((IDL_LONG *)(Argv[0]));
    buffer = (int *)(Argv[1]);
    if (datalen != 7) return IDL_PAR_NUM_ERROR;
    *buffer = INT_VARIABLE;
    *(buffer+1) = REAL_VARIABLE;
    *(buffer+2) = CHAR_VARIABLE;
    *(buffer+3) = BIT8_VARIABLE;
    *(buffer+4) = BIT16_VARIABLE;
    *(buffer+5) = BIT32_VARIABLE;
    *(buffer+6) = BIT64_VARIABLE;

    return NO_ERROR;

}
