
function correct_coma

    @adsec_common
    err   =   update_status()
    cur   =   sys_status.current
    idp   =   adsec.act_w_pos
    idc   =   adsec.act_w_curr
    term  =   indgen(22)+1
    x     =   adsec.act_coordinates[0,*]/455.
    y     =   adsec.act_coordinates[1,*]/455.     
    
    fit   =   surf_fit(x[idc],y[idc],cur[idc],term, /zern, coeff=coeff, umat=umat)
    coeff[0:3] = 0.
    coeff[5:n_elements(term)-1]=0.
    
    force =   umat##coeff
    pos   =   (pseudo_invert(adsec.ff_matrix, n_modes_to_drop=10))##force
    display, pos[idp], idp,/no_n,/sh, /sm, title='Delta command to flatten shell coma'
    ans   =   dialog_message('Do you want to apply the command?', /quest)
    if ans eq 'yes' then begin
        err   =   fsm_apply_cmd( -pos, /pass)
        if err ne adsec.error then log_print, 'Coma correction successfully completed'
    endif else begin
        log_print, 'Comma error will not be flattened now'
    endelse

end
