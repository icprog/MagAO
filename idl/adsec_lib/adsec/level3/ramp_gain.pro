; $Id: ramp_gain.pro,v 1.15 2009/11/24 16:57:08 marco Exp $
;+
; NAME:
;   RAMP_GAIN
;
; PURPOSE:
;	 This .pro provides to raise the proportional or speed gain control by steps.
;
;
; CATEGORY:
;   Adsec Hardware Function, Level 3
;
; CALLING SEQUENCE:
;		err=ramp_gain(	act_list, final_gain, MAX_AMP_GAIN=max_amp_gain, $
;						MAX_AMP_POS=max_amp_pos, MAX_CURR=max_curr, SPEED_LOOP_GAIN=speed )
; INPUTS:
;		act_list:	integer vector. Actuators list for which the gain is wanted to be raised.	
;		final_gain:	float [N/um] proportional or speed [Ns/m] final gain.
;
; OUTPUTS:
;   Error code (see INIT_ERROR_CODES.pro)
;
; KEYWORDS:
;   MAX_AMP_GAIN:   max gain step to apply
;   MAX_AMP_POS:    set a value for the max amplitude of position to apply.
;   MAX_CURR:    set a value to max_current to apply.
;	SPEED_LOOP_GAIN: If it is set, the gain speed is raised.
;   NOCHECK:         skip the check on RMS position (to be used only if FastDiagnostic is activated)
;
; COMMON BLOCKS:
;   Adsec std common blocks (see ADSEC_COMMON.pro )
;
; HISTORY
;
;   20 Feb 2004: A. Riccardi (AR) & M. Xompero (MX)
;                Fixed minor problems.
;	26 Jul 2004: AR & D.Zanotti (DZ)
;				 Added the MAX_CURR keyword	
;				 Changed gain data type for LBT compatibility.
;				 Added adsec.max_amp_pos_ramp,max_deltacurr,max_gain_step parameter.
;   12 Aug 2004, AR
;     More changes for LBT compatibility
;   02 Nov 2004, MX
;     Adsec.all changed in sc.all.
;   13 Jul 2007, MX
;     MAX_AMP_GAIN keyword added.
;   19 Sep 2007, MX
;     Zenith angle used for thresholding.
;   1 Aug 2008, MX
;     Added fast diagnostic startup in case of error.
;-
function ramp_gain, act_list, final_gain, MAX_AMP_POS=max_amp_pos, MAX_CURR=max_curr, SPEED_LOOP_GAIN=speed, MAX_AMP_GAIN=max_amp_gain, NOCHECK=nocheck

	@adsec_common
        
	if n_elements(max_curr) eq 0 then max_curr = (-mean(adsec.curr4bias_mag[adsec.true_act])-cos((90-sys_status.elevation_angle)/180.*!pi)+1.5)* adsec.weight_curr;max_curr = 2*adsec.weight_curr
	if n_elements(max_amp_pos) ne 0 then begin
 		if test_type(max_amp_pos,/real,N_EL=n_el) then begin
			log_message,"MAX_amp_pos must be real", CONT=(sc.debug eq 0)
            return, adsec_error.input_type
        endif
 		if n_el ne 1 then begin
            log_message,"Max_amp_pos must be scalar", CONT=(sc.debug eq 0)
            return, adsec_error.input_type
        endif
    endif else begin
        max_amp_pos = adsec.max_amp_pos_ramp
    endelse
	max_amp_curr = adsec.max_delta_curr ;1000

    if act_list[0] eq sc.all_actuators then the_act_list=indgen(adsec.n_actuators) else the_act_list = act_list

	if n_elements(final_gain) eq 1 then the_final_gain=replicate(final_gain, n_elements(the_act_list)) $
	else if n_elements(final_gain) ne n_elements(the_act_list) then begin
		log_message, "Gain and actuator list must have the same size", CONT=(sc.debug eq 0)
        return, adsec_error.input_type    
	endif

;	if min(final_gain) lt 0 then begin
;		message, "Gain cannot be negative"
;	endif

	err = get_gain(the_act_list, gain0, SPEED=speed)
	if err ne adsec_error.ok then return, err

	dgain = final_gain - gain0
	n_step_def = round((max(dgain) > 0)/(adsec.max_gain_step/mean(adsec.gain_to_bbgain[the_act_list]))) > 1

    if n_elements(max_amp_gain) gt 0 then begin

        tmp_step = round((max(dgain) > 0)/(max_amp_gain)) > 1
        n_step = max([n_step_def, tmp_step])

    endif else begin
        
        n_step = n_step_def
        
    endelse


	min_gain = gain0 < final_gain
	max_gain = gain0 > final_gain


 
    err = get_master_diagn_freq(ENABLE=enable_fast) 
    if err ne adsec_error.ok then return, err

    if ~keyword_set(NOCHECK) && enable_fast then begin
;            message,"You cannot check the position RMS using diagnostic buffer with Master Diagnostic and Oversampling frames enabled.", CONT=(sc.debug eq 0)
        old_ovs = rtr.oversampling_time
;        if enable_fast ne 0 then begin
        err = set_diagnostic(OVER=0.0)
        if err ne adsec_error.ok then return, err
        err = set_diagnostic(MASTER=0.0)
        if err ne adsec_error.ok then return, err
 ;       endif
        wait, 2*old_ovs

    endif


	for i=1,n_step do begin

		coil_disabled = test_coils()
        if coil_disabled then begin
            log_print, "Unexpected status of coils: disabled"
            return, adsec_error.IDL_UNEXP_COIL_STATUS
        endif
        new_gain = ((gain0+float(dgain)/n_step*i) < max_gain) > min_gain

		err = set_gain(the_act_list, new_gain, SPEED=speed)
		if err ne adsec_error.ok then return, err
		log_print, "Max actual gain: "+strtrim( max(new_gain),2)

		wait, 0.1
                
        if ~keyword_set(NOCHECK) then begin
            err = get_ave_pc(ave_pos, ave_curr, min_pos, max_pos, SAMPLES=1024UL)
            if err ne adsec_error.ok then return,err
            
            amp_pos = (max_pos-min_pos)[the_act_list]
            amp_ctrl_curr = amp_pos * (reform(new_gain)*adsec.gain_to_bbgain[the_act_list])
            is_max_curr = max(abs(ave_curr[the_act_list]),idx_max_curr) gt (max_curr*1.1)
            is_max_amp_curr = max(amp_ctrl_curr,idx_amp_ctrl_curr) gt max_amp_curr
            is_max_amp_pos = max(amp_pos,idx_amp_pos) gt max_amp_pos
            if is_max_curr or is_max_amp_curr or is_max_amp_pos then begin
                err=set_gain(the_act_list, gain0, /NO_CHECK, SPEED=speed)
                if err ne adsec_error.ok then return, err
                if enable_fast ne 0 then begin
                    err = set_diagnostic(/master)
                    if err ne adsec_error.ok then return, err
                    err = set_diagnostic(over=old_ovs)
                    if err ne adsec_error.ok then return, err
                endif
                log_print, "During the gain raising:"
                if is_max_curr then log_print, "curr. value" + string(max_curr)+ " exceeded max level at "+strtrim(the_act_list[idx_max_curr],2)
                if is_max_amp_curr then log_print, "curr. amplitude exceeded max level at "+strtrim(the_act_list[idx_amp_ctrl_curr],2)
                if is_max_amp_pos then log_print, "pos. amplitude exceeded max level at "+strtrim(the_act_list[idx_amp_pos],2)

                log_message,"Erron ramp control loop gains: the gain was reset to the initial gain.", CONT=(sc.debug eq 0)
                return, adsec_error.IDL_SAFE_PARAMS_OOB
            endif

	    endif

    endfor

   if ~keyword_set(NOCHECK) && enable_fast then begin
;    if enable_fast ne 0 then begin
        err = set_diagnostic(/master)
        if err ne adsec_error.ok then return, err
        err = set_diagnostic(over=old_ovs)
        if err ne adsec_error.ok then return, err
    endif


	return, adsec_error.ok
end
