; $Id: check_control.pro,v 1.6 2007/02/07 16:57:56 marco Exp $

function check_control, act_list, ctrl_status
;+
; act vector of actuators
; return the byte vector ctrl_status with the status of the ctrl
; 1B=enabled, 0B=disbled, Not valid values or undefined if an error occurred
;
; HISTORY
; 27 May 2004, A. Riccardi
;   used long/ulong data type when requested for LBT compatibility
;   07 Feb 2007 Marco Xompero
;       LUT of variable deleted and name refurbished.
;-
@adsec_common

err = read_seq_ch(act_list, dsp_map.control_enable, 1L, buf, /ULONG)
if err ne adsec_error.ok then return, err

ctrl_status = buf eq 1

return, adsec_error.ok

end
