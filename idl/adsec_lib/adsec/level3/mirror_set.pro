; $Id: mirror_set.pro,v 1.12 2006/06/20 09:55:47 labot Exp $
;+
;	NAME:
;		MIRROR_SET
;	
;	PURPOSE:
;
;	CATEGORY:
;   	Adsec Hardware Function, Level 3
;
;	CALLING SEQUENCE:
;
;	Err = mirror_set(THREE_POS=three_pos, THREE_ACT=three_act, I_GAIN=initial_gain, F_GAIN=mirror_set_gain $
;	, MAX_CURRENT=max_curr, MAX_POS_AMPLITUDE=max_amp_pos, MAX_ITERATIONS=max_iter,	IS_ISOSTATIC=is_isostatic)$
;	, STOP_ISOSTATIC = stop_isostatic, SKIP_ASK4CL=skip_ask4cl)
;
; 	INPUTS:
;
;	OUTPUTS:
;   Error code (see INIT_ERROR_CODES.pro)
;
;	KEYWORDS:
;   THREE_ACT:  actuators to use to isostatic configuration setting
;   THREE_POS:  position in which close the loop in isostatic configuration
;   I_GAIN:     initial proportional gain for the 3 closed loop actuators
;   F_GAIN:     final proportional gain for the 3 closed loop actuators
;
;   COMMON BLOCKS:
;   Adsec std common blocks (see ADSEC_COMMON.pro )
;
;	HISTORY
;	  ?? ??? ????   Written By Armando Riccardi (AR)
;	  12 Feb 2004 	Keyword STOP_ISOSTATIC added to stop the mirror setting in
;                       the isostatic configuration.
;	  16 Feb 2004	M. Xompero (MX). Added SKIP_ASK4CL keyword. If SKIP_ASK4CL is set,
;                       the program skip the dialog if we want to close
;                       the loop on all actuators with a dialog message.
;	  22 Jul 2004	D. Zanotti (DZ). Added two parameters in adsec structure: max_amp_pos,max_iter_integ
;     12 Aug 2004   AR
;       dsp_const.slowest_preshaper_step is used in set_prehaper call
;     02 Nov 2004, MX
;      Adsec.all changed in sc.all.
;     03 Aug 2005, MX
;      Accumulator setting restored after the mirror setting.
;     March 2006, DZ
;      Added the xadsec check.
;     20 Jun 2006, MX
;      Added the test about the coils status before proceed with the mirror set.
;-

function mirror_set, THREE_POS=three_pos, THREE_ACT=three_act, I_GAIN=i_gain, F_GAIN=f_gain $
                   , MAX_CURRENT=max_curr, MAX_POS_AMPLITUDE=max_amp_pos $
                   , MAX_ITERATIONS=max_iter, IS_ISOSTATIC=is_isostatic $
                   , STOP_ISOSTATIC = stop_isostatic, SKIP_ASK4CL=skip_ask4cl

@adsec_common
xadsec = (xregistered('xadsec') gt 0)
; setting the slowest preshaper available
err=set_preshaper(sc.all_actuators,dsp_const.slowest_preshaper_step)
if err ne adsec_error.ok then return, err

err=set_preshaper(sc.all_actuators,dsp_const.slowest_preshaper_step,/FF)
if err ne adsec_error.ok then return, err

; setting the accumulator parameters
err = set_acc(1024L, 500L, old_samples, old_delay)
if err ne adsec_error.ok then return, err

if n_elements(three_pos) eq 0 then begin
    message,' Three actuator initial positions are not defined.', CONT=(sc.debug eq 0)
	return, adsec_error.input_type
endif

if n_elements(three_act) eq 0 then begin		; just 3 actuators 120deg apart ;;int
	message,' Three actuator for isostatic configuration are not defined.', CONT=(sc.debug eq 0)
	return, adsec_error.input_type
endif

if n_elements(i_gain) eq 0 then begin
	message,'Initial gain for isostatic configuration are not defined.', CONT=(sc.debug eq 0)
    return, adsec_error.input_type
	endif

if n_elements(f_gain) eq 0 then begin           ;low if not defined, just for safety ;;long
    message,'Final gain for isostatic configuration are not defined.', CONT=(sc.debug eq 0)
	return, adsec_error.input_type
endif

if n_elements(max_curr) eq 0 then max_curr = 2*adsec.weight_curr	; max current for integrator() e close_loop() routines
if n_elements(max_amp_pos) eq 0 then max_amp_pos = adsec.max_amp_pos ;340nm max-min pos allowed in close_loop() routine
if n_elements(max_iter) eq 0 then max_iter = adsec.max_iter_integ			;4 max number of iteration in apply_curr_opt() and integrator() routines

err = test_coils(status)
    if err ne adsec_error.ok then return, err


err = set_isostatic(three_act, three_pos, i_gain, XADSEC=xadsec, /TILT)
if err ne adsec_error.ok then return, err

if keyword_set(stop_isostatic) then begin
	is_isostatic=1B
	return, adsec_error.ok
endif

if keyword_set(skip_ask4cl) then $
	answ = "yes" $
else $
	answ = dialog_message(["The mirror is in isostatic configuration", $
                       	   "Do you want to continue?"], /QUEST) 

if strlowcase(answ) eq "no" then begin
    is_isostatic=1B
    return, adsec_error.ok
endif else  is_isostatic=0B

;update_panels, /OPT

if xadsec then update_panels, /OPT else begin
    err = update_status()
    if err ne adsec_error.ok then begin
            print, "Error updating the status (err code "+strtrim(err,2)+")."
            return, err
    endif
endelse

wait, 0.1

print, "... applying the integrator..."
thr = max(sys_status[three_act].max_pos-sys_status[three_act].min_pos)
err = integrator(three_act,THR=thr,MAXCURR=max_curr,MAXITER=max_iter,PERR=perr,ITER=iter,PAMP=pamp)
if err ne adsec_error.ok then return, err

wait, 0.1
print, "... optimizing the current pattern... "
err = apply_curr_opt(N_ITER=max_iter, XADSEC=xadsec)
if err ne adsec_error.ok then return, err
wait, 0.1

;update_panels, /OPT

if xadsec then update_panels, /OPT else begin
    err = update_status()
    if err ne adsec_error.ok then begin
            print, "Error updating the status (err code "+strtrim(err,2)+")."
            return, err
    endif
endelse

print, "... applying the integrator..."
thr = max(sys_status[three_act].max_pos-sys_status[three_act].min_pos)
err = integrator(three_act,THR=thr,MAXCURR=max_curr,MAXITER=max_iter,PERR=perr,ITER=iter,PAMP=pamp)
if err ne adsec_error.ok then return, err
wait, 0.1

print, "... set the command to removing the tilt..."
err = remove_tilt(three_act)
if err ne adsec_error.ok then return, err
wait, 0.1

;update_panels, /OPT

if xadsec then update_panels, /OPT else begin
   err = update_status()
    if err ne adsec_error.ok then begin
            print, "Error updating the status (err code "+strtrim(err,2)+")."
            return, err
    endif
endelse

print, "... applying the integrator..."
thr = max(sys_status[three_act].max_pos-sys_status[three_act].min_pos)
err = integrator(three_act,THR=thr,MAXCURR=max_curr,MAXITER=max_iter,PERR=perr,ITER=iter,PAMP=pamp)
if err ne adsec_error.ok then return, err
wait, 0.1


print, "... optimizing the current pattern... "
err = apply_curr_opt(N_ITER=max_iter, XADSEC=xadsec)
if err ne adsec_error.ok then return, err
wait, 0.1


print, "... closing the loop..."

err = set_gain(three_act, f_gain, /NO_CHECK)
if err ne adsec_error.ok then return, err

err = close_loop(MAX_AMP_POS=max_amp_pos,MAX_CURR=max_curr, XADSEC=xadsec)
if err ne adsec_error.ok then return, err

wait, 0.1

;update_panels, /OPT

if xadsec then update_panels, /OPT else begin
    err = update_status()
    if err ne adsec_error.ok then begin
            print, "Error updating the status (err code "+strtrim(err,2)+")."
            return, err
    endif
endelse

wait, 0.1

print, "... setting the command at the current position..."
err = zero_ctrl_current()
if err ne adsec_error.ok then return, err

wait, 0.1

;update_panels, /OPT

if xadsec then update_panels, /OPT else begin
    err = update_status()
    if err ne adsec_error.ok then begin
            print, "Error updating the status (err code "+strtrim(err,2)+")."
            return, err
    endif
endelse

err = set_acc(old_samples, old_delay)

return, adsec_error.ok
end
