;$Id: mirror_set_offload.pro,v 1.15 2009/05/14 16:27:24 labot Exp $
;+
;	NAME:
;    MIRROR_SET_OFFLOAD
;	
;	PURPOSE:
;    Thin shell procedure to set the working conditions
;
;	CATEGORY:
;  	 Adsec Hardware Function, Level 3
;
;	CALLING SEQUENCE:
;    err = mirror_set_offload( FINAL_POS=final_pos , F_GAIN=f_gain, S_GAIN=s_gain                     $
;                            , MAX_CURR=max_curr, MAX_AMP_POS=max_amp_pos, CL_INIT_DCMD=cl_init_dcmd  $
;                            , CTRL_CURR_INT = ctrl_curr_int, OVERSAMPLING_TIME=ovs                   $
;                            , SHELL_FLOATING_POS=shell_floating_pos, SHELL_SET_POS=shell_set_pos     $
;                            , SKIP_ASK4CL=skip_ask4cl
;
;
; 	INPUTS:
;    None
;
;	OUTPUTS:
;   Error code (see INIT_ERROR_CODES.pro)
;
;	KEYWORDS:
;   FINAL_POS:          final position mean to reach 
;   S_GAIN:             proportional gain for the closed loop actuators
;   F_GAIN:             final proportional gain for the 3 closed loop actuators
;   MAX_CURR:           max current for integrator() e close_loop() routines
;   MAX_AMP_POS:        max-min pos allowed in close_loop() routine (ex: pistoning)
;   CL_INIT_DCMD:       initial delta position command of all other actuators
;   CTRL_CURR_INT:      enable/disable control current integrator on set
;   OVERSAMPLING_TIME:  period for oversampling frame in mirror set
;   SHELL_FLOATING_POS: mean shell position for initial setting before pistoning
;                       (if CTRL_CURR_INT and SHELL_SET_POS are set, it can be omitted)
;   SHELL_SET_POS:      position vector to use as initial command 
;                       (usually the shell before is set with the shell_floating_pos parameter, 
;                       the position is saved and given in the next step to the shell_set_pos
;                       keyword in order to use the control current pseudo integrator)
;   SKIP_ASK4CL:        automatically continue
;
;   COMMON BLOCKS:
;       Adsec std common blocks (see ADSEC_COMMON.pro )
;
;	HISTORY
;	  ?? ??? ????   Written By Armando Riccardi (AR)
;	  12 Feb 2004 	Keyword STOP_ISOSTATIC added to stop the mirror setting in
;                       the isostatic configuration.
;	  16 Feb 2004	M. Xompero (MX). Added SKIP_ASK4CL keyword. If SKIP_ASK4CL is set,
;                       the program skip the dialog if we want to close
;                       the loop on all actuators with a dialog message.
;	  22 Jul 2004	D. Zanotti (DZ). Added two parameters in adsec structure: max_amp_pos,max_iter_integ
;     12 Aug 2004   AR
;       dsp_const.slowest_preshaper_step is used in set_prehaper call
;     02 Nov 2004, MX
;      Adsec.all changed in sc.all.
;     03 Aug 2005, MX
;      Accumulator setting restored after the mirror setting.
;     March 2006, DZ
;      Added the xadsec check.
;     20 Jun 2006, MX
;      Added the test about the coils status before proceed with the mirror set.
;     13 Jul 2007, MX
;      Added OTHER_CL_INIT_POS keyword.
;     04 Nov 2007, MX
;       Based on mirror_set_v3.pro and set_isostatic_v3.pro.
;       Removed unused keywords.
;       Added control current integrator management.
;       Use mode offloading to set.
;       TODO: well managing fast diagnostic thresholds
;     31 Jan 2008, DZ
;       Added the  FINAL_PR_CMD, FINAL_PR_CURR, RIP_MIRROR keywords.  
;-

function mirror_set_offload, FINAL_POS=final_pos , F_GAIN=f_gain, S_GAIN=s_gain                    $
                   , MAX_CURR=max_curr, MAX_AMP_POS=max_amp_pos, CL_INIT_DCMD=cl_init_dcmd         $
                   , CTRL_CURR_INT = ctrl_curr_int, OVERSAMPLING_TIME=ovs                          $
                   , SHELL_FLOATING_POS=shell_floating_pos, SHELL_SET_POS=shell_set_pos            $
                   , AUTO=auto, F_CMD_PRESH = f_cmd_presh, F_FF_PRESH = f_ff_presh   $
                   , FINAL_PR_CMD = final_pr_cmd, FINAL_PR_CURR = final_pr_curr, RIP_MIRROR=rip_mirror 

;----------------------------------------------------------------------------
    ;COMMON BLOCKS

 ;   on_error, 2

    @adsec_common
    common temporary_block, comm0, c_step, n_step, cl_list
    xadsec = (xregistered('xadsec') gt 0)
    rip_mirror = 1


    ;BASIC PARAMETERS AND DEFINITIONS
    
    if n_elements(ovs) eq 0 then ovs = 0.02
    fast_period = ovs
    act_w_curr = adsec.act_w_curr
    n_act_w_curr = n_elements(act_w_curr)
    err = update_status()
    if err ne adsec_error.ok then log_message, "Error!!!", ERR=err
   
    if adsec.act_wo_cl_on_set[0] ge 0 then begin

        temp_set = adsec.act_wo_cl_on_set
        temp_cl = adsec.act_w_cl
        temp_wo_cl = adsec.act_wo_cl
        err = intersection(temp_set, temp_wo_cl, inter_list)
        if err eq 0 then begin
            err =  complement(inter_list, temp_set, final_crack_list)
            if err ne adsec_error.ok then return, adsec_error.input_type
        endif else begin
            final_crack_list = temp_set
        endelse
        err = complement(final_crack_list, adsec.act_w_cl, cl_list)
        if err ne adsec_error.ok then return, adsec_error.input_type

    endif else begin
        cl_list = adsec.act_w_cl
    endelse

;----------------------------------------------------------------------------
    ;INITIAL CHECKS

    if n_elements(final_pos) eq 0 then begin
        message,' Final positions are not defined.', CONT=(sc.debug eq 0), ERR=adsec_error.input_type
        return, adsec_error.input_type
    endif

    if (total(final_pos lt min(adsec.min_pos_val[adsec.true_act]) or final_pos gt 130e-6) gt 0) then begin
        message,' Final positions requested out of ranges.', CONT=(sc.debug eq 0), ERR=adsec_error.input_type
        return, adsec_error.input_type
    endif

    if n_elements(s_gain) eq 0 then begin 
        message,'Start gain is not defined.', CONT=(sc.debug eq 0), ERR=adsec_error.input_type
        return, adsec_error.input_type
    endif

    if (s_gain gt 0.03) then begin
        message,'Start gain is too large.', CONT=(sc.debug eq 0), ERR=adsec_error.input_type
        return, adsec_error.input_type
    endif

    if n_elements(max_amp_pos) eq 0 then max_amp_pos = adsec.max_amp_pos ;340nm max-min pos allowed in close_loop() routine
    if max_amp_pos gt 5e-6 then begin
        message,'Amplitude requested for pistoning is too large..', CONT=(sc.debug eq 0), ERR=adsec_error.input_type
        return, adsec_error.input_type
    endif

    
    if n_elements(max_curr) eq 0 then max_curr = 2*adsec.weight_curr ; max current for integrator() e close_loop() routines
    if n_elements(CL_INIT_DCMD) eq 0 then cl_init_dcmd = 6e-6
    if n_elements(CL_INIT_DCMD) eq adsec.n_actuators then cl_init_dcmd = cl_init_dcmd[cl_list]
    if n_elements(S_GAIN) eq 0 then s_gain = min(adsec.weight_curr/2.*0.25/cl_init_dcmd /1e6 )
    ;[N/um] ; forza magneti bias/4 (fattore sicurezza)/delta_posizione ;
    
    ;TEST on the TSTART_GAIN compatibility with FORCE SATURATION
    new_force = cl_init_dcmd*s_gain*mean(adsec.gain_to_bbgain[cl_list])
    cond_sat = total((new_force lt adsec.neg_sat_force/2.) or (new_force gt adsec.pos_sat_force/2.)) gt 0
    cond_max_curr = total(abs(new_force) gt adsec.max_curr) gt 0
    if cond_sat or cond_max_curr then begin    
        message,'Start gain and initial_delta_pos are not compatible for isostatic configuration.', CONT=(sc.debug eq 0), ERR=adsec_error.input_type
        return, adsec_error.input_type
    endif
    
    ;COILS ENABLE TEST
;    err = test_coils(status)
;    if err ne adsec_error.ok then return, err
    
;----------------------------------------------------------------------------
;-------------------- MIRROR SETUP ------------------------------------------   
;----------------------------------------------------------------------------

;----------------------------------------------------------------------------
    ;FORCE DISABLING TIMEOUT FRAMES DIAGNOSTIC

    err = set_diagnostic(over=0.0)
    if err ne adsec_error.ok then return,err

;----------------------------------------------------------------------------
    ;CLEANING CURRENTS AND COMMANDS

    err = clear_dacs(sc.all)
    if err ne adsec_error.ok then return, err

    err = clear_commands(sc.all)
    if err ne adsec_error.ok then return, err


    timepcmd = 0.1e-3
    timepcurr = 0.7e-3
    time_ps = max([timepcmd, timepcurr])*1.1 > 0.01

    pr_cmd = ((round(adsec.preshaper_len*adsec.sampling_time/timepcmd)  > 1) $
           < dsp_const.fastest_preshaper_step)
    pr_cur = ((round(adsec.preshaper_len*adsec.sampling_time/timepcurr)  > 1) $
           < dsp_const.fastest_preshaper_step)

    err = set_preshaper(sc.all, pr_cmd)
    if err ne adsec_error.ok then return, err
    err = set_preshaper(sc.all, pr_cur, /FF)
    if err ne adsec_error.ok then return, err

    acc_period = 0.8e-3

;----------------------------------------------------------------------------    
    ;PRESHAPERS SETTING
;    step_ps = dsp_const.slowest_preshaper_step
;    time_ps = adsec.preshaper_len/step_ps*adsec.sampling_time
;    err = set_preshaper(sc.all_actuators, step_ps)		
;    if err ne adsec_error.ok then return,err
;    err = set_preshaper( sc.all_actuators, step_ps, /FF)
;    if err ne adsec_error.ok then return,err

;----------------------------------------------------------------------------    
    ;ACCUMULATORS SETTINGS
;    acc_period = fast_period*0.95

    acc_period = 0.8e-3
    acc_delay = ceil(acc_period/2./adsec.sampling_time)
    acc_len   = long(acc_period/adsec.sampling_time-acc_delay)

    log_print, "ACC_DELAY: "+ strtrim(acc_delay,2)+" ("+strtrim(acc_delay*adsec.sampling_time,2),"s)"
    log_print, "ACC_LEN  : "+ strtrim(acc_len,2)+" ("+strtrim(acc_len*adsec.sampling_time,2),"s)"
    log_print, "MAX FREQ ALLOWED: "+ strtrim( 1d/acc_period , 2)

    err = set_acc(acc_len,acc_delay)
    if err ne adsec_error.ok then return, err
    wait, 0.1
    err = start_acc(/WAIT)
    if err ne adsec_error.ok then return, err

;----------------------------------------------------------------------------
    ;RECONSTRUCTOR SETTINGS

    ;set up the modes/commands/forces thresholds
    err = set_safe_skip(ACTLIST=adsec.act_w_cl, MODLIST=indgen(n_elements(adsec.act_w_cl)))
    if err ne adsec_error.ok then return, err
  
    ;set up the dummy recostruction/delay/m2c matrices
    b0 = fltarr(rtr.n_slope,adsec.n_actuators)
    b_delay = fltarr(rtr.n_slope,adsec.n_actuators, rtr.n_slope_delay)       ;(1 delay slopes)
    a_delay = fltarr(adsec.n_actuators,adsec.n_actuators,rtr.n_modes_delay)  ;(2 delay modes)
    ;b0[0,0] = identity(adsec.n_actuators)
    old_m2c_mat = *rtr.m2c_matrix
    new_m2c_mat = fltarr(rtr.n_modes2correct, adsec.n_actuators)
    err = set_m2c_matrix(new_m2c_mat)
    if err ne adsec_error.ok then message, "Error encountered: "+strtrim(err,2) 
    ;old_ff_mat = adsec.ff_matrix
    ;adsec.ff_matrix = fltarr(adsec.n_actuators, adsec.n_actuators)
    ;err = send_ff_matrix(adsec.ff_matrix)
    ;if err ne adsec_error.ok then return, err

    g_gain = replicate(0.0, adsec.n_actuators)

    err = set_b0_matrix(b0)
    if err ne adsec_error.ok then log_message, "Error encountered: "+strtrim(err,2), ERR=err

    err = set_b_delay_matrix(b_delay)
    if err ne adsec_error.ok then log_message, "Error encountered: "+strtrim(err,2), ERR=err

    err = set_a_delay_matrix(a_delay)
    if err ne adsec_error.ok then log_message, "Error encountered: "+strtrim(err,2), ERR=err

    err = set_g_gain(sc.all, g_gain)
    if err ne adsec_error.ok then log_message, "Error encountered: "+strtrim(err,2), ERR=err

;----------------------------------------------------------------------------
    ;CONTROL CURRENT PSEUDO-INTEGRATOR RESET

        err = set_icc(/RESET)
        if err ne adsec_error.ok then log_message, "Error encountered: "+strtrim(err,2), ERR=err

;        if keyword_set(CTRL_CURR_INT) then begin
;            err = set_icc(0.1, 1.6)
;            if err ne adsec_error.ok then message, "Error encountered: "+strtrim(err,2)
;        endif else begin
;            err = set_icc(/RESET)
;            if err ne adsec_error.ok then message, "Error encountered: "+strtrim(err,2)
;        endelse

;        err = update_status(samples)
;        if err ne adsec_error.ok then begin
;            message, "Error updating status!!",CONT= (sc.debug eq 0B)
;            return, err
;        endif

;----------------------------------------------------------------------------
    ;WFS/MIRROR/SKIP/PENDING COUNTERS RESET

    err = clear_skip_counter() 
    if err ne adsec_error.ok then return, err

;----------------------------------------------------------------------------
    ;PROP/DERIV GAINS CLEANING
    err = set_gain(sc.all, 0.0)
    if err ne adsec_error.ok then return,err
    err = set_gain(sc.all, 0.0, /SPEED)
    if err ne adsec_error.ok then return,err

;----------------------------------------------------------------------------
;------------------------ START MIRROR SET ----------------------------------
;----------------------------------------------------------------------------

;----------------------------------------------------------------------------
    ;INITIAL CURRENT APPLICATION

    ;cl_list: close loop actuators without adsec.act_wo_cl_on_set
    ;Actuators on crack temporary in open loop

    if adsec.act_wo_cl_on_set[0] gt -1 then begin

        err = set_safe_skip(ACTLIST=adsec.act_wo_cl_on_set, MINCMD=0)
        if err ne adsec_error.ok then return, adsec_error.input_type

    endif 

;----------------------------------------------------------------------------
    ;START CONTROL ON ALL ACTUATORS

    if keyword_set(CTRL_CURR_INT) then begin
        err = write_des_pos(adsec.act_w_pos,  shell_set_pos[adsec.act_w_pos])
        if err ne adsec_error.ok then return,err
    endif else begin
        err = write_des_pos(cl_list, sys_status.position[cl_list]+cl_init_dcmd)
        if err ne adsec_error.ok then return,err
    endelse

    err = start_control(cl_list)
    if err ne adsec_error.ok then return,err
    
;----------------------------------------------------------------------------
    ;PUT COMMANDS ON BIAS

    err = set_cmd2bias()
    if err ne adsec_error.ok then return,err

    err = update_status(samples)
    if err ne adsec_error.ok then return, err
    if keyword_set(xadsec) then update_panels, /OPT, /NO_READ

;----------------------------------------------------------------------------
    ;FASTDAGNOSTIC THRESHOLDS SETUP

    err =  getdiagnparam(process_list.fastdiagn.msgd_name,  'ChDistAverage', -1, -1, actual)
    if err ne adsec_error.ok then return, err
    old_alarm_v = (actual.alarm_min)
    old_warn_v = (actual.warning_min)
    alarm_v = 2e-6
    ;for kk=0 , n_elements(adsec.act_w_pos)-1 do begin
        ;changes = setdiagnparam(process_list.fastdiagn.msgd_name,  'ChDistAverage', adsec.act_w_pos[kk], adsec.act_w_pos[kk], ALARM_MIN=alarm_v, TIM=1)
        ;if changes ne 1 then return, adsec_error.generic_error
        ;changes = setdiagnparam(process_list.fastdiagn.msgd_name,  'ChDistAverage', adsec.act_w_pos[kk], adsec.act_w_pos[kk], WARNING_MIN=alarm_v, TIM=1)
        ;if changes ne 1 then return, adsec_error.generic_error
        changes = setdiagnparam(process_list.fastdiagn.msgd_name,  'ChDistAverage', -1, -1, ALARM_MIN=alarm_v, warning_min=alarm_v, TIM=1)
        log_print, "FAST CHANGED"
    ;endfor
    
    wait, 1

;----------------------------------------------------------------------------
    ;ENABLE MASTER DIAGNOSTIC

    err = set_disturb(/RESET, /DISABLE)
     if err ne adsec_error.ok then return, err

    err = set_diagnostic(/MASTER)
    if err ne adsec_error.ok then return, err

    err = set_diagnostic(OVER=ovs)
    if err ne adsec_error.ok then return, err
    
    wait, 10*fast_period > 0.1

    err = update_status()
    if err ne adsec_error.ok then return, err
    if keyword_set(xadsec) then update_panels, /OPT, /NO_READ

    pos0=sys_status.position[cl_list]
    gain = fltarr(n_elements(cl_list))
    counter=1
    max_count = 1000

;-----------------------------------------------------------------------------
    ;RAMPING CL_LIST GAIN

    err = ramp_gain(cl_list, s_gain)
    if err ne adsec_error.ok then return,err

    err = update_status()
    if err ne adsec_error.ok then return, err
    if keyword_set(XADSEC) then update_panels, /NO_READ

;-----------------------------------------------------------------------------
    ;START TO PUSH THE MIRROR MANTAINING FORCE/MOMENT
    
;    if keyword_set(CTRL_CURR_INT) then begin

 ;       err = apply_curr_opt(XADSEC=xadsec,N_ITER=1, /OFFLOAD)
;        if err ne adsec_error.ok then return,err

;        err = apply_curr_opt(XADSEC=xadsec,N_ITER=1, /OFFLOAD)
;        if err ne adsec_error.ok then return,err
;----------------------------------------------------------------------------
        ;CONTROL CURRENT PSEUDO-INTEGRATOR

        if keyword_set(CTRL_CURR_INT) then begin
            err = set_icc(5.0, 10.)
            if err ne adsec_error.ok then log_message, "Error encountered: "+strtrim(err,2), ERR=err
        endif else begin
            err = set_icc(/RESET)
            if err ne adsec_error.ok then log_message, "Error encountered: "+strtrim(err,2), ERR=err
        endelse

;        err = update_status(samples)
;        if err ne adsec_error.ok then begin
;            message, "Error updating status!!",CONT= (sc.debug eq 0B)
;            return, err
;        endif

;    endif

    if keyword_set(CTRL_CURR_INT) and n_elements(SHELL_SET_POS) gt 0 then comp_value = shell_set_pos $
                                  else comp_value = shell_floating_pos

    repeat begin
        ;if ~keyword_set(CTRL_CURR_INT) then begin

            err = apply_curr_opt(XADSEC=xadsec,N_ITER=1, /OFFLOAD)
            if err ne adsec_error.ok then return,err

;        endif else begin
            wait, 2*fast_period 
            err = update_status()
            if err ne adsec_error.ok then return,err
            if keyword_set(xadsec) then update_panels, /OPT, /NO_R
;        endelse
        
        log_print, "av. current  : "+strtrim(mean(sys_status.current[act_w_curr]), 2)
        log_print, "ctrl currents: "+strtrim(mean(sys_status.ctrl_current[cl_list]), 2)
        log_print, "ff currents  : "+strtrim(mean(sys_status.ff_current[cl_list]), 2)
        counter = counter+1
        
    endrep until (mean(sys_status.position[adsec.act_w_pos]-comp_value[adsec.act_w_pos]) gt 0) or $
                 (counter ge max_count) 


    if counter ge max_count then begin
        message, "The requested gain has not been applied. Too many iterations.", CONT=sc.debug eq 0B
        return, adsec_error.generic_error
    endif
 
;    if keyword_set(CTRL_CURR_INT) then begin
;        err = set_offload_cmd((shell_set_pos-shell_floating_pos)[adsec.act_w_pos], /START, /NOCHECK)
;        if err ne adsec_error.ok then return, err
;    endif


;----------------------------------------------------------------------------
    ;UPDATES ONLY FOR VISIVE PURPOSE

    wait, fast_period >0.01 
    err = update_status()
    if err ne adsec_error.ok then return, err
    if keyword_set(XADSEC) then begin
            wait, 2*fast_period 
            err = update_status()
            if err ne adsec_error.ok then return,err
            if keyword_set(xadsec) then update_panels, /OPT, /NO_R
            wait, 2*fast_period 
            err = update_status()
            if err ne adsec_error.ok then return,err
            if keyword_set(xadsec) then update_panels, /OPT, /NO_R
            wait, 2*fast_period 
            err = update_status()
            if err ne adsec_error.ok then return,err
            if keyword_set(xadsec) then update_panels, /OPT, /NO_R
            wait, 2*fast_period 
            err = update_status()
            if err ne adsec_error.ok then return,err
            if keyword_set(xadsec) then update_panels, /OPT, /NO_R
            wait, 2*fast_period 
            err = update_status()
            if err ne adsec_error.ok then return,err
            if keyword_set(xadsec) then update_panels, /OPT, /NO_R
            wait, 2*fast_period 
            err = update_status()
            if err ne adsec_error.ok then return,err
            if keyword_set(xadsec) then update_panels, /OPT, /NO_R
            wait, 2*fast_period 
            err = update_status()
            if err ne adsec_error.ok then return,err
            if keyword_set(xadsec) then update_panels, /OPT, /NO_R
            wait, 2*fast_period 
            err = update_status()
            if err ne adsec_error.ok then return,err
            if keyword_set(xadsec) then update_panels, /OPT, /NO_R
    endif

;----------------------------------------------------------------------------
    ;RESTORE FULLY WORKING FAST DIAGNOSTIC THRESHOLDS

    ;for kk=0 , n_elements(adsec.act_w_pos)-1 do begin
    ;    err = setdiagnparam(process_list.fastdiagn.msgd_name,  'ChDistAverage', adsec.act_w_pos[kk], adsec.act_w_pos[kk] $
    ;                        , WARNING_MIN=old_warn_v[kk], TIM=1)
    ;     if changes ne 1 then return, adsec_error.generic_error 
    ;     changes = setdiagnparam(process_list.fastdiagn.msgd_name,  'ChDistAverage', adsec.act_w_pos[kk], adsec.act_w_pos[kk], ALARM_MIN=old_alarm_v[kk], TIM=1)
    ;     if changes ne 1 then return, adsec_error.generic_error 
    ; endfor

     changes = setdiagnparam(process_list.fastdiagn.msgd_name,  'ChDistAverage', -1, -1, ALARM_MIN=old_alarm_v[0], warning_min=old_warn_v[0], TIM=1)
    
;----------------------------------------------------------------------------
    ;CHECK IF AUTOMATICALLY CONTINUE
    if mean(sys_status.position[adsec.act_w_pos]) gt 50e-6 then begin
        log_print, "WRONG SHELL POSITION! AUTOMATICAL RIPPING AND FAULT RECOVERY"
        err = rip()
        if err ne adsec_error.ok then return,err
        err = adam_disable_coils()
        if err ne adsec_error.ok then return,err
;        err = fsm_fault_recovery()
        rip_mirror = 1
    endif




    if ~keyword_set(AUTO) then begin
        answ = dialog_message(["The mirror is in "+strtrim(mean(sys_status.position[adsec.act_w_pos]),2 ), $
                                   "Do you want to continue to the final position?"], /QUEST)

        
        if strlowcase(answ) eq "no" then begin
            
            
            ;err = rip(XADSEC=xadsec)
            ;if err ne adsec_error.ok then return,err
            ;wait, 1
    
            ;err = clear_dacs(sc.all)
            ;if err ne adsec_error.ok then return,err

            ;err = clear_commands(sc.all)
            ;if err ne adsec_error.ok then return,err

            ;err = adam_disable_coils()
            ;if err ne adsec_error.ok then return,err
    
            ;err = set_diagnostic(MASTER=0, OVERS=0)
            ;if err ne adsec_error.ok then return, err
            
            ;rip_mirror = 1
            
            ;err = zero_ctrl_current(/OFF)
            return, adsec_error.ok
            
        endif 
    endif

;----------------------------------------------------------------------------
        ;PISTON APPLICATION
        err = get_commands(cl_list,comm0) ; dovrebbe essere 0....
        if err ne adsec_error.ok then return, err
        
        err = get_commands(cl_list,comm0_off, /OFFLOADCMD)
        if err ne adsec_error.ok then return, err
    
        tmp_pos = final_pos
        final_pos = fltarr(adsec.n_actuators) + tmp_pos
        cl_act_pos = final_pos[cl_list]
        dcomm = cl_act_pos-comm0
        n_step = max(floor(dcomm / max_amp_pos)) ;;;;;;DA DECIDERE SE METTERLO DA FILE DI CONF
        c_step = dcomm/n_step
        
        err = update_status()
        if err ne adsec_error.ok then return, err
        if keyword_set(xadsec) then update_panels, /OPT, /NO_READ

        for i_step=1,n_step do begin

            temp_comm =i_step*c_step
            dummy = fltarr(adsec.n_actuators)
            dummy[cl_list] = temp_comm
            dummy = dummy[adsec.act_w_pos]
            
            err = get_commands(adsec.act_w_curr, cmdcurr, /CURR)
            if err ne adsec_error.ok then return,err
            
            err = set_offload_force(cmdcurr, /SWITCH )
            if err ne adsec_error.ok then return,err
                
            err = set_offload_cmd(dummy, /SWITCH, /NOCHECK, /START )
            if err ne adsec_error.ok then return,err
            wait,time_ps
                    
            err = apply_curr_opt(XADSEC=xadsec,N_ITER=1, /OFFLOAD)
            if err ne adsec_error.ok then return,err

            log_print,"command/position: "+strtrim(mean(sys_status.bias_command[adsec.act_w_cl]+dummy[adsec.act_w_cl]),2)+" " $
                  +strtrim(mean(sys_status.position[adsec.act_w_cl]), 2)
            if mean(sys_status.position[cl_list]) ge mean(final_pos[cl_list]) then break     
        endfor

        err = update_status()
        if err ne adsec_error.ok then return, err
        if keyword_set(xadsec) then update_panels, /OPT, /NO_R

;----------------------------------------------------------------------------
    ;LOW ORDER REMOVING

;    if adsec.n_crates eq 1 then begin
;        err = remove_tilt(cl_list, zern_idx=[2,3,5,6], XADSEC=xadsec, /OFFLOAD) ;;;;;; TO  DO FINISH ;;;;;
;        if err ne adsec_error.ok then return,err
;    endif else begin
;        err = remove_tilt(cl_list, zern_idx=[5,6,9,10], XADSEC=xadsec) ;;;;;; TO  DO FINISH ;;;;;
;        if err ne adsec_error.ok then return,err
;    endelse

;    wait, time_ps
    
;----------------------------------------------------------------------------
    ;PUT CURRENT AND COMMANDS INTO BIAS

    err = update_status()
    if err ne adsec_error.ok then return,err
    if keyword_set(xadsec) then update_panels, /OPT, /NO_READ

    log_print, "... setting the force/commands in the bias position and nulling control currents..."
    err = zero_ctrl_current(/OFFLOAD)
    if err ne adsec_error.ok then return,err
    
;----------------------------------------------------------------------------
    ;RAMP GAINS ON ACT_WO_CL_ON_SET

    if n_elements(final_crack_list) gt 0 then begin

        comm_crack = sys_status.position[final_crack_list]

        err = start_control(final_crack_list)
        if err ne adsec_error.ok then return,err
        wait, 2*rtr.oversampling_time >0.01
    
        err = update_status()
        if err ne adsec_error.ok then return,err
        if keyword_set(xadsec) then update_panels, /OPT, /NO_READ

        err = ramp_gain(final_crack_list, s_gain, MAX_AMP_GAIN=0.001, /NOCHECK)
        if err ne adsec_error.ok then return,err

        err = set_safe_skip(ACTLIST=final_crack_list)
        if err ne adsec_error.ok then return,err

        err = update_status()
        if err ne adsec_error.ok then return,err
        if keyword_set(xadsec) then update_panels, /OPT, /NO_READ

    endif
    
;----------------------------------------------------------------------------
    ;PUT CURRENT AND COMMANDS INTO BIAS

    err = zero_ctrl_current(/OFFLOAD)
    if err ne adsec_error.ok then return,err
    wait, 2*time_ps

    err = update_status()
    if err ne adsec_error.ok then return, err
    if xadsec then update_panels, /OPT, /NO_READ

    if (keyword_set(final_pr_cmd) and keyword_set(final_pr_curr)) then begin
            max_settling_time = max(step2time_preshaper([final_pr_cmd, final_pr_curr])) 
            acc_period = max_settling_time+0.1e-3 ; 
    endif else max_settling_time = max(step2time_preshaper([pr_cmd, pr_cur]))
     
    acc_delay = ceil(max_settling_time/adsec.sampling_time)
    acc_len   = long(acc_period/adsec.sampling_time-acc_delay)



    old_os = rtr.oversampling_time
    err = set_diagnostic(over=0.0)
    wait, (2*old_os) > 0.01
    ;err = update_status(1)
    ;if err ne adsec_error.ok then return, err

    log_print, "ACC_DELAY: "+ strtrim(acc_delay,2)+" ("+strtrim(acc_delay*adsec.sampling_time,2)+" s)"
    log_print, "ACC_LEN  : "+ strtrim(acc_len,2)+" ("+strtrim(acc_len*adsec.sampling_time,2)+" s)"
    log_print, "MAX FREQ ALLOWED: "+ strtrim( 1d/acc_period , 2)
    log_print, "MAX SETTLING TIME "+ strtrim(max_settling_time)+" sec"    

    err = set_preshaper(sc.all,final_pr_cmd)
    if err ne adsec_error.ok then return, err
    err = set_preshaper(sc.all, final_pr_curr, /FF)
    if err ne adsec_error.ok then return, err


    err = set_acc(acc_len,acc_delay)
    if err ne adsec_error.ok then return, err
    ;wait, 0.1
    err = start_acc(/WAIT)
    if err ne adsec_error.ok then return, err

    err = set_diagnostic(over=old_os)
    if err ne adsec_error.ok then return, err
    if xadsec then update_panels, /OPT, /NO_READ
    

;----------------------------------------------------------------------------
    ;RESTORE FULLY WORKING FAST DIAGNOSTIC THRESHOLDS

    ;for kk=0 , n_elements(adsec.act_w_pos)-1 do begin
    ;    err = setdiagnparam(process_list.fastdiagn.msgd_name,  'ChDistAverage', adsec.act_w_pos[kk], adsec.act_w_pos[kk] $
    ;                        , WARNING_MIN=old_warn_v[kk], TIM=1)
    ;     if changes ne 1 then return, adsec_error.generic_error 
    ;     changes = setdiagnparam(process_list.fastdiagn.msgd_name,  'ChDistAverage', adsec.act_w_pos[kk], adsec.act_w_pos[kk], ALARM_MIN=old_alarm_v[kk], TIM=1)
    ;     if changes ne 1 then return, adsec_error.generic_error 
    ; endfor

    ; changes = setdiagnparam(process_list.fastdiagn.msgd_name,  'ChDistAverage', -1, -1, ALARM_MIN=old_alarm_v[0], warning_min=old_warn_v[0], TIM=1)
     rip_mirror = 0
     return, adsec_error.ok
end
