;$Id: find_cal_zo.pro,v 1.3 2008/08/01 08:00:14 labot Exp $
;This procedure finds out the zero order calibration by fitting a plane in the
;feed forward piston mode.
;x=beta * p
;f=alpha * c

Function find_cal_zo, BETAC=betac, ALPHA=alpha, PATH_SAVE=path_save, EXT=ext, $
                      POS_EIGVEC=pos_eigvec, FOR_EIGVEC=for_eigvec, ACT_IDX=act_idx, PIST_FIT=pist_fit

    ;Remove the piston from the mode ff_p_svec[i,*]
    ;using the first nmodes modes.

    @adsec_common
    if n_elements(pos_eigvec) eq 0 then pos_eigvec = adsec.ff_p_svec
    if n_elements(for_eigvec) eq 0 then for_eigvec = adsec.ff_f_svec
    if n_elements(act_idx) eq 0 then act_idx = adsec.act_w_pos

    ;Get the piston from the position eigenvectors
    mea = fltarr(n_elements(adsec.true_act))
    for i=0, n_elements(adsec.true_act)-1 do mea[i] = mean(pos_eigvec[i,*])
    dummy = max(abs(mea), idP)
    PistP = pos_eigvec[idP,act_idx]
    PistF = for_eigvec[idP,act_idx]
    if keyword_set(PIST_FIT) then begin

        ;Plot modal coefficients for piston fit
        vv = dblarr(1,672)+1.
        mm = transpose(adsec.ff_p_svec) ## vv
        window, /free
        plot, mm[0,0:20], ytitle='MODAL COEFFS [AU]', xtitle='MODES',psym=-4
        id = ''
        print,  [mm[0,0:20], indgen(1,21)]
        read, "ENTER THE FIRST MODE TO CUT: ", id
        id = ulong(id)
        mm[id:*] = 0
        pist = adsec.ff_p_svec ## mm
        window, /free
        display, pist[adsec.act_w_cl],adsec.act_w_cl, /sh
        betac = dblarr(adsec.n_actuators) + 1.0
        betac[adsec.act_w_cl] = 1./pist[adsec.act_w_cl]

    endif else begin

        ;xact = reform(adsec.act_coordinates[0,act_idx]/adsec_shell.out_radius)
        xact = reform(adsec.act_coordinates[0,act_idx])
        ;yact = reform(adsec.act_coordinates[1,act_idx]/adsec_shell.out_radius)
        yact = reform(adsec.act_coordinates[1,act_idx])

        ;Fit the Zernike Piston, Tip and Tilt and find the position calibration
        ;fitPistP = surf_fit(xact,yact, (PistP),[1,2,3],/ZERN, COEFF=coeffp)
        fitPistP = surf_fit(xact,yact, (PistP),[1,2,3], COEFF=coeffp)
        inv_beta = make_array(adsec.n_actuators, value=1d)
        inv_beta[act_idx] = abs(PistP)/abs(fitPistP)
        betac = 1/inv_beta
    
        ;Fit and find the force calibration
        ;fitPistF = surf_fit(xact,yact,(PistF),[1,2,3],/ZERN, COEFF=coefff)
        fitPistF = surf_fit(xact,yact,(PistF),[1,2,3], COEFF=coefff)
        alpha = make_array(adsec.n_actuators, value=1d)
        alpha[act_idx] = abs(PistF)/abs(fitPistF)
    endelse



    if keyword_set(PATH_SAVE) then begin
        if ~keyword_set(EXT) then ext=""    
	path_save=meas_path('capsens-calib')
        write_array_ascii, float(adsec.d0*betac), FILE=path_save+'d0_pist_calib'+ext+'.txt'
        d0_new = adsec.d0*betac
        write_array_ascii, float(adsec.d0_sd*betac), FILE=path_save+'d0_sd_pist_calib'+ext+'.txt'
        d0_sd_new = adsec.d0_sd*betac
        err = get_status(status_save)
        save, file = path_save+'d0_pist_calib'+ext+'.sav', d0_new, d0_sd_new, status_save, id
        if ~keyword_set(PIST_FIT) then write_array_ascii, float(adsec.act_efficiency*alpha), FILE=path_save+'act_eff_calib'+ext+'.txt'
        
    endif
    
    return, adsec_error.ok
    

End
