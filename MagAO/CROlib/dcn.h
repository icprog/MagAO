

#ifndef __dcn_h__
#define __dcn_h__

#define DEBUG 1



#ifdef __cplusplus
extern "C"
{
#endif

#include <netdb.h>
//#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <pthread.h>

/* jeffs version, dcn motor control */

#define DCN_STEPMODTYPE 3
#define DCN_STEP_DEFVEL 112 /* max 250 */
#define DCN_STEP_DEFACC 250

/* status word */
#define DCN_MOTOR_MOVING    0x01 
#define DCN_CKSUM_ERROR     0x02 
#define DCN_AMP_ENABLED     0x04 
#define DCN_POWER_ON        0x08 
#define DCN_AT_SPEED        0x10 
#define DCN_VEL_MODE        0x20
#define DCN_TRAP_MODE       0x40
#define DCN_HOME_IN_PROG    0x80

//7
#define DCN_STEP_MODE1X   (0x04|0x03)
//6
#define DCN_STEP_MODE2X   (0x04|0x02)
//5
#define DCN_STEP_MODE4X   (0x04|0x01)
//4
#define DCN_STEP_MODE8X   (0x04|0x00)    

#define ERRHAND
#define WARNHAND

#ifndef ERRHAND
///Error reporting macro. If not using the function __error_message(msg) then use stderr.
#define ERRMSG(msg) fprintf(stderr, "%s (%s line %i)\n", msg,__FILE__,__LINE__)
#else
extern void dcn_error_message(const char *, const char *, int);
#define ERRMSG(msg) dcn_error_message(msg, __FILE__, __LINE__)
#endif

#ifndef WARNHAND
///Error reporting macro. If not using the function __error_message(msg) then use stderr.
#define WARNMSG(msg) fprintf(stderr, "%s (%s line %i)\n", msg,__FILE__,__LINE__)
#else
extern void dcn_warning_message(const char *, const char *, int);
#define WARNMSG(msg) dcn_warning_message(msg, __FILE__, __LINE__)
#endif


/* one for each motor */

struct DCN {
  struct DCN *next;
  struct DCN_CONTROLLER *con;
  unsigned char addr;  /* address of motor controller */
  unsigned char unit; /* which motor: 0,A, 1,B or 2,C */
  unsigned char mode;

  int position;
  unsigned short st;
  unsigned char status;
  unsigned char vel;
  unsigned char acc;
  unsigned char modtype;
  unsigned char modver;
  unsigned char ad;
  unsigned char out;
  unsigned short inbyte;
};

#define DCN_MAX_UNITS 3 /* max units per controller */

struct DCN_CONTROLLER {
  struct DCN_CONTROLLER *next;
  struct DCN *dcn[DCN_MAX_UNITS];     /* list of dcn for this controller */
  struct DCN *active;  /* when active, dcn */
  unsigned char addr;
  unsigned char status;
};

struct DCN_HEADER {
  int fd;
  int verbose;
  int num_addrs; /* number of discovered addresses starting at 1 */
  struct DCN *dcn;
  struct DCN_CONTROLLER *con;

  pthread_mutex_t mutex;
};


///Construct and initialize the header.
/** \param verbose sets how much output is reported to stdout by dcn_pwrite, etc.
  * \param addr is either the filename of a tty or an IP address (which determined by port)
  * \param port if 0, then addr is assumed to by a tty, otherwise is the TCP/IP port
  * \retval pointer to the structure, 0 on error.
  */
struct DCN_HEADER *dcn_init(int verbose, const char *addr, int port);

///Resets the dcn static variables and sets pointers to NULL.
void dcn_shutdown();

void dcn_motor_set_params( struct DCN *dcn, unsigned char mode, unsigned char vel, unsigned char acc );

///Move the motor dcn rel steps.
/** Locks the header mutex first.
 */
int dcn_motor_move_rel( struct DCN *dcn, int rel );

int dcn_motor_stop( struct DCN *dcn, int issmooth );

int dcn_motor_enable( struct DCN *dcn );

int dcn_motor_disable( struct DCN *dcn );

///Wait for the motor dcn to report stopped, periodically polling.
/** Locks the header mutex before each poll.
 */
int dcn_motor_wait_stop( struct DCN *dcn );

///Query the motor status
/** Locks the header mutex before I/O to controller.
 */
unsigned short dcn_motor_status( struct DCN *dcn );

unsigned char dcn_motor_stat_only( struct DCN *dcn );

void dcn_verbose( int on );

int dcn_set_outputs( struct DCN *dcn, unsigned char out );

int dcn_flush(void);

int dcn_motor_vel( struct DCN *dcn, int vel );


//Moved from test_dcn.c (JRM)
struct DCN *find_dcn_motor( struct DCN_HEADER *head, int addr, int unit );

//Returns file descriptor of socket.
int serial_init(const char *address, int port);
int serial_close(int sockfd);

#ifdef __cplusplus
} // extern "C"
#endif


#endif  //__dcn_h__
