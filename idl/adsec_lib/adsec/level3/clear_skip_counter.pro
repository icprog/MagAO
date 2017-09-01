;$Id: clear_skip_counter.pro,v 1.1 2007/06/05 08:47:43 labot Exp $
;+
;   NAME:
;    CLEAR_SKIP_COUNTER
;
;   PURPOSE:
;    Cleaning error counters in the SWITCH BCU memory map.
;   
;   USAGE:
;    err = clear_skip_counter()
;   
;   INPUT:
;    None.
;
;   OUTPUT:
;    Error Code
;
;   KEYWORDS:
;    None.
;
;   HISTORY:
;    Created on 05 Jun 2007 by Marco Xompero (MX)
;    marco@arcetri.astro.it 
;
;-

Function clear_skip_counter, ALL=all

    @adsec_common
    err = write_same_dsp(0, switch_map.safeSkipFrameCnt,0l,/SWITCH)
    if err ne adsec_error.ok then return, err
    err = write_same_dsp(0, switch_map.pendingSkipFrameCnt,0l,/SWITCH)
    if err ne adsec_error.ok then return, err
    err = write_same_dsp(0, switch_map.numFLTimeout,0l,/SWITCH)
    if err ne adsec_error.ok then return, err
    err = write_same_dsp(0, switch_map.numFLCRCErr,0l,/SWITCH)
    if err ne adsec_error.ok then return, err

    return, adsec_error.ok
    
End
