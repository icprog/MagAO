Function analize_status, filename, VERBOSE=verbose
    
    @adsec_common
    restore, filename, /ver
    
    if n_elements(sys_status) gt 0 then begin
    
        ;CLOSED LOOP ACTS    
        dummy = where(sys_status.closed_loop, cc)
        if keyword_set(VERBOSE) then begin
            print, "CL ACT:               "+string(cc, format='(I3.3)')
            print, "MIRROR FRAME COUNTER: "+string(sys_status.MIRROR_CNT)
            print, "FL TIMEOUT:           "+string(sys_status.NUM_FL_TIMEOUT)
            print, "FL CRC:               "+string(sys_status.NUM_FL_CRC_ERR)
            print, "PENDING SKIP FRAMES   "+string(sys_status.PENDING_SKIP_FRAME_CNT)
            print, "SAFE SKIP FRAMES      "+string(sys_status.SAFE_SKIP_FRAME_CNT)
        endif
    
    endif

    if n_elements(hkpr_vars) gt 0 then begin
        
        ;DSP WATCHDOG
        idb = where(strmatch(hkpr_vars.family, '*Expired*', /FOLD))
        if max((hkpr_vars.last)[idb]) gt 0 then begin

            val = (hkpr_vars.last)[idb]
            nval = [transpose(val[0:(adsec.n_board_per_bcu*adsec.n_crates*adsec.n_dsp_per_board)/2-1]), $
                    transpose(val[(adsec.n_board_per_bcu*adsec.n_crates*adsec.n_dsp_per_board)/2:*])]
            nval = reform(nval, adsec.n_board_per_bcu*adsec.n_crates*adsec.n_dsp_per_board)
            wd_fault = where(nval, cc)
            dlist = strjoin(string(wd_fault, FORMAT='(I3.3)'), " ")
            ccs = string(cc, format='(I3.3)')
            if cc eq 168 then begin
                dlist = 'All'
                ccs = 'All'
            endif
            
            if keyword_set(VERBOSE) then log_print, "Watchdog expired for DSP ID: "+dlist+'(#'+ccs+')'
        endif else begin
            dlist = "None"
            ccs = "None"
        endelse
        ;print, "DSP WATCHDOG EXPIRED: "+dlist+'(#'+ccs+')'
    endif
    return, dlist
end
