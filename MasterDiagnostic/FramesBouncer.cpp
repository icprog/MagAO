
#include <errno.h>
#include "arpa/inet.h"
#include "netdb.h"

#include "FramesBouncer.h"
#include "MasterDiagnosticExceptions.h"
#include "stdconfig.h"
#include "BcuLib/BcuCommon.h"


using namespace Arcetri;
using namespace Arcetri::Bcu;


FramesBouncer::FramesBouncer( string name, string MyName, string bufname) {

	_logger = Logger::get(name);

	_name = name;

	_shmBufInfo = new BufInfo();
	_MyName = MyName;
	_bufname = bufname;

   _timeToDie = false;
   _decimation = 0;
   _socket=-1;

	_logger->log(Logger::LOG_LEV_INFO, "Constructing FramesBouncer %s... [%s:%d]", _name.c_str(), __FILE__, __LINE__);
	_logger->log(Logger::LOG_LEV_INFO, "> ShmBufName:           %s", bufname.c_str());

}

FramesBouncer::~FramesBouncer() {
	bufRelease( _MyName.c_str(), _shmBufInfo);
	delete _shmBufInfo;
   close(_socket);
	_logger->log(Logger::LOG_LEV_INFO, "FramesBouncer %s succesfully destroyed !", _name.c_str());
}


pthread_t FramesBouncer::start() {
	pthread_t framesBouncer;
	pthread_create(&framesBouncer, NULL, &execute, (void*)this);
    return framesBouncer;
}

void FramesBouncer::die() {
   _timeToDie = true;
}

int FramesBouncer::setDecimation( int decimation) {

   _logger->log(Logger::LOG_LEV_INFO, "FramesBouncer setting decimation to %d ", decimation);
   
   if ((decimation>=0) && (decimation<10)) {
      _decimation = decimation;
      return NO_ERROR;
   }
   else
      return VALUE_OUT_OF_RANGE_ERROR;
}

void* FramesBouncer::execute(void* thisPtr) throw(FramesBouncerException) {

	FramesBouncer* fb = (FramesBouncer*)thisPtr;
	try {
		fb->run();
	}
	catch(FramesBouncerException& e) {
		Logger::get()->log(Logger::LOG_LEV_FATAL, "%s", e.what().c_str());
		throw;	// Rethrow it to stop the application !!!
	}
	return NULL;
}

void FramesBouncer::run() throw(FramesBouncerException) {

	struct timeval start, end;
  	static int loops = 100;
  	double timeSec;
   int timeout=1000;
   int loopCounter = 0;

   _logger->log(Logger::LOG_LEV_INFO, "FramesBouncer %s STARTING !", _name.c_str());
	// --- Infinite loop up to die()
	while(!_timeToDie) {

		// Start time
  		if(!(loopCounter%loops)) {
  			gettimeofday(&start,NULL);
  		}

		_logger->log(Logger::LOG_LEV_INFO, "");
		_logger->log(Logger::LOG_LEV_INFO, "--------------------------------------------------------------------");
		_logger->log(Logger::LOG_LEV_INFO, "FramesBouncer %s waiting for a frame ready...", _name.c_str());

		// --- Wait for a frame from shared memory --- //
      getFastDiagnosticRawData( _MyName, _bufname, &mirrorDataBuf, timeout, true, &_shmBufInfo);


		_logger->log(Logger::LOG_LEV_INFO, "FramesFunnel %s: got frame from shm!!", _name.c_str());

      // Apply decimation
      if (_decimation>0) {
         static uint32 lastCounter = -1;

         // Only 1 frame per optical loop iteration
         uint32 counter = mirrorDataBuf.switc[0].header.WFSFrameCounter;
         if (counter == lastCounter) { 
            _logger->log(Logger::LOG_LEV_INFO, "FramesBouncer %s: Rejecting frame due to oversampling", _name.c_str());
            continue;
         }

         if (counter % (_decimation+1)) {
		      _logger->log(Logger::LOG_LEV_INFO, "FramesBouncer %s: Rejecting frame due to decimation", _name.c_str());
            continue;
         }
         lastCounter = counter;
      }

      sbobinaFrame( &mirrorDataBuf, &outputDataBuf);

      sendFrame(&outputDataBuf);


		if(!((++loopCounter)%loops)) {
			gettimeofday(&end,NULL);
  			timeSec = (end.tv_usec-start.tv_usec)/1E6 + (end.tv_sec - start.tv_sec);
 			_logger->log(Logger::LOG_LEV_INFO, "Frequency: %.2f Hz (%d)", loops/timeSec, loopCounter);
		}
   }
 	
}

void FramesBouncer::sbobinaFrame( AdsecDiagnRaw *mirrorDataBuf, MirrorOutputDiagn *outputDataBuf) throw(FramesBouncerException) {

   // Copy the headers
   memcpy( &(outputDataBuf->header), &(mirrorDataBuf->switc[0].header), sizeof(switch_bcu_header));
   memcpy( &(outputDataBuf->footer), &(mirrorDataBuf->switc[0].header), sizeof(switch_bcu_header));

   /*

   printf("Sending header:");
   for (i=0; i<4; i++)
      printf(" %08X", ((uint32*)&outputDataBuf->header)[i]);
   printf("\nSending footer:");
   for (i=0; i<4; i++)
      printf(" %08X", ((uint32*)&outputDataBuf->footer)[i]);
   printf("\n\n");
   */


   // Copy modal commands
   int crate, dsp, pos=0;
   
   for ( crate=0; crate < BcuMirror::N_CRATES; crate++)
      for ( dsp=0; dsp < BcuMirror::N_DSP_CRATE; dsp++)
         {
         outputDataBuf->Modes[pos+0] = mirrorDataBuf->crate[crate].dsp[dsp].Modes[0];
         outputDataBuf->Modes[pos+1] = mirrorDataBuf->crate[crate].dsp[dsp].Modes[1];
         outputDataBuf->Modes[pos+2] = mirrorDataBuf->crate[crate].dsp[dsp].Modes[2];
         outputDataBuf->Modes[pos+3] = mirrorDataBuf->crate[crate].dsp[dsp].Modes[3];

         outputDataBuf->FFCommand[pos+0] = mirrorDataBuf->crate[crate].dsp[dsp].FFCommand[0];
         outputDataBuf->FFCommand[pos+1] = mirrorDataBuf->crate[crate].dsp[dsp].FFCommand[1];
         outputDataBuf->FFCommand[pos+2] = mirrorDataBuf->crate[crate].dsp[dsp].FFCommand[2];
         outputDataBuf->FFCommand[pos+3] = mirrorDataBuf->crate[crate].dsp[dsp].FFCommand[3];

         outputDataBuf->DistAverage[pos+0] = mirrorDataBuf->crate[crate].dsp[dsp].DistAverage[0];
         outputDataBuf->DistAverage[pos+1] = mirrorDataBuf->crate[crate].dsp[dsp].DistAverage[1];
         outputDataBuf->DistAverage[pos+2] = mirrorDataBuf->crate[crate].dsp[dsp].DistAverage[2];
         outputDataBuf->DistAverage[pos+3] = mirrorDataBuf->crate[crate].dsp[dsp].DistAverage[3];
         pos += 4;
         }

}

void FramesBouncer::sendFrame( MirrorOutputDiagn *outputDataBuf) {

   static int frameCounter=0;

   if (_socket<=0) {
      _logger->log(Logger::LOG_LEV_DEBUG, "sendFrame(): frame not sent (destination not set)");
      return;
   }

   int totLen = sizeof(MirrorOutputDiagn);
   int remainingLen = totLen;
   int totalsent=0;
   BYTE packet[ Constants::TDP_PACKET_HEADER_SIZE + Constants::MAX_TDP_PACKET_SIZE];
   memset( packet, 0, Constants::TDP_PACKET_HEADER_SIZE + Constants::MAX_TDP_PACKET_SIZE);
   DiagnosticUdpHeader *header = (DiagnosticUdpHeader*) &((TDPHeader*)packet)->header;

   int packetId =0;
   while (remainingLen >0) {
      int len = remainingLen;
      if (len > Constants::MAX_TDP_PACKET_SIZE)
         len = Constants::MAX_TDP_PACKET_SIZE;

      header->tot_len = totLen;
      header->saddr = 0;
      header->packetId = packetId;
      header->frameId  = frameCounter;

      memcpy( packet + Constants::TDP_PACKET_HEADER_SIZE, ((BYTE *)outputDataBuf) + totalsent, len);

   /*  
   printf("Header: %d bytes\n", Constants::TDP_PACKET_HEADER_SIZE); 

    for (int i=0; i<3; i++)
         printf("0x%08X\n", ((uint32 *)header)[i]);

   printf("outputDataBuf: \n");
    for (int i=0; i<8; i++)
         printf("0x%08X\n", ((uint32 *)outputDataBuf)[i]);

   printf("Packet dump, %d bytes:\n\n", len + Constants::TDP_PACKET_HEADER_SIZE);
   for (int i=0; i< (len + Constants::TDP_PACKET_HEADER_SIZE)/4; i++) {
      if (i%4 == 0)
             printf("\n%08X: ", i);
          printf("%08X ", ((uint32*) packet)[i]);
           }
       printf("\n");
*/


      int sent = sendto( _socket, packet, len+Constants::TDP_PACKET_HEADER_SIZE, 0, (struct sockaddr *)&_sa, sizeof(_sa));
      if (sent != len+Constants::TDP_PACKET_HEADER_SIZE) {
         _logger->log(Logger::LOG_LEV_WARNING, "UDP error: sendto() reported %d bytes sent instead of %d", sent, len);
      }

      packetId++;
      totalsent += len;
      remainingLen -= len;
   }

   frameCounter++;

    _logger->log(Logger::LOG_LEV_INFO, "FramesFunnel %s: frame sent to listener", _name.c_str());

}




int FramesBouncer::setDestination( string host, int port) throw (FramesBouncerException) {

   if (_socket >0)
      close(_socket);

   if (host == "null") {
	   _logger->log(Logger::LOG_LEV_INFO, "setDestionation(): destination address is null, disabling bouncer");
      _socket=-1;
      return NO_ERROR;
   }


	_logger->log(Logger::LOG_LEV_INFO, "setDestination(): setting destination address:  %s:%d", host.c_str(), port);

   if ((_socket = socket(AF_INET, SOCK_DGRAM, 0)) == -1) {
       _logger->log(Logger::LOG_LEV_ERROR, "Error opening UDP socket: errno (%d) %s",
                                           errno, strerror(errno));
       return UDP_CREATION_ERROR;
   }

   memset(&_sa, 0, sizeof(_sa));
   _sa.sin_family = AF_INET;
   _sa.sin_port = htons(port);

   if (inet_aton(host.c_str(), &_sa.sin_addr) ==0) {
      /* Not an IP address, resolve the name */
      struct hostent *he;
      he = gethostbyname(host.c_str());
      if (!he) {
       _logger->log(Logger::LOG_LEV_ERROR, "Host %s not found: errno (%d) %s", host.c_str(),
                                           errno, strerror(errno));
       return UDP_HOST_NOT_FOUND;
      }

      _sa.sin_addr = *(struct in_addr*)he->h_addr;
   }
   return NO_ERROR;
}


