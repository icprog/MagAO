

#include "env/chardev.h"
#include <sys/time.h>

#include <cerrno>
#include <cstdlib>


namespace Arcetri {

    chardev::chardev(string remoteHost, int remotePort, int logLevel):
        TcpConnection(remoteHost, remotePort, logLevel)
    {
    }

    chardev::~chardev() {
    }

    void chardev::set_terminator(const char *terminator) {
        this->terminator = terminator;
    }

    void chardev::set_echo(bool echo) {
        this->echo = echo;
    }

    void chardev::set_check_echo(bool check_echo) {
        this->check_echo = check_echo;
    }

    void chardev::putch(char c, long timeout_ms) {
        // TODO: timeout not implemented yet
        sendTcp((BYTE*)&c, 1);
        msleep(CHARDEV_PUTC_MSLEEP);
    }

    void chardev::write_line(const char *text, long timeout_ms) {
        const char *data[] = { text, terminator, NULL };
        for(int x = 0; data[x] != NULL; x++) 
            for(int y = 0; data[x][y] != '\0'; y++)  {
                putch(data[x][y],timeout_ms);
            }
    }

    char chardev::getc(long timeout_ms) {

        BYTE c;
        
        size_t bytes = receiveTcp(&c,sizeof(BYTE), timeout_ms);

        if(bytes < sizeof(BYTE)) 
	  throw AOException("recv error", COMMUNICATION_ERROR);

        return (char)c;
    }

    const char *chardev::_read_line(long timeout_ms) {

        unsigned int counter = 0;
	struct timeval time_start, time_now;
        static char buffer[1024];
        buffer[0]='\0';

	gettimeofday(&time_start, NULL);

        while(1) {
            if(counter + 1 > sizeof(buffer))
	      throw AOException("buffer overflow", TOOMANY_ITEMS_ERROR);

            buffer[counter] = getc();

            while(buffer[counter] == '\r' || buffer[counter] == '\0') {
                buffer[counter] = getc(timeout_ms);
            }

            if(buffer[counter] == '\n') {
                buffer[counter] = '\0'; 
                return buffer;
            }

	    gettimeofday(&time_now, NULL);

	    long diff = (time_now.tv_sec*1000 + time_now.tv_usec/1000) - 
	      (time_start.tv_sec*1000 + time_start.tv_usec/1000);
	    
	    if (diff > timeout_ms) {
	      char msg[100];
	      snprintf(msg, 100, "timeout (%ld > %ld ms)", diff, timeout_ms);
	      throw AOException(msg, TIMEOUT_ERROR);
	    }
            counter++;
        }
    }

    const char *chardev::read_line(long timeout_ms) {
        const char *line;
        for(;;) {
	  try {
	    // skip empty lines
            line=_read_line(timeout_ms);
            if(line[0]!='\0') 
	      break;
	  }
	  catch (...) {
	    throw;
	  }
        }
        return line;
    }

} /* end of namespace */
