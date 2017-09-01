;file2 = '/towerdata/adsec_calib/CMD/shape/flat_sbagliato_dopo.sav'
;file1 = '/towerdata/adsec_calib/CMD/shape/flat_sbagliato_prima.sav'
;Pro appunti
;    @adsec_common
;file1 = '/towerdata/adsec_calib/CMD/shape/refshape.sav'
;file2 = '/towerdata/adsec_calib/CMD/shape/flat_data_OAT_calibrated.sav'
;file2 = filepath(ROOT=adsec_path.conf, SUB='data', '600modes_2009_08_10.sav')
;file1 = filepath(ROOT=adsec_path.conf, SUB='data', 'flat_data_OAT_calibrated.sav')

;file2 = filepath(ROOT=adsec_path.conf, SUB='data', 'flat_data_wfs_calibrated.sav')
function filt_flat5, sys_status_flat, sys_status_ini, ACT_LIST=act_list_tmp

    @adsec_common
    f1 = sys_status_ini
    f2 = sys_status_flat

    if n_elements(act_list_tmp) gt 0 then begin
        err = intersection(act_list_tmp, clact ,act_list)
        if err ne 0 then log_print, "Error: wrong list passed. The result can be without any sense"
        err = complement(act_list, clact, other_act)
        if err ne 0 then log_print, "Error: wrong list passed. The result can be without any sense"
        act_removed_list = act_list
    endif
    
    ;Compute delta position for flattening
    dpos = transpose(f2.position-f1.position)
    df = transpose(f2.current-mean(f2.current[adsec.act_w_curr])+mean(f1.current[adsec.act_w_curr])-f1.current)
    ;Compute new ff matrix excluding act_list
    idbad = [act_list , adsec.act_wo_cl]
    idgood = other_act
    ff_red = ff_matrix_reduction(adsec.ff_matrix, idbad, dx, k00, k01, k10, k11)

    la_svd, ff_red, wred, ured, vred, /DOUBLE

    m_dpos = transpose(vred) ## dpos[other_act]
    m_dpos[0:n_elements(other_act)-1-25] = 0.
    dpos_cut = vred ## m_dpos
    dcomm = fltarr(1, adsec.n_actuators)
    dcomm[idgood] = dpos_cut
    

    df1 = df[idgood]
    dc1 = dpos_cut
    df0 = df[idbad]
    dc0 = pseudo_invert(k00) ## (df0 - k01 ## dc1)
   
    dcomm[idbad] = dc0

    return, dcomm
end
