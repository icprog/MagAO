;$Id: ff_data_acquisition.pro,v 1.19 2009/05/22 16:32:55 labot Exp $
;+
;   NAME:
;    FF_DATA_ACQUISITION
;
;   PURPOSE:
;    The routine performs the data acquisition for the feed forward matrix
;    estimation.
;
;   USAGE:
;    err = ff_data_acquisition, nmeas, amp, samples, data, MODAL=modal, FF=ff, CL_ACT=cl_act $
;    , INTEGRATOR=integ, N_ITER_INTEGRATOR=iter, SMOOTH=do_smooth, MAX_PEAK_FORCE=max_peak_force
;
;   INPUT:
;    nmeas:     number of measurements to do. (positive and negative excitation)
;    amp:       amplitude of excitation(FLOAT).
;    samples:   number of samples to get (UNSIGNED LONG).
;
;   OUTPUT:
;    Error code.
;
;   KEYWORD:
;    MODAL:         use the modal estmation of the feed forward matrix (the default is zonal).
;    FF:            use the feed forward contribution of a precalculated matrix.
;    CL_ACT:        close loop actuators. (defaul adsec.act_w_cl)
;    INTEGRATOR:    perfoms the use of the integrator. (don't use with FF enabled.)
;    N_ITER_INTEGRATOR: number of iteration of the integrator. 
;    SMOOTH:        smoothed visializzation of the data retrieved.
;    MAX_PEAK_FORCE: set a new max peak force value.
;
;   NOTE:
;   A) Integrator parameter iteration not explicity set. 
;    The dafault value in integrator function is four, to be tested with the experience.
;   B) Data ordering:
;      reference, +amp(ch/mode_1), -amp(ch/mode_1), +amp(ch/mode_2), ... , -amp(ch/mode_n_meas), reference
;
;   
; HISTORY
;   Creted by Armando Riccardi (AR) on 2003 and completely rewritten to match LBT
;   specifications.
;
;   26 Jul 2004 Marco Xompero (MX)
;       Error catching remanaging. n_iter_integrator keyword passed without modifications. 
;       adsec.weight_times4peak, dsp_const.max_preshaper_step variable used. Managed data structure
;       for the new float dsp data management and corrected reading routines passing data. Added
;       CHECK keyword on all writings. Help written.
;   27 Jul 2004 MX
;       Added upper bound on max_peak_force. Fixed writing error on float.
;   04 Aug 2004 MX
;       max_preshaper_step changed in slowest_preshaper_step.
;   05 Aug 2004 AR and MX
;       used get and set_preshaper.
;       t_wait is computed considering also the setting of the preshaper.
;       other minor changes.
;   02 Nov 2004, MX
;       Adsec.all changed in sc.all.
;   08 Feb 2005, MX
;       Deleted accumulator setting before acuisition in ff mode. t_wait variable definition fixed.
;       Filters resetting added in the clear_ff call.
;   21 Jun 2005, MX
;       Added an update_status at the end of the procedure.
;   16 Nov 2005, D.Zanotti (DZ)
;       Deleted the last set_delta_position after the last acquisition,
;       because it reapplied the last mode.  
;   07 Feb 2007 MX
;       LUT of variable deleted and name refurbished.
;   08 Feb 2007 MX
;       Dummy mode compatible.
;-

; $Id: ff_data_acquisition.pro,v 1.19 2009/05/22 16:32:55 labot Exp $

function ff_data_acquisition, nmeas, amp, samples, data, MODAL=modal, FF=ff, CL_ACT=cl_act $
    , INTEGRATOR=integ, N_ITER_INTEGRATOR=iter, SMOOTH=do_smooth, MAX_PEAK_FORCE=max_peak_force; $
    ;, LIN_CK=lin_ck

@adsec_common

;checks and setting on default parameters
if n_elements(max_peak_force) eq 0 then max_peak_force=adsec.weight_curr * adsec.weight_times4peak else max_peak_force = max_peak_force < (adsec.weight_curr * adsec.weight_times4peak)

if n_elements(cl_act) eq 0 then cl_act=adsec.act_w_cl

n_cl_act = n_elements(cl_act)
if nmeas lt n_cl_act then begin
    message, "WARNING: you set less measures then number of actuators in closed loop", /INFO
endif

;if keyword_set(ff) then begin
;    delay = 500
;
;    err = set_acc(samples, delay)
;    if err ne adsec_error.ok then begin
;        message, "Error setting the accumulators.", CONT= (sc.debug eq 0B)
;        return, err
;    endif
;endif

data_proto={command: fltarr(adsec.n_actuators), $
            position: fltarr(adsec.n_actuators), $
            min_pos:  fltarr(adsec.n_actuators), $
            max_pos:  fltarr(adsec.n_actuators), $
            ctrl_current: fltarr(adsec.n_actuators), $
            bias_current: fltarr(adsec.n_actuators), $
            ff_current: fltarr(adsec.n_actuators)}

data=replicate(data_proto, 2*nmeas+2)

; reads the current values for the preshaper and sets the longest preshaping
err = get_preshaper(sc.all_actuators, old_preshaper_dec)
if err ne adsec_error.ok then return, err
err = get_preshaper(sc.all_actuators, old_preshaper_dec_ff, /FF)
if err ne adsec_error.ok then return, err

preshaper_dec=dsp_const.slowest_preshaper_step
preshaper_dec_ff=dsp_const.slowest_preshaper_step
err = set_preshaper(sc.all_actuators, preshaper_dec)
if err ne adsec_error.ok then return, err
err = set_preshaper(sc.all_actuators, preshaper_dec_ff, /FF)
if err ne adsec_error.ok then return, err

;t_wait = ((samples+500)*adsec.sampling_time*adsec.time_secure_factor) > 0.01
;t_wait = t_wait > (time_preshaper()*adsec.time_secure_factor)
if sc.name_comm eq "Dummy" then t_wait = 0.01 else $
    t_wait = (time_preshaper()*adsec.time_secure_factor) > 0.01 ;;;; DANGER!!!! t_wait COULD BE INFINITY!!!!


;clearing the feed forward currents and disable the ave ctrl curr integration
err=clear_ff(/RESET_FILTER)
if err ne adsec_error.ok then begin
    message, "Error clearing feed-forward currents!!",CONT= (sc.debug eq 0B)
    return, err
endif

err = update_status(samples)
if err ne adsec_error.ok then begin
    message, "Error updating status!!",CONT= (sc.debug eq 0B)
    return, err
endif


data[0].command=sys_status.command
data[0].position=sys_status.position
data[0].min_pos=sys_status.min_pos
data[0].max_pos=sys_status.max_pos
data[0].ctrl_current=sys_status.ctrl_current
data[0].bias_current=sys_status.bias_current
data[0].ff_current=sys_status.ff_current

command = sys_status.command
sign = [1, -1]

;cicle of measurements
for i=0, nmeas-1 do begin
    i_cl_act = i mod n_cl_act
    i_act = cl_act[i_cl_act]

	prev_command = command

;positive and negative excitation
    for j=0,1 do begin

        dummy_command=command
;modal or zonal excitation
        if keyword_set(modal) then begin
            ;i_cl_act=1
            ; use only the first n_cl_act mode
            peak_force = max(abs(adsec.ff_f_svec[i_cl_act,*]*adsec.ff_sval[i_cl_act]*amp))
            new_amp = (amp*float(max_peak_force)/peak_force) < amp
            print, "Peak force:",peak_force/amp*new_amp
            dummy_command=float(dummy_command+sign[j]*new_amp*adsec.ff_p_svec[i_cl_act, *])
        endif else begin
            dummy_command[i_act] = dummy_command[i_act] + sign[j]*amp
        endelse

;ff settings, if it exists.
        if keyword_set(ff) then begin

            err=set_delta_position_vector(dummy_command-prev_command)
            if err ne adsec_error.ok then begin
                message, "Error setting the delta position vector.",CONT= (sc.debug eq 0B)
                return, err
            endif

            err=start_ff()
            if err ne adsec_error.ok then begin
                message, "Error starting feed-forward contribution in excitation.",CONT= (sc.debug eq 0B)
                return, err
            endif
            wait, t_wait

        endif else begin

            err=set_position_vector(dummy_command, /check)
            if err ne adsec_error.ok then begin
                message, "Error setting the delta position vector.",CONT= (sc.debug eq 0B)
                return, err
            endif

            err=update_command()
            if err ne adsec_error.ok then begin
                message, "Error updating command in the excitation.",CONT= (sc.debug eq 0B)
                return, err
            endif

	        wait,t_wait
	        if keyword_set(integ) then begin
	            thr=max(sys_status.max_pos-sys_status.min_pos)
	            err=integrator(cl_act, THR=thr, PERR=perr, ITER=iter, PAMP=pamp)
	            if err ne adsec_error.ok then begin
                    message, "Error in the integration in the excitation.",CONT= (sc.debug eq 0B)
                    return, err
                endif
		        wait,0.01
	        endif

        endelse

        err = update_status(samples)
        if err ne adsec_error.ok then begin
            message, "Error updating the status.",CONT= (sc.debug eq 0B)
            return, err
        endif


        idx = 2*i+j+1
        data[idx].command=sys_status.command
        data[idx].position=sys_status.position
        data[idx].min_pos=sys_status.min_pos
        data[idx].max_pos=sys_status.max_pos
        data[idx].ctrl_current=sys_status.ctrl_current
        data[idx].bias_current=sys_status.bias_current
        data[idx].ff_current=sys_status.ff_current

        prev_command = dummy_command
    endfor


    if keyword_set(ff) then begin
        err=set_delta_position_vector((command-prev_command), /check)
        if err ne adsec_error.ok then begin
            message, "Error setting the delta position vector.",CONT= (sc.debug eq 0B)
            return, err
        endif

        err=start_ff()
        if err ne adsec_error.ok then begin
            message, "Error starting the feed-forward!!",CONT= (sc.debug eq 0B)
            return, err
        endif

        wait, t_wait
	    err = clear_ff()
		if err ne adsec_error.ok then begin
            message, "Error clearing the feed-forward contribution.",CONT= (sc.debug eq 0B)
            return, err
        endif

        wait,t_wait
    endif


    if keyword_set(modal) then begin
        print, 'Iteration:', strtrim(i,2), "  amp:", strtrim(sign[1]*new_amp), "  mode#",strtrim(i_cl_act,2)
    	display, (reform(data[idx].position-data[0].position))[cl_act], cl_act, SMOOTH=do_smooth, /SHOWBAR $
          , /NO_NUM, TITLE="mode# "+strtrim(i_cl_act)
      ; if keyword_set(lin_ck) then begin
;              window, 4
;       	display, (reform(data[idx].position+data[idx-1].position)-2*data[0].position)[cl_act], cl_act, SMOOTH=do_smooth, /SHOWBAR $
;          , /NO_NUM, TITLE="linearity cmd mode# "+strtrim(i_cl_act)
;       endif

    endif else begin
        print, 'Iteration:', strtrim(i,2), "  amp:", strtrim(sign[1]*amp), "  act#",strtrim(i_act,2)
        display, (reform(data[idx].position-data[0].position))[cl_act], cl_act, SMOOTH=do_smooth, /SHOWBAR $
              , /NO_NUM, TITLE="act#"+strtrim(i_act)+" ("+sc.act_name[i_act,2]+")" ;, MIN_V=-amp, MAX_V=amp
        ;display, (reform(data[idx].position-data[idx-1].position))[cl_act], cl_act, SMOOTH=do_smooth, /SHOWBAR $
        ;      , /NO_NUM, TITLE="act#"+strtrim(i_act)+" ("+sc.act_name[i_act,2]+")" ;, MIN_V=-amp, MAX_V=amp
    
        ;if keyword_set(lin_ck) then begin
;            window, 4
;            display, (reform(data[idx].position+data[idx-1].position)-2*data[0].position)[cl_act], cl_act, SMOOTH=do_smooth, /SHOWBAR $
;              , /NO_NUM, TITLE="linearity cmd act#"+strtrim(i_act)+" ("+sc.act_name[i_act,2]+")" ;, MIN_V=-amp, MAX_V=amp	
;       endif 
   endelse
    
    

    temp=(reform(sys_status.max_pos-sys_status.min_pos))[cl_act]
    max_temp=max(temp, idx_temp)
    print, 'Max MINMAX: ', strtrim(max_temp,2), " @ act#", strtrim(cl_act[idx_temp])
    temp=(reform(data[idx].position-command))[cl_act]
    max_temp=max(temp, idx_temp)
    print, 'Max CTRL error: ', strtrim(max_temp,2), " @ act#", strtrim(cl_act[idx_temp])
    min_temp=min(temp, idx_temp)
    print, 'Min CTRL error: ', strtrim(min_temp,2), " @ act#", strtrim(cl_act[idx_temp])
    temp=abs((reform(data[idx].ctrl_current+data[idx].bias_current+data[idx].ff_current))[cl_act])
    max_temp=max(temp, idx_temp)
    print, 'Max abs(current): ', strtrim(max_temp,2), " @ act#", strtrim(cl_act[idx_temp])
endfor



if not keyword_set(ff) then begin

    err=set_position_vector(command, fltarr(1,adsec.n_actuators),/check)
    if err ne adsec_error.ok then begin
        message, "Error setting null command vector!!",CONT= (sc.debug eq 0B)
        return, err
    endif

    err=update_command()
    if err ne adsec_error.ok then begin
        message, "Error updating the commands.",CONT= (sc.debug eq 0B)
        return, err
    endif

endif


err = update_status(samples)
if err ne adsec_error.ok then begin
    message, "Error updating the status.",CONT= (sc.debug eq 0B)
    return, err
endif


idx = 2*nmeas+1
data[idx].command=sys_status.command
data[idx].position=sys_status.position
data[idx].min_pos=sys_status.min_pos
data[idx].max_pos=sys_status.max_pos
data[idx].ctrl_current=sys_status.ctrl_current
data[idx].bias_current=sys_status.bias_current
data[idx].ff_current=sys_status.ff_current

;restoring initial conditions and clearing the ff matrix
err=write_bias_curr(sc.all_actuators,data[0].bias_current, /CHECK)
if err ne adsec_error.ok then begin
    message, "Error restoring the old bias current.",CONT= (sc.debug eq 0B)
    return, err
endif

err=clear_ff()
if err ne adsec_error.ok then begin
    message, "Error clearing the feed-forward contribution.",CONT= (sc.debug eq 0B)
    return, err
endif

err = write_seq_ch(sc.all_actuators, dsp_map.step_ptr_preshaper_cmd, old_preshaper_dec, /CHECK)
if err ne adsec_error.ok then begin
    message, "Error restoring old preshaper command step.",CONT= (sc.debug eq 0B)
    return, err
endif

err = write_seq_ch(sc.all_actuators, dsp_map.step_ptr_preshaper_curr, old_preshaper_dec_ff, /CHECK)
if err ne adsec_error.ok then begin
    message, "Error restoring old preshaper current step.",CONT= (sc.debug eq 0B)
    return, err
endif

err = update_status(samples)
if err ne adsec_error.ok then begin
    message, "Error updating the status.",CONT= (sc.debug eq 0B)
    return, err
endif

return,adsec_error.ok
end
