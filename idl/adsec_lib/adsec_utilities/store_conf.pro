Function store_conf, TEST=test

    @adsec_common
    tn = tracknum()
    sys = ['672a', '672b', '585m']
    for kk=0, n_elements(sys)-1 do begin
        path  = getenv('ADOPT_ROOT')+'/conf/adsec/'+sys[kk]+'/general/'
        spawn , 'find ' + path +' -name "*.txt"', txtfiles0
        path  = getenv('ADOPT_ROOT')+'/conf/adsec/'+sys[kk]+'/TS*'
        spawn , 'find ' + path +' -name "*.txt"', txtfiles1
        txtfiles2= [ getenv('ADOPT_ROOT')+'/conf/adsec/'+sys[kk]+'/configuration.txt' $
                   , getenv('ADOPT_ROOT')+'/conf/adsec/'+sys[kk]+'/processConf/fastdiagn/fastdiagn.param' $
                   , getenv('ADOPT_ROOT')+'/conf/adsec/'+sys[kk]+'/processConf/fastdiagn/fastdiagn.telemetry' $
                   , getenv('ADOPT_ROOT')+'/conf/adsec/'+sys[kk]+'/processConf/adamhousekeeper/adam_housekeeper.param' $
                   , getenv('ADOPT_ROOT')+'/conf/adsec/'+sys[kk]+'/processConf/adamhousekeeper/adam_housekeeper.telemetry' $
                   , getenv('ADOPT_ROOT')+'/conf/adsec/'+sys[kk]+'/processConf/housekeeper/housekeeper.param' $
                   , getenv('ADOPT_ROOT')+'/conf/adsec/'+sys[kk]+'/processConf/housekeeper/housekeeper.telemetry' ]
        path0 = getenv('ADOPT_ROOT')+'/conf/adsec/'
        out = getenv('ADOPT_MEAS')+'/adsec_calib/configuration'
        if n_elements(txtfiles) eq 0 then txtfiles = [txtfiles0, txtfiles1, txtfiles2] else txtfiles = [txtfiles,txtfiles0, txtfiles1,txtfiles2]
    endfor
    path = getenv('ADOPT_ROOT')+'/conf/adsec/commons/TS*'
    spawn , 'find ' + path +' -name "*.txt"', file2add
    txtfiles = [txtfiles, file2add]
    
    update_all=0
    cksum_file = getenv('ADOPT_ROOT')+'/conf/adsec/'+!AO_CONST.UNIT+'/cksum_file'

    if ~file_test(cksum_file, /REGULAR)  then begin
        log_print, LOG_LEV=!AO_CONST.log_lev_warning, "No CRC file found: building a new one and saving all the configuration in $ADOPT_ROOT"
        log_print, LOG_LEV=!AO_CONST.log_lev_info, "Building a new CRC file for the configurations..."
        openw, unit, cksum_file, /GET
        for i=0, n_elements(txtfiles)-1 do begin
            ;spawn, 'cksum '+txtfiles[i], out
            spawn, "sed -n '2,$p' "+txtfiles[i]+" | cksum", out
            printf, unit, out+" "+txtfiles[i]
        endfor
        free_lun, unit
        log_print, LOG_LEV=!AO_CONST.log_lev_info, "Built."
        update_all=1
    endif

    ;TEST if ALL the conf files are inside the cksum file or if the cksum is correct 
    log_print, LOG_LEV=!AO_CONST.log_lev_info, "Checking if some file is changed..."
    cksums = read_text_file(cksum_file)
    crc_fnames = strarr(n_elements(cksums))
    for i=0, n_elements(cksums)-1 do crc_fnames[i] = (strsplit(cksums[i], ' ', /ex))[2]
    for i=0, n_elements(txtfiles)-1 do begin
        id = where(crc_fnames eq txtfiles[i], cc)
        if id[0] eq -1 then begin
            log_print, LOG_LEV=!AO_CONST.log_lev_info, "Found missing file: "+txtfiles[i]
            if n_elements(missing_files) eq 0 then missing_files = txtfiles[i] else missing_files = [missing_files, txtfiles[i]]
        endif else begin
            if cc gt 1 then begin
                log_print,LOG_LEV=!AO_CONST.log_lev_error, "Error occurred during processing CRC configuration file. "
                log_print,LOG_LEV=!AO_CONST.log_lev_warning, "CRC file will be removed. "
                file_delete, cksum_file
                update_all=1
                return, adsec_error.generic_error
            endif else begin
                spawn, "sed -n '2,$p' "+txtfiles[i]+" | cksum", out
                curr_cksum = (strsplit(out, ' ', /ex))[0]
                ckfile_cksum = (strsplit(cksums[id], ' ', /ex))[0]
;                if i gt 85 then stop
                if ulong64(curr_cksum) ne ulong64(ckfile_cksum) then begin
                    log_print, LOG_LEV=!AO_CONST.log_lev_info, "Found changed file: "+txtfiles[i]
                    if n_elements(missing_files) eq 0 then missing_files = txtfiles[i] else missing_files = [missing_files, txtfiles[i]]
                endif
            endelse
        endelse
    endfor

    if update_all gt 0 then begin
        log_print, LOG_LEV=!AO_CONST.log_lev_info, "All configuration file will be stored..."
        missing_files = txtfiles
    endif

    if n_elements(missing_files) gt 0 then begin
        file_delete, cksum_file
        ;adding tn to the missing files
        log_print, LOG_LEV=!AO_CONST.log_lev_info, "Updating tracking number file for the configuration files..."
        spawn, "sed -i '1,1s/[0-9]*_[0-9]*/"+tn+"/' "+strjoin(missing_files, ' ')
        log_print, LOG_LEV=!AO_CONST.log_lev_info, "Done."
        ;update cksum_file 
        log_print, LOG_LEV=!AO_CONST.log_lev_info, "Updating CRC file for the configuration files..."
        openw, unit, cksum_file, /GET
        for i=0, n_elements(txtfiles)-1 do begin
            spawn, "sed -n '2,$p' "+txtfiles[i]+" | cksum", out
            printf, unit, out+" "+txtfiles[i]
        endfor
        free_lun, unit
        log_print, LOG_LEV=!AO_CONST.log_lev_info, "Updated."

        for i=0, n_elements(missing_files)-1 do begin
            out = getenv('ADOPT_MEAS')+'/adsec_calib/configuration/'
            tmp = strmid(file_basename(missing_files[i]), 0, strlen(file_basename(missing_files[i]))-4)
            outdir =filepath( ROOT=out + '/' +file_dirname(strmid(missing_files[i], strlen(path0))) + '/' + tmp + '/' +  tn +'/', '')
            name = outdir+file_basename(missing_files[i])
            if (file_info(outdir)).exists eq 0 then file_mkdir, outdir
            file_copy, missing_files[i], outdir, /OVER
        endfor

    endif else begin
        log_print, LOG_LEV=!AO_CONST.log_lev_info, "Nothing to do."
    endelse

            
    
    return, adsec_error.ok    

End

