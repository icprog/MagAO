function data_struct=aoGetDSPFullStatus(varargin)
% data_struct=aoGetDSPStatus([firstDsp],[lastDsp],[connectionNr])
% Gests full DSP status and returns a struct with: serial number, 4 temperatures and 8 coil currents for each dsp boards.
% Author(s): Mario: strating from aoGetDSPStatus function, included the coil driver currents
%
% Copyright 2004-2008 Microgate s.r.l.
% $Revision 0.1 $ $Date: 22/11/2005
% $Revision 0.2 $ $Date: 24/04/2007
% $Revision 0.3 $ $Date: 25/11/2008 modified to extend the routine for the 16ch DSP board
% $Revision 0.4 $ $Date: 22/02/2010 call to aoGetBoardSN was missing the connectionNr parameter (RBI)

firstDsp=0;
lastDsp=0;
connectionNr=1;

if nargin==1
    firstDsp=floor(varargin{1}/2)*2;
    lastDsp=floor(varargin{1}/2)*2;
elseif nargin==2
    firstDsp=floor(varargin{1}/2)*2;
    lastDsp=floor(varargin{2}/2)*2;
elseif nargin==3
    firstDsp=floor(varargin{1}/2)*2;
    lastDsp=floor(varargin{2}/2)*2;
    connectionNr=varargin{3};
end;

% device detection
% device detection
dsp_ver=aoGetBoardSN(firstDsp,lastDsp,connectionNr)>=1000;
if     min(dsp_ver) == 0 && max(dsp_ver) == 0
   device = 0;
elseif min(dsp_ver) == 1 && max(dsp_ver) == 1
   device = 1;
else
   error('This function doesn''t support hybrid DSP 8ch & 16ch');
end

idx=1;
data_struct=[];
if device==0
   for i=firstDsp:2:lastDsp

      a=mgp_op_rd_sram(i,i,1,'0x18173',connectionNr,'uint16');
      data_struct(idx).serial_number=a(1);

      a=mgp_op_rd_sram(i,i,2,'0x181a8',connectionNr,'int16');
      data_struct(idx).stratix_temp=a(1)*0.0078125;
      data_struct(idx).power_temp=a(2)*0.0078125;
      data_struct(idx).dsps_temp=a(3)*0.0078125;
      data_struct(idx).driver_temp=a(4)*0.0078125;
      a=mgp_op_rd_sram(i,i,1,'0x181aa',connectionNr);
      data_struct(idx).driver_status=a;
      if i ~= 248, % shall not be read on Accelerometers boards
         data_struct(idx).coil_current=aoGetDSPSpiCurrents(i,i+1,connectionNr);
      end;

      idx=idx+1;
   end
else
   for i=firstDsp:2:lastDsp

      a=mgp_op_rd_sram(i,i,1,'0x18173',connectionNr,'uint16');
      data_struct(idx).serial_number=a(1);

      a=mgp_op_rd_sram(i,i,3,'0x181f9',connectionNr,'int16')*0.0078125;
      data_struct(idx).stratix_temp=a(1);
      data_struct(idx).power_temp=a(2);
      data_struct(idx).dsps_temp=a(3);
      data_struct(idx).driver_temp0=a(4);
      data_struct(idx).driver_temp1=a(5);
      a=mgp_op_rd_sram(i,i,1,'0x181fc',connectionNr);
      data_struct(idx).driver_status=a;

      a=mgp_op_rd_sram(i,i,1,'0x18174',connectionNr);
      data_struct(idx).wesp_values=a;

      coils_stat=aoGetDSPSpiCurrents(i,i+1,connectionNr);

      data_struct(idx).coil_current=coils_stat(1:16);
      data_struct(idx).coil_voltage=coils_stat(17:32);
      idx=idx+1;
   end
end