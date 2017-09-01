; $Id
;+
; HISTORY
;   08 Aug 2009: AR, added coma-x and coma-y to the list of default Zernike
;                polynomials to be filtered out from the flattening shape
;   30 aprile 2010: Runa Briguglio modified to implement application of MMMatrix provided by Fernando; keyword oimat added
;- 


Function opt_int_mat_reduce2, n_modes2flat,TRACK=track, ZERN_NOT_FLAT = zern_mode2not_flatten, MULTIPLE_ACQ=multiple_acq, SKIP=skipvect, NMEAS=nmeas, INTER=inter, CHECK=check, OFFLINE=offline, FACTOR=factor, MANUAL=manual, AVETIME=avetime, SPHERic=spheric, n2drop=n2drop

    @adsec_common
    if n_elements(factor) eq 0 then scale_factor=0.5 else scale_factor=factor
    if n_elements(nmeas) eq 0 then nmeas=5
    if n_elements(track) eq 0 then track=''    
        file= dialog_pickfile(PATH=adsec_path.meas+'/flat/'+track, TITLE='Please select an IntMat to reduce:')
        print, file
            if file eq "" then begin
                message, "ERROR" 
            endif else begin
                restore, file, /ver
                flattening_file=file
            endelse
        
    if n_elements(zern_mode2not_flatten) eq 0 then zern_mode2not_flatten = [1,2,3,4,7,8];[1,2,3,4,5,6,7,8]; was [1,2,3]
    if n_elements(n_modes2flat) eq 0 then n_modes2flat = n_modes
    if n_modes2flat gt n_modes then begin
        print, "Wrong modes number flat request: reducing to modes #", n_modes
        n_modes2flat = n_modes
    endif
    if not keyword_set(OFFLINE) then begin
        err = get_status(status_save_init)
        if err ne adsec_error.ok then return, err
    endif


    if not keyword_set(manual) then begin
        opd2flatten=get_img4d(nmeas=nmeas, out=mask2flatten, basetime=avetime)
    endif else begin
        default_file=adsec_path.conf+'scripts/4d_opt_intmat_default.txt'
        def=read_ascii_structure(default_file)
        opd2read='$ADOPT_MEAS/adsec_calib/4d/'+def.folder4d+def.flat4d+'/'+def.foldh5
        flist=file_search(opd2read, '*.h5')
        nfile=n_elements(flist)
        n2ave=nfile
        for i=0, nfile-1 do begin
            print, 'reading file'+flist[i]
            opd=read_opd(flist[i], bad=mask, type='4d')
            if i eq 0 then begin
                totopd=opd*0.
                totmask=mask*0.+1
                nn=n_elements(opd)
            endif
            idx=where(mask eq 1B)
            if n_elements(idx)/float(nn) lt 0.5 then begin
                log_print, '4D Laser dimmed during the acquisition, this file will be discarted'
                n2ave -= 1
            endif else begin
                totopd  += opd
                totmask *= mask
            endelse
        endfor
        opd2flatten = totopd/n2ave
        mask2flatten= totmask
    endelse
    
    ;opd2flatten=congrid(opd2flatten, size_ini[0], size_ini[1],/inter) ; inserire controllo pupilla rispetto alla int mat
    ;mask2flatten=congrid(mask2flatten, size_ini[0], size_ini[1],/inter); rebin: any difference in images sizes is due to HW rebin on the 4D, so ol and new sizes are in integer factor....
    sopd = size(opd2flatten, /dim)
    xx=rebin((findgen(sopd[0])-pupil_xyre[0])/pupil_xyre[2],sopd,/SAMP)
    yy=rebin((findgen(1,sopd[1])-pupil_xyre[1])/pupil_xyre[2],sopd,/SAMP)
    is_new=1
    if n_elements(is_new) eq 0 then opd2flatten /=2; spiegare il significato
    idx = where(mask2flatten)
    fit=surf_fit(xx[idx],yy[idx],opd2flatten[idx],zern_mode2not_flatten,/ZERN)      ;removes piston, tip, tilt and focus
    
    if (n_elements(spheric) ne 0) then begin
         sph_fit=zern(11,xx[idx],yy[idx])*spheric[0]+ zern(22,xx[idx],yy[idx])*spheric[1]     ;removes piston, tip, tilt and focus
         opd2flatten[idx]+=sph_fit
    endif

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
    
    if n_elements(intersect_mask_idx) eq 0 then begin
        intersect_mask_idx = where((total(float(data.r_mask_cube[0:n_modes2flat-1, *]), 1) eq n_modes2flat)*r_mask2flatten[*], count)
        saved_intersect_mask_idx = intersect_mask_idx
    endif else begin
        saved_intersect_mask_idx = intersect_mask_idx
        intersect_mask_idx = where((total(float(data.r_mask_cube[0:n_modes2flat-1,*]), 1) eq n_modes2flat)*r_mask2flatten[*], count)
    endelse
    if count eq 0 then message, "ERROR"
    intersect_mask=fltarr(n_meta_pix_x, n_meta_pix_y)
;    intersect_mask=fltarr(n_pix_x, n_pix_y); CORRECTED MASK ERROR MX ON 20101123
    intersect_mask[intersect_mask_idx]= 1

    window, /free
    image_show, /as, intersect_mask, title='Intersection mask'

    for cc=0, n_elements(data.r_opd_cube[*,0])-1 do begin
        ;data.r_opd_cube[cc,intersect_mask_idx] -= mean(data.r_opd_cube[cc,intersect_mask_idx])
        fit=surf_fit(r_xx[intersect_mask_idx],r_yy[intersect_mask_idx],data.r_opd_cube[cc,intersect_mask_idx],zern_mode2not_flatten,/ZERN)
        data.r_opd_cube[cc,*] *= intersect_mask[*]
        data.r_opd_cube[cc,intersect_mask_idx] -= fit
    endfor

;        int_mat = data.r_opd_cube[*,intersect_mask_idx] ## diagonal_matrix(1d/extract_diagonal(data.applied_mode))

    tmp_r_opd = data.r_opd_cube[0:n_modes2flat-1,*]
    r_opd = temporary(tmp_r_opd[*,intersect_mask_idx])
    diag_amp = extract_diagonal(data.applied_mode)
    diag_amp = diag_amp[0:n_modes2flat-1]
    int_mat = r_opd ## diagonal_matrix(1d/diag_amp)
    undefine, r_opd

    eps = 0.1;(machar()).eps
    nmodes2drop=n_elements(zern_mode2not_flatten) > 1    
    if n_elements(n2drop) ne 0 then nmodes2drop=n2drop
    rec_mat = pseudo_invert(int_mat, /VERB,  V_MAT=v, U=u, W=s, EPS=eps, COUNT=s_count, IDX=s_idx, /DOUBLE, n_modes=nmodes2drop) 
    window, /free
    plot, s, title='Singular values of IntMat', psym=4
    oplot, s_idx,s[s_idx], psym=4, col=255
    if s_count gt 0 then print, s_idx


    flatten_modes = rec_mat ## r_opd2flatten[intersect_mask_idx]
    if n_elements(skipvect) gt 0 then flatten_modes[skipvect]=0
;    if n_elements(check) eq 0 then begin
;        err = update_status(samples)  ;da rimettere
;        if err ne adsec_error.ok then message, "ERROR!!"

;        sys0 = sys_status
;    endif

    ;delta_comm2flatten = scale_factor*float(adsec_save.ff_p_svec[0:n_modes2flat-1, *] ## flatten_modes[0:n_modes2flat-1]); mod RB at LBTO: status_save _init is no more provided;;;; 4 added!!!!! Negative added!
;;;;;;; FINE
delta_comm2flatten = scale_factor*float(command_matrix[0:n_modes2flat-1, *] ## flatten_modes[0:n_modes2flat-1]) ; mod RB to replace the line above. implemented @magellan to use slave command matrix. the data is loaded from if_functions folder in if_redux.pro.
; back-compatible: in if_redux, if the cmd_mat is empty command matrix is filled with adsec_save.ff_p_svec (from make_modal_disturb)


   delta_ff_force2flatten= float(adsec.ff_matrix ## delta_comm2flatten)

 
   window,/free
   display, -delta_comm2flatten[adsec.act_w_cl],adsec.act_w_cl, /SH, /SM,/refl,rot=240 $
           , TIT="-Delta-command for flattening", /no_n

   window,/free
   display, -delta_ff_force2flatten[adsec.act_w_cl],adsec.act_w_cl, /SH $
           , TIT="Feed-forward forces for flattening", /no_n

   if not keyword_set(OFFLINE) then begin
        ans = dialog_message(["Flattening requests max force="+strtrim(max(abs(delta_ff_force2flatten)),2)+"."$
 , "Do you want to apply it?"], /QUEST)

       if strlowcase(ans) eq "yes" then begin
            err = set_offload_cmd(-delta_comm2flatten[adsec.act_w_pos], /NOCH, /DELTA)
            if err ne adsec_error.ok then message, "ERROR!!"
            err = set_offload_force(-delta_ff_force2flatten[adsec.act_w_curr],/START, /NOCH, /DELTA)
            if err ne adsec_error.ok then message, "ERROR!!"
            wait, 2*rtr.oversampling_time

            if not keyword_set(manual) then flattened_opd=get_img4d(nmeas=3, out=flattened_mask)
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

  if not keyword_set(OFFLINE) then begin
       ans = dialog_message(["Do you want to save data?"], /QUEST)
       if strlowcase(ans) eq "yes" then begin
            data_ver=2
            ;data_ver=2: variables added: track,zern_mode2not_flatten, data_ver. opd2flatten is the mean of triggered samples in 4d/Zcopy/flat
            save_file=meas_path('flat')+'flattening_result.sav'
            log_print, 'Flattening data saved in file '+ save_file
            flattening_modes = adsec.ff_p_svec[0:n_modes2flat-1, *]; da modificare????
            status_save = status_save_init
            save, FILE=save_file, n_modes, data, rec_mat, int_mat, s, s_idx, s_count, v, pupil_xyre  $
                      , opd2flatten, mask2flatten , n_pix_x, n_pix_y $
                      , flattened_opd, flattened_mask, flattened_status $
                      , mask_cx, mask_cy, xx, yy, x0, x1, y0, y1 , is_new $
                      , n_meta_pix_x, n_meta_pix_y, flattening_modes, max_rms $
                      , n_pix_diam_x, n_pix_diam_y, min_pix,intersect_mask_idx $
                      , status_save, track,zern_mode2not_flatten, data_ver, tracknum, flattening_file
       endif
  endif

end
