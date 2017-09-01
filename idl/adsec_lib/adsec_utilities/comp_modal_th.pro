Function comp_modal_th, rec, ACT_LIST=act_list

    @adsec_common

    ;GET THRESHOLD CURRENT VALUE 
    rrt = rec ## transpose(rec)
    thr=2.
    vv = thr*sqrt(extract_diagonal(rrt))
    if n_elements(act_list) eq 0 then act_list = indgen(n_elements(adsec.act_w_cl))
    ;NULL NOT CORRECTED MODES
    modal_th = fltarr(n_elements(act_list))
    modal_th[act_list] = vv[act_list]
    return, modal_th

End
