; $Id: send_ctrl_coeff.pro,v 1.6 2004/11/02 16:01:58 marco Exp $
;+
; HISTORY
;   13 July 2004: A. Riccardi
;     re-written to match LBT specifications
;   02 Nov 2004, MX
;     Adsec.all changed in sc.all.
;-
function send_ctrl_coeff_iter, act_list, par, addr0, SAFE_VALUES=use_safe_values

    @adsec_common

    n_act = n_elements(act_list)
    for i_act=0,n_act-1 do begin
       act = act_list[i_act]
       N_per_ccount = adsec.N_per_ccount[act]
       m_per_pcount = adsec.m_per_pcount[act]

      gencoeff, par[i_act], N_per_ccount, m_per_pcount, coeff
       if i_act eq 0 then all_coeff = fltarr(n_elements(coeff),n_act)

       all_coeff[*,i_act] = coeff
    endfor

    if keyword_set(use_safe_values) then all_coeff[*]=0.0

    n_coeff = (size(all_coeff))[1]
    for i_coeff = 0L,n_coeff-1 do begin
       err = write_seq_ch(act_list, addr0+i_coeff*adsec.n_act_per_dsp, all_coeff[i_coeff,*], /CHECK)
       if err ne adsec_error.ok then return, err
    endfor

    return, adsec_error.ok
end



function send_ctrl_coeff, the_act_list, SPEED_ONLY=speed_only, POSERR_ONLY=poserr_only, SAFE_VALUES=use_safe_values

    @adsec_common

    if keyword_set(speed_only)+keyword_set(poserr_only) gt 1 then begin
       message, "SPEED_ONLY and POSERR_ONLY are exclusive keywords." $
              , CONT=(sc.debug eq 0B)
       return, adsec_error.input_type
    endif

    if sc.host_comm ne "Dummy" then begin
        ;; check if the control of some actuator is enabled
        err = check_control(the_act_list, ctrl_status)
        if err ne adsec_error.ok then return, err

        ;; avoid to download the coeffs if ctrl is enabled
        if total(ctrl_status) ne 0 then begin
           message, "Please, disable the control before loading the new ctrl coeffs." $
             , CONT = sc.debug eq 0B
           return, adsec_error.IDL_CLOOP_ACTIVE
        endif
    endif

    if the_act_list[0] eq sc.all_actuators then $
       act_list = indgen(adsec.n_actuators) $
    else $
       act_list=the_act_list


    if not keyword_set(speed_only) then begin
       err = send_ctrl_coeff_iter(act_list, adsec.err_par, dsp_map.pos_coeff, SAFE_VALUES=use_safe_values)
       if err ne adsec_error.ok then return, err
    endif
    if not keyword_set(poserr_only) then begin
       err = send_ctrl_coeff_iter(act_list, adsec.tfl_par, dsp_map.speed_coeff, SAFE_VALUES=use_safe_values)
       if err ne adsec_error.ok then return, err
    endif

end
