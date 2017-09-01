%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% diag buffer con lettura degli strain gauge      %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% lettura degli strain gauge
clear aoBufferArray;

% relaisOn;
% pause(1);

% set zero position
aoWrite('_hvcfc_TT1_pos_command',zeros(sys_data.hvc_nAct,1),0:sys_data.hvc_nAct-1);
aoWrite('_hvcfc_TT1_curr_command',zeros(sys_data.hvc_nAct,1),0:sys_data.hvc_nAct-1);
if 0
   aoBufferArray(1).bufferName='_hvcfc_TT1_float_DAC_value';	            % name of buffer, is not mandatory, just useful
   aoBufferArray(1).triggerPointer=0;		                                 % pointer of trigger
   aoBufferArray(1).triggerDataType=3;		                                 % trigger data type 1='single' 2='int32' 3='uint32'
   aoBufferArray(1).triggerDsp=0;			                                 % number of DSP 0 or 1
   aoBufferArray(1).triggerMask=4294967295;                                % trigger mask (these bits are checked) normaly should be 0xFFFFFFFF
   aoBufferArray(1).triggerValue=uint32(sys_data.hvcCntFreq*.04);          % trigger value
   aoBufferArray(1).triggerCompare=0;		                                 % trigger compare type: 0: none 1: "value<*pointer" 2: "value>*pointer" 3: "value=*pointer"
   aoBufferArray(1).dspPointer=aoGetAddress('_hvcfc_TT1_float_DAC_value'); % pointer of read/write element
   aoBufferArray(1).nrItem=3;	                                             % size of single read/write operation normally=1
   aoBufferArray(1).dataType=1;				                                 % data type 1='single' 2='int32' 3='uint32'
   aoBufferArray(1).dsp=0;					                    	               % which DSP 0 or 1
   aoBufferArray(1).len=buf_len;		                                       % length of buffer
   aoBufferArray(1).decFactor=0;                                           % decimation factor
   aoBufferArray(1).sdramPointerValue=0;	                                 % pointer where to/from store/read data
   aoBufferArray(1).direction=0;				                                 % direction 0=read data 1=write data
   aoBufferArray(1).circular=0;				                                 % 0=linear 1=circular
   aoBufferArray(1).bufferNumber=1;			                                 % which buffer number 1 to 6 for each board
   aoBufferArray(1).firstDsp=sys_data.hvc_board;                           % first DSP where to write setup
   aoBufferArray(1).lastDsp=sys_data.hvc_board;                            % last DSP where to write setup
   aoBufferWriteSetup(aoBufferArray(1));
   aoBufferStart(1,sys_data.hvc_board);
   aoWrite('_hvcfc_TT1_bias_current',sys_data.bias_current_XY,0:sys_data.hvc_nAct-1);      % offset to 35V
   aoBufferWaitStop(1,sys_data.hvc_board);
   V_out=aoBufferReadData(aoBufferArray(1));
   V_out=reshape(V_out,3,buf_len)';
   plot((0:buf_len-1)/sys_data.hvcCntFreq,V_out(:,1:2));
   xlabel('time [s]');
   ylabel('Volt [V]');
   grid;
else
   aoWrite('_hvcfc_TT1_bias_current',sys_data.bias_current_XY,0:sys_data.hvc_nAct-1);      % offset to 35V
end
pause(2);

aoBufferArray(1).bufferName='_hvcfc_TT1_distance';	              % name of buffer, is not mandatory, just useful
aoBufferArray(1).triggerPointer=0;		                          % pointer of trigger
aoBufferArray(1).triggerDataType=3;		                          % trigger data type 1='single' 2='int32' 3='uint32'
aoBufferArray(1).triggerDsp=0;			                          % number of DSP 0 or 1
aoBufferArray(1).triggerMask=4294967295;                         % trigger mask (these bits are checked) normaly should be 0xFFFFFFFF
aoBufferArray(1).triggerValue=uint32(sys_data.hvcCntFreq*.04);   % trigger value
aoBufferArray(1).triggerCompare=0;		                          % trigger compare type: 0: none 1: "value<*pointer" 2: "value>*pointer" 3: "value=*pointer"
aoBufferArray(1).dspPointer=aoGetAddress('_hvcfc_TT1_distance'); % pointer of read/write element
aoBufferArray(1).nrItem=2;	                                      % size of single read/write operation normally=1
aoBufferArray(1).dataType=1;				                          % data type 1='single' 2='int32' 3='uint32'
aoBufferArray(1).dsp=0;					                    	        % which DSP 0 or 1
aoBufferArray(1).len=buf_len;		                                % length of buffer
aoBufferArray(1).decFactor=0;                                    % decimation factor
aoBufferArray(1).sdramPointerValue=0;	                          % pointer where to/from store/read data
aoBufferArray(1).direction=0;				                          % direction 0=read data 1=write data
aoBufferArray(1).circular=0;				                          % 0=linear 1=circular
aoBufferArray(1).bufferNumber=1;			                          % which buffer number 1 to 6 for each board
aoBufferArray(1).firstDsp=sys_data.hvc_board;                    % first DSP where to write setup
aoBufferArray(1).lastDsp=sys_data.hvc_board;                     % last DSP where to write setup
aoBufferWriteSetup(aoBufferArray(1));
aoBufferStart(1,sys_data.hvc_board);
aoBufferWaitStop(1,sys_data.hvc_board);
xy_pos=aoBufferReadData(aoBufferArray(1));
xy_pos=reshape(xy_pos,2,buf_len)';
x_pos=mean(xy_pos(:,1));
y_pos=mean(xy_pos(:,2));
if 0
   plot((0:buf_len-1)/sys_data.hvcCntFreq,xy_pos);
   xlabel('time [s]');
   ylabel('strain gauge [rad]');
   grid;
end

aoWrite('_hvcfc_TT1_bias_command',[x_pos y_pos 0 0],0:sys_data.hvc_nAct-1);
aoWrite('_hvcuc_TT1_control_enable',[1 1],0:1);
aoWrite('_hvcfc_TT1_post_loop_gain',sys_data.post_loop_gain,0:sys_data.hvc_nAct-1);
pause(.1);
aoWrite('_hvcfc_TT1_pre_loop_gain',sys_data.pre_loop_gain,0:sys_data.hvc_nAct-1);
pause(.5);
if 0
   aoBufferWriteSetup(aoBufferArray(1));
   aoBufferStart(1,sys_data.hvc_board);
   aoWrite('_hvcfc_TT1_bias_command',sys_data.bias_command_XY,0:sys_data.hvc_nAct-1);
   aoBufferWaitStop(1,sys_data.hvc_board);
   xy_pos=aoBufferReadData(aoBufferArray(1));
   xy_pos=reshape(xy_pos,2,buf_len)';
   x_pos=mean(xy_pos(:,1));
   y_pos=mean(xy_pos(:,2));
   plot((0:buf_len-1)/sys_data.hvcCntFreq,xy_pos);
   xlabel('time [s]');
   ylabel('strain gauge [rad]');
   grid;
else
   aoWrite('_hvcfc_TT1_bias_command',sys_data.bias_command_XY,0:sys_data.hvc_nAct-1);
end
pause(.5);

fprintf('errore in open loop %e / %e\n',x_pos,y_pos);

disp('mirror set done!!!');
