/*
 * sensorcheck
 *
 * A daemon that periodically logs sensor information to syslog and MsgD-RTBD
 *
 * Based on sensord by Merlin Hughes
 *
 * Copyright (c) 2004 INAf - Osservatorio di Arcetri
 * Copyright (c) 1999-2002 Merlin Hughes <merlin@merlin.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <limits.h>
#include <string.h>
#include <signal.h>
#include <syslog.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "sensord.h"

#ifdef RTDB
// MsgD-RTDB header files
#include "errlib.h"
#include "msglib.h"
#include "rtdblib.h"

char *MyName = "Sensors";	// The name used to register with the MsgD-RTDB server
char MyHostname[32];		// This computer hostname
#endif

static int logOpened = 0;

static volatile sig_atomic_t done = 0;

#define LOG_BUFFER 4096

#include <stdarg.h>

void
sensorLog
(int priority, const char *fmt, ...) {
  static char buffer[1 + LOG_BUFFER];
  va_list ap;
  va_start (ap, fmt);
  vsnprintf (buffer, LOG_BUFFER, fmt, ap);
  buffer[LOG_BUFFER] = '\0';
  va_end (ap);
  if (debug || (priority < LOG_DEBUG)) {
    if (logOpened) {
      syslog (priority, "%s", buffer);
    } else {
      fprintf (stderr, "%s\n", buffer);
      fflush (stderr);
    }
  }
}

static void
signalHandler
(int sig) {
  signal (sig, signalHandler);
  switch (sig) {
    case SIGTERM:
      done = 1;
      break;
  }
}

static int
sensord
(void) {
  int ret = 0;
  int scanValue = 0, logValue = 0, rrdValue = 0;

  sensorLog (LOG_INFO, "sensord started");

  while (!done && (ret == 0)) {
    if (ret == 0)
      ret = reloadLib ();
    if ((ret == 0) && scanTime) { /* should I scan on the read cycle? */
      ret = scanChips ();
      if (scanValue <= 0)
        scanValue += scanTime;
    }
    if ((ret == 0) && logTime && (logValue <= 0)) {
      ret = readChips ();
      logValue += logTime;
    }
    if ((ret == 0) && rrdTime && rrdFile && (rrdValue <= 0)) {
      ret = rrdUpdate ();
      rrdValue += rrdTime;
    }
    if (!done && (ret == 0)) {
      int a = logTime ? logValue : INT_MAX;
      int b = scanTime ? scanValue : INT_MAX;
      int c = (rrdTime && rrdFile) ? rrdValue : INT_MAX;
      int sleepTime = (a < b) ? ((a < c) ? a : c) : ((b < c) ? b : c);
      sleep (sleepTime);
      scanValue -= sleepTime;
      logValue -= sleepTime;
      rrdValue -= sleepTime;
    }
  }

  sensorLog (LOG_INFO, "sensord %s", ret ? "failed" : "stopped");

  return ret;
}

static void
openLog
(void) {
  openlog ("sensord", 0, syslogFacility);
  logOpened = 1; 
}

static void
daemonize
(void) {
  int pid;
  struct stat fileStat;
  FILE *file;

  if (chdir ("/") < 0) {
    perror ("chdir()");
    exit (EXIT_FAILURE);
  }

  if (!(stat (pidFile, &fileStat)) &&
      ((!S_ISREG (fileStat.st_mode)) || (fileStat.st_size > 11))) {
    fprintf (stderr, "Error: PID file `%s' already exists and looks suspicious.\n", pidFile);
    exit (EXIT_FAILURE);
  }
 
  if (!(file = fopen (pidFile, "w"))) {
    fprintf (stderr, "fopen(\"%s\"): %s\n", pidFile, strerror (errno));
    exit (EXIT_FAILURE);
  }
  
  /* I should use sigaction but... */
  if (signal (SIGTERM, signalHandler) == SIG_ERR) {
    perror ("signal(SIGTERM)");
    exit (EXIT_FAILURE);
  }

  if ((pid = fork ()) == -1) {
    perror ("fork()");
    exit (EXIT_FAILURE);
  } else if (pid != 0) {
    fprintf (file, "%d\n", pid);
    fclose (file);
    exit (EXIT_SUCCESS);
  }

  if (setsid () < 0) {
    perror ("setsid()");
    exit (EXIT_FAILURE);
  }

  fclose (file);
  close (STDIN_FILENO);
  close (STDOUT_FILENO);
  close (STDERR_FILENO);
}

static void 
undaemonize
(void) {
  unlink (pidFile);
  closelog ();
}

int
main
(int argc, char **argv) {
  int ret = 0;
  
  if (parseArgs (argc, argv) ||
      parseChips (argc, argv))
    exit (EXIT_FAILURE);
  
  if (initLib () ||
      loadLib ())
    exit (EXIT_FAILURE);

  if (isDaemon)
    openLog ();
  if (rrdFile)
    ret = rrdInit ();
  
  if (ret) {
  } else if (doCGI) {
    ret = rrdCGI ();
  } else if (isDaemon) {
    daemonize ();

#ifdef RTDB
// This section has been added to use this program with MsgD_RTDB
    if (doMsgD_RTDB) {
      int stat;
      gethostname (MyHostname, 32);
      if ((stat = ConnectToServer ((char *)MsgD_RTDBserver, MyName, 1)) !=NO_ERROR) {
        sensorLog (LOG_ERR, "Error connecting to MsgD-RTDB server at: %s\n", MsgD_RTDBserver);
        sensorLog (LOG_ERR, "Error code: %d - %s\n\n", stat, strerror(errno));
        undaemonize ();
        exit (EXIT_FAILURE);
      } else
        sensorLog (LOG_INFO, "Connected to MsgD-RTDB server at: %s\n", MsgD_RTDBserver);
    }
#endif

    ret = sensord ();
    undaemonize ();
  } else {

#ifdef RTDB
// This section has been added to use this program with MsgD_RTDB
    if (doMsgD_RTDB) {
      int stat;
      gethostname (MyHostname, 32);
      if ((stat = ConnectToServer ((char *)MsgD_RTDBserver, MyName, 1)) !=NO_ERROR) {
        sensorLog (LOG_ERR, "Error connecting to MsgD-RTDB server at: %s\n", MsgD_RTDBserver);
        sensorLog (LOG_ERR, "Error code: %d - %s\n\n", stat, strerror(errno));
        undaemonize ();
        exit (EXIT_FAILURE);
      } else
        sensorLog (LOG_INFO, "Connected to MsgD-RTDB server at: %s\n", MsgD_RTDBserver);
    }
#endif

    if (doSet)
      ret = setChips ();
    else if (doScan)
      ret = scanChips ();
    else if (rrdFile)
      ret = rrdUpdate ();
    else
      ret = readChips ();
  }
  
  if (unloadLib ())
    exit (EXIT_FAILURE);
  
  return ret;
}
