MODULE idl_wraplib 
DESCRIPTION idl wrapper
VERSION 1.0  
SOURCE LB
BUILD_DATE SEP 14 2006 

##### INITIALIZATION #####

# @Function{API}: IDL_SETUP_WRAP
# Initialize the library and connects with the MsgD.
# Parameters: IDL_STRING configFile
# @
FUNCTION IDL_SETUP_WRAP		1 1

# @Function{API}: IDL_CLOSE_WRAP
# Clean the library and disconnects from the MsgD.
# @
FUNCTION IDL_CLOSE_WRAP		0 0	

##### GENERAL INFO #####

# @Function{API}: IDL_GETADOPTSIDE
# Get Side. Wrapper of Utils::GetAdoptSide
# @
FUNCTION IDL_GETADOPTSIDE		0 0	

# @Function{API}: IDL_GETADOPTSUBSYSTEM
# Get Subsystem. Wrapper of Utils::GetAdoptSubsystem
# @
FUNCTION IDL_GETADOPTSUBSYSTEM		0 0	

# @Function{API}: IDL_GETADOPTLOG
# Get Log directory. Wrapper of Utils::GetAdoptLog
# @
FUNCTION IDL_GETADOPTLOG		0 0	

##### LOG #####
# @Function{API}: IDL_LOG_WRAP
# Log into STD AO logfile
# Parameters: IDL_STRING string to log
# @
FUNCTION IDL_LOG_WRAP		0 2

##### UDP CONNECTION #####

# @Function{API}: UDPCONNECTION_INIT_WRAP
# Initialize the UdpConnection
# Call this before calling UDPCONNECTION_SEND_WRAP or UDPCONNECTION_RECEIVE_WRAP
# Parameters: IDL_INT localPort, IDL_STRING remoteIp, IDL_INT remotePort, IDL_INT receiveTimeout_ms
# @
FUNCTION UDPCONNECTION_INIT_WRAP 4 4

# @Function{API}: UDPCONNECTION_DESTROY_WRAP
# Destroy the UdpConnection
# @
FUNCTION UDPCONNECTION_DESTROY_WRAP 0 0

# @Function{API}: UDPCONNECTION_SEND_WRAP
# Send a buffer using the initialized UdpConnection
# Parameters: UCHAR* bufferOut, IDL_INT bufferLength (in BYTES)
# @
FUNCTION UDPCONNECTION_SEND_WRAP 2 2

# @Function{API}: UDPCONNECTION_RECEIVE_WRAP
# Receive a buffer using the initialized UdpConnection
# Parameters: UIDL_INT bufferLength (in BYTES)
# Returns the received buffer.
# @
FUNCTION UDPCONNECTION_RECEIVE_WRAP 1 1


##### ADAM #####

# @Function{API}: ADAM_CONNECT_WRAP
# Initialize the Adam connection (Udp)
# Call this before calling ADAM_CMD_WRAP.
# Parameters: none, all are got from adam.conf (the specified parameter is dummy)
#
# Es. IDL> adam_connect_wrap()
#
# @
FUNCTION ADAM_CONNECT_WRAP 0 1 KEYWORDS
FUNCTION ADAM_MODBUS_INIT_WRAP 0 1 KEYWORDS


# @Function{API}: ADAM_DISCONNECT_WRAP
# Close the connection with Adam
# Parameters: none (the specified parameter is dummy)
#
# Es. IDL> adam_disconnect_wrap()
#
# @
FUNCTION ADAM_DISCONNECT_WRAP	0 1 KEYWORDS
FUNCTION ADAM_MODBUS_DISCONNECT_WRAP	0 1 KEYWORDS

# @Function{API}: ADAM_CMD_WRAP
# Send a command to Adam and return the answer.
# Parameters: IDL_STRING cmd, IDL_INT cmdLen, IDL_INT answLen
#
# NOTE: the command must be "carriage return terminated". This
# means that the ASCII char(13), that is '\r' in C, must terminate it.
#
# Es.  IDL> print,  adam_cmd_wrap('$01M' + string([13B]), 5, 8)
#	   IDL> !016050
#	   IDL> print,  adam_cmd_wrap('$01M' + string([13B]), 5, 10)
#	   IDL> !01 1.27
#
# @
FUNCTION ADAM_CMD_WRAP 3 3 KEYWORDS
FUNCTION ADAM_MODBUS_RI_WRAP 0 0 KEYWORDS
FUNCTION ADAM_MODBUS_RO_WRAP 0 0 KEYWORDS
FUNCTION ADAM_MODBUS_WS_WRAP 1 1 KEYWORDS

# @Function{API}: ADAM_LED
# Send a led on/off to Adam2 and return the answer.
# Parameters: int led_number, /ON, /OFF 
#
# Es.  IDL> print,  adam_LED(0, /ON)
#
# @
FUNCTION ADAM_LED 1 1 KEYWORDS


##### DEVICES RESET #####

FUNCTION RESET_DEVICES_WRAP				6 6
FUNCTION RESET_DEVICES_MULTI_WRAP		7 7
FUNCTION RESET_DEVICES_MULTI_CHECK_WRAP	8 8


##### DIAGNOSTIC #####
 
FUNCTION GETDIAGNPARAM  	   5 5 KEYWORDS
FUNCTION SETDIAGNPARAM  	   4 5 KEYWORDS 
FUNCTION GETDIAGNVALUE  	   5 5 KEYWORDS
FUNCTION GETDIAGNBUFFER 	   5 5 KEYWORDS
FUNCTION DUMPDIAGNPARAMS 	   2 2 KEYWORDS

# @Function{API}: IDL_MASTDIAGN_DUMP_HISTORY
# Request to masterdiagnositc to dump the history buffer to file
# Parameters: string masterdiagnosticconffilename
# Returns: NONE
# @
FUNCTION DUMP_DIAGN_HISTORY	1 1

# @Function{API}: LOAD_OFFLINE_DIAG_FRAMES
# Request a DiagnApp to load
# Parameters: string diagnapp name, int frame index
# Returns: NONE 
# @
FUNCTION LOAD_OFFLINE_DIAG_FRAMES 2 2


##### LOG LEVEL #####

FUNCTION SETLOGLEVEL    2 3 KEYWORDS
FUNCTION GETLOGLEVEL    2 2 KEYWORDS


##### HEXAPOD ###

# HEXGETPOS: read the RTDB variable "SIDE.AOS00.HXPD_POS"
# hexmovsph: raggio[m], alpha [arcsec], beta[arcsec]
FUNCTION HEXMOVETO      	1 1 KEYWORDS
FUNCTION HEXMOVEBY      	1 1 KEYWORDS
FUNCTION HEXMOVESPH       	1 1 KEYWORDS     
FUNCTION HEXBRAKE      		0 0 KEYWORDS
FUNCTION HEXHOME      		0 0 


##### BCUs INFO #####

# @Function{API}: GETBCUID_WRAP
# Return the BCU ID correponding to a given BCU name.
#  
# Defined names are (see lib/BcuCommon.cpp for details):
#  - BCU_SWITCH
#  - BCU_MIRROR_0, ..., BCU_MIRROR_5
# @
FUNCTION GETBCUID_WRAP		1 1  


# @Function{API}: GETBCUIP_WRAP
# Return the BCU IP  correponding to a given BCU name.
# 
# Defined names are (see lib/BcuCommon.cpp for details):
#  - BCU_SWITCH
#  - BCU_MIRROR_0, ..., BCU_MIRROR_5
# @
FUNCTION GETBCUIP_WRAP       1 1   

# @Function{API}: GETBCUMASTERUDPPORT_WRAP
# Return the BCU MASTER UDP PORT correponding to a given BCU name.
# 
# Defined names are (see lib/BcuCommon.cpp for details):
#  - BCU_SWITCH 
#  - BCU_MIRROR_0, ..., BCU_MIRROR_5
# @
FUNCTION GETBCUMASTERUDPPORT_WRAP      1 1   

# @Function{API}: ISBCUREADY
# Check if BCUs are ready.
# @
FUNCTION ISBCUREADY		1 2  KEYWORDS 


##### BCU MEMORY READ/WRITE #####

FUNCTION READ_SEQ_MULTI_WRAP	9 9
FUNCTION WRITE_SEQ_MULTI_WRAP	10 10
FUNCTION WRITE_SAME_MULTI_WRAP	10 10

FUNCTION READ_SEQ_WRAP		8 8
FUNCTION WRITE_SEQ_WRAP		9 9
FUNCTION WRITE_SAME_WRAP	9 9


##### RTDB VARS HANDLING #####

# @Function{API}: GET_FULL_VARNAME_WRAP
# Return the full name (SIDE.PREFIX.NAME_NUM.DIR) from a short name.
# Parameters: IDL_STRING varShortName.
# @
FUNCTION GET_FULL_VARNAME_WRAP 1 1


# @Function{API}: READ_VAR_WRAP
# Read a scalar or array RTDB variable
# Parameters: IDL_STRING varName, (multy-type) varValue, IDL_LONG timeout_ms
# @
FUNCTION READ_VAR_WRAP 3 3


# @Function{API}: WRITE_VAR_WRAP
# Write a scalar or array RTDB variable
# Parameters: IDL_STRING varName, (multy-type) varValue, IDL_LONG vartype, IDL_LONG timeout_ms
# Supported var types: 
#	- INT_VARIABLE: 1001 (16 bit IDL_INT, 32 bit IDL_LONG)
#	- REAL_VARIABLE: 1002 (32 bit IDL_FLOAT, 64 bit IDL_DOUBLE)
#	- CHAR_VARIABLE: 1003
# @
FUNCTION WRITE_VAR_WRAP 4 4


# @Function{API}: READ_SCALAR_VAR_WRAP
# DEPRECATED: use READ_VAR instead.
# @
FUNCTION READ_SCALAR_VAR_WRAP 3 3


# @Function{API}: WRITE_SCALAR_VAR_WRAP
# DEPRECATED: use WRITE_VAR instead.
# @
FUNCTION WRITE_SCALAR_VAR_WRAP 4 4


### 4D INTERFEROMETER ###

# @Function{API}: IDL_4D_INIT
# Initialize the interferometer interface
# Always returns 0 (the initialization can't fail)
# @
FUNCTION IDL_4D_INIT		0 0

# @Function{API}: IDL_4D_FINALIZE
# Finalize the interferometer interface
# Always returns 0 (the finalization can't fail)
# @
FUNCTION IDL_4D_FINALIZE		0 0

# @Function{API}: IDL_4D_SET_TRIGGER
# Enable or disable the trigger
# Parameters: bool enable/disable
# Returns 0 on success, an error code if failed
# @
FUNCTION IDL_4D_SET_TRIGGER		1 1

# @Function{API}: IDL_4D_SET_POSTPROC
# Enable or disable the post-processing (at the moment the generation of HDF5 files)
# Parameters: string ppType (NONE, HDF5)
# Returns 0 on success, an error code if failed
# @
FUNCTION IDL_4D_SET_POSTPROC	1 1

# @Function{API}: IDL_4D_GET_MEASURE
# Get a single measurement from the PhaseCam 4020
# Parameters: meas name
# Returns the remote file name if success, an error code if failed
# @
FUNCTION IDL_4D_GET_MEASURE		1 1

# @Function{API}: IDL_4D_GET_BURST
# Get a burst of measurements from the PhaseCam 4020
# Parameters: int 'meas num', string 'burst name' (optional)
# If 'burst name' not specified, use an automatic name
# Returns the remote path if success, an error code if failed
# @
FUNCTION IDL_4D_GET_BURST	1 2

# @Function{API}: IDL_4D_TRIGGER
# Trigger the interferometer using the parallel port, pin ??.
# Parameters: None
# @
FUNCTION IDL_4D_TRIGGER		0 0

# @Function{API}: IDL_4D_POSTPROCESS
# Perform the post-processing previously set
# Parameters: string 'sourcePath', string 'measPrefix', int 'meas num' (optional, default 1)
# Returns the remote path if success, an error code if failed
# @
FUNCTION IDL_4D_POSTPROCESS	2 3

#@Function{API}: IDL_SET_GAIN
#
# Sets the AO loop gain.
FUNCTION SET_AO_GAIN 1 1

#@Function{API}: GETIFSPEC_WRAP
#
# Sets the AO loop gain.
FUNCTION GETIFSPEC_WRAP 1 1

#@Function{API}: LAO_STRERROR_WRAP
#
# Sets the AO loop gain.
FUNCTION LAO_STRERROR_WRAP 1 1
