;$Id: set_icc.pro,v 1.7 2009/07/03 13:25:32 labot Exp $$
;+
;	NAME:
;    SET_ICC
;	
;	PURPOSE:
;    Set up control current integrator parametets
;
;	CALLING SEQUENCE:
;    err=set_icc(cutfreq, gain, [/RESET])
;
;   INPUT:
;    cutfreq: cut frequency for the pseudo-integrator
;    gain:    gain to apply to the pseudo-integrator
;   
;   OUTPUT:
;    err:   error code
;
;   KEYWORD:
;    RESET: if set, clean all coefficients of control current integrator
;    HOLD:  filter parameters setting to hold current integrated current
;
;   HISTORY:
;    Written by Marco Xompero (MX)
;    on 04 Nov 2007
;    marco@arcetri.astro.it
;    12 Dic 2007, MX
;    HOLD keyword added.
;    31 Jan 2008, Daniela Zanotti (DZ)
;    SLOPE_FREQUENCY keyword added.   
;    13 Feb 2008, MX, DZ, Armando Riccardi (AR)
;    After fixing the integrator control current the computation of 
;    the parameters is made before the diagnostic stop.      
;-

Function set_icc, cutfreq_in, dc_gain_in, RESET=reset, HOLD=hold, SLOPE_FREQUENCY=slope_frequency, ACT_LIST=no_icc_list

    @adsec_common
    
    if n_elements(no_icc_list) eq 0 then no_icc_list = adsec.act_wo_icc
    case no_icc_list[0] of
        -1: act_list = adsec.act_w_cl
        -2: act_list = adsec.act_w_cl
        else: begin
            err = complement(no_icc_list, indgen(adsec.n_actuators), dummy_list)
            if err ne 0 then log_message, "ERROR", ERR=err, CONT= (sc.debug eq 0)
            err = intersection(dummy_list, adsec.act_w_cl, act_list)
            if err ne 0 then log_message, "ERROR", ERR=err, CONT= (sc.debug eq 0)
        end
    endcase

    old_ovs = rtr.oversampling_time 
    if ~keyword_set(SLOPE_FREQUENCY) then samp = old_ovs else samp=1./slope_frequency

;    err = set_diagnostic(OVER=rtr.oversampling_time, TIMEOUT=2000.)
;    if err ne 0 then log_message, "ERROR", ERR=err, CONT= (sc.debug eq 0)

    err = set_diagnostic(over=0.0, TIMEOUT=2000.)
    if err ne adsec_error.ok then return, err

    if keyword_set(RESET) then begin

        err = set_fc_gain(/RESET, /ALL)
        if err ne adsec_error.ok then log_message, "Error encountered: "+strtrim(err,2), ERR=err, CONT= (sc.debug eq 0)
        dc_gain = 0
        cutfreq =  0

    endif else begin

        if keyword_set(HOLD) then begin
            
    ;        err = set_diagnostic(over=0.0, /FREEZE_TIMEOUT)
    ;        if err ne adsec_error.ok then return, err
            err = set_fc_gain(/RESET, /ALL)
            if err ne adsec_error.ok then log_message, "Error encountered: "+strtrim(err,2), CONT= (sc.debug eq 0)
            af1 = replicate(0.0, adsec.n_actuators)
            af1[act_list] = 1.0
            err = set_fc_gain(af1, /af1)
            if err ne adsec_error.ok then log_message, "Error encountered: "+strtrim(err,2), CONT= (sc.debug eq 0)
            dc_gain = 0
            cutfreq = 0
            
        endif else begin

            dc_gain = dc_gain_in
            cutfreq = cutfreq_in
            err = int_ctrl_curr_coeffs(cutfreq, BFO = s_bf0, BF1 = s_bf1, AF1 = s_af1, dc_gain=dc_gain, SAMP=samp)
            if err ne adsec_error.ok then log_message, "Error encountered: "+strtrim(err,2), CONT= (sc.debug eq 0)
            bf0 = replicate(0.0, adsec.n_actuators)
            bf0[act_list] = s_bf0
     ;       err = set_diagnostic(over=0.0, /FREEZE_TIMEOUT)
     ;       if err ne adsec_error.ok then return, err
            err = set_fc_gain(bf0, /bf0)
            if err ne adsec_error.ok then log_message, "Error encountered: "+strtrim(err,2), CONT= (sc.debug eq 0)
            bf1 = replicate(0.0, adsec.n_actuators)
            bf1[act_list] = s_bf1
            err = set_fc_gain(bf1, /bf1)
            if err ne adsec_error.ok then log_message, "Error encountered: "+strtrim(err,2), CONT= (sc.debug eq 0)
            af1 = replicate(0.0, adsec.n_actuators)
            af1[act_list] = s_af1
            err = set_fc_gain(af1, /af1)
            if err ne adsec_error.ok then log_message, "Error encountered: "+strtrim(err,2), CONT= (sc.debug eq 0)
        endelse

    endelse
    err = set_diagnostic(over=old_ovs)
    if err ne adsec_error.ok then return, err

    sys_status.icc_dcgain = dc_gain
    sys_status.icc_dcgain[adsec.act_wo_icc] = 0
    sys_status.icc_cutfreq = cutfreq
    sys_status.icc_cutfreq[adsec.act_wo_icc] = 0

    return, adsec_error.ok

End
