;$Id: fsm_load_program.pro,v 1.11 2009/12/21 10:55:08 marco Exp $
;+
; NAME:
;   FSM_LOAD_PROGRAM
;
; PURPOSE:
;   Function for the passage from the Power-on state to the Operating state.    
;   In the Operating state the LBT672 crates have power.The BCU, SIGGEN, DSP 
;   boards and Power Backplane firmware has correctly bootstrapped. Housekeeping and Fast
;   diagnostic are running. The shell is pushed against the reference plate by bias magnets.
;
;   Events in Load_program  procedure:
;       -   Communications with DSP is tested;
;       -   DSP program is uploaded;
;       -   DSP memories are initialized;
;       -   The default configuration is loaded;   
;       -   DSP program is started;
;       -   Communications with DSP is tested again;
;       -   Adaptive Secondary Mirror (ASM) fast diagnostic(FST) is started;
;       -   The functionality of coils, current drives and capacitive sensor is checked;
;       -   Dust contamination is tested;
;       
;
; CATEGORY:
;   Supervisor IDL function.
;
; CALLING SEQUENCE:
;   err = FSM_LOAD_PROGRAM()
;
; INPUTS:
;   None.
;
; KEYWORD PARAMETERS:
;   AUTO            : if set the load program  is  done  automatically, 
;
;
;
; OUTPUTS:
;   err : Error code.
;
; RESTRICTIONS:
;   None.
;
; MODIFICATION HISTORY:
;   Created by D.Zanotti(DZ) on January 2005
;   Added a new capacitive sensor test.
;   25 Sep 2008 Marco Xompero
;   Rewrite for final unit.
;   
;
;-

function fsm_load_program, AUTO=auto, MOUNT_SHELL=mount_shell, PROGRESS=progress
    @adsec_common
    if sc.name_comm eq "Dummy" then return, adsec_error.ok
    ;sc.debug = 0
    verbose = ~keyword_set(auto)
    no_ask = Keyword_set(auto)
    if n_elements(progress) eq 0 then progress=[0,100]
    gr.num_type = -1
;xadsec
;============================================================================================================  
;Test the communications with DSP, upload DSP program, initialize DSP memories,
;load the default configuration, start DSP program,Test again communications with DSP.

;if keyword_set(verbose) then 
    log_print, 'FSM Load Program procedure start...'
    err = update_msg('Adsec '+!AO_CONST.unit+'-'+ !AO_CONST.telescope_side+' uploading DSP code.')
    if err ne adsec_error.ok then return, err
    cpr = progress[0]+fix(0.*(progress[1]-progress[0]))
    err = update_progress(cpr)
    if err ne adsec_error.ok then return, err


    for i=0,9 do begin

        ;ITERATION FOR PROGRAM LOADING
        log_print, "Try to load program...: iteration "+string(i, format='(I2.2)')
        err = proc_startup(NO_ASK=no_ask, PROGRESS=[progress[0]+1, progress[1]-10])
        if err ne adsec_error.ok then begin
            ;err1 = fsm_reset(PROGRESS=[progress[0],progress[0]+1])
            err1 = fsm_reset()
        endif else begin
            break
        endelse
    
    endfor
    if err ne adsec_error.ok then return, err


;if keyword_set(verbose) then 

;============================================================================================================  
;!!!!!!!!!!!!!!!!!!!!!!INIZIO PROCEDURE DA PENSARE E DA FARE!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! 
;============================================================================================================  
;The Fast Diagnostic and Housekeeping must be running
; NOW ENABLE FAST DIAGNOSTIC AT THE END OF DOWNLOAD PROGRAM
;============================================================================================================  

;============================================================================================================  

;Check  the functionality of coils, current drives and capacitive sensor.
;if keyword_set(verbose) then log_print, 'Initialiaze to check  the coils, current drives and capacitive sensor...'
; err = elec_function_test(NO_ASK=no_ask)
;        if err ne adsec_error.ok then return, err
;if keyword_set(verbose) then log_print,' done.' 
;============================================================================================================  


;Check  the dust contamination.
; if keyword_set(verbose) then print, 'Check no dust contamination...'
; err = find_dust(status_dust, act_dust, VERBOSE = verbose)
;        if err ne adsec_error.ok then return, err
 
 ;if status_dust then begin
 ;   message, 'The dust was found near the actuators: ', strtrim(act_dust,2), CONT=(sc.debug eq 0)
 ;   return, adsec.dust_contamination
 ;endif    
;if keyword_set(verbose) then print,' done.' 
;============================================================================================================  

;============================================================================================================  
;============================================================================================================
; procedure to test the capacitive sensor 

; if keyword_set(verbose) then print, format='("The capacitive sensor test  procedure...", $)'

 ;err = test_sensor(VERBOSE=verbose)
 ;if err ne adsec_error.ok then return, err


    err = update_status()
    if err ne adsec_error.ok then return, err
;============================================================================================================  
    if  xregistered('xadsec') gt 0 then  begin
        update_panels, /OPT, /NO_READ
    endif

    err = update_msg('Adsec '+!AO_CONST.unit+'-'+ !AO_CONST.telescope_side+' DSP code running.')
    if err ne adsec_error.ok then return, err

    if ~keyword_set(mount_shell) then begin
        
        cpr = progress[0]+fix(float(progress[1]-10)/100*(progress[1]-progress[0]))
        err = update_progress(cpr)
        if err ne adsec_error.ok then return, err
        wait, 5
        err  = fsm_rip(PROGRESS=[progress[1]-10, progress[1]])
        if err ne adsec_error.ok then return, err
    endif
    err = update_msg('Adsec '+!AO_CONST.unit+'-'+ !AO_CONST.telescope_side+' now properly working.')
    if err ne adsec_error.ok then return, err


;   log_print, 'Selecting default focal station: FrontBentGregorian'
;   err = select_wfs_port(1)
;   if err ne adsec_error.ok then return, err
 
    log_print,' Storing configuration changing if any...' 
    err = store_conf()
    if err ne adsec_error.ok then return, err

    log_print,' Load Program procedure finished' 


;============================================================================================================  
; update state current
;adsec_state.state_current = adsec_state.operating 
;============================================================================================================  
    
    return, set_fsm_state('Ready')
    


end
