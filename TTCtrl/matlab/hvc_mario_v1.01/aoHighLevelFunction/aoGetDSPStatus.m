function data_struct=aoGetDSPStatus(varargin)
% data_struct=aoGetDSPStatus([firstDsp] , [lastDsp] , [connection])
% Gests DSP status and returns a struct with 4 fields.
% Author(s): D. Pescoller
%
% Copyright 2004-2008 Microgate s.r.l.
% $Revision 0.1 $ $Date: 22/11/2005
% $Revision 0.2 $ $Date: 22/10/2007 multiple Crate connection (Mario)
% $Revision 0.3 $ $Date: 17/11/2008 modified to extend the routine for the 16ch DSP board
% $Revision 0.4 $ $Date: 22/02/2010 call to aoGetBoardSN was missing the connectionNr parameter; 
%                                   call to aoGetDSPSpiCurrents was missing the connectionNr parameter;
%                                   call to aoGetDSPSpiCurrents excluded for accelerometer boards (RBI)

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
dsp_ver=aoGetBoardSN(firstDsp,lastDsp,connectionNr)>=1000;
if     min(dsp_ver) == 0 && max(dsp_ver) == 0
   device = 0;
elseif min(dsp_ver) == 1 && max(dsp_ver) == 1
   device = 1;
else
   error('This function doesn''t support hybrid DSP 8ch & 16ch');
end

if device==0
   data_struct.temp=double(mgp_op_rd_sram(firstDsp,lastDsp,2,'0x181a8',connectionNr,'int16'))*0.0078125;
else
   a=double(mgp_op_rd_sram(firstDsp,lastDsp,3,'0x181f9',connectionNr,'int16'))*0.0078125;
	a=reshape(a,6,length(a)/6);
   a=a(1:5,:);
   data_struct.temp=a(:);
end

