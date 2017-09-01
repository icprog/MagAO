;
;+
; NAME:
;  START_RTR
;
; PURPOSE:
;   Sets nendof_rtr to 1, and sets start_rtr.
;
; USAGE:
;   err = start_rtr()
;
; HISTORY
;   Feb 2005 D.ZANOTTI(DZ)
;   20 May 2005: Marco Xompero (MX)
;      Dsp_map updated to rtr_map.
;   07 Feb 2007 MX
;       LUT of variable deleted and name refurbished.
;       Polling on nendof_ff removed.
;-

function start_rtr

@adsec_common

; Setting nendof_rtr
; err = write_same_dsp(sc.all_dsp, rtr_map.nendof_rtr,1UL,/check )
; if err ne adsec_error.ok then return, err

err = write_same_dsp(sc.all_dsp, rtr_map.start_rtr, 1ul)

return, err

end
