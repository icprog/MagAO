Function opt_int_mat_ptt, n_modes2flat, ZERN_NOT_FLAT = zern_mode2not_flatten, MULTIPLE_ACQ=multiple_acq, SKIP=skipvect, NMEAS=nmeas

    @adsec_common
    if keyword_set(MULTIPLE_ACQ) then begin
        files= dialog_pickfile(PATH='~/idl/adsec672a/meas/flat/', TITLE='Please select the MULTIPLE files to reduce:', /MULTIPLE)
        files = reverse(files[sort(files)])
        for i=0, n_elements(files)-1 do begin
            undefine, is_new  ;undefine the variable not present in the old interaction matrix
            print, "Reducing file: "+files[i]
            restore, files[i]
            
;            if n_elements(status_save_init) gt 0 then begin
;                vv = where(status_save_init.sys_status.closed_loop)
;                av = where(sys_status.closed_loop)
;                if total( (av eq vv)) ne n_elements(adsec.act_w_cl) then message, "ERROR: ACTUATORS MISMATCH"
;            endif
;            if n_elements(flattened_status) gt 0 then begin
;                vv = where(flattened_status.closed_loop)
;                av = where(sys_status.closed_loop)
;                if total( (av eq vv)) ne n_elements(adsec.act_w_cl) then message, "ERROR: ACTUATORS MISMATCH"
;            endif


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
        file= dialog_pickfile(PATH='~/idl/adsec672a/meas/flat/', TITLE='Please select the SINGLE file to reduce:')
        print, file
        if file eq "" then begin
            message, "ERROR" 
        endif else begin
            restore, file, /ver
        endelse

    endelse

    if n_elements(zern_mode2not_flatten) eq 0 then zern_mode2not_flatten = [1,2,3,4]
    if n_elements(n_modes2flat) eq 0 then n_modes2flat = n_modes
    if n_modes2flat gt n_modes then begin
        print, "Wrong modes number flat request: reducing to modes #", n_modes
        n_modes2flat = n_modes
    endif

    r_xx = xx[x0:x1, y0:y1]
    dummy = fltarr(n_pix_x, n_pix_y)
    dummy[0,0] = r_xx
    r_xx = rebin(dummy, n_meta_pix_x, n_meta_pix_y)
    
    r_yy = yy[x0:x1, y0:y1]
    dummy = fltarr(n_pix_x, n_pix_y)
    dummy[0,0] = r_yy
    r_yy = rebin(dummy, n_meta_pix_x, n_meta_pix_y)

    intersect_mask_idx = where((total(float(data.r_mask_cube[0:n_modes2flat-1, *]), 1) eq n_modes2flat), count)
    if count eq 0 then message, "ERROR"
    
    for cc=0, n_elements(data.r_opd_cube[*,0])-1 do begin
        fit=surf_fit(r_xx[intersect_mask_idx],r_yy[intersect_mask_idx],data.r_opd_cube[cc,intersect_mask_idx],[1,2,3],/ZERN)
        data.r_opd_cube[cc,intersect_mask_idx] -= fit
    endfor

    tmp_r_opd = data.r_opd_cube[0:n_modes2flat-1,*]
    r_opd = (temporary(tmp_r_opd))[*,intersect_mask_idx]
    diag_amp = extract_diagonal(data.applied_mode)
    diag_amp = diag_amp[0:n_modes2flat-1]
    int_mat = temporary(r_opd) ## diagonal_matrix(1d/diag_amp)
    
    la_svd, int_mat, w, u, v, DOUBLE=double

idx = sort(w)
w = w[idx]
u = u[idx,*]
v = v[idx,*]

print, "Ratio between lowest three singular values and fourth singula value: ", w[0:2]/w[3]

ww = w[0:2]
uu = u[0:2,*]
vv = v[0:2,*]

; in the actuator space
pp = status_save_init.adsec.ff_p_svec[0:n_modes2flat-1, *]##vv
 ptt = [replicate(1.0,1,672),adsec.act_coordinates[0,*],adsec.act_coordinates[1,*]]
window,/FREE
display, pp[0,status_save_init.adsec.act_w_cl],status_save_init.adsec.act_w_cl, /SH
window,/FREE
display, pp[1,status_save_init.adsec.act_w_cl],status_save_init.adsec.act_w_cl, /SH
window,/FREE
display, pp[2,status_save_init.adsec.act_w_cl],status_save_init.adsec.act_w_cl, /SH

; ideal piston, tip and tilt with usual normalization as for SVD modes
ptt  = [replicate(1.0,1,adsec.n_actuators),adsec.act_coordinates[0,*],adsec.act_coordinates[1,*]]
ptt[*,status_save_init.adsec.act_wo_cl]=0
ptt[0,*] /= sqrt(total(ptt[0,*]^2))
ptt[1,*] /= sqrt(total(ptt[1,*]^2))
ptt[2,*] /= sqrt(total(ptt[2,*]^2))

ptt1 = ptt[*,status_save_init.adsec.act_w_cl]
pp1 = pp[*,status_save_init.adsec.act_w_cl]

; fitting of:.  ptt1 = pp1 ## aa
aa = pseudo_invert(pp1) ## ptt1
ptt1_fit = pp1 ## aa

capsens_ptt = fltarr(3,status_save_init.adsec.n_actuators)
capsens_ptt[*,status_save_init.adsec.act_w_cl] = ptt1_fit

ans = dialog_message(["Do you want to save data?"], /QUEST)
    if strlowcase(ans) eq "yes" then begin
        save_file=dialog_pickfile(FILTER="*.sav")
        if save_file ne "" then begin
            save, FILE=save_file, pp, capsens_ptt, status_save_init
        endif
    endif
return, adsec_error.ok
end
