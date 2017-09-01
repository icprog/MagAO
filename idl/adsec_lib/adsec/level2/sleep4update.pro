;+
; SLEEP4UPDATE
;
; err = sleep4update()
;
; PURPOSE:  Set rtr_sleep to 1, in this case all the code is put in
; the safe area.
;
; HISTORY
;   3 Nov 2005 D.Zanotti & M.Xompero
;   20 Feb 2007 M.Xompero
;       Added sleeping for BCU and SwitchBCU
;-


function sleep4update

    @adsec_common

    err = write_same_dsp(sc.all, dsp_map.rtr_sleep, ulong(1))
    if err ne adsec_error.ok then return, err

    err = write_same_dsp(sc.all, bcu_map.RTRSleep, ulong(1), /BCU)
    if err ne adsec_error.ok then return, err

    err = write_same_dsp(sc.all, switch_map.RTRSleep, ulong(1), /SWITCH)
    if err ne adsec_error.ok then return, err

    return, err

end
