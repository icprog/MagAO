pro mostra, id_tmp, id2_tmp,  _EXTRA=_EXTRA, FILE=filename, XS=xs, YS=ys

    id = id_tmp
    
    @adsec_common
    loadct, 12
    !p.color=0
    !p.background = 'ffffff'xl
    oldc = !p.charsize
    oldt = !p.charthick
    !p.charsize = 2.0
    !p.charthick = 2.0
    if n_elements(XS) eq 0 then xs=1024
    if n_elements(YS) eq 0 then ys=1024


    window, /free, xs=xs, ys=ys
    aa = fltarr(672)+15
    aa[id]=0
    if n_elements(id2_tmp) then begin
        id2=id2_tmp
        aa[id2] = 5
        id = [id, id2]
    endif
    
;    if adsec.act_wo_pos[0] gt 0 then aa[adsec.act_wo_pos] = -0.5
;    if adsec.act_wo_curr[0] gt 0 then aa[adsec.act_wo_curr] = -0.5
    
    display, aa, NUM_LIST=id, NO_SMOOTH_VAL=20, OUT_VAL=15., min_v=0,max_v=16 ,MAP=512, _EXTRA=_EXTRA

    !p.color='ffffff'xl
    !p.background = 0
    !p.charsize = oldc
    !p.charthick = oldt
    loadct, 12
    if n_elements(filename) gt 0 then write_jpeg, filename, tvrd(true=3), true=3, quality=100

end    

