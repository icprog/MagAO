function data=m4_op_read(firstFin,lastFin,len,startAddress,varargin)
% m4_op_read(firstFin,lastFin,len,startAddress,[connectionNr],[dataType],[speedTrans])
% startAddress byte based according to Nios memory map
% len in dword 32 bit
% connectionNr: default is 1
% dataType: default is 'uint32'
% speedTrans: default is 0 (maximum speed)

% Author(s): Mario
%
% Copyright 2004-2008 Microgate s.r.l.

% get parameters
firstFin=getNumeric(firstFin);
lastFin=getNumeric(lastFin);
len=getNumeric(len);
startAddress=getNumeric(startAddress);

% check variable parameters
if     nargin==4
   connectionNr = 1;
   dataType     = 'uint32';
   speedTrans   = 0;
elseif nargin==5
   connectionNr = varargin{1};
   dataType     = 'uint32';
   speedTrans   = 0;
elseif nargin==6
   connectionNr = varargin{1};
   dataType     = varargin{2};
   speedTrans   = 0;
elseif nargin==7
   connectionNr = varargin{1};
   dataType     = varargin{2};
   speedTrans   = varargin{3};
else
    error('wrong number of parameters');
end;

% check for the first and last fin
num_fins=mgp_op_rd_m4fci(255,255,1,10,connectionNr);
if lastFin<firstFin
   error ('lastFin should be >= firstFin');
end
if lastFin>=num_fins
   error ('lastFin should be < number of fins');
end

% buf(1)  -- 0x00 rd     xxx = command pending, 000 = no command pending 
%         --      wr     000 = reset module
%         --             001 = break of frame
%         --             010 = start new frame
%         --             011 = start program download
%         --             100 = reset remote FPGA
%         --             101 = set remote FPGA
% buf(2)  -- 0x01 rd/wr  of normal command counter   (writing 0x1 reset the counter)
% buf(3)  -- 0x02 rd/wr  of brake command counter    (writing 0x1 reset the counter)
% buf(4)  -- 0x03 rd/wr  of frame error counter      (writing 0x1 reset the counter)
% buf(5)  -- 0x04 rd/wr  of fifo full error counter  (writing 0x1 reset the counter)
% buf(6)  -- 0x05 rd/wr  of header crc error counter (writing 0x1 reset the counter)
% buf(7)  -- 0x06 rd/wr  of data crc error counter
% buf(8)  -- 0x07 rd/wr  of last_fin | first_fin | trans. speed | command
% buf(9)  -- 0x08 rd/wr  of fl remote start addr   (in bytes - absolute value see NIOS memory map)
% buf(10) -- 0x09 rd/wr  of fl remote data length  (in dword)
% buf(11) -- 0x0A rdonly of fins number
% buf(12) -- 0x0B rd/wr  of local start addr       (in bytes - absolute value see NIOS memory map)
% buf(13) -- 0x0C rd/wr  of local data length      (in dword)
% buf(14) -- 0x0D rd/wr  of diagnostic start addr  (in bytes - absolute value see NIOS memory map)
% buf(15) -- 0x0E rd/wr  of diagnostic data length (in dword)
% buf(16) -- 0x0F not used

buf=zeros(16,1);
buf(8) = lastFin * hex2dec('01000000') + firstFin * hex2dec('00010000') + speedTrans * hex2dec('00000100') + 3;
buf(9) = startAddress;
buf(10)= len * 4;
buf(12)= hex2dec('08800000'); % local start addr * SDRAM *
buf(13)= len * (lastFin - firstFin + 1) * 4;

mgp_op_wr_m4fci(255,255,6,7,buf(8:13),connectionNr);
while 1
   try
      mgp_op_wr_m4fci(255,255,1,0,2,connectionNr);
      timeout=50;
      while timeout && bitand(mgp_op_rd_m4fci(255,255,1,0,connectionNr),7)
         pause(.01);
         timeout = timeout - 1;
      end
      if max(mgp_op_rd_m4fci(255,255,5,2,connectionNr))~=0
      %    M4FC_interface_status;
         error('error counters detected');
      end
      if ~timeout
      %    M4FC_interface_status;
         error('timeout error in read command');
      end
%       fprintf('^');      
      break
   catch
      mgp_op_wr_m4fci(255,255,6,1,ones(6,1));
%       disp('M4DP read communication error');
      fprintf('$');
   end
end

data=mgp_op_rd_sdram(255,255,buf(13)/4,bitand(buf(12),hex2dec('07FFFFFF'))/4,connectionNr,dataType);
