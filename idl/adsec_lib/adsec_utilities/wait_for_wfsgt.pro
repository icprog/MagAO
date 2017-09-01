function wait_for_wfsgt

    
    @adsec_common
    expired = 0
    cnt = 0
    bb = 0
    while (~expired) or (cnt ge 30) do begin
        wait, 0.02
        err = read_seq_dsp(0, switch_map.wfsglobaltimeout, 1l, /switch, bb, /UL)
        if err ne adsec_error.ok then return, err
        if bb eq 1 then begin
            expired = 1
            print, "WfsGlobalTimeout expired. Proceeding"
        endif
        cnt +=1
    endwhile
    if cnt gt 30 then begin
        log_print, "I'm waiting too long......"
        return, adsec_error.generic_error
    endif else begin
        return, adsec_error.ok
    endelse
        

End


