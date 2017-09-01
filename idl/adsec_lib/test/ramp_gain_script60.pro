;$Id: ramp_gain_script60.pro,v 1.4 2009/06/11 12:44:49 labot Exp $
function ramp_gain_script60

;TS3 ACCEPTANCE GAINS

@adsec_common
d_all = 0.25 ; 0.25;0.3
d_list =0.3 ; 0.25 ;0.4
;d_ext = 0.7
;d_mid = 0.35;
;d_in =  0.35;

p_all = 0.06;0.09  ;0.09
p_list = 0.13;0.09 ;0.09
p_bad = 0.1
;p_ext =0.2
;p_mid =0.1
;p_in =0.1
actual_bad_list = [368, 18];, 290]
bad_list = adsec.mir_act2act(indgen(adsec_shell.n_act_ring[13])+total(adsec_shell.n_act_ring[0:12]))
act_all = adsec.act_w_cl
err = intersection(bad_list, act_all, list)
if n_elements(list) ne n_elements(bad_list) then bad_list=list

err =complement(bad_list,act_all, act_list)
if err ne adsec_error.ok then begin
    print, 'The bad actuators list is wrong.'
    return, err
endif

; ramp derivative gain on all act
print, 'Set derivative gain on all actuators'
err = ramp_gain(act_all, d_all, /speed, /NOCH)
if err ne adsec_error.ok then begin
    message, 'Error ramping gain', cont = (sc.debug eq 0)
    return, err
endif
wait, 1

print, 'Set derivative gain on the best actuators'
err = ramp_gain(act_list, d_list, /speed, /NOCH)
if err ne adsec_error.ok then begin
    message, 'Error ramping gain', cont = (sc.debug eq 0)
    return, err
endif
wait, 1

print, 'Set proportional gain on all actuators'
err = ramp_gain(act_all, p_all, /NOCH)
if err ne adsec_error.ok then begin
    message, 'Error ramping gain', cont = (sc.debug eq 0)
    return, err
endif
wait, 1


if n_elements(actual_bad_list) ne 0 then begin

    err = complement(actual_bad_list, act_list, out_list)
    if err ne adsec_error.ok then begin
        print, 'The bad actuators list is wrong.'
        return, err
    endif
    
    print, 'Set proportional gain on the best actuators'
    err = ramp_gain(out_list, p_list, /NOCH)
    if err ne adsec_error.ok then begin
        message, 'Error ramping gain', cont = (sc.debug eq 0)
        return, err
    endif
    wait, 1

    
    print, 'Set proportional gain on the actual bad actuators'
    err = ramp_gain(actual_bad_list, p_bad, /NOCH)
    if err ne adsec_error.ok then begin
        message, 'Error ramping gain', cont = (sc.debug eq 0)
        return, err
    endif
    wait, 1

endif else begin

    print, 'Set proportional gain on the best actuators'
    err = ramp_gain(act_list, p_list, /NOCH)
    if err ne adsec_error.ok then begin
        message, 'Error ramping gain', cont = (sc.debug eq 0)
        return, err
    endif
    wait, 1

endelse


return, adsec_error.ok
end 
