; $Id
;+
; HISTORY
;   08 Aug 2009: AR, added coma-x and coma-y to the list of default Zernike
;                polynomials to be filtered out from the flattening shape
;   30 aprile 2010: Runa Briguglio modified to implement application of MMMatrix provided by Fernando; keyword oimat added
;   20100903: acquisition of triggered samples from 4D is implemented. this is done through the function get_img4d.
;
;- 


Function mirror_flat, n_modes2flat,TRACK=track, ZERN_NOT_FLAT = zern_mode2not_flatten, SKIP=skipvect, NMEAS=nmeas, INTER=inter, OFFLINE=offline, FACTOR=factor

    @adsec_common
    
    if n_elements(nmeas) eq 0 then nmeas=5
    if n_elements(track) eq 0 then track=''    
        file= dialog_pickfile(PATH=adsec_path.meas+'/flat/'+track, TITLE='Please select the SINGLE file to reduce:')
        print, file
            if file eq "" then begin
                message, "ERROR" 
            endif else begin
                restore, file, /ver
            endelse
        
    if n_elements(zern_mode2not_flatten) eq 0 then zern_mode2not_flatten = [1,2,3];[1,2,3,4,5,6,7,8]; was [1,2,3]
    if n_elements(n_modes2flat) eq 0 then n_modes2flat = n_modes
    if n_modes2flat gt n_modes then begin
        print, "Wrong modes number flat request: reducing to modes #", n_modes
        n_modes2flat = n_modes
    endif
    if ~keyword_set(OFFLINE) then begin
        err = get_status(status_save_init)
        if err ne adsec_error.ok then return, err
    endif

    opd2flatten=get_img4d(nmeas=nmeas, out=mask2flatten)
    
    sopd = size(opd2flatten, /dim)
    xx=rebin((findgen(sopd[0])-pupil_xyre[0])/pupil_xyre[2],sopd,/SAMP)
    yy=rebin((findgen(1,sopd[1])-pupil_xyre[1])/pupil_xyre[2],sopd,/SAMP)
    is_new=1
    if n_elements(is_new) eq 0 then opd2flatten /=2; spiegare il significato
    idx = where(mask2flatten)

    fit=surf_fit(xx[idx],yy[idx],opd2flatten[idx],zern_mode2not_flatten,/ZERN)      ;removes piston, tip, tilt and focus

    window, /FREE
    image_show, opd2flatten*float(mask2flatten), /sh, /as, TITLE='OPD2FLATTEN '
    opd2flatten[idx]=opd2flatten[idx]-fit

    window, /FREE
    image_show, opd2flatten*float(mask2flatten), /sh, /as, TITLE='OPD2FLATTEN TILT REMOVED'

    mask2flatten = mask2flatten[x0:x1, y0:y1]
    dummy= fltarr(n_pix_x, n_pix_y)
    dummy[0,0] = mask2flatten
    r_weight2flatten = rebin(dummy, n_meta_pix_x, n_meta_pix_y)
    r_mask2flatten = r_weight2flatten ge 0.6
    opd2flatten = opd2flatten[x0:x1, y0:y1]
    dummy = fltarr(n_pix_x, n_pix_y)
    dummy[0,0] = opd2flatten
    r_opd2flatten = rebin(dummy, n_meta_pix_x, n_meta_pix_y)
    idx = where(r_weight2flatten gt 0)
    r_opd2flatten[idx]=r_opd2flatten[idx]/r_weight2flatten[idx]


    r_xx = xx[x0:x1, y0:y1]
    dummy = fltarr(n_pix_x, n_pix_y)
    dummy[0,0] = r_xx
    r_xx = rebin(dummy, n_meta_pix_x, n_meta_pix_y)
    
    r_yy = yy[x0:x1, y0:y1]
    dummy = fltarr(n_pix_x, n_pix_y)
    dummy[0,0] = r_yy
    r_yy = rebin(dummy, n_meta_pix_x, n_meta_pix_y)
    
    if n_elements(intersect_mask_idx) eq 0 then begin ; modificare per mascherare solo sui modi da applicare
        intersect_mask_idx = where((total(float(data.r_mask_cube[0:n_modes2flat-1, *]), 1) eq n_modes2flat)*r_mask2flatten[*], count)
        saved_intersect_mask_idx = intersect_mask_idx
    endif else begin
        saved_intersect_mask_idx = intersect_mask_idx
        intersect_mask_idx = where((total(float(data.r_mask_cube[0:n_modes2flat-1,*]), 1) eq n_modes2flat)*r_mask2flatten[*], count)
    endelse
    if count eq 0 then message, "ERROR"
    intersect_mask=fltarr(n_pix_x, n_pix_y)
    intersect_mask[intersect_mask_idx]= 1
       
        for cc=0, n_elements(data.r_opd_cube[*,0])-1 do begin
            ;data.r_opd_cube[cc,intersect_mask_idx] -= mean(data.r_opd_cube[cc,intersect_mask_idx])
            fit=surf_fit(r_xx[intersect_mask_idx],r_yy[intersect_mask_idx],data.r_opd_cube[cc,intersect_mask_idx],[1,2,3],/ZERN)
            data.r_opd_cube[cc,*] *= intersect_mask[*]
            data.r_opd_cube[cc,intersect_mask_idx] -= fit
        endfor

        tmp_r_opd = data.r_opd_cube[0:n_modes2flat-1,*]
        r_opd = temporary(tmp_r_opd[*,intersect_mask_idx])
        diag_amp = extract_diagonal(data.applied_mode)
        diag_amp = diag_amp[0:n_modes2flat-1]
        int_mat = r_opd ## diagonal_matrix(1d/diag_amp)
        undefine, r_opd
        eps = 0.1;(machar()).eps

        rec_mat = pseudo_invert(int_mat, /VERB,  V_MAT=v, U=u, W=s, EPS=eps, COUNT=s_count, IDX=s_idx, /DOUBLE, INTER=inter) 
        if s_count gt 0 then print, s_idx


        flatten_modes = rec_mat ## r_opd2flatten[intersect_mask_idx]
        if n_elements(skipvect) gt 0 then flatten_modes[skipvect]=0

        if n_elements(factor) eq 0 then factor =1.
        delta_comm2flatten = factor*float(adsec_save.ff_p_svec[0:n_modes2flat-1, *] ## flatten_modes[0:n_modes2flat-1]); mod RB at LBTO: status_save _init is no more provided;;;; 4 added!!!!! Negative added!
        delta_ff_force2flatten= float(adsec.ff_matrix ## delta_comm2flatten)

 
        window,/free
        display, -delta_comm2flatten[adsec.act_w_cl],adsec.act_w_cl, /SH, /SM,/refl,rot=240 $
           , TIT="-Delta-command for flattening", /no_n

        window,/free
        display, -delta_ff_force2flatten[adsec.act_w_cl],adsec.act_w_cl, /SH $
           , TIT="Feed-forward forces for flattening", /no_n

        if ~keyword_set(OFFLINE) then begin
            ans = dialog_message(["Flattening requests max force="+strtrim(max(abs(delta_ff_force2flatten)),2)+"." $
                         , "Do you want to apply it?"], /QUEST)

            if strlowcase(ans) eq "yes" then begin
                err = set_offload_cmd(-delta_comm2flatten[adsec.act_w_pos], /NOCH, /DELTA)
                if err ne adsec_error.ok then message, "ERROR!!"
                err = set_offload_force(-delta_ff_force2flatten[adsec.act_w_curr],/START, /NOCH, /DELTA)
                if err ne adsec_error.ok then message, "ERROR!!"
                wait, 2*rtr.oversampling_time

                flattened_opd=get_img4d(nmeas=nmeas, out=flattened_mask)
                err = update_status()
                flattened_status = sys_status

                ans = dialog_message(["Do you want to go back to the previous shape?"], /QUEST)
                if strlowcase(ans) eq "yes" then begin
                    err = set_offload_cmd(delta_comm2flatten[adsec.act_w_pos], /NOCH, /DELTA)
                    if err ne adsec_error.ok then message, "ERROR!!"
                    err = set_offload_force(delta_ff_force2flatten[adsec.act_w_curr],/START, /NOCH, /DELTA)
                    if err ne adsec_error.ok then message, "ERROR!!"
                    wait, 2*rtr.oversampling_time
                endif
            endif

        endif

        if ~keyword_set(OFFLINE) then begin
            ans = dialog_message(["Do you want to save data?"], /QUEST)
            if strlowcase(ans) eq "yes" then begin
                data_ver=2  ;data_ver=2: variables added: track,zern_mode2not_flatten, data_ver. opd2flatten is the mean of triggered samples in 4d/Zcopy/img4d/hdf5
                save_file_dir=meas_path('flat')
                save_file=save_file_dir+'flattening_result.sav'
                log_print, 'Flattening data saved in file '+ save_file
                flattening_modes = adsec.ff_p_svec[0:n_modes2flat-1, *]; da modificare????
                status_save = status_save_init
                save, FILE=save_file, n_modes, data, rec_mat, int_mat, s, s_idx, s_count, v, pupil_xyre  $
                      , opd2flatten, mask2flatten , n_pix_x, n_pix_y $
                      , flattened_opd, flattened_mask, flattened_status $
                      , mask_cx, mask_cy, xx, yy, x0, x1, y0, y1 , is_new $
                      , n_meta_pix_x, n_meta_pix_y, flattening_modes, max_rms $
                      , n_pix_diam_x, n_pix_diam_y, min_pix,intersect_mask_idx, status_save, track,zern_mode2not_flatten, adsec_save, data_ver
                save_file=save_file_dir+'flattened_status.sav'
                save, FILE=save_file, flattened_status, data_ver,track,zern_mode2not_flatten
            endif
        endif
end
