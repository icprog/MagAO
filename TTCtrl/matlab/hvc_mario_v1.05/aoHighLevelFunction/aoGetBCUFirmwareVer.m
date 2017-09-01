function data_struct=aoGetBCUFirmwareVer(varargin)
% data_struct=aoGetBCUFirmwareVer([connection])
% Gests DSP status and returns a struct with 4 fields.
% Author(s): Mario
%
% Copyright 2004-2008 Microgate s.r.l.
% $Revision 0.1 $ $Date: 06/09/2007
% modified to extend to multi crate access (mario)
% 21/03/2012 correct a bug for the at multi crate access (mario)

connectionNr=1;

if nargin==1
    connectionNr=varargin{1};
end

data_struct.logic=uint32(bitand(mgp_op_rd_sram(255,255,1,'0x38002',connectionNr,'uint32'),65535));
data_struct.nios=uint32(mgp_op_rd_sram(255,255,1,'0x38001',connectionNr,'uint32'));
data_struct.logicHex=[dec2hex(bitand(uint32(data_struct.logic/256),255),2) '.' ...
                      dec2hex(bitand(uint32(data_struct.logic),255),2)];
% data_struct.niosHex=[dec2hex(bitand(uint32(data_struct.nios/16777216),255),2) '.' ...
%                      dec2hex(bitand(uint32(data_struct.nios/65536),255),2) '.' ...
%                      dec2hex(bitand(uint32(data_struct.nios),65535),4)];
data_struct.niosHex=[dec2hex(bitshift(bitand(uint32(data_struct.nios),255*256*65536),-24),2) '.' ...
                     dec2hex(bitshift(bitand(uint32(data_struct.nios),255*65536),-16),2) '.' ...
                     dec2hex(bitand(uint32(data_struct.nios),65535),4)];
