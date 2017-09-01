aoWrite('hvc_TT1_post_loop_gain',zeros(sys_data.nAct,1),0:sys_data.nAct-1);
aoWrite('hvc_TT2_post_loop_gain',zeros(sys_data.nAct,1),0:sys_data.nAct-1);
pause(.5);
aoWrite('hvc_TT1_control_enable',zeros(sys_data.nAct,1),0:sys_data.nAct-1);
aoWrite('hvc_TT1_pre_loop_gain',zeros(sys_data.nAct,1),0:sys_data.nAct-1);
aoWrite('hvc_TT1_bias_current',zeros(sys_data.nAct,1),0:sys_data.nAct-1);
aoWrite('hvc_TT1_pos_command',zeros(sys_data.nAct,1),0:sys_data.nAct-1);
aoWrite('hvc_TT1_curr_command',zeros(sys_data.nAct,1),0:sys_data.nAct-1);
aoWrite('hvc_TT2_control_enable',zeros(sys_data.nAct,1),0:sys_data.nAct-1);
aoWrite('hvc_TT2_pre_loop_gain',zeros(sys_data.nAct,1),0:sys_data.nAct-1);
aoWrite('hvc_TT2_bias_current',zeros(sys_data.nAct,1),0:sys_data.nAct-1);
aoWrite('hvc_TT2_pos_command',zeros(sys_data.nAct,1),0:sys_data.nAct-1);
aoWrite('hvc_TT2_curr_command',zeros(sys_data.nAct,1),0:sys_data.nAct-1);
pause(.5);
aoWrite('hvc_DAC_value',uint32(repmat(hex2dec('80008000'),1,4)),sys_data.hvc_board);

relaisOff;

disp('mirror rip done!!!');
