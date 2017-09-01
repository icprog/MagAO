aoWrite('_hvcfb_CurrHistAmplitude',0);
pause(5)
aoWrite('_hvcfc_TT0_bias_command',zeros(sys_data.hvc_nAct,1),0:sys_data.hvc_nAct-1);
aoWrite('_hvcfc_TT1_bias_command',zeros(sys_data.hvc_nAct,1),0:sys_data.hvc_nAct-1);
pause(.5);
aoWrite('_hvcfc_TT0_post_loop_gain',zeros(sys_data.hvc_nAct,1),0:sys_data.hvc_nAct-1);
aoWrite('_hvcfc_TT1_post_loop_gain',zeros(sys_data.hvc_nAct,1),0:sys_data.hvc_nAct-1);
pause(.5);
aoWrite('_hvcuc_TT0_control_enable',zeros(sys_data.hvc_nAct,1),0:sys_data.hvc_nAct-1);
aoWrite('_hvcfc_TT0_pre_loop_gain',zeros(sys_data.hvc_nAct,1),0:sys_data.hvc_nAct-1);
aoWrite('_hvcfc_TT0_bias_current',zeros(sys_data.hvc_nAct,1),0:sys_data.hvc_nAct-1);
aoWrite('_hvcfc_TT0_bias_command',zeros(sys_data.hvc_nAct,1),0:sys_data.hvc_nAct-1);
aoWrite('_hvcub_TT0_CurrHistEnable',0,0);
aoWrite('_hvcuc_TT1_control_enable',zeros(sys_data.hvc_nAct,1),0:sys_data.hvc_nAct-1);
aoWrite('_hvcfc_TT1_pre_loop_gain',zeros(sys_data.hvc_nAct,1),0:sys_data.hvc_nAct-1);
aoWrite('_hvcfc_TT1_bias_current',zeros(sys_data.hvc_nAct,1),0:sys_data.hvc_nAct-1);
aoWrite('_hvcfc_TT1_bias_command',zeros(sys_data.hvc_nAct,1),0:sys_data.hvc_nAct-1);
aoWrite('_hvcub_TT1_CurrHistEnable',0,0);
pause(1);

relaisOff;

disp('mirror rip done!!!');
