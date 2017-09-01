#ifndef UTILS_H_INCLUDE
#define UTILS_H_INCLUDE

#include <sys/time.h>

#include <string>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <sstream>
#include <vector>
#include <sys/time.h>
#include <sys/stat.h>
#include <ctype.h>
#include <errno.h>

using namespace std;

#include "aotypes.h"

// Root namespace
namespace Arcetri {


    class Utils {
	
      public:
        static string itoa(float f);
        static string itoa(int f);
        
        /*
         * Convert an integer to a string, formatting
         * the number to a given number of digits.
         * Es. itoa(32, 3) = 032
         */
        static string itoa(int f, unsigned int digits);
        
        /*
         * Convert a float to a string, formatting
         * the number to a given number of digits and precision.
         * Es. itoa(32.1234, 5,2) = 32.12
         */
        static string itoa(float f, unsigned int digits, unsigned int precision);
        
        /*
         * Convert an integer to a string that represent it
         * in binary format.
         * 
         * i.e. 12 -> 1100
         */
        static void itoStrBin(char* str, uint32 v);
        
        /*
         * Set the real time scheduling fro the caling process
         * 
         * The max allowed priority is 99
         */
        static void setSchedulingRR(int priority);
        
        /*
         * Return the value of ADOPT_LOG environment variable.
         * If not exists, returns an empty string.
         */
        static string getAdoptLog();
        
        /*
         * Return the value of ADOPT_ROOT environment variable.
         * If not exists, returns an empty string.
         */
        static string getAdoptRoot();
        
        /*
         * Return the value of ADOPT_SIDE environment variable.
         * If not exists, returns an empty string.
         */
        static string getAdoptSide();
        
        /*
         * Return the value of ADOPT_SUBSYSTEM environment variable.
         * If not exists, returns an empty string.
         */
        static string getAdoptSubsystem();

        /*
         * Return the date and time as ascii string
         */
        static string asciiDateAndTime();
	    
        /*
         * Return the date and time as short ascii string
         */
        static string loggerAsciiDateAndTime();

        /*
         *
         * Return the date as a human-friendly string.
         * Uses the date passed as an argument or, if 0, the current date.
         */
        static string printableDate( time_t t);

        /*
         * Return the date and time as a compact ascii string suitable for file names
         */
        static string asciiDateAndTimeCompact();

	    /*
	     * Return the C function 'time' as string of 14 digits
	     */
	    static string timeAsString();
	    
	    /*
	     * Return the C function 'time' as ISO string YYYY-MM-DDThh:mm:ss
	     */
	    static string timeAsIsoString();

        /*
         * Return the 'time' from an ISO string YYYY-MM-DDThh:mm:ss
         */
        static time_t timeFromIsoString(const string &time);

	    /*
	     * Return true only if the given string is a valid filename
	     */
        static bool fileExists(string filename);
	    
	    /*
	     * Return a unique file name with the given prefix.
	     * With empty argument will use the app invocation name
	     */
	    static string uniqueFileName(string prefix = "");

	    /*
	     * Return the a copy of the string in lowercase
	     */
	    static string lowercase(const string s);
	    
	    /*
	     * Return the a copy of the string in uppercase
	     */
	    static string uppercase(const string s);

        /* Returns whether a string ends in the specified extension */
        static bool hasExtension(const string s, const string extension);

        // Adds an extension to a filename, preventing multiple adding
        static string addExtension( string s, string extension);

        // Reads a directory and returns a vector of filenames 
        static int readDir( string dirname, vector<string> &files);


        /* Extracts the timestamp (tracking number) from a filename */
        static string getTracknum( string filename);

        /* Returns whether the dummy environment variable is set.
           When this variable is set, no communication with hardware
           should be attempted */
        static bool isDummy();
       
        //
        // Perform a matrix multiplication by a Vector
        // Result         
        //
        static void matrixVect(double* matr, double* vect, double* res, long n_row, long n_column);


        // Returns a deep-copied string, ensuring that the contents are duplicated instead of reference-counted

        static string deepCopy( string s);

            // Returns time diff in sec
        static double timediff(struct timeval *t1, struct timeval *t2);

	// Returns a properly formatted client name, with optional domain
	static string getClientname( string client, bool addSide=true, string domain="");


        // Exec a command and get the stdout into a string
        static string exec( const char *cmd);

	// Get info about a network interface
        static int getMacAddr( string ifName, string &mac, string &ip, string &netmask);

        // Check that two IP addresses belong to the same subnet
        static bool matchIP( string ip1, string ip2, string netmask);

        // Returns the configuration file full path given the AOApp identity
        static string getConffile( string identity);

        // Resolves a hostname
        static string resolveHost( string host);


	
    };

}

#endif
