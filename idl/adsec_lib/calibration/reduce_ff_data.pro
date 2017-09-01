Function reduce_ff_data, tn, do_plot=do_plot, FORCEMEAS=forcemeas, FILTEROUT=filterout

    @adsec_common
    loadct, 12
    path = getenv('ADOPT_MEAS')+'/adsec_calib/ff_matrix/'+tn+'/'
    fffile = file_search(path+'temp_data_ff*')
    if (file_info(fffile)).exists eq 0 then begin
        log_print, "The tracking number provided does not correspond to a valid ff matrix data set."
        return, adsec_error.file_error
    endif

    restore, fffile,/ver
    data0 = data
    ifile = file_info(path+'data_proto.sav')
    if ifile.exists eq 1 then begin
        print, "using partial saved data"
    ;    catch, error_status
    ;    if error_status ne 0 then begin
    ;        log_print, "PARTIAL DATA FOUND: restarting procedure...."
    ;        nmeas = i-1
    ;    endif
        restore, path+'data_proto.sav'
        restore, path+'bias_current.sav'
        data=replicate(data_proto, 2*nmeas+3)
        openr, unit, path+'rawdata.bin', /get
        datadisk = assoc(unit, data_proto)
        for i=0L, 2*nmeas+3-1 do begin
            data[i] = datadisk[i]
        endfor
       ;position
        tmp = data.position
        tmp = shift(temporary(tmp), 0,-1)
        data.position = temporary(tmp)

       ;current
        tmp = data.current
        tmp = shift(temporary(tmp), 0,-1)
        data.current = temporary(tmp)

       ;rms_pos
        tmp = data.rms_pos
        tmp = shift(temporary(tmp), 0,-1)
        data.rms_pos = temporary(tmp)

       ;rms_curr
        tmp = data.rms_curr
        tmp = shift(temporary(tmp), 0,-1)
        data.rms_curr = temporary(tmp)

        ;data cut
        data = temporary(data[0:2*nmeas+1])


    endif


    ;Show noise covariance
    ;if (size(covar))[0] eq 3 then m_covar=total(covar,3)/(long(times)*samples) $
    ;    else m_covar=covar/samples
    dummy=m_covar[*,cl_act]
    dummy=dummy[cl_act,*]
    min_v=min(dummy)
    max_v=max(dummy)
    m_covar_plot = m_covar
    err = complement(cl_act, indgen(adsec.n_actuators), out)
    if err ne 0 then begin
        m_covar_plot[out, *]=0
        m_covar_plot[*, out]=0
    endif
    if keyword_set(do_plot) then begin
        window, /FREE, retain=2
        image_show, m_covar_plot, /AS, /SH, max_v=max_v, min_v=min_v
        window, /FREE, retain=2
        display, (extract_diagonal(m_covar_plot))[adsec.act_w_cl], adsec.act_w_cl, /sh
    endif
;    restore, fffile, /ver
    dir = file_dirname(fffile)
    if n_elements(filterout) gt 0  then begin
        if ~(file_info(dir+'/filtered')).exists then file_mkdir, dir+'/filtered'
        dir = dir+'/filtered'
    endif

    err= ff_fit_v2(data, samples, ff_matrix, ff_u, ff_w, ff_v, df, dx, dc, BIAS_CURRENT=bias_current[cl_act] $
                  , GAIN=gain_n2m[cl_act], SINGULAR=n_sv, SIGMA=sigma_ff_matrix, CHISQ=chisq_ff_matrix $
                  , COVAR=m_covar, CL_ACT=cl_act, NO_MATCH_ORDERING=no_match_ordering, FILTEROUT=filterout)
    if err ne adsec_error.ok then begin
        log_print, "Error fitting the data acquired.", CONT=(sc.debug eq 0B)
        return, err
    endif

    if n_elements(filterout) eq 0 then filterout=-1
    dof = (nmeas-n_cl_act)*n_cl_act
    log_print, "Normalized Chi^2 ff_matrix:" + string(chisq_ff_matrix/dof)+ "  DOF:"+ strtrim(dof)

    f_calib_vector=0
    chisq_f_calib=0
    sigma_f_calib=0
    n_sv_f_calib=0

    u=ff_u
    v=ff_v
    w=ff_w
    act_w_ff = cl_act

    res = df-ff_matrix##dx
    res_var = total(res^2, 1)/nmeas

    if keyword_set(do_plot) then begin
        window, /FREE
        image_show, res, /SH, TITLE="residuals: df-ff_matrix##dx"
        window, /FREE
        display, sqrt(res_var), cl_act, TITLE="residual rms", /SH, /NO_N

    endif

    ff_full = dblarr(adsec.n_actuators, adsec.n_actuators)
    dummy = dblarr(adsec.n_actuators, n_cl_act)
    dummy[cl_act,*]=ff_matrix
    ff_full[*,cl_act]=dummy

    ff_reorder = ff_full[adsec.mir_act2act,*]
    ff_reorder = ff_reorder[*,adsec.mir_act2act]

    time_acq=systime(/sec)
    ;;save modes to command matrix
    m2c = v 

    ;; save the ff_matrix and force calibration fits
    save, nmeas, amp, delay, samples, ff, times, max_min, sigma_pos,  covar, data, ff_matrix, ff_u, ff_w, ff_v, df, dx, dc, $
          gain_n2m, n_sv, sigma_ff_matrix, chisq_ff_matrix, modal, f_calib_vector, chisq_f_calib, sigma_f_calib, n_sv_f_calib, $
          cl_act, ff_reorder,time_acq, status_save, aostatus, filterout, FILE=filepath(ROOT=dir, filename)+'.sav'
    fits_tn = (stregex(dir, '[0-9]+_[0-9]+',/EX))[0]
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
    ;writefits, filepath(ROOT=dir,"m2c_matrix_"+strsign+strpos+"_"+stramp+strmodal+".fits"), m2c
    ;print, filepath(ROOT=dir,"ff_matrix_"+strsign+strpos+"_"+stramp+strmodal+ext)
    ;print, filepath(ROOT=dir,"m2c_matrix_"+strsign+strpos+"_"+stramp+strmodal+".fits")
    ;if modal then begin
;    pos_eig = dblarr(adsec.n_actuators, adsec.n_actuators)
;    for_eig = dblarr(adsec.n_actuators, adsec.n_actuators)
;    pos_eig[0:n_elements(act_w_ff)-1 , act_w_ff] = v
;    for_eig[0:n_elements(act_w_ff)-1 , act_w_ff] = u


;    err = find_cal_zo(alpha=alpha, betac=betac, pos_eig=pos_eig, for_eig=for_eig, PATH_SAVE=dir, EXT=ext, /PIST)
;    ;endif


    

    

End
