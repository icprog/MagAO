;$Id: enable_pending.pro,v 1.2 2007/06/05 08:52:43 labot Exp $$
;+
;   NAME:
;    ENABLE_PENDING
;
;   PURPOSE:
;    This routine enable the pending command into switch bcu.
;
;   USAGE:
;    err = enable_pending(IS_PENDING=is_pending)
;
;   INPUT:
;    None.
;
;   OUTPUT:
;    err: error code.
;
;   KEYWORDS:
;    IS_PENDIG: variable filled with the current status of pending flag.
;
;   HISTORY
;    3 April 2007
;    Written by Marco Xompero (MX)
;    Osservatorio Astrofisico di Arcetri, ITALY
;    marco@arcetri.astro.it
;    5 Jun 2007, MX
;       IS_PENDING keyword added.
;-

Function enable_pending, IS_PENDING=is_pending

    @adsec_common

    err = read_seq_dsp(0, switch_map.EnableCmdPending, 1L, is_pending, /ULONG, /SWITCH)
    if err ne adsec_error.ok then return, err
    err = write_same_dsp(0, switch_map.EnableCmdPending, 1l, /SWITCH, /CHECK)
    return,err


End
