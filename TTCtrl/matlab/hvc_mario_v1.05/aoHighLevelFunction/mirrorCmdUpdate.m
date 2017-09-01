function mirrorCmdUpdate(command_vect,ff_force_vect,varargin)
%
% NAME:
%  mirrorCmdUpdate(command_vect,ff_force_vect,[dmSystem],[useFF],[delay],[useOVS],[wait])
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
%  [dmSystem]:    select the deformable system (0=M4-DP old, 1=D45, 'M4-DP'=M4-DP new with TCP interface) - default is 0 (M4-DP old)
%  [useFF]:       enable the ff force (0=disable, 1=enable) - default is 0 (disable) 
%                 this options is used only for D45 system
%  [delay]:       delay in seconds before to apply the command
%  [useOVS]:      enable the mirror update using the OVS system (0=disable, 1=enable) - default is 0 (disable) 
%                 this options is used only for D45 system
%  [wait]:        if 'yes' the function waits the end of the command (mirror setting + accumulators) before exiting, default is 'no'
%
% HISTORY
%  01/02/10 - Modified by Mario to expand for D45 system
%  01/10/14 - Modified by Mario to expand for new DP system
%

sys_data=evalin('base','sys_data');

dmSystem=0;
useFF=0;
delay=0;
useOVS=0;
wait='no';

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
if nargin>=6
   wait=varargin{5};
end

if     isnumeric(dmSystem) && dmSystem==0 % DP (vecchio)
   aa=uint32(typecast(int16(command_vect),'uint16'));
   bb=uint32(typecast(int16(ff_force_vect),'uint16'));
   aa=bitor(aa,bitshift(bb,16));
   aoWrite('_cntub_NewCmdForce',aa,1:sys_data.nBricks);
   aoWrite('_cntub_SOFGenerate',0,sys_data.brickMaster);
elseif isnumeric(dmSystem) && dmSystem==1 % D45
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
elseif ischar(dmSystem) && strcmpi(dmSystem,'M4-DP') % DP nuovo con interfaccia TCP
   busyStat=aoRead('_cntub_SOFGenerate',[],1:sys_data.nBricks);
   if any(busyStat)
      error('Unable to send the command, the system is busy... reply data is: %s\n',dec2bin(busyStat,5));
   end
   a=aoGetVar('_cntub_NewCmdForce');
   if length(ff_force_vect) ~= length(command_vect)
      error('Different command length between commmand and force voctors');
   end
   cmd_force_vect=bitshift(typecast(int32(ff_force_vect),'uint32'),16) + bitand(typecast(int32(command_vect),'uint32'),65535);
   cmd_force_vect=cmd_force_vect(:);
   if     length(cmd_force_vect) < a.nrItem
      cmd_force_vect=[cmd_force_vect;zeros(a.nrItem-length(cmd_force_vect),1)];
   elseif length(cmd_force_vect) > a.nrItem
      error('Command and Force vectors are too long');
   end
   aoWrite('_cntub_NewCmdForce',cmd_force_vect,[],1:sys_data.nBricks);
   if delay==0
      % immediate start, to use when a single brick is used
      aoWrite('_cntub_SOFGenerate',1,[],1:sys_data.nBricks);
   else
      % triggered start, use it in multiple bricks configuration
      globCnt=aoRead('_cntub_GlobalCounter',[],1:sys_data.nBricks);
      trigVal=bitand(max(globCnt)+min(65536,round(sys_data.cntFreq*delay)),65535);
      aoWrite('_cntub_SOFDelay',trigVal,[],1:sys_data.nBricks);
      aoWrite('_cntub_SOFGenerate',2,[],1:sys_data.nBricks);
   end
   if strcmpi(wait,'yes') % attesa della fine del comando
      while any(aoRead('_cntub_SOFGenerate',0,1:sys_data.nBricks))
         pause(.02);
      end
   end
else
   error('Invalid dmSystem option');
end
