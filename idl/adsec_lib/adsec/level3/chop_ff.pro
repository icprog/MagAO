; $Id: chop_ff.pro,v 1.1 2007/11/21 17:49:28 labot Exp $

;+
;   NAME:
;    CHOP_FF
;
;   PURPOSE:
;    Step response utility for chopping test.
;
;   NOTE:
;    The mirror has to be set and the local closed loop enabled.
;
;   USAGE:
;    err=step_ff(type, number, amp, times, trig, pr_cmd, pr_cur, response $
;        ,sys_status0=sys_status0,sys_status1=sys_status1, ADDR=addr2save)
;
;   INPUT:
;    type:              type of step response ('actuator', 'modal' or 'zern').
;    number:            list of actuators/modes to step.
;    amp:               mode/actuators amplitude.
;    times:             buffer length for the acquisition.
;    trig:              triggering value.
;
;   OUTPUT:
;    pr_cmd:            current preshaper set.
;    pr_cur:            command preshaper set.
;    response:          step response given.
;    sys_status0:       sys_status structure at the start of the step response.
;    sys_status1:       sys_status structure at the end of the step response.
;
;   KEYWORDS:
;    ADDR       : dsp memory map address to monitor. Default distance, float_dac_value, preshaped_cmd.
;
;   HISTORY:
;-


function chop_ff, status_save_a, status_save_b, response 

@adsec_common


    addr2save=[dsp_map.distance,dsp_map.float_dac_value,dsp_map.preshaped_cmd]
    old_ovs = rtr.oversampling_time
    chop_period = 0.2

    err = get_preshaper(sc.all, old_pr_cmd)
    if err ne adsec_error.ok then return, err
    err = get_preshaper(sc.all, old_pr_cur, /FF)
    if err ne adsec_error.ok then return, err

    err = set_diagnostic(OVER=0.0)
    if err ne adsec_error.ok then return, err

    ;PRESHAPER SETTINGS

    timepcmd = 15e-3
    timepcurr = 15e-3
    time_ps = max([timepcmd, timepcurr])*1.1 > 0.01

    pr_cmd = ((round(adsec.preshaper_len*adsec.sampling_time/timepcmd)  > 1) $
           < dsp_const.fastest_preshaper_step)
    pr_cur = ((round(adsec.preshaper_len*adsec.sampling_time/timepcurr)  > 1) $
           < dsp_const.fastest_preshaper_step)

    err = set_preshaper(sc.all, pr_cmd)
    if err ne adsec_error.ok then return, err
    err = set_preshaper(sc.all, pr_cur, /FF)
    if err ne adsec_error.ok then return, err

    thold = chop_period/2. - max([timepcmd, timepcurr])
    fulltime = chop_period*3.+chop_period/2 < adsec.sampling_time*(65535)
    length = floor(fulltime/adsec.sampling_time)

    err = set_diagnostic(OVER=old_ovs)
    if err ne adsec_error.ok then return, err
  

    ;COMMAND HISTORY GENERATION
    err = update_status()
    update_panels, /NO_READ

    cmd_a = (sys_status_a.position - sys_status.command)[adsec.act_w_pos]
    cmd_b = (sys_status_b.position - sys_status.command)[adsec.act_w_pos]


    ;CHOP TEST WITH BUFFERS WORKING 
    nbuff= indgen(n_addr2save)
    err = set_IO_buffer(sc.all_dsp, nbuff, addr2save, length, $
                        rtr_map.update_ff, trig, /TRIG, RECORD=adsec.n_act_per_dsp, /START)
    if err ne adsec_error.ok then return, err

    err = set_offload_cmd(cmd_a, /START, /SW)
    if err ne adsec_error.ok then return, err
    wait, thold                                           ;TIME INDETERMINATION DUE TO UNSYNCH FRAMES
    err = set_offload_cmd(cmd_b, /START, /SW)
    if err ne adsec_error.ok then return, err
    wait, thold
    err = set_offload_cmd(cmd_a, /START, /SW)
    if err ne adsec_error.ok then return, err
    wait, thold
    err = set_offload_cmd(cmd_b, /START, /SW)
    if err ne adsec_error.ok then return, err
    wait, thold    
    err = set_offload_cmd(cmd_a, /START, /SW)
    if err ne adsec_error.ok then return, err
    wait, thold
    err = set_offload_cmd(cmd_a*0., /START, /SW)
    if err ne adsec_error.ok then return, err
    wait, thold
    for i=0, 10 do begin
        err = check_working_io(sc.all, nbuff, work)
        if err ne adsec_error.ok then return, err
        if max(work) eq 0 then break else wait, 0.1
    endfor

    if i eq 10 then begin
        print, "BUFFERS NOT STOPPED!!!!"
        return, adsec_error.generic_error
    endif else begin
        err=read_IO_buffer_ch(sc.all_dsp, nbuff, length, response)
        if err ne adsec_error.ok then return, err
        return, adsec_error.ok
    endelse

    ;;RESTORING PREVIOUS PRESHAPER SETTINGS
    err = set_diagnostic(OVER=0.0)
    if err ne adsec_error.ok then return, err

    err = get_preshaper(sc.all, old_pr_cmd)
    if err ne adsec_error.ok then return, err
    err = get_preshaper(sc.all, old_pr_cur, /FF)
    if err ne adsec_error.ok then return, err

    err = set_diagnostic(OVER=old_ovs)
    if err ne adsec_error.ok then return, err
    return, adsec_error.ok
end
