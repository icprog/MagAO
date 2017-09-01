; $Id: test_coils.pro,v 1.7 2007/12/03 10:11:21 labot Exp $
;+
;   NAME:
;    TEST_COILS
;   
;   PURPOSE:
;    This routine test the coils status in the Adaptive Secondary. It returns NO_ERROR if all coils are enabled.
;   
;   USAGE:
;    err = test_coils()
;    
;   INPUT:
;    None.
;
;   KEYWORDS:
;    DISABLE:    if set, the routine returns NO_ERROR if all coils are disabled.
;
;   OUTPUT:
;    err: error code.
;
;   HISTORY
;    14 Oct 2005
;       Written by Marco Xompero (MX)
;       Osservatorio Astrofisico di Arcetri, ITALY
;       marco@arcetri.astro.it
;    3 May 2007
;       Modified by Daniela Zanotti(DZ)
;       The coils status was read from HouseKeeper by the GetDiagnValue function.
;    4 Nov 2007 , MX
;    Added test on BusPowerFault bit.
;       
;-

Function test_coils, status, DISABLED=disabled


    @adsec_common
   
    slow_diag_name = process_list.housekeeper.msgd_name
    
    err = GetDiagnValue(slow_diag_name, "DSPDriverStatusBusPowerFault", -1, -1, bpf, TIM=5)
    if err ne adsec_error.ok then return, err

    err = GetDiagnValue(slow_diag_name, "DSPDriverStatusBusDriverEnable", -1, -1, mush_enable, TIM=5)
    if err ne adsec_error.ok then return, err
     
    err = GetDiagnValue(slow_diag_name, "DSPDriverStatusDriverEnable", -1, -1, coils_enable_st, TIM=5)
    if err ne adsec_error.ok then return, err
     
    mush_status = total(mush_enable.last) gt 0

    current = intarr(adsec.n_actuators)

    act_list = indgen(adsec.n_actuators)
    current[act_list] = 1
    coils_enable = reform(coils_enable_st.last)
    bus_fault = reform(bpf.last)

    coils_status = total(coils_enable[adsec.act_w_curr] eq current[adsec.act_w_curr]) eq n_elements(adsec.act_w_curr)
    bus_status =  total(bus_fault[adsec.act_w_curr] eq current[adsec.act_w_curr]) eq n_elements(adsec.act_w_curr)

    print, bus_status
    err = complement(indgen(adsec.n_actuators), adsec.act_w_curr, act_wo_curr)
    if err eq 0 and n_elements(act_wo_curr) gt 0 then begin
        coils_status_wo_curr = total(coils_enable[act_wo_curr] eq current[adsec.act_wo_curr])
        if coils_status_wo_curr gt 0 then message, "Warning: driver enabled in "+strtrim(coils_status_without_curr)+"actuators without curr", /info
        endif
    
    if keyword_set(disabled) then begin

        status =  ~(mush_status and coils_status)
        if status then begin
            return, adsec_error.ok
        endif else begin
            message, 'The coils are not disabled. Returning...', /INFO   ;CONT=(sc.debug eq 0)
            return, adsec_error.IDL_UNEXP_COIL_STATUS
        endelse

    endif else begin

        status =  mush_status and coils_status 
        if status then begin
            return, adsec_error.ok
        endif else begin
            message, 'The coils are not enabled. Returning...', /INFO   ;, CONT=(sc.debug eq 0)
            return, adsec_error.IDL_UNEXP_COIL_STATUS
        endelse


    endelse
    

End
