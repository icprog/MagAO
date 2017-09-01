function [ff_matrix cmd2slaved_matrix]=slaveActs(sys_data, acts2Slave)
% [ff_slaved_matrix cmd2slaved_matrix]=ffRemoveActs(sys_data, acts2Remove)
% Slaves actuators 
%
% Parameters: 
% sys_data = system descriptor. The following fields are used:
%  - sys_data.ff_matrix
%  - sys_data.mirrNActAllOK (it must be = length(sys_data.ff_matrix)
%  - sys_data.mirrMapAllOK
% acts2Slave = list of actuators to be slaved. 'Mirror' numbering
%
% Returns:
%   ff_matrix         = 'reduced' feedforward matrix used to compute the forces 
%                       of the non-slaved (aka 'master') actuators. The matrix 
%                       is ordered according to mirror numbering in ascending order. 
%                       Size is nActMaster x nActMaster
%   cmd2slaved_matrix = matrix to compute the slaved commands from the 'master' 
%                       commands. Size is nActSlaved x nActMaster.
%                       The matrix is ordered according to mirror numbering in 
%                       ascending order, both in rows and columns.
% 
%
% Theoretical background
%
% For the sake of simplicity, let's reorder the FFWD matrix so that the
% slaved actuators are all in the bottom part of the matrix.
% We aim to compute the slaved actuators commands xS on base of commmands
% of the other actuators xC, imposing zero force on the 'slaved' actuators.
% { F } = [ K ] * { xC }
% { 0 }   [   ]   { xS }
%
% Decomposing K in blocks:
%
% { F } = [ Ka Kb ] * { xC }
% { 0 }   [ Kc Kd ]   { xS }
% 
% Solving the lower part of the equation:
%
% [ 0 ] = [ Kc ] * {xC} + [ Kd ] * { xS }
% 
% Therefore:
% 
% { xS } = - [ Kd ]^-1 * [ Kc ] * { xC }
% so cmd2slaved_matrix = [ Kd ]^-1 * [ Kc ]
%
% So we can compute the modified (reduced) feedforward matrix by computing
% the slaved commands corresponding to a unity command of the 'master' actuators:
% { xS1 } = - [ Kd ]^-1 * [ Kc ] * { I }
%
% [ Kr ] = [ Ka Kb ] * { I   }
%                      { xS1 }
%                       
%
% Author: R.Biasi
%
% Copyright 2014-2018 Microgate s.r.l.
% $Revision 0.1 $ $Date: 2014-12-17

if length(sys_data.ff_matrix)~=sys_data.mirrNActAllOK
   error('Invalid FFWD matrix size!!!');
end
acts2Slave=sort(acts2Slave);
acts2RemoveIdx=find(ismember(sys_data.mirrMapAllOK,acts2Slave));
actsStayThereIdx=setdiff(1:sys_data.mirrNActAllOK,acts2RemoveIdx);
lenStay=length(actsStayThereIdx);

ff_matrix=swapRowCol(sys_data.ff_matrix,[actsStayThereIdx acts2RemoveIdx]);
% computes the correction matrix
cmd2slaved_matrix=-inv(ff_matrix(lenStay+1:end,lenStay+1:end))*ff_matrix(lenStay+1:end,1:lenStay);
ff_matrix=ff_matrix(1:lenStay,:)*[eye(lenStay);cmd2slaved_matrix];

return;


% test code
% loads ffwd matrix K (actually any decently conditioned matrix would work)
sys_data.ff_matrix=K;
sys_data.mirrNActAllOK=length(sys_data.ff_matrix);
sys_data.mirrMapAllOK=[0:sys_data.mirrNActAllOK-1];

% defines the actuators to slave and the master actuators
acts2Slave=[4 10 22 34];
actsMaster=setdiff(sys_data.mirrMapAllOK,acts2Slave);

% computes the reduced matrix
[ff_matrix cmd2slaved_matrix]=slaveActs(sys_data, acts2Slave);
% generates a random command patterna
cmdFull=rand(size(sys_data.ff_matrix,1),1);
% extracts the master actuators
cmdMaster=cmdFull(actsMaster+1);
% computes the slaved commands from the master actuators
cmdSlaved=cmd2slaved_matrix*cmdMaster;
% creates the full command with the slaved
cmdFullWSlaved=cmdFull;
cmdFullWSlaved(acts2Slave+1)=cmdSlaved;

% final check
forcesFullWSlaved=sys_data.ff_matrix*cmdFullWSlaved;
fprintf('Rms of ''slaved'' actuators forces: %e\n',rms(forcesFullWSlaved(acts2Slave+1)));
fprintf('Rms of ''master'' actuators forces: %e\n',rms(forcesFullWSlaved(actsMaster+1)));

