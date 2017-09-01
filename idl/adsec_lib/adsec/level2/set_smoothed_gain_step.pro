; $Id: set_smoothed_gain_step.pro,v 1.3 2007/02/07 16:57:57 marco Exp $
;+
; SET_SMOOTHEED_GAIN_STEP
;
; err = set_smoothed_gain_step(act_list, step_gain [,/SPEED])
;
; HISTORY
; 14 July written
; 27 Aug 2004: fixed handling of speed keyword.
;   07 Feb 2007 MX
;       LUT of variable deleted and name refurbished.
;-
function set_smoothed_gain_step, act_list, step_gain, SPEED=speed

@adsec_common

if keyword_set(speed) then $
    addr = dsp_map.speed_smoothed_step $
else $
    addr = dsp_map.err_smoothed_step


if n_elements(step_gain) eq 1 then begin
    err = write_same_ch(act_list, addr, step_gain[0], /CHECK)
endif else begin
    err = write_seq_ch(act_list, addr, step_gain, /CHECK)
endelse

return, err

end
