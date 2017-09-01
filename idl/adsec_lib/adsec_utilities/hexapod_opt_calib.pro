function poll_hexapod, sec
    t0 = systime(/sec)
    for i=0,10 do begin
        if i eq 0 then begin
            err = hexgetpos(bpos)
    ;        if err ne adsec_error.ok then message, "ERROR!!"
        endif
        wait, 2.
        err = hexgetpos(cpos)
    ;    if err ne adsec_error.ok then message, "ERROR!!"
        if total(cpos - bpos) ne 0 then bpos=cpos else break
    endfor
    sec = systime(/sec)-t0
    print, "Hexapod in position after "+string(sec, format='(I3.3)')+" seconds."
        return, 0

end

; -236.29755       959.92871       1967.0145      -28.272390       127.99032
function hexapod_opt_calib, GETPOS=getpos, AVERAGE=average

    
@adsec_common

    err = idl_4d_init()
    if err ne adsec_error.ok then return, err

    if n_elements(average) eq 0 then average = 10
    
    if keyword_set(GETPOS) then err = hexgetpos(zero_hex_pos) else $
    zero_hex_pos = [-245.13167, 1073.7040, 1958.0206, -39.139843, 126.71857, 0.]
    zero_hex_pos[5] = 0

;    V = transpose([[0,400,0,-40,0, 0], [0,0,25,0,0,0], [400,0,0,0,40,0], [0,0,0,2,0,0],[0,0,0,0,2,0]])
    V = transpose([[0,0.400,0,-40,0, 0], [0,0,0.07,0,0,0], [0.400,0,0,0,40,0], [0,0,0,2,0,0],[0,0,0,0,2,0]])
    ;V = transpose([[0,100,0,-10,0, 0], [0,0,6,0,0,0], [100,0,0,0,10,0], [0,0,0,0.5,0,0],[0,0,0,0,0.5,0]])
    nm = n_elements(V[*,0])

    for kk=0, nm-1 do begin
        print, string(kk) + " 0"
        name_radix = "tmp0"
        err=  hexmoveto(zero_hex_pos)
        ;if err ne adsec_error.ok then message, "ERROR!!"

        err = poll_hexapod(sec)
        if average gt 1 then opd0=get_img4d(nmeas=average, out=mask0) $
        else err = get_m4d(name_radix, opd0, mask0)

        if kk eq 0 then begin
            sopd  = size(opd0, /dim)
            Vopd  = dblarr(sopd[0]*sopd[1], nm)
            Vmask = dblarr(sopd[0]*sopd[1], nm)
            mask = mask0
        endif

        print, string(kk) + " +1"
        name_radix = "tmp1"
        err = hexmoveto(zero_hex_pos + V[kk,*])
    ;    if err ne adsec_error.ok then message, "ERROR!!"
        err = poll_hexapod(sec)
        if average gt 1 then opd1=get_img4d(nmeas=average, out=mask1) $
        else err = get_m4d(name_radix, opd1, mask1)
    
        print, string(kk) + " -1"
        name_radix = "tmp2"
        err = hexmoveto(zero_hex_pos - V[kk,*])
    ;    if err ne adsec_error.ok then message, "ERROR!!"
        err = poll_hexapod(sec)
        if average gt 1 then opd2=get_img4d(nmeas=average, out=mask2) $
        else err = get_m4d(name_radix, opd2, mask2)
    
        print, string(kk) + " +0"
        name_radix = "tmp3"
        err = hexmoveto(zero_hex_pos)
    ;    if err ne adsec_error.ok then message, "ERROR!!"
        err = poll_hexapod(sec)
        if average gt 1 then opd3=get_img4d(nmeas=average, out=mask3) $
        else err = get_m4d(name_radix, opd3, mask3)
        
        Vopd[*, kk] = ((opd1-opd0)+(opd3-opd2))/2
        Vmask[*,kk] = mask1*mask2*mask3

        if average eq 1 then begin
            file_delete, '/towerdata/adsec_calib/4d/SINGLE_MEAS/hdf5/tmp0_0000.h5'
            file_delete, '/towerdata/adsec_calib/4d/SINGLE_MEAS/hdf5/tmp1_0000.h5'
            file_delete, '/towerdata/adsec_calib/4d/SINGLE_MEAS/hdf5/tmp2_0000.h5'
            file_delete, '/towerdata/adsec_calib/4d/SINGLE_MEAS/hdf5/tmp3_0000.h5'
        endif

    endfor
    err = hexmoveto(zero_hex_pos);was +V[kk,*]

    mask = reform(total(Vmask, 2) eq 5, sopd[0], sopd[1])

    pupil_xyre = pupilfit(mask, /circle, /disp, GUESS= [sopd[0]/2., sopd[1]/2., sopd[0]*0.8/2., 0.2])

    err= idl_4d_finalize()
    if err ne adsec_error.ok then message, "ERROR!!"

   xx = rebin(dindgen(sopd[0])-pupil_xyre[0], sopd[0], sopd[1])/pupil_xyre[2]
   yy = rebin(transpose(dindgen(sopd[1])-pupil_xyre[1]), sopd[0], sopd[1])/pupil_xyre[2]
   pupil = where(mask)
   nm = min(size(Vopd, /dim))
   nz=10
   coeff = dblarr(nm,nz)

   for kk=0, nm-1 do begin
       z = Vopd[*,kk]
       fit = surf_fit(xx[pupil], yy[pupil], z[pupil], indgen(nz)+1, /ZERN, coeff=tmp)
       coeff[kk, *] = tmp
    endfor
    M = coeff[*,[1,2,3,6,7]]
    Mplus = pseudo_invert(M, /double)
    tf_mat = V ## Mplus
    
    dir = meas_path('hexapod')
    if n_elements(average) eq 0 then average=1
    save, FILE=dir+"hexapod_opt_calib.sav", Vopd, Vmask, V, zero_hex_pos, pupil_xyre, opd0, sopd, mask, M, Mplus, tf_mat, average
    save, FILE=dir+"hexapod_tf_mat.sav",  tf_mat, pupil_xyre, xx, yy, nz, average
    print, dir
    return, adsec_error.ok
end
