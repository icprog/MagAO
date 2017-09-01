Function update_rtdb, CREATE = create

    @adsec_common
    if n_elements(AO_STATUS_BACKUP) eq 0 then AO_STATUS_BACKUP=!AO_STATUS
     

    if sc.name_comm eq "Dummy" then return, adsec_error.ok
    if ~!AO_CONST.side_master and keyword_set(create) then return, adsec_error.gemeric_error
    st = strupcase(strmid(!AO_CONST.telescope_side,0,1))
    if keyword_set(CREATE) then begin

        !AO_STATUS.act_w_pos = adsec.act_w_pos  
        err = write_var('ADSEC.ACT_W_POS',  !AO_STATUS.act_w_pos)
        if err ne adsec_error.ok then return, err
        !AO_STATUS.act_w_curr = adsec.act_w_curr 
        err = write_var('ACT_W_CURR',  !AO_STATUS.act_w_curr)
        if err ne adsec_error.ok then return, err

    end


    if !AO_CONST.side_master then begin


;        !AO_STATUS.m2c = ""
        err = write_var('M2C',  !AO_STATUS.m2c)
        if err ne adsec_error.ok then return, err

;        !AO_STATUS.b0_a = ""
        err = write_var('B0_A',  !AO_STATUS.b0_a)
        if err ne adsec_error.ok then return, err

;        !AO_STATUS.b0_b = ""
        err = write_var('B0_B',  !AO_STATUS.b0_b)
        if err ne adsec_error.ok then return, err

;        !AO_STATUS.a_delay = ""
        err = write_var('A_DELAY',  !AO_STATUS.a_delay)
        if err ne adsec_error.ok then return, err

;        !AO_STATUS.b_delay_a = ""
        err = write_var('B_DELAY_A',  !AO_STATUS.b_delay_a)
        if err ne adsec_error.ok then return, err

;        !AO_STATUS.b_delay_b = ""
        err = write_var('B_DELAY_B',  !AO_STATUS.b_delay_b)
        if err ne adsec_error.ok then return, err

;        !AO_STATUS.g_gain_a = ""
        err = write_var('G_GAIN_A',  !AO_STATUS.g_gain_a)
        if err ne adsec_error.ok then return, err

;        !AO_STATUS.g_gain_b = ""
        err = write_var('G_GAIN_B',  !AO_STATUS.g_gain_b)
        if err ne adsec_error.ok then return, err

;        !AO_STATUS.ff_matrix=""
        err = write_var('FF_MATRIX',  !AO_STATUS.ff_matrix)
        if err ne adsec_error.ok then return, err
        
        !AO_STATUS.mean_gap = mean(sys_status.position)
        err = write_var('MEAN_GAP',  !AO_STATUS.mean_gap)
        if err ne adsec_error.ok then return, err

        !AO_STATUS.n_slope = rtr.n_slope
        err = write_var('N_SLOPE',  !AO_STATUS.n_slope)
        if err ne adsec_error.ok then return, err

        !AO_STATUS.n_slope_delay = rtr.n_slope_delay
        err = write_var('N_SLOPE_DELAY',  !AO_STATUS.n_slope_delay)
        if err ne adsec_error.ok then return, err

        !AO_STATUS.n_modes_delay = rtr.n_modes_delay
        err = write_var('N_MODES_DELAY',  !AO_STATUS.n_modes_delay)
        if err ne adsec_error.ok then return, err

        !AO_STATUS.n_modes2correct = rtr.n_modes2correct
        err = write_var('N_MODES2CORRECT',  !AO_STATUS.n_modes2correct)
        if err ne adsec_error.ok then return, err

        !AO_STATUS.pgain = sys_status.pgain
        err = write_var('PGAIN',  !AO_STATUS.pgain)
        if err ne adsec_error.ok then return, err

        !AO_STATUS.dgain = sys_status.dgain             
        err = write_var('DGAIN',  !AO_STATUS.dgain)
        if err ne adsec_error.ok then return, err

        !AO_STATUS.icc_cutfreq = sys_status.icc_cutfreq             
        err = write_var('ICC_CUTFREQ',  !AO_STATUS.icc_cutfreq)
        if err ne adsec_error.ok then return, err

        !AO_STATUS.icc_dcgain = sys_status.icc_dcgain
        err = write_var('ICC_DCGAIN',  !AO_STATUS.icc_dcgain)
        if err ne adsec_error.ok then return, err

        for i=0, n_tags(adam_in)-1 do !AO_STATUS.adam_in[i] = adam_in.(i)       
        err = write_var('ADAM_IN',  !AO_STATUS.adam_in)
        if err ne adsec_error.ok then return, err

        for i=0, n_tags(adam_out)-1 do !AO_STATUS.adam_out[i] = adam_out.(i)
        err = write_var('ADAM_OUT',  !AO_STATUS.adam_out)
        if err ne adsec_error.ok then return, err

        !AO_STATUS.oversampling_time = rtr.oversampling_time
        err = write_var('OVERSAMPLING_TIME',  !AO_STATUS.oversampling_time)
        if err ne adsec_error.ok then return, err

        err = write_var('DISTURB',  !AO_STATUS.disturb)
        if err ne adsec_error.ok then return, err

        err = write_var('DISTURB_STATUS',  !AO_STATUS.disturb_status)
        if err ne adsec_error.ok then return, err

        err = write_var('SHAPE',  !AO_STATUS.shape)
        if err ne adsec_error.ok then return, err

        err = write_var('FSM_STATE',  !AO_STATUS.fsm_state)
        if err ne adsec_error.ok then return, err

        err = write_var('HO_MATRIX',  !AO_STATUS.PMHOFOR)
        if err ne adsec_error.ok then return, err

        err = write_var('ZERN_APPLIED',  !AO_STATUS.ZERN_APPLIED)
        if err ne adsec_error.ok then return, err

        err = write_var('REF_POS',  !AO_STATUS.REF_POS)
        if err ne adsec_error.ok then return, err

        err = write_var('REF_FOR',  !AO_STATUS.REF_FOR)
        if err ne adsec_error.ok then return, err

    endif else begin


        err = read_var('B0_A',  tmp)
        if err ne adsec_error.ok then return, err
        if n_elements(tmp) eq 0 then tmp = ""
        !AO_STATUS.b0_a = string(tmp)

        err = read_var('B0_B',  tmp)
        if err ne adsec_error.ok then return, err
        if n_elements(tmp) eq 0 then tmp = ""
        !AO_STATUS.b0_b = string(tmp)

        err = read_var('A_DELAY',  tmp)
        if err ne adsec_error.ok then return, err
        if n_elements(tmp) eq 0 then tmp = ""
        !AO_STATUS.a_delay = string(tmp)

        err = read_var('B_DELAY_A',  tmp)
        if err ne adsec_error.ok then return, err
        if n_elements(tmp) eq 0 then tmp = ""
        !AO_STATUS.b_delay_a = string(tmp)

        err = read_var('B_DELAY_B',  tmp)
        if err ne adsec_error.ok then return, err
        if n_elements(tmp) eq 0 then tmp = ""
        !AO_STATUS.b_delay_b = string(tmp)

        err = read_var('M2C',  tmp)
        if err ne adsec_error.ok then return, err
        if n_elements(tmp) eq 0 then tmp = ""
        !AO_STATUS.m2c = string(tmp)

        err = read_var('G_GAIN_A',  tmp)
        if err ne adsec_error.ok then return, err
        if n_elements(tmp) eq 0 then tmp = ""
        !AO_STATUS.g_gain_a = string(tmp)

        err = read_var('G_GAIN_B',  tmp)
        if err ne adsec_error.ok then return, err
        if n_elements(tmp) eq 0 then tmp = ""
        !AO_STATUS.g_gain_b = string(tmp)

        err = read_var('FF_MATRIX',  tmp)
        if err ne adsec_error.ok then return, err
        if n_elements(tmp) eq 0 then tmp = ""
        !AO_STATUS.ff_matrix=string(tmp)

        err = read_var('MEAN_GAP',  tmp)
        if err ne adsec_error.ok then return, err
        !AO_STATUS.mean_gap = tmp

        err = read_var('N_SLOPE',  tmp)
        if err ne adsec_error.ok then return, err
        !AO_STATUS.n_slope = tmp
        rtr.n_slope = tmp

        err = read_var('N_SLOPE_DELAY',  tmp)
        if err ne adsec_error.ok then return, err
        !AO_STATUS.n_slope_delay = tmp
        rtr.n_slope_delay = tmp

        err = read_var('N_MODES_DELAY',  tmp)
        if err ne adsec_error.ok then return, err
        !AO_STATUS.n_modes_delay = tmp
        rtr.n_modes_delay = tmp

        err = read_var('N_MODES2CORRECT',  tmp)
        if err ne adsec_error.ok then return, err
        !AO_STATUS.n_modes2correct = tmp
        rtr.n_modes2correct = tmp

        err = read_var('PGAIN',  tmp)
        if err ne adsec_error.ok then return, err
        !AO_STATUS.pgain = tmp
        sys_status.pgain = tmp

        err = read_var('DGAIN',  tmp)
        if err ne adsec_error.ok then return, err
        !AO_STATUS.dgain = tmp
        sys_status.dgain = tmp

        err = read_var('ICC_CUTFREQ',  tmp)
        if err ne adsec_error.ok then return, err
        !AO_STATUS.icc_cutfreq = tmp
        sys_status.icc_cutfreq = tmp

        err = read_var('ICC_DCGAIN',  tmp)
        if err ne adsec_error.ok then return, err
        !AO_STATUS.icc_dcgain = tmp
        sys_status.icc_dcgain = tmp

        err = read_var('ADAM_IN',  tmp0)
        if err ne adsec_error.ok then return, err
        !AO_STATUS.adam_in  = tmp0
        for i=0, n_elements(tmp0)-1 do adam_in.(i) = tmp0[i]

        err = read_var('ADAM_OUT',  tmp0)
        if err ne adsec_error.ok then return, err
        !AO_STATUS.adam_out = tmp0 
        for i=0, n_elements(tmp0)-1 do adam_out.(i) = tmp0[i]

        err = read_var('OVERSAMPLING_TIME',  tmp)
        if err ne adsec_error.ok then return, err
        !AO_STATUS.oversampling_time = tmp
        rtr.oversampling_time = tmp

        err = read_var('DISTURB',  tmp)
        if err ne adsec_error.ok then return, err
        !AO_STATUS.disturb = tmp

        err = read_var('DISTURB_STATUS',  tmp)
        if err ne adsec_error.ok then return, err
        !AO_STATUS.disturb_status = tmp

        err = read_var('SHAPE',  tmp)
        if err ne adsec_error.ok then return, err
        !AO_STATUS.shape = tmp

        err = read_var('FSM_STATE',  tmp)
        if err ne adsec_error.ok then return, err
        !AO_STATUS.fsm_state = tmp

        err = read_var('HO_MATRIX',  tmp)
        if err ne adsec_error.ok then return, err
        !AO_STATUS.PMHOFOR = tmp

        err = read_var('ZERN_APPLIED',  tmp)
        if err ne adsec_error.ok then return, err
        !AO_STATUS.ZERN_APPLIED = tmp

        err = read_var('REF_POS',  tmp)
        if err ne adsec_error.ok then return, err
        !AO_STATUS.REF_POS = tmp

        err = read_var('REF_FOR',  tmp)
        if err ne adsec_error.ok then return, err
        !AO_STATUS.REF_FOR = tmp

    endelse

    return, adsec_error.ok

end
