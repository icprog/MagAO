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
% 05/06/14 modificata la gestione del connectionNr, infatti tra LBT e VLT c'e' una differenza nel DSP_CONF_MAT, perche' la
%          la 2 seconda colonna (connectionNr) nel caso LBT parte da 1 mentre nel caso VLT parte da 0 perche' e' riferito a crate number
% 24/06/14 modified transport 'tcp' to 'hkl' e aggiunta la gestione mgp via 'tcp' per il DP

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
list=CONF_MAT(enableList,2:end);
list(:,2)=floor(list(:,2)/2)*2;
list=sortrows(list,2); % sorting in base alla scheda DSP
list=sortrows(list,1); % sorting in base al crate
[b,idx]=unique(list(:,1)*1000 + list(:,2));
start_idx=1;
for end_idx=idx'
   connectionNr=list(start_idx,1);
   if connectionNr>0
      [socket,trasport]=getSocket(connectionNr);
   else
      [socket,trasport]=getSocket(connectionNr+1);
   end
   switch lower(trasport)
      case 'udp'
         dspNr=list(start_idx,2);
         actList=list(start_idx:end_idx,5);
         if enable_disable==1
            coil_ena=sum(2.^(actList*2+1));
         else
            coil_ena=sum(2.^(actList*2));
         end
         mgp_op_reset_devices(dspNr,dspNr,[0,coil_ena],connectionNr);
      case 'hkl'
         dspNr=list(start_idx,2);
         actList=list(start_idx:end_idx,5);
         if enable_disable==1
            coil_ena=sum(2.^(actList*2+1));
         else
            coil_ena=sum(2.^(actList*2));
         end
         aoWrite('_rstub_resetCommand',[0 coil_ena 0],dspNr+connectionNr*26,1);
      case 'tcp'
         actList=list(start_idx:end_idx,5);
         bit_list=[];
         for ii=1:size(actList,1)
            bit_list=[bit_list {sprintf('drv_ena%02d',actList(ii))}];
         end
         if enable_disable==1
            [aa]=m4SetClearPicPio(bit_list,'set',connectionNr);
         else
            [aa]=m4SetClearPicPio(bit_list,'clear',connectionNr);
         end
         if ~strcmpi(aa,'MGP_CMD_SUCCESS')
            error('MGP/TCP command failed, the reply is %s',aa);
         end
      otherwise
         error('Invalid trasport socket type');
   end
   start_idx=end_idx+1;
end
