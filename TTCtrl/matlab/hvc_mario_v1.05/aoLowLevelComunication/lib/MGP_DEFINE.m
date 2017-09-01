function MGP_DEFINE()
% declares various AO comunication properties
% this is like a C-INCLUDE file

% Author(s): D. Pescoller
%
% Copyright 2004-2008 Microgate s.r.l.
% $Revision 0.1 $ $Date: 22/12/2004

assignin('caller','MGP_WRITETIMEOUT_SHORT',10); % timeout in seconds
assignin('caller','MGP_WRITETIMEOUT_LONG',70); % timeout in seconds
assignin('caller','MGP_WRITETIMEOUT_LONGLONG',180); % timeout in seconds
assignin('caller','MGP_READTIMEOUT_SHORT',10); % timeout in seconds
assignin('caller','MGP_READTIMEOUT_LONG',70); % timeout in seconds
assignin('caller','MGP_READTIMEOUT_LONGLONG',180); % timeout in seconds

assignin('caller','MGP_OP_CMD_SUCCESS',200);
assignin('caller','MAX_DWORD_LEN',364);
% DSP memory mapping for TS101
% assignin('caller','DSP_MEM_BASE',[0,hex2dec('80000'),hex2dec('100000'),hex2dec('180000')]);
assignin('caller','DSP_MEM_BASE',[ 0      524288     1048576     1572864]);
% assignin('caller','DSP_MEM_SIZE',[hex2dec('10000'),hex2dec('10000'),hex2dec('10000'),hex2dec('800')]);
assignin('caller','DSP_MEM_SIZE',[ 65536       65536       65536        2048]);
% DSP memory mapping for TS201
%assignin('caller','DSP_MEM_BASE',[hex2dec('000000'),hex2dec('040000'),hex2dec('080000'),hex2dec('0C0000'),hex2dec('100000'),hex2dec('140000'),hex2dec('1E0000'),hex2dec('1F0000')]);
assignin('caller','DSP_MEM_BASE',[ 0      262144      524288      786432     1048576     1310720     1966080     2031616]);
%assignin('caller','DSP_MEM_SIZE',[hex2dec('020000'),hex2dec('020000'),hex2dec('020000'),hex2dec('020000'),hex2dec('020000'),hex2dec('020000'),hex2dec('000400'),hex2dec('000400')]);
assignin('caller','DSP_MEM_SIZE',[ 131072      131072      131072      131072      131072      131072        1024        1024]);

assignin('caller','MGP_FL_WANTREPLY',1);
assignin('caller','MGP_FL_ASQUADWORD',2);
assignin('caller','MGP_FL_HL_RESET',4);
assignin('caller','MGP_FL_HL_IRQ',8);
assignin('caller','MGP_FL_HL_DMPM',16);

% old flash map
MGP_FLASH_TABLE(1).name='default_logic';
% MGP_FLASH_TABLE(1).startAddress=hex2dec('0');
MGP_FLASH_TABLE(1).startAddress=0;
% MGP_FLASH_TABLE(1).len=hex2dec('00040000');
MGP_FLASH_TABLE(1).len=262144;
MGP_FLASH_TABLE(2).name='user_logic';
% MGP_FLASH_TABLE(2).startAddress=hex2dec('00040000');
MGP_FLASH_TABLE(2).startAddress=262144;
% MGP_FLASH_TABLE(2).len=hex2dec('00040000');
MGP_FLASH_TABLE(2).len=262144;
MGP_FLASH_TABLE(3).name='default_program';
% MGP_FLASH_TABLE(3).startAddress=hex2dec('00080000');
MGP_FLASH_TABLE(3).startAddress=524288;
% MGP_FLASH_TABLE(3).len=hex2dec('00008000');
MGP_FLASH_TABLE(3).len=32768;
MGP_FLASH_TABLE(4).name='user_program';
% MGP_FLASH_TABLE(4).startAddress=hex2dec('00088000');
MGP_FLASH_TABLE(4).startAddress=557056;
% MGP_FLASH_TABLE(4).len=hex2dec('00040000');
MGP_FLASH_TABLE(4).len=262144;
MGP_FLASH_TABLE(5).name='conf_area';
% MGP_FLASH_TABLE(5).startAddress=hex2dec('000fc000');
MGP_FLASH_TABLE(5).startAddress=1032192;
% MGP_FLASH_TABLE(5).len=hex2dec('00002800');
MGP_FLASH_TABLE(5).len=10240;
assignin('caller','MGP_FLASH_TABLE',MGP_FLASH_TABLE);

assignin('caller','DEFAULT_LOGIC',1);
assignin('caller','USER_LOGIC',2);
assignin('caller','DEFAULT_PROGRAM',3);
assignin('caller','USER_PROGRAM',4);
assignin('caller','CONF_AREA',5);


% new flash map
MGP_FLASH_TABLE(1).name='default_logic';
% MGP_FLASH_TABLE(1).startAddress=hex2dec('0');
MGP_FLASH_TABLE(1).startAddress=0;
% MGP_FLASH_TABLE(1).len=hex2dec('00180000')/4;
MGP_FLASH_TABLE(1).len=393216;
MGP_FLASH_TABLE(2).name='default_program';
MGP_FLASH_TABLE(2).startAddress=MGP_FLASH_TABLE(1).startAddress+MGP_FLASH_TABLE(1).len;
% MGP_FLASH_TABLE(2).len=hex2dec('00040000')/4;
MGP_FLASH_TABLE(2).len=65536;
MGP_FLASH_TABLE(3).name='user_program';
MGP_FLASH_TABLE(3).startAddress=MGP_FLASH_TABLE(2).startAddress+MGP_FLASH_TABLE(2).len;
% MGP_FLASH_TABLE(3).len=hex2dec('00040000')/4;
MGP_FLASH_TABLE(3).len=65536;
MGP_FLASH_TABLE(4).name='user_logic';
MGP_FLASH_TABLE(4).startAddress=MGP_FLASH_TABLE(3).startAddress+MGP_FLASH_TABLE(3).len;
% MGP_FLASH_TABLE(4).len=hex2dec('00180000')/4;
MGP_FLASH_TABLE(4).len=393216;
MGP_FLASH_TABLE(5).name='unused_area0';
MGP_FLASH_TABLE(5).startAddress=MGP_FLASH_TABLE(4).startAddress+MGP_FLASH_TABLE(4).len;
% MGP_FLASH_TABLE(5).len=hex2dec('00070000')/4;
MGP_FLASH_TABLE(5).len=114688;
MGP_FLASH_TABLE(6).name='conf_area';
MGP_FLASH_TABLE(6).startAddress=MGP_FLASH_TABLE(5).startAddress+MGP_FLASH_TABLE(5).len;
% MGP_FLASH_TABLE(6).len=hex2dec('0000A000')/4;
MGP_FLASH_TABLE(6).len=10240;
MGP_FLASH_TABLE(7).name='unused_area1';
MGP_FLASH_TABLE(7).startAddress=MGP_FLASH_TABLE(6).startAddress+MGP_FLASH_TABLE(6).len;
% MGP_FLASH_TABLE(7).len=hex2dec('00006000');
MGP_FLASH_TABLE(7).len=24576;
assignin('caller','MGP_FLASH_TABLE',MGP_FLASH_TABLE);


assignin('caller','DEFAULT_LOGIC',1);
assignin('caller','DEFAULT_PROGRAM',2);
assignin('caller','USER_PROGRAM',3);
assignin('caller','USER_LOGIC',4);
assignin('caller','CONF_AREA',6);


%DSP boards DAC parameters
assignin('caller','DAC_BIT_TO_AMP',2.96104e-5);
assignin('caller','DAC_ZERO_BIT',32768);