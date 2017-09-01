function sys_stat=aoGetSysStatus(dmSystem,act_list,varargin)
% function sys_stat=aoGetSysStatus(dmSystem,act_list,varargin)
%
% Simple wrapper function to the system dedicated function getting generic MG device status 
%
% Parameters:
%  dmSystem: indicates which system is in use, valid data are 'DSM', 'M4-DP'
%  varargin: variable arguments according to the wrapped function
%
% Returns:
%  none
%
% Requires:

% Copyright 2012-2020 Microgate s.r.l.
% Author: Mario
% $Revision 0.1 $ $Date: 2012-11-27

if     isnumeric(dmSystem) && dmSystem==0 % DP (vecchio)
   error('No wrapped function is available');
elseif isnumeric(dmSystem) && dmSystem==1 % D45
   error('No wrapped function is available');
elseif ischar(dmSystem) && strcmpi(dmSystem,'DSM')   % DSM system
   sys_stat=dsmGetSysStatus(act_list,varargin{1:end});
elseif ischar(dmSystem) && strcmpi(dmSystem,'M4-DP') % new DP system
   if isempty(varargin)
      sys_stat=m4GetSysStatus(act_list);
   else
      sys_stat=m4GetSysStatus(act_list,varargin{1:end});
   end
else
   error('Unknown device selected');
end

return
