;High order matrix offload
function get_u_zern_matrix_ho, x, y, idx_list
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

Function comp_ho_offload_matrix, mat, AO=AO, M2C=m2c, NMODES_LIST=nmodes_list, SAV=do_sav, CUTMODE=cutmode, TRACKNUM=fits_tn, NZERN=nzern $
                               , FF_MATRIX_TN=ff_matrix_tn
    
    @adsec_common
    
    if n_elements(nzern) eq 0 then nzern=22
    if n_elements(ff_matrix_tn) eq 0 then begin
        ff_f_svec = adsec.ff_f_svec
        ff_p_svec = adsec.ff_p_svec
        ff_sval   = adsec.ff_sval
        act_w_cl  = adsec.act_w_cl
    endif else begin
        fileff = file_search(getenv('ADOPT_MEAS')+'/adsec_calib/ff_matrix/'+ff_matrix_tn+'/', 'ff_matrix*')
        restore, fileff

        act_w_cl  = act_w_ff
        ncl = n_elements(act_w_cl)

        tmp = fltarr(ncl, adsec.n_actuators)
        tmp[*, act_w_cl] = u
        ff_f_svec = fltarr(adsec.n_actuators, adsec.n_actuators)
        ff_f_svec[0:ncl-1, *] = tmp
        
        tmp = fltarr(ncl, adsec.n_actuators)
        tmp[*, act_w_cl] = v
        ff_p_svec = fltarr(adsec.n_actuators, adsec.n_actuators)
        ff_p_svec[0:ncl-1, *] = tmp

        ff_sval = fltarr(adsec.n_actuators)
        ff_sval[0:ncl-1]   = w
    endelse

    m2cname = 'UNKNOW'
    if keyword_set(AO) then begin

        type = 'AO'
        if n_elements(m2c) eq 0 then begin
            m2c = *rtr.m2c_matrix
            m2cname = !AO_STATUS.M2C
        endif 
        if n_elements(nmodes_list) eq 0 then begin
            recline = (*rtr.b0_a_matrix)[*,0]
            idm = where(recline eq 0, cc)
            if cc ne 0 then m2c[idm,*]=0
        endif else begin
            m2c[nmodes_list, *] = 0
        endelse
        mat= identity(adsec.n_actuators) - m2c ## pseudo_invert(m2c, /double)

    endif else begin

        type = 'SL'
        if n_elements(cutmode) eq 0 then cutmode = 100
        id = indgen(cutmode)
        id_not = cutmode+indgen(adsec.n_actuators-cutmode)
        u_cut = ff_f_svec        
        u_cut[id,*] = 0
        v_cut = ff_p_svec        
        v_cut[id,*] = 0
        w = ff_sval
        w_inv_cut = fltarr(adsec.n_actuators)
        null = where(w eq 0)
        if n_elements(null) eq adsec.n_actuators then begin
            print, 'WARNING: null FF matrix produces a null HO offload matrix...'
            return, fltarr(672, 672)
        endif
        if null[0] eq -1 then id_not_null = id_not else err = complement(null, id_not, id_not_null)
        w_inv_cut[id_not_null] = 1d/w[id_not_null]
        mat =  v_cut ## diagonal_matrix(w_inv_cut) ## transpose(u_cut)
    
    endelse 


    fact = 2. ;SURFACE to WAVEFRONT
    zsel = indgen(nzern)
    xx_zern = adsec.act_coordinates[0,act_w_cl]/adsec_shell.out_radius
    yy_zern = adsec.act_coordinates[1,act_w_cl]/adsec_shell.out_radius

    umat = get_u_zern_matrix_ho(-xx_zern, -yy_zern, indgen(nzern)+1)
    uplus = invert(transpose(umat) ## umat) ## transpose(umat)
    full_uplus = dblarr(adsec.n_actuators, nzern)
    full_uplus[act_w_cl, *] = uplus
    full_umat = dblarr(nzern, adsec.n_actuators)
    full_umat[*, act_w_cl] = umat

    m_coeffs = transpose(ff_p_svec) ## full_umat[zsel, *]
    ncl = n_elements(act_w_cl)
    mm_coeffs = rebin(m_coeffs,1, adsec.n_actuators)
    id = where( abs(mm_coeffs) gt 0.1*max(abs(mm_coeffs)))
    w_inv = fltarr(adsec.n_actuators)
    w_inv[id] = 1d/ff_sval[id]
    full_f2p = ff_p_svec ## diagonal_matrix(w_inv) ## transpose(ff_f_svec)
    dummy = full_f2p[act_w_cl, *]
    f2p = dummy[*, act_w_cl]

    pmfor =  uplus ## f2p
    full_pmfor = full_uplus *0.0d
    full_pmfor[act_w_cl, *] = pmfor   ;curr2modes

    umat_out = fact * full_umat
    pm_out = fact * full_uplus
    f2p_out = fact * full_f2p
    pmfor_out = fact * full_pmfor


    if keyword_set(do_sav) then begin

        if n_elements(ff_matrix_tn) eq 0 then fits_tn = tracknum()
        if n_elements(fits_tn) eq 0 then fits_tn = tracknum()
        path = meas_path('electric-projection', DAT=fits_tn)
        mkhdr, hdr, mat, /EXTEND
        sxaddpar, hdr, "TRACKNUM", fits_tn
        sxaddpar, hdr, "UNIT", !AO_CONST.UNIT
        sxaddpar, hdr, "SHELL", !AO_CONST.SHELL
        sxaddpar, hdr, "TYPE", type, "AO or SL mode"
        sxaddpar, hdr, "M2C", m2cname, "modes to command recontructor matrix"
        writefits, path+"pmhofor.fits", mat, hdr

        mkhdr, hdr, pmfor_out, /EXTEND
        sxaddpar, hdr, "TRACKNUM", fits_tn
        sxaddpar, hdr, "UNIT", !AO_CONST.UNIT
        sxaddpar, hdr, "SHELL", !AO_CONST.SHELL
        sxaddpar, hdr, "FFMATRIX", !AO_STATUS.FF_MATRIX, "filename of used ff matrix"
        writefits, path+"curr2modes.fits", pmfor_out, hdr


    endif

 

    return, mat


End
