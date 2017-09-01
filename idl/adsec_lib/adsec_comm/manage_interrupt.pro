;$Id: manage_interrupt.pro,v 1.3 2009/04/10 13:52:15 labot Exp $$
;+
;   NAME:
;    MANAGE_INTERUPT
;   
;   PURPOSE:
;    This routine manage the interupt .
;   
;   USAGE:
;    err = manage_interupt(/ENABLE, /DISABLE)
;    
;   INPUT:
;    None.
;
;   OUTPUT:
;    err: error code.
;
;   KEYWORDS:
;    ENABLE:    enable the interupt 
;    DISABLE:   disable the interupt
;
;   HISTORY:
;    21 Mar 2007
;       Written by Daniela Zanotti (DZ) and Marco Xompero(MX)
;       Osservatorio Astrofisico di Arcetri, ITALY
;       zanotti@arcetri.astro.it
;    20 April 2007 DZ & Fabio Tosetti(FT)
;          Substituted the call_external with the call to the C function. 
;-

Function manage_interrupt, ENABLE=enable, DISABLE=disable

    @adsec_common
; read the wave frequency
 if ~!AO_CONST.side_master then return, adsec_error.IDL_NOT_SIDE_MASTER

    freq_wave = adsec.wave_freq
    bcu_fwave= ulong(106.25d6*2/7/freq_wave);
 
;Dummy case
    if sc.host_comm eq "Dummy" then begin
        err = adsec_error.ok
    endif else begin

; bcu master GetDiagnValue
        err = who_is_master(bcu_master)
            if err ne adsec_error.ok then return, err 

        case 1 of
    
            keyword_set(ENABLE) : template=(16+bcu_fwave*65536)
            keyword_set(DISABLE): template=ulong(16+0*65536)
            else: begin
                message, "MANAGE_INTERUPT error: at least one keyword must be selected.", CONT=(sc.debug eq 0)
                return, adsec_error.input_type
            end
        endcase

    ;==============================================================
    ;debug printing variables
        datalength = 2L
        dspAddress = 0L
        data = [0ul, template]
        if sc.ll_debug then begin
            print, 'Debugging data'
            help, bcu_master
            print, "MirrCtrl_id ", sc.mirrctrl_id[bcu_master]
            help, datalength
            help, dspAddress
            print, data
            print, 'sc.debug ', sc.debug
            print, 'data ', data

        endif
            print,'!RESET MASK :', data
            ;Standard case
            err = reset_devices_wrap(sc.mirrctrl_id[bcu_master], 255, 255, dspAddress, datalength, data)
            if err ne adsec_error.ok then return, err
        endelse
    
    return, err

end

