; $Id: set_isostatic.pro,v 1.11 2009/05/22 16:32:55 labot Exp $

;+
; NAME:
; 	SET_ISOSTATIC
;
; PURPOSE:
;	This .pro provides to set the mirror in isostatic configuration.
;	First the shell is pressed on the reference plate, after the currents
;	are slowly released and the mirror is supported by three actuators
;	in closed loop and all the others in open loop with a bias current.
;
; CATEGORY:
;	Adsec Hardware Function, Level 3
;
; CALLING SEQUENCE:
;
;	Err = SET_ISOSTATIC (three_act, three_pos, f_gain, $
;                        INITIAL_CURRENT=initial_current, MAX_CURRENT=max_curr, $
;                        MAX_AMP_POS=max_amp_pos, $
;                        CL_ACT=cl_act, XADSEC=xadsec, TILT=tilt)
;
; INPUTS:
;	Three_act:	actuators to use to isostatic configuration setting
;	Three_pos:	position in which close the loop in isostatic configuration
;	f_gain:		final proportional gain for the 3 closed loop actuators
;
; OUTPUTS:
;	Error code (see INIT_ERROR_CODES.pro)
;
; KEYWORDS:
;	INITIAL_CURRENT:set if you want that the function returns the initial current set to the three acts.
;	MAX_CURRENT:	set a value to max_current to apply.
;	MAX_AMP_POS:	set a value for the max amplitude of position to apply.
;	CL_ACT:			set a list of others acts to CL.
;	XADSEC:			provide to update the user interface.
;	TILT:			provide to remove the tip-tilt from the three acts.
;
; COMMON BLOCKS:
;	Adsec std common blocks (see ADSEC_COMMON.pro )
;
; EXAMPLE:
;
;	To set the mirror in isostatic configuration:
;
;		err = SET_ISOSTATIC([1,25,33], replicate(adsec.nominal_gap,3),800)
;
; HISTORY
;
;   Written by Guido Brusa-Zappellini (GBZ) and A. Riccardi (AR)
;   Osservatorio Asrofisico di Arcetri, ITALY
;   <riccardi@arcetri.astro.it>
;
;   13 Mar 2002, AR
;       Bug fixed in the CONT keyword setting of the MESSAGE call.
;	04 Feb 2004, M. Xompero (MX)
;	    Bias magnet supported.
;	12 Feb 2004, AR & MX
;	    Raise_curr used instead of write_bias_curr.
;	17 Feb 2004, MX
;	    New adsec.secure_factor parameter used.
;	27 Jul 2004, D.Zanotti (DZ)  
;		New parameters in adsec structure:max_curr_tospread,max_amp_pos, min_gain_step, max_gain_step used,
;		max_curr, max_deltacurr, fact_reduction_curr.
;		Changed the management of g_step (limited by max_current)
;		Replaced the lines from 226 to 234 with the call to the apply_curr_opt function
;		Before the write_des_pos set the preshaper to min raising time with control gain equal to 0L, 
;		then red-estabilshed to the initial condition.
;   02 Nov 2004, MX
;       Adsec.all changed in sc.all.
;   07 Feb 2007 MX
;       LUT of variable deleted and name refurbished.
;-
function set_isostatic, three_act, three_pos, f_gain, $
                        INITIAL_CURRENT=initial_current, MAX_CURRENT=max_curr, $
                        MAX_AMP_POS=max_amp_pos, $
                        CL_ACT=cl_act, XADSEC=xadsec, TILT=tilt

@adsec_common

act_w_curr = adsec.act_w_curr
n_act_w_curr = n_elements(act_w_curr)
err = get_preshaper(sc.all_actuators, pp)		;read cmd preshaper
if err ne adsec_error.ok then return,err
err = get_preshaper(sc.all_actuators, fp,/FF)	;read curr preshaper
if err ne adsec_error.ok then return,err

step_ps = dsp_const.slowest_preshaper_step
time_ps = adsec.preshaper_len/step_ps*adsec.sampling_time
err = set_preshaper(sc.all_actuators, step_ps)		
if err ne adsec_error.ok then return,err
err = set_preshaper( sc.all_actuators, step_ps, /FF)
if err ne adsec_error.ok then return,err

if n_elements(cl_act) ne 0 then begin
    if not intersection(cl_act, three_act, inter_act) then $
        message, "CL_ACT cannot have elements in common with three_act"
    use_cl_act=1B
    all_cl_act = [three_act[*],cl_act[*]]
endif else begin
    use_cl_act=0B
    all_cl_act = three_act
endelse

if complement(all_cl_act,adsec.act_w_cl,dummy_list) then begin
    message, "The cl_list contains actuators not available for closing the loop", cont=(sc.debug eq 0B)
    return, adsec_error.generic_error
endif

if n_elements(max_curr) eq 0 then max_curr=2*adsec.weight_curr
if n_elements(max_amp_pos) eq 0 then max_amp_pos=adsec.max_amp_pos;10						 

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;err = raise_curr(initial_current)
initial_current = adsec.secure_factor*adsec.weight_curr
;da verificare tutte le correnti: secondo me sono doppie (magneti di bias)
err = raise_curr(initial_current)
if err ne adsec_error.ok then return,err

err = set_gain(all_cl_act, 0.0)
if err ne adsec_error.ok then return,err
err = set_gain(all_cl_act, 0.0, /SPEED)
if err ne adsec_error.ok then return,err

wait, 1.0

middle_current = adsec.secure_factor*adsec.weight_curr*adsec.fact_reduction_curr
middle_current = initial_current ;;;;;;;;;;;;;;;;;;;;;; MEZZO PESOO!!!!!! 
err = raise_curr(middle_current)
if err ne adsec_error.ok then return,err

wait, 1.0

err = update_status()
if err ne adsec_error.ok then return,err


three_pos0 = three_pos ;< replicate(-20000,n_elements(three_act))

err = set_preshaper(sc.all_actuators, adsec.preshaper_len)
if err ne adsec_error.ok then return,err

err = write_des_pos(three_act, three_pos0)
if err ne adsec_error.ok then return,err
err = start_control(three_act)
if err ne adsec_error.ok then return,err

err = set_preshaper(sc.all_actuators, step_ps)
if err ne adsec_error.ok then return,err


pos0=sys_status.position[three_act]
gain = fltarr(n_elements(three_act))
counter=1
max_count = 100
repeat begin

    while ((max(abs(sys_status.ctrl_current[three_act])) le adsec.max_curr_tospread) and $
           (total(gain lt f_gain) gt 0)) do begin  

		max_p_step = max(abs(three_pos0-pos0))
		;; gain is computed to avoid a current step larger of 1000
		;; gain is forced to be within 1 and 100
		;g_step = (min((4000.0/max_p_step)/adsec.gain_to_bbgain[three_act]) > 1L) < 100L
		g_step = (min((adsec.max_curr/max_p_step)/adsec.gain_to_bbgain[three_act]-gain) $
		> (adsec.min_gain_step/adsec.gain_to_bbgain[three_act]) ) < (adsec.max_gain_step/adsec.gain_to_bbgain[three_act]) ;da verificare in debug

        gain =(gain+g_step) < f_gain
        err = set_gain(all_cl_act, gain)
        if err ne adsec_error.ok then return,err

        wait, 0.01

        err = update_status(100)
        if err ne adsec_error.ok then return,err

	pos0 = sys_status.position[three_act]

        print, "currents:", sys_status.current[three_act]
        print, "pos:", sys_status.position[three_act]
        print, "commands:", sys_status.command[three_act]
        print, "gain:", gain
        print, "ctrl currents",sys_status.ctrl_current[three_act]
        counter = 1

    endwhile

    err = apply_curr_opt(/XAD,n_iter=1)
    if err ne adsec_error.ok then return,err

    print, "av. current ",mean(sys_status.current[act_w_curr])
    print, "ctrl currents",sys_status.ctrl_current[three_act]
    counter = counter+1

endrep until (total(gain ge f_gain) eq n_elements(gain)) or (counter ge max_count)

if counter ge max_count then begin
    message, "The requested gain has not been applied. Too many iterations.", CONT=sc.debug eq 0B
    return, adsec_error.generic_error
endif

;=== start commented code, it should be no longer needed
;err = set_gain(all_cl_act, f_gain)

;err = get_commands(three_act,comm0)

;c_step = adsec.max_deltacurr/(adsec.gain_to_bbgain[three_act]*gain)
;dcomm = three_pos-comm0
;n_step = max((fix(round(dcomm/c_step)))>1)
;c_step = dcomm/n_step

;for i_step=1,n_step do begin
;    temp_three_comm = comm0+i_step*c_step
;    err = write_des_pos(three_act, temp_three_comm)
;    if err ne adsec_error.ok then return,err
;    wait,time_ps
;    err = apply_curr_opt(/XAD,n_iter=1)
;    if err ne adsec_error.ok then return,err
;    print,"position command: ",temp_three_comm
;endfor
;=== end commented code

if keyword_set(xadsec) then update_panels, /OPT

if keyword_set(integrate) then begin
    err = integrator(three_act,THR=max_amp_pos,MAXCURR=max_curr,PERR=perr,ITER=iter,PAMP=pamp)
    if err ne adsec_error.ok then message, "ERROR!!"

	err = apply_curr_opt(ACT_LIST=act_w_curr,n_iter=3) 
	if err ne adsec_error.ok then return,err
		 


    err = get_ave_pc(ave_pos, ave_curr, min_pos, max_pos, SAMPLES=1000UL)
    if err ne adsec_error.ok then return,err

    amp_pos = (max_pos-min_pos)[three_act]
    amp_ctrl_curr = amp_pos * (f_gain*adsec.gain_to_bbgain[three_act])
    if (max(abs(ave_curr)) gt max_curr) or $
       (max(amp_pos) gt max_amp_pos) or $
       (max(amp_ctrl_curr) gt max_curr) then begin
          err=write_same_ch(sc.all_actuators, dsp_map.control_enable, 0L)
          if err ne adsec_error.ok then message, "ERROR!!"
          message,"The mirror control was reset at force balancing"
    endif

endif

if keyword_set(xadsec) then update_panels, /OPT

wait, 1.0

if keyword_set(tilt) then begin
    err = remove_tilt(three_act,/integr)
    if err ne adsec_error.ok then message, "ERROR!"

    if keyword_set(xadsec) then update_panels, /OPT
endif

err = set_preshaper(sc.all_actuators, pp)
if err ne adsec_error.ok then return,err
err = set_preshaper( sc.all_actuators, fp, /FF)
if err ne adsec_error.ok then return,err

return,0L
end
