; $Id: start_control.pro,v 1.6 2007/02/07 16:57:57 marco Exp $
;+
; START_CONTROL
;
; err = start_control(act_list)
;
; HISTORY
;   27 May 2004, A. Riccardi
;       long type data written for LBT compatibility
;       lon2float conversion no longer needed
;   07 Feb 2007 MX
;       LUT of variable deleted and name refurbished.
;
;-

function start_control, act_list

@adsec_common

err = write_same_ch(act_list, dsp_map.control_enable, 1L)

return, err
end
