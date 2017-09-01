Function test_demo_mode
    
;    @adsec_common
    if lmgr(/DEMO) gt 0 then begin
        print, "IDL license server is not running. IDL is back to DEMO mode.";, log_lev=!AO_CONST.LOG_LEV_ERROR
        return, -1
    endif else begin
        print, "IDL license file is running.";, log_lev=!AO_CONST.LOG_LEV_INFO
        return, 0
    endelse

End
