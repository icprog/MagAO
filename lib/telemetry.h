//@File: telemetry.h
//
// Interface to telemetry library
//@

#ifndef TELEMETRY_H
#define TELEMETRY_H

//@Function: list
//
// Returns a list of available telemetry variable names.
// Last element is NULL.
// [TODO] memory management.
//@

char **telemetry_list(void);

//@Function: get
//
// Returns an array of telemetry values.
// Array must be freed by user program.
//@

double *telemetry_get( char *varname, int start_epoch, int end_epoch);








#endif // TELEMETRY_H
