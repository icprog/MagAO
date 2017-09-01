

#include <string>

extern "C" {
#include "base/thrdlib.h"
#include "base/buflib.h"
}

#include "AOApp.h"


class thorlabsProxy: public AOApp
{
public:
   thorlabsProxy( string name, string side, string serverIP="127.0.0.1", int id=0);

   void PostInit();

   void Run();

   int thorlabsConnect();
   int thorlabsRead( unsigned char *, int );
   void thorlabsProcess( unsigned char *, int);

protected:
   int _sockfd;
   int _buflen;
   int _maxCS;
   int _counter;
   int _timeout;

   std::string _address;
   int _port;

   unsigned char *_myBuffer;
   BufInfo _bufInfo;
};
