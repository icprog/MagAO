;Calcola i risultati del filtro RTR sul secondario
Function filtro_rtr, g_in, B0_in, s_in, d_in, e_in, IDL=idl

    @adsec_common
    g=g_in
    b0=b0_in
    s = s_in
    d=d_in
    e=e_in
    if not keyword_set(IDL) then begin
        b0s = mamano(B0,s)
        dummy = g * (b0s + d)
        return, dummy+e
    endif else begin
        b0s = b0 ## s
        return, g * (b0s + transpose(d)) + transpose(e)
    endelse
    
End


;Calcola i risultati delle linee di ritardo
function calcola_de, BLOCK=block, d, e, c_s, c_m, IDL=idl
    
    @adsec_common
    if keyword_set(block) then addrbi = rtr_map.matrix_bb_delay else addrbi = rtr_map.matrix_ba_delay
    addrai =rtr_map.matrix_a_delay

    err = read_seq_dsp(0, rtr_map.modes_delay, adsec.n_actuators*rtr.n_modes_delay,bufd)
    modes_delay = reform(bufd, adsec.n_actuators,rtr.n_modes_delay)
    err= read_seq_dsp(0, rtr_map.slope_delay, rtr.n_slope*rtr.n_slope_delay,bufs)
    slope_delay = reform(bufs, rtr.n_slope,rtr.n_slope_delay)
    bid = loadmat(addrbi, rtr.n_slope, adsec.n_actuators, delay=rtr.n_slope_delay)
    aid = loadmat(addrai, adsec.n_actuators, adsec.n_actuators, delay=rtr.n_modes_delay )

    if not keyword_set(idl) then begin
        e = fltarr(adsec.n_actuators)
        for i = 0, rtr.n_modes_delay-1 do begin
            ai_curr = reform(aid[*,*,i])
            dp_curr = reform(modes_delay[*,(rtr.n_modes_delay-1-i+c_m) mod rtr.n_modes_delay])
            e = e + mamano(ai_curr, dp_curr, /REVERSE)
        endfor
        
        d = fltarr(adsec.n_actuators)
        for i = 0, rtr.n_slope_delay-1 do begin
            bi_curr = reform(bid[*,*,i])
            sv_curr = reform(slope_delay[*,(rtr.n_slope_delay-1-i+c_s) mod rtr.n_slope_delay])
            d = d + mamano(bi_curr, sv_curr, /REVERSE)
        endfor
        return,1
    endif else begin
        e = fltarr(adsec.n_actuators)
        for i = 0, rtr.n_modes_delay-1 do begin
            ai_curr = reform(aid[*,*,i])
            dp_curr = reform(modes_delay[*,(rtr.n_modes_delay-1-i+c_m) mod rtr.n_modes_delay])
            e = e + ai_curr ## dp_curr
        endfor
        
        d = fltarr(adsec.n_actuators)
        for i = 0, rtr.n_slope_delay-1 do begin
            bi_curr = reform(bid[*,*,i])
            sv_curr = reform(slope_delay[*,(rtr.n_slope_delay-1-i+c_s) mod rtr.n_slope_delay])
            d = d + bi_curr ## sv_curr
        endfor
        return,0
    endelse

End
