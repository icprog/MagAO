
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <termios.h>
#include <unistd.h>
#include "dcn.h"


/* Original file by Jeff Hagen of Steward Observatory.
 * Modifications by Jared Males of SO, for the Magellan AO project:
 *    Added #ifdef DEBUG protection of printf statements
 *    Added tcp/ip socket interface for use with a portserver
 */

/* New Focus gave me their dcn source code and I was never able to 
   make it work. 

   It did give me enough information to reverse engineer the windows 
   code by sniffing transmit and receive. 

   Messages are of the format:
   header  0xaa
   address   
   cmd
   ...
   checksum

   checksum is simply the sum of the all the message bytes starting at 
   the address up to and not including the checksum.
   
   Responses are of the format, response, checksum
   Most commands respond with motor status.

   We have 8301 pico motors and 8753 controllers.
   It should be noted that the controller does not really distingush well
   between the motors. The counter and status refer to the controller not
   the motor so it is up to the sw to recall the position and status of 
   each motor between moves.
*/ 

static void append_dcn( struct DCN_HEADER *head, struct DCN *dcn );
static unsigned char checksum( unsigned char *buf, int n );

static void hard_reset( int fd );
static int arm_timeout_read( int fd, unsigned char *rbuf, int n );
static int dcn_pwrite( int fd, unsigned char *buf, int n );
static int clear_pos( int fd, int addr);
void add_controller(int addr, int status, int ver, int type);
void build_uniq_addr( struct DCN *dcn, int *array, int num );

static struct DCN_HEADER *dcn_header = NULL;

static char ewmsg[256];
/* if num == 0 then initialize the network, othwise assume
 *   num contollers, 3 motors each 
 */

struct DCN_HEADER *dcn_init(int verbose, const char *dcnaddr, int port)
{
   int fd, i, addr, ret, num, dct;
   unsigned char cstr[256];
   unsigned char status, type, ver;
   struct termios tt;
   struct DCN *dcn;
   struct DCN_CONTROLLER *con;
   int *addr_array;

   if( dcn_header )
      return dcn_header;
  
   if(port)
   {
      fd = serial_init(dcnaddr, port);
   }
   else
   {
      fd = open(dcnaddr, O_RDWR, 0 );
      tcgetattr(fd, &tt);
      cfmakeraw(&tt);
      cfsetispeed(&tt, B19200);
      cfsetospeed(&tt, B19200);

      if (tcsetattr( fd, TCSANOW, &tt ) < 0 ) 
      {
         //perror("tcsetattr");
         ERRMSG(strerror(errno));
         close(fd);
         fd = -1;
         return 0;
      } 
      else 
      {
         tcflush(fd, TCIOFLUSH);
      }
   }

   if(fd <=0 )
   {
      dcn_header = 0;
      return 0;
   }
   dcn_header = (struct DCN_HEADER *)malloc( sizeof(struct DCN_HEADER));
   bzero( dcn_header, sizeof(struct DCN_HEADER ));
   dcn_header->fd = fd;
   dcn_header->verbose = verbose;

   num = 0;
   for( addr = 1; addr<0x20; addr++ ) 
   {
                                   /* Read the device type */
      cstr[0] = (char) 0xAA;         /* Header */
      cstr[1] = addr;                /* Send to addr */
      cstr[2] = 0x10 | 0x03;         /* READ_STAT */
      cstr[3] = 0x20;                /* SEND_ID,  send ID only */
      cstr[4] = (unsigned char)checksum( cstr+1, 3);
                  /* (cstr[1] + cstr[2] + cstr[3]);  checksum */
      dcn_pwrite( fd, cstr, 5 );

      i = arm_timeout_read( fd, cstr, 4 );
      if( i != 4 ) 
      { /* if failed for some reason, try again */
         cstr[0] = (char) 0xAA;         /* Header */
         cstr[1] = addr;                /* Send to addr */
         cstr[2] = 0x10 | 0x03;         /* READ_STAT */
         cstr[3] = 0x20;                /* SEND_ID,  send ID only */
         cstr[4] = (unsigned char)checksum( cstr+1, 3);
                  /* (cstr[1] + cstr[2] + cstr[3]);  checksum */
         dcn_pwrite( fd, cstr, 5 );
         i = arm_timeout_read( fd, cstr, 4 );
      }

      if( i !=4 )
         break;

      status = cstr[0];
      type = cstr[1];
      ver = cstr[2];
      add_controller( addr, status, ver, type );
      num++;
   }
  
   if( num == 0 ) 
   { /* do a hard reset */

      for( i=1; i<=0x20; i++ ) 
      {
         cstr[0] = 0xaa;
         cstr[1] = i; /* addr */
         cstr[2] = 0x0b;
         cstr[3] = checksum( cstr+1, 2 );
         ret = dcn_pwrite( fd, cstr, 4 );
      }
      hard_reset( fd );

      dcn_flush();

      addr = 1;
      while (1) 
      { /* First set the address to a unique value: */
         cstr[0] = 0xAA;              /* Header */
         cstr[1] = 0;                 /* Send to default address of 0 */
         cstr[2] = 0x20 | 0x01;       /* SET_ADDR */
         cstr[3] = addr;              /* Set new address sequentially */
         cstr[4] = 0xFF;              /* Set group address to 0xFF */
         cstr[5] = (unsigned char)checksum(cstr+1, 4 );
            /* (cstr[1] + cstr[2] + cstr[3] + cstr[4]);    checksum */
 
         dcn_pwrite( fd, cstr, 6 );
    
         if( arm_timeout_read( fd, cstr, 2 ) != 2 )
            break;

         if (cstr[0] != cstr[1])
         break;
                                   /* Read the device type */
         cstr[0] = (char) 0xAA;         /* Header */
         cstr[1] = addr;                /* Send to addr */
         cstr[2] = 0x10 | 0x03;         /* READ_STAT */
         cstr[3] = 0x20;                /* SEND_ID,  send ID only */
         cstr[4] = (unsigned char)checksum( cstr+1, 3);
                  /* (cstr[1] + cstr[2] + cstr[3]);  checksum */
         dcn_pwrite( fd, cstr, 5 );

         if( arm_timeout_read( fd, cstr, 4 ) != 4 ) break;

         status = cstr[0];
         type = cstr[1];
         ver = cstr[2];

         add_controller( addr, status, ver, type );

         addr++;  // increment the address
         num++;
      }
      #ifdef DEBUG
      printf("DCN found and initialized %d controllers\n", num );
      #endif
   } 
   else 
   {
      #ifdef DEBUG
      printf("DCN found %d controllers\n", num );
      #endif
   }

   /* build up the CON structure */

   dcn_header->num_addrs = num;
  
   addr_array = (int *)malloc(sizeof(int)*num );
   build_uniq_addr( dcn_header->dcn, addr_array, num );

   for( i=0; i<num; i++ ) 
   {
      con = (struct DCN_CONTROLLER *)malloc(sizeof(struct DCN_CONTROLLER));
      bzero( con, sizeof(struct DCN_CONTROLLER));
      con->next = dcn_header->con;
      con->addr = addr_array[i];
      dcn_header->con = con;
   }

   free(addr_array);

   con = dcn_header->con;
   dcn = dcn_header->dcn;
   while( con ) 
   {
      dct = 0;
      dcn = dcn_header->dcn;
      while( dcn ) 
      {
         if( dcn->addr == con->addr ) 
         {
            dcn->con = con;
            if( dct >= DCN_MAX_UNITS ) 
            {
               snprintf(ewmsg, 256, "ERROR controller has more than %d motors", DCN_MAX_UNITS);
               ERRMSG(ewmsg);
               dct = 0;
            }
            con->dcn[dct] = dcn;
            dct++;
         }
         dcn = dcn->next;
      }
      con = con->next;
   }

   con = dcn_header->con;
   while( con ) 
   {
      con->status = dcn_motor_stat_only( con->dcn[0] );
      con->active = con->dcn[0];
      con = con->next;
   }

   if(num ==0) serial_close(fd);

   pthread_mutex_init(&dcn_header->mutex, 0);
   return dcn_header;
}

void dcn_shutdown()
{
   if(dcn_header) free(dcn_header);
   dcn_header = 0;
}


void add_controller(int addr, int status, int ver, int type )
{
  struct DCN *dcn;
  int motor, fd;
  unsigned char cstr[256];

  fd = dcn_header->fd;

  if( dcn_header->verbose )
  {
    printf("found controller at addr 0x%x stat 0x%x modtype 0x%x modver 0x%x\n", addr, status, type, ver );
   }
   
  cstr[0] = (char) 0xAA;
  cstr[1] = addr;
  cstr[2] = 0x18;                /* SET_OUTPUTS */
  cstr[3] = 0x10;                /* SEND_ID,  send ID only */
  cstr[4] = (unsigned char)checksum( cstr+1, 3);
  dcn_pwrite( fd, cstr, 5 );

  if( arm_timeout_read( fd, cstr, 2 ) != 2 ) WARNMSG("oops");

  cstr[0] = (char) 0xAA;
  cstr[1] = addr;
  cstr[2] = 0x18;                /* SET_OUTPUTS */
  cstr[3] = 0x00;                /* SEND_ID,  send ID only */
  cstr[4] = (unsigned char)checksum( cstr+1, 3);
  dcn_pwrite( fd, cstr, 5 );

  if( arm_timeout_read( fd, cstr, 2 ) != 2 ) WARNMSG("oops");

   if( type == DCN_STEPMODTYPE )
   {
      for( motor=0; motor<3; motor++ )
      {
         dcn = (struct DCN *)malloc( sizeof(struct DCN));
         bzero( dcn, sizeof(struct DCN));
         dcn->addr = addr;
         dcn->unit = motor;
         dcn->status = status;
         dcn->vel = DCN_STEP_DEFVEL;
         dcn->acc = DCN_STEP_DEFACC;
         dcn->mode = DCN_STEP_MODE8X;
         dcn->modtype = type;
         dcn->modver = ver;
         dcn->position = 0;
         if(dcn_header->verbose) printf("added dcn addr %d unit %d\n", addr, motor );
         append_dcn( dcn_header, dcn );
      }
   }
   else
   {
      snprintf(ewmsg, 256, "unknown controller type %d", type );
      ERRMSG(ewmsg);
   }
}//void add_controller(int addr, int status, int ver, int type )

static int arm_timeout_read( int fd, unsigned char *rbuf, int n )
{
  fd_set rfd;
  struct timeval tm;
  int sel, count, i;

  count = 0;
  tm.tv_sec = 0;
  tm.tv_usec = 500000;
  
   while( count < n )
   {
      FD_ZERO( &rfd );
      FD_SET( fd, &rfd );

      if((sel =  select( FD_SETSIZE, &rfd, NULL, NULL, &tm ))<0 )
      {
         count = -1;
         break;
      }
  
      if( sel == 0 )
      {
         if( dcn_header->verbose ) printf("timeout with count %d\n", count );
         break;
      }

      if( FD_ISSET( fd, &rfd ))
      {
         if( read( fd, &rbuf[count], 1 ) != 1 )
         {
            snprintf(ewmsg, 256, "read error with count %d", count );
            ERRMSG(ewmsg);
            break;
         }
         else  count++;
      }
   }

   if( dcn_header->verbose )
   {
      printf("got ");
      for( i=0; i<count; i++ ) printf("0x%x ", rbuf[i]);
      printf("\n");
   }

   return count;
}

unsigned char checksum( unsigned char *buf, int n )
{
  int i;
  unsigned char sum;

  sum = 0;
  for( i=0; i<n; i++ ) 
    sum += buf[i];
    
  return sum;
}

void hard_reset( int fd )
{
  int i;
  unsigned char buf[16];

  buf[0] = 0;
  for( i=0;i<20; i++ )
    dcn_pwrite( fd, buf, 1 );

  buf[0] = 0xaa;
  buf[1] = 0xff;
  buf[2] = 0x0f;
  buf[3] = 0xe;
  dcn_pwrite( fd, buf, 4 );
}


static int dcn_pwrite( int fd, unsigned char *buf, int n )
{
  int i, ret;

  if( dcn_header->verbose ) {
    printf("wrote ");
    for( i=0; i<n; i++ )
      printf("0x%x ", buf[i]);
    printf("\n");
  }

  ret = write( fd, buf, n );
  return ret; 
}

int dcn_flush()
{
  int fd, num;
  unsigned char reply[256];

  fd = dcn_header->fd;
  tcflush(fd, TCIOFLUSH);

  num = arm_timeout_read( fd, reply, 256 );
  if( num ==256 )
  {
      WARNMSG("warning failed to flush");
  }
  else
  {
      if(dcn_header->verbose) printf("successful flush %d remained\n", num);
   }
  return (num == 256 ? -1:0 );
}

/* start move on one motor */

int dcn_motor_move_rel( struct DCN *dcn, int pos )
{
  unsigned char msg[32];
  unsigned char reply[32];
  union {
    int st;
    unsigned char c[4];
  } u;
  int num;
  int fd;
  unsigned int status;

  fd = dcn_header->fd;

  if( dcn != dcn->con->active ) {
    status = dcn_motor_stat_only( dcn );
    if( (status & DCN_MOTOR_MOVING)!=0 ) {
      return -1;
    }
  }

   #ifdef DEBUG
   printf("dcn_move_rel addr %d unit %d steps %d\n",  dcn->addr, dcn->unit,  pos );
   #endif
   dcn->con->active = dcn;

  pthread_mutex_lock(&dcn_header->mutex);
  
  if(pos == 0 )
  {
    pthread_mutex_unlock(&dcn_header->mutex);
    return 0;
  }

  clear_pos( fd, dcn->addr );
  dcn_motor_disable( dcn );

  dcn_set_outputs( dcn, dcn->unit );

  dcn_motor_enable( dcn );

  u.st = pos*25;

  msg[0] = 0xaa;
  msg[1] = dcn->addr;
  msg[2] = 0x66;
  msg[3] = dcn->mode;
  msg[4] = 0x0;
  msg[5] = 0x0;
  msg[6] = 0x0;
  msg[7] = 0x0;
  msg[8] = 0xff;
  msg[9] = checksum( msg+1, 8 );

  dcn_pwrite( fd, msg, 10 );
  num = arm_timeout_read( fd, reply, 2 );

  if( num != 2 || reply[0] != reply[1] ) {
    ERRMSG("checksum error in move_rel");
    pthread_mutex_unlock(&dcn_header->mutex);
    return -1;
  }

  msg[0] = 0xaa;
  msg[1] = dcn->addr;
  msg[2] = 0x74;
  msg[3] = 0x87;
  msg[4] = u.c[0];
  msg[5] = u.c[1];
  msg[6] = u.c[2];
  msg[7] = u.c[3];
  msg[8] = dcn->vel;
  msg[9] = dcn->acc;
  msg[10] = checksum( msg+1, 9 );

  dcn_pwrite( fd, msg, 11 );

//0xaa 0x1 0x66 0x4 0x0 0x0 0x0 0x0 0xff 0x6a 
//0xaa 0x1 0x74 0x87 0x32 0x0 0x0 0x0 0xed 0x2 0x1d 
//
  num = arm_timeout_read( fd, reply, 2 );
  pthread_mutex_unlock(&dcn_header->mutex);
  
  if( num != 2 || reply[0] != reply[1] ) {
    ERRMSG("checksum error in move_rel");
    return -1;
  } else
    return 0;
}

/* turn motor on at vel */

int dcn_motor_vel( struct DCN *dcn, int vel )
{
  unsigned char msg[32];
  unsigned char reply[32];
  int num, fd, dir;

  fd = dcn_header->fd;

  if(vel == 0 )
    return 0;

  dcn_motor_disable( dcn );

  dcn_set_outputs( dcn, dcn->unit );

  dcn_motor_enable( dcn );

  msg[0] = 0xaa;
  msg[1] = dcn->addr;
  msg[2] = 0x66;
  msg[3] = dcn->mode;
  msg[4] = 0x0;
  msg[5] = 0x0;
  msg[6] = 0x0;
  msg[7] = 0x0;
  msg[8] = 0xff;
  msg[9] = checksum( msg+1, 8 );

  dcn_pwrite( fd, msg, 10 );
  num = arm_timeout_read( fd, reply, 2 );

  if( num != 2 || reply[0] != reply[1] ) {
    ERRMSG("checksum error in dcn_motor_vel\n");
    return -1;
  }
  if( (dir = (vel < 0)) )
   vel = abs(vel);

  msg[0] = 0xaa;
  msg[1] = dcn->addr;
  msg[2] = 0x34;
  msg[3] = dir?0x96:0x86;
  msg[4] = (unsigned char)vel;
  msg[5] = dcn->acc;
  msg[6] = checksum( msg+1, 5 );

  dcn_pwrite( fd, msg, 7 );

//0xaa 0x1 0x66 0x4 0x0 0x0 0x0 0x0 0xff 0x6a 
//0xaa 0x1 0x74 0x87 0x32 0x0 0x0 0x0 0xed 0x2 0x1d 
//
  num = arm_timeout_read( fd, reply, 2 );
  if( num != 2 || reply[0] != reply[1] ) {
    ERRMSG("checksum error in dcn_motor_vel\n");
    return -1;
  } else
    return 0;
}

int dcn_motor_wait_stop( struct DCN *dcn )
{
  int i;
  unsigned char status;

  pthread_mutex_lock(&dcn_header->mutex);
  status = dcn_motor_stat_only( dcn );
  pthread_mutex_unlock(&dcn_header->mutex);

  for( i=0; i<3000; i++ ) {
    if( (status & DCN_MOTOR_MOVING)==0 ) {
      break;
    }
    pthread_mutex_lock(&dcn_header->mutex);
    status = dcn_motor_stat_only( dcn );
    pthread_mutex_unlock(&dcn_header->mutex);
    usleep(100000);
  }

 pthread_mutex_unlock(&dcn_header->mutex);
 return 0;
}

/*
  request all status bits,
 
  send 0xaa 0x1 0x13 0x7f 0x93
  resp 0x18 0x0 0x0 0x0 0x0 0x0 0x0 0x0 0x0 0x0 0x0 0x0 0x0 0x3 0x36 0x0 0x51

  returns 15 bytes:
  POS 4 bytes
  AD 1 byte
  ST 2 bytes
  INBYTE 1 byte
  HOME 4 bytes
  ID 2 bytes ( really mod and ver )
  OUT 1 byte
*/

unsigned short dcn_motor_status( struct DCN *dcn )
{
  unsigned char msg[256];
  unsigned char reply[256];
  reply[0] = 255;
  union {
    int st;
    unsigned char c[4];
    unsigned short s[2];
  } u;
  int num;
  int fd;

  fd = dcn_header->fd;

  /* send 0xaa 0x1 0x13 0x7f 0x93 */
  msg[0] = 0xaa;
  msg[1] = dcn->addr;
  msg[2] = 0x13;
  msg[3] = 0x7f;
  msg[4] = checksum( msg+1, 3 );

  pthread_mutex_lock(&dcn_header->mutex);
  dcn_pwrite( fd, msg, 5 );
  num = arm_timeout_read( fd, reply, 17 );
  pthread_mutex_unlock(&dcn_header->mutex);
  

  if( checksum(reply, 16 ) != reply[16] ) {
    WARNMSG("warning status checksum error\n");
  }

  /* decode response */

  u.c[0] = reply[1];
  u.c[1] = reply[2];
  u.c[2] = reply[3];
  u.c[3] = reply[4];
  dcn->position = u.st;
  
  dcn->ad = reply[5];

  u.c[0] = reply[6];
  u.c[1] = reply[7];
  dcn->status = u.s[0];

/* skip home and id bytes */

  dcn->inbyte = reply[8];
  dcn->out = reply[15];

  if(dcn_header->verbose) printf("read status 0x%x\n", reply[0] );

  return reply[0];
}


int dcn_motor_enable( struct DCN *dcn )
{
  unsigned char msg[10];
  unsigned char reply[10];
  int fd, addr, num;

  dcn->con->active = dcn;

  fd = dcn_header->fd;
  addr = dcn->addr;

  msg[0] = 0xaa;
  msg[1] = addr;
  msg[2] = 0x17;
  msg[3] = 0x9;
  msg[4] = checksum( msg+1, 3 );
  if( dcn_pwrite( fd, msg, 5 ) != 5 )
    ERRMSG("enable write error");

  num = arm_timeout_read( fd, reply, 2 );
  if( num != 2 || reply[0] != reply[1] )
  {
      ERRMSG("read error in enable");
    return -1;
  }
  else
    return 0;
}

int dcn_motor_disable( struct DCN *dcn )
{
  unsigned char msg[10];
  unsigned char reply[10];
  int num, fd, addr;
  fd = dcn_header->fd;
  addr = dcn->addr;

  dcn_motor_enable( dcn );

  msg[0] = 0xaa;
  msg[1] = addr;
  msg[2] = 0x17;
  msg[3] = 0x8;
  msg[4] = checksum( msg+1, 3 );
  if( dcn_pwrite( fd, msg, 5 ) != 5 )
    ERRMSG("disable write error\n");

  num = arm_timeout_read( fd, reply, 2 );
  
  if( num != 2 || reply[0] != reply[1] )
    return -1;
  else
    return 0;
}

void append_dcn( struct DCN_HEADER *head, struct DCN *dcn )
{
  struct DCN *d, *last;

  d = head->dcn;
  if( !d ) {
    head->dcn = dcn;
  } else {
    while( d ) {
      last = d;
      d = d->next;
    }
    last->next  = dcn;
  }
}


static int clear_pos( int fd, int addr )
{
  unsigned char msg[10];
  unsigned char reply[10];
  int num;

/* Clear 0xaa 0x1 0x0 0x1 */

  msg[0] = 0xaa;
  msg[1] = addr;
  msg[2] = 0;
  msg[3] = checksum( msg+1, 2);

  if( dcn_pwrite( fd, msg, 4 ) != 4 )
    ERRMSG("disable write error\n");

  num = arm_timeout_read( fd, reply, 2 );
  
  if( num != 2 || reply[0] != reply[1] )
    return -1;
  else
    return 0;
}

int dcn_motor_stop( struct DCN *dcn, int issmooth )
{
  unsigned char msg[10];
  unsigned char reply[10];
  int num, fd;

  dcn->con->active = dcn;

/* Stop smooth 0xaa 0x1 0x17 0x9 0x21 */
/* Stop abrupt 0xaa 0x1 0x17 0x5 0x1d */
/* addr 3: 0xaa 0x3 0x17 0x5 0x1f */

  fd = dcn_header->fd;
//  printf("dcn_stop addr %d unit %d\n", dcn->addr, dcn->unit );

  msg[0] = 0xaa;
  msg[1] = dcn->addr;
  msg[2] = 0x17;
  if( issmooth )
    msg[3] = 0x09;
  else
    msg[3] = 0x05;

  msg[4] = checksum( msg+1, 3);

  pthread_mutex_lock(&dcn_header->mutex);
  if( dcn_pwrite( fd, msg, 5 ) != 5 )
    ERRMSG("stop write error\n");

  num = arm_timeout_read( fd, reply, 2 );
  pthread_mutex_unlock(&dcn_header->mutex);
  
  if( num != 2 || reply[0] != reply[1] ) {
    ERRMSG("stop write reply error\n");
    return -1;
  } else
    return 0;
}

unsigned char dcn_motor_stat_only( struct DCN *dcn )
{
  unsigned char msg[32];
  unsigned char reply[256];
  int num;
  int fd;

  fd = dcn_header->fd;

  /* send 0xaa 0x1 0x13 0x00 0x14 */
  msg[0] = 0xaa;
  msg[1] = dcn->addr;
  msg[2] = 0x13;
  msg[3] = 0x0;
  msg[4] = checksum( msg+1, 3 );

  dcn_pwrite( fd, msg, 5 );
  num = arm_timeout_read( fd, reply, 2 );

  if( reply[0] != reply[1] ) {
    WARNMSG("warning status checksum error\n");
  }
  dcn->status = reply[0];

  return reply[0];
}

void dcn_motor_set_params( struct DCN *dcn, unsigned char mode, 
  unsigned char vel, unsigned char acc )
{
  if( vel<1 ) 
    vel = 1;
  if( vel > 250 )
    vel = 250;
  if( acc < 1 )
    acc = 1;
  if( acc> 250 )
    acc = 250;

  dcn->vel = vel;
  dcn->acc = acc;
  dcn->mode = mode;
}

void dcn_verbose( int on )
{
  dcn_header->verbose = on!=0;
}

/* select motor outputs e.g. 0xaa 0x1 0x18 0x0 0x19  */
/* must be done when disabled */

int dcn_set_outputs( struct DCN *dcn, unsigned char out )
{
  unsigned char msg[10];
  unsigned char reply[10];
  int num, fd;

  dcn->con->active = dcn;
  fd = dcn_header->fd;
  dcn->con->active = dcn;

  msg[0] = 0xaa;
  msg[1] = dcn->addr;
  msg[2] = 0x18;
  msg[3] = out;
  msg[4] = checksum( msg+1, 3 );

  dcn_pwrite( fd, msg, 5 );
  num = arm_timeout_read( fd, reply, 2 );

  if( num != 2 || reply[0] != reply[1] )
  {
    ERRMSG("checksum error in set_outputs\n");
    return -1;
  }

  return 0;
}

/* build an array of uniq addrs from the dcn list */
/* the array is num long */

void build_uniq_addr( struct DCN *dcn, int *array, int num )
{
  int addr, i, ct, tot, inar;
  struct DCN *d;

  addr = -1;
  tot = 0;
  for( i=0; i<num; i++ ) {
    d = dcn;
    while(d) {
      inar = 0;
      for( ct=0; ct<tot; ct ++ ) {
        if( d->addr == array[ct] )
          inar = 1;
      }
         
      if( inar == 0 ) {
        array[i] = d->addr;
        addr = d->addr;
        tot++;
        break;
      }
      d = d->next;
    }
  }
}

//Moved from test_dcn_c (JRM)

struct DCN *find_dcn_motor( struct DCN_HEADER *head, int addr, int unit )
{
  struct DCN *motor;

  motor = head->dcn;

  while( motor ) {
    if( motor->addr == addr && motor->unit == unit )
       break;
    motor = motor->next;
  }

  return motor;
}



int serial_init(const char *address, int port)
{
   int err= 0;
   struct sockaddr_in servaddr;
   int sockfd;
      
   sockfd = socket( AF_INET, SOCK_STREAM, IPPROTO_TCP);
   if (sockfd == -1)
      return -1;
      
   struct hostent *h = gethostbyname(address);
      
   servaddr.sin_family=AF_INET;
   servaddr.sin_port=htons(port);
   memcpy( &servaddr.sin_addr, h->h_addr_list[0], h->h_length);
   
   err = connect( sockfd, (struct sockaddr *) &servaddr, sizeof(servaddr));
   
   if(err)
   {
      ERRMSG("error attempting to connect to serial port over TCP/IP socket");
      serial_close(sockfd);
      return err;
   }
   
   return sockfd;
}

int serial_close(int sockfd)
{
   shutdown( sockfd, 2);
   close(sockfd);
   return 0;
}

