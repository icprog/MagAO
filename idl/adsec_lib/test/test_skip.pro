Function test_skip

    @adsec_common
    err = getdiagnvalue(process_list.fastdiagn.msgd_name, 'skipframerate', -1, -1, bb)
    if err ne adsec_error.ok then return, err
    sk_fr = bb.average

    err = getdiagnvalue(process_list.housekeeper.msgd_name, 'dspcommandskipframe', -1, -1, bb)
    if err ne adsec_error.ok then return, err
    cval = bb.last
    cf = total(cval) gt 0

    err = getdiagnvalue(process_list.housekeeper.msgd_name, 'dspmodalskipframe', -1, -1, bb)
    if err ne adsec_error.ok then return, err
    mval = bb.last
    mf = total(mval) gt 0

    err = getdiagnvalue(process_list.housekeeper.msgd_name, 'dspforceskipframe', -1, -1, bb)
    if err ne adsec_error.ok then return, err
    fval = bb.last
    ff = total(fval) gt 0

    log_print, "Skip frame rate: "+string(sk_fr, FORMAT='(f6.2)')
    if cf then begin
        idc = where(cval gt 0)
        log_print, "Command/DeltaCommand threshold exceeded on DSP#: "+strjoin(strtrim(string(idc), 2), ' ')
    endif else begin
        log_print, "Commands inside safe range"
    endelse
    if mf then begin
        idm = where(mval gt 0)
        log_print, "Mode/DeltaMode threshold exceeded on DSP#: "+strjoin(strtrim(string(idm), 2), ' ')
    endif else begin
        log_print, "Modes inside safe range"
    endelse
    if ff then begin
        idf = where(cval gt 0)
        log_print, "Force threshold exceeded on DSP#: "+strjoin(strtrim(string(idf), 2), ' ')
    endif else begin
        log_print, "Forces inside safe range"
    endelse

    return, adsec_error.ok

End


