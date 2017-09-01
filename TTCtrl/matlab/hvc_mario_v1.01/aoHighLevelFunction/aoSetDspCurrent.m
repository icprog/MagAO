function aoSetDspCurrent(currentVector,varargin)
% aoSetDspCurrent(currentVector,[firstDsp],[connectionNr])
% currentVector is an 8-element array representing the currents of each
% single Dsp (Ampere)
% currentVector can also be supplied in DAC-bits by converting it to uint32
% for example:
% aoSetDspCurrent(uint32([32768,32768,32768,32768,32768,32768,32768,32768]))
% firstDsp is optional default is 0
% lastDsp is = firstDsp + 1  default is 1
% connectionNr default is 1
% is an optional argument indicating which connection to use
% (for multiple crate operation) 


% Author(s): D. Pescoller, M. Andrighettoni
%
% Copyright 2004-2008 Microgate s.r.l.
% $Revision 0.1 $ $Date: 14/01/2005
% $Revision 0.2 $ $Date: 14/04/2007 multi-crate capability added, R.Biasi

MGP_DEFINE();

% old value for dsp nios release <= 3.07
% DAC_VALUE_ADDRESS = hex2dec('80172');
% new value for dsp nios release >= 3.08
DAC_VALUE_ADDRESS = hex2dec('1002');

connectionNr=1;
firstDsp=0;
if nargin==1
elseif nargin==2
   firstDsp=varargin{1};
elseif nargin==3
   firstDsp=varargin{1};
   connectionNr=varargin{2};
else
   error('invalid arguments number');
end

%verifica che il numero di canali da settare sia un multiplo di 8
if (rem(length(currentVector),8))
   error('invalid current vector length, should be a multiple of 8');
end

lastDsp=firstDsp+length(currentVector)/4-1;

%setup of DSP registers
mgp_op_wrsame_dsp(firstDsp,lastDsp,1,'0x180359','0x07700000',connectionNr);

%trasforma currentVector
if isfloat(currentVector)
    currentVector=currentVector/DAC_BIT_TO_AMP+DAC_ZERO_BIT;
end;
currentVector=uint32(currentVector);
%clamping valori
currentVector(currentVector>63353)=65535;

%set current
currentDWord=currentVector(1:2:end)+bitshift(currentVector(2:2:end),16);

for i=0:length(currentDWord)/2-1
   mgp_op_wrsame_dsp(firstDsp+i,firstDsp+i,2,DAC_VALUE_ADDRESS,currentDWord(i*2+1:i*2+2),connectionNr);
end

%preparazione
mgp_op_wrsame_dsp(firstDsp,lastDsp,1,'0x18035A','0xFEFFFFFF',connectionNr);
pause(.001);
%ripristino flag
mgp_op_wrsame_dsp(firstDsp,lastDsp,1,'0x180359','0x01000000',connectionNr);
