;$Id: test_bench.pro,v 1.1 2007/04/12 13:40:15 labot Exp $$
;+
;   NAME:
;       TEST_BENCH
;
;   PURPOSE:
;    ROUTINE TO TEST DIFFERENT BENCH MEMORY IN OPTICAL LOOP.
;
;   NOTE:
;
;   USAGE:
;    test_bench,INTEGRATOR_MODES = intmodes, NO_TH=no_th, N_FRAMES=n_frames,
;              
;
;   INPUT:
;
;   OUTPUT:
;       None.
;
;   KEYWORDS:
;   INTEGRATOR_MODES    = enable integrator modes.
;   NO_TH               = disable thresholds
;
;
;    HISTORY
;
;    03 April 2007, Daniela Zanotti(DZ)
;-

Pro test_bench,  INTEGRATOR_MODES = intmodes, NO_TH=no_th, N_FRAMES=n_frames
 
    @adsec_common
    common inc_fc_bench_block, c1_old, old_modes, delaya, delayb

    err = set_cmd2bias()
    if err ne adsec_error.ok then message, "Error encountered: "+strtrim(err,2)

    err = clear_dl()
    if err ne adsec_error.ok then message, "Error encountered: "+strtrim(err,2)

    if keyword_set(intmodes) then dummy = replicate(10e-6, adsec.n_actuators) else dummy = replicate(1e-6, adsec.n_actuators)
    if keyword_set(NO_TH) then dummy[indgen(672)] = 1e6
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
    if keyword_set(NO_TH) then dummy[indgen(672)] = -1e6
    err = set_skip_par(dummy, /CMD, /MIN)
    if err ne adsec_error.ok then message, "Error encountered: "+strtrim(err,2)

    dummy = replicate(110e-6, adsec.n_actuators)
    dummy[adsec.dummy_act] = 1e6
    if keyword_set(NO_TH) then dummy[indgen(672)] = 1e6
    err = set_skip_par(dummy, /CMD, /MAX)
    if err ne adsec_error.ok then message, "Error encountered: "+strtrim(err,2)

    if keyword_set(intmodes) then dummy = replicate(10e-6, adsec.n_actuators) else dummy = replicate(1e-6, adsec.n_actuators)
    dummy[adsec.dummy_act] = 1e6
    if keyword_set(NO_TH) then dummy[indgen(672)] = 1e6
    err = set_skip_par(dummy, /CMD, /DELTA)
    if err ne adsec_error.ok then message, "Error encountered: "+strtrim(err,2)

    dummy = replicate(-0.5, adsec.n_actuators)
    dummy[adsec.dummy_act] = -1e6
    if keyword_set(NO_TH) then dummy[indgen(672)] = -1e6
    err = set_skip_par(dummy, /FORCE, /MIN)
    if err ne adsec_error.ok then message, "Error encountered: "+strtrim(err,2)

    dummy = replicate(0.5, adsec.n_actuators)
    dummy[adsec.dummy_act] = 1e6
    if keyword_set(NO_TH) then dummy[indgen(672)] = 1e6
    err = set_skip_par(dummy, /FORCE, /MAX)
    if err ne adsec_error.ok then message, "Error encountered: "+strtrim(err,2)

    c1_old = 0.0
    old_modes = fltarr(adsec.n_actuators)
    delaya = replicate(0.0,1, adsec.n_actuators)
    delayb = replicate(0.0,1, adsec.n_actuators)


    disturb_mode = 55 ;for 672
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
    ; inizializzo blocco A 
    T = 1.5
    f0 = 0.2
    w0 = 2*!dpi*f0
    a = float(2.0/T+w0)
    b =  float(2.0/T-w0)
    coeff_af1_a  = b/a
    coeff_bf0_a  = 1./a
    coeff_bf1_a  = 1./a
    T = 1.5
    f0 = 0.2
    w0 = 0
    a = float(2.0/T+w0)
    b =  float(2.0/T-w0)
    coeff_af1_b  = b/a
    coeff_bf0_b  = 1./a
    coeff_bf1_b  = 1./a

    b0_a = fltarr(rtr.n_slope,adsec.n_actuators)
    b_delay_a = fltarr(rtr.n_slope,rtr.n_modes2correct)       ;(1 delay slopes)
    b_delay_a[0,0] = identity(rtr.n_modes2correct)*10.0
    a_delay = fltarr(adsec.n_actuators,adsec.n_actuators,2) ;(2 delay modes)
    m2c=readfits(adsec_path.data+'m2c_matrix.fits')
    ;m2c = fltarr(rtr.n_modes2correct,adsec.n_actuators)
    g_gain_a = replicate(1.0, adsec.n_actuators)+20.0
    b0_a[0,0] = identity(adsec.n_actuators)*7.5
    af01_a = replicate(coeff_af1_a, adsec.n_actuators)
    bf00_a = replicate(coeff_bf0_a, adsec.n_actuators)
    bf01_a = replicate(coeff_bf1_a, adsec.n_actuators)
    ;inizializzo blocco b   
    
    b0_b = fltarr(rtr.n_slope,adsec.n_actuators)
    b_delay_b = fltarr(rtr.n_slope,rtr.n_modes2correct)       ;(1 delay slopes)
    b_delay_b[0,0] = identity(rtr.n_modes2correct)*0.5
    g_gain_b = replicate(1.0, adsec.n_actuators)+1000.0
    b0_b[0,0] = identity(adsec.n_actuators)*3.0
    af01_b = replicate(coeff_af1_b, adsec.n_actuators)
    bf00_b = replicate(coeff_bf0_b, adsec.n_actuators)
    bf01_b = replicate(coeff_bf1_b, adsec.n_actuators)

    
    if keyword_set(intmodes) then a_delay[*,*,0] = identity(rtr.n_modes2correct)
    m2c[0,0] = identity(adsec.n_actuators)
    
    g_gain = replicate(1.0, adsec.n_actuators)
 
    err = set_b0_matrix(b0_a)
    if err ne adsec_error.ok then message, "Error encountered: "+strtrim(err,2)

    err = set_b0_matrix(b0_b, /block)
    if err ne adsec_error.ok then message, "Error encountered: "+strtrim(err,2)

    err = set_b_delay_matrix(b_delay_a)
    if err ne adsec_error.ok then message, "Error encountered: "+strtrim(err,2)

    err = set_b_delay_matrix(b_delay_b, /block)
    if err ne adsec_error.ok then message, "Error encountered: "+strtrim(err,2)

    err = set_a_delay_matrix(a_delay)
    if err ne adsec_error.ok then message, "Error encountered: "+strtrim(err,2)
    
    err = set_m2c_matrix(m2c)
    if err ne adsec_error.ok then message, "Error encountered: "+strtrim(err,2)

    err = set_g_gain(sc.all, g_gain_a)
    if err ne adsec_error.ok then message, "Error encountered: "+strtrim(err,2)

    err = set_g_gain(sc.all, g_gain_b,/block)
    if err ne adsec_error.ok then message, "Error encountered: "+strtrim(err,2)
    
    err=set_fc_gain(af01_a,/af1)
    if err ne adsec_error.ok then message, 'Error writing Af0 matrix!!'

    err=set_fc_gain(af01_b,/af1,/block)
    if err ne adsec_error.ok then message, 'Error writing Af0 matrix!!'

    err=set_fc_gain(bf00_a,/bf0)
    if err ne adsec_error.ok then message, 'Error writing Af0 matrix!!'

    err=set_fc_gain(bf00_b,/bf0,/block)
    if err ne adsec_error.ok then message, 'Error writing Af0 matrix!!'

    err=set_fc_gain(bf01_a,/bf1)
    if err ne adsec_error.ok then message, 'Error writing Af0 matrix!!'

    err=set_fc_gain(bf01_b,/bf1,/block)
    if err ne adsec_error.ok then message, 'Error writing Af0 matrix!!'



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

    ;DISTURBANCE FIRMWARE SETTINGS
    ;if keyword_set(DISTURBANCE) then begin
       
    ;    err = set_disturb(hist, /RESET, /ENABLE) 
    ;    if err ne adsec_error.ok then message, "ERROR"
        
    ;endif else begin
        
       ;err = set_disturb(hist, /RESET, /DISABLE) 
        err = set_disturb(/DISABLE)
        if err ne adsec_error.ok then message, "ERROR"

    ;endelse

    print, set_diagnostic(/master)
    if n_elements(n_frames) gt 0 then numframes = n_frames < (dlen-1) else numframes = dlen -1

    i=0
    BenchInUse = 0
    for j=0, 21 do begin
      
        if (j mod 2) then BenchInUse = 1 else BenchInUse = 0 
        
        for k=0,1 do begin
          
            slopes = [reform(hist[i,*]),fltarr(rtr.n_slope-adsec.n_actuators)]
            inc_fc_bench, slopes, hist[i,*], /SWITCH, /DOCH, BENCH=BenchInUse
            i+=1
            print, 'Banco:', BenchInUse    
        endfor
      
     endfor

; clean all setting gains
    err = set_g_gain(sc.all, /reset)
    if err ne adsec_error.ok then message, "Error encountered: "+strtrim(err,2)
    err = clear_ff()
    if err ne adsec_error.ok then message, "Error cleaning the ff current"
    err = set_fc_gain(/all)
    if err ne adsec_error.ok then message, 'I Guadagni non sono stati puliti'

end
            
