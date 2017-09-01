function aoDspStartCode(firstDsp,varargin)
% aoDspStartCode(firstDsp,[lastDsp],[connectionNr])
% starts DSP code
% lastDsp default is firstDsp
% connectionNr default is 1

% Author(s): D. Pescoller
%
% Copyright 2004-2008 Microgate s.r.l.
% $Revision 0.1 $ $Date: 2004/08/09
% $Revision 0.2 $ $Date: 09/03/2007 - multiple Crate connection D. Pescoller
% device detection mechanism reading the FPGA logic version (by Mario)
% changed device detection mechanism reading the board serial number (by Mario)
% modified the lastDsp as an optional input argument (by Mario)

connectionNr=1;
lastDsp=firstDsp;
if nargin>=2
    lastDsp=varargin{1};
end
if nargin>=3
    connectionNr=varargin{2};
end

% device detection
dsp_ver=aoGetBoardSN(firstDsp,lastDsp,connectionNr)>=1000;
if     min(dsp_ver) == 0 && max(dsp_ver) == 0
   device = 0;
elseif min(dsp_ver) == 1 && max(dsp_ver) == 1
   device = 1;
else
   error('This function doesn''t support hybrid DSP 8ch & 16ch');
end

% ultimo pacchetto per far partire il programma
% scrittura nel registro VIRPT (0x180730) dell'indirizzo 0
if device==0
   address=hex2dec('180730'); % per TS101
else
   address=hex2dec('1F0330'); % per TS201
end
data=uint32(0);
len=1;
mgp_op_wrsame_dsp(firstDsp,lastDsp,len,address,data,connectionNr,'uint32');
