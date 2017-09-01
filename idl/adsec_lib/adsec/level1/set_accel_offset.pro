Function set_accel_offset, val, crate_id

    @adsec_common
    if n_elements(crate_id) eq 0 then crate_id = 5
    if n_elements(val) eq 1 then vals = replicate(val, 8) else vals=val
    if n_elements(vals) gt 1 and n_elements(vals) ne 8 then return, adsec_error.input_type
    ;if (total(vals lt 0) gt 0) or (total(vals gt 'ffff'xul) gt 0) then return, adsec_error.input_type
    if test_type(val,/ULONG, /LONG, /FLOAT) then return, adsec_error.input_type
    err = write_same_dsp(crate_id, acc_map.offset, vals, /SIGGEN, /CHECK)
    return, err

End
