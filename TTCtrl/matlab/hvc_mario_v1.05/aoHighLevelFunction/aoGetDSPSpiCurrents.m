function coils_curr=aoGetDSPSpiCurrents(varargin)
% coils_curr=aoGetDSPSpiCurrents([firstDsp],[lastDsp],[connectionNr])
% Gests DSP status and returns a vector with 8 currents.
% Author(s): M. Andrighettoni
%
% Copyright 2004-2008 Microgate s.r.l.
% $Revision 0.1 $ $Date: 24/08/2006
% $Revision 0.2 $ $Date: 21/03/2007
% $Revision 0.3 $ $Date: 03/10/2008 modified to extend the routine for the 16ch DSP board
% changed device detection mechanism reading the board serial number

firstDsp=0;
lastDsp=0;
connectionNr=1;

if nargin==1
    firstDsp=varargin{1};
    lastDsp=varargin{1};
elseif nargin==2
    firstDsp=varargin{1};
    lastDsp=varargin{2};
elseif nargin==3
    firstDsp=varargin{1};
    lastDsp=varargin{2};
    connectionNr=varargin{3};
end;

% device detection
dsp_ver=aoGetBoardSN(firstDsp,lastDsp)>=1000;
if     min(dsp_ver) == 0 && max(dsp_ver) == 0
   device = 0;
elseif min(dsp_ver) == 1 && max(dsp_ver) == 1
   device = 1;
else
   error('This function doesn''t support hybrid DSP 8ch & 16ch');
end

if device==0
   % parametri standard
   DIAGNOSTIC_ADC_GAIN=1.20000e-3;
   DIAGNOSTIC_ADC_OFFSET=-1463e-3;
   ADC_spi_offset=ones(4*(lastDsp-firstDsp+1),1)*DIAGNOSTIC_ADC_OFFSET;
   ADC_spi_gain=ones(4*(lastDsp-firstDsp+1),1)*DIAGNOSTIC_ADC_GAIN;
   % sovrascivo la calibrazione standard con i parametri letti dalla flash della scheda
   % commentata questa parte perche' le schede per il burnin test non sono ancora calibrate
%    for i=firstDsp:2:lastDsp
%       ADC_spi_offset(i/2+1:i/2+8)=mgp_op_rd_sram(i,i,8,'0x18174',connectionNr,'single');
%       ADC_spi_gain(i/2+1:i/2+8)=mgp_op_rd_sram(i,i,8,'0x1817C',connectionNr,'single');
%    end
   coils_curr=double(bitand(mgp_op_rd_sram(firstDsp,lastDsp,4,'0x181ab',connectionNr,'uint16'),hex2dec('0FFF')));
   coils_curr=coils_curr(1:4*(lastDsp-firstDsp+1));
   coils_curr=coils_curr.*ADC_spi_gain+ADC_spi_offset;
else
   % parametri standard
   DIAGNOSTIC_ADC_CURR_OFFSET=-1.25;
   DIAGNOSTIC_ADC_CURR_GAIN=0.61035e-3;
   DIAGNOSTIC_ADC_VOLT_OFFSET=-5.;
   DIAGNOSTIC_ADC_VOLT_GAIN=2.4414e-3;
   ADC_spi_curr_offset=ones(8*(lastDsp-firstDsp+1),1)*DIAGNOSTIC_ADC_CURR_OFFSET;
   ADC_spi_curr_gain=ones(8*(lastDsp-firstDsp+1),1)*DIAGNOSTIC_ADC_CURR_GAIN;
   ADC_spi_volt_offset=ones(8*(lastDsp-firstDsp+1),1)*DIAGNOSTIC_ADC_VOLT_OFFSET;
   ADC_spi_volt_gain=ones(8*(lastDsp-firstDsp+1),1)*DIAGNOSTIC_ADC_VOLT_GAIN;
%    % sovrascivo la calibrazione standard con i parametri letti dalla flash della scheda
%    for i=firstDsp:2:lastDsp
%        ADC_spi_curr_offset(i/2+1:i/2+16)=mgp_op_rd_sram(i,i,16,'0x18174',connectionNr,'single');
%        ADC_spi_curr_gain(i/2+1:i/2+16)=mgp_op_rd_sram(i,i,16,'0x18184',connectionNr,'single');
%        ADC_spi_volt_offset(i/2+1:i/2+16)=mgp_op_rd_sram(i,i,16,'0x18194',connectionNr,'single');
%        ADC_spi_volt_gain(i/2+1:i/2+16)=mgp_op_rd_sram(i,i,16,'0x181A4',connectionNr,'single');
%    end
%    % raw data in bit
%    ADC_spi_curr_offset=ones(16*(lastDsp-firstDsp+1),1)*0;
%    ADC_spi_curr_gain=ones(16*(lastDsp-firstDsp+1),1)*1;
%    ADC_spi_volt_offset=ones(16*(lastDsp-firstDsp+1),1)*0;
%    ADC_spi_volt_gain=ones(16*(lastDsp-firstDsp+1),1)*1;

   coils_c=double(bitand(mgp_op_rd_sram(firstDsp,lastDsp,8,'0x181fd',connectionNr,'uint16'),hex2dec('0FFF')));
   coils_v=double(bitand(mgp_op_rd_sram(firstDsp,lastDsp,8,'0x18205',connectionNr,'uint16'),hex2dec('0FFF')));
   coils_curr=[coils_c(1:8*(lastDsp-firstDsp+1)) coils_v(1:8*(lastDsp-firstDsp+1))];
   coils_curr=coils_curr.*[ADC_spi_curr_gain ADC_spi_volt_gain]+[ADC_spi_curr_offset ADC_spi_volt_offset];
end
