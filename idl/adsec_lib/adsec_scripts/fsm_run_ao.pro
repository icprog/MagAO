Function fsm_run_ao, freq, dec, ovs_p

    @adsec_common
    log_print, "fsm_run_ao() start"
    if n_elements(ovs_p) eq 0 then ovs_p = 0.02
    if ovs_p gt 0.03    then return, adsec_error.input_type
    
    if rtr.oversampling_time eq 0 then begin
        log_message, "NO POSSIBLE TO RUN AO: AdSec unprotected.", ERR=adsec_error.IDL_OVS_NOT_RUNNING, CONT=(sc.debug eq 0)
        return, !AO_ERROR
    endif

; --
; 2KHz settings
; AP 20161116 commented out waiting for tests
;
    if freq gt 1100 then begin
        log_print, "Preparing for 2KHz operation"
        err = prepare4twok()
        if err ne adsec_error.ok then return, err
    endif
;
; --

    if ovs_p ne rtr.oversampling_time then begin
        err = update_ovs(ovs_p, dec=dec)
        if err ne adsec_error.ok then return, err
    endif

    err = disable_ho_offload()
    if err ne adsec_error.ok then return, err
    wait, 1

;    !AO_STATUS.fsm_state = "AORunning"
    log_print, "fsm_run_ao() end"
    return, set_fsm_state('AORunning')

End
