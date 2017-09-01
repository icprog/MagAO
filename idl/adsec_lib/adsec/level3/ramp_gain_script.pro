;$Id: ramp_gain_script.pro,v 1.1 2009/09/18 08:25:01 marco Exp $
Function ramp_gain_script, filename, the_cl_list


    ;ERROR CATCHING

    @adsec_common
    if n_elements(the_cl_list) eq 0 then cl_list = indgen(adsec.n_actuators) else cl_list = the_cl_list
    data = read_ascii_structure(filename)

    d_all = data.d_all
    p_all = data.p_all

    d_clist = data.d_clist
    p_clist = data.p_clist

    d_flist = data.d_flist
    p_flist = data.p_flist

    d_slist = data.d_slist
    p_slist = data.p_slist

    flist = data.flist
    slist = data.slist

    exlist = data.exclude_list
    if exlist[0] gt 0 then  begin
       err = complement(exlist, cl_list, out_list)
       if err eq 0 then cl_list = out_list
   endif


    ;LIST GENERATIONS: 
    ;ALL BUT DUMMY ACTS
    err = complement(adsec.dummy_act, cl_list, cl_list_wo_dummy)
    if err eq 1 then all_act = cl_list else all_act = cl_list_wo_dummy


    ;FLIST BUT DUMMIES
    err = intersection(flist, all_act, flist_wo_dummy)
    if err eq 1 then first_list_act = flist else first_list_act = flist_wo_dummy

    ;SLIST BUT DUMMIES
    err = intersection(slist, all_act, slist_wo_dummy)
    if err eq 1 then second_list_act = slist else second_list_act = slist_wo_dummy

    ;CLIST BUT FIST LIST, SECOND LIST, DUMMIES
    if flist[0] eq -2 then begin
        if slist[0] eq -2 then clist_act = all_act $
        else  begin
            err = complement(second_list_act, all_act, clist_wo_fs)
            if err eq 1 then clist_act = all_act else clist_act = clist_wo_fs
        endelse
    endif else begin
        if slist[0] eq -2 then begin
            err = complement(first_list_act, all_act, clist_wo_fs)
        endif else begin
            err = complement([first_list_act, second_list_act], all_act, clist_wo_fs)
        endelse
        if err eq 1 then clist_act = all_act else clist_act = clist_wo_fs
    endelse

    log_print, 'Set derivative gain on all actuators'
    err = ramp_gain(all_act, d_all, /speed, /NOCH)
    if err ne adsec_error.ok then begin
        message, 'Error ramping gain', cont = (sc.debug eq 0)
        return, err
    endif
    wait, 1

    log_print, 'Set proportional gain on all actuators'
    err = ramp_gain(all_act, p_all, /NOCH)
    if err ne adsec_error.ok then begin
        message, 'Error ramping gain', cont = (sc.debug eq 0)
        return, err
    endif
    wait, 1

    log_print, 'Set derivative gain on main list actuators'
    err = ramp_gain(clist_act, d_clist, /speed, /NOCH)
    if err ne adsec_error.ok then begin
        message, 'Error ramping gain', cont = (sc.debug eq 0)
        return, err
    endif
    wait, 1

    log_print, 'Set proportional gain on main list actuators'
    err = ramp_gain(clist_act, p_clist, /NOCH)
    if err ne adsec_error.ok then begin
        message, 'Error ramping gain', cont = (sc.debug eq 0)
        return, err
    endif
    wait, 1


    if first_list_act[0] ge 0 then begin

        log_print, 'Set derivative gain on first list actuators'
        err = ramp_gain(first_list_act, d_flist, /speed, /NOCH)
        if err ne adsec_error.ok then begin
            message, 'Error ramping gain', cont = (sc.debug eq 0)
            return, err
        endif
        wait, 1

        log_print, 'Set proportional gain on first list actuators'
        err = ramp_gain(first_list_act, p_flist, /NOCH)
        if err ne adsec_error.ok then begin
            message, 'Error ramping gain', cont = (sc.debug eq 0)
            return, err
        endif
        wait, 1

    endif

    if second_list_act[0] ge 0 then begin

        log_print, 'Set derivative gain on second list actuators'
        err = ramp_gain(second_list_act, d_slist, /speed, /NOCH)
        if err ne adsec_error.ok then begin
            message, 'Error ramping gain', cont = (sc.debug eq 0)
            return, err
        endif
        wait, 1

        log_print, 'Set proportional gain on second list actuators'
        err = ramp_gain(second_list_act, p_slist, /NOCH)
        if err ne adsec_error.ok then begin
            message, 'Error ramping gain', cont = (sc.debug eq 0)
            return, err
        endif

    end

    return, adsec_error.ok


End
