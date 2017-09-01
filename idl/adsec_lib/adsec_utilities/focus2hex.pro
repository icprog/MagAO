;+
; hex_delta_comm = focus2hex(f)
;
; returns the 6-element vector of differential command for the hexapod
; to produce the given focus WFE. The focus values are
; given um rms WFE. The focus is defined as Noll-Zernike polynomial..
; Formula used in this represent a linear approximation of focus wrt
; hexapod z displacements.
;
; in case the LONGITUDINAL keyword is set, the defocus (in um) passed as 
; argument is considered as longitudinal displacement of focal point at
; the  F/15 focus
;
; WARN! the current routines consider the otical configuration of the i
; SOLAR TOWER, not the telescope.
;
; EXAMPLE
; apply 1 um rms WFE focus using the hexapod
;   err = hexmoveby(focus2hex(1.0))
; 
; MODIFICATION HISTORY
; 23 Sep 2009
;   written by A. Riccardi
;-
function focus2hex, f, LONGITUDINAL=longitudinal

fin = f
dty_dy_c0 = -0.039628205128
dty_da_c0 =  0.204431216931
dcy_dy_t0 =  0.002036956067
dcy_da_t0 = -0.019527978339
df_dz = 0.051300000000

tx = 0.0
ty = 0.0
cx = 0.0
cy = 0.0
if keyword_set(longitudinal) then begin
    f = fin/320.0
endif else begin
    f = fin
endelse

dx = tx/dty_dy_c0+cx/dcy_dy_t0
dy = ty/dty_dy_c0+cy/dcy_dy_t0
dz = f/df_dz
da = ty/dty_da_c0+cy/dcy_da_t0
db = tx/dty_da_c0+cx/dcy_da_t0
dc = 0.0

return, [dx, dy, dz, da, db, dc] 
end

