%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% test della time history sincrona                %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
buf_len=20000;

decFactor=0;

aoBufferArray(1).bufferName='buff_1';	                    % name of buffer, is not mandatory, just useful
aoBufferArray(1).triggerPointer=0;		                    % pointer of trigger
aoBufferArray(1).triggerDataType=3;		                    % trigger data type 1='single' 2='int32' 3='uint32'
aoBufferArray(1).triggerDsp=0;			                    % number of DSP 0 or 1
aoBufferArray(1).triggerMask=4294967295;                   % trigger mask (these bits are checked) normaly should be 0xFFFFFFFF
aoBufferArray(1).triggerValue=0;                           % trigger value
aoBufferArray(1).triggerCompare=0;		                    % trigger compare type: 0: none 1: "value<*pointer" 2: "value>*pointer" 3: "value=*pointer"
aoBufferArray(1).dspPointer=aoGetAddress('hvc_TT1_distance'); % pointer of read/write element
aoBufferArray(1).nrItem=2;	                                % size of single read/write operation normally=1
aoBufferArray(1).dataType=1;				                    % data type 1='single' 2='int32' 3='uint32'
aoBufferArray(1).dsp=0;					                    	  % which DSP 0 or 1
aoBufferArray(1).len=buf_len;		                          % length of buffer
aoBufferArray(1).decFactor=decFactor;                      % decimation factor
aoBufferArray(1).sdramPointerValue=0;	                    % pointer where to/from store/read data
aoBufferArray(1).direction=0;				                    % direction 0=read data 1=write data
aoBufferArray(1).circular=0;				                    % 0=linear 1=circular
aoBufferArray(1).bufferNumber=1;			                    % which buffer number 1 to 6 for each board
aoBufferArray(1).firstDsp=sys_data.hvc_board;              % first DSP where to write setup
aoBufferArray(1).lastDsp=sys_data.hvc_board;               % last DSP where to write setup
aoBufferArray(2).bufferName='buff_2';	                    % name of buffer, is not mandatory, just useful
aoBufferArray(2).triggerPointer=0;		                    % pointer of trigger
aoBufferArray(2).triggerDataType=3;		                    % trigger data type 1='single' 2='int32' 3='uint32'
aoBufferArray(2).triggerDsp=0;			                    % number of DSP 0 or 1
aoBufferArray(2).triggerMask=4294967295;                   % trigger mask (these bits are checked) normaly should be 0xFFFFFFFF
aoBufferArray(2).triggerValue=0;                           % trigger value
aoBufferArray(2).triggerCompare=0;		                    % trigger compare type: 0: none 1: "value<*pointer" 2: "value>*pointer" 3: "value=*pointer"
% aoBufferArray(2).dspPointer=aoGetAddress('hvc_TT1_pos_current'); % pointer of read/write element
aoBufferArray(2).dspPointer=aoGetAddress('hvc_TT1_float_DAC_value'); % pointer of read/write element
aoBufferArray(2).nrItem=2;	                                % size of single read/write operation normally=1
aoBufferArray(2).dataType=1;				                    % data type 1='single' 2='int32' 3='uint32'
aoBufferArray(2).dsp=0;					                    	  % which DSP 0 or 1
aoBufferArray(2).len=buf_len;		                          % length of buffer
aoBufferArray(2).decFactor=decFactor;                      % decimation factor
aoBufferArray(2).sdramPointerValue=aoBufferArray(1).sdramPointerValue+aoBufferArray(1).nrItem*aoBufferArray(1).len;	                    % pointer where to/from store/read data
aoBufferArray(2).direction=0;				                    % direction 0=read data 1=write data
aoBufferArray(2).circular=0;				                    % 0=linear 1=circular
aoBufferArray(2).bufferNumber=2;			                    % which buffer number 1 to 6 for each board
aoBufferArray(2).firstDsp=sys_data.hvc_board;              % first DSP where to write setup
aoBufferArray(2).lastDsp=sys_data.hvc_board;               % last DSP where to write setup
aoBufferArray(3).bufferName='buff_3';	                    % name of buffer, is not mandatory, just useful
aoBufferArray(3).triggerPointer=0;		                    % pointer of trigger
aoBufferArray(3).triggerDataType=3;		                    % trigger data type 1='single' 2='int32' 3='uint32'
aoBufferArray(3).triggerDsp=0;			                    % number of DSP 0 or 1
aoBufferArray(3).triggerMask=4294967295;                   % trigger mask (these bits are checked) normaly should be 0xFFFFFFFF
aoBufferArray(3).triggerValue=0;                           % trigger value
aoBufferArray(3).triggerCompare=0;		                    % trigger compare type: 0: none 1: "value<*pointer" 2: "value>*pointer" 3: "value=*pointer"
aoBufferArray(3).dspPointer=aoGetAddress('hvc_TT1_cmdhist_cmd'); % pointer of read/write element
% aoBufferArray(3).dspPointer=aoGetAddress('hvc_TT1_uint_DAC_value'); % pointer of read/write element
% aoBufferArray(3).dspPointer=aoGetAddress('hvc_TT1CmdHistAPtr'); % pointer of read/write element
aoBufferArray(3).nrItem=2;	                                % size of single read/write operation normally=1
aoBufferArray(3).dataType=1;				                    % data type 1='single' 2='int32' 3='uint32'
% aoBufferArray(3).dataType=3;				                    % data type 1='single' 2='int32' 3='uint32'
aoBufferArray(3).dsp=0;					                    	  % which DSP 0 or 1
aoBufferArray(3).len=buf_len;		                          % length of buffer
aoBufferArray(3).decFactor=decFactor;                      % decimation factor
aoBufferArray(3).sdramPointerValue=aoBufferArray(2).sdramPointerValue+aoBufferArray(2).nrItem*aoBufferArray(2).len;	                    % pointer where to/from store/read data
aoBufferArray(3).direction=0;				                    % direction 0=read data 1=write data
aoBufferArray(3).circular=0;				                    % 0=linear 1=circular
aoBufferArray(3).bufferNumber=3;			                    % which buffer number 1 to 6 for each board
aoBufferArray(3).firstDsp=sys_data.hvc_board;              % first DSP where to write setup
aoBufferArray(3).lastDsp=sys_data.hvc_board;               % last DSP where to write setup
aoBufferWriteSetup(aoBufferArray(1:3));

mgp_op_reset_devices(sys_data.hvc_board,sys_data.hvc_board,[16384 0]); % reset the frequency of time history A
mgp_op_reset_devices(sys_data.hvc_board,sys_data.hvc_board,[32768 0]); % reset the frequency of time history B
aoWrite('hvc_CmdHistEna',0,sys_data.hvc_board);
aoWrite('hvc_TT1CmdHistPtr',[0 CMD_HIST_BUFF_SIZE/4],sys_data.hvc_board);
aoWrite('hvc_TT1CmdHistGain',0, sys_data.hvc_board);
aoWrite('hvc_TT2CmdHistGain',0, sys_data.hvc_board);
aoWrite('hvc_TT1_step_ptr_preshaper_cmd',sys_data.pres_buff_size*ones(sys_data.nAct,1),0:sys_data.nAct-1,1,sys_data.mirrorActMap);
aoWrite('hvc_TT1_step_ptr_preshaper_curr',sys_data.pres_buff_size*ones(sys_data.nAct,1),0:sys_data.nAct-1,1,sys_data.mirrorActMap);

% 
%mgp_op_wrsame_ccdi(255,255,1,0,3);                            % enable ccd emulator
%pause(.5);
%

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% metodo secco                        %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% sys_data.syncA_freq=100;
% sys_data.syncB_freq=7.35; % questa e' la minima frequenza accettabile
% mgp_op_reset_devices(sys_data.hvc_board,sys_data.hvc_board,[16384 uint32(SYNC_BASE_FREQ/sys_data.syncA_freq)*65536]); % set the frequency of time history A
% mgp_op_reset_devices(sys_data.hvc_board,sys_data.hvc_board,[32768 uint32(SYNC_BASE_FREQ/sys_data.syncB_freq)*65536]); % set the frequency of time history B
% aoWrite('hvc_CmdHistAGain',1,sys_data.hvc_board);
% aoWrite('hvc_CmdHistBGain',1,sys_data.hvc_board);
% aoWrite('hvc_CmdHistEna',1,sys_data.hvc_board); % 1 = abilita la time hist A su TT1
% aoWrite('hvc_CmdHistEna',2,sys_data.hvc_board); % 2 = abilita la time hist B su TT1
%                                % 4 = abilita la time hist A su TT2
%                                % 8 = abilita la time hist B su TT2

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% metodo smooth di rampa in frequenza %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% aoWrite('hvc_CmdHistAGain',1,sys_data.hvc_board);
% aoWrite('hvc_CmdHistEna',1,sys_data.hvc_board); % 1 = abilita la time hist A su TT1
%                                % 2 = abilita la time hist B su TT1
%                                % 4 = abilita la time hist A su TT2
%                                % 8 = abilita la time hist B su TT2
% for i=8:5:200
%    mgp_op_reset_devices(sys_data.hvc_board,sys_data.hvc_board,[16384 uint32(SYNC_BASE_FREQ/i)*65536]);
%    fprintf('.');
%    pause(.1);
% end
% fprintf('\n');

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% metodo smooth di rampa in ampiezza time history A %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% range logaritmico di ampiezze su range lineare di frequenze
% case_table=[linspace(1000,30,5)' ((log(linspace(30,1000,5)).^2-log(30)^2)/(log(1000)^2-log(30)^2).*linspace(0,2.4,5)+0.05)'];
% case_sel=1;
% sys_data.syncA_freq=case_table(case_sel,1);
% sys_data.syncA_gain=case_table(case_sel,2);
% sys_data.cntFreq/2048 ~=  29 Hz
% sys_data.cntFreq/1024 ~=  59 Hz
% sys_data.cntFreq/512  ~= 117 Hz
% sys_data.cntFreq/256  ~= 234 Hz
% sys_data.cntFreq/128  ~= 469 Hz
% sys_data.cntFreq/64   ~= 937 Hz

%sys_data.syncA_freq=sys_data.cntFreq/128; % per fare una sinusoide precisa fare: sys_data.cntFreq/2^n finche' si arriva piu' o meno alla frequenza voluta
sys_data.syncA_freq=400; % per fare una sinusoide precisa fare: sys_data.cntFreq/2^n finche' si arriva piu' o meno alla frequenza voluta
sys_data.syncA_gain=0.15e-3; % ampiezza in rad della sinusoide
% SYNC_BASE_FREQ/sys_data.syncA_freq

mgp_op_reset_devices(sys_data.hvc_board,sys_data.hvc_board,[16384 uint32(SYNC_BASE_FREQ/sys_data.syncA_freq)*65536]); % set the frequency of time history A
pause(.1);
% aoBufferStart(1:3,sys_data.hvc_board);
aoWrite('hvc_CmdHistEna',1,sys_data.hvc_board);
for i=linspace(0,sys_data.syncA_gain,50)
   %aoWrite('hvc_CmdHistAGain',i,sys_data.hvc_board);
   aoWrite('hvc_TT1CmdHistGain',i, sys_data.hvc_board);
   fprintf('.');
   pause(.005);
end
fprintf('\n');


%  for i=1:5
%      a1 = aoRead('hvc_global_counter',sys_data.hvc_board);
%      a2 = aoRead('hvc_TT1CmdHistCnt',sys_data.hvc_board);
%      a3 = aoRead('hvc_TT1CmdHistStep',sys_data.hvc_board);
%      a4 = aoRead('hvc_TT1CmdHistPtr',sys_data.hvc_board);
%  
%      a5 = aoRead('hvc_TT2CmdHistCnt',sys_data.hvc_board);
%      a6 = aoRead('hvc_TT2CmdHistStep',sys_data.hvc_board);
%  
%      disp(['hvc_global_counter: ', num2str(a1)])
%      disp(['hvc_TT1CmdHistCnt : ', num2str(a2)])
%      disp(['hvc_TT1CmdHistStep: ', num2str(a3)])
%      disp(['hvc_TT1CmdHistPtr : ', num2str(a4)])
%      disp(['hvc_TT2CmdHistCnt : ', num2str(a5)])
%      disp(['hvc_TT2CmdHistStep: ', num2str(a6)])
%      disp('\n')
%      pause(1);
%  end
pause(5);
aoBufferStart(1:3,sys_data.hvc_board);
aoBufferWaitStop(1,sys_data.hvc_board);

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% smooth down della rampa in ampiezza time history A %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
for i=linspace(sys_data.syncA_gain,0,50)
   aoWrite('hvc_TT1CmdHistGain',i,sys_data.hvc_board);
   fprintf('.');
   pause(.005);
end
fprintf('\n');
aoWrite('hvc_CmdHistEna',0,sys_data.hvc_board);
% aoBufferWaitStop(1,sys_data.hvc_board);

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% metodo smooth di rampa in ampiezza time history B %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% % aoBufferStart(1:3,sys_data.hvc_board);
% 
% sys_data.syncB_freq=7.35; % questa e' la minima frequenza accettabile
% mgp_op_reset_devices(sys_data.hvc_board,sys_data.hvc_board,[32768 uint32(SYNC_BASE_FREQ/sys_data.syncB_freq)*65536]); % set the frequency of time history B
% aoWrite('hvc_CmdHistEna',3,sys_data.hvc_board);
% for i=0:.1e-1:.8
%    aoWrite('hvc_CmdHistBGain',i,sys_data.hvc_board);
%    fprintf('.');
%    pause(.01);
% end
% fprintf('\n');
% 
% aoBufferStart(1:3,sys_data.hvc_board);
% aoBufferWaitStop(1,sys_data.hvc_board);
% 
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% smooth down della rampa in ampiezza time history B %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% for i=.8:-.1e-1:0
%    aoWrite('hvc_CmdHistBGain',i,sys_data.hvc_board);
%    fprintf('.');
%    pause(.01);
% end
% fprintf('\n');
% aoWrite('hvc_CmdHistEna',0,sys_data.hvc_board);

xy_pos=aoBufferReadData(aoBufferArray(1));
xy_pos=reshape(xy_pos,2,buf_len);
xy_cmd=aoBufferReadData(aoBufferArray(3));
xy_cmd=reshape(xy_cmd,2,buf_len);
% max(xy_pos(1,:))
% min(xy_pos(1,:))
% max(xy_cmd(1,:))-aoGetAddress('hvc_TT1CmdHistA')
% min(xy_cmd(1,:))-aoGetAddress('hvc_TT1CmdHistA')
xy_curr=aoBufferReadData(aoBufferArray(2));
xy_curr=reshape(xy_curr,2,buf_len);

time_vect=((0:buf_len-1))/sys_data.cntFreq*(decFactor+1);

% figure(1)
% hold off
% plot(time_vect,xy_pos(1,:),'b')
% hold on
% plot(time_vect,xy_cmd(1,:),'r')
% title('Command history');
% xlabel('Time [s]');
% ylabel('Position [rad]');
% legend('position','command');
% figure(2)
% hold off
% % plot(time_vect,xy_pos(1,:)-xy_cmd(1,:),'r')
% plot(time_vect,xy_curr(1,:),'b');
% xlabel('Time [s]');
% ylabel('Voltage [V]');
% figure(3)
% hold off
% % plot(time_vect,xy_pos(1,:)-xy_cmd(1,:),'r')
% plot(-diff(xy_curr(1,:)),'-b');
% xlabel('Time [s]');
% ylabel('Delta Voltage [bit]');

figure(1)
hold off
plot(xy_cmd(1,1:1500),'b')
hold on
plot(xy_cmd(2,1:1500),'r')
title('Command history');
xlabel('Time [s]');
ylabel('Position [rad]');
legend('position','command');

figure(4)
hold off
plot(xy_pos(1,1:1500),'b')
hold on
plot(xy_pos(2,1:1500),'r')
title('Command history');
xlabel('Time [s]');
ylabel('Position [rad]');
legend('position','command');


figure(2)
hold off
plot(xy_curr(1,1:1500),'b');
xlabel('Time [s]');
ylabel('Voltage [V]');
figure(3)
hold off
%plot(xy_curr(1,1:1500)*sys_data.DACr2BitGain_x-mean(xy_curr(1,1:1500)),'-b');
plot(xy_curr(1,1:1500)*sys_data.DACr2BitGain_x,'-b');
xlabel('Time [s]');
ylabel('Delta Voltage [bit]');

