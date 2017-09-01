function aoDspStopCode(firstDsp,lastDsp,varargin)
% aoDspStopCode(firstDsp,lastDsp,[connectionNr]) 
% starts DSP code
% connectionNr default is 1

% Author(s): D. Pescoller
%
% Copyright 2004-2008 Microgate s.r.l.
% $Revision 0.1 $ $Date: 2004/08/09
% $Revision 0.2 $ $Date: 09/03/2007 - multiple Crate connection D. Pescoller
% $Revision 0.3 $ $Date: 05/05/2010 - multiple Crate connection error corrected, missing on call to aoGetBoardSN - R.Biasi
% device detection mechanism reading the FPGA logic version and included the linkport registers initialization after SW reset
% changed device detection mechanism reading the board serial number

MGP_DEFINE();

connectionNr=1;
if nargin>=3
    connectionNr=varargin{1};
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

if     device == 0
   address=hex2dec('00180359');        % TS101
   data=uint32(hex2dec('00000020'));
elseif device == 1
   address=hex2dec('001F03A0');        % TS201
   data=uint32(hex2dec('00000030'));
else
   error('Invalid TigerShark device selected')
end
% primo pacchetto per mandare il DSP in reset e quindi in idle
len=1;
mgp_op_wrsame_dsp(firstDsp,lastDsp,len,address,data,connectionNr,'uint32');

% set of linkport registers

if device == 0
   % primo pacchetto per resettare il dma di ricezione
   % (write as a quad word in 180448)
   address=hex2dec('180448');
   data=uint32(zeros(1,4));
   len=4;
   FLAGS=MGP_FL_WANTREPLY+MGP_FL_ASQUADWORD; 
   mgp_op_wrsame_dsp(firstDsp,lastDsp,len,address,data,1,'uint32',FLAGS);

   % primo pacchetto per resettare il dma di ricezione
   % (write as a quad word in 18044c)
   address=hex2dec('18044c');
   data=uint32(zeros(1,4));
   len=4;
   FLAGS=MGP_FL_WANTREPLY+MGP_FL_ASQUADWORD; 
   mgp_op_wrsame_dsp(firstDsp,lastDsp,len,address,data,1,'uint32',FLAGS);

   % secondo pacchetto per l'inizializzazione dei linkport 
   % (write as a quad word in 1804e0)
   address=hex2dec('1804e0');
   data=uint32([0,0,hex2dec('4c8'),hex2dec('4c8')]);
   len=4;
   FLAGS=MGP_FL_WANTREPLY+MGP_FL_ASQUADWORD; 
   mgp_op_wrsame_dsp(firstDsp,lastDsp,len,address,data,1,'uint32',FLAGS);
elseif device == 1
   % primo pacchetto per resettare il dma di ricezione
   % (write as a quad word in 1F0020)
   address=hex2dec('1F0020');
   data=uint32(zeros(1,4));
   len=4;
   FLAGS=MGP_FL_WANTREPLY+MGP_FL_ASQUADWORD; 
   mgp_op_wrsame_dsp(firstDsp,lastDsp,len,address,data,1,'uint32',FLAGS);

   % primo pacchetto per resettare il dma di trasmissione
   % (write as a quad word in 1F0040)
   address=hex2dec('1F0040');
   data=uint32(zeros(1,4));
   len=4;
   FLAGS=MGP_FL_WANTREPLY+MGP_FL_ASQUADWORD; 
   mgp_op_wrsame_dsp(firstDsp,lastDsp,len,address,data,1,'uint32',FLAGS);

   % inizializzazione dei registri linkport (RX)
   len=1;
   FLAGS=MGP_FL_WANTREPLY; 
   address=hex2dec('1F00E0');
   data=uint32(hex2dec('00000000'));
   mgp_op_wrsame_dsp(firstDsp,lastDsp,len,address,data,1,'uint32',FLAGS);
   data=uint32(hex2dec('00000011'));
   mgp_op_wrsame_dsp(firstDsp,lastDsp,len,address,data,1,'uint32',FLAGS);
   % inizializzazione dei registri linkport (TX)
   address=hex2dec('1F00E4');
   data=uint32(hex2dec('00000000'));
   mgp_op_wrsame_dsp(firstDsp,lastDsp,len,address,data,1,'uint32',FLAGS);
   data=uint32(hex2dec('00000059'));
   mgp_op_wrsame_dsp(firstDsp,lastDsp,len,address,data,1,'uint32',FLAGS);
end
