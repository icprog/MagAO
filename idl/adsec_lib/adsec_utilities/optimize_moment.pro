; $Id: optimize_moment.pro,v 1.3 2003/06/10 18:32:24 riccardi Exp $

function optimize_moment, f, act_list

; calculates the force pattern with minimal force variance setting
; a constant total force and total momentum as constrains.
; It works even if the forces are applied by a subset of the acuators.

@adsec_common 

if n_elements(act_list) eq 0 then act_list=indgen(adsec.n_actuators)
n_act = n_elements(act_list)
n_f = n_elements(f)

if n_act ne n_f then begin
	message, "f and act_list must have the same # of elements."
	return, 0
endif

x = adsec.act_coordinates[0,act_list]
y = adsec.act_coordinates[1,act_list]
x2_tot = total(x^2)
y2_tot = total(y^2)
x_tot = total(x)
y_tot = total(y)
xy_tot = total(x*y)


f_ave = (moment(f, SDEV=f_sd))[0]					; average force
m_tot = [total( y*f), total(-x*f)]					; total moment

; solve for Lagrange multipliers
A = [[n_act,  y_tot,  x_tot], $
     [y_tot, y2_tot, xy_tot], $
     [x_tot, xy_tot, x2_tot]]
b = [0.0, 2.0*(y_tot*f_ave-m_tot[0]), 2.0*(x_tot*f_ave+m_tot[1])]

svdc, A, w, u, v
idx = where(w le max(w)*1e-5, count)
if count ne 0 then w[idx] = 0.0
l = svsol(u, w, v, b) ; Lagrange multipliers

f_bal = f_ave - 0.5*(l[0] + l[1]*y + l[2]*x)		; balanced force set

return, reform(f_bal)

end
