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
;-


Function ff_acquire_v2, tn

    @adsec_common
    ;restoring default data
    err= read_seq_dsp(0, switch_map.enableacccorrection, 1l, bb, /sw, /UL) 
    if err ne adsec_error.ok then return, err
    if bb ne 0UL then begin
        log_print, "The accelerometer correction is enabled and set to "+string(bb, format='(I4.4)')+": no ff matrix acquisition possible. Returning."
        return, adsec_error.generic_error
    endif
    


    default=read_ascii_structure(filepath(ROOT=adsec_path.conf,SUB='scripts', 'ff_acquire_default.txt'))
    aostatus = ptr2value(!AO_STATUS)
    if n_elements(tn) eq 0 then tn = tracknum()

    ;updating the status of IDL registers.
    err = update_status()

    if err ne adsec_error.ok then begin 
        log_print, "Error updating the status"
        return, err
    endif

    ;Set up....
    cl_act = where(sys_status.closed_loop, n_cl_act)
    if n_cl_act eq 0 then begin
        log_print, "No actuator in closed loop"
        return, err
    endif


    n_cycles=default.n_cycles

    ;the amp check is on ff_data_acquisition
    amp = default.amp

    modal = default.modal
    ff = default.ff   

    max_peak_force=adsec.weight_curr*default.max_peak_fact 

;    dir = filepath(ROOT=adsec_path.meas,SUB=[default.path_0,default.path_1],'')
    dir = meas_path('ff_matrix' ,DAT=tn)
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
            log_print, 'Unable to test the noise for the covariance matrix retrieval.'
            return, err
        endif

        ;; compute the covariance matrix of the mean
     ;   if (size(covar))[0] eq 3 then $
            m_covar=total(covar,3)/(long(times)*samples) 
     ;   else $
     ;       m_covar=covar/samples

; PATCH
        ;window, 0, retain=2
        dummy=m_covar[*,cl_act]
        dummy=dummy[cl_act,*]
        min_v=min(dummy)
        max_v=max(dummy)
        image_show, m_covar, /AS, /SH, max_v=max_v, min_v=min_v
        
        ;window, 1, retain=2
        ;display, extract_diagonal(m_covar), cl_act
        ;window, 2, retain=2
;PATCH
    endif
    ;
    ;window, 0, retain=2
    ;wset, 0
    
    err = get_gain(sc.all, gain)
    if err ne adsec_error.ok then log_print,  "Error on gain reading."
    gain_n2m = gain * 1e6 ;from N/um to N/M

    save, file = dir+'temp_data_ff'+"_"+stramp+strmodal+ext+'.sav',  nmeas, amp, samples, cl_act, gain_n2m, n_cl_act, m_covar, status_save, modal, no_match_ordering, stramp,strmodal,ext, strsign, strpos, stramp, filename, aostatus

    err= ff_data_acquisition_v2(nmeas, amp, samples, data, FF=ff, MODAL=modal, CL_ACT=cl_act  $
                                , SMOOTH=do_smooth, MAX_PEAK_FORCE=max_peak_force, BIAS_CURRENT=bias_current, TN=tn)
    if err ne adsec_error.ok then return, err

    if old_ot gt 0 then begin
        err = set_diagnostic(/master)
        if err ne adsec_error.ok then return, err
        err = set_diagnostic(over=old_ot)
        if err ne adsec_error.ok then return, err
        wait, 2*old_ot > 0.01
    endif

    err = update_status()
    if err ne adsec_error.ok then log_print,  "Error on updating status."

    err = get_status(status_save)
    if err ne adsec_error.ok then log_print,  "Error on updating status."

    
    save, file = dir+'temp_data_ff'+"_"+stramp+strmodal+ext+'.sav',  nmeas, amp, samples, data, cl_act, bias_current, gain_n2m, n_cl_act, m_covar, status_save, modal, no_match_ordering, stramp,strmodal,ext, strsign, strpos, stramp, filename, aostatus
End

