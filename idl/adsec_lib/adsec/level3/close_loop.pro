; $Id: close_loop.pro,v 1.11 2007/09/19 09:35:14 labot Exp $

;+
; NAME:
;  CLOSED_LOOP
;
; PURPOSE:
;   This .pro provides to close the loop on all actuators or a list of actuators.
;	The gain of closed loop is arised by ramp_gain from 0 to final gain.
;   The final gain is imposed equal to the mean of the three actuators gains used in isostatic setting.
;	
;
; CATEGORY:
;   Adsec Hardware Function, Level 3
;
; CALLING SEQUENCE:
;   Err = close_loop(ACT_LIST = cl_list,MAX_AMP_POS=max_amp_pos, MAX_CURR=max_curr, XADSEC=xadsec)			
;
; INPUTS:
;
; OUTPUTS:
;   Error code (see INIT_ERROR_CODES.pro)
;
; KEYWORDS:
;   ACT_LIST:       set a list of  actuators to close loop.
;   MAX_AMP_POS:    set a value for the max amplitude of position to apply.
;   MAX_CURRENT:    set a value to max_current to apply.
;   XADSEC:         provide to update the user interface.
;
; COMMON BLOCKS:
;   Adsec std common blocks (see ADSEC_COMMON.pro )
;	
; HISTORY
;
;   Written by G. Brusa and A. Riccardi (AR)
;   Osservatorio Asrofisico di Arcetri, ITALY
;   <riccardi@arcetri.astro.it>
;
;   13 Mar 2002, AR
;     Bug fixed in the CONT keyword setting of the MESSAGE call.
;	26 Jul 2004, Daniela Zanotti (DZ) 
;		Added the two keywords MAX_AMP_POS, MAX_CURR, in the ramp_gain called.
;		Added adsec.max_amp_pos.  
;		Data type adapted for LBT.
;   02 Nov 2004, MX
;     Adsec.all changed in sc.all.
;   14c Mar 2007, MX
;     Keyword FORCE to close loop on al actuators that can do it without any control added.
;-
function close_loop, ACT_LIST=cl_list, MAX_AMP_POS=max_amp_pos, MAX_CURR=max_curr, XADSEC=xadsec, FORCE=force, FINAL_GAIN=final_gain

    @adsec_common

    if n_elements(cl_list) eq 0 then begin
        cl_list=adsec.act_w_cl
    endif else begin
        if check_channel(cl_list) then begin
            message,"Wrong actuator list",cont=(sc.debug eq 0B)
            return,adsec_error.input_type
        endif
        if complement(cl_list,adsec.act_w_cl,dummy) then begin
            message,"act_list must be included in adsec.act_w_pos list"
            return,adsec_error.input_type
        endif
    endelse

    err = update_status(1024)
    if err ne adsec_error.ok then return,err

if ~keyword_set(FORCE) then begin

    old_cl_list=where(sys_status.closed_loop,count)
    err = complement(old_cl_list, cl_list, new_cl_list)
    if err ne 0 then begin
        message,"The initial closed loop act. must be included in the final closed loop list"
        return,adsec_error.IDL_INPUT_TYPE
    endif

    if n_elements(max_curr) eq 0 then max_curr=2*adsec.weight_curr
    if n_elements(max_amp_pos) eq 0 then max_amp_pos = adsec.max_amp_pos

    if count lt 3 then begin
        message,"At least three actuators must be already in closed loop"
        return,adsec_error.IDL_INPUT_TYPE
    endif

    if n_elements(new_cl_list) eq 0 then return,adsec_error.ok
    
    if total(sys_status.position[new_cl_list] le 1.02*adsec.min_pos_val[new_cl_list]) ne 0 $
      or total(sys_status.position[new_cl_list] ge 0.98*adsec.max_pos_val[new_cl_list]) ne 0 then begin
        message, "The mirror position is outside the valid range.", CONT=sc.debug eq 0B
        return, adsec_error.IDL_SAFE_PARAMS_OOB
    endif
endif else begin
    new_cl_list = cl_list
endelse

    err = set_gain(new_cl_list, 0.0)
    if err ne adsec_error.ok then return,err
    err = set_gain(new_cl_list, 0.0, /SPEED)
    if err ne adsec_error.ok then return,err
    
    err = get_preshaper(sc.all, cmd_ps_old)
    if err ne adsec_error.ok then return,err
    err = get_preshaper(sc.all, ff_ps_old, /FF)
    if err ne adsec_error.ok then return,err
    
    err = set_preshaper(sc.all, dsp_const.fastest_preshaper_step)
    if err ne adsec_error.ok then return,err
    err = set_preshaper(sc.all, dsp_const.fastest_preshaper_step,/FF)
    if err ne adsec_error.ok then return,err
    
    err = write_des_pos(new_cl_list, sys_status.position[new_cl_list])
    if err ne adsec_error.ok then return,err
    
    err = set_preshaper(sc.all, cmd_ps_old)
    if err ne adsec_error.ok then return,err
    err = set_preshaper(sc.all, ff_ps_old ,/FF)
    if err ne adsec_error.ok then return,err
    
    err = start_control(new_cl_list)
    if err ne adsec_error.ok then return,err

if ~keyword_set(FORCE) then begin  
    err = get_gain(old_cl_list, old_gain)
    if err ne adsec_error.ok then return,err
    ave_gain = mean(old_gain)
endif else begin
    ave_gain=0.0
endelse    
    if n_elements(final_gain) gt 0 then begin
        if final_gain gt ave_gain then begin
            message, "Wrong GAIN in input.", CONT = (sc.debug eq 0)
            return, adsec_error.input_type
        endif else begin
            ave_gain=final_gain
        endelse
    endif
    err = ramp_gain(new_cl_list, ave_gain, MAX_AMP_POS=max_amp_pos, MAX_CURR=max_curr)
    if err ne adsec_error.ok then begin
        dummy_err=stop_control(new_cl_list)
        wait, 0.1
        if keyword_set(xadsec) then update_panels
        return,err
    endif
    
    if keyword_set(xadsec) then update_panels

    
    return,adsec_error.ok
end

