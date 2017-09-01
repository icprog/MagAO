;$Id: test_intfc.pro,v 1.5 2007/04/12 13:06:10 labot Exp $$
;+
;   NAME:
;       TEST_INTFC
;
;   PURPOSE:
;    ROUTINE TEST FOR THE USE OF FORCE CONTROL INTEGRATOR.
;
;   NOTE:
;
;   USAGE:
;    test_intfc, AF1=af1, BF0=bf0, BF1=bf1, ALL=all , NO_TH=no_th, N_FRAMES=n_frames, PSEUDO_INT=PSEUDO_INT
;
;   INPUT:
;
;   OUTPUT:
;       None.
;
;   KEYWORDS:
;       AF1 = test on filter AF1 
;       BF0 = test on filter bf0
;       BF1 = test on filter bf1
;       NO_TH = no threshold applied
;       N_FRAMES = n_frames
;       PSEUDO_INT = implementation of pseudo integrator (tustin mode)
;
;
;    HISTORY
;
;    April 2007, Daniela Zanotti
;-


Pro test_intfc,AF1=af1, BF0=bf0, BF1=bf1, ALL=all , NO_TH=no_th, N_FRAMES=n_frames, PSEUDO_INT=pseudo_int
 
    @adsec_common
    common inc_fc_block, c1_old, old_modes, delaya, delayb
    ;block = 0
    err = set_cmd2bias()
    if err ne adsec_error.ok then message, "Error encountered: "+strtrim(err,2)

    err = clear_dl()
    if err ne adsec_error.ok then message, "Error encountered: "+strtrim(err,2)

    dummy = replicate(1e-6, adsec.n_actuators)
    
    if keyword_set(NO_TH) then dummy[indgen(adsec.n_actuators)] = 1e6
    dummy = replicate(1e6, adsec.n_actuators)
    err = set_skip_par(dummy, /MODE, /DELTA)
    if err ne adsec_error.ok then message, "Error encountered: "+strtrim(err,2)

    dummy = replicate(10e-6, adsec.n_actuators)
    dummy[adsec.dummy_act] = 1e6
    if keyword_set(NO_TH) then dummy[indgen(672)] = 1e6
    err = set_skip_par(dummy, /MODE, /MAX)
    if err ne adsec_error.ok then message, "Error encountered: "+strtrim(err,2)

    dummy = replicate(25e-6, adsec.n_actuators)
    dummy[adsec.dummy_act] = -1e6 
    if keyword_set(NO_TH) then dummy[indgen(adsec.n_actuators)] = -1e6
    err = set_skip_par(dummy, /CMD, /MIN)
    if err ne adsec_error.ok then message, "Error encountered: "+strtrim(err,2)

    dummy = replicate(110e-6, adsec.n_actuators)
    dummy[adsec.dummy_act] = 1e6
    if keyword_set(NO_TH) then dummy[indgen(adsec.n_actuators)] = 1e6
    err = set_skip_par(dummy, /CMD, /MAX)
    if err ne adsec_error.ok then message, "Error encountered: "+strtrim(err,2)

    dummy = replicate(1e-6, adsec.n_actuators)
    dummy[adsec.dummy_act] = 1e6
    if keyword_set(NO_TH) then dummy[indgen(adsec.n_actuators)] = 1e6
    err = set_skip_par(dummy, /CMD, /DELTA)
    if err ne adsec_error.ok then message, "Error encountered: "+strtrim(err,2)

    dummy = replicate(-0.5, adsec.n_actuators)
    dummy[adsec.dummy_act] = -1e6
    if keyword_set(NO_TH) then dummy[indgen(adsec.n_actuators)] = -1e6
    err = set_skip_par(dummy, /FORCE, /MIN)
    if err ne adsec_error.ok then message, "Error encountered: "+strtrim(err,2)

    dummy = replicate(0.5, adsec.n_actuators)
    dummy[adsec.dummy_act] = 1e6
    if keyword_set(NO_TH) then dummy[indgen(adsec.n_actuators)] = 1e6
    err = set_skip_par(dummy, /FORCE, /MAX)
    if err ne adsec_error.ok then message, "Error encountered: "+strtrim(err,2)

    c1_old = 0.0
    old_modes = fltarr(adsec.n_actuators)
    delaya = replicate(0.0,1, adsec.n_actuators)
    delayb = replicate(0.0,1, adsec.n_actuators)


    disturb_mode = 55; 55 for 672
    mode=readfits("../adsec_lib/test/mode_"+string(disturb_mode,FORMAT="(I2.2)")+".fits")
    dlen = long(rtr.disturb_len)
    t = findgen(dlen)/20*2*!PI
    amp = sin(t)
    hist = rebin(amp,dlen,adsec.n_actuators) $
             *rebin(reform(mode,1,adsec.n_actuators),dlen,adsec.n_actuators)
    hist = float(hist)
    dhist = [fltarr(1,adsec.n_actuators),hist[1:dlen-1,*]-hist[0:dlen-2,*]]
    
    mod_hist = float(transpose(adsec.ff_p_svec)##hist)
    dmod_hist = [fltarr(1,adsec.n_actuators),hist[1:dlen-1,*]-hist[0:dlen-2,*]]
    
    if rtr.n_modes2correct ne adsec.n_actuators then $
      message, "num of modes has to be equal to num of actuators for the test"
    
    b0 = fltarr(rtr.n_slope,adsec.n_actuators)
    b_delay = fltarr(rtr.n_slope,rtr.n_modes2correct)       ;(1 delay slopes)
    a_delay = fltarr(adsec.n_actuators,adsec.n_actuators,2) ;(2 delay modes)
    m2c = fltarr(rtr.n_modes2correct,adsec.n_actuators)


    b0[0,0] = identity(adsec.n_actuators)
    m2c[0,0] = identity(adsec.n_actuators)
    
    
    g_gain = replicate(1.0, adsec.n_actuators)
 
    err = set_b0_matrix(b0)
    if err ne adsec_error.ok then message, "Error encountered: "+strtrim(err,2)

    err = set_b_delay_matrix(b_delay)
    if err ne adsec_error.ok then message, "Error encountered: "+strtrim(err,2)

    err = set_a_delay_matrix(a_delay)
    if err ne adsec_error.ok then message, "Error encountered: "+strtrim(err,2)
    
    err = set_m2c_matrix(m2c)
    if err ne adsec_error.ok then message, "Error encountered: "+strtrim(err,2)

    err = set_g_gain(sc.all, g_gain)
    if err ne adsec_error.ok then message, "Error encountered: "+strtrim(err,2)

    ;cleaning of all gain block
    err = set_fc_gain(/all)
    if err ne adsec_error.ok then message, 'I Guadagni non sono stati puliti'

    ; clean all the int control current
    err = write_same_ch(sc.all, dsp_map.int_control_current, 0l, /ch)
    err = write_same_ch(sc.all, dsp_map.mean_control_current, 0l, /ch)


    cmd_preshaper = time2step_preshaper(0.9e-3, APPLIED=cmd_preshaper_sec, /VERB)
    cur_preshaper = time2step_preshaper(0.9e-3, APPLIED=cur_preshaper_sec, /VERB)

    ;set preshapers
    err = set_preshaper(sc.all, cmd_preshaper)
    if err ne adsec_error.ok then message, "ERROR"
    err = set_preshaper(sc.all, cur_preshaper, /FF)
    if err ne adsec_error.ok then message, "ERROR"

    acc_delay = ceil(0.9e-3/adsec.sampling_time)
    acc_len   = long(1e-3/adsec.sampling_time-acc_delay)

    ;set the accumulators
    err = set_acc(acc_len,acc_delay)
    if err ne adsec_error.ok then message, "ERROR"
    wait, 0.1
    err = start_acc(/WAIT)
    if err ne adsec_error.ok then message, "ERROR"

    err = set_disturb(/DISABLE)
    if err ne adsec_error.ok then message, "ERROR"

 ;   print, set_diagnostic(/master)
;    if n_elements(n_frames) gt 0 then numframes = n_frames < (dlen-1) else numframes = dlen -1
    ; pseudo deriv
    T = 1.5
    f0 = 0.2    
    w0 = 2*!dpi*f0
    a = float(2.0/T+w0)
    b =  float(2.0/T-w0)  
    coeff_af1  = b/a
    coeff_bf0  = 1./a
    coeff_bf1  = 1./a 
    
    if keyword_set(AF1) then   begin
        if keyword_set(pseudo_int) then af01 = replicate(coeff_af1, adsec.n_actuators) $
        else  af01 = randomn(seed,adsec.n_actuators)
        
        err=set_fc_gain(af01,/af1, block=block)
        if err ne adsec_error.ok then message, 'Error writing Af0 matrix!!'
    endif else af01 = fltarr(adsec.n_actuators)


   if keyword_set(BF0) then   begin
        if keyword_set(pseudo_int) then bf00 = replicate(coeff_bf0, adsec.n_actuators) $
        else bf00 = randomn(seed,adsec.n_actuators)
        err=set_fc_gain(bf00,/bf0, block=block)
        if err ne adsec_error.ok then message, 'Error writing Bf0 matrix!!'
    endif else bf00 = fltarr(adsec.n_actuators)

    if keyword_set(BF1) then   begin
        if keyword_set(pseudo_int) then bf01 = replicate(coeff_bf1, adsec.n_actuators) $
        else bf01 = randomn(seed,adsec.n_actuators)
        err=set_fc_gain(bf01,/bf1, block=block)
        if err ne adsec_error.ok then message, 'Error writing Bf1 matrix!!'
    endif  else bf01 =fltarr(adsec.n_actuators)


; the first reading

    err=read_seq_ch(sc.all, dsp_map.int_control_current, 1l,ffc1)
        if err then message, 'Errore in lettura'
    if total(ffc1) ne 0.0 then message, 'Error in the first step'

  ; test 1
    err=read_seq_ch(sc.all, dsp_map.mean_control_current, 1l, mctrl_curr0)
        if err then message, 'Errore nella lettura della mean ctrl'
    bfc1 = mamano(diagonal_matrix(bf01),reform(mctrl_curr0)) 
    
    for i=0, 5 do begin

        slopes = [reform(hist[i,*]),fltarr(rtr.n_slope-adsec.n_actuators)]
        inc_fc, slopes, hist[i,*], /SWITCH, /DOCH
        wait, 0.5
        err=read_seq_ch(sc.all, dsp_map.int_control_current, 1l,ffc0)
        if err then message, 'Errore in lettura di int ctrl'

        err=read_seq_ch(sc.all, dsp_map.mean_control_current, 1l, mctrl_curr0)
        if err then message, 'Errore nella lettura della mean ctrl'
       
        bfc0 = mamano(diagonal_matrix(bf00),reform(mctrl_curr0))
        afc1 = mamano(diagonal_matrix(af01),reform(ffc1))
        ffc_as = float(bfc0+bfc1)+afc1
        ffc_real = ffc0

        err_mac=(machar()).eps
        ok= total(ffc_as-reform(ffc_real))
            if ok ne 0.0 then begin
                print, ok
                if total(abs(ffc_as)-abs(reform(ffc_real))) ge err_mac then message, 'Test fallito!'
            endif

        ffc1=ffc0
        bfc1 =mamano(diagonal_matrix(bf01),reform(mctrl_curr0))
        ;stop

     endfor

    err = set_g_gain(sc.all, /reset)
    if err ne adsec_error.ok then message, "Error encountered: "+strtrim(err,2)
    err = set_fc_gain(/all)
    if err ne adsec_error.ok then message, 'I Guadagni non sono stati puliti'

    
end
            
