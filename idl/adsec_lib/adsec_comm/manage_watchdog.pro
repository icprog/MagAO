;$Id: manage_watchdog.pro,v 1.4 2007/11/05 10:36:28 marco Exp $$
;+
;   NAME:
;    MANAGE_WATCHDOG
;   
;   PURPOSE:
;    This routine manage the ethernet watchdog and then DSp watchdog into the Ad.Sec Bcus.  If it goes in timeout, the coils are disabled.
;   
;   USAGE:
;    err = manage_watchdog(bcu_num, /ENABLE, /DISABLE, /CLEAR, /DSP)
;    
;   INPUT:
;    bcu_num:   CrateBCU to manage.
;
;   OUTPUT:
;    err: error code.
;
;   KEYWORDS:
;    ENABLE:    enable the BCU ethernet watchdog
;    DISABLE:   disable the BCU ethernet watchdog
;    CLEAR:     clear the BCU ethermet watchdog counter.
;    DSP:       enable/disable DSP watchdog (CLEAR is not permitted)
;
;   HISTORY:
;   03 Mar 2007
;       Written by Marco Xompero (MX)
;       Osservatorio Astrofisico di Arcetri, ITALY
;       marco@arcetri.astro.it
;   20 April 2007 Daniela Zanotti(DZ) & Fabio Tosetti(FT)
;       Substituted the call_external with the call to the C function. 
;   04 Nov 2007
;       DSP keyword added.
;-

Function manage_watchdog, bcu_num, ENABLE=enable, CLEAR=clear,  DISABLE=disable, DSP=dsp

    @adsec_common

    if ~keyword_set(DSP) then begin
        case 1 of
    
            keyword_set(ENABLE) : template='100000'xul
            keyword_set(DISABLE): template='200000'xul
            keyword_set(CLEAR)  : template='300000'xul
            else: begin
                message, "MANAGE_WATCHDOG error: at least one keyword must be selected.", CONT=(sc.debug eq 0)
                return, adsec_error.input_type
            end
        endcase

    endif else begin
        case 1 of
    
            keyword_set(ENABLE) : template='200000'xul
            keyword_set(DISABLE): template='100000'xul
            
            else: begin
                message, "MANAGE_WATCHDOG error: at least one keyword must be selected (CLEAR not allowed).", CONT=(sc.debug eq 0)
                return, adsec_error.input_type
            end
        endcase
    endelse
    ;==============================================================
    ;debug printing variables
    datalength = 2L
    dspAddress = 0L
    data = [template, 0ul]
    first_dsp = 0
    last_dsp = adsec.n_dsp_per_board*adsec.n_board_per_bcu-1
    if sc.ll_debug then begin
        print, 'Debugging data'
        help, bcu_num
        print, "MirrCtrl_id ", sc.mirrctrl_id[bcu_num]
        help, datalength
        help, dspAddress
        print, data
        print, 'sc.debug ', sc.debug
        print, 'data ', data
    endif
        
    ;Dummy case
    if sc.host_comm eq "Dummy" then begin
        err = adsec_error.ok
    endif else begin
        if ~keyword_set(DSP) then begin
            ;Standard case
            err = reset_devices_wrap(sc.mirrctrl_id[bcu_num], 255, 255, dspAddress, datalength, data)
            if err ne adsec_error.ok then return, err
        endif else begin
            err = reset_devices_wrap(sc.mirrctrl_id[bcu_num], first_dsp, last_dsp, dspAddress, datalength, data)
            if err ne adsec_error.ok then return, err
        endelse


    endelse
    
    return, err

end

