;+
; hex_delta_comm = tilt2hex(onsky_tiltx, onsky_tilty)
;
; returns the 6-element vector of differential command for the hexapod i
; to produce the given on-sky tilt in x and y. The tilt values are
; given in on-sky arcsec. The coma is constrained to be 0.0.
; Formula used in this represent a linear approximation of tilt wrt
; hexapod angle and displacements.
;
; WARN! the current routines consider the otical configuration of the i
; SOLAR TOWER, not the telescope.
;
; EXAMPLE
; apply 1 on-sky arcsec using the hexapod and keeping zero coma
;   err = hexmoveby(tilt2hex(1.0, 0.0))
; 
; MODIFICATION HISTORY
; 23 Sep 2009
;   written by A. Riccardi
;-
function tilt2hex, tx, ty

dty_dy_c0 = -0.039628205128
dty_da_c0 =  0.204431216931
dcy_dy_t0 =  0.002036956067
dcy_da_t0 = -0.019527978339
df_dz = 0.051300000000

f = 0.0
cx = 0.0
cy = 0.0

dx = tx/dty_dy_c0+cx/dcy_dy_t0
dy = ty/dty_dy_c0+cy/dcy_dy_t0
dz = f/df_dz
da = ty/dty_da_c0+cy/dcy_da_t0
db = tx/dty_da_c0+cx/dcy_da_t0
dc = 0.0

return, [dx, dy, dz, da, db, dc] 
end

