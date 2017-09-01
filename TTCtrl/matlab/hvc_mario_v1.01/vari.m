% %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% % ciclo di isteresi per la calibrazione dello straingauge %
% %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% ch0_pos=8e-3;
% ch1_pos=0e-3;
% ch0_volt=8e-3;
% ch1_volt=0e-3;
% 
% aoWrite('hvc_TT1_step_ptr_preshaper_cmd',ones(sys_data.nAct,1),0:sys_data.nAct-1,1,sys_data.mirrorActMap);
% aoWrite('hvc_TT1_step_ptr_preshaper_curr',ones(sys_data.nAct,1),0:sys_data.nAct-1,1,sys_data.mirrorActMap);
% aoWrite('hvc_command_vector',[0 0 0 0 0 0 0 0],sys_data.hvc_board);
% aoWrite('hvc_current_vector',[0 0 0 0 0 0 0 0],sys_data.hvc_board);
% aoWrite('hvc_update_cmd',[0 0 0 1],sys_data.hvc_board);
% pause(1);
% 
% buf_len=40000;
% 
% aoBufferArray(1).bufferName='hvc_TT1_distance';	     % name of buffer, is not mandatory, just useful
% aoBufferArray(1).triggerPointer=aoGetAddress('hvc_update_cmd')+3;		                    % pointer of trigger
% aoBufferArray(1).triggerDataType=3;		                    % trigger data type 1='single' 2='int32' 3='uint32'
% aoBufferArray(1).triggerDsp=0;			                    % number of DSP 0 or 1
% aoBufferArray(1).triggerMask=4294967295;                   % trigger mask (these bits are checked) normaly should be 0xFFFFFFFF
% aoBufferArray(1).triggerValue=.1*sys_data.cntFreq;         % trigger value
% aoBufferArray(1).triggerCompare=3;		                    % trigger compare type: 0: none 1: "value<*pointer" 2: "value>*pointer" 3: "value=*pointer"
% aoBufferArray(1).dspPointer=aoGetAddress('hvc_TT1_float_ADC_value'); % pointer of read/write element
% aoBufferArray(1).nrItem=2;	                                % size of single read/write operation normally=1
% aoBufferArray(1).dataType=1;				                    % data type 1='single' 2='int32' 3='uint32'
% aoBufferArray(1).dsp=0;					                    	  % which DSP 0 or 1
% aoBufferArray(1).len=buf_len;		                          % length of buffer
% aoBufferArray(1).decFactor=0;                              % decimation factor
% aoBufferArray(1).sdramPointerValue=0;	                    % pointer where to/from store/read data
% aoBufferArray(1).direction=0;				                    % direction 0=read data 1=write data
% aoBufferArray(1).circular=0;				                    % 0=linear 1=circular
% aoBufferArray(1).bufferNumber=1;			                    % which buffer number 1 to 6 for each board
% aoBufferArray(1).firstDsp=sys_data.hvc_board;              % first DSP where to write setup
% aoBufferArray(1).lastDsp=sys_data.hvc_board;               % last DSP where to write setup
% aoBufferArray(2).bufferName='hvc_TT1_distance';	     % name of buffer, is not mandatory, just useful
% aoBufferArray(2).triggerPointer=aoGetAddress('hvc_update_cmd')+3;		                    % pointer of trigger
% aoBufferArray(2).triggerDataType=3;		                    % trigger data type 1='single' 2='int32' 3='uint32'
% aoBufferArray(2).triggerDsp=0;			                    % number of DSP 0 or 1
% aoBufferArray(2).triggerMask=4294967295;                   % trigger mask (these bits are checked) normaly should be 0xFFFFFFFF
% aoBufferArray(2).triggerValue=.1*sys_data.cntFreq;         % trigger value
% aoBufferArray(2).triggerCompare=3;		                    % trigger compare type: 0: none 1: "value<*pointer" 2: "value>*pointer" 3: "value=*pointer"
% aoBufferArray(2).dspPointer=aoGetAddress('hvc_TT1_float_DAC_value'); % pointer of read/write element
% aoBufferArray(2).nrItem=2;	                                % size of single read/write operation normally=1
% aoBufferArray(2).dataType=1;				                    % data type 1='single' 2='int32' 3='uint32'
% aoBufferArray(2).dsp=0;					                    	  % which DSP 0 or 1
% aoBufferArray(2).len=buf_len;		                          % length of buffer
% aoBufferArray(2).decFactor=0;                              % decimation factor
% aoBufferArray(2).sdramPointerValue=aoBufferArray(1).sdramPointerValue+aoBufferArray(1).nrItem*aoBufferArray(1).len;	                    % pointer where to/from store/read data
% aoBufferArray(2).direction=0;				                    % direction 0=read data 1=write data
% aoBufferArray(2).circular=0;				                    % 0=linear 1=circular
% aoBufferArray(2).bufferNumber=2;			                    % which buffer number 1 to 6 for each board
% aoBufferArray(2).firstDsp=sys_data.hvc_board;              % first DSP where to write setup
% aoBufferArray(2).lastDsp=sys_data.hvc_board;               % last DSP where to write setup
% 
% aoBufferWriteSetup(aoBufferArray(1:2),1);
% 
% xADC_vect=[];
% yADC_vect=[];
% xDAC_vect=[];
% yDAC_vect=[];
% for i=1:20
%    aoBufferTrigger(1:2,sys_data.hvc_board);
%    aoWrite('hvc_command_vector',[ch0_pos  ch1_pos  0 0 0 0 0 0],sys_data.hvc_board);
%    aoWrite('hvc_current_vector',[ch0_volt ch1_volt 0 0 0 0 0 0],sys_data.hvc_board);
%    aoWrite('hvc_update_cmd',[0 0 0 .2*sys_data.cntFreq],sys_data.hvc_board);
%    aoBufferWaitStart(1,sys_data.hvc_board);
%    aoBufferWaitStop(1,sys_data.hvc_board);
% 
%    xy_ADC=aoBufferReadData(aoBufferArray(1));
%    xy_ADC=reshape(xy_ADC,2,buf_len);
%    xADC_vect=[xADC_vect;xy_ADC(1,:)];
%    yADC_vect=[yADC_vect;xy_ADC(2,:)];
%    xy_DAC=aoBufferReadData(aoBufferArray(2));
%    xy_DAC=reshape(xy_DAC,2,buf_len);
% %    xDAC_vect=[xDAC_vect;xy_DAC(1,:)-mean(xy_DAC(1,1:1000))];
% %    yDAC_vect=[yDAC_vect;xy_DAC(2,:)-mean(xy_DAC(2,1:1000))];
%    xDAC_vect=[xDAC_vect;xy_DAC(1,:)];
%    yDAC_vect=[yDAC_vect;xy_DAC(2,:)];
% 
%    aoBufferTrigger(1:2,sys_data.hvc_board);
%    aoWrite('hvc_command_vector',[-ch0_pos  -ch1_pos  0 0 0 0 0 0],sys_data.hvc_board);
%    aoWrite('hvc_current_vector',[-ch0_volt -ch1_volt 0 0 0 0 0 0],sys_data.hvc_board);
%    aoWrite('hvc_update_cmd',[0 0 0 .2*sys_data.cntFreq],sys_data.hvc_board);
%    aoBufferWaitStart(1,sys_data.hvc_board);
%    aoBufferWaitStop(1,sys_data.hvc_board);
% 
%    xy_ADC=aoBufferReadData(aoBufferArray(1));
%    xy_ADC=reshape(xy_ADC,2,buf_len);
% %    xADC_vect=[xADC_vect;-xy_ADC(1,:)+mean(xy_ADC(1,1:1000))];
% %    yADC_vect=[yADC_vect;-xy_ADC(2,:)+mean(xy_ADC(2,1:1000))];
%    xADC_vect=[xADC_vect;xy_ADC(1,:)];
%    yADC_vect=[yADC_vect;xy_ADC(2,:)];
%    xy_DAC=aoBufferReadData(aoBufferArray(2));
%    xy_DAC=reshape(xy_DAC,2,buf_len);
% %    xDAC_vect=[xDAC_vect;-xy_DAC(1,:)+mean(xy_DAC(1,1:1000))];
% %    yDAC_vect=[yDAC_vect;-xy_DAC(2,:)+mean(xy_DAC(2,1:1000))];
%    xDAC_vect=[xDAC_vect;xy_DAC(1,:)];
%    yDAC_vect=[yDAC_vect;xy_DAC(2,:)];
% 
% end
% 
% start_time=1;
% stop_time=buf_len;
% time_vect=(0:buf_len-1)/sys_data.cntFreq;
% 
% figure(1)
% hold off
% subplot(2,1,1);
% plot(time_vect(start_time:stop_time),xADC_vect(:,start_time:stop_time))
% subplot(2,1,2);
% plot(time_vect(start_time:stop_time),yADC_vect(:,start_time:stop_time))
% 
% figure(2)
% hold off
% subplot(2,1,1);
% plot(time_vect(start_time:stop_time),xDAC_vect(:,start_time:stop_time))
% subplot(2,1,2);
% plot(time_vect(start_time:stop_time),yDAC_vect(:,start_time:stop_time))
% 
% % fprintf('ADC mean %.0f - %.0f\n',mean(xADC_vect),mean(yADC_vect));
% % fprintf('DAC mean %f - %f\n',mean(xDAC_vect),mean(yDAC_vect));
% 

% %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% % test dei canali dello specchio                  %
% %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% relaisOn;
% 
% % set zero position
% aoWrite('hvc_TT1_bias_current',[10.64e-3 10.64e-3 42],0:2)
% 
% % lettura di una variabile
% aoBufferArray(1).bufferName='hvc_TT1_float_ADC_value';	     % name of buffer, is not mandatory, just useful
% aoBufferArray(1).triggerPointer=aoGetAddress('hvc_CmdHistEna');		                    % pointer of trigger
% aoBufferArray(1).triggerDataType=3;		                    % trigger data type 1='single' 2='int32' 3='uint32'
% aoBufferArray(1).triggerDsp=0;			                    % number of DSP 0 or 1
% aoBufferArray(1).triggerMask=4294967295;                   % trigger mask (these bits are checked) normaly should be 0xFFFFFFFF
% aoBufferArray(1).triggerValue=uint32(sys_data.cntFreq*.04);% trigger value
% aoBufferArray(1).triggerCompare=0;		                    % trigger compare type: 0: none 1: "value<*pointer" 2: "value>*pointer" 3: "value=*pointer"
% aoBufferArray(1).dspPointer=aoGetAddress('hvc_TT1_float_ADC_value')+1; % pointer of read/write element
% aoBufferArray(1).nrItem=1;	                                % size of single read/write operation normally=1
% aoBufferArray(1).dataType=1;				                    % data type 1='single' 2='int32' 3='uint32'
% aoBufferArray(1).dsp=0;					                    	  % which DSP 0 or 1
% aoBufferArray(1).len=32768;		                          % length of buffer
% aoBufferArray(1).decFactor=0;                              % decimation factor
% aoBufferArray(1).sdramPointerValue=0;	                    % pointer where to/from store/read data
% aoBufferArray(1).direction=0;				                    % direction 0=read data 1=write data
% aoBufferArray(1).circular=0;				                    % 0=linear 1=circular
% aoBufferArray(1).bufferNumber=1;			                    % which buffer number 1 to 6 for each board
% aoBufferArray(1).firstDsp=sys_data.hvc_board;              % first DSP where to write setup
% aoBufferArray(1).lastDsp=sys_data.hvc_board;               % last DSP where to write setup
% aoBufferWriteSetup(aoBufferArray(1),1);
% aoBufferStart(1,0,sys_data.nDsp-1);
% aoBufferWaitStop(1,0,1);
% pp1=aoBufferReadData(aoBufferArray(1),1);
% 
% relaisOff;

% %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% % chiusura del loop                               %
% %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% buf_len=32768;
% 
% coeff=zeros(4,9);
% 
% % pid_K=0.016;
% % pid_I=5.1e3;
% % pid_D=1.23e-4;
% % pid_N=32.8e3;
% 
% %    pid_K      pid_I     pid_D      pid_a
% ku=0.1;pu=4.4e-4;p=[ 0.6*ku        2*0.6*ku/pu      0.6*ku*pu/8     2*pi*10e3];
% % p=[ 0.06        267      3.38e-6      2*pi*10e3];
% %    p=[ 0.016        5.1e3      1.2e-4      2*pi*5e3];
% pid_sys=tf([p(1)+p(3)*p(4)  p(1)*p(4)+p(2)   p(2)*p(4)],[1 p(4) 0]);
% %    pid_sys=tf(p(1),1)+tf(p(2),[1 0])+tf([p(3)*p(4) 0],[1 p(4) 0]); % equivalente
% pid_sysD=c2d(pid_sys,1/sys_data.cntFreq,'tustin');
% [num,den]=tfdata(pid_sysD,'v');
% coeff(1,:)=[num(1),0,0,num(3),num(2),0,0,den(3),den(2)];
% 
% coeffVector=reshape(coeff,1,4*9);
% aoWrite('hvc_TT1_pos_coeff',coeffVector,sys_data.hvc_board);
% aoWrite('hvc_TT2_pos_coeff',coeffVector,sys_data.hvc_board);
% aoWrite('hvc_TT1_step_ptr_preshaper_cmd',round(PRES_BUFF_SIZE/.001/sys_data.cntFreq)*ones(sys_data.nAct,1),0:sys_data.nAct-1,1,sys_data.mirrorActMap);
% 
% relaisOn;
% 
% % set zero position
% aoWrite('hvc_TT1_pos_command',[0 0 0 0],0:3);
% aoWrite('hvc_TT1_bias_current',[10.64e-3 10.64e-3 42],0:2)
% pause(2);
% 
% % lettura degli strain gauge
% aoBufferArray(1).bufferName='hvc_TT1_distance';	     % name of buffer, is not mandatory, just useful
% aoBufferArray(1).triggerPointer=aoGetAddress('hvc_CmdHistEna');		                    % pointer of trigger
% aoBufferArray(1).triggerDataType=3;		                    % trigger data type 1='single' 2='int32' 3='uint32'
% aoBufferArray(1).triggerDsp=0;			                    % number of DSP 0 or 1
% aoBufferArray(1).triggerMask=4294967295;                   % trigger mask (these bits are checked) normaly should be 0xFFFFFFFF
% aoBufferArray(1).triggerValue=uint32(sys_data.cntFreq*.04);% trigger value
% aoBufferArray(1).triggerCompare=0;		                    % trigger compare type: 0: none 1: "value<*pointer" 2: "value>*pointer" 3: "value=*pointer"
% aoBufferArray(1).dspPointer=aoGetAddress('hvc_TT1_distance'); % pointer of read/write element
% aoBufferArray(1).nrItem=2;	                                % size of single read/write operation normally=1
% aoBufferArray(1).dataType=1;				                    % data type 1='single' 2='int32' 3='uint32'
% aoBufferArray(1).dsp=0;					                    	  % which DSP 0 or 1
% aoBufferArray(1).len=buf_len;		                          % length of buffer
% aoBufferArray(1).decFactor=0;                              % decimation factor
% aoBufferArray(1).sdramPointerValue=0;	                    % pointer where to/from store/read data
% aoBufferArray(1).direction=0;				                    % direction 0=read data 1=write data
% aoBufferArray(1).circular=0;				                    % 0=linear 1=circular
% aoBufferArray(1).bufferNumber=1;			                    % which buffer number 1 to 6 for each board
% aoBufferArray(1).firstDsp=sys_data.hvc_board;              % first DSP where to write setup
% aoBufferArray(1).lastDsp=sys_data.hvc_board;               % last DSP where to write setup
% 
% aoBufferArray(2).bufferName='hvc_TT1_distance';	     % name of buffer, is not mandatory, just useful
% aoBufferArray(2).triggerPointer=aoGetAddress('hvc_CmdHistEna');		                    % pointer of trigger
% aoBufferArray(2).triggerDataType=3;		                    % trigger data type 1='single' 2='int32' 3='uint32'
% aoBufferArray(2).triggerDsp=0;			                    % number of DSP 0 or 1
% aoBufferArray(2).triggerMask=4294967295;                   % trigger mask (these bits are checked) normaly should be 0xFFFFFFFF
% aoBufferArray(2).triggerValue=uint32(sys_data.cntFreq*.04);% trigger value
% aoBufferArray(2).triggerCompare=0;		                    % trigger compare type: 0: none 1: "value<*pointer" 2: "value>*pointer" 3: "value=*pointer"
% % aoBufferArray(2).dspPointer=aoGetAddress('hvc_TT1_preshaped_cmd'); % pointer of read/write element
% aoBufferArray(2).dspPointer=aoGetAddress('hvc_TT1_pos_current'); % pointer of read/write element
% aoBufferArray(2).nrItem=2;	                                % size of single read/write operation normally=1
% aoBufferArray(2).dataType=1;				                    % data type 1='single' 2='int32' 3='uint32'
% aoBufferArray(2).dsp=0;					                    	  % which DSP 0 or 1
% aoBufferArray(2).len=buf_len;		                          % length of buffer
% aoBufferArray(2).decFactor=0;                              % decimation factor
% aoBufferArray(2).sdramPointerValue=aoBufferArray(1).sdramPointerValue+aoBufferArray(1).nrItem*aoBufferArray(1).len;	                    % pointer where to/from store/read data
% aoBufferArray(2).direction=0;				                    % direction 0=read data 1=write data
% aoBufferArray(2).circular=0;				                    % 0=linear 1=circular
% aoBufferArray(2).bufferNumber=2;			                    % which buffer number 1 to 6 for each board
% aoBufferArray(2).firstDsp=sys_data.hvc_board;              % first DSP where to write setup
% aoBufferArray(2).lastDsp=sys_data.hvc_board;               % last DSP where to write setup
% aoBufferWriteSetup(aoBufferArray(1:2),1);
% 
% aoBufferStart(1,0,sys_data.nDsp-1);
% aoBufferWaitStop(1,0,1);
% xy_pos=aoBufferReadData(aoBufferArray(1),1);
% xy_pos=reshape(xy_pos,2,buf_len);
% x_pos=mean(xy_pos(1,:));
% y_pos=mean(xy_pos(2,:));
% % plot(xy_pos');
% 
% aoWrite('hvc_TT1_bias_command',[x_pos y_pos 0 0],0:3);
% 
% aoWrite('hvc_TT1_control_enable',[1 1],0:1);
% 
% aoWrite('hvc_TT1_pre_loop_gain',[1 0 0 0],0:3);
% aoWrite('hvc_TT1_post_loop_gain',[1 0 0 0],0:3);
% 
% aoWrite('hvc_TT1_bias_command',[0 0 0 0],0:3);
% 
% pause(1);
% 
% aoBufferStart(1:2,0,sys_data.nDsp-1);
% aoWrite('hvc_TT1_pos_command',[0e-3 0e-3 0 0],0:3);
% aoBufferWaitStop(1,0,1);
% xy_pos_cl=aoBufferReadData(aoBufferArray(1),1);
% xy_pos_cl=reshape(xy_pos_cl,2,buf_len);
% xy_cmd=aoBufferReadData(aoBufferArray(2),1);
% xy_cmd=reshape(xy_cmd,2,buf_len);
% 
% % mirrorRip;
% 
% figure(1)
% hold off
% plot(xy_pos_cl(1,:),'b')
% % hold on
% % plot(xy_cmd(1,:),'r')
% figure(2)
% hold off
% plot(xy_cmd(1,:),'r')
% 
% % aoRead('hvc_TT1_pos_current',0:3)
% 
% % aoWrite('hvc_TT1_post_loop_gain',[1e-3 0 0 0],0:3);

% %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% % loop optimization                               %
% %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% 
% sys_data.coeff=zeros(4,9);
% ku=0.04;pu=4.4e-4;
% %   pid_K   pid_I        pid_D        pid_a
% %    p=[ 0.8*ku  2*0.6*ku/pu  0.6*ku*pu/8  2*pi*10e3];
% p=[ 0.6*ku  2*0.8*ku/pu  0.8*ku*pu/8  2*pi*10e3];
% pid_sys=tf([p(1)+p(3)*p(4)  p(1)*p(4)+p(2)   p(2)*p(4)],[1 p(4) 0]);
% pid_sysD=c2d(pid_sys,1/sys_data.cntFreq,'tustin');
% [num,den]=tfdata(pid_sysD,'v');
% sys_data.coeff(1,:)=[num(1),0,0,num(3),num(2),0,0,den(3),den(2)];
% sys_data.coeff(2,:)=[num(1),0,0,num(3),num(2),0,0,den(3),den(2)];
% coeffVector=reshape(sys_data.coeff,1,4*9);
% aoWrite('hvc_TT1_pos_coeff',coeffVector,sys_data.hvc_board);
% aoWrite('hvc_TT2_pos_coeff',coeffVector,sys_data.hvc_board);
% aoWrite('hvc_TT1_step_ptr_preshaper_cmd',round(sys_data.pres_buff_size/.002/sys_data.cntFreq)*ones(sys_data.nAct,1),0:sys_data.nAct-1,1,sys_data.mirrorActMap);
% aoWrite('hvc_TT1_step_ptr_preshaper_curr',round(sys_data.pres_buff_size/.002/sys_data.cntFreq)*ones(sys_data.nAct,1),0:sys_data.nAct-1,1,sys_data.mirrorActMap);
