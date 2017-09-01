; $Id: stop_control.pro,v 1.7 2007/03/07 12:16:00 labot Exp $
;+
; STOP_CONTROL
;
; err = stop_control(act_list)
;
; HISTORY
;   27 May 2004, A. Riccardi
;       written long type data for LBT compatibility
;   07 Feb 2007 MX
;       LUT of variable deleted and name refurbished.
;       Polling on nendof_ff removed.
;-
function stop_control, act_list

    @adsec_common

    err = write_same_ch(act_list, dsp_map.control_enable, 0L)

    return, err
end
