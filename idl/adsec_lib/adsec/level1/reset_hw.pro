;$Id: reset_hw.pro,v 1.4 2009/05/22 16:36:43 labot Exp $$
; NAME:
;   RESET_HW
; PURPOSE:
;   High level function to reset the hardware components
; USAGE:
;   err = reset_hw(/SWITCH, /BCU)
; INPUT:
;   None
; OUTPUT:
;   Error code.
; KEYWORDS:
;   SWITCHB: reset only the Switch BCU
;   BCU: reset only the Crate BCU
; HISTORY:
;   Created by Marco Xompero
;   on 13 Feb 2007
;   20 april 2007 Daniela Zanotti(DZ)
;   Substituted the multi calls to reset_device with only one call in multi mode. 
;-

Function reset_hw, SWITCHB=is_switch, BCU=is_bcu

    @adsec_common
    if ~keyword_set(is_switch) and ~keyword_set(is_bcu) then begin
        is_switch = 1
        is_bcu = 1
    endif
    if keyword_set(is_switch) then begin
        print, "RESET SWITCH BCU"
        err = reset_devices(0,0, /SWITCH, /ALLC, /MULTI, TIMEOUT=15)
        if err ne adsec_error.ok then return, err
    endif
    if keyword_set(is_bcu) then begin
;        for i=0, adsec.n_bcu-1 do begin
;            err = reset_devices(i,i,/BCU, /ALLC)
;            if err ne adsec_error.ok then return, err
;        endfor
        print, "RESET CRATE BCUs"
        for i=0, adsec.n_bcu-1 do begin
            err = reset_devices(i,i,/BCU, /ALLC, /MULTI, TIMEOUT = 15)
            if err ne adsec_error.ok then return, err
        endfor
 
    endif
   
    return, adsec_error.ok


End
