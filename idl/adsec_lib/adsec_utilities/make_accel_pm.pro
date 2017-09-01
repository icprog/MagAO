Function make_accel_pm

    x = [ -155.9,        104.3,    -199,           242.9,      252.3,         -355.4    ]*1d-3
    y = [  319.8,        340.2,    -295.3,         -260.5,    -145.7,         -18.9     ]*1d-3 
    z = [ 128.137,       115.3234, 128.137,        115.3234,  138.4035,       115.3234  ]*1d-3
    p = atan(y,x)
    q = atan(x,z+1.974247)
    q[[0,2,4]] = !dpi/2   ;annullo le proiezioni in z degli accelerometri orizzontali
    p[[0,2]] += !dpi/2   ;oriento correttamente gli accelerometri orizzontali
    p[4] -= !dpi/2   ;l'ultimo orizzontale e' messo storto


    x0 = sin(q)*cos(p)
    y0 = sin(q)*sin(p) 
    z0 = cos(q)

    M2=dblarr(6,6)
    for i = 0, 5 do begin
        M2[0,i]=  x0[i]                        ;dxv OK
        M2[1,i]=  y0[i]                        ;dyv OK
        M2[2,i]=  z0[i]                        ;dzv OK
        M2[3,i]=  y[i]*z0[i]-z[i]*y0[i]        ;tx  OK
        M2[4,i]=  -x[i]*z0[i]+z[i]*x0[i]       ;ty  OK
        M2[5,i]=  x[i]*y0[i]-y[i]*x0[i]        ;tz  OK
    endfor
    PM=invert(M2)

    return, PM

End
;vettori di test (validi solo per z nullo)
;PISTONE
;idv = [1,3,5]
;ido = [0,2]
;ido_r = 4
;vv = fltarr(1, 6)
;vv[idv] = cos(q[idv])

;TILT attorno a Y
;angle = !pi/3
;vv1 = fltarr(1, 6)
;vv1[idv] = - cos(q[idv]) * x[idv] * tan(angle) 
;vv1[ido] = 0
;vv1[ido_r] = 0

;TILT attorno a X
;angle = !pi/3
;vv2 = fltarr(1, 6)
;vv2[idv] = cos(q[idv]) * y[idv] * tan(angle) 
;vv2[ido] = 0
;vv2[ido_r] = 0

;TILT attorno a X
;angle = !pi/3
;vv4 = fltarr(1, 6)
;vv4[idv] = 0
;vv4[ido] = angle*sqrt(x[ido]^2+y[ido]^2) ;*cos(p[ido]-atan(x[ido],y[ido]))
;vv4[ido_r] = -angle*sqrt(x[ido_r]^2+y[ido_r]^2); *cos(p[ido_r]-atan(x[ido],y[ido]))

;TRASLAZIONE lungo X
;vv3 = fltarr(1, 6)
;ds = 1.
;vv3[idv] = ds * cos(p[idv])*sin(q[idv])
;vv3[ido] = ds * cos(p[ido])*sin(q[ido])
;vv3[ido_r] =  ds * cos(p[ido_r])*sin(q[ido_r])




