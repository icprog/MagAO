function methodInit()
% this function initialiazes the optimizer
% it is supposed to set the following values:
%
% p initial value for the optimization parameters
% methodParameter
% OPTIONS 

% Author(s): D. Pescoller
%
% Copyright 2006-2010 Microgate s.r.l.
% $Revision 0.1 $ $Date: 30/08/2007

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%%%%%%%%%%%%%% EDIT HERE 1/1 %%%%%%%%%%%%%
ku=0.1;pu=4.4e-4;
pid_K=0.8*ku;
pid_I=2*0.6*ku/pu;
pid_D=0.6*ku*pu/8;
pid_N=2*pi*10e3;

methodParameter.Kp_0=pid_K;
methodParameter.Ki_0=pid_I;
methodParameter.Kd_0=pid_D;
methodParameter.N_0=pid_N;
methodParameter.criteria=4;  % this is the criteria used for the cost function: see method STEP 5)   
methodParameter.fig=1;  % figure number where to report results   

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% in this section it is possible to choose which parameters we want to
% optimize. By changing the XX_EVAL, if it is initialized to a p(x) it
% will be optimized, if it is initialized to a XX_0 it will not, the starting 
% value is always used. Take also a look to method STEP 1).
% Note that p must contain just the parameters which will be optimized
%
% EXAMPLE 1: if we want to optimize just the proportional part the 
% section below would be:
% 
% methodParameter.Kp_EVAL='p(1)';   
% methodParameter.Ki_EVAL='methodParameter.Ki_0';
% methodParameter.Kd_EVAL='methodParameter.Kd_0';
% methodParameter.N_EVAL='methodParameter.N_0';
% p=[
%     methodParameter.Kp_0
%    ];
% 
% EXAMPLE 2: if we want to optimize just the proportional part the 
% section below would be:
%
% methodParameter.Kp_EVAL='methodParameter.Kp_0';   
% methodParameter.Ki_EVAL='p(1)';
% methodParameter.Kd_EVAL='p(2)';
% methodParameter.N_EVAL='methodParameter.N_0';
% p=[
%     methodParameter.Ki_0
%     methodParameter.Kd_0
%    ];

% methodParameter.Kp_EVAL='p(1)';   
% methodParameter.Ki_EVAL='p(2)';
% methodParameter.Kd_EVAL='p(3)';
% methodParameter.N_EVAL='p(4)';
% 
% p=[
%     methodParameter.Kp_0
%     methodParameter.Ki_0
%     methodParameter.Kd_0
%     methodParameter.N_0
%   ];

methodParameter.Kp_EVAL='p(1)';   
methodParameter.Ki_EVAL='methodParameter.Ki_0';
methodParameter.Kd_EVAL='p(2)';
methodParameter.N_EVAL='methodParameter.N_0';

p=[
    methodParameter.Kp_0
    methodParameter.Kd_0
  ];
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%


% OPTIONS=optimset('MaxFunEvals',10,'MaxIter',10,'Display','iter','LargeScale','on');
OPTIONS=optimset('MaxFunEvals',10,'MaxIter',10,'Display','iter');
clear method;
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%%%%%%%%%%%%%% STOP EDIT 1/1 %%%%%%%%%%%%%
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

%assign all initialization values to the optimize function
assignin('caller','OPTIONS',OPTIONS);
assignin('caller','p',p);
assignin('caller','methodParameter',methodParameter);
