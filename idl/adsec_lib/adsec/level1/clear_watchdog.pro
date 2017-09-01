;$Id: clear_watchdog.pro,v 1.2 2008/02/05 09:59:33 labot Exp $$
;+
;   NAME:
;    CLEAR_WATCHDOG
;   
;   PURPOSE:
;    This routine reset the ethernet watchdog BCUs internal counter. If the internal counter on BCU goes in timeout, the coils are disabled.
;   
;   USAGE:
;    err = clear_watchdog()
;    
;   INPUT:
;    None.
;
;   KEYWORDS:
;    None.
;
; HISTORY
;   3 Mar 2007
;       Written by Marco Xompero (MX)
;       Osservatorio Astrofisico di Arcetri, ITALY
;       marco@arcetri.astro.it
;-

Function clear_watchdog

    @adsec_common

    For i=0, adsec.n_bcu -1 do Begin
        
        err = manage_watchdog(i, /CLEAR)
        ;if err ne adsec_error.ok then message, 'WatchDog not activated!!!!', cont = (sc.debug eq 1)
        
    endFor

    return, err

End
