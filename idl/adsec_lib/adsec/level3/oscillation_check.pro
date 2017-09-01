;+
;   NAME:
;    OSCILLATION_CHECK
;
;   PURPOSE:
;    Check for the safety of the shell testing low freq oscillation and using fast diagnostic data
;   read directly from SDRAM.
;
;   USAGE:
;    err = OSCILLATION_CHECK(status, hist, NOFAST=nofast)
;
;   INPUT:
;    hist: number of frames to use for the checks.
;
;   OUTPUT:
;    err :      Error code.
;    status:    oscillation status of the mirror (1 the shell is oscillating, else 0)
;   
;   KEYWORDS:
;    NOFAST :   keyword passed to load_diag_data procedure to inibit the check on fast-diagnostic status.
;
;   HISTORY:
;    Created by Marco Xompero on 11 Dic 2005.
;    marco@arcetri.astro.it
;   7 july 2006 Daniela Zanotti(DZ)
;    Changed stdev,obsolete commad , with stddev.          
;-



Function oscillation_check, status, hist, NOFAST = nofast

    @adsec_common
    if n_elements(hist) eq 0 then hist = 30;

    ;read the current fast diagnostic data with 30 frames
    err = load_diag_data(hist, data, /READFC, NOFAST=nofast)
    if err ne 0 then return, err

    ;use the dist parameter to check for the oscillations
    shell_pos = fltarr(adsec.n_actuators)
    for i=0, adsec.n_actuators-1 do shell_pos[i] = stddev(data.dist_average[i,*])

    ;test the oscillation state
    if max(shell_pos) gt 100e-9 then status=1 else status=0

    status=0 ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;; DA SISTEMARE ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    return, adsec_error.ok

    
End
