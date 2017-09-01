function mirrorCmdUpdate(command_vect,ff_force_vect,varargin)
%
% NAME:
%  mirrorCmdUpdate(command_vect,ff_force_vect,[dmSystem],[useFF],[delay],[useOVS])
%
% PURPOSE:
%	The routine performs the mirror command update for the deformable mirror systems
%
% USAGE:
%	mirrorCmdUpdate (command_vect,ff_force_vect,[dmSystem],[useFF])
%
% INPUT:
%  command_vect:  vector of command to apply
%  ff_force_vect: vector of ff force to apply
%  [dmSystem]:    select the deformable system (0=M4-DP, 1=D45) - default is 0 (M4-DP)
%  [useFF]:       enable the ff force (0=disable, 1=enable) - default is 0 (disable) 
%                 this options is used only for D45 system
%  [delay]:       delay in seconds before to apply the command
%  [useOVS]:      enable the mirror update using the OVS system (0=disable, 1=enable) - default is 0 (disable) 
%                 this options is used only for D45 system
%
% HISTORY
%  01/02/10 - Modified by Mario to expand for D45 system
%

sys_data=evalin('base','sys_data');

dmSystem=0;
useFF=0;
delay=0;
useOVS=0;

if nargin>=3
   dmSystem=varargin{1};
end
if nargin>=4
   useFF=varargin{2};
end
if nargin>=5
   delay=varargin{3};
end
if nargin>=6
   useOVS=varargin{4};
end

if dmSystem==0
   cmd_force_vect=typecast(ff_force_vect*65536,'uint32')+command_vect;
   while 1
      try
   %       m4_op_write(0,sys_data.nFins-1,length(cmd_force_vect),aoGetAddress('new_cmd_force'),cmd_force_vect,1,'uint32',0,1);
         m4_op_write(0,sys_data.nFins-1,length(cmd_force_vect),aoGetAddress('new_cmd_force'),cmd_force_vect,1,'uint32',4,0);
         res_cmd=m4_op_read(0,sys_data.nFins-1,length(cmd_force_vect),aoGetAddress('new_cmd_force'),1,'uint32',4);
         if max(res_cmd~=repmat(cmd_force_vect,sys_data.nFins,1))
            disp('invalid command update');
         else
            pause(.01);
            m4_op_write(0,sys_data.nFins-1,1,aoGetAddress('new_cmd_force')+length(cmd_force_vect)*4,1,1,'uint32',0,1);
         end
         break;
      catch
         M4FC_rst_interface_status;
      end
   end
elseif dmSystem==1
   if useFF==1 && useOVS==1
      error('The FF and OVS options are mutually exclusive');
   end
   if useOVS==1 && ~aoRead('_bm2_enableLoop',sys_data.BcuID,1:sys_data.nCrates)
      error('The OVS should be enabled to use this option');
   end
   if useOVS==0
      full_command_vect=zeros(sys_data.flnAct,1);
      full_command_vect(1:sys_data.nAct)=command_vect;
      aoWrite('_dm2_commandVect',full_command_vect,0:sys_data.nDsp-1);
      aoWrite('_dm2_runningAcc',1,0:sys_data.nDsp-1);
      if useFF==0
         aoWrite('_dm2_FFWholeCurr',ff_force_vect,0:sys_data.nAct-1,1,sys_data.mirrorActMap);
         aoWrite('_dm2_updateCmd',delay*sys_data.cntFreq+1,0:sys_data.nDsp-1);
      else
         aoWrite('_dm2_FFwdCurr',ff_force_vect,0:sys_data.nAct-1,1,sys_data.mirrorActMap);
         aoWrite('_dm2_updateFF',delay*sys_data.cntFreq+1,0:sys_data.nDsp-1);
      end
   else
      aoWrite('_bm2_singleCmdVect',command_vect,sys_data.BcuID,1:sys_data.nCrates);
      aoWrite('_bm2_updateSingleCmd',delay*sys_data.ICCFreq+1,sys_data.BcuID,1:sys_data.nCrates);
   end
else
   error('Invalid dmSystem option');
end
