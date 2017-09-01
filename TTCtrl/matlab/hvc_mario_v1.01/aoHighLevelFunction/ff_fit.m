function [ff_matrix ff_u ff_w ff_v ff_err]=ff_fit(sys_data, data, varargin)
% [ff_matrix ff_u ff_w ff_v]=ff_fit(sys_data, data,[act_w_ff])
% Computes the feedforward matrix on base of an experimental data set
%
% Parameters: 
% sys_data = system data structure. Following fields are used:
%   nAct
% data = acquired data usign ff_data_acquisition()
%   data are organized as vector of structures, containing the following fields:
%     * command = vector of commanded position (length: nAct)
%     * mean_position = vector of mean position at the end of the step (length: nAct)
%     * mean_tot_curr = vector of mean force at the end of the step (length: nAct)
%     * bias_current = vector of bias current (length: nAct)
%     * ff_current = vector of feed forward current (length: nAct)
%   Only the fields mean_position and mean_tot_curr are used by this function
%   The vector has nAct*2*nMeas+2 elements, organized as: 
%     initial status
%     command on act #0, measurement #0 (positive step)
%     command on act #0, measurement #0 (negative step)
%     command on act #0, measurement #1 (positive step)
%     command on act #0, measurement #1 (negative step)
%      ...
%     command on act #0, measurement #nMeas-1 (positive step)
%     command on act #0, measurement #nMeas-1 (negative step)
%     command on act #1, measurement #0 (positive step)
%     command on act #1, measurement #0 (negative step)
%      ...
%     command on act #nAct-1, measurement #nMeas-1 (positive step)
%     command on act #nAct-1, measurement #nMeas-1 (negative step)
%     final status
%   The first and last sub-vectors refer to the initial and final state
%
% act_w_ff = optional parameter. 
%   vector with the list of actuators actually used (i.e. in closed loop), 
%   according to mirror numbering.
%   If some actuators are not used, ff_matrix and ff_v are modified:
%     * on ff_matrix, null rows and columns corresponding to missing 
%                  actuators indexes are inserted 
%     * on ff_v matrix, null rows only corresponding to missing 
%                  actuators indexes are inserted 
% IMPORTANT NOTE: act_w_ff MUST be sorted in increasing order
% Returns:
%   ff_matrix = feedforward matrix. The matrix is automatically filled with 
%       zero rows and columns on non used actuators
%   ff_u = SVD decomposition left matrix
%   ff_v = SVD decomposition right matrix
%   ff_w = modal frequencies vector
%
% Author(s): M. Andrighettoni/R.Biasi
%
% Copyright 2004-2009 Microgate s.r.l.
% $Revision 0.1 $ $Date: 03/09/2009

act_w_ff = 0:sys_data.mirrMapAllOK-1;
if nargin >= 3,
   act_w_ff=varargin{1};
   if sort(act_w_ff) ~= act_w_ff,
       error('act_w_ff vector does not increase monotonically!');
   end;
end

% data reorganization: index for positive and negative movement of the shell
idx_pos = 2:2:length(data)-1;
idx_neg = 3:2:length(data)-1;

position_pos=[data(idx_pos).mean_position];
position_neg=[data(idx_neg).mean_position];
tot_curr_pos=[data(idx_pos).mean_tot_curr];
tot_curr_neg=[data(idx_neg).mean_tot_curr];

% absolute delta movement calcolus
delta_position=position_pos(act_w_ff+1,:)-position_neg(act_w_ff+1,:);
delta_tot_curr=tot_curr_pos(act_w_ff+1,:)-tot_curr_neg(act_w_ff+1,:);

% a=reshape((1:length(delta_position)),length(delta_position)/45,45);a=a(2:length(delta_position)/45-1,:);a=a(:);
% a=reshape((1:length(delta_position)),length(delta_position)/45,45)';a=a(:,3:7);a=a(:);
% delta_position=delta_position(:,a);
% delta_tot_curr=delta_tot_curr(:,a);

ff_matrix=delta_tot_curr*pinv(delta_position);
[ff_u,ff_w,ff_v]=svd(ff_matrix);

% riordino delle matrici dal piu' morbido al piu' rigido (non serve!!!)
[a idx]=sort(diag(ff_w));
ff_u=ff_u(:,idx);
ff_w=ff_w(:,idx);
ff_w=ff_w(idx,:);
ff_v=ff_v(:,idx);
ff_w=diag(ff_w);

% calcolo dell'errore residuo
ff_err=delta_tot_curr-ff_matrix*delta_position;
