
function get_u_zern, x, y, idx_list

;on_error, 2

    n2 = n_elements(idx_list)
    m = n_elements(x)

    u = dblarr(n2, m, /nozero)
    rho = sqrt(x^2+y^2)
    theta = atan(y, x)
    for i=0,n2-1 do begin
        u(i, *) = zern_jpolar(idx_list[i], rho, theta)
    endfor

    return,u

end




Function offload_hexapod, SEND=send, HXPROJ=hxproj


    ;    get = getdiagnvalut('FASTDGN00', 'OffloadZernCoeffs', -1, -1, buf)
    ;    dummy = transpose(buf.last)
    
            dummy = transpose([-5.0501521e-08,  -1.3201397e-07,   1.9739385e-07,  -2.5652431e-07,  -1.8772109e-07,  -7.4998995e-08,   7.7468711e-08,   5.4641568e-09   $
         , 1.1972786e-08,   8.3161879e-09,  -9.7518675e-09,  -7.1809028e-09,   3.6872522e-08,  -1.7501191e-08,   4.8302092e-09,   3.9912986e-09   $
         , -5.1039103e-09,  -1.7164016e-08,  -9.8812459e-09,  -8.4204092e-10,   6.7887028e-09,  -8.4914016e-08])
    
    if n_elements(hxproj) eq 0 then begin
        nzern = 8
        idz = [1, 2, 3, 6, 7]
        zc = transpose(dummy[idz])
    ;    amp = fltarr(5, 5)
    ;    amp = [-3, 3, 10, 10, 10]  ;ARCSEC AND MICRON
        restore, '/home/labot/idl/adsec672a/data/hexapod_opt_calib_w_4d.sav', /ver
        sopd = float(size(opd0, /dim))
        
        ;b_o = read_opd(path+base, type='4d', bad= b_m)    
        n_o = fltarr(sopd[0], sopd[1], 10)
        n_m = fltarr(sopd[0], sopd[1], 10)
        nameso = string(indgen(10), format='("opd",I1.1)')
        namesm = string(indgen(10), format='("mask",I1.1)')
        amp = fltarr(5,5)
        amp[0,0] = 2*dxy 
        amp[1,1] = 2*dxy 
        amp[2,3] = 2*dab 
        amp[3,4] = 2*dab 
        amp[4,2] = 2*dz  


        for i=0, n_elements(namesm)-1 do begin
            
            dummy = execute('n_o[*,*,i] = '+nameso[i])
            dummy = execute('n_m[*,*,i] = '+namesm[i])

        endfor


        idm = where(total(n_m, 3) eq 10, cc)
        opdmat = fltarr(n_elements(namesm)/2., cc)
        for i=0, n_elements(namesm)/2.-1 do begin
            dummy = reform(n_o[*,*,2*i])
            dummy2 = reform(n_o[*,*,2*i+1])
            opdmat[i,*] = dummy[idm]-dummy2[idm]
        endfor
        

    ;    val = pupilfit(mask0, guess=[500,500, 200, 0.01], /circle, /disp)
        val = [497.010, 520.260, 461.520, 0.0990666]
        xx = rebin((mk_vector(sopd[0], 0,sopd[0])-val[0])/val[2], sopd[0], sopd[1], /SAMP)
        yy = rebin((transpose(mk_vector(sopd[1], 0,sopd[1]))-val[1])/val[2], sopd[0], sopd[1], /SAMP)
        umat = call_function('get_u_zern', xx[idm], yy[idm], indgen(nzern)+1)
        uplus = invert(transpose(umat) ## umat) ## transpose(umat)
        dummy = uplus ## opdmat
        coeffs = dummy[*, idz] 

        proj = amp ## pseudo_invert(coeffs)
        hxproj = proj[*,[0,1,3,4,2]]
    endif
    hexmov = hxproj ## zc
    print, hexmov
    if keyword_set(SEND) then begin
        print, hexmoveby(hexmov)
    end
    stop

end
