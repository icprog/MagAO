#ifndef CHARDEV_H
#define CHARDEV_H

#include "TcpConnection.h"
#include <stdexcept>

#include <ctime>

#define CHARDEV_PUTC_MSLEEP 50
#define CHARDEV_TIMEOUT_DEFAULT 5000L

namespace Arcetri {

    class chardev: public TcpConnection {

      private:
        const char *_read_line(long timout_ms);
      protected:
        const char *terminator;
        bool echo;
        bool check_echo;

      public:

        chardev(string remoteHost, int remotePort, int logLevel = Logger::LOG_LEV_WARNING);
        ~chardev();

        void set_terminator(const char *);
        void set_echo(bool);
        void set_check_echo(bool);

        void putch(char, long timeout_ms = CHARDEV_TIMEOUT_DEFAULT);
        char getc(long timeout_ms = CHARDEV_TIMEOUT_DEFAULT);
        void write_line(const char *, long timeout_ms = CHARDEV_TIMEOUT_DEFAULT);
        const char *read_line(long timout_ms = CHARDEV_TIMEOUT_DEFAULT);

    };

} /* end of namespace */

#endif
