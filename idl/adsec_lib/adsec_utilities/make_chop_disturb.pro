;+
; MAKE_MODAL_DISTURB
;
;EXAMPLES:
;print, make_modal_disturb(0, 100, /SEND, /SAV, EXT='pippo')    -> ./meas/disturbance/%date%/disturb_modal_000_100_pippo.sav
;print, make_modal_disturb(0, 100, /SEND, /SAV, /ZONAL)    -> ./meas/disturbance/%date%/disturb_zonal_000_100.sav

Function make_chop_disturb,  SEND=send, TRACK=track, EXT=ext, SAVEFILE=savefile                    $
                            , HIST=fhist, CHOP_A = chop_a, CHOP_B=chop_b, OVSTIME=ovstime
    @adsec_common
    if n_elements(ovstime) ne 0 then ovs = ovstime else ovs=0.002
    err = update_status()
    if err ne adsec_error.ok then return, err

    np=floor(15e-3/ovs)
    shape_preshaper = -(cos(!PI*findgen(np)/(np-1))-1)/2
    shape_preshaper[0] = 0
    shape_preshaper[np-1] = 1
    ns = 25-np
    shape_steady = fltarr(ns)+1.
    wave = [shape_preshaper,shape_steady, 1.-shape_preshaper, 1.-shape_steady] 
    nw = n_elements(wave)
    nrep = rtr.disturb_len/(nw)
    fullwave = (rebin(wave, nw, rtr.disturb_len/(nw)))[*]
    
    make_chopping_tilt, adsec.act_w_cl, tilt
    if ~keyword_set(chop_a) then begin
        pos_chop_a = -tilt*25e-6
    endif else begin
 ;       restore, chop_a, /ver
        pos_chop_a = chop_a
    endelse
    if ~keyword_set(chop_b) then begin
        pos_chop_b =  tilt*25e-6
    endif else begin
;        restore, chop_b, /ver
        pos_chop_b = chop_b
    endelse
    pos_chop_a -= mean(pos_chop_a[adsec.act_w_cl])
    pos_chop_b -= mean(pos_chop_b[adsec.act_w_cl])
    pos_chop_a = reform(reform(pos_chop_a), 1, adsec.n_actuators)
    pos_chop_b = reform(reform(pos_chop_b), 1, adsec.n_actuators)
    dpos  = rebin(pos_chop_b-pos_chop_a, rtr.disturb_len, adsec.n_actuators)
    posa  = rebin(pos_chop_a, rtr.disturb_len, adsec.n_actuators)
    presh = rebin(fullwave, rtr.disturb_len, adsec.n_actuators)
    pos0  = rebin(transpose(sys_status.position)-mean(sys_status.position[adsec.act_w_cl]), rtr.disturb_len, adsec.n_actuators)
    fhist = float(posa + dpos*presh -pos0)
    fhist[*,adsec.act_wo_cl] = 0
;modal filtering, addes RB
;    mc = transpose(adsec.ff_p_svec) ## fhist
;    idc = where(abs(mc) lt stddev(mc[n_elements(adsec.act_w_cl)-100:n_elements(adsec.act_w_cl)]*50))
;    ;err = complement(indgen(9), indgen(672), idc)
;    mc[idc] = 0.0

       ;TILT PRODUCED
;        tilt = adsec.ff_p_svec ## mc
;fhist=tilt
force = adsec.ff_matrix ## fhist

    print, "ptv [m], minmax: ", minmax(fhist)
    print, "peak force [N], minmax: ", minmax(force)

    if keyword_set(send) then begin
        err = set_disturb(/reset, /counter)
        if err ne adsec_error.ok then return,err
        err = set_disturb(fhist)
        if err ne adsec_error.ok then return,err
    endif
  
    if keyword_set(SAVEFILE) then begin
        adsec_save = adsec
        
        filename=meas_path('disturbance')
        ww=strsplit(filename, '/', /ex)
        track=ww[n_elements(ww)-1]
        type_str='chopping'
        filename= filename+'disturb_'+type_str+'.sav'
        save, file=filename,  adsec_save, fhist, type_str
    endif

    return, adsec_error.ok
    
    
end
     
