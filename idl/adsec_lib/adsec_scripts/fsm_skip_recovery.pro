Function recovery_w_null_slopes

    @adsec_common
    log_print, "Null Gain application and start sending null slope..."
    

    err = set_g_gain(sc.all, fltarr(adsec.n_actuators))
    if err ne adsec_error.ok then return, err
    err = set_g_gain(sc.all, fltarr(adsec.n_actuators), /BLOCK)
    if err ne adsec_error.ok then return, err
    slopes = fltarr(rtr.n_slope)


    err = read_seq_dsp(0, rtr_map.param_selector, 1L, pbs, /UL)
    if err ne adsec_error.ok then return, err

    ;Mantain current offload block, current RTR block selection
    ;Set the FL enabling and the SDRAM storage
    offload_block = pbs and 2L^13
    rtr_block = pbs and 1L
    sto_block = pbs and 2L^14
    new_pbs = 'C0'xl or offload_block or rtr_block or sto_block

    old_ovs = rtr.oversampling_time
    err = set_diagnostic(OVER=0.0, TIMEOUT=2000)
    if err ne adsec_error.ok then return, err
    wait, 0.1
    err = wfs_switch_step(slopes, PBS=new_pbs)
    if err ne adsec_error.ok then return, err
    err = set_diagnostic(OVER=old_ovs)
    if err ne adsec_error.ok then return, err
    log_print, "Cleaned last slopes command."
    return, adsec_error.ok
 
End


Function fsm_skip_recovery, FREEZE=freeze, FORCE=force, NOLOADSHAPE=noloadshape


    @adsec_common
    sc.debug=0B
    shape_name = !AO_STATUS.shape
    
    if keyword_set(FREEZE) then log_print, "fsm_skip_recovery FREEZE set"
    if keyword_set(FORCE) then log_print, "fsm_skip_recovery FORCE set"
    if keyword_set(NOLOADSHAPE) then log_print, "fsm_skip_recovery NOLOADSHAPE set"


    ;TEST on disturbance status
    err = read_seq_dsp(0, rtr_map.param_selector, 1L, bb, /UL)
    if err ne adsec_error.ok then return, err
;    if (bb && 2L^12) or (bb && 2L^15) gt 0 then begin
;    if ((bb and 2L^12) gt 0) or ((bb and 2L^15) gt 0) then begin
;        log_message, "The disturbance command application is still enabled. Please stop it and retry.", CONT = (sc.debug eq 0), err=adsec_error.IDL_DIST_ACTIVE
;        return, !AO_ERROR
;    endif


    ;;TEST FOR NO SC ACTIVITIES IN PROGRESS
    err = test_skip_frame(/NOVERB, WFS_CNT = cnt0, PENDING=psf0, SAFE=ssf0)
    if err ne adsec_error.ok then return, err
    wait, 0.2
    err = test_skip_frame(bit, /DSP, /NOVERB, WFS_CNT = cnt1, PENDING=psf1, SAFE=ssf1  )
    if err ne adsec_error.ok then return, err

    if (cnt1 - cnt0) ne 0 then begin
        ;log_message, "Slope Computer is now routing slopes do the Adaptive Secondary. Please stop it and retry.", CONT = (sc.debug eq 0), err=adsec_error.sc_sending_slopes
        log_message, "Slope Computer is now routing slopes do the Adaptive Secondary. Please stop it and retry.", CONT = (sc.debug eq 0), err=adsec_error.IDL_SC_SENDING_SLOPES
        return, !AO_ERROR
    endif

    msf = bit[0,*]; 1=skipped because beyond modal threshold
    csf = bit[1,*]; 1=skipped because beyond position command threshold
    fsf = bit[2,*]; 1=skipped because beyond force command threshold
;;
    skip_detected = ~(total(msf) eq 0 && total(csf) eq 0 && total(fsf) eq 0)
    
    if ~keyword_set(FORCE) then begin
        if (total(msf) eq 0 && total(csf) eq 0 && total(fsf) eq 0) then begin
            log_print, "No skipping frames. Recovering from SafeSkipFrames not needed."
            return, set_fsm_state("AOSet")
        endif
    endif

    err =  recovery_w_null_slopes()
    if err ne adsec_error.ok then return, err
    
    log_print, "Modes out of safe range    ID:"+ strjoin(string(where(msf, cm), format='(" #", I3)'))
    log_print, "Commands out of safe range ID:"+ strjoin(string(where(csf, cc), format='(" #", I3)'))
    log_print, "Forces out of safe range   ID:"+ strjoin(string(where(fsf, cf), format='(" #", I3)'))
    if cm gt 0 then begin
        log_print, "SafeSkipFrames caused by modes out of range detected."
    endif
    if cc gt 0 then begin
        log_print, "SafeSkipFrames caused by commands out of range detected."
    endif
    if cf gt 0 then begin
        log_print, "SafeSkipFrames caused by forces out of range detected."
    endif

    log_print, "Recovery from any SafeSkipFrames started..."

    old_ovs = rtr.oversampling_time

    log_print, "Getting info on actual mirror position..."
    if err ne adsec_error.ok then return, err


    err = read_seq_dsp(0, rtr_map.modes_vector, long(adsec.n_actuators), modes1)
    if err ne adsec_error.ok then log_message, "Error applying low level settings CMD #c", ERR=err
    err = get_commands(sc.all, ol, /OFFLOAD)
    if err ne adsec_error.ok then log_message, "Error applying low level settings CMD #d", ERR=err
    current_m2c_mat = *rtr.m2c_matrix
    cmd  = (current_m2c_mat ## transpose(modes1) )
;    cmd = cmd/1.1
    curr = float(adsec.ff_matrix ## cmd)
    offload_cmd  = float(ol+cmd) 

;    err = update_status()
;    if err ne adsec_error.ok then return, err
;    err = get_commands(sc.all, ol, /OFFLOAD)
;    if err ne adsec_error.ok then log_message, "Error applying low level settings CMD #d", ERR=err
;    offload_cmd  = float(sys_status.position-sys_status.command+ol) 

    log_print, "Reconstructor filter cleaning..."
    err = set_diagnostic(OVER=0.0, TIMEOUT=5000.)
    if err ne adsec_error.ok then return, err
    log_print, "Control current integrator cleaning up..."
    err = set_fc_gain(/RESET, /ALL)
    if err ne adsec_error.ok then return, err
    err = clear_dl()
    if err ne adsec_error.ok then return, err

    if keyword_set(FREEZE) and ~(skip_detected) then begin
;    if keyword_set(FREEZE) then begin
        log_print, "Move reconstructor commands in offload commands"
        err = set_offload_cmd(offload_cmd[adsec.act_w_pos], /NOCHECK, /SW, /START)
        err = set_offload_cmd(offload_cmd[adsec.act_w_pos], /NOCHECK, /SW)
        if err ne adsec_error.ok then log_message, "Error applying low level settings CMD #q", ERR=err
        log_print, "Done."
    endif else begin

        ;RESTORE ORIGINAL OFFLOAD COMMANDS
        err = set_offload_cmd(sys_status.flat_offload_cmd[adsec.act_w_pos], /NOCHECK, /SW)
        if err ne adsec_error.ok then return, err

        err = set_offload_force(sys_status.flat_offload_curr[adsec.act_w_curr], /NOCHECK, /START, /SW)
        if err ne adsec_error.ok then return, err

    endelse


    log_print, "Null new delta command contribution"
    err = write_same_ch(sc.all, dsp_map.ff_pure_current, 0L, /CHECK)
    if err ne adsec_error.ok then return, err
    log_print, "Null new delta command contribution"
    err = write_same_ch(sc.all, dsp_map.new_delta_command, 0L, /CHECK)
    if err ne adsec_error.ok then return, err
    log_print, "Restore old oversampling..."
    err = set_diagnostic(OVER=old_ovs)
    if err ne adsec_error.ok then return, err
    log_print, "Done."
    err = wait_until_event(/MIRR)
    if err ne adsec_error.ok then return, err
 
    log_print, "Restore control current integrator action...."
    err = update_ovs(old_ovs)
    if err ne adsec_error.ok then return, err
    log_print, "Done."
     
;    if ~keyword_set(noloadshape) or (skip_detected) then begin
    if ~keyword_set(noloadshape) then begin
        log_print, "Reload last shape selected"
        err = fsm_load_shape(shape_name,/NOSKIPRECOVERY)
        if err ne adsec_error.ok then return, err
        log_print, "Done."
    endif
        
;    !AO_STATUS.b0_a = ""
;    !AO_STATUS.b0_b=""
;    !AO_STATUS.a_delay=""
;    !AO_STATUS.b_delay_a=""
;    !AO_STATUS.b_delay_b=""
;    !AO_STATUS.m2c=""
    !AO_STATUS.g_gain_a=""
    !AO_STATUS.g_gain_b=""
;    !AO_STATUS.disturb=""
    err = update_rtdb()
    if err ne adsec_error.ok then return, err
    log_print, "All Done. Succefully recovered from SafeSkipFrames."
    
    return, set_fsm_state("AOSet")

End
