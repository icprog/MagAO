;Trigger ID management
Function set_if_trigger, dec_factor, offset
    
    @adsec_common
    if rtr.oversampling_time eq 0 then begin
        log_print, "oversampling not set. Returning."
        return, adsec_error.IDL_OVS_NOT_RUNNING
    endif
    if n_elements(dec_factor) eq 0 then dec_factor = 1L
    if n_elements(offset) eq 0 then offset = 0L
    dec_factor = dec_factor[0]
    offset = offset[0]
    if test_type(dec_factor, /long, /ulong, /int) then begin
        log_print, "Wrong parameter in input: they has to be long or ulong"
        return, adsec_error.input_type
    endif
    if test_type(offset, /long, /ulong, /int) then begin
        log_print, "Wrong parameter in input: they has to be long or ulong"
        return, adsec_error.input_type
    endif

    err = write_same_dsp(sc.all, switch_map.TriggerDecFactor, ulong(dec_factor), /SWITCH)
    if err ne adsec_error.ok then return, err
    if dec_factor eq 0 then log_print, "Trigger disabled" $
    else    log_print, "Trigger decimation factor set to: " + string(ulong(dec_factor), format='(I3.3)') + $
               " corresponding to a trigger frequency of " + $
               string(1./(ulong(dec_factor)*rtr.oversampling_time), format='(F7.2)')+ " [Hz]"

    err = write_same_dsp(sc.all, switch_map.TriggerOffset, ulong(offset), /SWITCH)
    if err ne adsec_error.ok then return, err
    if offset eq 0 then log_print, "Offset disabled" $
    else log_print, "Trigger offset set to: " + string(ulong(offset), format='(I3.3)') + $
               " corresponding with a delay of: " + string(ulong(offset)*rtr.oversampling_time*1e3, format='(F5.1)') + " [ms]"

    return, adsec_error.ok
   
End
