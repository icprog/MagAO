;$Id: opt_int_mat_acquire.pro,v 1.4 2009/08/06 10:50:55 labot Exp $
;Script per lo spianamento dello specchio
;January 2006, M.Xompero and D.Zanotti
; In the save_file flattened_mask is saved.
;   MX,DZ & AR
; Changed the script in a pro.
;

Function opt_int_mat_acquire, start_mode, end_mode, MAX_RMS=max_rms, PUPIL=pupil_xyre, ZONAL=zonal, SUFFIX=suffix_str $
  , ACCEPT_PIXELS=accept_pix, MAX_PEAK_FORCE=max_peak_force_value, CAST_FF2FLOAT=do_cast

    @adsec_common
    tn=tracknum()
    zonal_mat = identity(adsec.n_actuators)
    list1 = adsec.mir_act2act
    mir_cl = adsec.act2mir_act[adsec.act_w_cl]
    print, intersection(list1, mir_cl, outl)
    mir_cl_list = adsec.mir_act2act[outl]
    zonal_mat = zonal_mat[mir_cl_list,*]
    
    if n_elements(suffix_str) eq 0 then suffix_str = ""

    root_4d = '$ADOPT_MEAS/adsec_calib/4d/Zcopy'
    dir = meas_path('flat', date=tn)
    print, dir


    if (file_info(root_4d)).exists eq 0 then file_mkdir, root_4d
    if (file_info(root_4d+'/SINGLE_MEAS')).exists eq 0 then begin
        message, "PLEASE MOUNT THE INTERFEROMETER HD on /home/labot/4d", /INFO
        return, adsec_error.generic_error
    endif
    n_modes=end_mode-start_mode+1
    if n_elements(max_rms) eq 0 then max_rms = 300e-9
    max_rms = max_rms < 300e-9

    if keyword_set(zonal) then begin
        max_amp = max_rms
    endif else begin
        max_amp = max_rms*sqrt(n_elements(adsec.act_w_cl))
    endelse

    if n_elements(max_peak_force_value) eq 0 then begin 
        max_peak_force= 3*adsec.weight_curr
    endif else begin
        max_peak_force=max_peak_force_value
    endelse

    ;INITIAL STATUS SAVING
    err = get_status(status_save_init)
    if err ne adsec_error.ok then return, err

    ;INTERFEROMETER CONFIGURATION
    interf_type = '4d'
    thr=0.5
    thr_phase = !PI
    n_frames = 1


    ;4D INTERFACE INITIALIZATION
    err = idl_4d_init()
    if err ne adsec_error.ok then return, err


    ;BASE FRAME ACQUISITION
    good_found = 0B
    max_trial = 5
    n_trial = 0
    repeat begin
        name_radix = 'base_frame'
        err = get_m4d(name_radix, opd, mask, VALUES=values)
        if err ne adsec_error.ok then return, err

        good_pix = total(mask)
        image_show, opd, /AS, /SH
        if keyword_set(accept_pix) then ans = "yes" else begin
            ans = DIALOG_MESSAGE( 'The number of good pixels is '+strtrim(round(good_pix),2)+' accept ?', /QUESTION )
        endelse
        if strlowcase(ans) eq 'yes' then begin
            good_found=1B
            min_pix=0.9*good_pix
        endif
        n_trial=n_trial+1
    endrep until good_found or (n_trial eq max_trial)

    if not good_found then begin
        err = idl_4d_finalize()
        if err ne adsec_error.ok then return, err
        message, "ERROR! No good base frame found!", /INFO
        return, adsec_error.generic_error
    endif

    if n_elements(pupil_xyre) eq 0 then begin
        err = idl_4d_finalize()
        if err ne adsec_error.ok then return, err
        if start_mode ne 0 then begin
            message, "ERROR! To continue an interaction matrix acquisition you MUST pass the pupil parameters.", /INFO
            return, adsec_error.generic_error
        endif
        pupil_xyre = pupilfit(mask, /circle, /DISP, GUESS=[500, 500, 400, 0.05])
        ans = DIALOG_MESSAGE( 'Was the pupil fit ok?', /QUESTION )
        if strlowcase(ans) ne 'yes' then return, adsec_error.generic_error
        err = idl_4d_init()
        if err ne adsec_error.ok then return, err
    endif

    ;BASE FRAME ANALISYS FOR DATA SIZE REDUCTION
;    eps = adsec_shell.in_radius/adsec_shell.out_radius
    aspect = values.measurement0.genraw.aspect._data
;    n_pix_diam_x = 2*sqrt(total(mask)*aspect/(!PI*(1-eps^2)))
;    n_pix_diam_y = n_pix_diam_x/aspect
    n_pix_diam_x = pupil_xyre[2]*2
    n_pix_diam_y = pupil_xyre[2]*2
    
    nx_pix4d = (size(mask, /DIM))[0]
    ny_pix4d = (size(mask, /DIM))[1]

    xx = rebin(findgen(nx_pix4d), nx_pix4d, ny_pix4d, /SAMP)
    yy = rebin(findgen(1, ny_pix4d), nx_pix4d , ny_pix4d, /SAMP)
    mask_cx = total(mask*xx)/total(mask)
    mask_cx = pupil_xyre[0]
;    mask_cy = total(mask*yy)/total(mask)
    mask_cy = pupil_xyre[1]
    xx = (xx-mask_cx)/ceil(n_pix_diam_x/2)
    yy = (yy-mask_cy)/ceil(n_pix_diam_y/2)

    x0 = round(mask_cx - 1.1*n_pix_diam_x/2) > 0
    x1 = round(mask_cx + 1.1*n_pix_diam_x/2) < (nx_pix4d-1)
    y0 = round(mask_cy - 1.1*n_pix_diam_y/2) > 0
    y1 = round(mask_cy + 1.1*n_pix_diam_y/2) < (ny_pix4d-1)

    n_meta_pix_diam = 4*(2*adsec_shell.out_radius)/((adsec_shell.out_radius-adsec_shell.in_radius)/adsec_shell.n_rings)

    pix_per_meta_pix_x = round(n_pix_diam_x/n_meta_pix_diam) > 1
    pix_per_meta_pix_y = round(n_pix_diam_y/n_meta_pix_diam) > 1

    n_pix_x = ceil(float(x1-x0+1)/pix_per_meta_pix_x)*pix_per_meta_pix_x
    n_pix_y = ceil(float(y1-y0+1)/pix_per_meta_pix_y)*pix_per_meta_pix_y

    n_meta_pix_x = n_pix_x/pix_per_meta_pix_x
    n_meta_pix_y = n_pix_y/pix_per_meta_pix_y

    
    cl_act = where(sys_status.closed_loop, n_cl_act)
    cl_act=adsec.act_w_cl
    n_cl_act = n_elements(cl_act)
    if n_cl_act eq 0 then begin
        message, "No actuator in closed loop", CONT=(sc.debug eq 0B)
    ;    return, err
    endif

    
    ;DATA ACQUISITION
    data = {                                                        $
        applied_mode : fltarr(n_modes, n_modes),                    $
        r_opd_cube   : fltarr(n_modes, n_meta_pix_x*n_meta_pix_y),  $
        r_mask_cube  : bytarr(n_modes, n_meta_pix_x*n_meta_pix_y),  $
        position     : fltarr(n_modes, adsec.n_actuators, 3),       $
        current      : fltarr(n_modes, adsec.n_actuators, 3)        $
    }

    time_data = time_estimator(n_modes, /OPTIMIZE, /RESET)
    time_presh = time_preshaper() > 0.01
    for i=0, n_modes-1 do begin
        
        ;if (((i / 50) eq i/50.) and (i gt 49)) then begin ;or (i eq 0) then begin
        ;    err = hextrack(/APPLY)
            err= idl_4d_init()
        ;    wait, 10
        ;endif

        if keyword_set(zonal) then begin
            mode2apply = max_amp*zonal_mat[start_mode+i,*] ;; actuator eith CL!!!
        endif else begin
            ;max_amp = (900d-9*sqrt(n_elements(adsec.act_w_cl)))*((start_mode+i+1)/10.0)^(-0.66)
            max_amp = (800d-9*sqrt(n_elements(adsec.act_w_cl)))*((start_mode+i+1)/10.0)^(-0.66)
            max_amp = max_amp < (max_rms*sqrt(n_elements(adsec.act_w_cl)))
            max_amp = max_amp > (90d-9*sqrt(n_elements(adsec.act_w_cl)))
            mode2apply = max_amp*adsec.ff_p_svec[start_mode+i,*]
        endelse

                                ;force2apply = float(adsec.ff_sval[start_mode+i]*max_amp*adsec.ff_f_svec[start_mode+i,*])
        if keyword_set(do_cast) then begin
            mode2apply = float(mode2apply)
            force2apply = float(adsec.ff_matrix) ## mode2apply
        endif else begin        
            force2apply = float(adsec.ff_matrix ## mode2apply)
        endelse
        
        maxf = max(abs(force2apply))
        if maxf ne 0.0 then begin
            scale = (max_peak_force/maxf) < 1.0
            amp = scale*max_amp
            mode2apply = scale*mode2apply
            force2apply = scale*force2apply

            window,0
            display, mode2apply[adsec.act_w_cl],adsec.act_w_cl ,/SH, /SM, title='Mode #'+strtrim(start_mode+i)
        endif else begin
            scale = 1.0
            amp = max_amp
        endelse
        mode2apply = float(mode2apply)
        force2apply = float(force2apply)

        print, "PEAK FORCE [N]:", strtrim(max(abs(force2apply)),2)
        if keyword_set(zonal) then begin
            print, "AMP RMS   [nm]:", strtrim(amp/sqrt(n_elements(adsec.act_w_cl))*1e9,2)
        endif else begin
            print, "AMP       [nm]:", strtrim(amp*1e9,2)
        endelse
        print, "SCALE value   :", strtrim(scale,2)
        err = set_offload_cmd(mode2apply[adsec.act_w_pos], /NOCH, /DELTA)
        if err ne adsec_error.ok then message, "ERROR!!"
        err = set_offload_force(force2apply[adsec.act_w_curr],/START, /DELTA, /NOCH)
        if err ne adsec_error.ok then message, "ERROR!!"
        wait, 2*rtr.oversampling_time+time_presh
        err = update_status()
        if err ne adsec_error.ok then message, "ERROR!!"
        s_tmp0 = sys_status
        wait, 2*rtr.oversampling_time+time_presh

        name_radix = "tempo0"
        err = get_m4d(name_radix, opd0, mask0, MIN_PIX=min_pix, /TAKE_MEAS)
        if err ne adsec_error.ok then message, "ERROR!!"

        err = set_offload_cmd(-2*mode2apply[adsec.act_w_pos], /NOCH, /DELTA)
        if err ne adsec_error.ok then message, "ERROR!!"
        err = set_offload_force(-2*force2apply[adsec.act_w_curr],/START, /DELTA, /NOCH)
        if err ne adsec_error.ok then message, "ERROR!!"
        wait, 2*rtr.oversampling_time+time_presh
        err = update_status()
        if err ne adsec_error.ok then message, "ERROR!!"
        s_tmp1 = sys_status
        wait, 2*rtr.oversampling_time+time_presh

        name_radix = "tempo1"
        err = get_m4d(name_radix, opd1, mask1, MIN_PIX=min_pix, /TAKE_MEAS)
        if err ne adsec_error.ok then message, "ERROR!!"

        err =  set_offload_cmd(2*mode2apply[adsec.act_w_pos], /NOCH, /DELTA)
        if err ne adsec_error.ok then message, "ERROR!!"
        err = set_offload_force(2*force2apply[adsec.act_w_curr],/START, /DELTA, /NOCH)
        if err ne adsec_error.ok then message, "ERROR!!"
        wait, 2*rtr.oversampling_time+time_presh
        err = update_status()
        if err ne adsec_error.ok then message, "ERROR!!"
        s_tmp2 = sys_status
        wait, 2*rtr.oversampling_time+time_presh

        name_radix = "tempo2"
        err = get_m4d(name_radix, opd2, mask2, MIN_PIX=min_pix, /TAKE_MEAS)
        if err ne adsec_error.ok then message, "ERROR!!"

        err =  set_offload_cmd(-mode2apply[adsec.act_w_pos], /NOCH, /DELTA)
        if err ne adsec_error.ok then message, "ERROR!!"
        err = set_offload_force(-force2apply[adsec.act_w_curr],/START, /DELTA, /NOCH)
        if err ne adsec_error.ok then message, "ERROR!!"
        wait, 2*rtr.oversampling_time + time_presh


        ;NOW READING FILES
        name_radix = "tempo0"
        err = get_m4d(name_radix, opd0, mask0, MIN_PIX=min_pix, /READ_FILE)
        if err ne adsec_error.ok then message, "ERROR!!"

        name_radix = "tempo1"
        err = get_m4d(name_radix, opd1, mask1, MIN_PIX=min_pix, /READ_FILE)
        if err ne adsec_error.ok then message, "ERROR!!"

        name_radix = "tempo2"
        err = get_m4d(name_radix, opd2, mask2, MIN_PIX=min_pix, /READ_FILE)
        if err ne adsec_error.ok then message, "ERROR!!"

; moved above
;        err =  set_offload_cmd(-mode2apply[adsec.act_w_pos], /NOCH, /DELTA)
;        if err ne adsec_error.ok then message, "ERROR!!"
;        err = set_offload_force(-force2apply[adsec.act_w_curr],/START, /DELTA, /NOCH)
;        if err ne adsec_error.ok then message, "ERROR!!"
;        wait, 2*rtr.oversampling_time + time_presh

        mask = mask0*mask1*mask2
        mask = mask[x0:x1, y0:y1]

        opd = ((opd0-opd1)+(opd2-opd1))/2
        opd = opd[x0:x1, y0:y1]
        opd_dummy = fltarr(n_pix_x, n_pix_y)
        opd_dummy[0,0] = opd

        mask_dummy= fltarr(n_pix_x, n_pix_y)
        mask_dummy[0,0] = mask
        opd_dummy=opd_dummy*mask_dummy

        
        WINDOW, 1
        image_show, opd_dummy, /SH, /AS,/EQUALIZE,  TITLE='OPD READ EQUALIZED'


        r_mask = rebin(mask_dummy, n_meta_pix_x, n_meta_pix_y)
        data.r_mask_cube[i,*]= r_mask ge 0.3

        r_opd = rebin(opd_dummy, n_meta_pix_x, n_meta_pix_y)
        idx = where(r_mask gt 0)
        r_opd[idx] = r_opd[idx]/r_mask[idx]
        data.r_opd_cube[i,*] = r_opd

        data.applied_mode[i,i] = amp*2
        data.position[i,*,0] = s_tmp0.position
        data.position[i,*,1] = s_tmp1.position
        data.position[i,*,2] = s_tmp2.position
        data.current[i,*,0] = s_tmp0.current
        data.current[i,*,1] = s_tmp1.current
        data.current[i,*,2] = s_tmp2.current
        window,2
        image_show, reform(data.r_opd_cube[i,*],n_meta_pix_x, n_meta_pix_y), /SH, /AS, /EQUALIZ, TITLE='OPD REBINNED EQUALIZED'
        time_str = strtrim(string(time_estimator(i,time_data, unit=unit_str)),2)
        print, "Time ESTIMATION: "+time_str+" "+unit_str


    endfor

 ;FINAL STATUS SAVING
    err = get_status(status_save_end)
    if err ne adsec_error.ok then return, err

    is_new=1
    
    zonal=keyword_set(zonal)
    if zonal then begin
        save, file = dir+'temp_data_opt_zonal_'+string(start_mode, format='(I3.3)')+"_"+string(end_mode, format='(I3.3)')+suffix_str+".sav" $ 
              ,status_save_init, status_save_end, data, cl_act, n_modes, opd, mask , is_new $
              , xx, yy, n_pix_x, n_pix_y, n_meta_pix_x, n_meta_pix_y, mask_cx, mask_cy $
              , x0, x1, y0, y1, xx, yy, max_rms, min_pix, n_pix_diam_x, n_pix_diam_y, pupil_xyre, start_mode, end_mode, zonal, zonal_mat, mir_cl_list
    endif else begin
        save, file = dir+'temp_data_opt_'+string(start_mode, format='(I3.3)')+"_"+string(end_mode, format='(I3.3)')+suffix_str+".sav" $ 
              ,status_save_init, status_save_end, data, cl_act, n_modes, opd, mask , is_new $
              , xx, yy, n_pix_x, n_pix_y, n_meta_pix_x, n_meta_pix_y, mask_cx, mask_cy $
              , x0, x1, y0, y1, xx, yy, max_rms, min_pix, n_pix_diam_x, n_pix_diam_y, pupil_xyre, start_mode, end_mode, zonal
    endelse
    return, adsec_error.ok


end
