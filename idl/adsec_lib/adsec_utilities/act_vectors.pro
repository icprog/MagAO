pro act_vectors, act_del, act_all, act_r0, act_r1, act_r2
    
    @adsec_common
    ;act_del = [25, 26]
    act_in = adsec.mir_act2act[indgen(adsec_shell.n_act_ring[0])]
    err= intersection(act_in, adsec.act_w_cl, act_in)
    act_mean = adsec.mir_act2act[indgen(adsec_shell.n_act_ring[1])+adsec_shell.n_act_ring[0]]
    err= intersection(act_mean, adsec.act_w_cl, act_mean)
    act_out = adsec.mir_act2act[indgen(adsec_shell.n_act_ring[2])+total(adsec_shell.n_act_ring[0:1])]
    err= intersection(act_out, adsec.act_w_cl, act_out)

    if not intersection(act_del, act_in, dummy) then $
        err=complement(dummy, act_in, act_r0) else act_r0=act_in
    if not intersection(act_del, act_mean, dummy) then $
        err=complement(dummy, act_mean, act_r1) else act_r1=act_mean
    if not intersection(act_del, act_out, dummy) then $
        err=complement(dummy, act_out, act_r2) else act_r2=act_out

    if  complement(act_del, adsec.act_w_cl, act_all) then act_all = adsec.act_w_cl

end
