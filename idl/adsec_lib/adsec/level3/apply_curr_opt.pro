; $Id: apply_curr_opt.pro,v 1.11 2007/11/09 14:45:07 marco Exp $
;+
;   NAME:
;    APPLY_CURR_OPT
;   
;   PURPOSE:
;    Control force spreading in order to mantain total moment and force.
;   
;   USAGE:
;    err = apply_curr_opt(ACT_LIST=act_list, N_ITER=n_iter, XADSEC=xadsec, OFFLOAD=offload)
;
;   INPUT:
;    None.
;
;   OUTPUT:
;    err: error code
;
;   KEYWORDS:
;    ACT_LIST:  actuators list considering for the optimization; dafault adsec.act_w_curr
;    N_ITER:    number of iteration in which apply the force into the bias
;    XADSEC:    update engineering GUI
;    OFFLOAD:   use command offload in SWITCH BCU 
;
;   HISTORY
;
;    Written by G. Brusa and A. Riccardi (AR)
;    Osservatorio Asrofisico di Arcetri, ITALY
;    <riccardi@arcetri.astro.it>
;
;    13 Mar 2002, AR
;       Bug fixed in the CONT keyword setting of the MESSAGE call.
;    04 Feb 2004, AR and M.Xompero(MX)
;       Bias magnet supported.
;    26 Jul 2004, AR and D.Zanotti(DZ) 
;       Changed bal_operating_currents from int to float
;    02 Nov 2004, MX
;       Adsec.all changed in sc.all.
;    13 Jul 2007, MX
;       Correct time_ps replace 0.1 in wait instruction.
;    04 Nov 2007, MX
;       Keyword OFFLOAD added.
;-
function apply_curr_opt, ACT_LIST=act_list, N_ITER=n_iter, XADSEC=xadsec, OFFLOAD=offload

    @adsec_common
    time_ps = time_preshaper(/FF)
    if n_elements(act_list) eq 0 then begin
        act_list=adsec.act_w_curr
    endif else begin
        if check_channel(act_list) then begin
            message,"Wrong actuator list",cont=(sc.debug eq 0B)
            return,adsec_error.input_type
        endif
        if complement(act_list,adsec.act_w_curr,dummy) then begin
            message,"act_list must be included in adsec.act_w_pos list"
            return,adsec_error.input_type
        endif
    endelse

    if n_elements(n_iter) eq 0 then begin
        n_iter=3
    endif else begin

        if test_type(n_iter,/int,/long,N_EL=n_el) then begin
            message,"n_iter must be integer",cont=(sc.debug eq 0B)
            return,adsec_error.input_type
        endif
        if n_el ne 1 then begin
            message,"n_iter must be scalar",cont=(sc.debug eq 0B)
            return,adsec_error.input_type
        endif

    endelse

    ovs = rtr.oversampling_time
    if ovs eq 0 then ovs = 0.05

    for i=1,n_iter do begin

        wait, 2*ovs > 0.1
        err = update_status()
        if err ne adsec_error.ok then return,err
        ;f_bal = optimize_moment(sys_status.current[act_list]-(adsec.curr4bias_mag[act_list]*adsec.weight_curr), $
        ;             act_list) + (adsec.curr4bias_mag[act_list] * adsec.weight_curr)
       ; f_bal = sys_status.current[act_list]
        f_bal = optimize_moment(sys_status.current[act_list], act_list) 
        bal_operating_currents = fltarr(adsec.n_actuators)
        bal_operating_currents[act_list] = f_bal
        if keyword_set(OFFLOAD) then begin
            ;nuove righe
            err = get_commands(adsec.act_w_pos,cmd, /offloadcmd)
            if err ne adsec_error.ok then return,err
            err = set_offload_cmd(cmd, /switch, /NOCHECK)
            if err ne adsec_error.ok then return,err   
            err = set_offload_force(bal_operating_currents[adsec.act_w_curr], /SWITCH, /START)
            if err ne adsec_error.ok then return,err
        endif else begin
            err = write_bias_curr(sc.all_actuators, bal_operating_currents)
            if err ne adsec_error.ok then return,err
        endelse
        wait, 2*time_ps > (2*rtr.oversampling_time)
        err = update_status()
        if err ne adsec_error.ok then return, err

        if keyword_set(XADSEC) then update_panels, /OPT, /NO_READ

    endfor

    return,adsec_error.ok
end
