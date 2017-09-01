;+
; hex_delta_comm = coma2hex(comax_wfe_rms, comay_wfe_rms)
;
; returns the 6-element vector of differential command for the hexapod i
; to produce the given coma coefficients in x and y. The coma coeffs arei
; given in um rms of WFE. The coma definition follows the Noll-Zernike
; polinomials.
; Formula used in this represent a linear approximation of coma wrt
; hexapod angle and displacements.
;
; WARN! the current routines consider the otical configuration of the i
; SOLAR TOWER, not the telescope.
;
; EXAMPLE
; apply 1um rms of comax WFE using the exapod
;   err = hexmoveby(coma2hex(1.0, 0.0))
; 
; MODIFICATION HISTORY
; 23 Sep 2009
;   written by A. Riccardi
;-
function coma2hex, cx, cy

dty_dy_c0 = -0.039628205128
dty_da_c0 =  0.204431216931
dcy_dy_t0 =  0.002036956067
dcy_da_t0 = -0.019527978339
df_dz = 0.051300000000

f = 0.0
tx = 0.0
ty = 0.0

dx = tx/dty_dy_c0+cx/dcy_dy_t0
dy = ty/dty_dy_c0+cy/dcy_dy_t0
dz = f/df_dz
da = ty/dty_da_c0+cy/dcy_da_t0
db = tx/dty_da_c0+cx/dcy_da_t0
dc = 0.0

return, [dx, dy, dz, da, db, dc] 
end

