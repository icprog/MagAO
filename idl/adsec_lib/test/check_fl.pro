;$Id: check_fl.pro,v 1.2 2007/02/07 17:23:10 marco Exp $$
;+
; NAME:
;   CHECK_FL
;
; PURPOSE:
;   Script to check the status of the FastLink connection (working or not).
;
; CATEGORY:
;   Supervisor IDL function.
;
; CALLING SEQUENCE:
;   err = check_fl(status)
;
; INPUTS:
;   None.
;
; KEYWORD PARAMETERS:
;   None.
; OUTPUTS:
;   err : Error code.
;   status: boolean variable returned with the status of the FL. 1 means active, 0 means inactive.
;
; RESTRICTIONS:
;   None.
;
; MODIFICATION HISTORY:
;   13 Gen 2005
;    Created by Marco Xompero (MX)
;    marco@arcetri.astro.it
;-

Function check_fl, status

    @adsec_common
    ;read the param_selector
    err = read_seq_dsp(sc.all, rtr_map.param_selector, 1l, pbs, /UL)
    if err ne adsec_error.ok then begin
        print, "Error occurred. FastLink connection not tested."
        return, err
    endif

    ;set the bit 1,2,3 and rewrite into DSPs
    pbs_new = pbs or 14UL

    ;write and read again the new value of param_selector
    err = write_seq_dsp(sc.all, rtr_map.param_selector, pbs_new)
    if err ne adsec_error.ok then begin
        print, "Error occurred. FastLink connection not tested."
        return, err
    endif

    wait, 1
    err = read_seq_dsp(sc.all, rtr_map.param_selector, 1l, pbs_read, /UL)
    if err ne adsec_error.ok then begin
        print, "Error occurred. FastLink connection not tested."
        return, err
    endif
    ;print, [conv2bin(pbs), conv2bin(pbs_new) , conv2bin(pbs_read)]

    ;if changed, status=1, else status=0
    if total(abs(pbs_read-pbs_new) eq 0) then status=0B else status=1B
    if status eq 0B then begin
         err = write_seq_dsp(sc.all, rtr_map.param_selector, pbs, /CHECK)
         if err ne adsec_error.ok then begin
             print, "Error occurred. FastLink connection not tested."
             return, err
         endif
    endif
    return, adsec_error.ok

End
