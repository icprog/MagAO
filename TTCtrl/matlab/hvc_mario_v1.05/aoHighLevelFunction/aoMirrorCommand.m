function aoMirrorCommand(dmSystem,cmdVect,varargin)
% function aoMirrorCommand(dmSystem,cmdVect,varargin)
%
% Simple wrapper function to the system dedicated function to send a single command to the selected system
%
% Parameters:
%  dmSystem: indicates which system is in use, valid data are 'DSM', 'M4-DP'
%  cmdVect:  mirror command vector, the length depends to the selected system
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
   dsmMirrorCommand(cmdVect,varargin);
elseif ischar(dmSystem) && strcmpi(dmSystem,'M4-DP') % new DP system
   m4MirrorCommand(cmdVect,varargin);
else
   error('Unknown device selected');
end

return
