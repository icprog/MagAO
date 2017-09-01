Pro find_pos_cal, d0v, cv

    @adsec_common

    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;       ZERNIKE PART       ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    path = "~/idl/adsec672a/meas/status_save/"
    ;pathd = path+'2008_04_23/'
;    pathd = path+'2008_04_28/'
;    restore, pathd+"status_save_TS1_70um_pist_calibration_c_pre_zp.sav", /VERB
    pathd = path+'2008_04_29/'
    restore, pathd+"status_save_TS1_70um_pist_calibration_d_zp.sav", /VERB
    restore, path+"bad_calib_act.sav", /VERB
    do_save = 1B


    adsec_save=status_save.adsec
    adsec_shell_save=status_save.adsec_shell
    cl_list = status_save.adsec.act_w_cl

    pos_list_save =  status_save.adsec.act_w_pos
    bad_calib_act = cl_list
    bad_calib_act_save = bad_calib_act
    

    ;if bad_calib_act[0] ne -1 then begin
    ;        print, intersection(bad_calib_act, adsec_save.true_act , true_bad_calib_act) 
    ;        print, intersection(true_bad_calib_act, cl_list , bad_calib_act_w_pos) 
    ;        print, complement(bad_calib_act_w_pos, cl_list, pp)
    ;        cl_list = pp
    ;endif
    pos = status_save.sys_status.position

    window,/FREE
    display, pos[cl_list], cl_list, /SH, /NO_NUM, /SM

    pp = fltarr(adsec_save.n_actuators)
    x = adsec_save.act_coordinates[0,cl_list]
    y = adsec_save.act_coordinates[1,cl_list]
    radial_ave_pos = fltarr(adsec_shell.n_rings)
    for i=0,adsec_shell_save.n_rings-1 do begin
        dr = (adsec_shell_save.ring_radius[1]-adsec_shell_save.ring_radius[0])/2
        idx = where(abs(sqrt(x^2+y^2)-adsec_shell_save.ring_radius[i]) lt dr)
        act_list = cl_list[idx]
        pp[act_list] = i
        radial_ave_pos[i] = mean(pos[act_list])
    endfor

    window,/FREE
    plot, adsec_shell_save.ring_radius, radial_ave_pos, title="RADIAL AVE POS"

    z = pos[cl_list]
    xn=x/adsec_shell_save.out_radius
    yn=y/adsec_shell_save.out_radius
    xfn = adsec_save.act_coordinates[0,pos_list_save]/adsec_shell_save.out_radius
    yfn = adsec_save.act_coordinates[1,pos_list_save]/adsec_shell_save.out_radius
    nrad = 5
    j_list = indgen((nrad+1)*(nrad+2)/2+1)+1
    fitf = surf_fit(xn, yn, z, j_list, /ZERN, COEFF=coeff, ZOUT=[xfn,yfn])
    fit_all = fltarr(adsec_save.n_actuators)
    fit_all[pos_list_save]=fitf
    fit = fit_all[cl_list]
    res = pos[cl_list]-fit
    window, /FREE
    display, res, cl_list, /SH, /NO_NUM

    window, /FREE
    display, fitf, pos_list_save, /SH, /NO_NUM, /SM

    window, /FREE
    plothist, pos[cl_list]-fit, BIN=1e-6

;    idx = where(res gt 7e-6, count)
;    if count ne 0 then begin
;        bad_calib_act = cl_list[idx];

;        pp = fltarr(adsec_save.n_actuators)
;        pp[bad_calib_act]=1
;        window, /FREE
;        display, pp, /NO_NUM
;    endif

;    if bad_calib_act_save[0] eq -1 then count_save=0 else count_save=n_elements(bad_calib_act_save)

;    print,"Number of bad actuators:", count_save
;    print,"Number of bad actuators to add:", count
;    if count ne 0 and do_save then begin
;        bad_calib_act = [bad_calib_act,bad_calib_act_save]
;        bad_calib_act = bad_calib_act[sort(bad_calib_act)]
;       save, bad_calib_act, file=pathd+"'bad_calib_act.sav"
;       print, "New list of bad actuators saved"
;    endif

    ;DELTA BETWEEN FIT and POSITION
    betac = fltarr(adsec_save.n_actuators)
    for i=0, n_elements(bad_calib_act)-1 do begin
        dummy = where(adsec_save.act_wo_pos eq bad_calib_act[i], cc)
        if cc gt 1 then message, "ERROR"
        if cc ne 1 then begin
            betac[bad_calib_act[i]] = pos[bad_calib_act[i]]-fit_all[bad_calib_act[i]]
        endif
    endfor

    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    ;;;;;;;;;;;;;;;;;;;;;;;;;;;     PISTON     ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

    ;Remove the piston from the mode ff_p_svec[i,*]
    ;using the first nmodes modes.

    pos_eigvec = status_save.adsec.ff_p_svec
    for_eigvec = status_save.adsec.ff_f_svec

    ;Get the piston from the position eigenvectors
    mea = fltarr(n_elements(adsec_save.true_act))
    for i=0, n_elements(adsec_save.true_act)-1 do mea[i] = mean(pos_eigvec[i,*])
    dummy = max(abs(mea), idP)
    PistP = pos_eigvec[idP,cl_list]
    PistF = for_eigvec[idP,cl_list]
    ;Plot modal coefficients for piston fit
    vv = dblarr(1,adsec_save.n_actuators)+1.
    mm = transpose(pos_eigvec) ## vv
    
    ;MANUAL MODE CUT SELECTION
    window, /FREE
    plot, mm[0,0:20], ytitle='MODAL COEFFS [AU]', xtitle='MODES',psym=-4
    id = ''
    print,  [mm[0,0:20], indgen(1,21)]
    read, "ENTER THE FIRST MODE TO CUT: ", id
;    mode2cut = [5,13L]

 ;   id0 = 5
    id = ulong(13L)
    mm[id:*] = 0
 ;   mm[0:id0] = 0
    pist = pos_eigvec ## mm

    window, /FREE
    display, pist[adsec.act_w_cl],adsec.act_w_cl, /sh, TITLE='GAMMAC'
    gammac = dblarr(adsec.n_actuators) + 1.0
    gammac[adsec.act_w_cl] = 1d/pist[adsec.act_w_cl]


    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    ;;;;;;;;;;;;;;;;;;;;;;;;;;;     COMPUTATION  ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

    pos = status_save.sys_status.position
    adc_bits = adsec_save.adc_bits
    vratio   = adsec_save.capsens_vmax/adsec_save.capsens_vref
    d0v = adsec_save.d0
    cv =  adsec_save.c_ratio

    d0v[cl_list] = (gammac[cl_list] * (pos[cl_list]-betac[cl_list])^2*status_save.adsec.d0[cl_list]/(pos[cl_list]^2))
    cv[cl_list] = -d0v[cl_list]/(pos[cl_list]-betac[cl_list])+status_save.sys_status.voltage[cl_list]/2.^adc_bits*vratio


    window, /FREE
    display, d0v[cl_list], cl_list, /SH, TITLE='D0'


    window, /FREE
    display, cv[cl_list], cl_list, /SH, TITLE='Cratio'
    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    ;;;;;;;;;;;;;;;;;;;;;;;;;;;     SAVING       ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

    ext = '_dummy'
    pm = meas_path('calibration')
    save, betac, gammac, d0v, cv, status_save, bad_calib_act, id, cl_list, file=pathd+"d0_zern_pist_calib.sav"
    openw, 1, pm+"d0_zern_pist"+ext+".txt"
    printf, 1, "float "+strtrim(n_elements(d0v),2)
    printf, 1, transpose(d0v)
    close, 1
    openw, 1, pm+"cr_zern_pist"+ext+".txt"
    printf, 1, "float "+strtrim(n_elements(cv),2)
    printf, 1, transpose(cv)
    close, 1
    return

End











