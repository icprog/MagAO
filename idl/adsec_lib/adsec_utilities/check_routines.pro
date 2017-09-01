Function compile_pro, currname
    
    catch, error_status
    if error_status ne 0 then return, -1 
    resolve_routine, currname, /COMPILE, /EITHER
    return, 0

End
Function check_routines, bad
    
    if strlowcase(!D.NAME) eq 'win' then return, adsec_error.ok
    pnames = strsplit(!path, ':', /EX)
    id = where(strmatch(pnames, '*adsec_lib*'))
    adsec_lib = pnames[id]
    id = where(strmatch(adsec_lib, '*test*'), compl=idnot)
    adsec_lib = adsec_lib[idnot]
    

    for i=0, n_elements(adsec_lib)-1 do begin

        spawn, 'find '+adsec_lib[i]+' -name "*.pro"', out
        if i eq 0 then fnames = file_basename(out, '.pro') else fnames = [fnames, file_basename(out, '.pro') ]

    endfor
    fnames = fnames[uniq(fnames, sort(fnames))]

    exclude = ['check_routines','general_setup', 'ao_define', 'startup-adsec', 'startup', 'startup-adsec-diagn', 'startup-onlycom', 'startup-adsec-autogain', 'adsec_common', 'sigma_common', 'xadsec_common','test_noise_script', 'dyn_data_script', 'ff_acquire_v2', 'ff_reduce_v2', 'step_script', 'find_chopping_gap', 'make_noise_plot', 'init_adsec', 'startup-ocom']
    for i=0, n_elements(exclude)-1 do begin
        id = where(fnames eq exclude[i], compl=idnot)
        if id[0] ne -1 then fnames = fnames[idnot] else print, exclude[i] + " NOT FOUND!"
    endfor

    priority = ['fsm_func_check', 'init_dsp_map', 'update_rtdb',  'init_adam', 'get_flat_table']
    for i=0, n_elements(priority)-1 do begin
        id = where(fnames eq priority[i], compl=idnot)
        fnames = [fnames[id], fnames[idnot]]
    endfor

    badb = intarr(n_elements(fnames))
    for i=0, n_elements(fnames)-1 do begin
       
        badb[i] = compile_pro(fnames[i])
;        if badb[i] ne 0 then stop

    endfor

    id = where(badb eq -1)
    if id[0] eq -1 then return, 0 $
    else begin
        bad = fnames[id]
        log_print, "Error in check_routines() procedure: there is at least one procedure that NOT COMPILES."
        return, -10099
    endelse
            
End
