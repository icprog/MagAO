;$Id: disable_watchdog.pro,v 1.2 2007/11/05 09:56:16 marco Exp $   
;+
;   NAME:
;    DISABLE_WATCHDOG
;   
;   PURPOSE:
;    This routine disable the ethernet watchdog or the DSP watchdog. 
;   
;   USAGE:
;    err = disable_watchdog()
;    
;   INPUT:
;    None.
;
;   OUTPUT:
;    err: error code.
;
;   KEYWORDS:
;    DSP:   disable DSP watchdog.
;
; HISTORY
;   3 Mar 2007
;       Written by Marco Xompero (MX)
;       Osservatorio Astrofisico di Arcetri, ITALY
;       marco@arcetri.astro.it
;   5 Nov 2007, MX
;       Updated with DSP watchdog.
;-

Function disable_watchdog, DSP=dsp

    @adsec_common

    For i=0, adsec.n_bcu-1 do Begin
        
        err = manage_watchdog(i,/DISABLE, DSP=dsp)
        if err ne adsec_error.ok then message, 'WatchDog not activated!!!!', cont = (sc.debug eq 1)
        
    endFor

    return, err

End
