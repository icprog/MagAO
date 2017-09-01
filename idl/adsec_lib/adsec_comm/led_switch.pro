;$Id: led_switch.pro,v 1.2 2007/12/12 13:45:58 labot Exp $
;+
;   NAME:
;    LED_SWITCH_OFF
;
;   PURPOSE:
;    Switch ON/OFF most of AdSec diagnostic lights. An ON/OFF keyword MUST BE PROVIDED.
;
;   USAGE:
;    err = led_switch_off([/ON, /OFF][,/SWITCHB, /BCU])
;
;   INPUT:
;    None.
;   
;   OUTPUT:
;    err: error code.
;
;   KEYWORDS:
;    SWITCHB: switch only Switch BCU
;    BCU:switch only CrateBCUs and all DSP boards
;    ON: switch on
;    OFF: switch off
;
;   HISTORY:
;    Created by Marco Xompero (MX)
;    on 11 Nov 2007
;    marco@arcetri.astro.it
;-
Function led_switch, SWITCHB=is_switch, BCU=crate, ON=on, OFF=off

    @adsec_common
    if ~keyword_set(is_switch) and ~keyword_set(crate) then begin
        is_switch=1B 
        crate = 1B
    endif

    if keyword_set(ON) then  command=['4000000'xul, 0]
    if keyword_set(OFF) then command=['2000000'xul, 0]

    if keyword_set(is_switch) then begin
        ;err = reset_devices_wrap(sc.mirrctrl_id[adsec.n_bcu], 255, 255, 0L, 2, command)
        ;if err ne adsec_error.ok then return, err

    endif
    if keyword_set(crate) then begin
        for i=0, adsec.n_crates-1 do begin
            err = reset_devices_wrap(sc.mirrctrl_id[i],$
                                                 255, 255, 0, 2L, command)
            if err ne adsec_error.ok then return, err
            err = reset_devices_wrap(sc.mirrctrl_id[i],$
                                                 252, 252, 0, 2L, command)
            if err ne adsec_error.ok then return, err
            err = reset_devices_wrap(sc.mirrctrl_id[i],$
                                                 0, 2*adsec.n_board_per_bcu-1, 0, 2L, command)
            if err ne adsec_error.ok then return, err
        endfor

    endif

End
