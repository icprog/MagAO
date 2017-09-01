; $Id: open_superv.pro,v 1.9 2008/12/22 16:31:13 labot Exp $

;+
;
; OPEN_SUPERV
;
;   This function initialize the communication process with the supervisor.
;   The name is given into the 'sc' structure, into the 'init_gvar.pro' routine.
;
; err = open_superv()
;
; INPUTS
;
; OUTPUTS
;
;   err:             long-int scalar. (see adsec_error in init_gvar.pro)
;
; KEYWORDS
;
; HISTORY:
;
;       Jun 24 2003, Marco Xompero (MX)
;       <marco@arcetri.astro.it>
;
;       13 July 2004: A. Riccardi (AR)
;        default timeout is now set using sc.min_timeout common variable
;       
;       05 Feb 2007: MX
;        Added configuration MirrCtrl ID by GetBcuId function.
;
;-

function open_superv

@adsec_common

    ;ERROR HANDLING
    catch, error_status
    IF Error_status NE 0 THEN BEGIN 
        print, "SE SONO QUI HO FATTO IL CATCH"
        !ERROR_STATE.code = err
        PRINT, 'Error index: ', Error_status 
        PRINT, 'Error message: ', !ERROR_STATE.MSG 
        PRINT, 'ERROR_STATE.code: ', !ERROR_STATE.code
        idl_error_status=1
      ;  catch, /CANCEL
        return, !ERROR_STATE.code
    endif





;=================================================
; checks for the input parameters
;

;check for the feasebility of the connection
;if sc.busy_comm eq 1 then begin
;    err= adsec_error.communication
;    message, "You cannot connect because there are another connection pendent."
;    err= adsec_error.communication
;endif
;set the default value of the timeout
if n_elements(timeout) eq 0 then begin
    timeout=sc.min_timeout
endif else begin
;check the type (must be LONG)
    if test_type(timeout, /LONG, N_EL=n_el) then begin
        message, "timeout must be a long integer", CONT=(sc.debug eq 0B)
        return, adsec_error.input_type
    endif
;check the number of elements (must be one)
    if n_el ne 1 then begin
        message, "timeout must be a scalar", CONT=(sc.debug eq 0B)
        return, adsec_error.input_type
    endif
endelse

;
;**** end of the checks
;==============================================================


if n_elements(sc.mirrctrl_id) lt (adsec.n_bcu+1) then begin
    message, "There are not enough communicator for all the system", CONT=(sc.debug eq 0B)
    return, err
endif

if sc.host_comm eq "Dummy" then begin
    err = adsec_error.ok
    sc.mirrctrl_id = indgen(adsec.n_bcu+1)
    print, "DUMMY COMMUNICATION ENABLED"
endif else begin
    err = idl_setup_wrap(sc.name_comm)
    if err eq adsec_error.ok then sc.busy_comm =1b $
    else begin
        message, "adsec_comm error: "+strtrim(err,2),CONT=(sc.debug eq 0B)
        print, "SE SONO QUI NON HO FATTO IL CATCH"
        return, err
    endelse

    err = getBcuId(idb)
    if err ne adsec_error.ok then return, err
    err = getBcuId(ids, /SWITCHB)
    if err ne adsec_error.ok then return, err

    err = getBcuIp(ipb)
    if err ne adsec_error.ok then return, err
    err = getBcuIp(ips, /SWITCHB)
    if err ne adsec_error.ok then return, err

    sc.busy_comm =1b 
    sc.mirrctrl_id[0: adsec.n_crates] = [idb, ids]
    sc.bcu_ip[0: adsec.n_crates] = [ipb, ips]
endelse

return, err

end
