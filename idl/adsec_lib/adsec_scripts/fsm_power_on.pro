;$Id $
;+
; NAME:
;   FSM_POWER_ON
;
; PURPOSE:
;   Function for the passage from the Power off state
;   to Power on state.
;   In the Power on state Adaptive Secondary Main Power Supplies (MPSs)
;   are switched on. The crates have power.
;
; CATEGORY:
;   Supervisor IDL function.
;
; CALLING SEQUENCE:
;   err = LBT_POWER_ON()
;
; INPUTS:
;   None.
;
; KEYWORD PARAMETERS:
;   AUTO            : if set no verbose is showed.
;-
Function fsm_power_on;, AUTO=auto


    @adsec_common
    if sc.name_comm eq "Dummy" then return, adsec_error.ok
    err_cnt  = 0
    sc.debug = 0
    

;    catch, Error_status
    Error_status=0

    ;This statement begins the error handler:  
    if Error_status ne 0 then begin

        err_cnt += 1
        if err_cnt gt 1 then begin
            log_print, "RECURSIVE ERROR."
            log_print, "PANIC: NO ACTION CAN BE DONE!!!!!!"
            return, !AO_ERROR
        endif
        err1 = !AO_ERROR

        log_print, !ERROR_STATE.MSG + " " +strtrim(!AO_ERROR, 2)

        err2 = adam_power_off()
        return, err1
        
    endif


    log_print, "fsm_power_on() command started."
    err = update_msg('Adsec '+!AO_CONST.unit+'-'+ !AO_CONST.telescope_side+' power on started.')
    if err ne adsec_error.ok then return, err

    err = update_progress(0)
    if err ne adsec_error.ok then return, err

    adam_startup = 1 

    err = init_adam(STARTUP=adam_startup, /KEEP_TSS)
    if err ne adsec_error.ok then log_message,"The ADAM connection cannot be established.", ERR=err
    
    ;===========================================================================
    err = adam_sys_reset()
    if err ne adsec_error.ok then log_message, "Error resetting BCU: power on not completed.", ERR=err
    
    err = update_progress(30)
    if err ne adsec_error.ok then return, err

;    config_diagnostics

;    max_cnt = 30
;    cnt = 0
;    while 1 do begin
;
;        err = getdiagnvalue('HOUSEKPR00', 'CheckDewPoint', -1, -1, bb)
;        if err ne adsec_error.ok then log_message,"Unable to get data from HOUSEKEEPER.", ERR=err
;        log_print, "CheckDewPoint :"+string(bb.last)
;        
;        if bb gt 5 then break 
;        if cnt gt max_cnt then log_message,"WARNING: critical dew point.", ERR=err
;        cnt +=1
;        wait, 2.
;
;    endwhile

    err = fsm_reset(PROGRESS=[30,99])
    if err ne adsec_error.ok then return, err

    err = set_fsm_state("PowerOn")
    if err ne adsec_error.ok then return, err

    err = update_msg('Adsec '+!AO_CONST.unit+'-'+ !AO_CONST.telescope_side+' power on done.')
    if err ne adsec_error.ok then return, err

    err = update_progress(100)
    if err ne adsec_error.ok then return, err
    log_print, "fsm_power_on() command done."
    return, adsec_error.ok
End
    
