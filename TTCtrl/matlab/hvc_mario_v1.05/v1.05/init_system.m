
   %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
   % scarico del codice DSP                       %
   %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
   fprintf('Downloading HVC-DSP code...\n');
   aoDownloadCode(sys_data.hvc_board(1),sys_data.hvc_board(end),HVCMainProgram,0,sys_data.SCConnectionID);

   %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
   % firmware variables initilization             %
   %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
   fprintf('Initilizing HVC variables...\n');

   aoWrite('_hvcub_delay_DAC',              round(sys_data.DACDelayTime*sys_data.dspFreq),    sys_data.hvc_board, sys_data.SCConnectionID);
   aoWrite('_hvcub_DAC_not_sent',           0,                                                sys_data.hvc_board, sys_data.SCConnectionID);
   aoWrite('_hvcub_DAC_not_ready',          0,                                                sys_data.hvc_board, sys_data.SCConnectionID);
   aoWrite('_hvcub_EnableIsr',              0,                                                sys_data.hvc_board, sys_data.SCConnectionID);
   aoWrite('_hvcfb_MaxDeltaCommand',        sys_data.TTMaxDeltaCmd,                           sys_data.hvc_board, sys_data.SCConnectionID);
   aoWrite('_hvcub_prev_global_counter',    0,                                                sys_data.hvc_board, sys_data.SCConnectionID);
   aoWrite('_hvcfb_minmax_forget_fact',     sys_data.MMForgetFact,                            sys_data.hvc_board, sys_data.SCConnectionID);
   aoWrite('_hvcub_TT0_delay_acc',          round(sys_data.delayAcc*sys_data.hvcCntFreq),     sys_data.hvc_board, sys_data.SCConnectionID);
   aoWrite('_hvcub_TT0_num_samples',        round(sys_data.averageAcc*sys_data.hvcCntFreq),   sys_data.hvc_board, sys_data.SCConnectionID);
   aoWrite('_hvcfb_TT0_inv_num_samples',    1/round(sys_data.averageAcc*sys_data.hvcCntFreq), sys_data.hvc_board, sys_data.SCConnectionID);
   aoWrite('_hvcfb_TT0_min_command',        sys_data.TTMinCmd,                                sys_data.hvc_board, sys_data.SCConnectionID);
   aoWrite('_hvcfb_TT0_max_command',        sys_data.TTMinCmd,                                sys_data.hvc_board, sys_data.SCConnectionID);
   aoWrite('_hvcub_TT1_delay_acc',          round(sys_data.delayAcc*sys_data.hvcCntFreq),     sys_data.hvc_board, sys_data.SCConnectionID);
   aoWrite('_hvcub_TT1_num_samples',        round(sys_data.averageAcc*sys_data.hvcCntFreq),   sys_data.hvc_board, sys_data.SCConnectionID);
   aoWrite('_hvcfb_TT1_inv_num_samples',    1/round(sys_data.averageAcc*sys_data.hvcCntFreq), sys_data.hvc_board, sys_data.SCConnectionID);
   aoWrite('_hvcfb_TT1_min_command',        sys_data.XYMinCmd,                                sys_data.hvc_board, sys_data.SCConnectionID);
   aoWrite('_hvcfb_TT1_max_command',        sys_data.XYMaxCmd,                                sys_data.hvc_board, sys_data.SCConnectionID);
   aoWrite('_hvcfc_TT0_dist_A_coeff',       [sys_data.ADCbit2rad_x    sys_data.ADCbit2rad_y    0                       0], 0:(sys_data.hvc_nAct-1));
   aoWrite('_hvcfc_TT0_dist_B_coeff',       [sys_data.ADCradOffset_x  sys_data.ADCradOffset_y  0                       0], 0:(sys_data.hvc_nAct-1));
   aoWrite('_hvcfc_TT0_sat_DAC_value',      [sys_data.DAC_SatValTT_x  sys_data.DAC_SatValTT_y  sys_data.DAC_SatVal_c   0], 0:(sys_data.hvc_nAct-1));
   aoWrite('_hvcfc_TT0_nsat_DAC_value',     [sys_data.DAC_nSatValTT_x sys_data.DAC_nSatValTT_y sys_data.DAC_nSatVal_c  0], 0:(sys_data.hvc_nAct-1));
   aoWrite('_hvcfc_TT0_DAC_N2A_gain',       [sys_data.DACN2AGain      sys_data.DACN2AGain      sys_data.DACN2AGain     0], 0:(sys_data.hvc_nAct-1));
   aoWrite('_hvcfc_TT0_DAC_A2bit_gain',     [sys_data.DACr2BitGain_x  sys_data.DACr2BitGain_y  sys_data.DACV2BitGain_c 0], 0:(sys_data.hvc_nAct-1));
   aoWrite('_hvcfc_TT0_DAC_bit_offset',     [sys_data.DACBitOffset    sys_data.DACBitOffset    sys_data.DACBitOffset_c 0], 0:(sys_data.hvc_nAct-1));
   aoWrite('_hvcfc_TT0_pre_smoothed_step',  sys_data.SmoothStep*ones(sys_data.hvc_nAct,1),                                 0:(sys_data.hvc_nAct-1));
   aoWrite('_hvcfc_TT0_post_smoothed_step', sys_data.SmoothStep*ones(sys_data.hvc_nAct,1),                                 0:(sys_data.hvc_nAct-1));
   aoWrite('_hvcuc_TT0_step_ptr_preshaper_cmd',  max([1 round(sys_data.pres_buff_size/sys_data.cmdPreshTimeTT/sys_data.hvcCntFreq)])*ones(sys_data.hvc_nAct,1), 0:sys_data.hvc_nAct-1);
   aoWrite('_hvcuc_TT0_step_ptr_preshaper_curr', max([1 round(sys_data.pres_buff_size/sys_data.ffPreshTimeTT/sys_data.hvcCntFreq)])*ones(sys_data.hvc_nAct,1),  0:sys_data.hvc_nAct-1);
   aoWrite('_hvcfc_TT1_dist_A_coeff',       [sys_data.ADCbit2m_x      sys_data.ADCbit2m_y      0                       0], 0:(sys_data.hvc_nAct-1));
   aoWrite('_hvcfc_TT1_dist_B_coeff',       [sys_data.ADCmOffset_x    sys_data.ADCmOffset_y    0                       0], 0:(sys_data.hvc_nAct-1));
   aoWrite('_hvcfc_TT1_sat_DAC_value',      [sys_data.DAC_SatValXY_x  sys_data.DAC_SatValXY_y  0                       0], 0:(sys_data.hvc_nAct-1));
   aoWrite('_hvcfc_TT1_nsat_DAC_value',     [sys_data.DAC_nSatValXY_x sys_data.DAC_nSatValXY_y 0                       0], 0:(sys_data.hvc_nAct-1));
   aoWrite('_hvcfc_TT1_DAC_N2A_gain',       [sys_data.DACN2AGain      sys_data.DACN2AGain      sys_data.DACN2AGain     0], 0:(sys_data.hvc_nAct-1));
   aoWrite('_hvcfc_TT1_DAC_A2bit_gain',     [sys_data.DACm2BitGain_x  sys_data.DACm2BitGain_y  0                       0], 0:(sys_data.hvc_nAct-1));
   aoWrite('_hvcfc_TT1_DAC_bit_offset',     [sys_data.DACBitOffset    sys_data.DACBitOffset    sys_data.DACBitOffset_c 0], 0:(sys_data.hvc_nAct-1));
   aoWrite('_hvcfc_TT1_pre_smoothed_step',  sys_data.SmoothStep*ones(sys_data.hvc_nAct,1),                                 0:(sys_data.hvc_nAct-1));
   aoWrite('_hvcfc_TT1_post_smoothed_step', sys_data.SmoothStep*ones(sys_data.hvc_nAct,1),                                 0:(sys_data.hvc_nAct-1));
   aoWrite('_hvcuc_TT1_step_ptr_preshaper_cmd',  max([1 round(sys_data.pres_buff_size/sys_data.cmdPreshTimeXY/sys_data.hvcCntFreq)])*ones(sys_data.hvc_nAct,1), 0:sys_data.hvc_nAct-1);
   aoWrite('_hvcuc_TT1_step_ptr_preshaper_curr', max([1 round(sys_data.pres_buff_size/sys_data.ffPreshTimeXY/sys_data.hvcCntFreq)])*ones(sys_data.hvc_nAct,1),  0:sys_data.hvc_nAct-1);

   aoWrite('_hvcfb_TT0_pos_coeff',      sys_data.coeff_TT(:),        sys_data.hvc_board,sys_data.SCConnectionID);
   aoWrite('_hvcfb_TT1_pos_coeff',      sys_data.coeff_XY(:),        sys_data.hvc_board,sys_data.SCConnectionID);
   aoWrite('_hvcub_CurrHistStartPtr',   mvcub_CurrHistStartPtr,      sys_data.hvc_board,sys_data.SCConnectionID);
   aoWrite('_hvcfb_CurrHistLen',        length(mvcfb_CurrHistory)/4, sys_data.hvc_board,sys_data.SCConnectionID);
   aoWrite('_hvcfb_CurrHistAmplitude',  0,                           sys_data.hvc_board,sys_data.SCConnectionID);
   aoWrite('_hvcfb_CurrHistIntGain',    sys_data.HistIntGain,        sys_data.hvc_board,sys_data.SCConnectionID);
   aoWrite('_hvcub_TT0_CurrHistEnable', 0,                           sys_data.hvc_board,sys_data.SCConnectionID);
   aoWrite('_hvcub_TT1_CurrHistEnable', 0,                           sys_data.hvc_board,sys_data.SCConnectionID);
   aoWrite('_hvcuc_TT0_control_enable', 0,                           sys_data.hvc_board,sys_data.SCConnectionID);
   aoWrite('_hvcuc_TT1_control_enable', 0,                           sys_data.hvc_board,sys_data.SCConnectionID);
   aoWrite('_hvcfb_TT0_CurrHistStep',   0,                           sys_data.hvc_board,sys_data.SCConnectionID);
   aoWrite('_hvcfb_TT1_CurrHistStep',   0,                           sys_data.hvc_board,sys_data.SCConnectionID);
   aoWrite('_hvcfb_CurrHistory',        mvcfb_CurrHistory,           sys_data.hvc_board,sys_data.SCConnectionID);
   
   %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
   % inizializzazione del preshaper               %
   %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
   oneShape=(sin(-pi/2:2*pi/(sys_data.pres_buff_size*2-1):pi/2)+1)/2;
   shape=repmat(oneShape,1,size(sys_data.hvc_board,2));
   aoWrite('_hvcfb_preshaper_cmd_buffer',shape,sys_data.hvc_board);
   aoWrite('_hvcfb_preshaper_curr_buffer',shape,sys_data.hvc_board);

   %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
   % faccio partire il codice dei DSPs            %
   %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
   aoDspStartCode(sys_data.hvc_board(1),sys_data.hvc_board(end),sys_data.SCConnectionID);

   %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
   % abilitazione dell'isr                        %
   %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
   aoWrite('_hvcub_EnableIsr',1*ones(size(sys_data.hvc_board)),sys_data.hvc_board,sys_data.SCConnectionID);

disp('init system done!!!');
