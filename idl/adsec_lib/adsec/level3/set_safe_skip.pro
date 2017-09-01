;$Id: set_safe_skip.pro,v 1.7 2009/11/24 16:57:09 marco Exp $
;+
;   NAME:
;    SET_SAFE_SKIP
;
;   PURPOSE:
;    Set right thresholds in order to mantain safe the thin shell in the calibration procedures.
;
;   USAGE:
;    err = set_safe_skip()
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
;    Created by Marco Xompero (MX) on 4 Jun 2007
;    marco@arcetri.astro.it
;
;-

Function set_safe_skip, DELTAMODE=deltamode, MAXABSMODE=maxabsmode          $
                      , DELTACMD=deltacmd, MAXCMD=maxcmd, MINCMD=mincmd     $
                      , MAXFORCE=maxforce, MINFORCE=minforce                $
                      , ACTLIST=actlist, RESET=reset, MODLIST=modlist

    @adsec_common
    if n_elements(actlist) eq 0 then actlist = indgen(adsec.n_actuators)
    if n_elements(modlist) eq 0 then modlist = indgen(adsec.n_actuators)
    if n_elements(deltamode) eq 0 then deltamode = 10e-6*sqrt(n_elements(adsec.act_w_cl))
    if n_elements(maxabsmode) eq 0 then maxabsmode = 5e-6*sqrt(n_elements(adsec.act_w_cl))
    if n_elements(deltacmd) eq 0 then deltacmd = 10e-6
    if n_elements(maxcmd) eq 0 then maxcmd = 120e-6
    if n_elements(mincmd) eq 0 then mincmd = 1e-6;adsec.d0[actlist]/2.*adsec.capsens_vref/adsec.wave_max_ptv+0.1e-6;36e-6;36e-6
    if n_elements(maxforce) eq 0 then maxforce = 0.70;adsec.pos_sat_force * 0.98; introdurre adsec.pos_sat_ff_force 
    if n_elements(minforce) eq 0 then minforce = -0.70;adsec.neg_sat_force * 0.98; introdurre adsec.neg_sat_ff_force 
    dummy_pos_val = 1e6
    dummy_neg_val = -1e6
    
    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

    dummy = replicate(dummy_pos_val, adsec.n_actuators)
    if ~keyword_set(reset) then dummy[modlist] = deltamode
    err = set_skip_par(dummy, /MODE, /DELTA)
    if err ne adsec_error.ok then return, err

    dummy = replicate(dummy_pos_val, adsec.n_actuators)
     if ~keyword_set(reset) then dummy[modlist] = maxabsmode
    err = set_skip_par(dummy, /MODE, /MAX)
    if err ne adsec_error.ok then return, err

    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

    dummy = replicate(dummy_neg_val, adsec.n_actuators)
     if ~keyword_set(reset) then dummy[actlist] = mincmd
    err = set_skip_par(dummy, /CMD, /MIN)
    if err ne adsec_error.ok then return, err

    dummy = replicate(dummy_pos_val, adsec.n_actuators)
     if ~keyword_set(reset) then dummy[actlist] = deltacmd
    err = set_skip_par(dummy, /CMD, /DELTA)
    if err ne adsec_error.ok then return, err

    dummy = replicate(dummy_pos_val, adsec.n_actuators)
     if ~keyword_set(reset) then dummy[actlist] = maxcmd
    err = set_skip_par(dummy, /CMD, /MAX)
    if err ne adsec_error.ok then return, err

    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

    dummy = replicate(dummy_neg_val, adsec.n_actuators)
     if ~keyword_set(reset) then dummy[actlist] = minforce
    err = set_skip_par(dummy, /FORCE, /MIN)
    if err ne adsec_error.ok then return, err

    dummy = replicate(dummy_pos_val, adsec.n_actuators)
    if ~keyword_set(reset) then dummy[actlist] = maxforce
    err = set_skip_par(dummy, /FORCE, /MAX)
    if err ne adsec_error.ok then return, err

    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

    return, adsec_error.ok

End
