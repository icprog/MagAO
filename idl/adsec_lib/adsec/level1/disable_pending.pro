;$Id: disable_pending.pro,v 1.1 2007/04/11 13:13:49 labot Exp $$
;+
;   NAME:
;    DISABLE_PENDING
;
;   PURPOSE:
;    This routine disable the pending command into switch bcu.
;
;   USAGE:
;    err = disable_pending()
;
;   INPUT:
;    None.
;
;   OUTPUT:
;    err: error code.
;
;   KEYWORDS:
;    None.
;
; HISTORY
;   3 April 2007
;       Written by Marco Xompero (MX)
;       Osservatorio Astrofisico di Arcetri, ITALY
;       marco@arcetri.astro.it
;-

function disable_pending
    
@adsec_common
    err = write_same_dsp(0, switch_map.ENABLECMDPENDING, 0l, /SWITCH, /CHECK)

    return, err


end
