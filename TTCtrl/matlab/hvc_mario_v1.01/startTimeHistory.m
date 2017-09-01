
mgp_op_reset_devices(sys_data.hvc_board,sys_data.hvc_board,[16384 0]); % reset the frequency of time history A
mgp_op_reset_devices(sys_data.hvc_board,sys_data.hvc_board,[32768 0]); % reset the frequency of time history B
aoWrite('hvc_CmdHistEna',0,sys_data.hvc_board);
aoWrite('hvc_TT1CmdHistPtr',[0 CMD_HIST_BUFF_SIZE/4],sys_data.hvc_board);
aoWrite('hvc_TT1CmdHistGain',0, sys_data.hvc_board);
aoWrite('hvc_TT2CmdHistGain',0, sys_data.hvc_board);
aoWrite('hvc_TT1_step_ptr_preshaper_cmd',sys_data.pres_buff_size*ones(sys_data.nAct,1),0:sys_data.nAct-1,1,sys_data.mirrorActMap);
aoWrite('hvc_TT1_step_ptr_preshaper_curr',sys_data.pres_buff_size*ones(sys_data.nAct,1),0:sys_data.nAct-1,1,sys_data.mirrorActMap);

sys_data.syncA_freq= modulation_Hz;
sys_data.syncA_gain= modulation_Rad;

if modulation_Hz >0
    mgp_op_reset_devices(sys_data.hvc_board,sys_data.hvc_board,[16384 uint32(SYNC_BASE_FREQ/sys_data.syncA_freq)*65536]); % set the frequency of time history A
else
    mgp_op_reset_devices(sys_data.hvc_board,sys_data.hvc_board,[16384 0*65536]); % set the frequency of time history A
    disp('Sync with ccd active')
end

pause(.1);
aoWrite('hvc_CmdHistEna',1,sys_data.hvc_board);
for i=linspace(0,sys_data.syncA_gain,50)
   aoWrite('hvc_TT1CmdHistGain',i, sys_data.hvc_board);
   fprintf('.');
   pause(.005);
end
fprintf('\n');

