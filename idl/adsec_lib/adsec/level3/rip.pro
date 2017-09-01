; $Id: rip.pro,v 1.18 2009/11/24 16:57:09 marco Exp $
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
function rip, XADSEC=xadsec, WIND_BIAS_CURR = wind_bias_curr

    @adsec_common


    ;;; trucco per disabilitare le slope che vengono dal sensore
    ;;; mentre rippo modifica lo start_rtr pointer e la punto su una
    ;;; zona di memoria nulla e non usata (switch_map.null_value)


    if n_elements(wind_bias_curr) eq 0 then begin
        wind_bias_curr = fltarr(adsec.n_actuators)
        wind_bias_curr[adsec.act_w_curr]=0.00
    endif
    if mean(wind_bias_curr[adsec.act_w_curr]) lt 0 then begin
        message,"PISTON REQUESTED!! not done.", /INFO
        return, adsec_error.input_type
        
    endif
    time_ps = 0.1

    if keyword_set(XADSEC) then begin
        answer = dialog_message("Do you want to put the mirror in the rest state?" $
                                , /QUESTION)
    endif else answer="yes"
    
    ;step_ps = round(adsec.preshaper_len/time_ps*adsec.sampling_time)
    ;err = set_preshaper(sc.all_actuators, step_ps)		
    ;if err ne adsec_error.ok then return,err
    ;err = set_preshaper( sc.all_actuators, step_ps, /FF)
    ;if err ne adsec_error.ok then return,err

    if strlowcase(answer) eq "yes" then begin

        err = get_master_diagn_freq(ENABLE=enable_fast) 
        if err ne adsec_error.ok then return, err
        old_ovs = rtr.oversampling_time
        if enable_fast ne 0 then begin
            old_ovs = rtr.oversampling_time
            err = set_diagnostic(OVER=0.0)
            if err ne adsec_error.ok then return, err
            err = set_disturb(/disable)
            if err ne adsec_error.ok then return, err
            err = set_diagnostic(MASTER=0)
            if err ne adsec_error.ok then return, err
            
        endif
        step_ps = round(adsec.preshaper_len/time_ps*adsec.sampling_time)
        err = set_preshaper(sc.all_actuators, step_ps)		
        if err ne adsec_error.ok then return,err
        err = set_preshaper( sc.all_actuators, step_ps, /FF)
        if err ne adsec_error.ok then return,err
        
        ;step_ps = round(adsec.preshaper_len/time_ps*adsec.sampling_time)
        ;err = set_preshaper(sc.all_actuators, step_ps)		
        ;if err ne adsec_error.ok then return,err
        ;err = set_preshaper( sc.all_actuators, step_ps, /FF)
        ;if err ne adsec_error.ok then return,err

        wait, old_ovs > 0.01
        
        err = set_curr2bias()
        if err ne adsec_error.ok then return, err

        ;time_ps = time_preshaper(/FF)
        
        err = update_status()
        if err ne adsec_error.ok then return, err
        if keyword_set(XADSEC) then update_panels, /OPT, /NO_READ
        bias_curr0 = sys_status.bias_current
        

        if keyword_set(XADSEC) then  disp_mess, "Stopping the control for all the actruators...", /APPEND
        err = stop_control(sc.all_actuators)
        if (err ne adsec_error.ok) then return, err
        if keyword_set(XADSEC) then disp_mess, "... done.", /APPEND
        
        if keyword_set(XADSEC) then disp_mess, "Clearing the feedforward currents...", /APPEND
        err = clear_ff()
        if err ne adsec_error.ok then return, err
        if keyword_set(XADSEC) then disp_mess, "... done.", /APPEND
        
        if keyword_set(XADSEC) then disp_mess, "Ramping the bias current to wind protection value...", /APPEND
        ;if total(abs(bias_curr0)) ne 0.0 then begin
        ;    start_curr = adsec.secure_factor*adsec.weight_curr $
        ;                 + adsec.curr4bias_mag*adsec.weight_curr
        ;    nstep = 100
        ;    tot_time = 2.0      ; [s]
        ;    curr_fact = (1-(findgen(nstep+1)/nstep)^(2))
        ;                        ;curr_step = start_curr/nstep
        ;    for i=0, nstep do begin
        ;        err = write_bias_curr(sc.all_actuators, start_curr*curr_fact[i])
        ;        if err ne adsec_error.ok then return, err
        ;        wait, (tot_time/nstep) < 0.01
        ;    endfor
        ;endif
    
        err = clear_dacs()
        if err ne adsec_error.ok then return, err
        err = clear_commands()
        if err ne adsec_error.ok then return, err
        wait, time_ps

        err = write_bias_curr(sc.all_actuators,wind_bias_curr )
        if err ne adsec_error.ok then return, err
        wait, time_ps

        err = set_gain(sc.all, 0.0)
        if err ne adsec_error.ok then return, err
        
        err = set_gain(sc.all, 0.0, /SPEED)
        if err ne adsec_error.ok then return, err

;        err = set_offload_cmd(/reset, /NOCH)
;        if err ne adsec_error.ok then return, err
;        err = set_offload_force(/reset, /NOCH)
;        if err ne adsec_error.ok then return, err

;        err = set_cmd2bias(/FULL)
;        if err ne adsec_error.ok then return, err

;        err = write_bias_cmd(fltarr(adsec.n_actuators))
;         if err ne adsec_error.ok then return, err


    ;CLEAR DISTURBANCE
    err = set_disturb(fltarr(rtr.disturb_len, adsec.n_actuators))
    if err ne adsec_error.ok then return, err

    err = set_disturb(/DIS, /RESET, /COUNT)
    if err ne adsec_error.ok then return, err




        wait, time_ps
        err = update_status()
        if err ne adsec_error.ok then return, err
        if keyword_set(XADSEC) then update_panels, /OPT, /NO_READ
        ; return value for future implementations
        ;err = set_diagnostic(over=0.05)
        ;if err ne adsec_error.ok then return, err
        ;wait, 0.1
        ;err = set_diagnostic(over=0.0)

        alarm_v = 2e-6
        changes = setdiagnparam(process_list.fastdiagn.msgd_name, 'ChDistAverage', -1, -1, ALARM_MIN=alarm_v, warning_min=alarm_v, TIM=1)
      
        err = set_diagnostic(/master, over=old_ps)
        if err ne adsec_error.ok then return, err
;        err = adam_disable_coils()
;        if err ne adsec_error.ok then return, err

        return, adsec_error.ok


    endif else return, adsec_error.ok
    
end
