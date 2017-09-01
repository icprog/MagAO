function varargout = aoBufferStart(selection,varargin)
% aoBufferStart(selection,[firstDsp],[lastDsp],[connectionNr])
% selection=array of buffer numbers to start
% firstDsp (mgp) default = 0
% lastDsp (mgp) default = 0
% connectonNr default = 1

% Author(s): D. Pescoller
%
% Copyright 2004-2008 Microgate s.r.l.
% $Revision 0.1 $ $Date: 2004/08/09
% $Revision 0.2 $ $Date: 09/03/2007 - multiple Crate connection D. Pescoller

firstDsp=0;
lastDsp=0;
connectionNr=1;

if nargin==2
    firstDsp=varargin{1};
    lastDsp=varargin{1};
elseif nargin==3
    firstDsp=varargin{1};
    lastDsp=varargin{2};
elseif nargin==4
    firstDsp=varargin{1};
    lastDsp=varargin{2};
    connectionNr=varargin{3};
end;

mask(selection*2)=1;

maskString=num2str(mask(end:-1:1));
startData=bin2dec(strrep(maskString,' ',''));
mgp_op_wrsame_diagbuf(firstDsp,lastDsp,1,0,uint32(startData),connectionNr,'uint32');
