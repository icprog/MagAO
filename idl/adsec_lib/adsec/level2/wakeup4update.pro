;+
; WAKEUP4UPDATE
;
; err = wake4update()
;
; PURPOSE:  Setting  rtr_sleep to 0 repeats all the register
; initialization, puts to 0 the delay_line e puts all the start to 0 and
; waits a new star_rtr.
;
; HISTORY
; 3 Nov 2005 D.Zanotti & M.Xompero
;   20 Feb 2007 M.Xompero
;       Added sleeping for BCU and SwitchBCU
;-


function wakeup4update

    @adsec_common

    err = write_same_dsp(sc.all_dsp, dsp_map.rtr_sleep, ulong(0))
    if err ne adsec_error.ok then return, err

    err = write_same_dsp(sc.all, bcu_map.RTRSleep, ulong(0), /BCU)
    if err ne adsec_error.ok then return, err

    err = write_same_dsp(sc.all_dsp, switch_map.RTRSleep, ulong(0), /SWITCH)
    if err ne adsec_error.ok then return, err

    return, err

end
