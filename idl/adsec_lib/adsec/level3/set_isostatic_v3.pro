; $Id: set_isostatic_v3.pro,v 1.4 2009/05/22 16:32:56 labot Exp $

;+
; NAME:
; 	SET_ISOSTATIC_v3
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
;	Err = SET_ISOSTATIC (three_act, initial_three_pos, three_pos, s_gain, f_gain, $
;                        INITIAL_CURRENT=initial_current, MAX_CURRENT=max_curr, $
;                        MAX_AMP_POS=max_amp_pos, $
;                        CL_ACT=cl_act, XADSEC=xadsec, TILT=tilt)
;
; INPUTS:
;	Three_act:	actuators to use to isostatic configuration setting
;   Initial_three_pos: initial command for the three actuators used for isostatic
;	Three_pos:	position in which close the loop in isostatic configuration
;   s_gain:     minimum initial gain to use in three actuators close loop operations
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
;   OTHER_CL_INIT_POS: forced initial delta command for the others closed loop actuators
;   OTHER_CL_INIT_GAIN: forced initial gain for the others closed loop actuators
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
;   13 Jul 2007, MX
;       Crack management inserted. OTHER_CL_XXX keywords added.
;-
function set_isostatic_v3, three_act, initial_three_pos, three_pos, s_gain, f_gain, $
                        INITIAL_CURRENT=initial_current, MAX_CURRENT=max_curr, $
                        MAX_AMP_POS=max_amp_pos, OTHER_CL_INIT_POS=other_cl_init_pos, $
                        CL_ACT=cl_act, XADSEC=xadsec, TILT=tilt, OTHER_CL_INIT_GAIN=other_cl_init_gain

@adsec_common
common temporary_block, comm0, c_step, n_step, cl_list


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
if n_elements(max_amp_pos) eq 0 then max_amp_pos=adsec.max_amp_pos						 
if n_elements(OTHER_CL_INIT_POS) eq 0 then other_cl_init_pos = 6e-6
if n_elements(OTHER_CL_INIT_GAIN) eq 0 then other_cl_init_gain = adsec.weight_curr/2.*0.25/other_cl_init_pos /1e6;[N/um] ; forza magneti bias/4 (fattore sicurezza)/delta_posizione ;;-> da fare il controllo sulle orecchie a mano...

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;err = raise_curr(initial_current)
initial_current = adsec.secure_factor*adsec.weight_curr ; azzera le correnti in modo da uniformare i bias current
;da verificare tutte le correnti: secondo me sono doppie (magneti di bias)
err = raise_curr(initial_current)
if err ne adsec_error.ok then return,err

err = set_gain(sc.all, 0.0)
if err ne adsec_error.ok then return,err
err = set_gain(sc.all, 0.0, /SPEED)
if err ne adsec_error.ok then return,err

wait, 1.0


err = update_status()
if err ne adsec_error.ok then return,err

three_pos0 = initial_three_pos 
;cricca = adsec.mir_act2act([221, 223, 224, 276, 277, 278, 279, 337, 338, 339, 340, 404, 405, 406, 407, 477, 478, 479, 480, 556, 557, 558, 559])
cricca = adsec.mir_act2act([221, 223, 224, 276, 277, 278, 279, 337, 338, 339, 340, 404, 405, 406, 407, 477, 478, 479, 480, 556, 558, 559])
err = complement([all_cl_act, cricca],adsec.act_w_cl,other_cl_act) 
if err ne adsec_error.ok then return,adsec_error.input_type
other_cl_pos0 = sys_status.position[other_cl_act]
err = complement(cricca, adsec.act_w_cl, cl_list)
if err ne adsec_error.ok then return, adsec_error.input_type

err = set_preshaper(sc.all_actuators, adsec.preshaper_len)
if err ne adsec_error.ok then return,err

err = write_des_pos(three_act, three_pos0)
if err ne adsec_error.ok then return,err
err = start_control(three_act)
if err ne adsec_error.ok then return,err

err = write_des_pos(other_cl_act, other_cl_pos0+other_cl_init_pos)
if err ne adsec_error.ok then return,err
err = start_control(other_cl_act)
if err ne adsec_error.ok then return,err

err = set_preshaper(sc.all_actuators, step_ps)
if err ne adsec_error.ok then return,err


pos0=sys_status.position[three_act]
gain = fltarr(n_elements(three_act))
counter=1
max_count = 1000

;FIRST STEP ALONE IN ORDER TO SET the MINIMUM GAIN IN THE THREE ACTS (minimum gain gt adsec.max_gain_step)
if (total(gain lt s_gain) gt 0) then begin  

    max_p_step = max(abs(three_pos0-pos0))
    gain = s_gain 
    new_force = max_p_step*gain*mean(adsec.gain_to_bbgain[three_act])
    cond_sat = total((new_force lt adsec.neg_sat_force/2.) or (new_force gt adsec.pos_sat_force/2.)) gt 0
    cond_max_curr = total(abs(new_force) gt adsec.max_curr) gt 0
    if cond_sat or cond_max_curr then begin    
        message,'Start gain and initial_delta_pos are not compatible for isostatic configuration.', CONT=(sc.debug eq 0)
	    return, adsec_error.input_type
    endif

    g_maxstep=(adsec.max_gain_step)/mean(adsec.gain_to_bbgain[three_act])
    if gain gt g_maxstep then begin
        gain0 = 0.0
        itt = floor(gain / g_maxstep)    
        for j = 0, itt-1 do begin
            gain0 = gain0 + g_maxstep
            err = set_gain(all_cl_act, gain0)
            if err ne adsec_error.ok then return,err
        endfor
    endif
    err = set_gain(all_cl_act, gain)
    if err ne adsec_error.ok then return,err

    wait, 0.01

    err = update_status(100)
    if err ne adsec_error.ok then return,err
    if keyword_set(XADSEC) then update_panels, /OPT, /NO_READ
    pos0 = sys_status.position[three_act]

    print, "currents:", sys_status.current[three_act]
    print, "pos:", sys_status.position[three_act]
    print, "commands:", sys_status.command[three_act]
    print, "gain:", gain
    print, "ctrl currents",sys_status.ctrl_current[three_act]

endif

err = ramp_gain(other_cl_act, other_cl_init_gain)
if err ne adsec_error.ok then return, err


repeat begin

    while ((max(abs(sys_status.ctrl_current[three_act])) le adsec.max_curr_tospread) and $
           (total(gain lt f_gain) gt 0)) do begin  

		max_p_step = max(abs(three_pos0-pos0))
		g_step = (min((adsec.max_curr/max_p_step)/adsec.gain_to_bbgain[three_act]-gain) $
		> (adsec.min_gain_step/adsec.gain_to_bbgain[three_act]) ) < (adsec.max_gain_step/adsec.gain_to_bbgain[three_act]) 

        gain =(gain+g_step) < f_gain
        err = set_gain(all_cl_act, gain)
        if err ne adsec_error.ok then return,err

        wait, time_ps

        err = update_status(100)
        if err ne adsec_error.ok then return,err
        if keyword_set(xadsec) then update_panels, /OPT, /NO_R

        pos0 = sys_status.position[three_act]

        print, "currents:", sys_status.current[three_act]
        print, "pos:", sys_status.position[three_act]
        print, "commands:", sys_status.command[three_act]
        print, "gain:", gain
        print, "ctrl currents",sys_status.ctrl_current[three_act]
        counter = 1

    endwhile

    err = apply_curr_opt(XADSEC=xadsec,N_ITER=1)
    if err ne adsec_error.ok then return,err

    print, "av. current ",mean(sys_status.current[act_w_curr])
    print, "ctrl currents",sys_status.ctrl_current[three_act]
    counter = counter+1

endrep until (total(gain ge f_gain) eq n_elements(gain)) or (counter ge max_count)

if counter ge max_count then begin
    message, "The requested gain has not been applied. Too many iterations.", CONT=sc.debug eq 0B
    return, adsec_error.generic_error
endif

;=== start commented code, it should be no longer needed *********************************
;err = set_gain(all_cl_act, f_gain)   -> c'e' gia'
;question
wait, 2*time_ps ;;;;;TO BE UNDERSTOOD!!!!!!!!!

err = update_status()
if err ne adsec_error.ok then return, err
if keyword_set(XADSEC) then begin
    update_panels, /OPT
    update_panels, /OPT
    update_panels, /OPT
    update_panels, /OPT
    update_panels, /OPT
    update_panels, /OPT
    update_panels, /OPT
    update_panels, /OPT
endif

answ = dialog_message(["The mirror is in "+strtrim(mean(sys_status.position[adsec.act_w_pos]),2 ), $
                           "Do you want to continue to the final position?"], /QUEST)

if strlowcase(answ) eq "no" then begin
    ;err = rip(XADSEC=xadsec)
    ;if err ne adsec_error.ok then return,err
    
    ;err = clear_dacs(sc.all)
    ;if err ne adsec_error.ok then return,err

    ;err = disable_coils()
    ;if err ne adsec_error.ok then return,err
    
    err = update_status()
    if err ne adsec_error.ok then return, err
    if keyword_set(XADSEC) then update_panels, /OPT, /NO_READ
    
    return, adsec_error.ctrl_disabled

endif 

;
;err = get_commands(three_act,comm0)
err = get_commands(cl_list,comm0)
if err ne adsec_error.ok then return, err

cl_act_pos = replicate(mean(three_pos), n_elements(cl_list))
dcomm = cl_act_pos-comm0
;dcomm = three_pos-comm0
n_step = max(floor(dcomm / adsec.max_amp_pos)) ;;;;;;DA DECIDERE SE METTERLO DA FILE DI CONF
c_step = dcomm/n_step

;stop
err = update_status()
if err ne adsec_error.ok then return, err

;err = zero_ctrl_current()
;if err ne 0 then return, adsec_error.ok

for i_step=1,n_step do begin
    temp_comm = comm0+i_step*c_step
    ;temp_comm[three_act] = sys_status.position[three_act] < comm0[three_act]
    err = write_des_pos(cl_list, temp_comm)
    if err ne adsec_error.ok then return,err
    wait,time_ps
    err = apply_curr_opt(XADSEC=xadsec,N_ITER=1)
    if err ne adsec_error.ok then return,err
    print,"position command: ",temp_comm[three_act]
endfor

;=== end commented code *********************************

if keyword_set(xadsec) then update_panels, /OPT

;err = remove_tilt(cl_list, zern_idx=[5,6,9,10], XADSEC=xadsec)
;if err ne adsec_error.ok then return,err


;decrease gain on three_act
err = get_gain(other_cl_act, gain_other)
if err ne adsec_error.ok then return,err

gain_act = mean(gain_other)

err = get_gain(three_act, gain0)

g_maxstep=(adsec.max_gain_step)/mean(adsec.gain_to_bbgain[three_act])
dgain = float(gain_act-gain0)

if total(abs(dgain) gt g_maxstep) then begin
        nstep = max(floor(abs(dgain / g_maxstep)))
        gstep = dgain/nstep
        for j = 1, nstep do begin
            temp_gain = gain0 + gstep*j
            err = set_gain(three_act, temp_gain)
            if err ne adsec_error.ok then return,err
            wait, time_ps
            err = update_status()
            if err ne adsec_error.ok then return, err
            if keyword_set(XADSEC) then update_panels, /OPT, /NO_READ
        endfor
endif
err = set_gain(three_act, gain_act)
if err ne adsec_error.ok then return,err

wait, time_ps

err = update_status()
if err ne adsec_error.ok then return, err
if keyword_set(xadsec) then update_panels, /OPT

;remove zernike low orders (Idx: 5,6,9,10)

err = remove_tilt(cl_list, zern_idx=[5,6,9,10], XADSEC=xadsec)
if err ne adsec_error.ok then return,err

wait, time_ps

;set the actual position as command in the crack and equalize the gains
err = update_status()
if err ne adsec_error.ok then return,err
if keyword_set(xadsec) then update_panels, /OPT, /NO_READ

comm_crack = sys_status.position[cricca]

err = set_gain(cricca, 0.0)
if err ne adsec_error.ok then return,err

err = write_des_pos(cricca, comm_crack)
if err ne adsec_error.ok then return,err

err = start_control(cricca)
if err ne adsec_error.ok then return,err

if keyword_set(xadsec) then update_panels, /OPT

err = ramp_gain(cricca, gain_act, MAX_AMP_GAIN=0.0001)
if err ne adsec_error.ok then return,err

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
