; $Id: get_gain.pro,v 1.6 2007/02/07 16:57:57 marco Exp $
;
;+
; GET_GAIN
;
;   err = get_gain(act_list, gain [,/SPEED_LOOP_GAIN])
;
; HISTORY
;   27 May 2004, A. Riccardi
;   long data type (1L) used in read_seq_ch for LBT compatibility
;   07 Feb 2007 MX
;       LUT of variable deleted and name refurbished.
;-
function get_gain, act_list, gain, SPEED_LOOP_GAIN=speed_loop_gain

@adsec_common

if keyword_set(speed_loop_gain) then begin
    addr = dsp_map.speed_loop_gain
endif else begin
    addr = dsp_map.err_loop_gain
endelse

err = read_seq_ch(act_list, addr, 1L, gain)
return, err

end
