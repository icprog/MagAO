;$Id: set_disturb.pro,v 1.10 2009/07/28 14:21:34 labot Exp $$
;+
;   NAME:
;    SET_DISTURB
;
;   PURPOSE:
;    Clearing, resetting and setting the disturbance DSP memory area. 
;
;   USAGE:
;    err = SET_DISTURB(vect,[/RESET, /DISABLE, /ENABLE])
;
;   INPUT:
;    vect: data disturbance with format [time,adsec.n_actuators]
;  
;   OUTPUT:
;    Error code.
;
;   KEYWORDS:
;    RESET:   if set, the disturbance buffer is cleared and all PTR will be reset before any other operation.
;    DISABLE: disable the disturbance add-on to the new_delta_command
;    ENABLE:  enable the disturbance add-on to the new_delta_command
;
;   HISTORY:
;    Written by Marco Xompero(MX) on 25 Jan 2007
;    marco@arcetri.astro.it
;    05 Mar 2007 MX
;       Disturbance vect on input added.
;    20 Mar 2007, DZ
;       Bug on disturbance setting fixed.
;    3 April 2007, DZ
;       Bug on disturbance cleaning fixed.
;-


Function set_disturb, vect, RESET=reset, DISABLE=disable, ENABLE=enable, COUNTER=counter, FITSFILE=fitsfile, MASTER_ENABLE=master_enable, MASTER_DISABLE=master_disable


    @adsec_common

    if keyword_set(DISABLE) or keyword_set(RESET) then begin
        err = write_same_dsp(0, switch_map.CmdDisturbAct, 0L, /CHECK, /SWITCH)
        if err ne adsec_error.ok then return, err
        ;err = read_seq_dsp(0, switch_map.ParamSelector, 1L, pbs, /SWITCH)
        ;if err ne adsec_error.ok then return, err
        ;pbs2w = pbs and (not (2l^15+2l^12))
        ;err = write_same_dsp(0, switch_map.ParamSelector, pbs2w, /CHECK, /SWITCH)
        ;if err ne adsec_error.ok then return, err
        !AO_STATUS.disturb_status = 0L
    endif


    if keyword_set(RESET) then begin
        
        ;Reset current pointer
        err = write_same_dsp(sc.all, dsp_map.disturb_curr_ptr, rtr_map.disturb_buffer,/CHECK)
        if err ne adsec_error.ok then return, err
        
        if ~keyword_set(COUNTER) then begin
            ;Clear data memory
            ;err = write_same_dsp(sc.all, rtr_map.disturb_buffer, fltarr(long(rtr.disturb_len)*adsec.n_act_per_dsp), /CHECK)
            err  = write_same_ch(sc.all, rtr_map.disturb_buffer,fltarr(rtr.disturb_len), /CHECK)
            if err ne adsec_error.ok then return, err
        endif


        ;Reset all pointers
        err = write_same_dsp(sc.all, dsp_map.disturb_start_ptr, rtr_map.disturb_buffer, /CHECK)
        if err ne adsec_error.ok then return, err

        err = write_same_dsp(sc.all, dsp_map.disturb_stop_ptr, rtr_map.disturb_buffer+long(rtr.disturb_len)*adsec.n_act_per_dsp, /CHECK)
        if err ne adsec_error.ok then return, err

        err = write_same_dsp(sc.all, dsp_map.disturb_curr_ptr, rtr_map.disturb_buffer, /CHECK)
        if err ne adsec_error.ok then return, err
        
    endif

    ;if total(abs(vect-rebin(vect[0,*],disturb_len, adsec.n_actuators))) then begin
    ;    message, 'WARNING: piston disturbance will be set...', /INFO
    ; sizev = size(vect, /dim)
    ;vdata= reform(vect, sizev[0], adsec.n_act_per_dsp, adsec.n_dsp)
    ;vdata= transpose(temporary(vdata), [1,0,2])
    ;vdata = reform(temporary(vdata),adsec.n_act_per_dsp*adesc.n_dsp ,2 )
    ;endif
    
    if n_elements(vect) ne 0 then begin
        if test_type(vect, /FLOAT) then begin
            message, "The disturbance vector must be float of size TIMExN_ACT", CONT = (sc.debug eq 0)
            return, adsec_error.input_type
        endif
        if (size(vect, /DIM))[1] ne adsec.n_actuators then begin
            message, "The disturbance vector must be float of size TIMExN_ACT", CONT = (sc.debug eq 0)
            return, adsec_error.input_type
        endif
        hist2w = reform(vect, long(rtr.disturb_len), adsec.n_act_per_dsp, adsec.n_dsp)
        hist2w = transpose(temporary(hist2w), [1,0,2])
        hist2w = reform(temporary(hist2w), adsec.n_act_per_dsp*long(rtr.disturb_len), adsec.n_dsp)
        err = write_same_dsp(0, switch_map.CmdDisturbAct, 0L, /CHECK, /SWITCH)
        if err ne adsec_error.ok then return, err
        ;write_seq_ch scrive 1 word per channel
        ;err = write_seq_ch(sc.all, rtr_map.disturb_buffer, vect, /CHECK)
        ;stop
        err = write_seq_dsp(sc.all, rtr_map.disturb_buffer, hist2w, /CHECK)
        if err ne adsec_error.ok then return, err
        if n_elements(fitsfile) gt 0 then !AO_STATUS.disturb = fitsfile

    endif


    if keyword_set(ENABLE) then begin
        err = write_same_dsp(0, switch_map.CmdDisturbAct, 1L, /SWITCH)
        if err ne adsec_error.ok then log_print, "Error enabling disturbance."
        err = read_seq_dsp(0, switch_map.ParamSelector, 1L, pbs, /switch)
        if err ne adsec_error.ok then return, err
        pbs2w = pbs or (2L^12 + 2L^15)
        err = write_same_dsp(0, switch_map.ParamSelector, pbs2w,  /SWITCH)
        if err ne adsec_error.ok then return, err
        !AO_STATUS.disturb_status = 1L
    endif

    if keyword_set(MASTER_ENABLE) then begin
        err = write_same_dsp(0, switch_map.CmdDisturbAct, 1L, /CHECK, /SWITCH)
        if err ne adsec_error.ok then return, err
    endif
    if keyword_set(MASTER_DISABLE) then begin
        err = write_same_dsp(0, switch_map.CmdDisturbAct, 2L, /CHECK, /SWITCH)
        if err ne adsec_error.ok then return, err
    endif
    
    return, update_rtdb()

End
