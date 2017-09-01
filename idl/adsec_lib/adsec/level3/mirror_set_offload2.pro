;$Id: mirror_set_offload2.pro,v 1.5 2009/11/24 16:57:08 marco Exp $
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

function mirror_set_offload2, FINAL_POS=final_pos , F_GAIN=f_gain, S_GAIN=s_gain                       $
                   , MAX_CURR=max_curr, MAX_AMP_POS=max_amp_pos, CL_INIT_DCMD=cl_init_dcmd             $
                   , CTRL_CURR_INT = ctrl_curr_int, OVERSAMPLING_TIME=ovs                              $
                   , SHELL_FLOATING_POS=shell_floating_pos, SHELL_SET_POS=shell_set_pos                $
                   , SKIP_ASK4CL=skip_ask4cl, F_CMD_PRESH = f_cmd_presh, F_FF_PRESH = f_ff_presh       $
                   , FINAL_PR_CMD = final_pr_cmd, FINAL_PR_CURR = final_pr_curr, RIP_MIRROR=rip_mirror $
                   , ENABLE_COILS = enable_coils, PROGRESS=progress

;----------------------------------------------------------------------------
    ;COMMON BLOCKS

    @adsec_common
    common temporary_block, comm0, c_step, n_step, cl_list
    xadsec = (xregistered('xadsec') gt 0)
    rip_mirror = 1
    if n_elements(progress) eq 0 then progress=[0,100]

    
;----------------------------------------------------------------------------
    ;ERROR HANDLING
;    catch, error_status
;    IF Error_status NE 0 THEN BEGIN 
;        err = adam_disable_coils(/TSS)
;        if err ne adsec_error.ok then begin
;            print, "PANIC: not able to disable coils and activate TSS"
;        endif 
;        PRINT, 'Error index: ', Error_status 
;        PRINT, 'Error message: ', !ERROR_STATE.MSG 
;        idl_error_status=1
;        catch, /CANCEL
;        return, adsec_error.generic_error
;    endif




;----------------------------------------------------------------------------
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
        log_message,' Final positions are not defined.', CONT=(sc.debug eq 0), ERR=adsec_error.input_type
        return, adsec_error.input_type
    endif

    if (total(final_pos lt min(adsec.min_pos_val[adsec.true_act]) or final_pos gt 130e-6) gt 0) then begin
        log_message,' Final positions requested out of ranges.', CONT=(sc.debug eq 0), ERR=adsec_error.input_type
        return, adsec_error.input_type
    endif

    if n_elements(s_gain) eq 0 then begin 
        log_message,'Start gain is not defined.', CONT=(sc.debug eq 0), ERR=adsec_error.input_type
        return, adsec_error.input_type
    endif

    if (s_gain gt 0.03) then begin
        log_message,'Start gain is too large.', CONT=(sc.debug eq 0), ERR=adsec_error.input_type
        return, adsec_error.input_type
    endif

    if n_elements(max_amp_pos) eq 0 then max_amp_pos = adsec.max_amp_pos ;340nm max-min pos allowed in close_loop() routine
    if max_amp_pos gt 5e-6 then begin
        log_message,'Amplitude requested for pistoning is too large..', CONT=(sc.debug eq 0), ERR=adsec_error.input_type
        return, adsec_error.input_type
    endif

    
    if n_elements(max_curr) eq 0 then max_curr = 2*adsec.weight_curr ; max current for integrator() e close_loop() routines
    if n_elements(CL_INIT_DCMD) eq 0 then begin
        if !AO_CONST.unit eq '585m' then cl_init_dcmd=6e-6 else cl_init_dcmd = 12e-6
    endif
    
    if n_elements(CL_INIT_DCMD) eq adsec.n_actuators then cl_init_dcmd = cl_init_dcmd[cl_list]
    if n_elements(S_GAIN) eq 0 then s_gain = min(adsec.weight_curr/2.*0.25/cl_init_dcmd /1e6 )
    ;[N/um] ; forza magneti bias/4 (fattore sicurezza)/delta_posizione ;
    
    ;TEST on the TSTART_GAIN compatibility with FORCE SATURATION
    new_force = cl_init_dcmd*s_gain*mean(adsec.gain_to_bbgain[cl_list])
    cond_sat = total((new_force lt adsec.neg_sat_force/2.) or (new_force gt adsec.pos_sat_force/2.)) gt 0
    cond_max_curr = total(abs(new_force) gt adsec.max_curr) gt 0
    if cond_sat or cond_max_curr then begin    
        log_message,'Start gain and initial_delta_pos are not compatible for isostatic configuration.', CONT=(sc.debug eq 0), ERR=adsec_error.input_type
        return, adsec_error.input_type
    endif
    
    
;----------------------------------------------------------------------------
;-------------------- MIRROR SETUP ------------------------------------------   
;----------------------------------------------------------------------------

;----------------------------------------------------------------------------
    ;FORCE DISABLING TIMEOUT FRAMES DIAGNOSTIC
    err = update_msg('Adsec '+!AO_CONST.unit+'-'+ !AO_CONST.telescope_side+' exiting from blind zone.')
    if err ne adsec_error.ok then return, err
    cpr = progress[0]
    err = update_progress(cpr)
    if err ne adsec_error.ok then return, err



    err = set_diagnostic(over=0.0)
    if err ne adsec_error.ok then return,err

;----------------------------------------------------------------------------
    ;CLEANING CURRENTS AND COMMANDS

    err = clear_dacs(sc.all)
    if err ne adsec_error.ok then return, err

    err = clear_commands(sc.all)
    if err ne adsec_error.ok then return, err


;----------------------------------------------------------------------------    
    ;PRESHAPERS SETTING

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
    ;NULL RECONSTRUCTOR SETTINGS

    ;set up the modes/commands/forces thresholds
    err = set_safe_skip(ACTLIST=adsec.act_w_cl, MODLIST=indgen(n_elements(adsec.act_w_cl)))
    if err ne adsec_error.ok then return, err
  
    ;set up the dummy recostruction/delay/m2c matrices
    b0 = fltarr(rtr.n_slope,adsec.n_actuators)
    b_delay = fltarr(rtr.n_slope,adsec.n_actuators, rtr.n_slope_delay)       ;(1 delay slopes)
    a_delay = fltarr(adsec.n_actuators,adsec.n_actuators,rtr.n_modes_delay)  ;(2 delay modes)
    new_m2c_mat = fltarr(rtr.n_modes2correct, adsec.n_actuators)
    err = set_m2c_matrix(new_m2c_mat)
    if err ne adsec_error.ok then log_message, "Error encountered: "+strtrim(err,2) , ERR=err

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

;    if keyword_set(CTRL_CURR_INT) then begin
;        err = write_des_pos(adsec.act_w_pos,  shell_set_pos[adsec.act_w_pos])
;        if err ne adsec_error.ok then return,err
;    endif else begin
    err = write_des_pos(cl_list, sys_status.position[cl_list]+cl_init_dcmd)
    if err ne adsec_error.ok then return,err
;    endelse

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

    err =  getdiagnparam(process_list.fastdiagn.msgd_name, 'ChDistAverage', -1, -1, actual)
    if err ne adsec_error.ok then return, err
    alarm_v = sys_status.position
    warn_v = sys_status.position + 5e-6
    set_alarm_v = min(sys_status.position)-5e-6
    set_warn_v  = min(sys_status.position)-5e-6
    act_ranges = list2ranges(adsec.act_w_pos)
    srange = size(act_ranges, /DIM)
    if n_elements(srange) eq 1 then range_n = 1 else range_n = srange[1]
    for kk=0 , range_n-1 do begin
        changes = setdiagnparam(process_list.fastdiagn.msgd_name, 'ChDistAverage', act_ranges[0,kk], act_ranges[1,kk], ALARM_MIN=set_alarm_v, WARNING_MIN=set_warn_v, TIM=1)
        if changes le 0 then return, changes
    endfor
    log_print, "Fastdiagnostic process distance threshold changed for initial shell setting."


;----------------------------------------------------------------------------
    ;ENABLE MASTER DIAGNOSTIC

    err = set_disturb(/RESET, /DISABLE)
    if err ne adsec_error.ok then return, err

    err = set_diagnostic(/MASTER)
    if err ne adsec_error.ok then return, err

    err = set_diagnostic(OVER=ovs)
    if err ne adsec_error.ok then return, err

    wait, 2*rtr.oversampling_time > 0.1

;----------------------------------------------------------------------------
    ;ENABLE COILS DIAGNOSTIC

    if keyword_set(ENABLE_COILS) then begin

        err = adam_update()
        if err ne adsec_error.ok then return, err
        tss_n_status = adam_out.tss_disable

        err = adam_enable_coils_dis_tss()
        if err ne adsec_error.ok then return, err

        for i=0, 5 do begin
            log_print, "Coil testing after TSS disabling..."
            wait, 1
            err = test_coils()
            if err eq adsec_error.ok then begin
                log_print, "Coil successfully enabled."
                break
            endif
            
        endfor
        if err ne adsec_error.ok then return, break
        

        log_print, "Restoring TSS previous status...."
        if tss_n_status then err = adam_disable_tss() else err=adam_enable_tss()
        if err ne adsec_error.ok then return, err
        log_print, "Coil test after the TSS status restoring..."
        err = test_coils()
        if err ne adsec_error.ok then begin
            log_print, "Coil are not enabled."
            return, err
        endif
        
;        err = adam_disable_tss()
;        if err ne adsec_error.ok then return, err
    endif

    err = update_status()
    if err ne adsec_error.ok then return, err
    if keyword_set(xadsec) then update_panels, /OPT, /NO_READ

    pos0=sys_status.position[cl_list]
    gain = fltarr(n_elements(cl_list))
    counter=1
    max_count = 30

;-----------------------------------------------------------------------------
    ;RAMPING CL_LIST GAIN

    err = ramp_gain(cl_list, s_gain, /NOCH)
    if err ne adsec_error.ok then return,err

    err = update_status()
    if err ne adsec_error.ok then return, err
    if keyword_set(XADSEC) then update_panels, /NO_READ

;-----------------------------------------------------------------------------
    ;START TO PUSH THE MIRROR MANTAINING FORCE/MOMENT
    
    comp_value = shell_floating_pos
    ocnt = 0

    repeat begin

        err = apply_curr_opt(XADSEC=xadsec,N_ITER=1, /OFFLOAD)
        if err ne adsec_error.ok then return,err

        wait, 2*fast_period 
        err = update_status()
        if err ne adsec_error.ok then return,err
        if keyword_set(xadsec) then update_panels, /OPT, /NO_R
        
        log_print, "av. current    : "+strtrim(mean(sys_status.current[act_w_curr]), 2)
        log_print, "ctrl currents  : "+strtrim(mean(sys_status.ctrl_current[cl_list]), 2)
        log_print, "ff currents    : "+strtrim(mean(sys_status.ff_current[cl_list]), 2)
        log_print, "mean position  : "+strtrim(mean(sys_status.position[cl_list]), 2)
        log_print, "mean req pos   : "+strtrim(mean(comp_value[adsec.act_w_pos]),2)
        counter = counter+1

        coil_disabled = test_coils()
        
        if coil_disabled then begin
            log_print, "Unexpected status of coils: disabled"
            return, adsec_error.IDL_UNEXP_COIL_STATUS
        endif
        
        print, mean(sys_status.position[adsec.act_w_pos]-comp_value[adsec.act_w_pos])
        print, (mean(sys_status.position[adsec.act_w_pos]) - mean(comp_value[adsec.act_w_pos]))
        print, (median(sys_status.position[adsec.act_w_pos]) - median(comp_value[adsec.act_w_pos]))
        print,  mean(comp_value[adsec.act_w_pos]),minmax(comp_value[adsec.act_w_pos])
        print,  median(comp_value[adsec.act_w_pos])

        ocnt +=1

    endrep until (mean(sys_status.position[adsec.act_w_pos]) ge mean(comp_value[adsec.act_w_pos]))  or $
                 (counter ge max_count) 


    if counter ge max_count then begin
        log_message, "The requested gain has not been applied. Too many iterations.", CONT=sc.debug eq 0B, ERR=adsec_error.IDL_RAMP_GAIN_ERROR
        return, adsec_error.IDL_RAMP_GAIN_ERROR
    endif
 
;----------------------------------------------------------------------------
    ;CHECK IF AUTOMATICALLY CONTINUE

    err = update_status()
    if err ne adsec_error.ok then return, err

    if mean(sys_status.position[adsec.act_w_pos]) gt 60e-6 then begin
        log_print, "WRONG SHELL POSITION! AUTOMATICAL RIPPING"
        return, adsec_error.IDL_SAFE_PARAMS_OOB
    endif
    err = update_msg('Adsec '+!AO_CONST.unit+'-'+ !AO_CONST.telescope_side+' pistoning to working gap.')
    if err ne adsec_error.ok then return, err
    cpr =  progress[0]+fix(0.5*(progress[1]-progress[0]))
    err = update_progress(cpr)
    if err ne adsec_error.ok then return, err



;----------------------------------------------------------------------------
    ;NULLING CONTROL CURRENT, RAMP THE PROPORTIONAL GAIN ENOUGH TO PERMIT THE WIND REJECTION

    wait, 2*rtr.oversampling_time > 0.1

    err = update_status()
    if err ne adsec_error.ok then return, err

    err = zero_ctrl_current2()
    if err ne adsec_error.ok then return, err
    err = ramp_gain(sc.all, 0.03, /NOCH)
    if err ne adsec_error.ok then return, err
;    err = ramp_gain_script60()
;    if err ne adsec_error.ok then return, err

;----------------------------------------------------------------------------
    ;ACTIVATE THE CONTROL CURRENT INTEGRATOR

    err = update_ovs(rtr.oversampling_time)
    if err ne adsec_error.ok then return, err

;----------------------------------------------------------------------------
    ;RESTORE WORK SET FAST DIAGNOSTIC THRESHOLDS

;    for kk=0 , n_elements(adsec.act_w_pos)-1 do begin
;        changes = setdiagnparam('FASTDGN00', 'ChDistAverage', adsec.act_w_pos[kk], adsec.act_w_pos[kk] $
;                            , WARNING_MIN=warn_v[kk], ALARM_MIN=alarm_v[kk])
;        if changes ne 1 then return, adsec_error.changes 
;    endfor
    log_print, "Fastdiagnostic process distance threshold set to working standard settings."

    if ~keyword_set(SKIP_ASK4CL) then begin
        answ = dialog_message(["The mirror is in "+strtrim(mean(sys_status.position[adsec.act_w_pos]),2 ), $
                                   "Do you want to continue to the final position?"], /QUEST)

        
        if strlowcase(answ) eq "no" then begin
            
            
            return, adsec_error.ok
            
        endif 
    endif

   
;----------------------------------------------------------------------------
    ;PISTON APPLICATION

    wait, 2*rtr.oversampling_time > 0.1
    err = zero_ctrl_current2()
    if err ne adsec_error.ok then return, err

    err = update_status()
    if err ne adsec_error.ok then return, err
    if keyword_set(xadsec) then update_panels, /OPT, /NO_READ

    err = piston_shell(final_pos, max_amp_pos, XADSEC=xadsec)
    if err ne adsec_error.ok then return, err

;----------------------------------------------------------------------------
    ;RAMP GAINS ON ACT_WO_CL_ON_SET

    if n_elements(final_crack_list) gt 0 then begin

	print, "Ramp gain on slot"
        comm_crack = sys_status.position[final_crack_list]

;;;;; ==== start ARMA
print, zero_ctrl_current2()
print, zero_ctrl_current2()
err = set_gain(final_crack_list,0.0)
if err ne adsec_error.ok then return,err
;;;;; ==== end ARMA

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
	print, "End ramp gain on slot"

    endif

;in case of issues start comment from here
        
    err = zero_ctrl_current(/OFFLOAD)
    if err ne adsec_error.ok then return,err
    wait, 2*time_ps
    
    err = update_status()
    if err ne adsec_error.ok then return, err

    if (keyword_set(final_pr_cmd) and keyword_set(final_pr_curr)) then begin
            max_settling_time = max(step2time_preshaper([final_pr_cmd, final_pr_curr])) 
            acc_period = max_settling_time+0.1e-3 ; 
    endif else max_settling_time = max(step2time_preshaper([pr_cmd, pr_cur]))

;    max_settling_time=0.7e-3
;    acc_period = max_settling_time+0.1e-3 ; 
    acc_delay = ceil(max_settling_time/adsec.sampling_time)
    acc_len   = long(acc_period/adsec.sampling_time-acc_delay)

    old_os = rtr.oversampling_time
    err = set_diagnostic(over=0.0, MASTER=0)
    wait, (2*old_os) > 0.01
    ;err = update_status(1)
    ;if err ne adsec_error.ok then return, err
    
    log_print, "ACC_DELAY: "+ strtrim(acc_delay,2)+" ("+strtrim(acc_delay*adsec.sampling_time,2)+" s)"
    log_print, "ACC_LEN  : "+ strtrim(acc_len,2)+" ("+strtrim(acc_len*adsec.sampling_time,2)+" s)"
    log_print, "MAX FREQ ALLOWED: "+ strtrim( 1d/acc_period , 2)
    log_print, "MAX SETTLING Time "+ strtrim(max_settling_time)+" sec"

    err = set_preshaper(sc.all,final_pr_cmd)
    if err ne adsec_error.ok then return, err
    err = set_preshaper(sc.all, final_pr_curr, /FF)
    if err ne adsec_error.ok then return, err


    err = set_acc(acc_len,acc_delay)
    if err ne adsec_error.ok then return, err
    ;wait, 0.1
    err = start_acc(/WAIT)
    if err ne adsec_error.ok then return, err

    err = wfs_switch_step()
    if err ne adsec_error.ok then return, err


    err = set_diagnostic(over=old_os, MASTER=1)
    if err ne adsec_error.ok then return, err
;end comments

    cpr =  progress[1]
    err = update_progress(cpr)
    if err ne adsec_error.ok then return, err


;----------------------------------------------------------------------------
     rip_mirror = 0
     return, adsec_error.ok
end
