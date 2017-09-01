;;;FF MATRIX DATA REDUCTION (compatible in dummy mode)

    loadct, 12
    ;dir = meas_path('ff_matrix')
    ;restore, dir+'temp_data_ff.sav', /ver
    fffile = dialog_pickfile()
    restore, fffile, /ver
    dir = file_dirname(fffile)

    err= ff_fit_v2(data, samples, ff_matrix, ff_u, ff_w, ff_v, df, dx, dc, BIAS_CURRENT=bias_current[cl_act] $
                  , GAIN=gain_n2m[cl_act], SINGULAR=n_sv, SIGMA=sigma_ff_matrix, CHISQ=chisq_ff_matrix $
                  , COVAR=m_covar, CL_ACT=cl_act, NO_MATCH_ORDERING=no_match_ordering)
    if err ne adsec_error.ok then begin
        message, "Error fitting the data acquired.", CONT=(sc.debug eq 0B)
    ;    return, err
    endif

    dof = (nmeas-n_cl_act)*n_cl_act
    print, "Normalized Chi^2 ff_matrix:",chisq_ff_matrix/dof, "  DOF:", strtrim(dof)

;f_calib_fit, data, gain, m_covar, f_calib_vector, CHISQ=chisq_f_calib, SIGMA=sigma_f_calib, $
    ;             SINGULAR=n_sv_f_calib
    ;print, "Normalized Chi^2 f_calib  :",chisq_f_calib/((nmeas-1)*n_cl_act)

    f_calib_vector=0
    chisq_f_calib=0
    sigma_f_calib=0
    n_sv_f_calib=0

    u=ff_u
    v=ff_v
    w=ff_w
    act_w_ff = cl_act

    ;; display residulas
    if sc.name_comm ne "Dummy" then begin
    
        res = df-ff_matrix##dx
        res_var = total(res^2, 1)/nmeas
        image_show, res, /SH, TITLE="residuals: df-ff_matrix##dx"

        window, /free
        image_show, abs(res), /SH, TITLE="residuals: abs(df-ff_matrix##dx)"

        window, /free
        display, sqrt(res_var), cl_act, TITLE="residual rms", /SH

        ;if lin_ck then begin
        ;    test_linear, data, cl_act
        ;endif
    endif
    ff_full = dblarr(adsec.n_actuators, adsec.n_actuators)
    dummy = dblarr(adsec.n_actuators, n_cl_act)
    dummy[cl_act,*]=ff_matrix
    ff_full[*,cl_act]=dummy

    ff_reorder = ff_full[adsec.mir_act2act,*]
    ff_reorder = ff_reorder[*,adsec.mir_act2act]

    ;m2c = dblarr(adsec.n_actuators, adsec.n_actuators)
    ;dummy = dblarr(adsec.n_actuators, n_cl_act)
    ;dummy[cl_act,*]=ff_v
    ;m2c[*,cl_act]=dummy

    time_acq=systime(/sec)
    ;;save modes to command matrix
    m2c = v 

;    err = get_status(status_save)

    ;; save the ff_matrix and force calibration fits
    save, nmeas, amp, delay, samples, ff, times, max_min, sigma_pos,  covar, data, ff_matrix, ff_u, ff_w, ff_v, df, dx, dc, $
          gain_n2m, n_sv, sigma_ff_matrix, chisq_ff_matrix, modal, f_calib_vector, chisq_f_calib, sigma_f_calib, n_sv_f_calib, $
          cl_act, ff_reorder,time_acq, status_save, FILE=filepath(ROOT=dir, filename)

        ;; save a file containing ff_matrix useable as startup ff_matrix file (see adsec_path.data directory)
;    save, ff_matrix, act_w_ff, u, w, v, FILE=filepath(ROOT=dir,"ff_matrix_"+strsign+strpos $
;             +"_"+stramp+strmodal+ext)
    fits_tn = stregex(dir, '[0-9]+_[0-9]+',/EX)
    mkhdr, hdr, act_w_ff, /EXTEND
    sxaddpar, hdr, "TRACKNUM", fits_tn
    writefits, filepath(ROOT=dir,'ff_list.fits'), act_w_ff, hdr
    mkhdr, hdr, u, /EXTEND
    sxaddpar, hdr, "TRACKNUM", fits_tn
    writefits, filepath(ROOT=dir,'ff_f_svec.fits'), u, hdr
    mkhdr, hdr, v, /EXTEND
    sxaddpar, hdr, "TRACKNUM", fits_tn
    writefits, filepath(ROOT=dir,'ff_p_svec.fits'), v, hdr
    mkhdr, hdr, w, /EXTEND
    sxaddpar, hdr, "TRACKNUM", fits_tn
    writefits, filepath(ROOT=dir,'ff_sval.fits'), w, hdr
    mkhdr, hdr, ff_matrix, /EXTEND
    sxaddpar, hdr, "TRACKNUM", fits_tn
    writefits, filepath(ROOT=dir,'ff_matrix.fits'), ff_matrix, hdr

    ; save m2c matrix file fits for optical loop.
    writefits, filepath(ROOT=dir,"m2c_matrix_"+strsign+strpos+"_"+stramp+strmodal+".fits"), m2c
    print, filepath(ROOT=dir,"ff_matrix_"+strsign+strpos+"_"+stramp+strmodal+ext)
    print, filepath(ROOT=dir,"m2c_matrix_"+strsign+strpos+"_"+stramp+strmodal+".fits")
    if modal then begin
        pos_eig = dblarr(adsec.n_actuators, adsec.n_actuators)
        for_eig = dblarr(adsec.n_actuators, adsec.n_actuators)
        pos_eig[0:n_elements(act_w_ff)-1 , act_w_ff] = v
        for_eig[0:n_elements(act_w_ff)-1 , act_w_ff] = u

        err = find_cal_zo(alpha=alpha, betac=betac, pos_eig=pos_eig, for_eig=for_eig, PATH_SAVE=dir, EXT=ext)
    endif

end
