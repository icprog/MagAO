;$Id: clear_rtr.pro,v 1.3 2007/06/05 08:48:31 labot Exp $
;+
;   NAME:
;    CLEAR_RTR
;
;   PURPOSE:
;    Clearing all the gains and matrix of the reconstructor.
;
;   INPUT:
;    None:
;  
;   OUTPUT:
;    Error code.
;
;   KEYWORDS:
;    B0:        first matrix of the filter input signal weights of the recontructor.
;    B_DELAY:   matrix set of the weights for the recontructor filter input.
;    A_DELAY:   matrix set of the weights for the recontructor filter output.
;    GAIN_G:    general gain of the reconstructor filter.
;
;   NOTE:
;    It will be reset all first and second memory block for each parameter, if available.
;
;   HISTORY:
;    Written by Daniela Zanotti(DZ) and Marco Xompero(MX) on 9 Feb 2005
;    zanotti@arcetri.astro.it
;    20 May 2005: Marco Xompero (MX)
;       Dsp_map updated to rtr_map.
;    05 Jun 2007, MX
;       Fixed bug in A delay matrix cleaning.
;-


Function clear_rtr, B0=b0, B_DELAY=b_delay, A_DELAY=a_delay, GAIN_G=gain_g

@adsec_common
err = intarr(6)
if not keyword_set(B0) and not keyword_set(B_DELAY) $ 
    and not keyword_set(A_DELAY) and not keyword_set(GAIN_G) then begin $
    b0=1
    b_delay=1
    a_delay=1
    gain_g=1
endif

if keyword_set(B0) then begin
    err[0] = set_b0_matrix(fltarr(rtr.n_slope, adsec.n_actuators))
    if err[0] ne adsec_error.ok then begin 
        message, 'B0 matrix first block not cleared', CONT=(sc.debug eq 0)
        return, err[0]
    endif
    err[1] = set_b0_matrix(fltarr(rtr.n_slope, adsec.n_actuators), /BLOCK)
    if err[1] ne adsec_error.ok then begin 
        message, 'B0 matrix second block not cleared', CONT=(sc.debug eq 0)
        return, err[1]
    endif
endif

if keyword_set(B_DELAY) then begin
    err[2] = set_b_delay_matrix(fltarr(rtr.n_slope, adsec.n_actuators, rtr.n_slope_delay))
    if err[2] ne adsec_error.ok then begin 
        message, 'B delay matrix first block not cleared', CONT=(sc.debug eq 0)
        return, err[2]
    endif
    err[3] = set_b_delay_matrix(fltarr(rtr.n_slope, adsec.n_actuators, rtr.n_slope_delay), /BLOCK)
    if err[3] ne adsec_error.ok then begin 
        message, 'B delay matrix second block not cleared', CONT=(sc.debug eq 0)
        return, err[3]
    endif
endif

if keyword_set(A_DELAY) then begin
    err[4] = set_a_delay_matrix(fltarr(adsec.n_actuators, adsec.n_actuators, rtr.n_modes_delay))
    if err[4] ne adsec_error.ok then begin 
        message, 'A delay matrix not cleared', CONT=(sc.debug eq 0)
        return, err[4]
    endif
endif

if keyword_set(GAIN_G) then begin
    err[5] = set_g_gain(/RESET)
    if err[5] ne adsec_error.ok then begin 
        message, 'G gain not cleared', CONT=(sc.debug eq 0)
        return, err[5]
    endif
endif

return, adsec_error.ok

End
