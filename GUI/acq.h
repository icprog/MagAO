#ifndef ACQ_H_INCLUDED
#define ACQ_H_INCLUDED

#include "arblib/base/ArbitratorInterface.h"

#include "AOApp.h"
using namespace Arcetri;

class acq: public AOApp {

public:
   acq( int argc, char *argv[]);

   virtual ~acq();

public:
      int _lamp;

protected:
      void PostInit();
      void Run();

      void init();

      string _jobfile;

      string _savedGain;

    ArbitratorInterface* _wfsIntf;
    ArbitratorInterface* _adsecIntf;
    ArbitratorInterface* _aoarbIntf;

    AlertNotifier *_wfsNotifier;
    AlertNotifier *_adsecNotifier;
    AlertNotifier *_aoNotifier;

    RTDBvar *_lampVarReq;
    RTDBvar *_lampVarCur;
    RTDBvar *_gainVarCur;

    static int lampNotify(void *pt, Variable *msgb);

    int setGain( string gainfile);

    int sendCmdAdsec( Command *cmd, string &errstr, Command **reply = NULL);
    int sendCmdWfs( Command *cmd, string &errstr, Command **reply = NULL);
    int sendCmd( Command *cmd, ArbitratorInterface *intf, string &errstr, Command **reply = NULL);


};

#endif // ACQUIRE_H_INCLUDED




