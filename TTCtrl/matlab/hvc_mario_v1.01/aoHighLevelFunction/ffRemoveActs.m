function [ff_matrix ff_u ff_w ff_v]=ffRemoveActs(sys_data, acts2Remove)
% [ff_matrix ff_u ff_w ff_v]=ffRemoveActs(sys_data, acts2Remove)
% Recomputes the feedforward matrix removing non-working actuators
%
% Parameters: 
% sys_data = system descriptor. The following fields are used:
%  - sys_data.ff_matrix
%  - sys_data.mirrNActAllOK (it must be = length(sys_data.ff_matrix)
%  - sys_data.mirrMapAllOK
% acts2Remove = list of actuators to be removed. 'Mirror' numbering
% Returns:
%   ff_matrix = 'reduced' feedforward matrix
%   ff_u = SVD decomposition left matrix
%   ff_v = SVD decomposition right matrix
%   ff_w = modal stiffness vector
%
% Theoretical background
%
% For the sake of simplicity, let's reorder the FFWD matrix so that the
% non-used actuators are all in the bottom part of the matrix.
% Assume also to impose a set of unit-displacements (single pistons) on all
% 'good' actuators. We impose zero force on the 'bad' actuators, therefore
% the displacement on these actuatots is unknown:
% [ F ] = [ K ] * [ I ]
% [ 0 ]   [   ]   [ x ]
%
% Decomposing K in blocks:
%
% [ F ] = [ Ka Kb ] * [ I ]
% [ 0 ]   [ Kc Kd ]   [ x ]
% 
% Solving the lower part of the equation:
%
% [ 0 ] = [ Kc ] + [ Kd ] * [ x ]
% 
% Therefore:
% 
% x = - [ Kd ]^-1 * [ Kc ]
%
% So we can compute the modified (reduced) feedforward matrix:
%
% [ Kr ] = [ Ka Kb ] * [ I ]
%                      [ x ]
%                       
%
% Author: R.Biasi
%
% Copyright 2004-2010 Microgate s.r.l.
% $Revision 0.1 $ $Date: 07/02/2010

if length(sys_data.ff_matrix)~=sys_data.mirrNActAllOK
   error('Invalid FFWD matrix size!!!');
end

acts2RemoveIdx=find(ismember(sys_data.mirrMapAllOK,acts2Remove));
actsStayThereIdx=setdiff(1:sys_data.mirrNActAllOK,acts2RemoveIdx);
len2Remove=length(acts2RemoveIdx);
lenStay=length(actsStayThereIdx);

ff_matrix=swapRowCol(sys_data.ff_matrix,[actsStayThereIdx acts2RemoveIdx]);
% computes the correction matrix
x=-inv(ff_matrix(lenStay+1:end,lenStay+1:end))*ff_matrix(lenStay+1:end,1:lenStay);
ff_matrix=ff_matrix(1:lenStay,:)*[eye(lenStay);x];

% SVD
[ff_u,ff_w,ff_v]=svd(ff_matrix);

% re-orders the svd decomposition (really necessary? most likely ML makes it automatically...)
[a idx]=sort(diag(ff_w));
ff_u=ff_u(:,idx);
ff_w=ff_w(:,idx);
ff_w=ff_w(idx,:);
ff_v=ff_v(:,idx);
return;


% test code
% loads ffwd matrix K (actually any decently conditioned matrix would work
load('S:\Progetti\Engineering\New AO\GMT\Matlab\PhaseScreen\AnalisiMarco\Ultimo\K_review.mat');
sys_data.ff_matrix=K;
sys_data.mirrNActAllOK=length(sys_data.ff_matrix);
sys_data.mirrMapAllOK=[sys_data.mirrNActAllOK-1:-1:0];
acts2Remove=[10 34 22 556 312 4];
% now generates a force pattern forcing 0 force on the 'OFF' actuators
acts2RemoveIdx=find(ismember(sys_data.mirrMapAllOK,acts2Remove));
actsStayThereIdx=setdiff(1:sys_data.mirrNActAllOK,acts2RemoveIdx);
x=randn(sys_data.mirrNActAllOK,1);
F=K*x;
% computes the reduced matrix
[ff_matrix ff_u ff_w ff_v]=ffRemoveActs(sys_data, acts2Remove);
% extracts the good actuators and computes the forces on the 'good' actuators
Fr=ff_matrix*x(actsStayThereIdx);
% reconstruct the full force pattern, setting zero on 'bad' actuators
F_(actsStayThereIdx)=Fr;
F_(acts2RemoveIdx)=0;
F_=F_(:);
% applies the reconstructed pattern to the full stiuffness matrix
x_=inv(K)*F_;
% shows that the positions are identical on all 'good' actuators
figure(1);
plot(0:sys_data.mirrNActAllOK-1,x,'b',0:sys_data.mirrNActAllOK-1,x_,'r',...
   acts2RemoveIdx-1,x_(acts2RemoveIdx),'g*'); title('Actutors positions');
legend('Full matrix','Reduced matrix','Removed actuators');
figure(2);
plot(0:sys_data.mirrNActAllOK-1,x_-x, acts2RemoveIdx-1,x_(acts2RemoveIdx)-x(acts2RemoveIdx),'g*'); 
title('Position difference');
