; $Id: set_shape_preshaper.pro,v 1.5 2009/03/02 13:06:11 labot Exp $
;+
; SET_SHAPE_PRESHAPER
;
; err = set_shape_preshaper(act_list, [,/FF])
;
; HISTORY
;   Written by Armando Riccardi on 2004 (AR)
;   riccardi@arcetri.astro.it
;   01/12/2004 Marco Xompero (Mx)
;       Added keyword NOSHAPE for an active preshaper always set to 1.
;   07 Feb 2007 MX
;       LUT of variable deleted and name refurbished.
;-
function set_shape_preshaper, dsp_list, FF=ff, NOSHAPE=noshape, NEW_SHAPE = new_shape

@adsec_common

if keyword_set(ff) then $
    addr = dsp_map.preshaper_curr_buffer $
else $
    addr = dsp_map.preshaper_cmd_buffer

n = adsec.preshaper_len
shape_preshaper = -(cos(!PI*findgen(n)/(n-1))-1)/2
if keyword_set(noshape) then shape_preshaper = fltarr(n)+1 
if keyword_set(new_shape) then shape_preshaper = new_shape
if shape_preshaper[n-1]-1.0 ne 0 then message, "ERROR ON PRESHAPER SHAPE"

if keyword_set(new_shape) then begin

    err = get_preshaper(dsp_list, old_len, FF=ff)
    if err ne adsec_error.ok then return, err

    err = set_preshaper(dsp_list, dsp_const.fastest_preshaper_step)
    if err ne adsec_error.ok then return, err

    err = write_same_dsp(dsp_list, addr, shape_preshaper[0:n-2], /CHECK)
    if err ne adsec_error.ok then return, err

    err = set_preshaper(dsp_list, old_len)
    if err ne adsec_error.ok then return, err

endif else begin

    err = write_same_dsp(dsp_list, addr, shape_preshaper, /CHECK)
    if err ne adsec_error.ok then return, err

endelse

return, err

end
