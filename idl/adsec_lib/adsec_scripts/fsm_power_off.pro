;$Id $
;+
; NAME:
;   FSM_POWER_OFF
;
; PURPOSE:
;   Function for the passage from the Power on state
;   to Power off state.
;   
;
; CATEGORY:
;   Supervisor IDL function.
;
; CALLING SEQUENCE:
;   err = LBT_POWER_OFF()
;
; INPUTS:
;   None.
;
; KEYWORD PARAMETERS:
;   AUTO            : if set no verbose is showed.
;-
Function fsm_power_off, AUTO=auto

    @adsec_common
    log_print, 'fsm_power_off() procedure started'
    err = update_msg('Adsec '+!AO_CONST.unit+'-'+ !AO_CONST.telescope_side+' power off started.')
    if err ne adsec_error.ok then return, err 
    if n_elements(progress) eq 0 then progress=[0,100]
    
    cpr = 0
    err = update_progress(cpr)
    if err ne adsec_error.ok then return, err


    verbose = ~keyword_set(auto)
    ;adsec_state.state_current = adsec_state.power_off
    ;!AO_STATUS.fsm_state = "PowerOff"
    err = write_scalar_var_wrap(strupcase(strmid(!AO_CONST.telescope_side,0,1))+'.ADSEC.OVS_P', 0L, 1001, 0L)
    if err ne adsec_error.ok then return, err

    ;IF WIND ENABLE TSS ELSE DISABLE TSS
    err = adam_disable_coils()
    if err ne adsec_error.ok then return, err
    
    err = adam_power_off()
    if err ne adsec_error.ok then return, err

    err = update_msg('Adsec '+!AO_CONST.unit+'-'+ !AO_CONST.telescope_side+' power off done.')
    if err ne adsec_error.ok then return, err 
    if n_elements(progress) eq 0 then progress=[0,100]
    
    cpr = 100
    err = update_progress(cpr)
    if err ne adsec_error.ok then return, err

    log_print, 'fsm_power_off() procedure done'

    
    return, set_fsm_state("PowerOff")
    
End
