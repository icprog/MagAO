function address=aoGetAddress(varargin)
% address=aoGetVar([varName],[indexOffset])
% search variable which name is varName and return its full varStruct
% varName can also be the ao address decimal value or hex value ('0x')
% offset is used to navigate variables database
% for example:
% address=aoGetAddress('global_counter')
% address=aoGetAddress('0x80000')
% address=aoGetAddress(524288)
% address=aoGetAddress() in this case a var selectGui will be displayed

% Author(s): D. Pescoller
%
% Copyright 2004-2008 Microgate s.r.l.
% $Revision 0.1 $ $Date: 22/12/2004


varStruct=aoGetVar(varargin{:});
address=varStruct(1).memPointer;
