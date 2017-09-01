%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% step response                                   %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
aoWrite('hvc_TT1_step_ptr_preshaper_cmd',round(sys_data.pres_buff_size/.01/sys_data.cntFreq)*ones(sys_data.nAct,1),0:sys_data.nAct-1,1,sys_data.mirrorActMap);
aoWrite('hvc_TT1_step_ptr_preshaper_curr',round(sys_data.pres_buff_size/.01/sys_data.cntFreq)*ones(sys_data.nAct,1),0:sys_data.nAct-1,1,sys_data.mirrorActMap);

buf_len=32768;

% lettura degli strain gauge
aoBufferArray(1).bufferName='buff_1';	                    % name of buffer, is not mandatory, just useful
aoBufferArray(1).triggerPointer=aoGetAddress('hvc_update_cmd')+3;		                    % pointer of trigger
aoBufferArray(1).triggerDataType=3;		                    % trigger data type 1='single' 2='int32' 3='uint32'
aoBufferArray(1).triggerDsp=0;			                    % number of DSP 0 or 1
aoBufferArray(1).triggerMask=4294967295;                   % trigger mask (these bits are checked) normaly should be 0xFFFFFFFF
aoBufferArray(1).triggerValue=500;                         % trigger value
aoBufferArray(1).triggerCompare=3;		                    % trigger compare type: 0: none 1: "value<*pointer" 2: "value>*pointer" 3: "value=*pointer"
aoBufferArray(1).dspPointer=aoGetAddress('hvc_TT1_distance'); % pointer of read/write element
aoBufferArray(1).nrItem=2;	                                % size of single read/write operation normally=1
aoBufferArray(1).dataType=1;				                    % data type 1='single' 2='int32' 3='uint32'
aoBufferArray(1).dsp=0;					                    	  % which DSP 0 or 1
aoBufferArray(1).len=buf_len;		                          % length of buffer
aoBufferArray(1).decFactor=0;                              % decimation factor
aoBufferArray(1).sdramPointerValue=0;	                    % pointer where to/from store/read data
aoBufferArray(1).direction=0;				                    % direction 0=read data 1=write data
aoBufferArray(1).circular=0;				                    % 0=linear 1=circular
aoBufferArray(1).bufferNumber=1;			                    % which buffer number 1 to 6 for each board
aoBufferArray(1).firstDsp=sys_data.hvc_board;              % first DSP where to write setup
aoBufferArray(1).lastDsp=sys_data.hvc_board;               % last DSP where to write setup
aoBufferArray(2).bufferName='buff_2';	                    % name of buffer, is not mandatory, just useful
aoBufferArray(2).triggerPointer=aoGetAddress('hvc_update_cmd')+3;		                    % pointer of trigger
aoBufferArray(2).triggerDataType=3;		                    % trigger data type 1='single' 2='int32' 3='uint32'
aoBufferArray(2).triggerDsp=0;			                    % number of DSP 0 or 1
aoBufferArray(2).triggerMask=4294967295;                   % trigger mask (these bits are checked) normaly should be 0xFFFFFFFF
aoBufferArray(2).triggerValue=500;                         % trigger value
aoBufferArray(2).triggerCompare=3;		                    % trigger compare type: 0: none 1: "value<*pointer" 2: "value>*pointer" 3: "value=*pointer"
aoBufferArray(2).dspPointer=aoGetAddress('hvc_TT1_preshaped_cmd'); % pointer of read/write element
% aoBufferArray(2).dspPointer=aoGetAddress('hvc_TT1_pos_current'); % pointer of read/write element
aoBufferArray(2).nrItem=2;	                                % size of single read/write operation normally=1
aoBufferArray(2).dataType=1;				                    % data type 1='single' 2='int32' 3='uint32'
aoBufferArray(2).dsp=0;					                    	  % which DSP 0 or 1
aoBufferArray(2).len=buf_len;		                          % length of buffer
aoBufferArray(2).decFactor=0;                              % decimation factor
aoBufferArray(2).sdramPointerValue=aoBufferArray(1).sdramPointerValue+aoBufferArray(1).nrItem*aoBufferArray(1).len;	                    % pointer where to/from store/read data
aoBufferArray(2).direction=0;				                    % direction 0=read data 1=write data
aoBufferArray(2).circular=0;				                    % 0=linear 1=circular
aoBufferArray(2).bufferNumber=2;			                    % which buffer number 1 to 6 for each board
aoBufferArray(2).firstDsp=sys_data.hvc_board;              % first DSP where to write setup
aoBufferArray(2).lastDsp=sys_data.hvc_board;               % last DSP where to write setup
aoBufferArray(3).bufferName='buff_3';	                    % name of buffer, is not mandatory, just useful
aoBufferArray(3).triggerPointer=aoGetAddress('hvc_update_cmd')+3;		                    % pointer of trigger
aoBufferArray(3).triggerDataType=3;		                    % trigger data type 1='single' 2='int32' 3='uint32'
aoBufferArray(3).triggerDsp=0;			                    % number of DSP 0 or 1
aoBufferArray(3).triggerMask=4294967295;                   % trigger mask (these bits are checked) normaly should be 0xFFFFFFFF
aoBufferArray(3).triggerValue=500;                         % trigger value
aoBufferArray(3).triggerCompare=3;		                    % trigger compare type: 0: none 1: "value<*pointer" 2: "value>*pointer" 3: "value=*pointer"
aoBufferArray(3).dspPointer=aoGetAddress('hvc_TT1_preshaped_curr'); % pointer of read/write element
% aoBufferArray(3).dspPointer=aoGetAddress('hvc_TT1_float_DAC_value'); % pointer of read/write element
aoBufferArray(3).nrItem=2;	                                % size of single read/write operation normally=1
aoBufferArray(3).dataType=1;				                    % data type 1='single' 2='int32' 3='uint32'
aoBufferArray(3).dsp=0;					                    	  % which DSP 0 or 1
aoBufferArray(3).len=buf_len;		                          % length of buffer
aoBufferArray(3).decFactor=0;                              % decimation factor
aoBufferArray(3).sdramPointerValue=aoBufferArray(2).sdramPointerValue+aoBufferArray(2).nrItem*aoBufferArray(2).len;	                    % pointer where to/from store/read data
aoBufferArray(3).direction=0;				                    % direction 0=read data 1=write data
aoBufferArray(3).circular=0;				                    % 0=linear 1=circular
aoBufferArray(3).bufferNumber=3;			                    % which buffer number 1 to 6 for each board
aoBufferArray(3).firstDsp=sys_data.hvc_board;              % first DSP where to write setup
aoBufferArray(3).lastDsp=sys_data.hvc_board;               % last DSP where to write setup
aoBufferArray(4).bufferName='buff_4';	                    % name of buffer, is not mandatory, just useful
aoBufferArray(4).triggerPointer=aoGetAddress('hvc_update_cmd')+3;		                    % pointer of trigger
aoBufferArray(4).triggerDataType=3;		                    % trigger data type 1='single' 2='int32' 3='uint32'
aoBufferArray(4).triggerDsp=0;			                    % number of DSP 0 or 1
aoBufferArray(4).triggerMask=4294967295;                   % trigger mask (these bits are checked) normaly should be 0xFFFFFFFF
aoBufferArray(4).triggerValue=500;                         % trigger value
aoBufferArray(4).triggerCompare=3;		                    % trigger compare type: 0: none 1: "value<*pointer" 2: "value>*pointer" 3: "value=*pointer"
% aoBufferArray(4).dspPointer=aoGetAddress('hvc_TT1_pos_current'); % pointer of read/write element
% aoBufferArray(4).dspPointer=aoGetAddress('hvc_TT1_float_DAC_value'); % pointer of read/write element
aoBufferArray(4).dspPointer=aoGetAddress('hvc_TT1_uint_DAC_value'); % pointer of read/write element
aoBufferArray(4).nrItem=2;	                                % size of single read/write operation normally=1
% aoBufferArray(4).dataType=1;				                    % data type 1='single' 2='int32' 3='uint32'
aoBufferArray(4).dataType=3;				                    % data type 1='single' 2='int32' 3='uint32'
aoBufferArray(4).dsp=0;					                    	  % which DSP 0 or 1
aoBufferArray(4).len=buf_len;		                          % length of buffer
aoBufferArray(4).decFactor=0;                              % decimation factor
aoBufferArray(4).sdramPointerValue=aoBufferArray(3).sdramPointerValue+aoBufferArray(3).nrItem*aoBufferArray(3).len;	                    % pointer where to/from store/read data
aoBufferArray(4).direction=0;				                    % direction 0=read data 1=write data
aoBufferArray(4).circular=0;				                    % 0=linear 1=circular
aoBufferArray(4).bufferNumber=4;			                    % which buffer number 1 to 6 for each board
aoBufferArray(4).firstDsp=sys_data.hvc_board;              % first DSP where to write setup
aoBufferArray(4).lastDsp=sys_data.hvc_board;               % last DSP where to write setup
aoBufferWriteSetup(aoBufferArray(1:4),1);

aoBufferTrigger(1:4,sys_data.hvc_board);
aoWrite('hvc_command_vector',[-1e-3 0e-3 0 0 0 0 0 0],sys_data.hvc_board);
aoWrite('hvc_current_vector',[-1e-3 0e-3 0 0 0 0 0 0],sys_data.hvc_board);
aoWrite('hvc_update_cmd',[0 0 0 1000],sys_data.hvc_board);
aoBufferWaitStart(1,sys_data.hvc_board);
aoBufferWaitStop(1,sys_data.hvc_board);

xy_pos=aoBufferReadData(aoBufferArray(1));
xy_pos=reshape(xy_pos,2,buf_len);
xy_cmd=aoBufferReadData(aoBufferArray(2));
xy_cmd=reshape(xy_cmd,2,buf_len);
xy_cnt_curr=aoBufferReadData(aoBufferArray(3));
xy_cnt_curr=reshape(xy_cnt_curr,2,buf_len);
xy_tot_curr=aoBufferReadData(aoBufferArray(4));
xy_tot_curr=reshape(xy_tot_curr,2,buf_len);
aoWrite('hvc_command_vector',[0 0 0 0 0 0 0 0],sys_data.hvc_board);
aoWrite('hvc_current_vector',[0 0 0 0 0 0 0 0],sys_data.hvc_board);
aoWrite('hvc_update_cmd',[0 0 0 1],sys_data.hvc_board);

start_time=1;
stop_time=10000;
time_vect=((0:buf_len-1)-499)/sys_data.cntFreq;

figure(1)
subplot(2,2,1);
hold off
plot(time_vect(start_time:stop_time),xy_pos(1,start_time:stop_time),'b')
hold on
plot(time_vect(start_time:stop_time),xy_cmd(1,start_time:stop_time),'r')
grid
title('Step response on x axis');
xlabel('Time [s]');
ylabel('Position [rad]');
legend('position','command','Location','East');
% axis([-.003 .006 -2e-4 22e-4]);
subplot(2,2,2);
hold off
% plot(time_vect(start_time:stop_time),xy_cnt_curr(1,start_time:stop_time)-aoRead('hvc_TT0_bias_current',0),'b')
plot(time_vect(start_time:stop_time),xy_cnt_curr(1,start_time:stop_time),'b')
hold on
plot(time_vect(start_time:stop_time),xy_tot_curr(1,start_time:stop_time),'r')
xlabel('Time [s]');
ylabel('Voltage [V]');
legend('Control voltage','Total voltage','Location','East');
aa=axis;aa(1)=-.003;aa(2)=.006;
% axis(aa);
subplot(2,2,3);
hold off
plot(time_vect(start_time:stop_time),xy_pos(2,start_time:stop_time),'b')
hold on
plot(time_vect(start_time:stop_time),xy_cmd(2,start_time:stop_time),'r')
grid
title('Step response on y axis');
xlabel('Time [s]');
ylabel('Position [rad]');
legend('position','command','Location','East');
% axis([-.003 .006 -2e-4 22e-4]);
subplot(2,2,4);
hold off
% plot(time_vect(start_time:stop_time),xy_cnt_curr(1,start_time:stop_time)-aoRead('hvc_TT0_bias_current',0),'b')
plot(time_vect(start_time:stop_time),xy_cnt_curr(2,start_time:stop_time),'b')
hold on
plot(time_vect(start_time:stop_time),xy_tot_curr(2,start_time:stop_time),'r')
xlabel('Time [s]');
ylabel('Voltage [V]');
legend('Control voltage','Total voltage','Location','East');
aa=axis;aa(1)=-.003;aa(2)=.006;
% axis(aa);

figure(2);
hold off
plot(diff(xy_tot_curr(1,start_time:stop_time)),'b');
hold on
plot(diff(xy_tot_curr(2,start_time:stop_time)),'r');
xlabel('Time [s]');
ylabel('Delta Voltage [bit]');
