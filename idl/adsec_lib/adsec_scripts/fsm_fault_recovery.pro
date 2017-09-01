;$Id: fsm_fault_recovery.pro,v 1.13 2009/12/21 10:55:08 marco Exp $$
;+
; NAME:
;   FSM_FAULT_RECOVERY
;
; PURPOSE:
;   Script to recover the Adaptive Secondary Mirror from a fault event.
; 
;
; CATEGORY:
;   Supervisor IDL function.
;
; CALLING SEQUENCE:
;   err = fsm_fault_recovery()
;
; INPUTS:
;   None.
;
; KEYWORD PARAMETERS:
;   none.
;
; OUTPUTS:
;   err : Error code.
;
; RESTRICTIONS:
;   None.
;
; MODIFICATION HISTORY:
;   13 Gen 2005
;    Created by Marco Xompero (MX)
;    marco@arcetri.astro.it
;   March 2006, Daniela Zanotti(DZ)
;     Added the update panels and disable coils.  
;-

Function fsm_fault_recovery, FORCE=force, AUTO=auto


    @adsec_common

    do_dump=0

    if sc.name_comm eq "Dummy" then return, adsec_error.ok
    if do_dump then begin
        filename = add_timestamp('$ADOPT_LOG/fault.sav')
        wait, 1
        err = set_diagnostic(over=0)
        err = load_diag_data(10000, drw,/readfc, /swi, /NOFAST)
        err = load_diag_data(10000, dr,/readfc, /NOFAST)
        save, file=filename, dr, drw 
    endif


    err = update_msg('Adsec '+!AO_CONST.unit+'-'+ !AO_CONST.telescope_side+' recovery from FAILURE started.')
    if err ne adsec_error.ok then return, err


    ;IDENTIFY MASTER DIAGNOSTIC TRACKING NUMBER
    names = file_search(getenv('ADOPT_LOG')+'/ADAPTIVE-SECONDARY_*')
    last = names[n_elements(names)-1]
    if last ne ""  or (file_info(last)).ctime gt (systime(/SEC)+600) then begin
        ;RETRIEVE THE TRACKING NUMBER STRINGS
        tn = stregex(last, '\_.*[0-9\_]', /ex)
    endif else begin
        tn = add_timestamp('', /ap)
    endelse

    log_print, "Get info from "+process_list.HOUSEKEEPER.MSGD_NAME+" variables..."
    err = getdiagnvalue(process_list.HOUSEKEEPER.MSGD_NAME , '*', -1,-1, hkpr_vars)
    if err ne adsec_error.ok then begin
        hkpr_vars = 'Undefined'
        log_print, "Unable to get variable values: skipped"
    endif else begin
        log_print, "Done"
    endelse

    log_print, "Get info from "+process_list.ADAMHOUSEKEEPER.MSGD_NAME+" variables..."
    err = getdiagnvalue(process_list.ADAMHOUSEKEEPER.MSGD_NAME , '*', -1,-1, adam_vars)
    if err ne adsec_error.ok then begin
        hkpr_vars = 'Undefined'
        log_print, "Unable to get variable values: skipped"
    endif else begin
        log_print, "Done"
    endelse
    
    log_print, "Get info from "+process_list.FASTDIAGN.MSGD_NAME+" variables..."
    err = getdiagnvalue(process_list.FASTDIAGN.MSGD_NAME , '*', -1,-1, fast_vars)
    if err ne adsec_error.ok then begin
        hkpr_vars = 'Undefined'
        log_print, "Unable to get variable values: skipped"
    endif else begin
        log_print, "Done"
    endelse

    log_print, "Try get status of global counters..."
    err = read_seq_dsp(sc.all, dsp_map.global_counter, 1L, bb, /ULONG)
    if err ne adsec_error.ok then begin
        adsec_vars = 'Undefined'
        log_print, "Unable to get variable values: skipped"
    endif else begin
        adsec_vars = {global_counter: bb}
        log_print, "Done"
    endelse
    
    dump_status = !AO_STATUS
    dump_error = !AO_ERROR
    filename = filepath(ROOT=getenv("ADOPT_LOG"), '')+'ASM_FAULT_DUMP'+tn+'.sav'
    log_print, "Saving debug data in "+filename+"..."
    save, file = filename, hkpr_vars, adam_vars, fast_vars, adsec_vars, dump_status, dump_error 
    if file_test(filename, /REGULAR) then log_print, "Done" else log_print, "Failed"

    log_print, "Disable coils for avoiding any dangerous reset."
    err = adam_disable_coils()
;    if err ne adsec_error.ok then begin


    err = setdiagnparam(process_list.HOUSEKEEPER.MSGD_NAME, 'checkdewpoint', 0, 0 $
                            , ALARM_MIN=0, WARNING_MIN=2, WARNING_MAX=1e6, ALARM_MAX=1e6)


    err = fsm_reset()
    if err ne adsec_error.ok then begin

        log_message, "Reset for fault recovery failed.", ERR=err, CONT=(sc.debug eq 0)
        return, !AO_ERROR

    endif
    
    ;VERY SMALL COMMUNICATION TEST AFTER THE RESET
    err = read_seq_dsp(sc.all, '80000'xl, 1L, bb, /ULONG)
    if err ne adsec_error.ok then begin
        log_message, "System DSP BOARDs not started up after reset. Try again.", ERR=err, CONT=(sc.debug eq 0)
        return, !AO_ERROR
    endif
    err = read_seq_dsp(sc.all, '80000'xl, 1L, bb, /BCU, /ULONG)
    if err ne adsec_error.ok then begin
        log_message, "System CRATE BCUs not started up after reset. Try again.", ERR=err, CONT=(sc.debug eq 0)
        return, !AO_ERROR
    endif
    err = read_seq_dsp(sc.all, '80000'xl, 1L, bb, /SWITCH, /ULONG)
    if err ne adsec_error.ok then begin
        log_message, "System SWITCH BCU not started up after reset. Try again.", ERR=err, CONT=(sc.debug eq 0)
        return, !AO_ERROR
    endif


    log_print, "Recovery process: Try RELOAD DSP PROGRAMS ..."
    err = fsm_load_program(/AUTO)
    if err ne adsec_error.ok then return, err
    log_print, "Done"


;    wait, 3
;    log_print, "RIP the shell"
;    err = fsm_rip()
;    if err ne adsec_error.ok then return, err
;    wait, 3
    
    err = update_msg('Adsec '+!AO_CONST.unit+'-'+ !AO_CONST.telescope_side+' recovery from FAILURE done.')
    if err ne adsec_error.ok then return, err

    log_print,"Fault recovery procedure done."
    !AO_STATUS.fsm_state = "Ready"
    return, update_rtdb()


end

