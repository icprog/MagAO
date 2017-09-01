function MGP_DEFINE();
% declares various AO comunication properties
% this is like a C-INCLUDE file

% Author(s): D. Pescoller
%
% Copyright 2004-2008 Microgate s.r.l.
% $Revision 0.1 $ $Date: 22/12/2004

%disp('MGP_DEFINE')

assignin('caller','MGP_OP_CMD_SUCCESS',200);
assignin('caller','MAX_DWORD_LEN',364);
assignin('caller','DSP_MEM_BASE',[0,hex2dec('80000'),hex2dec('100000'),hex2dec('180000')]);
assignin('caller','DSP_MEM_SIZE',[hex2dec('10000'),hex2dec('10000'),hex2dec('10000'),hex2dec('800')]);

assignin('caller','MGP_FL_WANTREPLY',1);
assignin('caller','MGP_FL_ASQUADWORD',2);
assignin('caller','MGP_FL_HL_RESET',4);
assignin('caller','MGP_FL_HL_IRQ',8);
assignin('caller','MGP_FL_HL_DMPM',16);

MGP_FLASH_TABLE(1).name='default_logic';
MGP_FLASH_TABLE(1).startAddress=hex2dec('0');
MGP_FLASH_TABLE(1).len=hex2dec('00040000');
MGP_FLASH_TABLE(2).name='user_logic';
MGP_FLASH_TABLE(2).startAddress=hex2dec('00040000');
MGP_FLASH_TABLE(2).len=hex2dec('00040000');
MGP_FLASH_TABLE(3).name='default_program';
MGP_FLASH_TABLE(3).startAddress=hex2dec('00080000');
MGP_FLASH_TABLE(3).len=hex2dec('00008000');
MGP_FLASH_TABLE(4).name='user_program';
MGP_FLASH_TABLE(4).startAddress=hex2dec('00088000');
MGP_FLASH_TABLE(4).len=hex2dec('00040000');
MGP_FLASH_TABLE(5).name='conf_area';
MGP_FLASH_TABLE(5).startAddress=hex2dec('000fc000');
MGP_FLASH_TABLE(5).len=hex2dec('00002800');
assignin('caller','MGP_FLASH_TABLE',MGP_FLASH_TABLE);


%DSP boards DAC parameters
assignin('caller','DAC_BIT_TO_AMP',2.96104e-5);
assignin('caller','DAC_ZERO_BIT',32768);
