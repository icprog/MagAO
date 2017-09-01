;$Id: find_touch.pro,v 1.5 2009/07/28 14:21:34 labot Exp $$
;+
;   NAME:
;    FIND_TOUCH
;
;   PURPOSE:
;    Find possible dust contamination in the gap between thin-shell and reference plate.
;
;   USAGE:
;    err = find_touch()
;
;   NB
;    ADD the saturation check
Function find_touch, name, delta_pos, idx_good, VERBOSE=verbose, POS0=pos0, POS1=pos1, NO_SAVEFILE=no_savefile

    @adsec_common

    delta_curr = adsec.weight_curr *0.5
    if n_elements(name) eq 0 then name = "touch"
    ;STOP MASTER DIAGNOSTIC, LOAD THE
    ;1/8th REFERENCE WAVE, PUT COMMAND INTO BIAS AND START MASTER
    ;DIAGNOSTIC

    ;ENABLE COILS AND DISABLE TSS
   
    ;UPDATE_STATUS on local gap

    ;APPLY FORCE TO PUSH THE SHELL AGAINST THE REFERENCE BODY

    ;UPDATE_STATUS on new gap

    ;STOP MASTER DIAGNOSTIC, RELOAD THE
    ;FULL REFERENCE WAVE, PUT COMMAND INTO BIAS AND START MASTER
    ;DIAGNOSTIC

    
    old_ovs = rtr.oversampling_time

    err = set_diagnostic(over=0.0, MASTER=0)
    if err ne adsec_error.ok then print, "Message ERROR!!!"
    wait, 2*old_ovs  
   ; loading the 1/8 wave reference

   filename=filepath(ROOT=adsec_path.commons, sub=['ref_wave'], 'wave_1o8.txt')

   disp_mess, 'Disabling DSP WATCHDOG...',/APPEND
   err = disable_watchdog(/DSP)
            if err ne adsec_error.ok then begin
                disp_mess, '... error on  DSP watchdog disabling.', /APPEND
                return, err 
            endif
   disp_mess, '... done.', /APPEND


   err = init_adsec_wave(filename, /VER)
            if err ne adsec_error.ok then begin
                disp_mess, 'Reference wave parameters initialization failed!', /APPEND
                return, err
            endif
   disp_mess, 'Uploading reference wave paramenters...', /APPEND
            err = send_wave()
            if err ne adsec_error.ok then begin
                disp_mess, 'Reference wave parameters initialization failed!', /APPEND
                return, err
            endif


   err =  init_adsec_cals()
            if err ne adsec_error.ok then begin
                disp_mess, 'Calibration failed!', /APPEND
                return, err
            endif
   err = send_linearization()
             if err ne adsec_error.ok then begin
                 disp_mess, '... error on uploading the linearization coeffs.', /APPEND
                 return, err
             endif
    disp_mess, '... done.', /APPEND
    disp_mess, 'Enabling DSP WATCHDOG...',/APPEND
             err = enable_watchdog(/DSP)
             if err ne adsec_error.ok then begin
                 disp_mess, '... error on  DSP watchdog enabling.', /APPEND
                 return, err
             endif
    disp_mess, '... done.', /APPEND
  ; end of wave change


    err = get_ave_pc(ave_pos, ave_curr)
    if err ne adsec_error.ok then return, err
    pos0 = ave_pos

    err = adam_enable_coils_dis_tss()
    

    err = write_bias_curr(sc.all, delta_curr)
    if err ne adsec_error.ok then return, err

    wait, 1.

    err = get_ave_pc(ave_pos, ave_curr)
    if err ne adsec_error.ok then return, err
    pos1 = ave_pos

    err = adam_disable_coils()

    err = rip()
    if err ne adsec_error.ok then return, err

    delta_pos=pos1-pos0

;    idx_no_sat = where((pos0 ne -32768) and (pos1 ne -32768), c_no_sat)
;    if c_no_sat eq 0 then message,'All the actuators are saturated'
    idx_no_sat = indgen(adsec.n_actuators)
    err = complement(adsec.act_wo_pos, indgen(adsec.n_actuators), new_act_list)

    is_empty = intersection(idx_no_sat, new_act_list, idx_good)
    if is_empty then message,'No valid actuator to display'

    if ~keyword_set(NO_SAVEFILE) then begin
        ff=meas_path('touch')+name+'.sav'
        print, ff
        save, file=ff, delta_pos, pos0, pos1, idx_good
    endif 

    if keyword_set(verbose) then begin
        minv = min([pos0[idx_good],pos1[idx_good]], MAX=maxv)

        if (!d.flags and 256) eq 256 then window,/free,xs=1024,ys=768
        display, delta_pos[idx_good]*1e6,idx_good,/sm,/sh, tit='Delta Position [!7l!3m]', num_type=0, XTITLE='[mm]', YTITLE='[mm]',  BAR_TITLE='[!7l!3m]'
        if (!d.flags and 256) eq 256 then window,/free,xs=1024,ys=768
        display, pos0[idx_good]*1e6,idx_good,/sh,/sm,tit='Initial Position [!7l!3m]', MIN=minv*1e6, MAX=maxv*1e6, num_type=0, XTITLE='[mm]', YTITLE='[mm]', BAR_TITLE='[!7l!3m]'
        if (!d.flags and 256) eq 256 then window,/free,xs=1024,ys=768
        display, pos1[idx_good]*1e6,idx_good,/sh,/sm,tit='Final Position [!7l!3m]', MIN=minv*1e6, MAX=maxv*1e6, num_type=0, XTITLE='[mm]', YTITLE='[mm]', BAR_TITLE='[!7l!3m]'
    
;        display, dpos[adsec.act_w_cl], adsec.act_w_cl, /sm, pos=fit_dpos, map=256
;         display, pos0[adsec.act_w_cl], adsec.act_w_cl, /sm, pos=fit_pos0, map=256
;         display, pos1[adsec.act_w_cl], adsec.act_w_cl, /sm, pos=fit_pos1, map=256
; print, minmax(fit_pos0[where(abs(fit_dpos) lt 0.1e-6 )])
; print, minmax(fit_pos0[where(abs(fit_dpos) lt 0.2e-6 )])
; print, minmax(fit_pos0[where(abs(fit_dpos) lt 0.3e-6 )])
;         mask1 = abs(fit_dpos) lt 0.2e-6
;         image_show, fit_pos0*float(mask1), /sh
;         mask2 = fit_pos1 gt 30e-6
;         image_show, fit_pos0*float(mask1*mask2), /sh

    endif
    window, /free
    print, load_wave(1)
    err = set_diagnostic(over=old_ovs, /MASTER)
    if err ne adsec_error.ok then print, "Message ERROR!!!"
    wait, 0.5

    return,adsec_error.ok

end







