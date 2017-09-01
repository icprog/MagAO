;$Id: start_dl.pro,v 1.1 2007/02/07 17:00:25 marco Exp $$
;+
;   NAME:
;    START_DL
;
;   PURPOSE:
;    Triggering the real CMD and FORCE application, diagnostic collecting and storage. The bit#0
;    set to 1 generates the effective update of commands and currents, the bit#1 generates the
;    collecting and download of diagnostic data.
;
;   USAGE:
;    err = START_DL([/DIAGNOSTIC])
;
;   INPUT:
;    None.
;
;   OUTPUT:
;    Error code.
;
;   KEYWORDS:
;    DIAGNOSTIC: It sets only bit#1 to collect and download diagnostic data.
;
;   HISTORY:
;    Created by Marco Xompero (MX) on 26 Jan 2007 
;    marco@arcetri.astro.it
;-

Function start_dl, DIAGNOSTIC=diagnostic

    @adsec_common

    word = 1L
    if ~keyword_set(DIAGNOSTIC) then begin
        word += 2L
    endif
    err = write_same_dsp(sc.all_dsp, rtr_map.start_dl, word)

    return, err

end
