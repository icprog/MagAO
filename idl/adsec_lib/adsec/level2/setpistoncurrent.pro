;ELEVATION IN DEGREES

Function setpistoncurrent, elevation

    @adsec_common
    if finite(elevation, /NAN) then begin
        log_print, "Wrong elevation value: "+string(elevation), log_lev = !AO_CONST.LOG_LEV_ERROR
        log_print, "The elevation will be set to safest: 90 [deg]"
        elevation=90
    endif
    if elevation gt 93 or elevation lt -3 then begin
        log_print, "Wrong elevation value: "+string(elevation)
        return, adsec_error.input_type
    endif
    
    ampVal = 60 + 30*cos(elevation /180.*!pi)
    log_print, "Setting overcurrent protection for "+string(elevation, format='(F5.2)')+" [deg]"
    err = set_curr_limits(ampVal, 0, /PUSHTOTAL)
    if err ne adsec_error.ok then return, err
    sys_status.elevation_angle = elevation
    log_print, "Done."


    return, adsec_error.ok

End
