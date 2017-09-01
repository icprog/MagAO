function data_struct=aoGetStatus(dmSystem,varargin)
% data_struct=aoGetStatus([connectionNr],[raw_data])
% 
% Simple wrapper function to the system dedicated function getting generic MG device status
%
%  dmSystem: indicates which system is in use, valid data are 'DSM', 'M4-DP'
%  varargin: variable arguments according to the wrapped function
%
% Author(s): Mario
%
% Copyright 2004-2008 Microgate s.r.l.
% $Revision 0.1 $ $Date: 30/05/2014 - first release

if     isnumeric(dmSystem) && dmSystem==0 % DP (vecchio)
   error('No wrapped function is available');
elseif isnumeric(dmSystem) && dmSystem==1 % D45
   error('No wrapped function is available');
elseif ischar(dmSystem) && strcmpi(dmSystem,'DSM')   % DSM system
   error('No wrapped function is available');
elseif ischar(dmSystem) && strcmpi(dmSystem,'M4-DP') % new DP system
   if isempty(varargin)
      data_struct=m4GetStatus();
   else
      data_struct=m4GetStatus(varargin{1:end});
   end
else
   error('Unknown device selected');
end

return
