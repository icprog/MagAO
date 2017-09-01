; $Id: fsm_rip.pro,v 1.8 2009/12/21 10:55:08 marco Exp $
;+
; NAME:
;   RIP
;
; PURPOSE:
;   To put the mirror in 'REST IN PEACE' state.
;
; CATEGORY:
;   Adsec Hardware Function, Level 3
;
; CALLING SEQUENCE:
;
;   Err = RIP()
;
;
; OUTPUTS:
;   Error code (see INIT_ERROR_CODES.pro)
;
; COMMON BLOCKS:
;   Adsec std common blocks (see ADSEC_COMMON.pro )
;
; EXAMPLE:
;
;   To put the mirror in the REST IN PEACE state just write:
;
;     err = rip()
;
; HISTORY:
;   Created by Armando Riccardi (AR) on ??/??/2003
;   04 Feb 2004   AR
;     Bias magnets supported.
;   17 Feb 2004   Marco Xompero (MX)
;     New Secure_factor adsec variable used.
;   28 May 2004,AR
;     Modifications to match LBT formats
;   02 Nov 2004, MX
;     Adsec.all changed in sc.all.
;   14 Mar 2007, MX
;     Oversampling disable before RIP added
;   13 Jul 2007, MX
;     Wait time fixed using preshaper.
;-
function fsm_rip, XADSEC=xadsec, WIND_BIAS_CURR = wind_bias_curr, PROGRESS=progress

    @adsec_common
    if sc.name_comm eq "Dummy" then return, adsec_error.ok
    if n_elements(progress) eq 0 then progress=[0,100]


    ;;; trucco per disabilitare le slope che vengono dal sensore
    ;;; mentre rippo modifica lo start_rtr pointer e la punto su una
    ;;; zona di memoria nulla e non usata (switch_map.null_value)
    
    err = update_msg('Adsec '+!AO_CONST.unit+'-'+ !AO_CONST.telescope_side+' going SAFE and clean up.')
    if err ne adsec_error.ok then return, err
    cpr = progress[0]+fix(0.*(progress[1]-progress[0]))
    err = update_progress(cpr)
    if err ne adsec_error.ok then return, err

log_print, 'Here, i should have disabled the WFS port.......'
;    err = select_wfs_port(0)
;    if err ne adsec_error.ok then return, err
    

    ;ERROR HANDLING
    log_print, "Installing error handling code"
    error_status=0
    ;catch, error_status
    IF Error_status NE 0 THEN BEGIN 
        log_print, "Error occurred in RIP() procedure, attempt to disable coils and exiting..."
        err = adam_disable_coils()
        idl_error_status=1
        return, err
    endif

    log_print, "Reset optical loop gain to zero"
    err = set_g_gain(sc.all, 0.0)
    if err ne adsec_error.ok then return, err

    act_i = -1
;    changes = setdiagnparam('FASTDGN00', 'ChDistAverage', -1, -1, ALARM_MIN=2e-6)
;    changes = setdiagnparam('FASTDGN00', 'ChCurrAverage', -1, -1, ALARM_MAX=1.0, ALARM_MIN=-1.0)
;    changes = setdiagnparam('FASTDGN00', 'ChIntControlCurrent', -1, -1, ALARM_MIN=-2.0, ALARM_MAX=2.0)

    log_print, "Disable actuators coils with ADAM"
    err = adam_disable_coils()
    if err ne adsec_error.ok then return, err
    log_print, "Coils disabled. Cleaning currents"

    err = rip()
    if err ne adsec_error.ok then return, err
    
;    for act_i=0, adsec.n_actuators-1 do begin
;        changes=setdiagnparam(diag_name,"ChIntControlCurrent", act_i, act_i, $
;                              ALARM_MIN=-0.69, WARNING_MIN=-0.3, WARNING_MAX=0.3, ALARM_MAX=0.69 $
;                              , MEAN_PERIOD=0, CONS_ALL=0)    
;    
;        changes=setdiagnparam(diag_name,"ChDistAverage", act_i, act_i, $
;                              ALARM_MIN=adsec.min_lin_dist[act_i], WARNING_MIN=adsec.min_lin_dist[act_i]+1e-6 $
;                              , WARNING_MAX=125e-6, ALARM_MAX=135e-6 $
;                              , MEAN_PERIOD=0, CONS_ALL=0)
;        changes=setdiagnparam(diag_name,"ChCurrAverage", act_i, act_i, $
;                              ALARM_MIN=-0.79, WARNING_MIN=-0.7, WARNING_MAX=0.75, ALARM_MAX=0.79 $
;                              , MEAN_PERIOD=0, CONS_ALL=0)
;    endfor
;    !AO_STATUS.fsm_state = "Ready"
    cpr = progress[0]+fix(0.25*(progress[1]-progress[0]))
    err = update_progress(cpr)
    if err ne adsec_error.ok then return, err
    
    log_print, "Recontructor cleaning started..."
    err = clear_dl()
    if err ne adsec_error.ok then return, err
    err = clear_ff()
    if err ne adsec_error.ok then return, err
    err = clear_rtr()
    if err ne adsec_error.ok then return, err
    log_print, "Recontructor cleaning done."
    
    cpr = progress[0]+fix(0.50*(progress[1]-progress[0]))
    err = update_progress(cpr)
    if err ne adsec_error.ok then return, err

    log_print, "Start sending diagnostic frames... "
    err = set_safe_skip(MINCMD=0., MAXCMD=120e-6)
    if err ne adsec_error.ok then return, err
    err = wfs_switch_step(PBS='c0'xl)
    if err ne adsec_error.ok then return, err

    err = set_diagnostic(OVER=0.02, /MASTER)
    if err ne adsec_error.ok then return, err
    log_print, "Done"

    err = disable_ho_offload()
    if err ne adsec_error.ok then return, err

    err = set_disturb(/disable)
    if err ne adsec_error.ok then return, err

    err = set_if_trigger(0,0)
    if err ne adsec_error.ok then return, err
    log_print, 'IF Trigger successfully set'    
    err = set_accelerometer(/STOP_ACC)
    if err ne adsec_error.ok then return, err
    log_print, 'Accelerometers successfully set'
    cpr = progress[0]+fix(0.75*(progress[1]-progress[0]))
    err = update_progress(cpr)
    if err ne adsec_error.ok then return, err
    log_print, 'Update progress successfully completed (2)'

;;
; here threshold increasing for all DISTRMS
    log_print, "Change RMS thresholds to avoid spurious diagnostic dumps when the shell is REST."
        changes = setdiagnparam(process_list.fastdiagn.msgd_name,  'ChDistRms', -1, -1, WARNING_MAX=5e-6, ALARM_MAX=15e-6, TIM=1)
    if changes gt 0 then log_print, "RMS thresholds successfully changed." else log_print, "RMS thresholds not successfully changed. Continue anyway."
;;
    ;PATCH requested by RB, 16/11/2011
    filename=filepath(ROOT=adsec_path.commons, sub=['ref_wave'], 'wave_1o8.txt')
    err = update_wave(filename)
    if err ne adsec_error.ok then return, err
    log_print, 'Reference wave decreased'
    ;END PATCH requested by RB, 16/11/2011

    cpr = progress[1]
    err = update_progress(cpr)
    if err ne adsec_error.ok then return, err
    log_print, 'Update progress successfully completed (2)'

    err = update_msg('Adsec '+!AO_CONST.unit+'-'+ !AO_CONST.telescope_side+' SAFE position reached.')
    if err ne adsec_error.ok then return, err

    err = short_dust_test()
    if err ne adsec_error.ok then return, err
    log_print, 'Dust test successfully completed'

    log_print, "FSM Rip done."

    return, set_fsm_state("Ready")
    
end
