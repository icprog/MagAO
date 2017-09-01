;+
; INIT_RTR_MAP
;
;  Initialize rtr_map structure.
;
;   INPUT:
;       None.
;   OUTPUT:
;    Error code.
;
; HISTORY
;
;  created by D.Zanotti.
;  Osservatorio Astrofisico di Arcetri, ITALY
;  <zanotti@arcetri.astro.it>
;
;   03 Aug 2005
;    SAVEFILE keyword added. DZ
;
;
;-
function init_rtr_map, FILENAME=filename, SAVEFILE=savefile

    @adsec_common

    if (keyword_set(filename)) then begin
           rtr_map = read_ascii_structure(filename, DEBUG=0)
           if n_elements(rtr_map) eq 0 then begin
                message, 'The configuration file "'+filename+'" does not exist or has a wrong format.'
           endif

    endif else begin
        err=rtr_dsp_map(ST=rtr_st, SAV=savefile)
        if err ne adsec_error.ok then message,"The rtr structure was not initialized"
        rtr_map=rtr_st
    endelse

    return, adsec_error.ok
end
