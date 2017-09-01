Function wait_until_event, MAX_IT=max_it $
                         , WFSFC=wfsfc, MIRRFC=mirrfc $
                         , WFSGT = wfsgt

    @adsec_common
    if n_elements(MAX_IT) eq 0 then max_it=10
    if keyword_set(WFSFC) then begin
        address = rtr_map.switch_SCStartRTR
        swb = 1L
    endif
    if keyword_set(WFSGT) then begin
        address = switch_map.WFSGlobalTimeoutCnt
        swb = 1L
    endif
    if keyword_set(MIRRFC) then begin
        address = switch_map.MirrFramesCounter
        swb = 1L
    endif
    
    if ~keyword_set(WFSFC) and ~keyword_set(WFSGT) and ~keyword_set(MIRRFC) then return, adsec_error.idl_par_num_error
    err = read_seq_dsp(0, address, 1L, cnt0, /UL, SW=swb)
    if err ne adsec_error.ok then return, err
    found=0L

    for i=0, max_it-1 do begin
        
        err = read_seq_dsp(0, address, 1L, cnt1, /UL, SW=swb)
        if err ne adsec_error.ok then return, err
        if cnt1 gt cnt0 then begin
            found = 1L
            break
        endif else wait, 0.01
        
    endfor
    if found then return, adsec_error.ok else return, adsec_error.IDL_EVENT_ERROR

end
