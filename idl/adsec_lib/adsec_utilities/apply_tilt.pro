Function apply_tilt, TIP=tip
    
    @adsec_common
    if n_elements(act_list) eq 0 then act_list = adsec.act_w_cl
    if n_elements(angle) eq 0 then angle=!pi/2
    if keyword_set(TIP)then angle = 0

    file_shell_ts3 =filepath(ROOT=adsec_path.commons, SUB='TS3', 'shell.txt')
    shell_ts3 = read_ascii_structure(file_shell_ts3)
    dim = 256
    shell_x = mk_vector(dim, -1, 1)
    shell_xx = rebin(shell_x, dim, dim)

    shell_y = transpose(mk_vector(dim, -1, 1))
    shell_yy = rebin(shell_y, dim, dim)
    shell_rr = sqrt(shell_xx^2+shell_yy^2)
    pupil =(shell_rr gt shell_ts3.in_radius/shell_ts3.out_radius ) * (shell_rr lt 1)
    idx = where(pupil, np)
    tilt_img = fltarr(long(dim)*dim)
    for i=1L, np-1 do tilt_img[idx[i]] = zern(2, shell_xx[idx[i]], shell_yy[idx[i]])*cos(angle) + zern(3, shell_xx[idx[i]], shell_yy[idx[i]])*sin(angle)
    tilt_img = reform(tilt_img, dim, dim) / max(tilt_img)


    ;40 [um] al bordo (2.5 arcsec + 1.5 arcsec per AO e FS)
    ;tilt_img *= 1e-6
    pxs = 1./2./(shell_ts3.out_radius)*float(dim)
    pys = 1./2./(shell_ts3.out_radius)*float(dim)
    xx_in =  (adsec.act_coordinates[0,*] + shell_ts3.out_radius)*pxs
    yy_in =  (adsec.act_coordinates[1,*] + shell_ts3.out_radius)*pys
    tilt_on_act = interpolate(tilt_img, xx_in, yy_in)

    pos2fit = fltarr(1,adsec.n_actuators)
    pos2fit[act_list] = tilt_on_act[act_list]

    mc = transpose(adsec.ff_p_svec) ## pos2fit

    idc = where(abs(mc) lt stddev(mc[n_elements(adsec.act_w_cl)-100:n_elements(adsec.act_w_cl)]*50))
    err = complement(indgen(9), indgen(672), idc)
    mc[idc] = 0.0

    ;TILT PRODUCED
    tilt = adsec.ff_p_svec ## mc
    force = adsec.ff_matrix ## tilt

    return, tilt
end
