;+
; START_MM
;
; PURPOSE:
;  Sets nendof_mm to 1, and sets start_mm.
;
; err = start_mm()
;
; HISTORY
; Feb 2005 D.Zanotti(DZ)
;   20 May 2005: Marco Xompero (MX)
;      Dsp_map updated to rtr_map.
;   07 Feb 2007 MX
;       LUT of variable deleted and name refurbished.
;       Polling on nendof_mm removed.
;-

function start_mm

@adsec_common

; Setting nendof_mm
; err = write_same_dsp(sc.all_dsp, rtr_map.nendof_mm,1UL,/check )
; if err ne adsec_error.ok then return, err

err = write_same_dsp(sc.all_dsp, rtr_map.start_mm, 1ul)

return, err

end
