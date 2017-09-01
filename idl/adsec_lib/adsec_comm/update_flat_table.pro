Function update_flat_table, READTABLE=READTABLE

    @adsec_common
    if n_elements(AO_STATUS_BACKUP) eq 0 then AO_STATUS_BACKUP=!AO_STATUS
    if keyword_set(READTABLE) then !AO_STATUS.FLAT_LIST = get_flat_table()

    if sc.name_comm eq "Dummy" then return, adsec_error.ok

    ;st = strupcase(strmid(!AO_CONST.telescope_side,0,1))

    if !AO_CONST.side_master then begin

        err = write_var('FLAT_LIST',  !AO_STATUS.FLAT_LIST)
        if err ne adsec_error.ok then return, err

        err = write_var('LAST_FLAT_WRITTEN',  !AO_STATUS.LAST_FLAT_WRITTEN)
        if err ne adsec_error.ok then return, err

        err = write_var('LAST_FLAT_LOADED',  !AO_STATUS.LAST_FLAT_LOADED)
        if err ne adsec_error.ok then return, err


    endif else begin


        err = read_var('FLAT_LIST',  tmp)
        if err ne adsec_error.ok then return, err
        if n_elements(tmp) eq 0 then tmp = ""
        !AO_STATUS.FLAT_LIST = string(tmp)

        err = read_var('LAST_FLAT_WRITTEN',  tmp)
        if err ne adsec_error.ok then return, err
        if n_elements(tmp) eq 0 then tmp = ""
        !AO_STATUS.LAST_FLAT_WRITTEN = string(tmp)

        err = read_var('LAST_FLAT_LOADED',  tmp)
        if err ne adsec_error.ok then return, err
        if n_elements(tmp) eq 0 then tmp = ""
        !AO_STATUS.LAST_FLAT_LOADED = string(tmp)

    endelse

    return, adsec_error.ok

end
