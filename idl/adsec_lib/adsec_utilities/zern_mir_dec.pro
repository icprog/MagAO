;this procedure fits a set of zernike modes into a set of mirror modes.

pro zern_mir_dec, file=file, mode2ex=mode2ex, nz=nz_tmp


@adsec_common
;1 nanometro di modo rms

;restore, './data/flatten2apply_068um.sav', /ver
restore, dialog_pickfile(), /ver
;restore, './flattened_mask.sav', /ver
;restore, adsec_state.flatten2apply
dim = size(flattened_opd, /dim)
if ~keyword_set(mode2ex) then mode2exclude = 24 else mode2exclude=mode2ex

;if ~keyword_set(SAVED_MODES) then begin

    ;poi queste righe coi nuovi flat vanno cancellate.
    idx_fm = where(flattened_mask)

    ;generate zernike polynomials
    ;xx and yy are coord normalized to unitary ray
    n_meta_pix_diam = 4*(2*adsec_shell.out_radius)/((adsec_shell.out_radius-adsec_shell.in_radius)/adsec_shell.n_rings)

    pix_per_meta_pix_x = round(n_pix_diam_x/n_meta_pix_diam) > 1
    pix_per_meta_pix_y = round(n_pix_diam_y/n_meta_pix_diam) > 1

    n_pix_x = ceil(float(x1-x0+1)/pix_per_meta_pix_x)*pix_per_meta_pix_x
    n_pix_y = ceil(float(y1-y0+1)/pix_per_meta_pix_y)*pix_per_meta_pix_y

    mask2flatten = flattened_mask[x0:x1, y0:y1]
    dummy= fltarr(n_pix_x, n_pix_y)
    dummy[0,0] = mask2flatten
    r_weight2flatten = rebin(dummy, n_meta_pix_x, n_meta_pix_y)
    r_mask2flatten = r_weight2flatten ge 0.6

    opd2flatten = flattened_opd[x0:x1, y0:y1]
    dummy = fltarr(n_pix_x, n_pix_y)
    dummy[0,0] = opd2flatten
    r_opd2flatten = rebin(dummy, n_meta_pix_x, n_meta_pix_y)
    idx = where(r_weight2flatten gt 0)
    r_opd2flatten[idx]=r_opd2flatten[idx]/r_weight2flatten[idx]

    intersect_mask_idx = where((total(r_mask_cube, 1) eq n_modes)*r_mask2flatten, count)

    ;ATTENTION: this part can be VERY SLOW AND HUGE FOR THE MEMORY USAGE
;    order = (ceil((adsec.n_act_ring(n_elements(adsec.n_act_ring)-1))/2.)

    ;increase the order to have some other mode that can match
;    order += 1

    ;fill the radia
    if n_elements(nz_tmp) eq 0 then nz = 6 else nz=nz_tmp;(order+1)(order+2)/2
    zernike = indgen(nz)+1
    m = n_elements(idx_fm)

    zmat = dblarr(nz, m, /nozero)
    rho = sqrt(xx[idx_fm]^2+yy[idx_fm]^2)
    theta = atan(yy[idx_fm], xx[idx_fm])
    for i=0, nz-1 do begin
        zmat[i, *] = zern_jpolar(zernike[i], rho, theta)
    endfor
    zcube = fltarr(nz, n_elements(intersect_mask_idx))
    tmp = fltarr(n_pix_x, n_pix_y)
    for i=0, nz-1 do begin
        dummy = fltarr(dim[0], dim[1])
        dummy[idx_fm] = zmat[i,*] 
        dummy = dummy[x0:x1, y0:y1]
        tmp[0,0] = dummy
        zcube[i,*] = (rebin(tmp, n_meta_pix_x, n_meta_pix_y))[intersect_mask_idx]/r_weight2flatten[intersect_mask_idx]
    endfor
    
    zmodes_all = rec_mat ## zcube
    zmodes = zmodes_all
    if mode2exclude ne 0 then zmodes[*,n_elements(zmodes[0,*])-mode2exclude :*]=0.0
    zmodes = [[float(zmodes)], [fltarr(nz, n_elements(adsec.dummy_act))]]
    dzcom = float((adsec.ff_p_svec ## zmodes))
    dff_force= adsec.ff_matrix ## dzcom

;endif

if ~keyword_set(FILE)   then  save, file='zernike_optical_modes.sav', dff_force, dzcom, zmodes, zmodes_all, mode2exclude $
                        else save, file=file, dff_force, dzcom, zmodes, zmodes_all, mode2exclude

    ppp = fltarr(n_meta_pix_x, n_meta_pix_y, nz)
    for i=0, nz-1 do begin
        dummy = fltarr(n_meta_pix_x, n_meta_pix_y)
        dummy[intersect_mask_idx] = zcube[i,*]
        ppp[*,*,i] = dummy
    endfor





end
