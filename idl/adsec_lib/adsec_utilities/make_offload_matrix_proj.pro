function get_u_zern_matrix, x, y, idx_list
; x and y must be coordinates normalized to unit pupil radius
; idx_list: list of zernike index to compute (Noll numbering)

;on_error, 2

    n2 = n_elements(idx_list)
    m = n_elements(x)

    u = dblarr(n2, m, /nozero)
    rho = sqrt(x^2+y^2)
    theta = atan(y, x) ;flipping required to project on M1
    for i=0,n2-1 do begin
        u(i, *) = zern_jpolar(idx_list[i], rho, theta)
    endfor

    return,u

end

;+
; MAKE_OFFLOAD_MATRIX_PROJ
;
; make_offload_matrix_proj, imfile, NZERNIKE=nzern, CL_ACT=cl_act, UMAT=umat_out $
;                            , PMPOS=pm_out, PMFOR=pmfor_out, F2P=f2p_out $
;                            , Z2C=z2c
;
; imfile(input):  path of MMmatrix from 4D measurements.
;                 Ex. '/home/FLAO_data/phase_maps/MMmatrix_20090811_setg1.sav'
;                 The file must contain:
;                   MMMATRIX
;                   DPIX
;                   IDX_MASK
;                   XX
;                   YY
;                   MM2C
;
; NZERNIKE: (optional input) number of Zernike modes to manage (index from 1 to nzern)
;           Ordering following Noll. Default value is 22.
; PMPOS: (output) projection matrix transforming actuator commands to WF (=2*surface)
;        Zernike-Noll coefficients in the M1 system of coordinates. WF is positive,
;        for instance, when it is produced by a bump on M1 surface.
;
;F2P projection matrix from forces to position
;For mode offload computation:
; - compute dpos from flat and actual posiion
; - select for piston tip tilt astigmatismthe position feedback
; - use for focus, coma and spherical aberration the maximum value between poa and force feedback

Pro make_offload_matrix_proj, imfile, NZERNIKE=nzern, CL_ACT=act_w_cl, UMAT=umat_out $
                            , C2Z=c2z, PMFOR=pmfor_out, F2C=f2c, SLAVE_ACT=slave $
                            , Z2C=z2c, THRESHOLD=thr, VERBOSE=verbose, SAVE=do_save, TRACKNUM=fits_tn

    @adsec_common
    if n_elements(nzern) eq 0 then nzern=22

    if n_elements(imfile) eq 0 then message, "Optical interation matrix file has to be passed"

    if n_elements(thr) eq 0 then thr=1e-4

    ;FACTOR FOR MATCHING TELECOPE REQUESTS:
    ; factor 2 to pass from surface to wavefront(WF)
    ; poi devo flippare le x e le y degli zernike (secondario vs primario) e fare cosi' le matrico
    ; i coefficienti trovati sono ESATTAMENTE quelli da dare al primario


    restore, imfile, /ver
    ; Restore variables:
    ;  DPIX: size of the square array containing the pupil
    ;  IDX_MASK: index list of the DPIX x DPIX array of elements inside the pupil
    ;  XX,YY: x and y coordinates normalized to unit pupil radius at idx_mask
    ;  MMMATRIX: nmodes x n_elements(idx_mask) array to convert surface modal coeffs vector to optical surface at idx_mask 
    ;  MM2C: nmodes x adsec.n_actuators array to convert surface modal coeffs vector to actuator commands
    x = xx
    y = yy
    zsel=indgen(nzern)
    fact = 1/2d ;WF to surface factor

    ;zsel=[4,7,8,11,12,13,16,17,18,19,22] -1

    dim_mmmat = size(mmmatrix, /DIM) 
    nmodes = dim_mmmat[0]
    nidx = dim_mmmat[1]
    act_w_cl = where(rebin(mm2c^2,1,672) ne 0, n_act_w_cl)
    act_wo_cl = where(rebin(mm2c^2,1,672) eq 0, n_act_wo_cl)
	if n_elements(slave) gt 0 then begin
			print, "Slave actuators found..."+strtrim(n_elements(slave),2)
			err= complement(slave, indgen(672),master) 
			if err ne 0 then message, "Unexpected error on complement..."
			err = intersection(master, act_w_cl, master_w_cl)
			if err ne 0 then message, "Unexpected error on intersection..."
			act_w_cl = master_w_cl
			err= complement(master_w_cl, indgen(672),act_wo_cl)
			n_act_w_cl = n_elements(act_w_cl) 
			n_act_wo_cl = n_elements(act_wo_cl) 
	endif

    n_act = n_act_w_cl + n_act_wo_cl
    if n_act ne adsec.n_actuators then message, "Unexpected number of actuators"
    mmmatrix -= rebin(rebin(mmmatrix,nmodes,1),dim_mmmat, /SAMP)
    print, "pseudo-inverting MMmatrix..."
    rec_mat = pseudo_invert(mmmatrix, /VERB, /DOUBLE, W=w, U_MAT=u, V_MAT=v $
                           , N_MODES_TO_DROP=1, IDX=idx_w, COUNT=count_w) ; piston forced to be filtered out
    if keyword_set(verbose) then begin
        window, /FREE
        display, (mm2c##v[idx_w,*])[act_w_cl], act_w_cl, /SH, /SM, TITLE="rejected mode (act commands)"
        window, /FREE
        aa=fltarr(dpix,dpix)
        aa[idx_mask]=u[idx_w,*]
        image_show, aa, /AS, /SH, TITLE="rejected mode (Interferometer signal)"
        window, /FREE
        plot, w, PSYM=4, TITLE="Singular values (red: rejected mode)"
        plots, idx_w, w[idx_w], PSYM=4, COLOR=255L
    endif
    if n_elements(slave) gt 0 then begin
		print, "pseudo-invert MM2C"
		ifunc = mmmatrix ## pseudo_invert(mm2c, /VERB)
	endif else begin    
        ifunc = mmmatrix ## transpose(mm2c)
	endelse

    idx_max_ifunc=lonarr(n_act)
    max_ifunc = fltarr(n_act)
    for i=0,n_act-1 do begin
        dummy=max(abs(ifunc[i,*]),iii)
        idx_max_ifunc[i]=iii
        max_ifunc[i]=ifunc[i,iii]
    endfor
   
    if total(max_ifunc[act_w_cl] le 0) gt 0 then print, "WARNING: Influence Function peak expected to be positive, found negative"
    x_max=x[idx_max_ifunc]
    y_max=y[idx_max_ifunc]
    ang1 = atan(y_max[act_w_cl],x_max[act_w_cl])-atan(adsec.act_coordinates[1,act_w_cl],adsec.act_coordinates[0,act_w_cl])
    ;idx = where(ang1 lt 0, idx_count)
    ;if idx_count gt 0 then ang1[idx] += 2*!PI
    idx = where(ang1 lt -!pi, idx_count)
    if idx_count gt 0 then ang1[idx] += 2*!PI
    idx = where(ang1 gt !pi, idx_count)
    if idx_count gt 0 then ang1[idx] -= 2*!PI
    ang2 = atan(y_max[act_w_cl],x_max[act_w_cl])-atan(adsec.act_coordinates[1,act_w_cl],-adsec.act_coordinates[0,act_w_cl])
    ;idx = where(ang2 lt 0, idx_count)
    ;if idx_count gt 0 then ang2[idx] += 2*!PI
    idx = where(ang2 lt -!pi, idx_count)
    if idx_count gt 0 then ang2[idx] += 2*!PI
    idx = where(ang2 gt !pi, idx_count)
    if idx_count gt 0 then ang2[idx] -= 2*!PI
    sigma_ang1 = stddev(ang1)
    sigma_ang2 = stddev(ang2)
    print, "Angular spread [deg] wo x-mirroring:",sigma_ang1/!PI*180.0, ", w x-mirroring:", sigma_ang2/!PI*180.0
    if sigma_ang1 lt sigma_ang2 then begin
        do_x_mirror = 0B
        ang_vec = ang1
    endif else begin
        do_x_mirror = 1B
        ang_vec = ang2
    endelse
    ang = mean(ang_vec)
    sigma_ang = stddev(ang_vec)

    r1 = sqrt(x_max[act_w_cl]^2+y_max[act_w_cl]^2)
    r2 = sqrt(adsec.act_coordinates[0,act_w_cl]^2+adsec.act_coordinates[1,act_w_cl]^2)
    scale_fac = mean(r1/r2)
    sigma_scale_fac = stddev(r1/r2)
    print, "rotation [deg]:",ang/!PI*180.0, "(sigma:",sigma_ang/!PI*180.0,")"
    print, "scale_factor:",scale_fac,"(sigma:",sigma_scale_fac,")"

    rot_mat = [[ cos(ang),-sin(ang)], $
               [ sin(ang), cos(ang)]]
    rot_mat *= scale_fac
 
    xy_orig = transpose(adsec.act_coordinates)
    xy = xy_orig
    if do_x_mirror then xy[*,0] *= -1

    dummy = rot_mat ## xy
    x_rot = dummy[*,0]
    y_rot = dummy[*,1]

    dx = mean(x_max-x_rot)
    dy = mean(x_max-x_rot)
    
    x_rot += dx
    y_rot += dy

    inv_rot_mat = invert(rot_mat)
    xy_zern = [[x_max-dx], $
               [y_max-dy]]
    xy_zern = inv_rot_mat ## xy_zern
    x_zern = xy_zern[*,0]
    y_zern = xy_zern[*,1]
    if do_x_mirror then x_zern *= -1

    if keyword_set(verbose) then begin
        window, /FREE
        plot, ang_vec*180/!PI, PSYM=4, TITLE="Clocking spread [deg], x_mirror:"+strtrim(do_x_mirror,2)
        oplot, [0,n_act_w_cl-1], ang*180/!PI*[1,1], COLOR=255L
        window, /FREE
        plot, r2, r1/r2, PSYM=4, XTIT="radial distance [mm]", YTIT="scale factor [1/mm]"
        oplot, [0,n_act_w_cl-1], scale_fac*[1,1], COLOR=255L
        window, /FREE
        plot, x_max[act_w_cl], y_max[act_w_cl], PSYM=4, TITLE="red: teorical geometry"
        oplot, x_rot[act_w_cl],y_rot[act_w_cl], PSYM=1, COLOR=255L
        for i=0,n_act_w_cl-1 do oplot, [x_max[act_w_cl[i]],x_rot[act_w_cl[i]]], [y_max[act_w_cl[i]],y_rot[act_w_cl[i]]]
        window, /FREE
        plot, x_zern[act_w_cl], y_zern[act_w_cl], PSYM=4, TITLE="red: teorical geometry"
        oplot, xy_orig[act_w_cl,0],xy_orig[act_w_cl,1], PSYM=1, COLOR=255L
        for i=0,n_act_w_cl-1 do oplot, [x_zern[act_w_cl[i]],xy_orig[act_w_cl[i],0]], [y_zern[act_w_cl[i]],xy_orig[act_w_cl[i],1]]
    endif


    xxyy_zern = [[x-dx], $
                 [y-dy]]
    xxyy_zern = inv_rot_mat ## xxyy_zern 
    xxyy_zern /= adsec_shell.out_radius
    xx_zern = xxyy_zern[*,0]
    yy_zern = xxyy_zern[*,1]
    if do_x_mirror then xx_zern *= -1


    umat = get_u_zern_matrix(-xx_zern, -yy_zern, indgen(nzern)+1)
    ;uplus = invert(transpose(umat) ## umat) ## transpose(umat)

    m_coeffs = rec_mat ## (fact*umat)

    ;mode selection using RMS threshold
    dummy_cum = dblarr(nmodes, nidx) 
    zern_fit_rms = dblarr(nzern, nmodes)
    for  i=0, nzern-1 do begin
       print, "Zernike to analyze: "+string(i, format='(I2.2)')
       for j=0, nmodes-1 do begin
            dummy = mmmatrix[j,*] * m_coeffs[i, j]
                
            if j eq 0 then begin
                dummy_cum[j,*] = dummy
            endif else begin
                dummy_cum[j,*] = dummy_cum[j-1,*] +dummy
            endelse
            zern_fit_rms[i, j] = stddev(dummy_cum[j,*])
       endfor

    endfor

    modes_cut_id = fltarr(nzern)
    m_coeffs_cut = m_coeffs
    ;Piston takes no action
    m_coeffs_cut[0, *] = 0
    for i=1, nzern-1 do begin
        modes_cut_id[i] = min(where(abs(zern_fit_rms[i,nmodes-1]-zern_fit_rms[i,*]) lt abs(zern_fit_rms[i,nmodes-1])*thr))+1
        modes_cut_id[i] = modes_cut_id[i] < (nmodes-1)
        m_coeffs_cut[i, modes_cut_id[i]:*]=0
    endfor
    print, "threshold used for cutting mirror modes:", thr
    print, "number of mirror modes used to compiute the zernikes:"
    print, modes_cut_id
     
    z2c  = mm2c[0:nmodes-1, *] ## m_coeffs_cut ;pmz

    print, "pseudo-inverting z2c..."
    c2z = pseudo_invert(z2c, /VERB, W=w_z2c)
    c2z[act_wo_cl,*]=0                      ;pos2modes
	umat_out=umat

    if keyword_set(do_save) then begin
      if n_elements(fits_tn) eq 0 then fits_tn = tracknum()
      path = meas_path('optical-projection', DAT=fits_tn)
      mkhdr, hdr, c2z, /EXTEND
      sxaddpar, hdr, "TRACKNUM", fits_tn
      sxaddpar, hdr, "UNIT", !AO_CONST.UNIT
      sxaddpar, hdr, "SHELL", !AO_CONST.SHELL
      sxaddpar, hdr, "MMMATRIX", file_basename(imfile), "filename of used matrix of FF mirror modes influence functions"
      sxaddpar, hdr, "THRESHOLD", thr, "threshold on relative rms used to remove high order mirror modes"
      writefits, path+"pos2modes.fits", c2z, hdr

      mkhdr, hdr, z2c, /EXTEND
      sxaddpar, hdr, "TRACKNUM", fits_tn
      sxaddpar, hdr, "UNIT", !AO_CONST.UNIT
      sxaddpar, hdr, "SHELL", !AO_CONST.SHELL
      sxaddpar, hdr, "MMMATRIX", file_basename(imfile), "filename of used matrix of FF mirror modes influence functions"
      sxaddpar, hdr, "THRESHOLD", thr, "threshold on relative rms used to remove high order mirror modes"
      writefits, path+"pmz.fits", z2c, hdr

    endif
end

;Pro test_offload_matrix_proj
;    @adsec_common
;    file1 = '/towerdata/adsec_calib/CMD/shape/flat_2009_05_18_154957.sav'
;    file2 = '/towerdata/adsec_calib/CMD/shape/20090722_091937_stability_2009_04_07_20deg_optical_flat_with_100modes_opt_mat_2009_03_31_05deg.sav'
;    restore, file1
;    f1 = flattened_status
;    restore, file2
;    f2 = flattened_status
;    dpos = transpose(f2.position-f1.position)
;    curr = transpose(f2.current)
;    make_offload_matrix_proj,22, UMAT=umat, PMPOS=pm, PMFOR=pmf, F2P = fsp
;    z1 = pm ## dpos
;        
    ;per selezionare il modi decompongo i modi di zernike piu' rigidi in modi dello specchio e vedo quali sono quelli a piu' alta componente
;    zsel=[4,7,8,11,12,13,16,17,18,19,22] -1
;    m_coeffs = transpose(adsec.ff_p_svec) ## umat[zsel, *]
;    ncl = n_elements(adsec.act_w_cl)
;    mm_coeffs = rebin(m_coeffs,1, adsec.n_actuators)
;    id = where( abs(mm_coeffs) gt 0.1*max(abs(mm_coeffs)))
;
;    w_inv = fltarr(adsec.n_actuators)
;    w_inv[id] = 1d/adsec.ff_sval[id]
;    
;    eq_pos = adsec.ff_p_svec ## diagonal_matrix(w_inv) ## transpose(adsec.ff_f_svec) ## curr
;
;    z2 = pm ## eq_pos
;    print, [indgen(1, 22)+1, z1, z2]   
;;
;End
   

