#include "AdamTestLib.h"

#include <stdio.h>


extern "C" {
#include "base/timelib.h"
}


using namespace Arcetri;




// *********************************** ADAM ************************************ //

Adam::Adam( string ipAddr, string adamName, bool testConn) throw (AdamException) {
	try {

		_logger = Logger::get("ADAM", Logger::stringToLevel("INF"));
      int fd = dup(1);
      //_logger->setLogFp(stdout);
		_logger->log(Logger::LOG_LEV_INFO, "Creating Adam...");

		int localUdpPortStart = 8070;
		int localUdpPortEnd = 8090;
		string adamHost = ipAddr;
		int adamUdpPort = 1024;
		int timeout_ms = 100;

		try {
		_BCU_RESET_LOW_CMD = (string)"#" + BCU_RESET_LOW_CMD + (string)"\r";
		_BCU_RESET_HI_CMD = (string)"#" + BCU_RESET_HI_CMD + (string)"\r";
		_SELECT_USER_CMD = (string)"#" + SELECT_USER_CMD + (string)"\r";
		_SELECT_DEFAULT_CMD = (string)"#" + SELECT_DEFAULT_CMD + (string)"\r";
		_CLEAR_FPGA_LOW_CMD = (string)"#" + CLEAR_FPGA_LOW_CMD + (string)"\r";
		_CLEAR_FPGA_HI_CMD = (string)"#" + CLEAR_FPGA_HI_CMD + (string)"\r";
		_DISABLE_COILS_CMD = (string)"#" + DISABLE_COILS_CMD + (string)"\r";
		_ENABLE_COILS_CMD = (string)"#" + ENABLE_COILS_CMD + (string)"\r";
		_ENABLE_TSS_CMD = (string)"#" + ENABLE_TSS_CMD + (string)"\r";
		_DISABLE_TSS_CMD = (string)"#" + DISABLE_TSS_CMD + (string)"\r";
		_ENABLE_MAIN_PWR_CMD = (string)"#" + ENABLE_MAIN_PWR_CMD + (string)"\r";
		_DISABLE_MAIN_PWR_CMD = (string)"#" + DISABLE_MAIN_PWR_CMD + (string)"\r";
		} catch (...){}

	
		int localUdpPort = localUdpPortStart;
		bool connected = false;
		// Loop on all available UDP ports
		while(!connected && localUdpPort <= localUdpPortEnd) {
			try {
				_adamConn = new UdpConnection(localUdpPort, adamHost, adamUdpPort, timeout_ms, Logger::stringToLevel("INF"));
				connected = true;
	            		_logger->log(Logger::LOG_LEV_INFO, "Adam:  udp connection to %s:%d - assigned to local port %d - timout %d [%s:%d]",
                       		adamHost.c_str(), adamUdpPort, localUdpPort, timeout_ms, __FILE__, __LINE__ );
			}
			catch(UdpFatalException& e) {
				_logger->log(Logger::LOG_LEV_WARNING, "Adam: UdpFatalException error %s", e.what().c_str() );
				_logger->log(Logger::LOG_LEV_WARNING, "Adam: Udp port %d already in use: will try the next available port", localUdpPort);
				localUdpPort++;
			}
			catch(AOException& e) {
				_logger->log(Logger::LOG_LEV_WARNING, "Adam: GENERIC error %s", e.what().c_str() );
				_logger->log(Logger::LOG_LEV_WARNING, "Adam: Udp port %d already in use: will try the next available port", localUdpPort);
				localUdpPort++;
			}
		}

		// Check if connected
		if(connected) {
         if (testConn) {
			// send command to check that connection is ok
			if ( sendCommand("$016\r") == false ) {
			    _logger->log(Logger::LOG_LEV_ERROR, "Adam: impossible to communicate with the device");
			    delete _adamConn;
			    throw AdamException("Impossible to communicate with the device", ADAM_INIT_ERROR, __FILE__, __LINE__);
			};
			_logger->log(Logger::LOG_LEV_INFO, "Adam: communication succesfully initialized (local port: %d)", localUdpPort);
         }
		}
		else {
			_logger->log(Logger::LOG_LEV_ERROR, "Adam: communication initialization failed (not local udp ports available)");
			throw UdpFatalException("Impossible to create the Adam connection (not local udp ports available)");
		}
	}
	catch(Config_File_Exception& e) {
		Logger::get("ADAM", Logger::LOG_LEV_ERROR)->log(Logger::LOG_LEV_ERROR, "%s", e.what().c_str());
      printf("%s\n", e.what().c_str());
		throw AdamException("Error in Adam creation: wrong config file", ADAM_INIT_ERROR, __FILE__, __LINE__);
	}
}


Adam::~Adam() {
	delete _adamConn;
}


UdpConnection* Adam::getUdpConnection() {
	return _adamConn;
}

bool Adam::resetBcu() {
	_logger->log(Logger::LOG_LEV_INFO, "Adam: resetting Bcu...");
	bool res;
	if((res = sendCommand(_BCU_RESET_HI_CMD)) && (res = sendCommand(_BCU_RESET_LOW_CMD))) {
		_logger->log(Logger::LOG_LEV_INFO, "Adam: Bcu succesfully reset");
	}
	else {
		_logger->log(Logger::LOG_LEV_ERROR, "Adam: Bcu reset failed");
	}
	return res;
}

bool Adam::selectUser() {
	_logger->log(Logger::LOG_LEV_INFO, "Adam: selecting user...");
	bool res;
	if((res = sendCommand(_SELECT_USER_CMD))) {
		_logger->log(Logger::LOG_LEV_INFO, "Adam: user succesfully selected");
	}
	else {
		_logger->log(Logger::LOG_LEV_ERROR, "Adam: user selection failed");
	}
	return res;
}

bool Adam::selectDefault() {
	_logger->log(Logger::LOG_LEV_INFO, "Adam: selecting default...");
	bool res;
	if((res = sendCommand(_SELECT_DEFAULT_CMD))) {
		_logger->log(Logger::LOG_LEV_INFO, "Adam: default succesfully selected");
	}
	else {
		_logger->log(Logger::LOG_LEV_ERROR, "Adam: default selection failed");
	}
	return res;
}

bool Adam::clearFpga() {
	_logger->log(Logger::LOG_LEV_INFO, "Adam: clearing fpga...");
	bool res;
	if((res = sendCommand(_CLEAR_FPGA_LOW_CMD)) && (res = sendCommand(_CLEAR_FPGA_HI_CMD))) {
		_logger->log(Logger::LOG_LEV_INFO, "Adam: fpga succesfully cleared");
	}
	else {
		_logger->log(Logger::LOG_LEV_ERROR, "Adam: fpga clear failed");
	}
	return res;
}

bool Adam::disableCoils() {
	_logger->log(Logger::LOG_LEV_INFO, "Adam: disabling coils...");
	bool res;
	if((res = sendCommand(_DISABLE_COILS_CMD))) {
		_logger->log(Logger::LOG_LEV_INFO, "Adam: coils succesfully disabled");
	}
	else {
		_logger->log(Logger::LOG_LEV_ERROR, "Adam: coils disabling failed");
	}
	return res;
}

bool Adam::enableCoils() {
	_logger->log(Logger::LOG_LEV_INFO, "Adam: enabling coils...");
	bool res;
	if((res = sendCommand(_ENABLE_COILS_CMD))) {
		_logger->log(Logger::LOG_LEV_INFO, "Adam: coils succesfully enabled");
	}
	else {
		_logger->log(Logger::LOG_LEV_ERROR, "Adam: coils enabling failed");
	}
	return res;
}

bool Adam::enableTss() {
	_logger->log(Logger::LOG_LEV_INFO, "Adam: enabling Tss...");
	bool res;
	if((res = sendCommand(_ENABLE_TSS_CMD))) {
		_logger->log(Logger::LOG_LEV_INFO, "Adam: Tss succesfully enabled");
	}
	else {
		_logger->log(Logger::LOG_LEV_ERROR, "Adam: Tss enabling failed");
	}
	return res;
}

bool Adam::disableTss() {
	_logger->log(Logger::LOG_LEV_INFO, "Adam: disabling Tss...");
	bool res;
	if((res = sendCommand(_DISABLE_TSS_CMD))) {
		_logger->log(Logger::LOG_LEV_INFO, "Adam: Tss succesfully disabled");
	}
	else {
		_logger->log(Logger::LOG_LEV_ERROR, "Adam: Tss disabling failed");
	}
	return res;
}

bool Adam::enableMainPower() {
	_logger->log(Logger::LOG_LEV_INFO, "Adam: enabling main power...");
	bool res;
	if((res = sendCommand(_ENABLE_MAIN_PWR_CMD))) {
		_logger->log(Logger::LOG_LEV_INFO, "Adam: main power succesfully enabled");
	}
	else {
		_logger->log(Logger::LOG_LEV_ERROR, "Adam: main power enabling failed");
	}
	return res;
}

bool Adam::disableMainPower() {
	_logger->log(Logger::LOG_LEV_INFO, "Adam: disabling main power...");
	bool res;
	if((res = sendCommand(_DISABLE_MAIN_PWR_CMD))) {
		_logger->log(Logger::LOG_LEV_INFO, "Adam: main power succesfully disabled");
	}
	else {
		_logger->log(Logger::LOG_LEV_ERROR, "Adam: main power disabling failed");
	}
	return res;
}

bool Adam::Led0On() {
	_logger->log(Logger::LOG_LEV_INFO, "Adam: Led 0 on...");
	bool res;
	if( (res = sendCommand(_LED_0_ON_CMD)) ) {
		_logger->log(Logger::LOG_LEV_INFO, "Adam: Led 0 on ok");
	}
	else {
		_logger->log(Logger::LOG_LEV_ERROR, "Adam: Led 0 on failed");
	}
	return res;
}
bool Adam::Led0Off() {
	_logger->log(Logger::LOG_LEV_INFO, "Adam: Led 0 off...");
	bool res;
	if( (res = sendCommand(_LED_0_OFF_CMD)) ) {
		_logger->log(Logger::LOG_LEV_INFO, "Adam: Led 0 off ok");
	}
	else {
		_logger->log(Logger::LOG_LEV_ERROR, "Adam: Led 0 off failed");
	}
	return res;
}

bool Adam::Led1On() {
	_logger->log(Logger::LOG_LEV_INFO, "Adam: Led 1 on...");
	bool res;
	if( (res = sendCommand(_LED_1_ON_CMD)) ) {
		_logger->log(Logger::LOG_LEV_INFO, "Adam: Led 1 on ok");
	}
	else {
		_logger->log(Logger::LOG_LEV_ERROR, "Adam: Led 1 on failed");
	}
	return res;
}
bool Adam::Led1Off() {
	_logger->log(Logger::LOG_LEV_INFO, "Adam: Led 1 off...");
	bool res;
	if( (res = sendCommand(_LED_1_OFF_CMD)) ) {
		_logger->log(Logger::LOG_LEV_INFO, "Adam: Led 1 off ok");
	}
	else {
		_logger->log(Logger::LOG_LEV_ERROR, "Adam: Led 1 off failed");
	}
	return res;
}
bool Adam::Led2On() {
	_logger->log(Logger::LOG_LEV_INFO, "Adam: Led 1 on...");
	bool res;
	if( (res = sendCommand(_LED_2_ON_CMD)) ) {
		_logger->log(Logger::LOG_LEV_INFO, "Adam: Led 1 on ok");
	}
	else {
		_logger->log(Logger::LOG_LEV_ERROR, "Adam: Led 1 on failed");
	}
	return res;
}
bool Adam::Led2Off() {
	_logger->log(Logger::LOG_LEV_INFO, "Adam: Led 2 off...");
	bool res;
	if( (res = sendCommand(_LED_2_OFF_CMD)) ) {
		_logger->log(Logger::LOG_LEV_INFO, "Adam: Led 2 off ok");
	}
	else {
		_logger->log(Logger::LOG_LEV_ERROR, "Adam: Led 2 off failed");
	}
	return res;
}
bool Adam::Led3On() {
	_logger->log(Logger::LOG_LEV_INFO, "Adam: Led 3 on...");
	bool res;
	if( (res = sendCommand(_LED_3_ON_CMD)) ) {
		_logger->log(Logger::LOG_LEV_INFO, "Adam: Led 3 on ok");
	}
	else {
		_logger->log(Logger::LOG_LEV_ERROR, "Adam: Led 3 on failed");
	}
	return res;
}
bool Adam::Led3Off() {
	_logger->log(Logger::LOG_LEV_INFO, "Adam: Led 3 off...");
	bool res;
	if( (res = sendCommand(_LED_3_OFF_CMD)) ) {
		_logger->log(Logger::LOG_LEV_INFO, "Adam: Led 3 off ok");
	}
	else {
		_logger->log(Logger::LOG_LEV_ERROR, "Adam: Led 3 off failed");
	}
	return res;
}

bool Adam::sendCommand(string strCommand) {
	try {
		const int cmdLen = 9;
		const int ansLen = 5;

		BYTE cmd[cmdLen];
		memset(cmd, 0, cmdLen);
		memcpy(cmd, strCommand.c_str(), cmdLen);
		BYTE ans[ansLen];
		memset(ans, 0, ansLen);
		_adamConn->send(cmd, cmdLen);
		_logger->log(Logger::LOG_LEV_DEBUG, "Command to Adam: %s", cmd);
		_logger->log(Logger::LOG_LEV_DEBUG, "Answer buffer: %02X %02X %02X %02X", ans[0], ans[1], ans[2], ans[3]);
		_adamConn->receive(ans, ansLen-1);
		_logger->log(Logger::LOG_LEV_DEBUG, "Answer buffer: %02X %02X %02X %02X", ans[0], ans[1], ans[2], ans[3]);
		_logger->log(Logger::LOG_LEV_DEBUG, "Answer from Adam: %s", ans);

        if (ans[0] == '?') 
            return false;
		else 
			return true;
	
	}
	catch(UdpException& e) {
		_logger->log(Logger::LOG_LEV_DEBUG, "%s", e.what().c_str());
		return false;
	}
}

// ******************************** ADAMMODBUS ********************************* //

AdamModbus::AdamModbus( string name, bool initTCP) throw(AdamException) {

   try {
		_config = Config_File("conf/adsec/current/processConf/adam.conf");

		_logger = Logger::get(name, Logger::stringToLevel(_config["LogLevel"]));
		_logger->printStatus();

		_logger->log(Logger::LOG_LEV_INFO, "Creating %s...", name.c_str());
		_timeout_ms = _config["TIMEOUT_TCP_MS"];
		// Creates TCP connection only on request
		_tcpConn = NULL;
		if(initTCP) {
			initTcp();
		}

		_logger->log(Logger::LOG_LEV_INFO, "%s succesfully created!", name.c_str());
	}
	catch(Config_File_Exception& e) {
		Logger::get(name, Logger::LOG_LEV_ERROR)->log(Logger::LOG_LEV_ERROR, "%s", e.what().c_str());
		throw AdamException("Error in AdamModbus creation: wrong config file", ADAM_INIT_ERROR, __FILE__, __LINE__);
	}
	catch(TcpCreationException& e) {
		Logger::get(name, Logger::LOG_LEV_ERROR)->log(Logger::LOG_LEV_ERROR, "%s", e.what().c_str());
		throw AdamException("Error in AdamModbus creation: unable to create the Tcp connection", ADAM_INIT_ERROR, __FILE__, __LINE__);
	}
}

AdamModbus::~AdamModbus() {
	clearTcp();
}

void AdamModbus::initTcp() throw(TcpCreationException) {
	if(_tcpConn == NULL) {
		_logger->log(Logger::LOG_LEV_DEBUG, "Creating TCP connection...");
		_tcpConn = new TcpConnection(_config["ADAM_HOST"], _config["ADAM_TCP_PORT"], Logger::stringToLevel(_config["LogLevel"]));
		_logger->log(Logger::LOG_LEV_INFO, "Created TCP connection with timeout %d ms", _timeout_ms);
	}
	else {
		_logger->log(Logger::LOG_LEV_INFO, "TCP connection with timeout %d ms already active!", _timeout_ms);
	}
}

void AdamModbus::clearTcp() {
	delete _tcpConn;
	_tcpConn = NULL;
}

// function (05 HEX)
// TESTED 20.03.2008: OK
bool AdamModbus::SendForceSingleCoil(int i_iAddr,
									 int i_iCoilIndex,
									 int i_iData)
{
	_logger->log(Logger::LOG_LEV_DEBUG, ">>> SendForceSingleCoil <<<");

	int iHexLen, iRTULen;
	BYTE szRTUCmd[ADAM_MAX_MSGLEN]={0};
	char szModCmd[ADAM_MAX_MSGLEN] = {0};
	int coilData;

	if (i_iData == 0)
		coilData = 0x0000;
	else
		coilData = 0xFF00;
    memset(_szHexSend, 0, ADAM_MAX_MSGLEN);
	sprintf(szModCmd, "%02X%02X%04X%04X", i_iAddr, 5, i_iCoilIndex-1, coilData);
	sprintf(_szHexSend, "0000000000%02X%s", strlen(szModCmd)/2, szModCmd);
	iHexLen = strlen(_szHexSend);
	// form TCP-RTU command
	HexToRTU(_szHexSend, szRTUCmd);
	iRTULen = iHexLen/2;
	return SendTCP(szRTUCmd, iRTULen);
}

// function (05 HEX)
// TESTED 20.03.2008: OK
bool AdamModbus::RecvForceSingleCoil()
{
	_logger->log(Logger::LOG_LEV_DEBUG, ">>> RecvForceSingleCoil <<<");

	BYTE szRecv[ADAM_MAX_MSGLEN];

	// recvDataLen must be equal 12;
	// 'header(12)', 'addr(2)', 'fun(2)', 'Coil addr Hi-Lo(4)',
	// 'Force data Hi-Lo(4)', 24 TCP-HEX bytes, 12 TCP-RTU bytes
	int recvDataLen = RecvTCP(szRecv);
	if (recvDataLen==12)
    {
    	_logger->log(Logger::LOG_LEV_TRACE, ">> Received data: %d bytes", recvDataLen);
    	RTUToHex(szRecv, recvDataLen, _szHexRecv);

    	_logger->log(Logger::LOG_LEV_TRACE, ">> Sent function code: %2X %2X", _szHexSend[14], _szHexSend[15]);
        _logger->log(Logger::LOG_LEV_TRACE, ">> Recv function code: %2X %2X",  _szHexRecv[14],  _szHexRecv[15]);

       	if (_szHexSend[14] == _szHexRecv[14] &&
			_szHexSend[15] == _szHexRecv[15])
			// function code must be the same, otherwise, exception
			return true;
    }
	return false;
}

// function (06 HEX)
// TESTED 20.03.2008: OK
bool AdamModbus::SendPresetSingleReg(int i_iAddr,
									 int i_iRegIndex,
									 int i_iData)
{
	_logger->log(Logger::LOG_LEV_DEBUG, ">>> SendPresetSingleReg <<<");

	int iHexLen, iRTULen;
	BYTE szRTUCmd[ADAM_MAX_MSGLEN]={0};
	char szModCmd[ADAM_MAX_MSGLEN] = {0};

    memset(_szHexSend, 0, ADAM_MAX_MSGLEN);
	sprintf(szModCmd, "%02X%02X%04X%04X", i_iAddr, 6, i_iRegIndex-1, i_iData);
	sprintf(_szHexSend, "0000000000%02X%s", strlen(szModCmd)/2, szModCmd);
	iHexLen = strlen(_szHexSend);
	// form TCP-RTU command
	HexToRTU(_szHexSend, szRTUCmd);
	iRTULen = iHexLen/2;
	return SendTCP(szRTUCmd, iRTULen);
}

// function (01 HEX)
// NOT TESTED
bool AdamModbus::SendReadCoilStatus(int i_iAddr,
									 int i_iStartReg,
									 int i_iNumReg)
{
	_logger->log(Logger::LOG_LEV_DEBUG, ">>> SendReadCoilStatus <<<");

	int iHexLen, iRTULen;
	BYTE szRTUCmd[ADAM_MAX_MSGLEN]={0};
	char szModCmd[ADAM_MAX_MSGLEN] = {0};

    memset(_szHexSend, 0, ADAM_MAX_MSGLEN);
	sprintf(szModCmd, "%02X%02X%04X%04X", i_iAddr, 1, i_iStartReg, i_iNumReg);
	sprintf(_szHexSend, "0000000000%02X%s", strlen(szModCmd)/2, szModCmd);
	iHexLen = strlen(_szHexSend);
	_logger->log(Logger::LOG_LEV_TRACE, "Sending      : %s", _szHexSend );
	// form TCP-RTU command
	HexToRTU(_szHexSend, szRTUCmd);
	iRTULen = iHexLen/2;
	return SendTCP(szRTUCmd, iRTULen);
}

// function (02 HEX)
// NOT TESTED
bool AdamModbus::SendReadInputStatus(int i_iAddr,
									 int i_iStartReg,
									 int i_iNumReg)
{
	_logger->log(Logger::LOG_LEV_DEBUG, ">>> SendReadInputStatus <<<");

	int iHexLen, iRTULen;
	BYTE szRTUCmd[ADAM_MAX_MSGLEN]={0};
	char szModCmd[ADAM_MAX_MSGLEN] = {0};

    memset(_szHexSend, 0, ADAM_MAX_MSGLEN);
	sprintf(szModCmd, "%02X%02X%04X%04X", i_iAddr, 2, i_iStartReg, i_iNumReg);
	sprintf(_szHexSend, "0000000000%02X%s", strlen(szModCmd)/2, szModCmd);
	iHexLen = strlen(_szHexSend);
	_logger->log(Logger::LOG_LEV_TRACE, "Sending      : %s", _szHexSend );
	// form TCP-RTU command
	HexToRTU(_szHexSend, szRTUCmd);
	iRTULen = iHexLen/2;
	return SendTCP(szRTUCmd, iRTULen);
}

bool AdamModbus::ReadCoilStatus( int i_iAddr,
                                 int i_iStartReg,
                                 int i_iNumReg,
                                 void *data)
{

   if (!SendReadCoilStatus(i_iAddr, i_iStartReg, i_iNumReg))
      return false;

   BYTE szRecv[ADAM_MAX_MSGLEN];
   char debug[ADAM_MAX_MSGLEN*2+1];
   int recvDataLen;

   if ((recvDataLen = RecvTCP(szRecv)) >= 4) {
    	RTUToHex(szRecv, recvDataLen, debug);
      debug[recvDataLen*2]=0;
      memcpy(data, szRecv+9, 1+i_iNumReg/8);
	   _logger->log(Logger::LOG_LEV_TRACE, "Received (%d): %s", recvDataLen,debug );
      return true;
   } else
      return false;
}


bool AdamModbus::ReadInputStatus( int i_iAddr,
                                 int i_iStartReg,
                                 int i_iNumReg,
                                 void *data)
{

   if (!SendReadInputStatus(i_iAddr, i_iStartReg, i_iNumReg))
      return false;

   BYTE szRecv[ADAM_MAX_MSGLEN];
   char debug[ADAM_MAX_MSGLEN*2+1];
   int recvDataLen;

   if ((recvDataLen = RecvTCP(szRecv)) >= 4) {
    	RTUToHex(szRecv, recvDataLen, debug);
      debug[recvDataLen*2]=0;
      memcpy(data, szRecv+9, 1+i_iNumReg/8);
	   _logger->log(Logger::LOG_LEV_TRACE, "Received (%d): %s", recvDataLen, debug );
      return true;
   } else
      return false;
}

// function (06 HEX)
// TESTED 20.03.2008: OK
bool AdamModbus::RecvPresetSingleReg()
{
	_logger->log(Logger::LOG_LEV_DEBUG, ">>> RecvPresetSingleReg <<<");

	BYTE szRecv[ADAM_MAX_MSGLEN];

	// recvDataLen must be equal 12;
	// 'header(12)', 'addr(2)', 'fun(2)', 'Coil addr Hi-Lo(4)',
	// 'Force data Hi-Lo(4)', 24 TCP-HEX bytes, 12 TCP-RTU bytes
	int recvDataLen = RecvTCP(szRecv);
	if (recvDataLen==12)
    {
    	_logger->log(Logger::LOG_LEV_TRACE, ">> Received data: %d bytes", recvDataLen);
    	RTUToHex(szRecv, recvDataLen, _szHexRecv);
    	_logger->log(Logger::LOG_LEV_TRACE, ">> Sent function code: %2X %2X", _szHexSend[14], _szHexSend[15]);
        _logger->log(Logger::LOG_LEV_TRACE, ">> Recv function code: %2X %2X", _szHexRecv[14], _szHexRecv[15]);
        if (_szHexSend[14] == _szHexRecv[14] &&
			_szHexSend[15] == _szHexRecv[15])
			// function code must be the same, otherwise, exception
			return true;
    }
	return false;
}

// function (0F HEX)
bool AdamModbus::SendForceMultiCoils(int i_iAddr,
									 int i_iCoilIndex,
									 int i_iTotalPoint,
									 int i_iTotalByte,
									 BYTE i_szData[])
{
	int idx, iHexLen, iRTULen;
	BYTE szRTUCmd[ADAM_MAX_MSGLEN]={0};
	char szTmp[8];
	char szModCmd[ADAM_MAX_MSGLEN] = {0};

	// form modbus command
    memset(_szHexSend, 0, ADAM_MAX_MSGLEN);
	sprintf(szModCmd, "%02X%02X%04X%04X%02X", i_iAddr, 0x0F, i_iCoilIndex-1,
			i_iTotalPoint, i_iTotalByte);
	for (idx=0; idx<i_iTotalByte; idx++)
	{
		memset(szTmp, 0, 8);
		sprintf(szTmp, "%02X", i_szData[idx]);
		strcat(szModCmd, szTmp);
	}

	// form TCP-HEX command
	sprintf(_szHexSend, "0000000000%02X%s", strlen(szModCmd)/2, szModCmd);
	iHexLen = strlen(_szHexSend);
	// form TCP-RTU command
	HexToRTU(_szHexSend, szRTUCmd);
	iRTULen = iHexLen/2;
	return SendTCP(szRTUCmd, iRTULen);
}

// function (0F HEX)
bool AdamModbus::RecvForceMultiCoils()
{
	BYTE szRecv[ADAM_MAX_MSGLEN];

	// recvDataLen must be equal 12;
	// 'header(12)', 'addr(2)', 'fun(2)', 'addr Hi-Lo(4)',
	// 'quantity Hi-Lo(4)', 24 TCP-HEX bytes, 12 TCP-RTU bytes
	int recvDataLen = RecvTCP(szRecv);
	if (recvDataLen==12)
    {
        RTUToHex(szRecv, recvDataLen, _szHexRecv);
		if (_szHexSend[14] == _szHexRecv[14] &&
			_szHexSend[15] == _szHexRecv[15])
			// function code must be the same, otherwise, exception
			return true;
    }
	return false;
}

// function (10 HEX)
bool AdamModbus::SendPresetMultiRegs(int i_iAddr,
									 int i_iStartReg,
									 int i_iTotalReg,
									 int i_iTotalByte,
									 BYTE i_szData[])
{
	int idx, iHexLen, iRTULen;
	BYTE szRTUCmd[ADAM_MAX_MSGLEN]={0};
	char szTmp[8];
	char szModCmd[ADAM_MAX_MSGLEN] = {0};

	// form modbus command
    memset(_szHexSend, 0, ADAM_MAX_MSGLEN);
	sprintf(szModCmd, "%02X%02X%04X%04X%02X", i_iAddr, 0x10, i_iStartReg-1,
			i_iTotalReg, i_iTotalByte);
	for (idx=0; idx<i_iTotalByte; idx++)
	{
		memset(szTmp, 0, 8);
		sprintf(szTmp, "%02X", i_szData[idx]);
		strcat(szModCmd, szTmp);
	}

	// form TCP-HEX command
	sprintf(_szHexSend, "0000000000%02X%s", strlen(szModCmd)/2, szModCmd);
	iHexLen = strlen(_szHexSend);
	// form TCP-RTU command
	HexToRTU(_szHexSend, szRTUCmd);
	iRTULen = iHexLen/2;
	return SendTCP(szRTUCmd, iRTULen);
}

// function (10 HEX)
bool AdamModbus::RecvPresetMultiRegs()
{
	BYTE szRecv[ADAM_MAX_MSGLEN];

	// recvDataLen must be equal 12;
	// 'header(12)', 'addr(2)', 'fun(2)', 'addr Hi-Lo(4)',
	// 'No. of reg Hi-Lo(4)', 24 TCP-HEX bytes, 12 TCP-RTU bytes
	int recvDataLen = RecvTCP(szRecv);
	if (recvDataLen==12)
    {
        RTUToHex(szRecv, recvDataLen, _szHexRecv);
		if (_szHexSend[14] == _szHexRecv[14] &&
			_szHexSend[15] == _szHexRecv[15])
			// function code must be the same, otherwise, exception
			return true;
    }
	return false;
}


void AdamModbus::HexToRTU(char *i_szHexCmd, BYTE *o_szRTUCmd) {
	int idx, iRTULen;
	char szTmp[8];

	iRTULen = strlen(i_szHexCmd)/2;
	for (idx=0; idx<iRTULen; idx++)
	{
		unsigned int aux;
		memset(szTmp, 0, 8);
		memcpy(szTmp, i_szHexCmd + idx*2, 2);
		sscanf(szTmp, "%X", &aux);
		o_szRTUCmd[idx]=aux&0xff;
	}
}

void AdamModbus::RTUToHex(BYTE *i_szRTUCmd, int i_RTULen, char *o_szHexCmd) {
	int idx;
	char szTmp[8];

	o_szHexCmd[0] = '\0';
	for (idx=0; idx<i_RTULen; idx++)
	{
		memset(szTmp, 0, 8);
		sprintf(szTmp, "%02X", i_szRTUCmd[idx]);
		strcat(o_szHexCmd, szTmp);
	}
}

bool AdamModbus::SendTCP(BYTE i_szData[], int i_iLen) {

	try {
		_tcpConn->sendTcp(i_szData, i_iLen);
		return true;
	}
	catch(TcpException& e) {
		_logger->log(Logger::LOG_LEV_ERROR, e.what().c_str());
		return false;
	}
}

int AdamModbus::RecvTCP(BYTE o_szData[])	{

	try {
		int recvDataLen = _tcpConn->receiveTcp(o_szData, ADAM_MAX_MSGLEN, _timeout_ms);
		return recvDataLen;
	}
	catch(TcpException& e) {
		_logger->log(Logger::LOG_LEV_ERROR, e.what().c_str());
		return 0;
	}
}

void AdamModbus::setLogLevel( int level) {
   _logger->setLevel(level);
}


// ******************************** ADAMWATCHDOG ********************************* //

AdamWatchdog::AdamWatchdog( bool autoStart) throw(AdamException)
			 :AdamModbus( "ADAM-WATCHDOG", false) {

	_wdStarted = false;
	_wdEnabled = false;

	_period_us = 1000*(int)_config["WD_PERIOD_MS"];

	_WD_CHANNEL_ADDR =    _config["WD_CHANNEL_ADDR"];
	_WD_DELAY_REG_ADDR =  _config["WD_DELAY_REG_ADDR"];
	_WD_DELAY_MS10 =      _config["WD_DELAY_MS10"];

	_logger->log(Logger::LOG_LEV_INFO, "--------------- AdamWatchdog --------------");
	_logger->log(Logger::LOG_LEV_INFO, "Period:           %d ms", _period_us/1000);
	_logger->log(Logger::LOG_LEV_INFO, "Channel addr:     %d", _WD_CHANNEL_ADDR);
	_logger->log(Logger::LOG_LEV_INFO, "Delay (reg addr): %d ms/10 (%d)", _WD_DELAY_MS10, _WD_DELAY_REG_ADDR);
	_logger->log(Logger::LOG_LEV_INFO, "-------------------------------------------");
	printf("\n");

	if(autoStart) {
		start();
	}
	else {
		_logger->log(Logger::LOG_LEV_INFO, "AdamWatchdog is now stopped: start it with start()");
	}
}

AdamWatchdog::~AdamWatchdog() {
	stop();
	_logger->log(Logger::LOG_LEV_INFO, "AdamWatchdog destroyed");
}

bool AdamWatchdog::enableWatchdog() {
	_logger->log(Logger::LOG_LEV_DEBUG, "Adam: enabling watchdog...");
	if(SendForceSingleCoil(0x01, _WD_CHANNEL_ADDR, 0) && RecvForceSingleCoil()) {
		if(SendPresetSingleReg(0x01, _WD_DELAY_REG_ADDR, _WD_DELAY_MS10) && RecvPresetSingleReg()) {
			_logger->log(Logger::LOG_LEV_INFO, "Adam: watchdog succesfully enabled");
			return true;
		}
		else {
			_logger->log(Logger::LOG_LEV_ERROR, "Adam: watchdog enabling failed (error in set DELAY)");
			return false;
		}
	}
	else {
		_logger->log(Logger::LOG_LEV_ERROR, "Adam: watchdog enabling failed (error in set CHANNEL to 0)");
		return false;
	}
}


bool AdamWatchdog::sendWatchdog() {
	_logger->log(Logger::LOG_LEV_DEBUG, "Adam: sending watchdog...");
	if(SendForceSingleCoil(0x01, _WD_CHANNEL_ADDR, 1) && RecvForceSingleCoil()) {
		_logger->log(Logger::LOG_LEV_INFO, "Adam: watchdog succesfully sent");
		return true;
	}
	else {
		_logger->log(Logger::LOG_LEV_ERROR, "Adam: watchdog sending failed");
		return false;
	}
}

void AdamWatchdog::start() {
	_logger->log(Logger::LOG_LEV_DEBUG, "Starting AdamWatchdog...");
	if(!_wdStarted) {
		_wdStarted = true;
		_thread = new boost::thread(boost::bind(&AdamWatchdog::execute, this));
	}
	else {
		_logger->log(Logger::LOG_LEV_WARNING, "AdamWatchdog already running!");
	}
}

void AdamWatchdog::stop() {
	_logger->log(Logger::LOG_LEV_DEBUG, "Stopping AdamWatchdog...");
	if(_wdStarted) {
		_wdStarted = false;
		_thread->join();
		_wdEnabled = false;
		delete _thread;
		_logger->log(Logger::LOG_LEV_INFO, "AdamWatchdog succesfully stopped");
	}
	else {
		_logger->log(Logger::LOG_LEV_WARNING, "AdamWatchdog not running!");
	}
}

void AdamWatchdog::execute() {
	// WD enabling loop
	while(_wdStarted) {
		try {
			initTcp();
			_wdEnabled = enableWatchdog();
			// WD loop: when exit try to enable again
			while(_wdStarted && _wdEnabled && sendWatchdog()) {
				nusleep(_period_us);
			}
			clearTcp();
		}
		catch(TcpCreationException& e) {
			// Do nothing, simply retry
			_logger->log(Logger::LOG_LEV_ERROR, "Adam connection failed: will retry in %d ms...", _period_us/1000);
		}

		// Wait before trying a new enable
		nusleep(_period_us);
	}
}

