; $Id: start_ff.pro,v 1.10 2007/02/23 15:31:01 labot Exp $
;+
; START_FF
;
; err = start_ff([trig])
;
; trig:  int or long scalar. FF is started after trig-1 loop steps.
;        trig is 1UL as default (it means: start FF now).
;
; HISTORY
; 27 May 2004, A. Riccardi (AR)
;   long type data written for LBT compatibility
;
; 12 Aug 2004, AR
;   optional input added: delay of the start_ff trigger.
; 02 Nov 2004, MX
;   Adsec.all changed in sc.all.
; 15 Feb 2005, D.Zanotti(DZ)
;   Added setting of nendof_ff.
;   20 May 2005: Marco Xompero (MX)
;      Dsp_map updated to rtr_map.
;   07 Feb 2007 MX
;       LUT of variable deleted and name refurbished.
;       Polling on nendof_ff removed.
;   23 Feb 2007 MX
;       Fix manual application of ff using start_ff and update_ff
;-

function start_ff, delay

@adsec_common

; Setting nendof_ff
;err = write_same_dsp(sc.all_dsp, rtr_map.nendof_ff,1UL,/check )
;if err ne adsec_error.ok then return, err

n_el = n_elements(delay)
if n_el eq 0 then begin
    delay = 1UL
endif else begin
    if test_type(delay, /ULONG, /LONG, /UINT, /INT) then begin
        message, "Delay must be integer.", CONT=sc.debug eq 0B
        return, adsec_error.input_type
    endif
    if n_el ne 1 then begin
        message, "Delay must be a scalar.", CONT=sc.debug eq 0B
        return, adsec_error.input_type
    endif
    if delay[0] le 0 then begin
        message, "Delay must be strictly positive.", CONT=sc.debug eq 0B
        return, adsec_error.input_type
    endif
endelse

err = write_same_dsp(sc.all_dsp, rtr_map.start_ff, 1l)
if err ne adsec_error.ok then return, err
wait, 0.01
;err = write_same_dsp(sc.all_dsp, rtr_map.start_dl, 1l)
;if err ne adsec_error.ok then return, err
err = write_same_dsp(sc.all_dsp, rtr_map.update_ff, ulong(delay[0]))

return, err

end
