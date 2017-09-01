Function retrieve_shortname, name

    return, stregex(file_basename(name), '^.*[A-Z]', /ex)
End



Function get_flat_shortname, instrument, name
    
    @adsec_common

   
    ;if instrument is not provided, select "IRTC2"
    if n_elements(instrument) eq 0 then instrument="IRTC" 
    if instrument[0] eq "" then instrument="IRTC" 
    
    if n_elements(name) eq 0 then name = 'default'
    if name eq "" then name = 'default'
   
;    list = get_flat_table(instrument)
;    if list[0] eq "" then return, instrument+".DEFAULT.sav"

;    info = (file_info(list)).atime
;    not_used_id = (sort(info))[0]
;    
;    nf = n_elements(list)
;    id = long(strmid(stregex(file_basename(list),'FLAT[^.]*', /EXTRACT),4))
;    fullId = lonarr(1000)
;    fullId[id] = 1
;    newId = (where(fullId eq 0))[0]
;    if newId eq -1 then newId = not_used_id
;    return, instrument+".FLAT"+string(newId, FORMAT='(I3.3)') 
    return, stregex(strupcase(instrument), '[^\.]*', /ex)+'.'+file_basename(name)

End

Function get_flat_table, instruments

    @adsec_common

;    path_calib = filepath(ROOT=adsec_path.conf, SUB=['data','flat'], '')
    path_calib = filepath(ROOT=adsec_path.data, SUB=['flat'], '')
    path_meas  = filepath(ROOT=adsec_path.meas, SUB='flat', '')
    
    list_calib   = file_search(path_calib,'', /TEST_REG)
;    list_meas    = file_search(path_meas,'', /TEST_REG)
;    if n_elements(instruments) gt 0 then begin
;        for i=0, n_elements(instruments)-1 do begin
;            tmp = stregex(strupcase(instruments[i]), '[^\.]*', /ex)
;            id_c  =  where(stregex(list_calib,tmp+'\.') gt 0)
;            id_m  =  where(stregex(list_meas,tmp+'\.') gt 0)
;            if (id_c[0] ge 0 and id_m[0] ge 0) then toadd = [list_calib[id_c], list_meas[id_m]] $
;            else if (id_c[0] ge 0) then toadd = list_calib[id_c] $
;            else if (id_m[0] ge 0) then toadd = list_meas[id_m] 
;            if n_elements(toadd) gt 0 then $
;                if n_elements(list) eq 0 then $
;                    list = toadd              $
;                else list = [list, toadd] 
;            undefine, toadd
;        endfor
;    endif else begin
;        tmp = file_basename([list_calib, list_meas])
;        list = [list_calib, list_meas]
;        list = list[uniq(tmp)]
;    endelse
    
    ;delete from list the file with same filename
    if n_elements(list_calib) eq 0 then return, "" else return, strjoin(list_calib[uniq(file_basename(list_calib))], ';')

End

