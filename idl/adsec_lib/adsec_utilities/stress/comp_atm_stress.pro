pro comp_atm_stress, n_layer=nlayer, noshow=noshow
    @adsec_common
    @sigma_common
    restore, stress.basedir+'/fitted_commands.sav' , /v
    
    if keyword_set(nlayer) then n_layer=nlayer

    maxstress = dblarr(n_layer)
    dmin = dblarr(n_layer)
    nact = lonarr(n_layer)
    act_used = lonarr(adsec_shell.n_true_act)                                                         

    for i=0,n_layer-1 do begin
        sigma_shape, vectorS, commands[i,*], MAX=XY, STRESS=0, NOSHOW=noshow, $
                 NACT=nact_d, DMIN=dmin_d, /POS
        maxstress[i] = max(vectorS)
        nact[i] = nact_d
        dmin[i] = dmin_d
        act_used[nact[i] mod adsec_shell.n_true_act] += 1 
    endfor

    display, act_used[adsec.act2mir_act], num_ty=1 ,/as,/sh

    save, maxstress, dmin, nact, act_used, n_layer, file=stress.basedir+'/fitted_stress.sav'
    stop

end
