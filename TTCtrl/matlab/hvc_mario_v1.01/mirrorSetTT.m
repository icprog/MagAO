%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% chiusura del loop                               %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
buf_len=32768;

a1=aoRead('hvc_TT1_step_ptr_preshaper_cmd',0:sys_data.nAct-1,1,sys_data.mirrorActMap);
a2=aoRead('hvc_TT1_step_ptr_preshaper_curr',0:sys_data.nAct-1,1,sys_data.mirrorActMap);

aoWrite('hvc_TT1_step_ptr_preshaper_cmd',round(sys_data.pres_buff_size/0.2/sys_data.cntFreq)*ones(sys_data.nAct,1),0:sys_data.nAct-1,1,sys_data.mirrorActMap);
aoWrite('hvc_TT1_step_ptr_preshaper_curr',round(sys_data.pres_buff_size/0.2/sys_data.cntFreq)*ones(sys_data.nAct,1),0:sys_data.nAct-1,1,sys_data.mirrorActMap);

% set zero position
aoWrite('hvc_TT1_pos_command',[0 0 0 0],0:3);
aoWrite('hvc_TT1_curr_command',[0 0 0 0],0:3);
aoWrite('hvc_TT1_bias_current',sys_data.bias_current_TT,0:3)
pause(1);

% lettura degli strain gauge
aoBufferArray(1).bufferName='hvc_TT1_distance';	           % name of buffer, is not mandatory, just useful
aoBufferArray(1).triggerPointer=0;		                    % pointer of trigger
aoBufferArray(1).triggerDataType=3;		                    % trigger data type 1='single' 2='int32' 3='uint32'
aoBufferArray(1).triggerDsp=0;			                    % number of DSP 0 or 1
aoBufferArray(1).triggerMask=4294967295;                   % trigger mask (these bits are checked) normaly should be 0xFFFFFFFF
aoBufferArray(1).triggerValue=uint32(sys_data.cntFreq*.04);% trigger value
aoBufferArray(1).triggerCompare=0;		                    % trigger compare type: 0: none 1: "value<*pointer" 2: "value>*pointer" 3: "value=*pointer"
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
aoBufferWriteSetup(aoBufferArray(1));
aoBufferStart(1,sys_data.hvc_board);
aoBufferWaitStop(1,sys_data.hvc_board);
xy_pos=aoBufferReadData(aoBufferArray(1));
xy_pos=reshape(xy_pos,2,buf_len);
x_pos=mean(xy_pos(1,:));
y_pos=mean(xy_pos(2,:));
% plot(xy_pos');

aoWrite('hvc_TT1_bias_command',[x_pos y_pos 0 0],0:3);
aoWrite('hvc_TT1_control_enable',[1 1],0:1);
aoWrite('hvc_TT1_post_loop_gain',sys_data.post_loop_gain,0:3);
pause(.1);
aoWrite('hvc_TT1_pre_loop_gain',sys_data.pre_loop_gain,0:3);
pause(.5);
aoWrite('hvc_TT1_bias_command',sys_data.bias_command_TT,0:3);
pause(.5);
aoWrite('hvc_TT1_step_ptr_preshaper_cmd',a1,0:sys_data.nAct-1,1,sys_data.mirrorActMap);
aoWrite('hvc_TT1_step_ptr_preshaper_curr',a2,0:sys_data.nAct-1,1,sys_data.mirrorActMap);

fprintf('errore in open loop %e / %e\n',x_pos,y_pos);

disp('mirror set done!!!');
