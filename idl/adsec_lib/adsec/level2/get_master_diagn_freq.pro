;$Id: get_master_diagn_freq.pro,v 1.1 2007/11/05 10:32:20 marco Exp $$
;+
;   NAME:
;    GET_MASTER_DIAGN_FREQ
;   
;   PURPOSE:
;    Get FAST diagnostic main characteristics.
;
;   USAGE:
;    err = get_master_diagn_freq([ENABLE=en, OVERSAMPLING_TIME=ovs, FAST_TIME= ft, MASTER=mas)
;
;   INPUT:
;    None.
;
;   OUTPUT:
;    err:   error code.
;    en:    int,    2 if master diagnostic and overssampling time are enabled
;                   0 both disable
;                   1 master diagnostic enable and oversampling disable
;                   -1 else
;    ovs:   oversampling period
;    ft:    fast diagnostic elaboration period   ->>> TODO
;    ms:    only master diagnostic status
;
;   HISTORY:
;    Created by Marco Xompero (MX)
;    on 4 Nov 2007
;    marco@arcetri.astro.it
;-

Function get_master_diagn_freq, ENABLE=enable, OVERSAMPLING_TIME=ovs_time, FAST_TIME = fast_time, MASTER=enable_mas

    @adsec_common
    ;FILLING OVERSAMPLING TIMe
    err = read_seq_dsp(0, switch_map.TimeoutFrameRate, 1L, tmt, /SWITCH, /Ul)
    if err ne adsec_error.ok then return, err
    if tmt gt 0 then enable_ovs=1 else enable_ovs=0
    ovs_time = tmt/106.25d*7/16/1e6
    
    ;Getting MASTER status
    err = read_seq_board(0, bcu_sram_map.enable_master_diag, 1L, val, /SWITCH, /UL, SET_PM = bcu_datasheet.sram_mem_id) 
    if err ne adsec_error.ok then return, err
    if val gt 0 then enable_mas=1 else enable_mas=0

    ;CHECKING ENABLE/DISABLE
    if enable_mas eq 1 then enable = enable_mas+enable_ovs else enable = -enable_ovs

    ;Fast Elaboration time
    ;;;;;;;;;;;;;;;;;; TODO

    return, adsec_error.ok

End
