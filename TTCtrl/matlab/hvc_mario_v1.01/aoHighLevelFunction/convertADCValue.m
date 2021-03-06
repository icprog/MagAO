function res_data=convertADCValue(ADCValue,channel,varargin)
%
% NAME:
%  convertADCValue(ADCValue,channel,[dmSystem])
%
% PURPOSE:
%	This function converts an ADCvalue to a micron using the linearization LUT of the channel
%
% USAGE:
%	res_data = convertADCValue(ADCValue,channel)
%
% INPUT:
%  ADCValue:   adc value to convert (scalar or vector)
%  channel:    mirror channel of adc value/s (only mirror map is accepted)
%  [dmSystem]: select the deformable system (0=M4-DP, 1=D45) - default is 0 (M4-DP)
%
% OUTPUT:
%  res_data: converted data in meters
%
% KEYWORD:
%
% NOTE:
%
% HISTORY
%  01/02/10 - Modified by Mario to expand for D45 system
%

sys_data=evalin('base','sys_data');

dmSystem=0;
if nargin>=3
   dmSystem=varargin{1};
end

if dmSystem==0
   adc_low  = bitand(uint32(ADCValue),2^sys_data.LUT_gap-1);
   adc_high = bitshift(uint32(ADCValue),-sys_data.LUT_gap);

   if size(sys_data.linearizer_vector,2)==128
      base     = bitshift(sys_data.linearizer_vector(channel+1,adc_high+1),-16);
      coeff    = bitand(sys_data.linearizer_vector(channel+1,adc_high+1),65535);
   else
      base     = sys_data.linearizer_vector(channel+1,adc_high*2+2);
      coeff    = sys_data.linearizer_vector(channel+1,adc_high*2+1);
   end

   res_data = (double(abitshift(int32(coeff(:).*adc_low(:)),-sys_data.LUT_gap)+int32(base(:))))/2^sys_data.scale_input; % questo shift e' fatto fisso in logica == lut_gap
%    res_data = (double(abitshift(int32(coeff(:).*adc_low(:)),-16)+int32(base(:))))/2^sys_data.scale_input; % questo shift e' fatto fisso in logica == lut_gap
elseif dmSystem==1
   ADCValue=min(max(ADCValue,sys_data.linearizer.minADCValue(channel+1)),sys_data.linearizer.maxADCValue(channel+1));
   res_data=(sys_data.linearizer.A_coeff(channel+1)*ADCValue+sys_data.linearizer.B_coeff(channel+1))./(sys_data.linearizer.C_coeff(channel+1)*ADCValue+sys_data.linearizer.D_coeff(channel+1));
else
   error('Invalid dmSystem option');
end