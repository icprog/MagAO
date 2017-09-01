;$Id: wfs_switch_step.pro,v 1.3 2009/07/28 14:21:34 labot Exp $$
;+
;   NAME:
;    WFS_SWITCH_STEP
;
;   PURPOSE:
;    Routine to simulate a WFS step sent to the Switch BCU.
;
;   USAGE:
;    err = wfs_switch_step(vect, WAIT=sleep_time,NOVERB=noverb, SHORT=short, WFSC=wfsc)
;
;   INPUT:
;    vect:              Slope vector.
;
;   NOTE:
;    This routine overrides the param_selector parameter.
;
;   OUTPUT:
;    err: error code.
;
;   KEYWORDS:
;    WAIT:      time to wait
;    NOVERB:    unset verbose mode
;    SHORT:     write only n_actuators values in slope vector
;    WFSC:      wfs counter to increase, else read in the electronic boards
;    TEST:      test if any skip parameter is set
;
;   HISTORY:
;    Created by Marco Xompero (MX) on 4 Jun 2007
;    marco@arcetri.astro.it
;
;-

Function wfs_switch_step, vect, WAIT=sleep_time, NOVERB=noverb, SHORT=short, WFSC=wfsc, TEST=test, PBS=pbs, ENA_DIST=ena_dist, DIS_DIST=dis_dist

    @adsec_common
    n_dsp = adsec.n_actuators/adsec.n_act_per_dsp
    
    if n_elements(vect) eq 0 then vect = fltarr(rtr.n_slope)

    ;Read Header from SwitchBCU
    ;
    ;Incrementing frames counter and setting param_selector in correct mode if not
    ;head[1] = head[1] or 2UL^6
    ;if (head[1] and 2UL^12) then message, "Disable disturbance before!!!"

    if n_elements(wfsc) eq 0 then begin
        err = read_seq_dsp(0,rtr_map.switch_SCStartRTR, 1, wfsc, /UL, /SWITCH)
        if err ne adsec_error.ok then return, err
    endif else begin
        wfsc += 1
    endelse

    if n_elements(pbs) eq 0 then begin
        err = read_seq_dsp(0,rtr_map.switch_SCStartRTR+1, 1, pbs, /UL, /SWITCH)
        if err ne adsec_error.ok then return, err
        pbs =  pbs or 2UL^6 or 2UL^7 ;(diagnostic SDRAM + FASTLINK ENABLE)
        if keyword_set(ENA_DIST) then pbs = pbs or 2UL^12 or 2UL^15
        if keyword_set(DIS_DIST) then pbs = not ((not pbs) or  2UL^12 or 2UL^15)


    endif

    dummy = [1UL, 0UL]
    
    ;Write Header from SwitchBCU

    ;Writing slopes
    if keyword_set(short) then id = indgen(672) else id= indgen(rtr.n_slope)
    err = write_same_dsp(0, rtr_map.switch_SCSlopeVector, vect[id], /CHECK, /SWITCH)    
    if err ne adsec_error.ok then return, err

    err = write_same_dsp(0, rtr_map.switch_SCStartRTR, [wfsc,pbs], /SWITCH, /CHECK)
    if err ne adsec_error.ok then return, err

    err = write_same_dsp(0, rtr_map.switch_SCStartRTR+2, dummy, /SWITCH)    
    if err ne adsec_error.ok then return, err
    ;wait, 0.01

    ;SWITCH Bcu automations do the rest....
    
    t0 = systime(/SEC)
    if keyword_set(TEST) then begin
        err = test_skip_frame(bufskip, NOVERB=noverb, SKIPFLAG=skipflag)
        if err ne adsec_error.ok then return, err
        if skipflag then begin
            message, "Error: bad thresholds settings or FastLink problems. Returning.", CONT=(sc.debug eq 0B)
            return, adsec_error.IDL_SAFE_SKIP_ACTIVE
        endif
    endif
    t1 = systime(/SEC) - t0
    
    if n_elements(sleep_time) then wait, (sleep_time - t1 ) > 0
    
    return, adsec_error.ok

End
