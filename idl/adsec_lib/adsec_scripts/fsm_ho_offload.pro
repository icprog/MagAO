Function fsm_ho_offload, fitsname, NOLUT=nolut, NOHO=noho

    @adsec_common

    do_force_noho = 1B
    do_force_nolut = 0B
    if do_force_nolut then nolut=1B
    if do_force_noho then noho=1B

    if ~keyword_set(noho) then begin
        log_print, 'HO_Offload: processing'
        log_print, fitsname
        ;qui ci va il  segno MENO per la correzione
;;; ATTENZIONE IL CASO fitsbname non definito non e' trattato bene qui
        if n_elements(fitsname) gt 0 then begin
            vv = readfits(fitsname)
;            display,-( vv)[adsec.act_w_cl], adsec.act_w_cl, /no_n, /sh, title='Force delta'
        endif
        vv[adsec.act_wo_cl] = 0
        kk = -0.5*vv
        ;display, kk[adsec.act_w_cl], adsec.act_w_cl, /sh, /as
        err = fsm_apply_cmd(kk, /passed, /delta)

;        if ~keyword_set(nolut) then begin
            if err ne adsec_error.ok then begin
                return, err
            endif else begin
                sys_status.cflat_offload_cmd += kk
                sys_status.cflat_offload_curr += adsec.ff_matrix ## kk
            endelse
;        endif
    endif else begin
        log_print, 'HO_Offload forced to be skipped'
    endelse

    if ~keyword_set(nolut) then begin
        log_print, "Applying LUT correction for low order Zernike"

        st = strupcase(strmid(!AO_CONST.telescope_side,0,1))
        elev_name = 'TEL.EL'
        err = read_var(elev_name,  elevation, /common, SUBS='AOS') 
        if err ne adsec_error.ok then return, err
        if ~finite(elevation) then begin
            log_print, "Elevation in NAN: nothing applied.", LOG_LEV=log_lev_error
            return, adsec_error.VAR_RANGE_ERROR
        endif
        elevation /= 3600.
        sys_status.elevation_angle = elevation
        if elevation gt 90 then elevation = 90
        if elevation lt 0 then elevation=0
        log_print, "elevation is [deg] "+strtrim(elevation,2)
        vv = get_zern_flat_lut(elevation)
        log_print, "z6 is [m rms] "+strtrim(vv[5],2)
        err = fsm_correct_modes(vv, /PASSED)
        if err ne adsec_error.ok then return, err
        log_print, "Done."
    endif else begin
        log_print, "Forcing NO LUT correction for low order Zernike"
    endelse
    
    return, adsec_error.ok
End
