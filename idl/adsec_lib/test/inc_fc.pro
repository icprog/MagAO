; $Id: inc_fc.pro,v 1.9 2007/04/12 12:34:34 labot Exp $
;+
;   NAME:
;    INC_FC
;
;   PURPOSE:
;    INCREMENTAL FRAMES COUNTER ROUTINE.
;
;   NOTE:
;
;   USAGE:
;    inc_fc, vect, disturb_vect, WAIT=sleep_time, MIRROR=mirror, SWITCHB=is_switch, BUFSKIP=bufskip, DOCHECK=docheck
;           , DISTRBANCE=disturbance, NOVERB=noverb
;
;   INPUT:
;    vect:              Slope vector.
;    disturb_vect:      Disturbance vector.
;
;   OUTPUT:
;       None.
;
;   KEYWORDS:
;       
;
;
;    HISTORY
;
;  09 Aug 2005, A. Riccardi
;    added implementation of data recirculation by Slope Computer BCU
;-
Pro inc_fc, vect, disturb_vect, WAIT=sleep_time, MIRROR=mirror, SWITCHB=is_switch, BUFSKIP=bufskip, DOCHECK=docheck, DISTURBANCE=disturbance, NOVERB=noverb

    @adsec_common
    common inc_fc_block, c1_old, old_modes, delaya, delayb
    n_dsp = adsec.n_actuators/adsec.n_act_per_dsp
    if ~keyword_set(disturbance) then dvhist = disturb_vect*0.0 else dvhist=disturb_vect
    
    if n_elements(vect) eq 0 then vect = fltarr(rtr.n_slope)
    if keyword_set(is_switch) then begin

        ;Read Header from SwitchBCU
        ;
        ;Incrementing frames counter and setting param_selector in correct mode if not
        ;head[1] = head[1] or 2UL^6
        ;if (head[1] and 2UL^12) then message, "Disable disturbance before!!!"

        err = read_seq_dsp(0,rtr_map.switch_SCStartRTR, 1, wfs, /UL, /SWITCH)
        ;err = read_seq_dsp(0,rtr_map.switch_SCstartrtr+1, 1l, pbs, /UL)
        pbs =  2UL^6 or 2UL^7 ;(diagnostic SDRAM + FASTLINK ENABLE)
        dummy = [1UL, 0UL]
        wfs += 1
        ;print, "New WFS Frame Counter: "+string(wfs)

        ;Write Header from SwitchBCU

        ;Writing null slopes
        err = write_same_dsp(0, rtr_map.switch_SCSlopeVector, vect, /CHECK, /SWITCH)    
        if err ne adsec_error.ok then message, "Error encountered: "+strtrim(err,2)
    
        err = write_same_dsp(0, rtr_map.switch_SCStartRTR, [wfs,pbs], /SWITCH, /CHECK)
        if err ne adsec_error.ok then message, "Error encountered: "+strtrim(err,2)

        err = write_same_dsp(0, rtr_map.switch_SCStartRTR+2, dummy, /SWITCH)    
        if err ne adsec_error.ok then message, "Error encountered: "+strtrim(err,2)
        ;wait, 0.01

        ;SWITCH Bcu automations do the rest....
        if keyword_set(docheck) then begin
            ;wait, 0.01
            err = read_seq_dsp(sc.all, rtr_map.slope_vector, rtr.n_slope, dsp_slopes)
            if err ne adsec_error.ok then message, "Error encountered: "+strtrim(err,2)

            if max(abs(dsp_slopes-rebin(vect,rtr.n_slope,n_dsp))) ne 0.0 then begin
                print, "AAARCH!!!"
                err = read_seq_dsp(sc.all, rtr_map.slope_vector, rtr.n_slope, dsp_slope2)
                if err ne adsec_error.ok then message, "Error encountered: "+strtrim(err,2)
                if max(abs(dsp_slope2-rebin(vect,rtr.n_slope,n_dsp))) ne 0.0 then message, "Errore scrittura slopes su DSP"
            endif

            err = read_seq_ch(sc.all, dsp_map.modes, 1l, dsp_modes)
            if err ne adsec_error.ok then message, "Error encountered: "+strtrim(err,2)
        
 ;           true_modes = diagonal_matrix(*rtr.g_gain_a)##(*rtr.b0_a_matrix ## transpose(vect) + delayB) + delayA
            step_0 =transpose(total([mamano(*rtr.b0_a_matrix,transpose(vect)),delayB], 1, /preserve,/Tpool_no))
                ;true_modes = mamano(diagonal_matrix(*rtr.g_gain_a),transpose(step_0))+delayA
            true_modes =transpose(total([mamano(diagonal_matrix(*rtr.g_gain_a),transpose(step_0)),delayA], 1, /preserve,/Tpool_no))
 
            if max(abs(dsp_modes-true_modes)) ne 0.0 then $
              message, "Errore calcolo modi"

            ;PARTIAL CHECK MODES INTO THE SWITCH
            err = read_seq_dsp(sc.all, rtr_map.switch_modesvector, rtr.n_modes2correct, switch_modes, /SWITCH)
            if err ne adsec_error.ok then message, "Error encountered: "+strtrim(err,2)
            if max(abs(dsp_modes[12:*]-switch_modes[12:*])) ne 0.0 then message, "Errore ricircolo modes su switch"

            err = read_seq_dsp(sc.all, rtr_map.modes_vector, adsec.n_actuators, dsp_modes_vec_full)
            if err ne adsec_error.ok then message, "Error encountered: "+strtrim(err,2)

            if rtr.n_modes2correct ne adsec.n_actuators then begin
                dsp_modes_vec = dsp_modes_vec_full[0:rtr.n_modes2correct-1, *]
                residue = dsp_modes_vec_full[rtr.n_modes2correct:*, *]
                if max(abs(residue)) ne 0.0 then message, "Errore: modes non used non nulli!!!"
            endif else begin
                dsp_modes_vec = dsp_modes_vec_full
            endelse

            
            dummy = rebin(transpose(dsp_modes),adsec.n_actuators, n_dsp)
            
            if max(abs(dsp_modes_vec-dummy[0:rtr.n_modes2correct-1, *])) ne 0.0 then begin
                err = test_skip_frame(bufskip,/DSP)
                message, "Errore ricircolo modes su DSP"
            endif

            err = read_seq_ch(sc.all, dsp_map.new_delta_command, 1l, c1)
            if err ne adsec_error.ok then message, "Error encountered: "+strtrim(err,2)
 
            true_c1 = mamano(*rtr.m2c_matrix,transpose(dsp_modes[0:rtr.n_modes2correct-1])) + dvhist
            ;true_c1 = *rtr.m2c_matrix ## transpose(dsp_modes[0:rtr.n_modes2correct-1]) + dvhist
            if max(abs(true_c1-c1)) ne 0.0 then message, "Errore calcolo new_delta_command"
            ;wset, 0
            ;display, c1, /no_num 
            ff_command = c1-c1_old
            c1_old = c1
            
            err = read_seq_ch(sc.all, dsp_map.ff_command, 1l, dsp_ff_command)
            if err ne adsec_error.ok then message, "Error encountered: "+strtrim(err,2)
            
            if max(abs(ff_command-dsp_ff_command)) ne 0.0 then message, "Errore calcolo ff_command su DSP"
            err = read_seq_dsp(sc.all, rtr_map.switch_ffcommandvector, adsec.n_actuators, switch_ff_command, /SWITCH)
            if err ne adsec_error.ok then message, "Error encountered: "+strtrim(err,2)
            
            if max(abs(switch_ff_command-dsp_ff_command)) ne 0.0 then begin
                err = test_skip_frame(/DSP)
                message, "Errore ricircolo ff_command su switch"
            endif

            err = read_seq_dsp(sc.all, rtr_map.ff_command_vector, adsec.n_actuators, dsp_ff_command_vector)
            if err ne adsec_error.ok then message, "Error encountered: "+strtrim(err,2)
 
            if max(abs(dsp_ff_command_vector-rebin(transpose(dsp_ff_command),adsec.n_actuators,n_dsp))) ne 0.0 then begin
                err = test_skip_frame(/DSP)
                message, "Errore ricircolo ff_command_vector su DSP"
            endif

        ;ff_current = adsec.ff_matrix ## transpose(ff_command)
            ff_current = mamano(adsec.ff_matrix,transpose(ff_command))

            err = read_seq_ch(sc.all, dsp_map.ff_current, 1l, dsp_ff_current)
            if err ne adsec_error.ok then message, "Error encountered: "+strtrim(err,2)

            if  max(abs(ff_current-dsp_ff_current)) ne 0.0 then $
              message, "Errore calcolo ff_current"

            old_vect = vect
            old_old_modes = old_modes
            old_modes = dsp_modes
            true_delayb = mamano(*rtr.b_delay_a_matrix,transpose(old_vect))
            ;true_delayb = *rtr.b_delay_a_matrix ## transpose(old_vect)
            true_delaya = mamano(((*rtr.a_delay_matrix)[*,*,0]),transpose(old_modes)) $
                         + mamano(((*rtr.a_delay_matrix)[*,*,1]),transpose(old_old_modes))

            ;true_delaya = (*rtr.a_delay_matrix)[*,*,0] ## transpose(old_modes) $
;                          + (*rtr.a_delay_matrix)[*,*,1] ## transpose(old_old_modes)

            err = read_seq_ch(sc.all, dsp_map.precalc_slope, 1l, precalc_slope)
            if err ne adsec_error.ok then message, "Error encountered: "+strtrim(err,2)
            err = read_seq_ch(sc.all, dsp_map.precalc_modes, 1l, precalc_modes)
            if err ne adsec_error.ok then message, "Error encountered: "+strtrim(err,2)
        
            if max(abs(true_delayb - precalc_slope)) ne 0.0 then $
              message, "Errore nel calcolo delle delay lines di slope"
            if max(abs(true_delaya - precalc_modes)) ne 0.0 then $
              message, "Errore nel calcolo delle delay lines di mode"

            delayB = precalc_slope
            delayA = precalc_modes

        endif      
        
        
        
    endif else begin

        ;Reading correct frames counter and setting param_selector
        err = read_seq_dsp(0,rtr_map.param_selector, 1l, bb, /UL)
        if err ne adsec_error.ok then message, "Error encountered: "+strtrim(err,2)
        bb = bb or 2UL^6
        if (bb and 2UL^12) then message, "Disable disturbance before!!!"
        err = write_same_dsp(sc.all,rtr_map.param_selector, bb)
        if err ne adsec_error.ok then message, "Error encountered: "+strtrim(err,2)

        if keyword_set(MIRROR) then begin 

            err = read_seq_dsp(0,rtr_map.mirror_frames_counter, 1l, bb, /UL)
            if err ne adsec_error.ok then message, "Error encountered: "+strtrim(err,2)
            err = write_same_dsp(sc.all,rtr_map.mirror_frames_counter, bb+1UL)
            if err ne adsec_error.ok then message, "Error encountered: "+strtrim(err,2)

        endif else begin

            err = read_seq_dsp(0,rtr_map.WFS_frames_counter, 1l, bb, /UL)
            if err ne adsec_error.ok then message, "Error encountered: "+strtrim(err,2)
            err = write_same_dsp(sc.all,rtr_map.WFS_frames_counter, bb+1UL)
            if err ne adsec_error.ok then message, "Error encountered: "+strtrim(err,2)

        endelse

        ;Writing null frames
        err = write_same_dsp(sc.all, rtr_map.slope_vector, fltarr(rtr.n_slope), /CHECK)    
        if err ne adsec_error.ok then message, "Error encountered: "+strtrim(err,2)
        err = start_rtr()
        if err ne adsec_error.ok then message, "Error encountered: "+strtrim(err,2)

        err = read_seq_ch(sc.all, dsp_map.modes, 1L, modes)
        if err ne adsec_error.ok then message, "Error encountered: "+strtrim(err,2)
        err = write_same_dsp(sc.all, rtr_map.modes_vector, modes, /CHECK)
        if err ne adsec_error.ok then message, "Error encountered: "+strtrim(err,2)
        err = start_mm()

        if err ne adsec_error.ok then message, "Error encountered: "+strtrim(err,2)
        err = read_seq_ch(sc.all, dsp_map.ff_command, 1L, ff_command)
        if err ne adsec_error.ok then message, "Error encountered: "+strtrim(err,2)
        err = write_same_dsp(sc.all, rtr_map.ff_command_vector, ff_command, /CHECK)    
        if err ne adsec_error.ok then message, "Error encountered: "+strtrim(err,2)
        err= start_ff()
        if err ne adsec_error.ok then message, "Error encountered: "+strtrim(err,2)

        err = start_dl(/DIAG)
        if err ne adsec_error.ok then message, "Error encountered: "+strtrim(err,2)

        delayB = precalc_slope
        delayA = precalc_modes

    endelse

    ;Checking for the skip frame conditions...
    ;if keyword_set(docheck) then begin
         if ~keyword_set(NOVERB) then err = test_skip_frame(bufskip)
        if err ne adsec_error.ok then message, "Error encountered: "+strtrim(err,2)
    ;endif
    if n_elements(sleep_time) then wait, sleep_time
    
    return

End
