;the hexapod power line is controlled by the OUT5 line of the ADAM controller
function hexapod_power_on

     @adsec_common
    err = adam_update()
        if err ne adsec_error.ok then return, err
    out_str = adam_out
    out_str.out5 = 1
    err = adam_send(out_str)
    if err ne adsec_error.ok then return, err
    return, update_rtdb()

end
