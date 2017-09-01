#ifndef ADAMLIB_H_INCLUDE
#define ADAMLIB_H_INCLUDE

#include "AOExcept.h"
#include "UdpConnection.h"
#include "TcpConnection.h"
#include "Logger.h"
#include "stdconfig.h"

#include <boost/thread/thread.hpp>
#include <boost/bind.hpp>



#define BCU_RESET_LOW_CMD "011000"
#define BCU_RESET_HI_CMD "011001"
#define SELECT_USER_CMD "011101"
#define SELECT_DEFAULT_CMD "011100"
#define CLEAR_FPGA_LOW_CMD "011200"
#define CLEAR_FPGA_HI_CMD "011201"
#define DISABLE_COILS_CMD "011300"
#define ENABLE_COILS_CMD  "011301"
#define DISABLE_TSS_CMD "011401"
#define ENABLE_TSS_CMD "011400"
#define DISABLE_MAIN_PWR_CMD "011700"
#define ENABLE_MAIN_PWR_CMD "011701"


using namespace Arcetri;

namespace Arcetri {

/*
 * A generic Adam exception
 */
class AdamException: public AOException {

	public:

		AdamException(string msg, int errCode, string file = "", int line = 0):AOException(msg, errCode, file, line) { exception_id = "AdamException"; }

		virtual ~AdamException() {}
};


/*@Class: Adam
 * Hi-level communication with Adam 6K
 *
 * This class provide hi-level routines to send commands
 * to the Adam 6K device using ASCII over UDP.
 * @
 */
class Adam {

	public:

		/*
		 * Default constructor.
		 * Reads the adam parameters from the default config file.
		 *
		 * Initialize the "ADAM" logger using LogLevel got from config (adam.conf).
		 *
		 */
		Adam( string ipAddr, string adamName="", bool testConn = false) throw (AdamException);

		~Adam();

		/*
		 * Default commands
		 */
		bool resetBcu();
		bool selectUser();
		bool selectDefault();
		bool clearFpga();
		bool disableCoils();
		bool enableCoils();
		bool disableTss();
		bool enableTss();
		bool disableMainPower();
		bool enableMainPower();
		bool Led0On();
		bool Led0Off();
		bool Led1On();
		bool Led1Off();
		bool Led2On();
		bool Led2Off();
		bool Led3On();
		bool Led3Off();

		/*
		 * Return the wrapped UdpConnection.
		 * Can be used to send custom commands.
		 */
		 UdpConnection* getUdpConnection();


	protected:

		Config_File _config;

		/*
		 * Send a generic command in the following format:
		 * #aabb[data]\r:
		 * - aa: represents the 2-character hexadecimal Modbus network address of the ADAM-6000 module (always 01)
		 * - bb: '1n' write the output channel 'n'
		 * - data: 01 write the bit "1"
		 * Returns true only if command succesfully accepted by Adam.
		 */
		bool sendCommand(string cmd);


	private:

		// Available commands (read from adam.conf)
		string _BCU_RESET_LOW_CMD;
		string _BCU_RESET_HI_CMD;
		string _SELECT_USER_CMD;
		string _SELECT_DEFAULT_CMD;
		string _CLEAR_FPGA_LOW_CMD;
		string _CLEAR_FPGA_HI_CMD;
		string _DISABLE_COILS_CMD;
		string _ENABLE_COILS_CMD;
		string _ENABLE_TSS_CMD;
		string _DISABLE_TSS_CMD;
		string _ENABLE_MAIN_PWR_CMD;
		string _DISABLE_MAIN_PWR_CMD;
		string _LED_0_ON_CMD;
		string _LED_0_OFF_CMD;
		string _LED_1_ON_CMD;
		string _LED_1_OFF_CMD;
		string _LED_2_ON_CMD;
		string _LED_2_OFF_CMD;
		string _LED_3_ON_CMD;
		string _LED_3_OFF_CMD;

		UdpConnection* _adamConn;

		Logger* _logger;
};


/*
 * @Class: AdamModbus
 * Hi-level communication with Adam 6K
 *
 * This class provide hi-level routines to send commands
 * to the Adam 6K device using MODBUS over TCP.
 *
 * The code is an udated subset of the library provided by Advantech
 * in the FAQ section of ADAM-6052 device
 * (http://support.advantech.com.tw/support/KnowledgeBaseSRDetail.aspx?SR_ID=1-RAQBT)
 *
 *  =======================
 *       Modbus command
 *  =======================
 *  int iStartIndex = 1; // the first one is "1", not "0"
 *  int iTotalPoint = 2;
 *  unsigned char szData[32]={0};
 *  int iTotalByte;
 *
 *  if (SendReadInputRegs(1, iStartIndex, iTotalPoint) && RecvReadInputRegs(szData, &iTotalByte) {
 * 		.......
 *  }
 */
class AdamModbus {

	public:

		/*
		 * Constructor
		 * Reads the adam parameters from the default config file (adam.conf)
		 *
		 * Initialize the "name" logger using LogLevel got from config.
		 *
		 */
		AdamModbus(string name="ADAM-MODBUS", bool initTCP=true) throw(AdamException);

		/*
		 * Destructor
		 */
		virtual ~AdamModbus();

		/*
		 * Initialize/clear the TCP connection
		 */
		void initTcp() throw(TcpCreationException);
		void clearTcp();

      void setLogLevel(int level);

		// function : 05 (HEX 05)
		bool SendForceSingleCoil(int i_iAddr,
								 int i_iCoilIndex,
								 int i_iData); // 0: OFF; otherwise: ON
		bool RecvForceSingleCoil();

		// function : 06 (HEX 06)
		bool SendPresetSingleReg(int i_iAddr,
								 int i_iRegIndex,
								 int i_iData);
		bool RecvPresetSingleReg();

		// function : 15 (HEX 0F)
		bool SendForceMultiCoils(int i_iAddr,
								 int i_iCoilIndex,
								 int i_iTotalPoint,
								 int i_iTotalByte,
								 unsigned char i_szData[]);
		bool RecvForceMultiCoils();

		// function : 16 (HEX 10)
		// i_iStartReg : start from 1
		bool SendPresetMultiRegs(int i_iAddr,
								 int i_iStartReg,
								 int i_iTotalReg,
								 int i_iTotalByte,
								 unsigned char i_szData[]);
		bool RecvPresetMultiRegs();

      // function : 02 (HEX 02)
      // Returns a bit mask into <data>, which must be at least
      // 1 byte for each 8 registers read.
      bool ReadInputStatus(int i_iAddr,
                          int i_iStartReg,
                          int i_iEndReg,
                          void *data);

      // Low-level send function
      bool SendReadInputStatus(int i_iAddr,
                              int i_iStartReg,
                              int i_iEndReg);



      // function : 01 (HEX 01)
      // Returns a bit mask into <data>, which must be at least
      // 1 byte for each 8 registers read.
      bool ReadCoilStatus(int i_iAddr,
                          int i_iStartReg,
                          int i_iEndReg,
                          void *data);

      // Low-level send function
      bool SendReadCoilStatus(int i_iAddr,
                              int i_iStartReg,
                              int i_iEndReg);



	protected:

		Config_File _config;

		Logger* _logger;


	private:

		TcpConnection* _tcpConn;

		int _timeout_ms;

		static const int ADAM_MAX_MSGLEN = 256;	// [to check]

		char _szHexSend[ADAM_MAX_MSGLEN];
		char _szHexRecv[ADAM_MAX_MSGLEN];

	private:

		void HexToRTU(char *i_szHexCmd, unsigned char *o_szRTUCmd);
		void RTUToHex(unsigned char *i_szRTUCmd, int i_RTULen, char *o_szHexCmd);

		bool SendTCP(unsigned char i_szData[], int i_iLen);
		int RecvTCP(unsigned char o_szData[]);
};

/*
 * @Class: AdamWatchdog
 * A threaded watchdog for the Adam.
 */
class AdamWatchdog: public AdamModbus {

	public:

		/*
		 * Create a watchdog
		 * Reads the adam parameters from the default config file (adam.conf)
		 *
		 * Initialize the "ADAM-WATCHDOG" logger using LogLevel got from config.
		 */
		AdamWatchdog(bool autoStart = false) throw(AdamException);

		/*
		 * Destroy a watchdog
		 */
		~AdamWatchdog();

		/*
		 * Start the watchdog
		 */
		void start();

		/*
		 * Stop the watchdog
		 */
		void stop();

	private:

		/*
		 * Send an enable watchdog command
		 */
		 bool enableWatchdog();

		/*
		 * Send a watchdog command
		 */
		 bool sendWatchdog();

		 /*
		  * Method executed in a thread by start()
		  */
		 void execute();


	private:

		int _WD_DELAY_MS10;
		int _WD_DELAY_REG_ADDR;
		int _WD_CHANNEL_ADDR;

		int  _period_us;

		bool _wdEnabled;
		bool _wdStarted;

		boost::thread* _thread;

};


} // END namespace Arcetri

#endif /*ADAMLIB_H_INCLUDE*/
