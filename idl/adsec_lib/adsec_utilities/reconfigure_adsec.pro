;FUNCTION FOR RECONFIGURE ADSEC AFTER AN ACTUATOR REMOVE FROM CL ACT
;
Pro reconfigure_adsec, RESTOREF=restoref, RESET=reset

    @adsec_common
    if keyword_set(RESET) then begin
        
        return
    endif
    if keyword_set(restoref) then begin
        file = dialog_pickfile()
    endif else begin
 
        print, "Insert the actuator DSP ID which you want to remove for BAD CAPACITIVE SENSOR behavior:"
        print, "(Dash separated list, blank for no changes)"
        print, "Insert the actuator DSP ID which you want to remove for BAD COIL behavior:"
        print, "(Dash separated list, blank for no changes)"
        print, "Insert the actuator DSP ID which you want to remove from CONTROL CURRENT INTEGRATION"
        print, "Do you want to replace the feed-forward matrix with the new reduced one?" 
        print, "Yes/NO"
        print, "New offload projection matrices will be computed (for AO and SL mode)"
        print, "New projection matrix for accelerometer use will be computed"
        print, "New FastDiagnostic configuration file will be written and installed"

    endelse

End
