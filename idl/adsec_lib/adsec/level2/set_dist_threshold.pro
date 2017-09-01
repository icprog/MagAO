;$Id: set_dist_threshold.pro,v 1.2 2008/03/18 08:59:35 labot Exp $$
;+
;   NAME:
;    SET_DIST_THRESHOLD
;
;   PURPOSE:
;    Configure firmware control on thin shell actuatl position. If the position exceed the level set, the system
;    goes in safe status by activating bus_sys_fault line. If activated, the PB need a reconfiguration before restart.
;
;   USAGE:
;    err = set_dist_threshold(act_list, dist_th, cnt_th)
;
;   INPUT:
;    act_list:  actuator list
;    dist_th:   threshold on actuators position
;    cnt_th:    consecutive fault activating the protection (1 for single shot)
;
;   OUTPUT:
;    Error code.
;
;   KEYWORD:
;    None
;    
;   NOTE:
;    On DSP program download, the default dist_th value is 1 [m].
;
;   HISTORY
;    Created on 18 Mar 2007, Marco Xompero
;    marco@arcetri.astro.it
;-


Function set_dist_threshold, act_list, dist_th, cnt_th, CLEAN=clean

    @adsec_common
    if n_elements(dist_th) eq 0 then dist_th=35e-6
    if n_elements(cnt_th) eq 0 then cnt_th = 1ul
    if n_elements(act_list) eq 0 then act_list = indgen(adsec.n_actuators)

    err = write_same_ch(act_list, dsp_map.pos_check_threshold, dist_th, /CHECK)
    if err ne adsec_error.ok then return, err

    err = write_same_ch(act_list, dsp_map.pos_check_cnt_threshold, cnt_th, /CHECK)
    if err ne adsec_error.ok then return, err

    if keyword_set(CLEAN) then begin
        err = write_same_ch(sc.all, dsp_map.pos_check_int_cnt_latch, 0L)
        if err ne adsec_error.ok then return, err
    endif

    return, adsec_error.ok

End
