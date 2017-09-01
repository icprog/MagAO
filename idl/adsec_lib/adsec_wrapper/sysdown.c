/*
 *  * example.c: very simple example of port I/O
 *   *
 *    * This code does nothing useful, just a port write, a pause,
 *     * and a port read. Compile with `gcc -O2 -o example example.c',
 *      * and run as root with `./example'.
 *       */

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/io.h>


#define BASEPORT 0x378 /* lp0 */
#define COMMAND  0x00  /* command for the relais */

int main()
{

   /* Get access to the ports */
   if (ioperm(BASEPORT, 3, 1)) {perror("ioperm"); exit(1);}
       
   /* Drop root privileges */
   setuid(getuid());


   /* Set the data signals (D0-7) of the port to all low (0) */
   printf("Command: %d\n", COMMAND);
   outb((unsigned char)COMMAND, BASEPORT);
         
   /* Sleep for a while (100 ms) */
   usleep(100000);
           
   /* Read from the status port (BASE+1) and display the result */
   printf("Read: %d\n", inb(BASEPORT));

   /* We don't need the ports anymore */
   if (ioperm(BASEPORT, 3, 0)) {perror("ioperm"); exit(1);}

   exit(0);
}
