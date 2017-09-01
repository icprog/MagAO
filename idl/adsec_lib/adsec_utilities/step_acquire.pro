Function step_acquire, tn

    if n_elements(tn) eq 0 then tn=tracknum()
    ao_status = ptr2value(!AO_STATUS)

    err = get_commands(sc.all, cmd, /OFFLOADCMD)
    if err ne adsec_error.ok then return, err
    err = get_commands(sc.all, force,/CURRCMD)
    if err ne adsec_error.ok then return, err
    if rtr.oversampling_time gt 0 then err = zero_ctrl_current(/OFFLOAD)    $ 
                                else  err = zero_ctrl_current()
    if err ne adsec_error.ok then return, err

    err= get_status(status_save)
    adsec_save = adsec
    save, adsec_save, status_save, ao_status, FILE=filepath(ROOT=dir, "adsec_save"+ext+".sav")

    default = read_ascii_structure(filepath(ROOT=adsec_path.conf,sub='scripts','step_default.txt'))
    ask_file = default.ask_file
    timepcur = default.pr_cur_set_time
    timepcmd = default.pr_cmd_set_time
    smooth=default.smooth

    ;setting up...
    ;file on output
    dir = meas_path('step', DAT=tn)
    ext = default.ext

    ;save the data if set
    do_save = default.do_save
    n_samples = default.n_samples
    trig = default.trig
    if timepcmd eq 0 then pr_cmd = dsp_const.fastest_preshaper_step else $
      pr_cmd = ((round(adsec.preshaper_len*adsec.sampling_time/timepcmd)  > 1) $
               < dsp_const.fastest_preshaper_step)
    if timepcur eq 0 then pr_cur = dsp_const.fastest_preshaper_step else $
      pr_cur = ((round(adsec.preshaper_len*adsec.sampling_time/timepcur)  > 1) $
               < dsp_const.fastest_preshaper_step)

    curr2save = default.curr2save
    max_t_range = default.max_t_range
    plot_all = default.plot_all
    pos_psym = default.pos_psym


    if default.act_list[0] ge 0 then begin
        type='actuator'
        act_list = default.act_list
        amp_list = replicate(default.act_amp, n_elements(act_list))

        for i=0,n_elements(act_list)-1 do begin
            number=act_list[i]
            amp=amp_list[i]
            if plot_all then oplot_list=adsec.act_w_cl else undefine, oplot_list
            err = get_step( type, number, amp, n_samples, trig, pr_cmd, pr_cur, response $
                            , EXT=ext, DIR=dir, NO_SAVE=(~ do_save), CURRENT2SAVE=curr2save $
                            , MAX_T=max_t_range, OVERPLOT_LIST=oplot_list, POS_PSYM=pos_psym, SMOOTH=smooth, NO_PLOT=default.no_plot)
            if err ne adsec_error.ok then return, err
            log_print,'ACT #'+strtrim(act_list[i])
            err=update_status()
            if err ne adsec_error.ok then return, err
            ;update_panels,/opt, /no_read
            wait,.1
        endfor
    endif


    if default.mod_list[0] ge 0 then begin
        type='modal'
        
        mod_list = default.mod_list
        ;if ask_file then begin
;
 ;           log_print, "Please select the amplitude file"
  ;          amp_list = readfits(dialog_pickfile())
   ;         if n_elements(amp_list) lt n_elements(mod_list) then begin 
       ;         message, "Wrong mode_list amp_list number." 
    ;        endif else begin
     ;           amp_list = amp_list[0:n_elements(mod_list)-1]
      ;      endelse

       ; endif else begin

            amp_list = replicate(default.mod_amp, n_elements(mod_list))

       ; endelse
    
    amp_list = amp_list * sqrt(total(adsec.ff_sval ne 0))

    if plot_all then oplot_list=indgen(n_elements(where(adsec.ff_sval ne 0.0))) else undefine, oplot_list


    f_mat = adsec.ff_f_svec[mod_list,*] ## (diagonal_matrix(adsec.ff_sval[mod_list] * reform(amp_list)))
  
    max_f_mat = max(abs(f_mat),DIM=2);tiene conto solo della corrente di ff e non di quella gia' presente
;    f_threshold = (adsec.weight_times4peak-1.0) * adsec.weight_curr; adsec.weight_times4peak* adsec.weight_curr
    f_threshold = default.f_threshold
    id_f = where(max_f_mat gt f_threshold, cc)

    log_print, "Max FF Force to apply before thresholding:", max_f_mat
    log_print, "Force threshold:", f_threshold

    if (cc ne 0) and (ask_file eq 0) then amp_list[id_f] *= (f_threshold / max_f_mat[id_f])


    for i=0,n_elements(mod_list)-1 do begin
        number=mod_list[i]
        amp=amp_list[i]
        ;print, mod_list[i], amp_list[i], max_f_mat[i]
        
        err = get_step2( type, number, amp, n_samples, trig, pr_cmd, pr_cur, response $
                        , EXT=ext, DIR=dir, NO_SAVE=(~ do_save), CURRENT2SAVE=curr2save $
                        , MAX_T=max_t_range, OVERPLOT_LIST=oplot_list, POS_PSYM=pos_psym, SMOOTH=smooth $
                        , NO_PLOT=default.no_plot, SAME_DISPLAY_CUTS=default.same_display_cuts)
        if err ne adsec_error.ok then return, err
        log_print,'MODE #'+strtrim(mod_list[i])
        err=update_status()
        if err ne adsec_error.ok then return, err

        wait,.1
    endfor
endif








end

