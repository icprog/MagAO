Function fsm_stop_ao, HOLD=hold

    @adsec_common

    log_print, "fsm_stop_ao start"
    if keyword_set(HOLD) then begin
        freeze=1
        noloadshape=1
    endif
    err = fsm_skip_recovery(/FORCE, FREEZE=freeze, NOLOADSHAPE=noloadshape)
    if err ne adsec_error.ok then return, err

; --
; 2KHz settings
;
; AP 20161116 commented out waiting for tests
;
    log_print, "Restoring 1KHz configuration"
    err = restore2onek()
    if err ne adsec_error.ok then return, err
;
; --


    ;err = fsm_skip_recovery()
;    !AO_STATUS.fsm_state = "AOSet"
    
;    err = enable_ho_offload();   mod 20100514 LBTO
;    if err ne adsec_error.ok then return, err

    wait, 5*rtr.oversampling_time > 0.2
    log_print, "fsm_stop_ao end"
    return, err

End
