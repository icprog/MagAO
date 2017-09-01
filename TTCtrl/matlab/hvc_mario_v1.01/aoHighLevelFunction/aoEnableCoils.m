function aoEnableCoils(enableList,varargin)
% controls the drives enable
% aoEnableCoils(enableList,[Ena/Dis],[dspConfCol])
% array is the list of which drives should be enabled
%
% enable_disable default = 1 enables - 0 disables the listed channels without changing the others channel
% dspConfMatCol  default = DSP_MY_INDEX
%
% This function enables or disables the actuator coils
%
% Author(s): Mario
%
% Copyright 2004-2008 Microgate s.r.l.
% 28/01/10 created by mario
% 29/09/10 modified to accept DSP_CONF_MAT indexes (of any column) being larger than numel(DSP_CONF_MAT)-1. By Roby - see Roby wrong


enable_disable=1;
dspConfMatCol=evalin('base','DSP_MY_INDEX');
if nargin>=2
   enable_disable=varargin{1};
end
if nargin>=3
   dspConfMatCol=varargin{2};
end

CONF_MAT=evalin('base','DSP_CONF_MAT');

% add value collumn
% inizio Roby wrong!!!
% enableList=enableList+1; 
% CONF_MAT=sortrows(CONF_MAT,dspConfMatCol);
% list=unique(CONF_MAT(enableList,2:6),'rows');
% list(:,2)=floor(list(:,2)/2)*2;
% [b,idx]=unique(list(:,2));
% fine roby wrong
% inizio roby good
[tmp,enableList]=intersect(CONF_MAT(:,dspConfMatCol),enableList);
list=CONF_MAT(enableList,2:6);
list(:,2)=floor(list(:,2)/2)*2;
list=sortrows(list,2); % sorting in base alla scheda DSP
[b,idx]=unique(list(:,2));
% fine roby good
start_idx=1;
for end_idx=idx'
   connectionNr=list(start_idx,1);
   dspNr=list(start_idx,2);
   actList=list(start_idx:end_idx,5);
   if enable_disable==1
      coil_ena=sum(2.^(actList*2+1));
   else
      coil_ena=sum(2.^(actList*2));
   end
   mgp_op_reset_devices(dspNr,dspNr,[0,coil_ena],connectionNr);
   start_idx=end_idx+1;
end
