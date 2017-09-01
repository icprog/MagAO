#define VERS_MAJOR   1
#define VERS_MINOR   0

#include <string>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>


#include "thorlabsProxy.h"


extern "C" {
#include "base/timelib.h"
}


int debug=0;

thorlabsProxy::thorlabsProxy( std::string name, std::string side, string serverIP, int id) :
    AOApp( name, side, serverIP, id)
{
   _logger->setLevel( Logger::LOG_LEV_TRACE);
}
   
void thorlabsProxy::PostInit()
{
   int stat;

   _address = "193.206.155.78";
   _port    = 14500;

   _buflen = 1024*768*2;
   _maxCS = 10;
   _timeout = 1000;

   _myBuffer = new unsigned char[_buflen];

   Logger::get()->log( Logger::LOG_LEV_INFO, "Creating shared buffer %s:THORLABS", MyFullName().c_str());
   if ((stat = bufCreate((char*)MyFullName().c_str(), "THORLABS", BUFCONT, _buflen, _maxCS, &_bufInfo)) == NO_ERROR)
      Logger::get()->log( Logger::LOG_LEV_INFO, "Shared buffer %s created", _bufInfo.com.name);
   else
      Logger::get()->log( Logger::LOG_LEV_ERROR, "Error creating shared buffer: (%d) %s", stat, lao_strerror(stat));

   _counter=0;
}

int thorlabsProxy::thorlabsConnect()
{
   struct sockaddr_in servaddr;

   _sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

   servaddr.sin_family=AF_INET;
   servaddr.sin_port=htons(_port);
   servaddr.sin_addr.s_addr = inet_addr(_address.c_str());
                       
   if ( connect( _sockfd, (struct sockaddr *) &servaddr, sizeof(servaddr)) == 0)
      Logger::get()->log( Logger::LOG_LEV_INFO, "Connected to thorlabs\n");
   else
      Logger::get()->log( Logger::LOG_LEV_ERROR, "Error connected to thorlabs\n");

   return 0;
}

int thorlabsProxy::thorlabsRead( unsigned char *buf, int buflen)
{
   int totread=0;
   char cmd[] = "frame";

   write( _sockfd, cmd, strlen(cmd));

   while (totread <  buflen)
      {
      int n = read( _sockfd, buf+totread, buflen);
      if (n>0)
         totread += n;
      else
         return -1;
      }

   return 0;
}

void thorlabsProxy::thorlabsProcess( unsigned char *buf, int buflen)
{
   int npixels = buflen/2;
   unsigned short int *pointer = (unsigned short int *)buf;

   for (int i=0; i<npixels; i++)
      {
      // Byte swap and right shift of 6 bits
      unsigned short int value = pointer[i];
      pointer[i] = (((value<<8) & 0xFF00) | ((value>>8) & 0xFF)) >> 6;
      }
}


void thorlabsProxy::Run()
{
   while (!TimeToDie())
      {
      if (thorlabsRead(_myBuffer, _buflen) == 0)
         {
         thorlabsProcess( _myBuffer, _buflen);
         bufWrite( &_bufInfo, _myBuffer, _counter++, _timeout);
         Logger::get()->log( Logger::LOG_LEV_INFO, "Frame %d written to shared memory", _counter);
         }
      else
         {
         Logger::get()->log( Logger::LOG_LEV_ERROR, "Error reading from camera, reconnecting...");
         msleep(2000);
         thorlabsConnect();
         }
      }
}

int main()
{

   SetVersion(VERS_MAJOR,VERS_MINOR);

   try {
      thorlabsProxy proxy("ThorlabsProxy", "L", "127.0.0.1", 0);
      proxy.Exec();
  } catch (AOException &e) {
      Logger::get()->log( Logger::LOG_LEV_FATAL, "%s:%d: %s", __FILE__, __LINE__, e.what().c_str());
  }

}
