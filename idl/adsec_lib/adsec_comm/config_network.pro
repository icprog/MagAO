; $Id: config_network.pro,v 1.1 2009/04/10 13:49:42 labot Exp $$

;+
;
; CONFIG_NETWORK
;
;   This function initialize the communication process with the unit throught the supervisor.
;
; USAGE:
;
; err = config_network()
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

function config_network

    @adsec_common

    ;ERROR HANDLING
    catch, error_status
    IF Error_status NE 0 THEN BEGIN 
        print, "CONFIG_NETWORK"
        PRINT, 'Error index: ', Error_status 
        PRINT, 'Error message: ', !ERROR_STATE.MSG 
        PRINT, 'ERROR_STATE.code: ', !ERROR_STATE.code
        idl_error_status=1
        return, !ERROR_STATE.code
    endif


    if n_elements(sc.mirrctrl_id) lt (adsec.n_bcu+1) then begin
        message, "There are not enough communicator for all the system", CONT=(sc.debug eq 0B)
        return, err
    endif

    if sc.host_comm eq "Dummy" then begin
        err = adsec_error.ok
        sc.mirrctrl_id = indgen(adsec.n_bcu+1)
        print, "DUMMY NETWORK ENABLED"
    endif else begin
        err = getBcuId(idb)
        if err ne adsec_error.ok then return, err
        err = getBcuId(ids, /SWITCHB)
        if err ne adsec_error.ok then return, err

        err = getBcuIp(ipb)
        if err ne adsec_error.ok then return, err
        err = getBcuIp(ips, /SWITCHB)
        if err ne adsec_error.ok then return, err

        sc.mirrctrl_id[0: adsec.n_crates] = [idb, ids]
        sc.bcu_ip[0: adsec.n_crates] = [ipb, ips]
    endelse

    return, err

end
