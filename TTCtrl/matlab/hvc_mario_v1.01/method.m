function cost=method(p,methodParameter)
% this function  does the "experiments" and evaluates the result
% we can conceptually distinguish the following steps:
%
% STEP 1) construct environment based on the new set of parameters
% STEP 2) load the new environment
% STEP 3) execute the test/experiment
% STEP 4) read back data
% STEP 5) generate a cost value
% STEP 6) reporting                            

% Author(s): D. Pescoller
%
% Copyright 2006-2010 Microgate s.r.l.
% $Revision 0.1 $ $Date: 30/08/2007

persistent iteration ERRORSTART DISTANCESTART bias;

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%%%CONFIGURATION SECTION

%import some variables from workspace
CHANNEL=0;
sys_data=evalin('base','sys_data');
TRIGGER_LEVEL=evalin('base','TRIGGER_LEVEL');
FOLLOW_ERROR=evalin('base','FOLLOW_ERROR');
FOLLOW_ERROR_REDUCED=evalin('base','FOLLOW_ERROR_REDUCED');
STEP=4e-3;
MAX_EXCURSION=0e-3;
MIN_EXCURSION=0e-3;
BUF_LEN=4000;             %buffer length
MIRROR='TT1';

%%%END OF CONFIGRATION SECTION
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

ENABLE_MASK=['11',
             '21',
             '41',
             '81'];


fig=methodParameter.fig;
%step command generation
command=zeros(1,BUF_LEN);
command(floor(BUF_LEN/2):end)=STEP;

%we need to disable the trigger level during the step
triggerLevel=ones(1,BUF_LEN)*TRIGGER_LEVEL;
triggerLevel(floor(BUF_LEN/2)-100:floor(BUF_LEN/2)+1750)=200000;


if (isempty(bias))
    bias=0;
else
    bias=bias+STEP;
    if bias > MAX_EXCURSION
       bias = 0;
    end
end;

% enable oscillation detection
aoWrite(['hvc_',MIRROR,'ServoOscOnOff'],hex2dec(ENABLE_MASK(CHANNEL+1,:)),sys_data.hvc_board);

% actuator positioning to zero at first step
if isempty(iteration)
   % disable oscillation detection during move at this point the controlled is assumed to be stable!!! 
   aoWrite(['hvc_',MIRROR,'ServoOscOnOff'],hex2dec('0'),sys_data.hvc_board); 
   disp('smootly move actuator to zero position');
   % sets the preshapers to the slowest speed (16384/sys_data.cntFreq=0.366s with sys_data.cntFreq=60kHz)
   aoWrite(['hvc_',MIRROR,'_step_ptr_preshaper_cmd'],ones(4,1),0:3);
   aoWrite(['hvc_',MIRROR,'_pos_command'],zeros(4,1),0:3);
   pause(2+16384/sys_data.cntFreq);
   % enable oscillation detection  
   aoWrite(['hvc_',MIRROR,'ServoOscOnOff'],hex2dec(ENABLE_MASK(CHANNEL+1,:)),sys_data.hvc_board); 
end

% actuator positioning
currentPos=aoRead(['hvc_',MIRROR,'_pos_command'],0:3);
if abs(currentPos(CHANNEL+1)-bias) > STEP/8
   % disable oscillation detection during move at this point the controlled is assumed to be stable!!! 
   aoWrite(['hvc_',MIRROR,'ServoOscOnOff'],hex2dec('0'),sys_data.hvc_board); 
   bias=0;
   disp('smootly move actuator to test start position');
   % sets the preshapers to the slowest speed (16384/sys_data.cntFreq=0.366s with sys_data.cntFreq=60kHz)
   aoWrite(['hvc_',MIRROR,'_step_ptr_preshaper_cmd'],ones(4,1),0:3);
   aoWrite(['hvc_',MIRROR,'_pos_command'],zeros(4,1),0:3);
   pause(.5+16384/sys_data.cntFreq);
   % enable oscillation detection  
   aoWrite(['hvc_',MIRROR,'ServoOscOnOff'],hex2dec(ENABLE_MASK(CHANNEL+1,:)),sys_data.hvc_board); 
end
% remove preshaper...
aoWrite(['hvc_',MIRROR,'_step_ptr_preshaper_cmd'],16384*ones(4,1),0:3);

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% STEP 1) construct environment based on the new set of parameters
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
pid.p=eval(methodParameter.Kp_EVAL);
pid.i=eval(methodParameter.Ki_EVAL);
pid.d=eval(methodParameter.Kd_EVAL);
pid.N=eval(methodParameter.N_EVAL);
pid_sys=tf([pid.p+pid.d*pid.N  pid.p*pid.N+pid.i   pid.i*pid.N],[1 pid.N 0]);
pid_sysD=c2d(pid_sys,1/sys_data.cntFreq,'tustin');
[num,den]=tfdata(pid_sysD,'v');
coeff=sys_data.coeff;
coeff(CHANNEL+1,:)=[num(1),0,0,num(3),num(2),0,0,den(3),den(2)];
coeffVector=reshape(coeff,1,4*9);

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% STEP 2) load the new environment                                
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% prepare the buffer configuration
aoBufferTemplate.bufferName='';
aoBufferTemplate.triggerPointer=0;
aoBufferTemplate.triggerDataType=1;
aoBufferTemplate.triggerDsp=0;
aoBufferTemplate.triggerMask=0;
aoBufferTemplate.triggerValue=0;
aoBufferTemplate.triggerCompare=0;
aoBufferTemplate.dspPointer=0;
aoBufferTemplate.nrItem=1;
aoBufferTemplate.dataType=1;
aoBufferTemplate.dsp=0;
aoBufferTemplate.len=BUF_LEN;
aoBufferTemplate.decFactor=0;
aoBufferTemplate.sdramPointerValue=0;
aoBufferTemplate.direction=0;
aoBufferTemplate.circular=0;
aoBufferTemplate.bufferNumber=1;
aoBufferTemplate.firstDsp=sys_data.hvc_board;
aoBufferTemplate.lastDsp=sys_data.hvc_board;
%
aoBufferArray(1)=aoBufferTemplate;
if     CHANNEL==0
    aoBufferArray(1).dspPointer=aoGetAddress(['hvc_',MIRROR,'ServoOscSum02'])+2;
elseif CHANNEL==1
    aoBufferArray(1).dspPointer=aoGetAddress(['hvc_',MIRROR,'ServoOscSum13'])+2;
elseif CHANNEL==2
    aoBufferArray(1).dspPointer=aoGetAddress(['hvc_',MIRROR,'ServoOscSum02'])+3;
end    
aoBufferArray(1).dataType=3;


aoBufferArray(2)=aoBufferTemplate;
aoBufferArray(2).dspPointer=aoGetAddress(['hvc_',MIRROR,'_distance'])+CHANNEL;

aoBufferArray(3)=aoBufferTemplate;
aoBufferArray(3).dspPointer=aoGetAddress(['hvc_',MIRROR,'_pos_command'])+CHANNEL;
aoBufferArray(3).direction=1;

aoBufferArray(4)=aoBufferTemplate;
aoBufferArray(4).dspPointer=aoGetAddress(['hvc_',MIRROR,'_preshaped_cmd'])+CHANNEL;

aoBufferArray(5)=aoBufferTemplate;
aoBufferArray(5).dspPointer=aoGetAddress(['hvc_',MIRROR,'ServoOscTrigLevel'])+CHANNEL;
aoBufferArray(5).dataType=3;
aoBufferArray(5).direction=1;

aoBufferArray(6)=aoBufferTemplate;
aoBufferArray(6).dspPointer=aoGetAddress(['hvc_',MIRROR,'_float_DAC_value'])+CHANNEL;
aoBufferArray(6).dataType=1;

sdramPointerValue=0;
for i=1:6
    aoBufferArray(i).bufferNumber=i;
    aoBufferArray(i).sdramPointerValue=sdramPointerValue;
    sdramPointerValue=sdramPointerValue+BUF_LEN;
end

aoBufferWriteSetup(aoBufferArray(1:6));
aoBufferWriteData(aoBufferArray(3),command+bias);
aoBufferWriteData(aoBufferArray(5),triggerLevel);

%read current coeffs
stableCoeffVector=aoRead(['hvc_',MIRROR,'_pos_coeff'],sys_data.hvc_board);
disp('preparing simulation');

% load reduced follow error
aoWrite(['hvc_',MIRROR,'CLErrorTrigLevel'],ones(4,1)*FOLLOW_ERROR_REDUCED,sys_data.hvc_board);

% start of diagnostic buffers to see how the coefficients get loaded
aoBufferStart([1,2,4,6],sys_data.hvc_board,sys_data.hvc_board);
% load new servo coefficients
aoWrite(['hvc_',MIRROR,'_pos_coeff'],coeffVector,sys_data.hvc_board);
% wait buffer finish
aoBufferWaitStop(2,sys_data.hvc_board);
% load normal follow error
aoWrite(['hvc_',MIRROR,'CLErrorTrigLevel'],ones(4,1)*FOLLOW_ERROR,sys_data.hvc_board);

figure(fig);
subplot(2,2,1);
pseudoVarMGAOS=aoBufferReadData(aoBufferArray(1));
plot(pseudoVarMGAOS(:));
legend('MGAOS');
title('pseudovar during servo coeffs load');
subplot(2,2,2);
distance=aoBufferReadData(aoBufferArray(2));
plot(distance);
title('distance during servo coeffs load');

%check if servo was stable or oscillates
if bitget(aoRead(['hvc_',MIRROR,'ServoOscOnOff'],sys_data.hvc_board),9+CHANNEL) | ...
   bitget(aoRead(['hvc_',MIRROR,'ServoOscOnOff'],sys_data.hvc_board),13+CHANNEL)     
    %reload stable coeffs 
    aoWrite(['hvc_',MIRROR,'_pos_coeff'],stableCoeffVector,6)
    disp('**************');
    fprintf(1,'pid_K=%e;\n',pid.p);
    fprintf(1,'pid_I=%e;\n',pid.i);
    fprintf(1,'pid_D=%e;\n',pid.d);
    fprintf(1,'pid_N=%e;\n',pid.N);
    disp('coeffs are UNSTABLE');
    disp('**************');
    disp('servo oscillation detected during servo coeffs load');
    disp('press a Key to continue');
    dec2hex(aoRead(['hvc_',MIRROR,'ServoOscOnOff'],sys_data.hvc_board))
    pause();
    disp('restoring system in close loop');
    aoWrite(['hvc_',MIRROR,'_pos_command'],zeros(4,1),0:3);
    pause(16384/sys_data.cntFreq);
    bias=0;
    aoWrite(['hvc_',MIRROR,'ServoOscOnOff'],hex2dec('0'),sys_data.hvc_board);                 
    mirrorRip;
    oscDetectInit;
    aoWrite(['hvc_',MIRROR,'ServoOscOnOff'],hex2dec('0'),sys_data.hvc_board);                 
    mirrorSet;
    aoWrite(['hvc_',MIRROR,'ServoOscOnOff'],hex2dec(ENABLE_MASK(CHANNEL+1,:)),sys_data.hvc_board); 
    cost=inf;
    return;
end

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% STEP 3) execute the test/experiment                               
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%


% start of diagnostic buffers
aoBufferStart([1,2,3,4,5,6],sys_data.hvc_board,sys_data.hvc_board);
%wait buffer finish
aoBufferWaitStop(2,sys_data.hvc_board);


%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% STEP 4) read experiment result                            
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

distance=aoBufferReadData(aoBufferArray(2))-bias-45e-6;
preshaped_cmd=aoBufferReadData(aoBufferArray(4))-bias;
voltCmd=aoBufferReadData(aoBufferArray(1));
% voltCmd=bitand(voltCmd,65535);
% voltCmd=(voltCmd-32768)/-271.905;

err=distance-preshaped_cmd;
err=err(:);
t=[0:BUF_LEN-1]/sys_data.cntFreq;
tCost=t(:);
% tCost(floor(BUF_LEN/2):end)=0;
tCost(1:floor(BUF_LEN/2))=0;
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% STEP 5) generate a cost value                            
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
method=methodParameter.criteria;
switch method
 case 1
  mf=max(find(abs(err)>STEP*0.1))+max(find(abs(err)>STEP*0.5));       
 case 2 %ISE
  mf=sum(((err*1e8).*(err*1e8).*tCost));
 case 3 %ISTE
  mf=sum((tCost.*tCost).*((err).*(err)));
 case 4 %ISTTE
  mf=sum((tCost.*tCost).*(tCost.*tCost).*((err).*(err)))*1e12;
end;
cost=mf;
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% STEP 6) reporting                            
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
if (isempty(iteration))
 ERRORSTART=err;
 DISTANCESTART=distance;
 iteration=1;
end;


disp(['cost=',num2str(cost)]);
fprintf('bias =%f\n',bias);
disp('PARAMETERS')
disp('**************');
fprintf(1,'pid_K=%e;\n',pid.p);
fprintf(1,'pid_I=%e;\n',pid.i);
fprintf(1,'pid_D=%e;\n',pid.d);
fprintf(1,'pid_N=%e;\n',pid.N);
figure(fig);
subplot(2,2,4);
whitebg('white');
plot(t*1000,ERRORSTART,'r',...
     t*1000,err,...
     t*1000,DISTANCESTART,'m',...
     t*1000,distance);
% axis([248 258 -3e-6 3e-6])
legend('errStart','err','distanceStart','distance','Location','best');
grid;
title('Servo err (actual: blue start:red)');
xlabel('Time [ms]');

figure(fig);
subplot(2,2,3);
pseudoVarMGAOS=aoBufferReadData(aoBufferArray(1));
plot(pseudoVarMGAOS(:));
legend('MGAOS');
title('pseudovar during test');
%check if servo was stable or oscillates
if bitget(aoRead(['hvc_',MIRROR,'ServoOscOnOff'],sys_data.hvc_board),9+CHANNEL) | ...
   bitget(aoRead(['hvc_',MIRROR,'ServoOscOnOff'],sys_data.hvc_board),13+CHANNEL)     
    %reload stable coeffs 
    aoWrite(['hvc_',MIRROR,'_pos_coeff'],stableCoeffVector,6)
    disp('servo oscillation detected during test');
    disp('press a Key to continue');
    dec2hex(aoRead(['hvc_',MIRROR,'ServoOscOnOff'],sys_data.hvc_board))
    pause();
    disp('restoring system in close loop');
    aoWrite(['hvc_',MIRROR,'_pos_command'],zeros(4,1),0:3);
    pause(16384/sys_data.cntFreq);
    bias=0;
    aoWrite(['hvc_',MIRROR,'ServoOscOnOff'],hex2dec('0'),sys_data.hvc_board);                 
    mirrorRip;
    oscDetectInit;
    aoWrite(['hvc_',MIRROR,'ServoOscOnOff'],hex2dec('0'),sys_data.hvc_board);                 
    mirrorSet;
    aoWrite(['hvc_',MIRROR,'ServoOscOnOff'],hex2dec(ENABLE_MASK(CHANNEL+1,:)),sys_data.hvc_board); 
    cost=inf;
    disp('coeffs are UNSTABLE');
    disp('**************');
    return;
end
disp('coeffs are STABLE');
fprintf(1,'DIGITAL COEFFS:\n')
fprintf(1,'%f ',coeff(CHANNEL+1,:))
fprintf(1,'\n')
disp('***********************************************');

figure(2);
cc=aoBufferReadData(aoBufferArray(6));
plot(cc);
