Function fsm_correct_modes, dcomm_file, PASSED=passed, DELTA=delta, HOLD=hold, NO_UPDATE_REFERENCE=no_update_ref
    
    @adsec_common
    if n_elements(fileim) eq 0 then fileim = 'im_reduced_from_ff_matrix_OAT_calibrated.sav-ff_matrix_+6.005e-05_3.400e-06m_c.sav-653modes_2009_08_10.sav'
    if sc.name_comm eq "Dummy" then return, adsec_error.ok
   ; print, dcomm_file
   ; return, adsec_error.ok
    if ~keyword_set(PASSED) then begin
        dc_exists  = (file_info(dcomm_file)).exists     
        if ~dc_exists then begin
            log_message, "The provided file does not exist.", CONT = (sc.debug eq 0), ERR=adsec_error.invalid_file
            return, adsec_error.invalid_file
        endif

        dcomm = readfits(dcomm_file)
    endif else begin
        dcomm = dcomm_file
    endelse
    if n_elements(dcomm) ne 22 then begin
        log_message, "The command to apply has wrong format.(must be 1x22, FLOAT)", CONT = (sc.debug eq 0), ERR=adsec_error.input_type
        return, adsec_error.input_type
    endif

    ;FORCE PISTON to 0
    dcomm[0] = 0
    
    ;ANGLE RETRIEVING
;    vv=fltarr(1, 22) & vv[5]=1 
;    tmp=pm_offload.pmz ## vv 
;    display, tmp[adsec.act_w_cl], adsec.act_w_cl, /sh, /no_n, /sm, pos=zimg, /no_plot
;    idx = where(zimg ne median(zimg))
;    szimg = size(zimg, /dim)
;    xx = rebin(mk_vector(szimg[0], -1, 1), szimg[0], szimg[0], /samp)
;    yy = rebin(transpose(mk_vector(szimg[0], -1, 1)), szimg[0], szimg[0], /samp)
;    fit = surf_fit(xx[idx], yy[idx], zimg[idx], [5,6], /zern, coeff=coeff) 
;    angle = atan(coeff[0], coeff[1])/!pi*180 
    
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; start commenting: angle and rotation are no longer needed
;
;    ;ROTATION MATRIX DEFINITION
;    rmat = (rot_mat(-sys_status.pmz_angle, 3, /deg))[0:1, 0:1] 
;
;    ;ROTATION of Z2,Z3 - Z5,Z6 - Z7,Z8 - Z9,Z10 - Z12,13 - Z14,15 -Z16,17 -Z18,19 - Z20, 21
    rdcomm = transpose(dcomm)
;    zidx = [2, 5, 7, 9, 12, 14, 16, 18, 20] 
;    for i=0, n_elements(zidx)-1 do begin
;        rdcomm[zidx[i]-1:zidx[i]] = rmat ## rdcomm[zidx[i]-1:zidx[i]]
;    endfor
; stop commenting
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
        
    ;Zernike GENERATION
    ;newcmd = pm_offload.pmz ## transpose(dcomm)
    newcmd = pm_offload.pmz ## rdcomm
    newcmd[adsec.act_wo_cl] = 0
    err = update_status()
    if err ne adsec_error.ok then return, err

    err = fsm_apply_cmd(newcmd, /PASSED, DELTA=delta, HOLD=hold)
    if err ne adsec_error.ok then return, err

    err = update_status()
    if err ne adsec_error.ok then return, err

    pos_vector = sys_status.position
    cur_vector = sys_status.current

    if ~keyword_set(no_update_ref) then begin
        !AO_STATUS.ref_pos = pos_vector
        !AO_STATUS.ref_for = cur_vector
    endif

    if keyword_set(DELTA) then begin
        err = READ_VAR('ZERN_APPLIED', zvector)
        if err ne adsec_error.ok then return, err
        !AO_STATUS.zern_applied = zvector+dcomm
    endif else begin
        !AO_STATUS.zern_applied = dcomm
    endelse

    return, update_rtdb()    

End
