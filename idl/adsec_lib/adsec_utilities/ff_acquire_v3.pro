; $Id: ff_acquire_v2.pro,v 1.1 2009/11/25 08:52:46 marco Exp $
;+
;   NAME:
;    FF_ACQUIRE_V2
; 
;   PURPOSE:
;    Acquisition of Feed-forward matrix
;
;   USAGE:
;    ff_acquire_v2
;
;   INPUT:
;    None.
;   
;   OUTPUT:
;    None.
;
;   KEYWORDS:
;    None.
;
;   HISTORY:
;    Written by M.Xompero on 5 Jun 2007, fixing ff_fit.pro routine.
;    Osservatorio Asrofisico di Arcetri, ITALY
;    <marco@arcetri.astro.it>
;    2008 MX
;    Added set_diagnostic to the old oversampling time.   
;    20 Feb 2008 D.Zanotti 
;    The procedure was moved from adsecP45 to adsec672.
;    The realese is freezed after acceptance test in microgate. 
;    
;    28 feb Runa Briguglio: converted into a function to allow automatic ff_reduce launch; SAVED_FILE added as an output keyword
;-


function  ff_acquire_v3, REDUCE=reduce, SAVED_FILE=file2save

    @adsec_common
    ;restoring default data
    default=read_ascii_structure(filepath(ROOT=adsec_path.conf,SUB='scripts', 'ff_acquire_default.txt'))

    ;updating the status of IDL registers.
    err = update_status()

    if err ne adsec_error.ok then begin 
        message, "Error updating the status", CONT=(sc.debug eq 0B)
    ;    return, err
    endif

    ;Set up....
    cl_act = where(sys_status.closed_loop, n_cl_act)
    if n_cl_act eq 0 then begin
        message, "No actuator in closed loop", CONT=(sc.debug eq 0B)
    ;    return, err
    endif


    n_cycles=default.n_cycles

    ;the amp check is on ff_data_acquisition
    amp = default.amp

    modal = default.modal
    ff = default.ff   

    max_peak_force=adsec.weight_curr*default.max_peak_fact 

;    dir = filepath(ROOT=adsec_path.meas,SUB=[default.path_0,default.path_1],'')
    dir = meas_path('ff_matrix')
    ext = default.ext
    do_smooth = default.do_smooth
    no_match_ordering = default.no_match_ordering

    nmeas = n_cycles*n_cl_act
    delay = default.delay                     ;; accumulator settings
    samples = default.samples               ;; samples of accumulation

    times = default.times   ;; number of times the noise covar matrix is measured
    integ = default.integ  ;; set to use an integrator (don't use it if FF enabled)

    lin_ck = default.lin_ck ;; set to plot the linearity of commands/current

    ;; average commanded position for filename
    position = mean(sys_status.command[cl_act])

    stramp = strtrim(string(amp,format="(e10.3)"),2)
    if modal then strmodal = "m" else strmodal = "a"
    if position ge 0 then strsign="+" else strsign="-"
    strpos = strtrim(string(abs(position),format="(e10.3)"),2)
    filename="ff_"+strsign+strpos+"_"+stramp+strmodal+ext
    ;loadct, 3

    old_ot = rtr.oversampling_time
    if old_ot gt 0 then begin
        err = set_diagnostic(over=0.0)
        err = set_diagnostic(master=0.0)
        wait, 2*old_ot > 0.01
    endif

    ;***************
    if times ne 0 then begin
        err = test_noise(times, max_min, sigma_pos, coeff, covar,SAMPLES=samples)
        if err ne adsec_error.ok then begin
            message, 'Unable to test the noise for the covariance matrix retrieval.', CONT=(sc.debug eq 0B)
    ;        return, err
        endif

        ;; compute the covariance matrix of the mean
        if (size(covar))[0] eq 3 then $
            m_covar=total(covar,3)/(long(times)*samples) $
        else $
            m_covar=covar/samples

        window, 1, retain=2
        dummy=m_covar[*,cl_act]
        dummy=dummy[cl_act,*]
        min_v=min(dummy)
        max_v=max(dummy)
        image_show, m_covar, /AS, /SH, max_v=max_v, min_v=min_v
    endif
    ;
    window, 0, retain=2
    wset, 0
    
    err = get_gain(sc.all, gain)
    if err ne adsec_error.ok then message,  "Error on gain reading."
    gain_n2m = gain * 1e6 ;from N/um to N/M

    err= ff_data_acquisition_v2(nmeas, amp, samples, data, FF=ff, MODAL=modal, CL_ACT=cl_act  $
                                , SMOOTH=do_smooth, MAX_PEAK_FORCE=max_peak_force, BIAS_CURRENT=bias_current)
    ;if err ne adsec_error.ok then return, err
    if old_ot gt 0 then begin
        err = set_diagnostic(/master)
        err = set_diagnostic(over=old_ot)
        wait, 2*old_ot > 0.01
    endif

    err = update_status()
    if err ne adsec_error.ok then message,  "Error on updating status."

    err = get_status(status_save)
    if err ne adsec_error.ok then message,  "Error on updating status."

    file2save = dir + 'temp_data_ff'+"_"+stramp+strmodal+ext+'.sav'     
    save, file = file2save,  nmeas, amp, samples, data, cl_act, bias_current, gain_n2m, n_cl_act, m_covar, status_save, modal, no_match_ordering, stramp,strmodal,ext, strsign, strpos, stramp, filename;, delay, ff, times, max_min, sigma_pos, covar

  if (keyword_set(reduce)) then begin
      print, ff_reduce_v3(filename=file2save)
  endif
return, 0
End

;    err= ff_fit_v2(data, samples, ff_matrix, ff_u, ff_w, ff_v, df, dx, dc, BIAS_CURRENT=bias_current[cl_act] $
;                  , GAIN=gain_n2m[cl_act], SINGULAR=n_sv, SIGMA=sigma_ff_matrix, CHISQ=chisq_ff_matrix $
;                  , COVAR=m_covar, CL_ACT=cl_act, NO_MATCH_ORDERING=no_match_ordering)
;    if err ne adsec_error.ok then begin
;        message, "Error fitting the data acquired.", CONT=(sc.debug eq 0B)
;    ;    return, err
;    endif

;    dof = (nmeas-n_cl_act)*n_cl_act
;    print, "Normalized Chi^2 ff_matrix:",chisq_ff_matrix/dof, "  DOF:", strtrim(dof)

;;f_calib_fit, data, gain, m_covar, f_calib_vector, CHISQ=chisq_f_calib, SIGMA=sigma_f_calib, $
;    ;             SINGULAR=n_sv_f_calib
;    ;print, "Normalized Chi^2 f_calib  :",chisq_f_calib/((nmeas-1)*n_cl_act)

;    f_calib_vector=0
;    chisq_f_calib=0
;    sigma_f_calib=0
;    n_sv_f_calib=0

;    u=ff_u
;    v=ff_v
;    w=ff_w
;    act_w_ff = cl_act

;    ;; display residulas
;    if sc.name_comm ne "Dummy" then begin
    
;        res = df-ff_matrix##dx
;        res_var = total(res^2, 1)/nmeas
;        image_show, res, /SH, TITLE="residuals: df-ff_matrix##dx"

;        wset, 1
;        display, sqrt(res_var), cl_act, TITLE="residual rms", /SH

;        if lin_ck then begin
;            test_linear, data, cl_act
;        endif
;    endif
;    ff_full = dblarr(adsec.n_actuators, adsec.n_actuators)
;    dummy = dblarr(adsec.n_actuators, n_cl_act)
;    dummy[cl_act,*]=ff_matrix
;    ff_full[*,cl_act]=dummy

;    ff_reorder = ff_full[adsec.mir_act2act,*]
;    ff_reorder = ff_reorder[*,adsec.mir_act2act]

;    ;m2c = dblarr(adsec.n_actuators, adsec.n_actuators)
;    ;dummy = dblarr(adsec.n_actuators, n_cl_act)
;    ;dummy[cl_act,*]=ff_v
;    ;m2c[*,cl_act]=dummy

;    time_acq=systime(/sec)
;    ;;save modes to command matrix
;    m2c = v 

;    err = get_status(status_save)

;    ;; save the ff_matrix and force calibration fits
;    save, nmeas, amp, delay, samples, ff, times, max_min, sigma_pos,  covar, data, ff_matrix, ff_u, ff_w, ff_v, df, dx, dc, $
;          gain_n2m, n_sv, sigma_ff_matrix, chisq_ff_matrix, modal, f_calib_vector, chisq_f_calib, sigma_f_calib, n_sv_f_calib, $
;          cl_act, ff_reorder,time_acq, status_save, FILE=filepath(ROOT=dir, filename)

;        ;; save a file containing ff_matrix useable as startup ff_matrix file (see adsec_path.data directory)
;    save, ff_matrix, act_w_ff, u, w, v, FILE=filepath(ROOT=dir,"ff_matrix_"+strsign+strpos $
;             +"_"+stramp+strmodal+ext)
;    ; save m2c matrix file fits for optical loop.
;    writefits, filepath(ROOT=dir,"m2c_matrix_"+strsign+strpos+"_"+stramp+strmodal+".fits"), m2c
;    print, filepath(ROOT=dir,"ff_matrix_"+strsign+strpos+"_"+stramp+strmodal+ext)
;    print, filepath(ROOT=dir,"m2c_matrix_"+strsign+strpos+"_"+stramp+strmodal+".fits")
;    if modal then begin
;        pos_eig = dblarr(adsec.n_actuators, adsec.n_actuators)
;        for_eig = dblarr(adsec.n_actuators, adsec.n_actuators)
;        pos_eig[0:n_elements(act_w_ff)-1 , act_w_ff] = v
;        for_eig[0:n_elements(act_w_ff)-1 , act_w_ff] = u

;        err = find_cal_zo(alpha=alpha, betac=betac, pos_eig=pos_eig, for_eig=for_eig, PATH_SAVE=dir, EXT=ext)
;    endif

;end
