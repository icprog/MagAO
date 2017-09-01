;+
;   NAME:
;    SET_NENDOF
;
;   PURPOSE:
;    INITIALIZES ALL THE NENDOF TO 1UL.
;
;   USAGE:
;    err = set_nendof()
;
;   INPUT:
;    None.
;
;   OUTPUT:
;    Error code.
;
;   KEYWORD:
;    None.
;
;   NOTE:
;    None.
;
; HISTORY
;   14 Feb 2005 Daniela Zanotti (DZ)
;   20 May 2005 Marco Xompero (MX)
;      Dsp_map updated to rtr_map.
;   25 Jan 2007 MX. Routine dismissed. It will be removed.
;-
function set_nendof

@adsec_common

; Setting nendof_rtr
err = write_same_dsp(sc.all_dsp, rtr_map.nendof_rtr,1ul,/check )
if err ne adsec_error.ok then return, err

; Setting nendof_mm
err = write_same_dsp(sc.all_dsp, rtr_map.nendof_mm,1ul,/check )
if err ne adsec_error.ok then return, err

; Setting nendof_ff
err = write_same_dsp(sc.all_dsp, rtr_map.nendof_ff,1ul,/check )
if err ne adsec_error.ok then return, err

return,adsec_error.ok

end
