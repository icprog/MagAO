;Function to test if there is contamination into the GAP or some actuator is not working
Function short_dust_test
    
    @adsec_common
    err = update_status()
    pos =double( sys_status.position)
    thmax = 135d-6
    thmin = 50d-6
;    thmax = 37.15d-6
;    thmin = 37.1d-6
    cl = adsec.act_w_cl
    id = where(pos[cl] gt thmax, cc)
    if cc gt 0 then begin
        idc = cl[id]
        update_popup=1
        dis = string(fix(idc[0])*1e6, format='(I4.4)')
        msg = "MISFUNCTIONING CAPSENS: (ID:"+string(idc[0], FORMAT='(I3.3)')+"-"+dis+"um)"
        log_print, "Found misfunctioning capacitive sensors", LOG_LEV=!AO_CONST.log_lev_error
        distr = string(pos[idc]*1e6, format='(I8.8)')+'um;'
        clstr =  string([idc], format = '(I3.3)')+':'
        str = [[clstr],[distr]]
        str = strjoin((transpose(str))[*], '')
        log_print, str, LOG_LEV=!AO_CONST.log_lev_error
        err= update_msg(msg) 
;        if err ne adsec_error.ok then return, err
        err= update_msg(msg, 'NWACT',1, /POPMSG) 
;        if err ne adsec_error.ok then return, err
        return, adsec_error.IDL_NW_ACT
    endif else begin
        update_popup=0
        msg = "No misfunctioning capacitive sensors found."
        log_print, msg, LOG_LEV=!AO_CONST.log_lev_info
        err= update_msg(msg)
        if err ne adsec_error.ok then return, err
        err= update_msg('','NWACT',/POPMSG, /CLEAR) 
        if err ne adsec_error.ok then return, err
    endelse

;    if update_popup then begin
;        wait, 2  ;needed to avoid overlapping messages in data dictionary
;    endif 

    
    if id[0] gt 0 then err = complement(cl[id], adsec.act_w_cl, new_list) else new_list = adsec.act_w_cl
    id = where(pos[new_list] gt thmin, cc)
    if cc gt 0 then begin
        idc = cl[id]
        update_popup=1
        dis = string(fix(pos(idc[0])*1e6), format='(I3.3)')
        msg = "POSSIBLE GAP CONTAMINATION: (ID:"+string(idc[0], FORMAT='(I3.3)')+"-"+dis+"um)"
        log_print, "Found possible gap contamination", LOG_LEV=!AO_CONST.log_lev_error
        distr = string(pos[idc]*1e6, format='(I3.3)')+'um;'
        clstr =  string(idc, format = '(I3.3)')+':'
        str = [[clstr],[distr]]
        str = strjoin((transpose(str))[*], '')
        log_print, str, LOG_LEV=!AO_CONST.log_lev_error
        err= update_msg(msg) 
;        if err ne adsec_error.ok then return, err
        err= update_msg(msg, 'CONTAMINATION',1,  /POPMSG) 
;        if err ne adsec_error.ok then return, err
        return, adsec_error.IDL_DUST_CONTAMINATION
    endif else begin
        update_popup=0
        str = "No gap contamination found."
        log_print, str, LOG_LEV=!AO_CONST.log_lev_info
        err= update_msg(msg) 
        if err ne adsec_error.ok then return, err
        err= update_msg('', 'CONTAMINATION',/CLEAR, /POPMSG) 
        if err ne adsec_error.ok then return, err
    endelse


End
