; $Id
;+
; HISTORY
;   08 Aug 2009: AR, added coma-x and coma-y to the list of default Zernike
;                polynomials to be filtered out from the flattening shape
;- 


Function opt_int_mat_reduce, n_modes2flat, ZERN_NOT_FLAT = zern_mode2not_flatten, MULTIPLE_ACQ=multiple_acq, SKIP=skipvect, NMEAS=nmeas, INTER=inter, check=check

    @adsec_common
    if n_elements(nmeas) eq 0 then nmeas=5
    if keyword_set(MULTIPLE_ACQ) then begin
        files= dialog_pickfile(PATH='/home/aosup/adopt/measures/adsec_calib/flat/', TITLE='Please select the MULTIPLE files to reduce:', /MULTIPLE)
        files = reverse(files[sort(files)])
        for i=0, n_elements(files)-1 do begin
            undefine, is_new  ;undefine the variable not present in the old interaction matrix
            print, "Reducing file: "+files[i]
            restore, files[i]
            
            if n_elements(status_save_init) gt 0 then begin
                vv = where(status_save_init.sys_status.closed_loop)
                av = where(sys_status.closed_loop)
                if total( (av eq vv)) ne n_elements(adsec.act_w_cl) then message, "ERROR: ACTUATORS MISMATCH"
            endif
            if n_elements(flattened_status) gt 0 then begin
                vv = where(flattened_status.closed_loop)
                av = where(sys_status.closed_loop)
                if total( (av eq vv)) ne n_elements(adsec.act_w_cl) then message, "ERROR: ACTUATORS MISMATCH"
            endif


            if n_elements(end_mode) eq 0 then end_mode=n_modes-1
            if n_elements(start_mode) eq 0 then start_mode=0
            if i eq 0 then begin
                ;;; TEMPORARY VARIABLE DEFINITION
                data_temp = {                                                           $
                            applied_mode : fltarr(end_mode+1, end_mode+1),                    $
                            r_opd_cube   : fltarr(end_mode+1, n_meta_pix_x*n_meta_pix_y),  $
                            r_mask_cube  : bytarr(end_mode+1, n_meta_pix_x*n_meta_pix_y)   $
                            }
                new_end_mode = end_mode
            endif
            data_temp.applied_mode[start_mode:end_mode, start_mode:end_mode] = data.applied_mode
            data_temp.r_opd_cube[start_mode:end_mode, *] = data.r_opd_cube
            data_temp.r_mask_cube[start_mode:end_mode,*] = data.r_mask_cube
        endfor
        print, "End collecting..."
        data = temporary(data_temp)
        n_modes = new_end_mode+1
        start_mode = 0
        end_mode = new_end_mode
        

    endif else begin
        file= dialog_pickfile(PATH='/home/aosup/adopt/measures/adsec_calib/flat/', TITLE='Please select the SINGLE file to reduce:')
        print, file
        if file eq "" then begin
            message, "ERROR" 
        endif else begin
            restore, file, /ver
        endelse

    endelse

    if n_elements(zern_mode2not_flatten) eq 0 then zern_mode2not_flatten = [1,2,3];[1,2,3,4,7,8]
    if n_elements(n_modes2flat) eq 0 then n_modes2flat = n_modes
    if n_modes2flat gt n_modes then begin
        print, "Wrong modes number flat request: reducing to modes #", n_modes
        n_modes2flat = n_modes
    endif
if keyword_set(check) then begin
        opd2flatten=read_opd('/home/aosup/adopt/measures/adsec_calib/start.h5', bad=mask2flatten, type='4d')
endif else begin

    err = idl_4d_init()
    name_radix = "before_flat_"+string(n_modes2flat, format='(I3.3)')+'_modes'
    err = get_m4d(name_radix, opd2flatten, mask2flatten) 
    sopd = size(opd2flatten, /dim)

    for i=1, nmeas-1 do begin
        
;    name_radix = "before_flat_"+string(n_modes2flat, format='(I3.3)')+'_modes'
        err = get_m4d(name_radix, opd_tmp, mask_tmp )
        opd2flatten += (opd_tmp)
        mask2flatten *= float(mask_tmp)

    endfor
   opd2flatten /= float(nmeas)

;    err = get_m4d(name_radix, opd2flatten, mask2flatten)
    if err ne adsec_error.ok then message, "ERROR!!"
    err = idl_4d_finalize()
endelse
    if n_elements(is_new) eq 0 then opd2flatten /=2
    help, is_new
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

    ;SAVE the value of intersect_mask_idx loaded from file
    if n_elements(intersect_mask_idx) eq 0 then begin
        intersect_mask_idx = where((total(float(data.r_mask_cube[0:n_modes2flat-1, *]), 1) eq n_modes2flat)*r_mask2flatten[*], count)
        saved_intersect_mask_idx = intersect_mask_idx
    endif else begin
        saved_intersect_mask_idx = intersect_mask_idx
        intersect_mask_idx = where((total(float(data.r_mask_cube[0:n_modes2flat-1,*]), 1) eq n_modes2flat)*r_mask2flatten[*], count)
    endelse
    if count eq 0 then message, "ERROR"
    
;    if total(intersect_mask_idx - saved_intersect_mask_idx) ne 0 or n_elements(rec_mat) eq 0 then begin
        
        answ = dialog_message("The reconstruction matrix must be recalculated: proceed?",/QUESTION)
        if answ ne "Yes" then return, adsec_error.generic_error
        

        for cc=0, n_elements(data.r_opd_cube[*,0])-1 do begin
            ;data.r_opd_cube[cc,intersect_mask_idx] -= mean(data.r_opd_cube[cc,intersect_mask_idx])
            fit=surf_fit(r_xx[intersect_mask_idx],r_yy[intersect_mask_idx],data.r_opd_cube[cc,intersect_mask_idx],[1,2,3],/ZERN)
            data.r_opd_cube[cc,intersect_mask_idx] -= fit
        endfor

;        int_mat = data.r_opd_cube[*,intersect_mask_idx] ## diagonal_matrix(1d/extract_diagonal(data.applied_mode))

        tmp_r_opd = data.r_opd_cube[0:n_modes2flat-1,*]
        r_opd = temporary(tmp_r_opd[*,intersect_mask_idx])
        diag_amp = extract_diagonal(data.applied_mode)
        diag_amp = diag_amp[0:n_modes2flat-1]
        int_mat = r_opd ## diagonal_matrix(1d/diag_amp)
        undefine, r_opd




        ;svdc, data.r_opd_cube[*,intersect_mask_idx], s, u, v, /DOUBLE

        ;window, /free
        ;plot, s, title='SINGULAR VALUES', psym=-4
        ;print, s
        eps = 0.1;(machar()).eps
        ;idx = where(s/max(s) gt eps, idx_s, COMPL=compl_idx)
        ;spi = dblarr(n_elements(s))
        ;spi[idx] = 1/s[idx]

        ;if idx_s[0] gt 0 then print,"Cutted eigenvalues:", compl_idx
; forced cutting of the piston eigvalue

        ;if idx_s eq n_elements(s) then begin
        ;    dum = min(s, id_min)
        ;    spi[id_min] = 0.0
        ;endif

        ;spi = diagonal_matrix(spi)
        
;        rec_mat = data.applied_mode ## v ## spi ## transpose(u)
        rec_mat = pseudo_invert(int_mat, /VERB,  V_MAT=v, U=u, W=s, EPS=eps, COUNT=s_count, IDX=s_idx, /DOUBLE, INTER=inter)  ;/iner removed
        if s_count gt 0 then print, s_idx

;    endif

    ;computation of flattening

    flatten_modes = rec_mat ## r_opd2flatten[intersect_mask_idx]
    if n_elements(skipvect) gt 0 then flatten_modes[skipvect]=0

    err = update_status(samples)
    if err ne adsec_error.ok then message, "ERROR!!"

    sys0 = sys_status

;;;;;;; AGGIUNTO MARCO
    ;nuovi modi nela base attuale
    delta_comm2flatten = float(adsec.ff_p_svec[0:n_modes2flat-1, *] ## flatten_modes[0:n_modes2flat-1]); was status_save_init.
;;;;;;; FINE


 ;   delta_comm2flatten = float((adsec.ff_p_svec[0:n_modes2flat-1, *] ## flatten_modes[0:n_modes2flat-1]))
    delta_ff_force2flatten= float(adsec.ff_matrix ## delta_comm2flatten)

;    delta_comm2flatten = -float((adsec.ff_p_svec[0:n_modes2flat-1, *] ## flatten_modes[0:n_modes2flat-1]))
;    delta_ff_force2flatten= -float(adsec.ff_matrix ## delta_comm2flatten)

    window,3
    display, delta_comm2flatten[adsec.act_w_cl],adsec.act_w_cl, /SH, /SM, /no_n $
           , TIT="Delta-command for flattening"

    window,4
    display, delta_ff_force2flatten[adsec.act_w_cl],adsec.act_w_cl, /SH,/no_n $
           , TIT="Feed-forward forces for flattening"

    ans = dialog_message(["Flattening requests max force="+strtrim(max(abs(delta_ff_force2flatten)),2)+"." $
                         , "Do you want to apply it?"], /QUEST)

    if strlowcase(ans) eq "yes" then begin

        err = set_offload_cmd(-delta_comm2flatten[adsec.act_w_pos], /NOCH, /DELTA)
        if err ne adsec_error.ok then message, "ERROR!!"
        err = set_offload_force(-delta_ff_force2flatten[adsec.act_w_curr],/START, /NOCH, /DELTA)
        if err ne adsec_error.ok then message, "ERROR!!"
        wait, 2*rtr.oversampling_time

        err = idl_4d_init()
        name_radix = "result_flat_"+string(n_modes2flat, format='(I3.3)')+'_modes'
        err = get_m4d(name_radix, flattened_opd, flattened_mask, MIN_PIX=min_pix)
        if err ne adsec_error.ok then message, "ERROR!!"
        err = idl_4d_finalize()
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

    ans = dialog_message(["Do you want to save data?"], /QUEST)
    if strlowcase(ans) eq "yes" then begin
        save_file=dialog_pickfile(FILTER="*.sav",PATH='/home/aosup/adopt/measures/adsec_calib/flat/')
        if save_file ne "" then begin
            flattening_modes = adsec.ff_p_svec[0:n_modes2flat-1, *]
            status_save = status_save_init
            save, FILE=save_file, n_modes, data, rec_mat, int_mat, s, s_idx, s_count, v, pupil_xyre  $
                  , opd2flatten, mask2flatten , n_pix_x, n_pix_y $
                  , flattened_opd, flattened_mask, flattened_status $
                  , mask_cx, mask_cy, xx, yy, x0, x1, y0, y1 , is_new $
                  , n_meta_pix_x, n_meta_pix_y, flattening_modes, max_rms $
                  , n_pix_diam_x, n_pix_diam_y, min_pix,intersect_mask_idx, status_save
        endif
    endif

end
