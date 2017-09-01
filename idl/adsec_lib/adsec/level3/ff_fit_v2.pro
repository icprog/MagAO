;$Id: ff_fit_v2.pro,v 1.3 2008/02/05 10:19:22 labot Exp $$
;+
;   NAME:
;    FF_FIT_v2
;
;   PURPOSE:
;    This function provide the feed forward matrix fitting given by a set of measurements 
;    with a best least-square method.
;
;   USAGE:
;    err = ff_fit(data, samples, ff_matrix, ff_u, ff_w, ff_v, df, dx, dc,$
;                 GAIN=g, SINGULAR=sv, SIGMA=sigma, CHISQ=chisq, COVAR=covar_x,$ 
;                 CL_ACT=cl_act, NO_MATCH_ORDERING=no_match_ordering
;
;   INPUT:
;    data:
;    samples:   number of samples
;   
;
;   OUTPUT:
;    err:       error code.
;    ff_matrix: feed forward matrix fitted.
;    ff_u:      feed forward matrix left matrix for singular values decomposition.
;    ff_w:      feed forward matrix diagonal matrix with singular values.
;    ff_v:      feed forward matrix right matrix for singular values decomposition.
;    df:        delta current vector between positive and negative movement of the shell.
;    dx:        delta position vector between positive and negative movement of the shell.
;    dc:        delta command vector between positive and negative movement of the shell.
;
;   KEYWORD:
;    GAIN:              returns the fitted loop proportional gain for each actuator.
;    SINGULAR:          Contains the number of singular values equal to zero.
;    SIGMA:             returns the standard deviation for the fitted matrix elements.
;    CHISQ:             returns the chi-square vale for the 2D fitting.
;    COVAR:             This matrix is the covariance matrix used for the fitting of the
;                       feed forward matrix. This keyword MUST BE SPECIFIED.
;    CL_ACT:            if given, the function compute the feed forward matrix
;                       on this set of actuators on closed-loop.
;    NO_MATCH_ORDERING: disable the final mode reordering.
;
;   NOTE:
;    This routine suppose the set of data with a gaussian distribution. It performs
;    the maximization of the probability to obtain the given set of measurements
;    by minimizing the exponent of the gaussian probability distribution. But we have 
;    a correlation given by the proportional loop gain between the forces and the positions
;    so we perform a fit for each actuator loop gain and a fit of the matrix the connect
;    the positions to the command from which retrieve the feed forward matrix.
;
;   =================================================================
;    WARNING: this routine uses the data collected by ff_data_acquisition and
;    works only with data taken using the same pure proportional control low
;    for all the actuators in closed loop and the same list of actuators in closed loop.
;    Moreover the actuators in open loop can apply only a bias current that has not
;    changed during the acquisition. We report below the prototype of a data vector 
;    element:
;
;    data_proto={dcommand: intarr(adsec.n_actuators), $
;                position: fltarr(adsec.n_actuators), $
;                rms_pos:  intarr(adsec.n_actuators), $
;                rms_curr:  intarr(adsec.n_actuators), $
;                current: fltarr(adsec.n_actuators), $
;                ff_current: intarr(adsec.n_actuators)}
;   
;    Each measure is got in application of a positive and negative command.
;    The bias current is supposed to be constant, is passed but not used.
;    For more informations cfr. ff_data_acquisition.pro
;   ================================================================
;
;   HISTORY
;    Written by M.Xompero on 5 Jun 2007, fixing ff_fit.pro routine.
;    Osservatorio Asrofisico di Arcetri, ITALY
;    <marco@arcetri.astro.it>
;-

Function ff_fit_v2, data, samples, ff_matrix, ff_u, ff_w, ff_v, df, dx, dc                  $
                  , GAIN=g, BIAS_CURRENT=bias_current                                       $
                  , SINGULAR=sv, SIGMA=sigma, CHISQ=chisq, COVAR=covar_x, CL_ACT=cl_act     $
                  , NO_MATCH_ORDERING=no_match_ordering, FILTEROUT=filterout

@adsec_common

;Check on the closed loop actuators number.
if n_elements(cl_act) eq 0 then cl_act = adsec.act_w_cl
n_cl_act = n_elements(cl_act)

if n_cl_act eq adsec.n_actuators then begin
    n_ol_act = 0
endif else begin
    err=complement(cl_act, indgen(adsec.n_actuators), ol_act)
    if err ne adsec_error.ok then message, "Unexpected error"
    n_ol_act = n_elements(ol_act)
endelse

if n_elements(g) eq 0 then message, "The gain must be provided."

;reorganization data
ndata=n_elements(data)
dcommand=transpose(data.dcommand)
position=transpose(data.position)
rms_pos=transpose(data.rms_pos)
rms_current=transpose(data.rms_curr)
ff_current=transpose(data.ff_current)
current=transpose(data.current)
;ctrl_current=transpose(data.current) - ff_current

;index for positive and negative movement of the shell
idx_pos = 2*lindgen((ndata-2)/2)+1
idx_neg = idx_pos+1
undefine, data

;absolute delta movement calcolus
;delta_ctrl_current=ctrl_current[idx_pos,*]-ctrl_current[idx_neg,*]
delta_ff_current=temporary(ff_current[idx_pos,*]-ff_current[idx_neg,*])
delta_position=temporary(position[idx_pos,*]-position[idx_neg,*])
delta_command=temporary(dcommand[idx_pos,*]-dcommand[idx_neg,*])
delta_current = temporary(current[idx_pos,*]-current[idx_neg,*])

;Checks on the goodness of the measurements
;if (n_ol_act ne 0) and (sc.name_comm ne "Dummy") then begin
if (n_ol_act ne 0) and (sc.name_comm ne "Dummy") then begin

    if max(abs(delta_ff_current[*,ol_act])) ne 0.0 then begin
        message, "The acts. in open loop cannot change their ff current" $
            ,CONT=(sc.debug eq 0B)
        return, adsec.generic_error
    endif
endif

;only close loop actuators considered for the ff matrix fitting
    ;delta_ctrl_current=delta_ctrl_current[*,cl_act]
    delta_ff_current=delta_ff_current[*,cl_act]
    delta_position=delta_position[*,cl_act]
    delta_command=delta_command[*,cl_act]
    delta_current = delta_current[*,cl_act]

if n_elements(filterout) gt 0 then begin
   err = complement(filterout, indgen(n_elements(delta_position[*,0])), measlist)
   if err ne 0 then message, "wrong filter list provided!!"
endif else measlist = indgen(n_elements(delta_position[*,0]))


dx = temporary(delta_position[measlist, *])
dc = temporary(delta_command[measlist, *])

;## nuova def di df (output)
df = temporary(delta_current[measlist, *]) ;- rebin(bias_current, n_cl_act , n_elements(delta_current), /samp)
dff = temporary(delta_ff_current[measlist, *]) ;- rebin(bias_current, n_cl_act , n_elements(delta_current), /samp)

;check on the covar matrix 

if n_elements(covar_x) eq 0 then begin
     message, "A Covariance matrix MUST BE PASSED by COVAR keyword."   
endif else begin
    s = size(covar_x)
    if total(s[0:2] ne [2,adsec.n_actuators,adsec.n_actuators]) ne 0 then $
        message, "Covar matrix has a wrong format"
endelse

;covariance matrix only for close loop actuators
if n_cl_act ne adsec.n_actuators then begin
    sub_covar_x = covar_x[cl_act,*]
    sub_covar_x = sub_covar_x[*,cl_act]
endif else sub_covar_x = covar_x

;if sc.name_comm eq "Dummy" then begin
;    sub_covar_x = dx ## transpose(dx)
;endif

;Matrix fitting of (G-F)=A and G*Dc-Dff-Dbias-DIntCTRLCurr = A ## Dx
;Dbias and DIntCTRLCurr are supposed be null !!!!!!!!!!!!!!!
n_meas = n_elements(dc[*,0])

lin2d_fitxy, dx, rebin(transpose(g),n_meas,n_cl_act)*dc-dff, A $
                        , SINGULAR=sv, /DOUBLE, COVAR_X=sub_covar_x*2.0, SIGMA=sigma, $
                        CHISQ=chisq, EPS=1d-5, CONVERGED=conv
if ~conv and (sc.name_comm ne "Dummy") then begin
    message, "The fitting of the gain of the loop doesn't converge", $
             CONT=(sc.debug eq 0B)
    return, adsec_error.fitting_fail
endif

g_mat=dblarr(n_cl_act,n_cl_act)
set_diagonal, g_mat, g
ff_matrix=g_mat-A

svdc, ff_matrix, ff_w, ff_u, ff_v, /DOUBLE

idx = sort(ff_w)
ff_w = ff_w[idx]
ff_v = ff_v[idx, *]
ff_u = ff_u[idx, *]

if adsec.meas_ff_used and (not keyword_set(no_match_ordering)) then begin
    if total(adsec.act_w_cl ne cl_act) ne 0 then begin
        message, "Actuator list for the old FF mismatch the actual list. No mode reordering.", /INFO
    endif else begin

        old_modes = adsec.ff_p_svec[cl_act, *]
        old_modes = old_modes[*, cl_act]

        p_coeff = transpose(old_modes) ## ff_v

        v_ord = lonarr(n_cl_act)
        negative = bytarr(n_cl_act)
        new_idx_to_use = lindgen(n_cl_act)
        threshold = 1.0/sqrt(2.0)

        for i=0,n_cl_act-1 do begin
            idx_to_use = new_idx_to_use
            max_p_coeff = max(abs(p_coeff[i,idx_to_use]), idx)
            idx_of_max = idx_to_use[idx]
            if max_p_coeff lt threshold then begin
                message, "WARNING: the ordering of modes may be affected by error. "+strtrim(idx_of_max,2), /CONT
            endif
            v_ord[i] = idx_of_max
            if p_coeff[i,idx_of_max] lt 0 then negative[i]=1B
            dummy = complement(idx_of_max, idx_to_use, new_idx_to_use, count)
        endfor

        idx = where(negative eq 1B, count)
        if count ne 0 then begin
            ff_v[idx, *] = -ff_v[idx, *]
            ff_u[idx, *] = -ff_u[idx, *]
        endif

        idx = sort(v_ord)
        ff_w = ff_w[idx]
        ff_v = ff_v[idx, *]
        ff_u = ff_u[idx, *]
    endelse
endif

return, adsec_error.ok

end

