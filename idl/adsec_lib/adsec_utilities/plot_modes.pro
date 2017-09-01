; $Id: plot_modes.pro,v 1.4 2003/11/18 16:15:24 riccardi Exp $

pro plot_modes, v, idx, SV=w

nm = n_elements(v[*,0])
if n_elements(idx) eq 0 then idx=indgen(nm)
px = round(sqrt(nm))
py = nm / px
if nm mod px ne 0 then py=py+1
!p.multi = [0,px,py]

if n_elements(w) eq 0 then tit=replicate("",nm) else tit="SV="+string(w,FORMAT="(E11.3)")
for m=0,nm-1 do display, reform(v[m,idx]), idx, /NO_NU, /SMOOTH, $
                          OUT_VAL=0.0, XSTY=4, YSTY=4, XMARGIN=[0,0], YMARGIN=[0,2], $
                          TITLE=tit[m]

!p.multi=0

end
