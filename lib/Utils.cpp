#include "Utils.h"
#include "Logger.h"
#include <cstdlib>
using namespace Arcetri;

#include "math.h"
#include <iomanip>

#include <netdb.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <arpa/inet.h>

#include <sys/types.h>
#include <dirent.h>
#include <errno.h>


string Utils::itoa(int f) {
    stringstream ss;
    string str;
    ss << f;
    ss >> str;
    return str;
}

string Utils::itoa(int f, unsigned int digits) {
    stringstream ss;
    string str;
    ss << f;
    ss >> str;
    if(str.size() < digits) {
    	for(unsigned int i = 0; i < digits-str.size(); i++) {
    		str = "0" + str;
    	}
    }
    return str;
}

string Utils::itoa(float f, unsigned int digits, unsigned int precision) {
    stringstream ss;
    string str;
    ss << fixed << setw(digits) << setprecision(precision);
    ss << f;
    ss >> str;
    return str;
}


string Utils::itoa(float f) {
	stringstream ss;
	string str;
	ss << f;
	ss >> str;
	return str;
}

void Utils::itoStrBin(char* str, uint32 v) {
	for(unsigned int i=0; i<32; i++) {
		str[32-i] = (char)(48 + v%2);
		v = v/2;
	}
}

void Utils::setSchedulingRR(int priority) {

	int ret;
	struct sched_param param;
    param.sched_priority = priority;
    if((ret = sched_setscheduler(0, SCHED_RR, &param)) != 0) {
        Logger::get()->log(Logger::LOG_LEV_WARNING, "Real-time priority not set !");
    }
    else {
    	struct sched_param my_sched_param;
  	sched_getparam(0, &my_sched_param);
        Logger::get()->log(Logger::LOG_LEV_INFO, "Real-time priority (%d) succesfully set !", my_sched_param.sched_priority);
    }
}

string Utils::getAdoptLog() {
	string logDir = "";
	char* adoptLog = getenv("ADOPT_LOG");
    if (adoptLog == NULL) {
        // Removed because it segfaults during static initialization
        //Logger::get()->log(Logger::LOG_LEV_ERROR, "Environment variable ADOPT_LOG not set! [%s:%d]", __FILE__, __LINE__);
    } else {
		logDir = adoptLog;
	}
	return logDir;
}

string Utils::getAdoptSide() {
	string Side = "";
	char* adoptSide = getenv("ADOPT_SIDE");
    if (adoptSide == NULL) {
        Logger::get()->log(Logger::LOG_LEV_ERROR, "Environment variable ADOPT_SIDE not set! [%s:%d]", __FILE__, __LINE__);
    } else {
		Side = adoptSide;
	}
	return Side;
}

string Utils::getAdoptRoot() {
	string Root = "";
	char* adoptRoot = getenv("ADOPT_ROOT");
    if (adoptRoot == NULL) {
        Logger::get()->log(Logger::LOG_LEV_ERROR, "Environment variable ADOPT_ROOT not set! [%s:%d]", __FILE__, __LINE__);
    } else {
		Root = adoptRoot;
	}
	return Root;
}

string Utils::getAdoptSubsystem() {
	string Subsystem = "";
	char* adoptSubsystem = getenv("ADOPT_SUBSYSTEM");
    if (adoptSubsystem == NULL) {
        adoptSubsystem = (char *)"WFS";
        Logger::get()->log(Logger::LOG_LEV_WARNING, "Environment variable ADOPT_SUBSYSTEM not set! Default to WFS [%s:%d]", __FILE__, __LINE__);
    } else {
		Subsystem = adoptSubsystem;
	}
	return Subsystem;
}

/*
 * Return the date and time as ascii string
 */
string Utils::asciiDateAndTime() {
    struct timeval tv;
    struct tm *lt;
    char strtime[27];

    gettimeofday(&tv, NULL);

    lt=localtime((time_t *)&tv.tv_sec);


    snprintf( strtime, 27, "%04d-%02d-%02d %02d:%02d:%02d.%06d",
              lt->tm_year+1900, lt->tm_mon+1,lt->tm_mday,
              lt->tm_hour,lt->tm_min,lt->tm_sec,(int)tv.tv_usec);

    return string(strtime,26);
}


/*
 * Return the date and time as short ascii string
 */
string Utils::loggerAsciiDateAndTime() {
    struct timeval tv;
    struct tm *lt;
    char strtime[23];	// Consider the null termination!

    gettimeofday(&tv, NULL);
	
    lt=localtime((time_t *)&tv.tv_sec);
	
    snprintf( strtime, 23, "%04d%02d%02d %02d:%02d:%02d.%04d",
              lt->tm_year+1900, lt->tm_mday,lt->tm_mon+1, 
              lt->tm_hour,lt->tm_min,lt->tm_sec,(int)(tv.tv_usec/100));

    return string(strtime);
}


/*
 *
 * Return the date as a human-friendly string.
 * Uses the date passed as an argument or, if 0, the current date.
 */
string Utils::printableDate( time_t t) {
    char str[10];

    if (t == 0)
        t = time(NULL);
    struct tm *tm = gmtime(&t);

    strftime( str, 11, "%Y-%m-%d", tm);
    return str;
}


/*
 * Return the date and time as a compact ascii string suitable for file names
 */
string Utils::asciiDateAndTimeCompact() {
    struct timeval tv;
    struct tm *lt;
    char strtime[23];	// Consider the null termination!

    gettimeofday(&tv, NULL);
	
    lt=localtime((time_t *)&tv.tv_sec);
	
    snprintf( strtime, 23, "%04d%02d%02d_%02d%02d%02d",
              lt->tm_year+1900, lt->tm_mon+1,lt->tm_mday, 
              lt->tm_hour,lt->tm_min,lt->tm_sec);

    return string(strtime);
}


/*
 * Return the C function 'time' as string of 14 digits
 */
string Utils::timeAsString() {
    char strtime[20];
    time_t curTime;
    time(&curTime);
    snprintf(strtime, 15, "%014d", (int)curTime);
    return string(strtime);
}

/*
 * Return the C function 'time' as ISO string YYYY-MM-DDThh:mm:ss
 */
string Utils::timeAsIsoString() {
    char strtime[20];
    time_t curTime;

    time(&curTime);
    strftime(strtime, 20, "%Y-%m-%dT%H:%M:%S", localtime(&curTime));
    return string(strtime);
}


/*
 * Return the 'time' from an ISO string YYYY-MM-DDThh:mm:ss
 */
time_t Utils::timeFromIsoString(const string &time) {
    
    time_t ret = -1;
    struct tm tm;

    if (strptime(time.c_str(), "%Y-%m-%dT%H:%M:%S", &tm) != NULL) {
        tm.tm_isdst = -1;
        ret = mktime(&tm);
    }
    return ret;
}

    


/*
 * Return true only if the given string is a valid filename
 */
bool Utils::fileExists(string filename) {
    struct stat fileStat;
    if(stat(filename.c_str(), &fileStat) == 0) {
        return true;
    }
    else {
        return false;
    }
}

/*
 * Return a unique file name with the given prefix.
 * With empty argument will use the app invocation name
 */
string Utils::uniqueFileName(string prefix) {
    if (prefix == "")
        return prefix + program_invocation_short_name + "_" + itoa(getpid());
    else
        return prefix + itoa(getpid());
}

/*
 * Return the a copy of the string in lowercase
 */
string Utils::lowercase(const string s) {
    const char* uppercaseCStr = s.c_str();
    char* lowercaseCStr = new char[s.length()+1];
    lowercaseCStr[s.length()] = '\0';
	    	
    for(unsigned int i=0; i<s.length(); i++) {
        lowercaseCStr[i] = tolower(uppercaseCStr[i]);
    }
    string lowercaseStr = string(lowercaseCStr);
    delete[] lowercaseCStr;
    return lowercaseStr;
}

/*
 * Return the a copy of the string in uppercase
 */
string Utils::uppercase(const string s) {
    const char* lowercaseCStr = s.c_str();
    char* uppercaseCStr = new char[s.length()+1];
    uppercaseCStr[s.length()] = '\0';
	    	
    for(unsigned int i=0; i<s.length(); i++) {
        uppercaseCStr[i] = toupper(lowercaseCStr[i]);
    }
    string uppercaseStr = string(uppercaseCStr);
    delete uppercaseCStr;
    return uppercaseStr;
}

/* Returns whether a string ends in the specified extension */
bool Utils::hasExtension(const string s, const string extension) {
    if (s.size() >= extension.size())
        if (s.substr( s.size() - extension.size()).compare(extension) == 0)
            return true;
    return false;
}

/* Extracts the timestamp (tracking number) from a filename */
string Utils::getTracknum( string filename) {

    if (filename.size()< 15)
        return "";
    return filename.substr( filename.size()-15, 15);
}


/* Returns whether the dummy environment variable is set.
   When this variable is set, no communication with hardware
   should be attempted */
bool Utils::isDummy() {
    return getenv("ADOPT_DUMMY") != NULL;
}


void Utils::matrixVect(double* matr, double* vect, double* res, long n_row, long n_column){
    //if(1) std::cout << "--- FunctModalProj matrix_vect()" << std::endl;
    memset(res, 0, n_row*sizeof(*res));
    double *a=matr;
    double *b=vect;
    double *b_end = b + n_column;
    double *c=res;
    double *c_end = c + n_row;
    for ( ; c < c_end; c++){
	double *b = vect;
        for ( ; b < b_end; )
            *c += *a++ * *b++;
    }
}

string Utils::deepCopy( string s) {
   
   string s1(s.size(),0);
   memcpy( (void*)s1.c_str(), s.c_str(), s.size());
   return s1;
}

double Utils::timediff(struct timeval *t1, struct timeval *t2) // Returns time diff in sec
{
    double ns= t1->tv_sec-t2->tv_sec;
    ns += (t1->tv_usec-t2->tv_usec)*1e-6;
    return ns;
}


string Utils::getClientname( string client, bool addSide, string domain)
{
	if (addSide) client += "."+getAdoptSide();
	if (domain != "") client += "@M_"+domain;
	return client;
}

string Utils::exec( const char *cmd) {
    char buffer[128];

    FILE *pipe = popen( cmd, "r");
    if (!pipe) return "";

    string output="";
    while (!feof(pipe)) {
        if (fgets(buffer, 128, pipe) != NULL)
            output += buffer;
    }
    pclose(pipe);
    return output;
}

string read_hwaddr(const struct ifreq &ifr) {

   ostringstream oss (ostringstream::out);
   const struct sockaddr &hwaddr=ifr.ifr_hwaddr ;
   for (size_t k=0 ; k<6 ; ++k) {
      if (k) oss << ':' ;
      unsigned int v=static_cast<unsigned char>(hwaddr.sa_data[k]) ;
      oss << hex << setw(2) << setfill('0') << static_cast<int>(v);
   }
   return oss.str();
}

string read_ip(const struct ifreq &ifr) {
   ostringstream oss (ostringstream::out);
   const struct sockaddr &addr=ifr.ifr_addr ;
   for (size_t k=0 ; k<4 ; k++) {
      unsigned int v=static_cast<unsigned char>(addr.sa_data[k+2]) ;
      if (k) oss << '.';

      oss << dec << static_cast<int>(v) ;
   }
   return oss.str();
}



int Utils::getMacAddr( string ifName, string &mac, string &ip, string &netmask) {

    //create a dummy socekt to request info
    struct protoent *proto= getprotobyname("tcp");
    if (!proto)
        return NETWORK_ERROR;

    int sock=socket(PF_INET, SOCK_STREAM, proto->p_proto) ;
    struct ifreq ifr;

    ostringstream oss (ostringstream::out);
    memset(&ifr, 0, sizeof(struct ifreq)) ;
    strcpy(ifr.ifr_name, ifName.c_str()) ;

    // MAC address
    oss  << ifName << " ";
    if (ioctl(sock, SIOCGIFHWADDR, &ifr)==0)
       mac = read_hwaddr(ifr);
    else
       mac = "";

    // IP address
    if (ioctl(sock, SIOCGIFADDR, &ifr)==0)
       ip = read_ip(ifr);
    else
       ip = "";

    // NETMASK
    if (ioctl(sock, SIOCGIFNETMASK, &ifr)==0)
       netmask =  inet_ntoa(((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr);
     else
       netmask = "";

    close(sock) ;
    //printf("Utils::getMacAddr(): ifName=%s ip=%s netmask=%s mac=%s\n", ifName.c_str(), ip.c_str(), netmask.c_str(), mac.c_str());
    return NO_ERROR;
}

bool Utils::matchIP( string ip1, string ip2, string netmask) {

    int addr1[4];
    int addr2[4];
    int mask[4];
    size_t pos1=0;
    size_t pos2=0;
    size_t pos3=0;
    int i;
    bool match = true;

    //printf("Utils::matchIP(): ip1=%s\nip2=%s\nmask=%s\n", ip1.c_str(), ip2.c_str(), netmask.c_str());

    if ((ip1=="") || (ip2=="") || (netmask==""))
       return false;

    for (i=0; i<4; i++) {
       if (pos1 != string::npos) {
           addr1[i] = atoi(ip1.substr(pos1).c_str());
           pos1 = ip1.find_first_of('.', pos1)+1;
       }
 
       if (pos2 != string::npos) {
           addr2[i] = atoi(ip2.substr(pos2).c_str());
           pos2 = ip2.find_first_of('.', pos2)+1;
       }
      
       if (pos3 != string::npos) {
           mask[i] = atoi(netmask.substr(pos3).c_str());
           pos3 = netmask.find_first_of('.', pos3)+1;
       }
    }

    for (i=0; i<4; i++) {
	//printf("i=%d addr[i]=%d  addr2[i]=%d mask[i]=%d\n", i, addr1[i], addr2[i], mask[i]);
        match = match && ((addr1[i] & mask[i]) == (addr2[i] & mask[i]));
    }
   
    return match;
}

// Returns the configuration file full path given the AOApp identity

string Utils::getConffile( string identity) {

   return getAdoptRoot()+"/conf/"+lowercase(getAdoptSubsystem())+"/current/processConf/"+identity+".conf";
}

// Resolves a hostname into an IP address

string Utils::resolveHost( string host) {

   struct in_addr sin_addr;

   if (inet_aton(host.c_str(), &sin_addr) ==0) {
      /* Not an IP address, resolve the name */
      struct hostent *he;
      he = gethostbyname(host.c_str());
      if (!he)
          return "";

      sin_addr = *(struct in_addr*)he->h_addr;
   }

   char buf[INET_ADDRSTRLEN];
   if (inet_ntop(AF_INET, &sin_addr, buf, 256) == NULL)
      return "";
   return string(buf);
}


string Utils::addExtension( string s, string extension) {

    if (s== "")
        return s;

    if ((s.size() < extension.size()) || (s.substr(s.size() - extension.size()) != extension))
        s += extension;

    return s;
}


int Utils::readDir( string dirname, vector<string> &files) {

   DIR *dp;
   struct dirent *dirp;
   if((dp = opendir(dirname.c_str())) == NULL) {
      return errno;
   }

   while ((dirp = readdir(dp)) != NULL) {
      files.push_back(string(dirp->d_name));
   }
   closedir(dp);
   return 0;
}








