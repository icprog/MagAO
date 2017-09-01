; $Id: tfl_gui.pro,v 1.3 2003/06/10 18:32:24 riccardi Exp $
;
;+
; NAME:
;       tfl_gui
;
; PURPOSE:
;       tfl_gui generates the Graphical User Interface (GUI) for
;       setting the parameters of the Time FiLtering (tfl) module.
;       A parameter file called tfl_nnnnn.sav is created, where nnnnn
;       is the number n_module associated to the module instance.
;       The file is stored in the project directory proj_name located
;       in the working directory.
;
; CATEGORY:
;       Graghical User Interface (GUI) program
;
; CALLING SEQUENCE:
;       error = tfl_gui(n_module, proj_name)
;
; INPUTS:
;       n_module:   integer scalar. Number associated to the intance
;                   of the TFL module. n_module > 0.
;       proj_name:  string. Name of the current project.
;
; OUTPUTS:
;       error: long scalar, error code (see adsec_error var in init_gvar.pro).
;
; COMMON BLOCKS:
;       common error_block, error
;       common tfl_gui_block, par, old_par
;
; MODIFICATION HISTORY:
;       program written: 08/03/1999, A. Riccardi (OAA),
;                        <riccardi@arcetri.astro.it>.
;   13 Mar 2002, AR
;     Bug fixed in the CONT keyword setting of the MESSAGE call.
;-

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; convert real and imagnary part of a complex
; number into a string
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
function complex2str, re, im, FORMAT=format

str = string(re, FORMAT=format)
idx = where(im ne 0d0, count)
if count ne 0 then str[idx]=str[idx]+" + j*"+string(im[idx], FORMAT=format)
return, str

end

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; compute the GZP representation of the filter
; starting from the PID one. Update the related
; field in the state variable
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
function tfl_update_pid, state, GROUP=group

@adsec_common

widget_control, state.id.prop_fld, GET_VALUE=kp
widget_control, state.id.integ_fld, GET_VALUE=ki
widget_control, state.id.deriv_fld, GET_VALUE=kd
widget_control, state.id.lowpass_fld, GET_VALUE=A
kp = double(kp)
ki = double(ki)
kd = double(kd)
A  = double(A)

if kp lt 0.0 then begin
    dummy = dialog_message("The proportional gain cannot "+ $
                           "be negative.", $
                           DIALOG_PARENT = group, $
                           TITLE = 'PID error', /ERROR)
    return, -1L
endif
if ki lt 0.0 then begin
    dummy = dialog_message("The integrative gain cannot "+ $
                           "be negative.", $
                           DIALOG_PARENT = group, $
                           TITLE = 'PID error', /ERROR)
    return, -1L
endif
if kd lt 0.0 then begin
    dummy = dialog_message("The derivative gain cannot "+ $
                           "be negative.", $
                           DIALOG_PARENT = group, $
                           TITLE = 'PID error', /ERROR)
    return, -1L
endif
if (kd gt 0.0) and (A le 0.0) then begin
    dummy = dialog_message("The low-pass cut cannot "+ $
                           "be less or equal to zero.", $
                           DIALOG_PARENT = group, $
                           TITLE = 'PID error', /ERROR)
    return, -1L
endif

pid2gzp, kp, ki, kd, A, gain, z, p, N_Z=nz, N_P=np

dof = state.active_dof
state.par[1, dof].s_const = gain
if nz gt 0 then state.par[1, dof].s_zero = z else state.par[1, dof].s_zero=0
if np gt 0 then state.par[1, dof].s_pole = p else state.par[1, dof].s_pole=0
state.par[1, dof].n_s_zero = nz
state.par[1, dof].n_s_pole = np

return, ADSEC_ERROR.OK
end


;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; Update the string field related to the zeros of
; the filter
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
function tfl_zero2str, par

if par.n_s_zero eq 0 then begin
    return, 'No Zeros'
endif else begin
    re = double(par.s_zero[0:par.n_s_zero-1])
    im = imaginary(par.s_zero[0:par.n_s_zero-1])
    str = complex2str(re, im)
    str = 'z['+strtrim(indgen(par.n_s_zero), 2)+'] = '+str
    return, str
endelse

end

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; Update the string field related to the poles of
; the filter
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
function tfl_pole2str, par

if par.n_s_pole eq 0 then begin
    return, 'No Poles'
endif else begin
    re = double(par.s_pole[0:par.n_s_pole-1])
    im = imaginary(par.s_pole[0:par.n_s_pole-1])
    str = complex2str(re, im)
    str = 'p['+strtrim(indgen(par.n_s_pole), 2)+'] = '+str
    return, str
endelse

end

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; Update the string filed where the digital filter
; is shown
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
function tfl_z_coeff2str, z_par

str = 'out(t)='
if z_par.n_z_num gt 0 then begin
    str = str+strtrim(z_par.z_num_coeff[0],2)+'*in(t)'
    for k=1,z_par.n_z_num-1 do begin
        coeff = z_par.z_num_coeff[k]
        if coeff ne 0 then begin
            if coeff gt 0 then sig = '+' else sig = ''
            if k eq 1 then kk='' else kk=strtrim(k,2)
            str = str+sig+strtrim(coeff,2)+'*in(t-'+kk+'T)'
        endif
    endfor
endif
str1 = '       '
for k=1,z_par.n_z_den-1 do begin
    coeff = -z_par.z_den_coeff[k]
    if coeff ne 0 then begin
        if coeff gt 0 then sig = '+' else sig = ''
        if k eq 1 then kk='' else kk=strtrim(k,2)
        str1 = str1+sig+strtrim(coeff,2)+'*out(t-'+kk+'T)'
    endif
endfor

return, [str, str1]
end


;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; Update the digital filter and redraw the franfer
; function
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
pro tfl_draw_filter, state

@adsec_common

dof  = state.active_dof
type = state.active_type[dof]

if state.par[type, dof].s_const[0] eq 0.0 then begin
    wset, state.num.amp
    erase
    xyouts, !D.X_SIZE/2, !D.Y_SIZE/2, "TF(s)=0: zero-gain filter.", $
      /DEVICE, CHARSIZE=2.0, ALIGN=0.5
    wset, state.num.phase
    erase
    return
endif

n_s_zero = state.par[type, dof].n_s_zero
if n_s_zero gt 0 then begin
    zeros = state.par[type, dof].s_zero[0:n_s_zero-1]
    s_num = zero2coeff(zeros)
endif else begin
    s_num = [1d0, 0d0]
endelse

n_s_pole = state.par[type, dof].n_s_pole
if n_s_pole gt 0 then begin
    poles = state.par[type, dof].s_pole[0:n_s_pole-1]
    s_den = zero2coeff(poles)
endif else begin
    s_den = [1d0, 0d0]
endelse

n_z_num = state.z_par[type].n_z_num
if n_z_num gt 0 then begin
    z_num = state.z_par[type].z_num_coeff[0:n_z_num-1]
    if n_elements(z_num) eq 1 then z_num=[z_num, 0d0]
endif else begin
    z_num = [1d0, 0d0]
endelse

n_z_den = state.z_par[type].n_z_den
if n_z_den gt 0 then begin
    z_den = state.z_par[type].z_den_coeff[0:n_z_den-1]
    if n_elements(z_den) eq 1 then z_den=[z_den, 0d0]
endif else begin
    z_den = [1d0, 0d0]
endelse

np = 512

iu = dcomplex(0d0, 1d0)         ; imaginary unit
f0 = 1d0/adsec.sampling_time ; sampling frequency [Hz]
freq_res = 1d0                  ; [Hz]
; frequency vector
f_vec = mk_vector(np, freq_res, 0.5d0*f0, /DOUBLE, /LOG)
tf = poly(iu*2d0*!DPI*f_vec, [s_num])
tf = tf/poly(iu*2d0*!DPI*f_vec, [s_den])
tf = state.par[type, dof].s_const[0] * tf

den0 = poly(0d0, [s_den])
gain0 = state.par[type, dof].s_const[0]
if den0 ne 0d0 then begin
    fdc = 0d0
    gain0 = abs(gain0 * poly(0d0, [s_num]) / den0)
    title_amp = "DC gain (TF@0Hz)="+strtrim(string(gain0,FORMAT="(G10.1)"),2)
endif else begin
    fdc = freq_res
    gain0 = abs(gain0 * poly(iu*2d0*!DPI*fdc, [s_num]) / poly(iu*2d0*!DPI*fdc, [s_den]))
    title_amp = "Amp.@"+strtrim(string(fdc, FORMAT="(G8.3)"),2) $
      +"Hz ="+strtrim(string(gain0, FORMAT="(G8.3)"),2)
endelse


; z^-1 = exp(-s*T_s)
;ztf = poly(exp(-iu*2*!DPI*atan(!DPI*f_vec)), z_num)
;ztf = ztf/poly(exp(-iu*2*!DPI*atan(!DPI*f_vec)), z_den)
ztf = poly(iu*2d0*f0*tan(!DPI*f_vec/f0), [s_num])
ztf = ztf/poly(iu*2d0*f0*tan(!DPI*f_vec/f0), [s_den])
ztf = state.par[type, dof].s_const[0] * ztf

widget_control, state.id.hold_on_btn, GET_VALUE=hold_on
if hold_on[0] then ztf = ztf*hold_on_tf(iu*2*!DPI*f_vec, 2*adsec.sampling_time)/(2*adsec.sampling_time)
;; 2*adsec.sampling_time is used instead of adsec.sampling_time
;; because it takes into account of both the Hold-On and the sampling
;; as an average of the input error during a period adsec.sampling_time

wset, state.num.amp
plot_amp, [f_vec, f_vec], [tf, ztf], /DB $
  , xtitle='Freq !7m!X [Hz]' $
  , YTITLE='Amplitude of TF(2!7pim!X) [dB]' $
  , CHARSIZE=1.2 $
  , XSTY=18, YSTY=18, /NODATA, TITLE=title_amp
plot_amp, f_vec, tf, /DB, /OVERPLOT, THICK=2
plot_amp, f_vec, ztf, /DB, /OVERPLOT, LINE=2, THICK=2

wset, state.num.phase
plot_phase, [f_vec, f_vec], [tf, ztf], /DEG $
  , xtitle='Freq !7m!X [Hz]' $
  , ytitle='Phase of TF(2!7pim!X) [deg]' $
  , CHARSIZE=1.2 $
  , XSTY=18, YSTY=18, /NODATA
plot_phase, f_vec, tf, /DEG, /OVERPLOT, THICK=2
plot_phase, f_vec, ztf, /DEG, /OVERPLOT, LINE=2, THICK=2

end


;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; compute the digital filter from the analog filter
; model. Use the Tustin transform
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
pro tfl_comp_df, state, TYPE=type

@adsec_common

; error status from the event handler procedure
common error_block, error

dof = state.active_dof
if n_elements(type) eq 0 then type = state.active_type[dof]
n_s_zero = state.par[type, dof].n_s_zero
n_s_pole = state.par[type, dof].n_s_pole

if n_s_zero eq 0 then begin
    s_num = [dcomplex(state.par[type, dof].s_const)]
endif else begin
    s_num = state.par[type, dof].s_const $
      * zero2coeff(state.par[type, dof].s_zero[0:n_s_zero-1])
endelse

if n_s_pole eq 0 then begin
    s_den = [dcomplex(1d0)]
endif else begin
    s_den = zero2coeff(state.par[type, dof].s_pole[0:n_s_pole-1])
endelse

; s = 2/T*(1-z^-1)/(1+z^-1) : tustin (bilinear) transform
fs = 1d0/adsec.sampling_time
tustin, s_num, s_den, fs, z_num, z_den

n_z_num = n_elements(z_num)
n_z_den = n_elements(z_den)

;stop

state.z_par[type].n_z_num = n_z_num
state.z_par[type].n_z_den = n_z_den

im_sq = total(imaginary(z_num)^2)+total(imaginary(z_den)^2)
re_sq = total(double(z_num)^2)+total(double(z_den)^2)
eps = (machar(/DOUBLE)).eps

if im_sq gt eps^2*re_sq then begin
    message, 'Unexpected complex pole or zero without its conjugated.', $
      CONT=(sc.debug eq 0B)
    error = adsec_error.generic_error
    ;; widget_control, event.top, /DESTROY
    return
endif

state.z_par[type].z_num_coeff = 0d0
state.z_par[type].z_den_coeff = 0d0

state.z_par[type].z_num_coeff[0:n_z_num-1] = double(z_num)
state.z_par[type].z_den_coeff[0:n_z_den-1] = double(z_den)

end


;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; status setting procedure ;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;
pro tfl_gui_set, state

dof  = state.active_dof
type = state.active_type[dof]

case type of

    0: begin
        ;; do nothing
    end

    1: begin
        ;; do nothing
    end

    2: begin
        zero_selected = widget_info(state.id.zero_list, /LIST_SELECT)
        zero_selected = zero_selected ne -1

        case state.par[type, dof].n_s_zero of
            0: begin
                widget_control, state.id.edit_zero_btn, SENSITIVE=0
                widget_control, state.id.del_zero_btn,  SENSITIVE=0
                widget_control, state.id.add_zero_btn,  SENSITIVE=1
            end

            state.max_n_coeff-1: begin
                widget_control, state.id.edit_zero_btn, SENSITIVE=zero_selected
                widget_control, state.id.del_zero_btn,  SENSITIVE=zero_selected
                widget_control, state.id.add_zero_btn,  SENSITIVE=0
            end

            else: begin
                widget_control, state.id.edit_zero_btn, SENSITIVE=zero_selected
                widget_control, state.id.del_zero_btn,  SENSITIVE=zero_selected
                widget_control, state.id.add_zero_btn,  SENSITIVE=1
            end
        endcase


        pole_selected = widget_info(state.id.pole_list, /LIST_SELECT)
        pole_selected = pole_selected ne -1

        case state.par[type, dof].n_s_pole of
            0: begin
                widget_control, state.id.edit_pole_btn, SENSITIVE=0
                widget_control, state.id.del_pole_btn,  SENSITIVE=0
                widget_control, state.id.add_pole_btn,  SENSITIVE=1
            end

            state.max_n_coeff-1: begin
                widget_control, state.id.edit_pole_btn, SENSITIVE=pole_selected
                widget_control, state.id.del_pole_btn,  SENSITIVE=pole_selected
                widget_control, state.id.add_pole_btn,  SENSITIVE=0
            end

            else: begin
                widget_control, state.id.edit_pole_btn, SENSITIVE=pole_selected
                widget_control, state.id.del_pole_btn,  SENSITIVE=pole_selected
                widget_control, state.id.add_pole_btn,  SENSITIVE=1
            end
        endcase
    end
endcase

end

;;;;;;;;;;;;;;;;;;;;;;
; tfl_gui event loop ;
;;;;;;;;;;;;;;;;;;;;;;
;
pro tfl_gui_event, event

@adsec_common

common error_block, error
common tfl_gui_block, par, old_par

; read the GUI state structure
widget_control, event.top, GET_UVALUE=state

; Handle a kill request. It is considered as a cancel event.
; The right error returning is guaranteed only if GROUP_LEADER keyword
; is set to a valid parent id in the tfl_gui call.
if tag_names(event, /STRUCTURE_NAME) eq 'WIDGET_KILL_REQUEST' then begin
    error = adsec_error.cancel
    widget_control, event.top, /DESTROY
endif

; Handle other events.
; Get the user value of the event sender
widget_control, event.id, GET_UVALUE = uvalue

case uvalue of

    ;; handle event from standard save button
    'save': begin

        ;; cross check controls among the parameters
        ;;
        ;; PID filter checks
        dof = state.active_dof
        type= state.active_type[dof]

        if type eq 1 then begin
            ;; PID filter checks
            err = tfl_update_pid(state, GROUP=event.top)
            ;; return without saving if the test failed
            if err then return
        endif
        ;;
        ;; End of the checks

        ;; Computation of the discrete filter
        tfl_comp_df, state

        n_dof = n_elements(state.active_type)
        ;; saves the current parameter structure
        par = state.par[state.active_type, indgen(n_dof)]
        ;; kill the GUI returning a null error
        error = adsec_error.ok
        widget_control, event.top, /DESTROY
    end

    ;;; standard help button
    ;'help' : begin
    ;   widget_control, /HOURGLASS
    ;    spawn, !caos_env.browser+" "+!caos_env.help+"\#"+$
    ;      strupcase(state.par[0,0].module.mod_name)+"&"
    ;end

    ;; standard restore button:
    'restore': begin
        ;; revert the desired parameter values
        par = old_par

        ;n_dof = n_elements(state.active_type)
        ;n_par = n_elements(par)
        ;if n_par ne n_dof then begin
        ;    msg = ["The number of the restored degree of freedoms is not", $
        ;           "the same of the current ones. Exit the GUI using", $
        ;           "the CANCEL button and restart the GUI setting ", $
        ;           strtrim(n_par, 2) + $
        ;           " degree of freedom. Then retry to restore defaults."]
        ;    dummy = dialog_message(msg, /ERROR, DIALOG_PARENT=event.top, $
        ;                          TITLE="TFL Error")
        ;    ;; return without restoring
        ;    return
        ;endif

        type = par.type

        dof = 0
        state.active_dof  = dof
        state.active_type = type

        is_neg_fb = par[0].negative_fb

        widget_control, state.id.neg_fb_btn, SET_VALUE=is_neg_fb
        widget_control, state.id.type_btn, SET_VALUE=type[0]
        widget_control, state.id.dof_sld, SET_VALUE=state.active_dof+1

        ;; write the reset state structure
        n_dof = n_elements(state.active_type)
        state.par[type, indgen(n_dof)] = par

        case type[0] of

            2: begin
                ;; set the default values for all the widgets
                widget_control, state.id.const_fld, $
                  SET_VALUE=par[0].s_const
                widget_control, state.id.zero_list, $
                  SET_VALUE=tfl_zero2str(par[0])
                widget_control, state.id.pole_list, $
                  SET_VALUE=tfl_pole2str(par[0])
            end

            1: begin
                g  = par[0].s_const
                z  = par[0].s_zero
                p  = par[0].s_pole
                nz = par[0].n_s_zero
                np = par[0].n_s_pole
                err = gzp2pid(g, z, p, kp, ki, kd, A, N_Z=nz, N_P=np)
                if err then begin
                    message, "No valid PID filetr.", $
                      /CONT
                    error = adsec_error.generic_error
                    widget_control, event.top, /DESTROY
                    return
                endif else begin
                    widget_control, state.id.prop_fld, SET_VALUE=kp
                    widget_control, state.id.integ_fld, SET_VALUE=ki
                    widget_control, state.id.deriv_fld, SET_VALUE=kd
                    widget_control, state.id.lowpass_fld, SET_VALUE=A
                endelse
            end

            0: begin
                widget_control, state.id.gain_fld, SET_VALUE=par[0].s_const
            end
        endcase

        ;; show the input form associated to the restored filter type
        for i=0,2 do $
          widget_control, state.id.tf_base[i], MAP=(i eq type[0])

        ;; reset the setting parameters status
        tfl_gui_set, state

        ;; update the digital filter recursive formula
        tfl_comp_df, state
        widget_control, state.id.z_coeff_fld $
          , SET_VALUE=tfl_z_coeff2str(state.z_par[type[0]])
        ;; redraw the filter
        tfl_draw_filter, state
        ;; write the GUI state structure
        widget_control, event.top, SET_UVALUE=state
    end

    ;; standard cancel button (exit without saving)
    'cancel'  : begin
        error = adsec_error.cancel
        widget_control, event.top, /DESTROY
    end

    'tfl_comp_zcoeff': begin
        dof = state.active_dof
        type= state.active_type[dof]
        if type eq 1 then begin
            ;; PID filter checks
            err = tfl_update_pid(state, GROUP=event.top)
            if err then return
        endif

        tfl_comp_df, state
        widget_control, state.id.z_coeff_fld $
          , SET_VALUE=tfl_z_coeff2str(state.z_par[type])
    end

    'tfl_redraw': begin
        dof  = state.active_dof
        type = state.active_type[dof]
        if type eq 1 then begin
            ;; PID filter checks
            err = tfl_update_pid(state, GROUP=event.top)
            if err then return
        endif

        tfl_comp_df, state
        widget_control, state.id.z_coeff_fld $
          , SET_VALUE=tfl_z_coeff2str(state.z_par[type])
        tfl_draw_filter, state
    end

    'tfl_zero_list': begin
        tfl_gui_set, state
    end

    'tfl_pole_list': begin
        tfl_gui_set, state
    end

    'tfl_const_fld': begin
        dof  = state.active_dof
        type = state.active_type[dof]
        widget_control, state.id.const_fld, GET_VALUE=s_const
        state.par[type, dof].s_const = s_const
        widget_control, event.top, SET_UVALUE=state
    end

    'tfl_gain_fld': begin
        dof  = state.active_dof
        type = state.active_type[dof]
        widget_control, state.id.gain_fld, GET_VALUE=s_const
        state.par[type, dof].s_const = s_const
        widget_control, event.top, SET_UVALUE=state
    end

    'tfl_prop_fld': begin
        ;; do nothing
    end

    'tfl_integ_fld': begin
        ;; do nothing
    end

    'tfl_deriv_fld': begin
        ;; do nothing
    end

    'tfl_lowpass_fld': begin
        ;; do nothing
    end

    'tfl_add_zero': begin
        dof  = state.active_dof
        type = state.active_type[dof]

        err = tfl_new_item(the_zero, GROUP=event.top)
        if err ne adsec_ERROR.ok then return

        n_s_pole = state.par[type, dof].n_s_pole
        if n_s_pole ne 0 then begin
            eps = (machar(/DOUBLE)).eps
            s_pole = state.par[type, dof].s_pole[0:n_s_pole-1]
            is_eq = abs(s_pole-the_zero) le eps*abs(the_zero)
            if total(is_eq) ne 0 then begin
                dummy = dialog_message(['You cannot entry a zero equal' $
                                        , 'to one of the poles.'], /ERR $
                                       , DIALOG_PARENT=event.top, $
                                       TITLE="TFL Error")
                return
            endif
        endif

        nz = state.par[type, dof].n_s_zero
        state.par[type, dof].s_zero[nz] = the_zero
        state.par[type, dof].n_s_zero = nz+1

        widget_control, state.id.zero_list $
          , SET_VALUE=tfl_zero2str(state.par[type, dof])
        tfl_gui_set, state

        widget_control, event.top, SET_UVALUE=state
    end

    'tfl_add_pole': begin
        dof  = state.active_dof
        type = state.active_type[dof]

        err = tfl_new_item(the_pole, GROUP=event.top)

        if err ne adsec_ERROR.ok then return

        n_s_zero = state.par[type, dof].n_s_zero
        if n_s_zero ne 0 then begin
            eps = (machar(/DOUBLE)).eps
            s_zero = state.par[type, dof].s_zero[0:n_s_zero-1]
            is_eq = abs(s_zero-the_pole) le eps*abs(the_pole)
            if total(is_eq) ne 0 then begin
                dummy =dialog_message(['You cannot entry a pole equal to one' $
                                        , 'of the zeros.'], /ERR $
                                       , DIALOG_PARENT=event.top)
                return
            endif
        endif

        np = state.par[type, dof].n_s_pole
        state.par[type, dof].s_pole[np] = the_pole
        state.par[type, dof].n_s_pole = np+1

        widget_control, state.id.pole_list $
          , SET_VALUE=tfl_pole2str(state.par[type, dof])
        tfl_gui_set, state

        widget_control, event.top, SET_UVALUE=state
    end

    'tfl_edit_zero': begin
        dof  = state.active_dof
        type = state.active_type[dof]

        the_line=widget_info(state.id.zero_list, /LIST_SELECT)
        n_s_zero = state.par[type, dof].n_s_zero
        if (the_line lt 0) or (the_line ge n_s_zero) then return
        the_zero = state.par[type, dof].s_zero[the_line]

        err = tfl_new_item(the_zero, GROUP=event.top)
        if err ne adsec_ERROR.ok then return

        n_s_pole = state.par[type, dof].n_s_pole
        if n_s_pole ne 0 then begin
            eps = (machar(/DOUBLE)).eps
            s_pole = state.par[type, dof].s_pole[0:n_s_pole-1]
            is_eq = abs(s_pole-the_zero) le eps*abs(the_zero)
            if total(is_eq) ne 0 then begin
                dummy =dialog_message(['You cannot entry a zero equal to one' $
                                        , 'of the poles.'], /ERR $
                                       , DIALOG_PARENT=event.top)
                return
            endif
        endif

        state.par[type, dof].s_zero[the_line] = the_zero

        widget_control, state.id.zero_list $
          , SET_VALUE=tfl_zero2str(state.par[type, dof])
        tfl_gui_set, state

        widget_control, event.top, SET_UVALUE=state
    end

    'tfl_edit_pole': begin
        dof  = state.active_dof
        type = state.active_type[dof]

        the_line=widget_info(state.id.pole_list, /LIST_SELECT)
        n_s_pole = state.par[type, dof].n_s_pole
        if the_line lt 0 or the_line ge n_s_pole then return
        the_pole = state.par[type, dof].s_pole[the_line]

        err = tfl_new_item(the_pole, GROUP=event.top)
        if err ne adsec_ERROR.ok then return

        n_s_zero = state.par[type, dof].n_s_zero
        if n_s_zero ne 0 then begin
            eps = (machar(/DOUBLE)).eps
            s_zero = state.par[type, dof].s_zero[0:n_s_zero-1]
            is_eq = abs(s_zero-the_pole) le eps*abs(the_pole)
            if total(is_eq) ne 0 then begin
                dummy =dialog_message(['You cannot entry a pole equal to one' $
                                        , 'of the zeros.'], /ERR $
                                       , DIALOG_PARENT=event.top)
                return
            endif
        endif

        state.par[type, dof].s_pole[the_line] = the_pole

        widget_control, state.id.pole_list $
          , SET_VALUE=tfl_pole2str(state.par[type, dof])
        tfl_gui_set, state

        widget_control, event.top, SET_UVALUE=state
    end

    'tfl_del_zero': begin
        dof  = state.active_dof
        type = state.active_type[dof]

        the_line=widget_info(state.id.zero_list, /LIST_SELECT)
        n_s_zero = state.par[type, dof].n_s_zero
        if (the_line lt 0) or (the_line ge n_s_zero) then return

        n_s_zero = state.par[type, dof].n_s_zero
        if n_s_zero eq 1 then begin
            state.par[type, dof].s_zero = 0B
            state.par[type, dof].n_s_zero = 0
        endif else begin
            idx = where(indgen(n_s_zero) ne the_line, count)
            if count eq 0 then return

            s_zero = state.par[type, dof].s_zero
            state.par[type, dof].s_zero = 0B
            state.par[type, dof].s_zero[0] = s_zero[idx]
            state.par[type, dof].n_s_zero = count
        endelse

        widget_control, state.id.zero_list $
          , SET_VALUE=tfl_zero2str(state.par[type, dof])
        tfl_gui_set, state

        widget_control, event.top, SET_UVALUE=state
    end

    'tfl_del_pole': begin
        dof  = state.active_dof
        type = state.active_type[dof]

        the_line=widget_info(state.id.pole_list, /LIST_SELECT)
        n_s_pole = state.par[type, dof].n_s_pole
        if the_line lt 0 or the_line ge n_s_pole then return

        n_s_pole = state.par[type, dof].n_s_pole
        if n_s_pole eq 1 then begin
            state.par[type, dof].s_pole = 0B
            state.par[type, dof].n_s_pole = 0
        endif else begin
            idx = where(indgen(n_s_pole) ne the_line, count)
            if count eq 0 then return

            s_pole = state.par[type, dof].s_pole
            state.par[type, dof].s_pole = 0B
            state.par[type, dof].s_pole[0] = s_pole[idx]
            state.par[type, dof].n_s_pole = count
        endelse

        widget_control, state.id.pole_list $
          , SET_VALUE=tfl_pole2str(state.par[type, dof])
        tfl_gui_set, state

        widget_control, event.top, SET_UVALUE=state
    end

    'hold_on': begin
        ;; do nothing
    end

    'tfl_neg_fb': begin
        is_negative_fb = event.value[0]

        state.par.negative_fb = is_negative_fb

        widget_control, event.top, SET_UVALUE=state
    end

    'tfl_type': begin
        type = event.value

        dof  = state.active_dof
        state.active_type[dof] = type

        ;; show the related input form
        for i=0,2 do $
          widget_control, state.id.tf_base[i], MAP=(i eq type)

        ;; set the right sensitive stati
        tfl_gui_set, state

        ;; update the digital filter recursive formula
        tfl_comp_df, state
        widget_control, state.id.z_coeff_fld $
          , SET_VALUE=tfl_z_coeff2str(state.z_par[type])

        ;; redraw the filter
        tfl_draw_filter, state

        widget_control, event.top, SET_UVALUE=state
    end

    'tfl_dof': begin
        dof_new = event.value-1

        dof = state.active_dof
        if dof eq dof_new then return ;; return without doing anything

        ;;;;;;
        ;; Check the validity of the current filter before
        ;; changing the degree of freedom
        ;;;;;;
        type = state.active_type[dof]

        if type eq 1 then begin
            ;; PID filter checks
            err = tfl_update_pid(state, GROUP=event.top)
            ;; return without changing the d.o.f. if the test failed
            if err then return
        endif
        ;;
        ;; End of the checks

        ;; if the checks don't fail, display the filter for the
        ;; new degree of freedom
        dof = dof_new
        state.active_dof = dof
        type = state.active_type[dof]


        ;; fill the fields of the GZP type
        widget_control, state.id.const_fld, $
          SET_VALUE=state.par[2, dof].s_const
        widget_control, state.id.zero_list, $
          SET_VALUE=tfl_zero2str(state.par[2, dof])
        widget_control, state.id.pole_list, $
          SET_VALUE=tfl_pole2str(state.par[2, dof])

        ;; fill the fields of the PID type
        g  = state.par[1, dof].s_const
        z  = state.par[1, dof].s_zero
        p  = state.par[1, dof].s_pole
        nz = state.par[1, dof].n_s_zero
        np = state.par[1, dof].n_s_pole
        err = gzp2pid(g, z, p, kp, ki, kd, A, N_Z=nz, N_P=np)
        if err and (type eq 1) then begin
            msg = ["Corrupted PID filter. Switching to GZP type."]
            dummy = dialog_message(msg, /WARN, $
                                   DIALOG_PARENT = event.top, $
                                   TITLE = 'PID Warning')

            type = 2
            state.active_type[dof] = type
        endif else begin
            widget_control, state.id.prop_fld, SET_VALUE=kp
            widget_control, state.id.integ_fld, SET_VALUE=ki
            widget_control, state.id.deriv_fld, SET_VALUE=kd
            widget_control, state.id.lowpass_fld, SET_VALUE=A
        endelse

        ;; fill the field of the "Pure Integrator" type
        widget_control, state.id.gain_fld, SET_VALUE=state.par[0, dof].s_const

        ;; set the right type selection
        widget_control, state.id.type_btn, SET_VALUE=type

        ;; show the related input form
        for i=0,2 do $
          widget_control, state.id.tf_base[i], MAP=(i eq type)

        ;; set the right sensitive stati
        tfl_gui_set, state

        ;; update the digital filter recursive formula
        tfl_comp_df, state
        widget_control, state.id.z_coeff_fld $
          , SET_VALUE=tfl_z_coeff2str(state.z_par[type])

        ;; redraw the filter
        tfl_draw_filter, state

        widget_control, event.top, SET_UVALUE=state
    end
endcase

end

;;;;;;;;;;;;;;;;;;;;;;;;;
; GUI generation code
;;;;;;;;;;;;;;;;;;;;;;;;;
;
function tfl_gui, the_par, GROUP_LEADER=group, MODAL=modal

@adsec_common

; error status from the event handler procedure
common error_block, error
common tfl_gui_block, par, old_par

if n_elements(the_par) eq 0 then begin
    par = tfl_gen_default()           ; set the default par structure
endif else begin
   if test_type(the_par, /STRUC, DIM=dim) then $
      message, 'the input parameter '    $
              +'must be a structure'
   if (total(dim[0]) ne 1) then       $
      message, 'the input parameter must be a structure vector'
   par = the_par
endelse
old_par = par

n_par = n_elements(par)
n_par_old = n_par

; dimension of the draw panels
x_draw = 350
y_draw = 250

; computes the dimension of the zero/pole list
max_n_coeff = n_elements(par[0].s_zero)
y_list = max_n_coeff-1
str = (complex2str(1d0, 1d0))[0]
x_list = strlen(str)+6

; list of predefined filter types
type_list = ['Pure integrator','PID','Generic gain-zero-pole']
tf_base_id = lonarr(n_elements(type_list))

id = { $                        ; widget id structure
       tf_base       : tf_base_id, $ ; parameters of TF
       gain_fld      : 0L, $    ; gain for pure integrator (fld id)
       prop_fld      : 0L, $    ; proportional gain in the PID filter (fld id)
       integ_fld     : 0L, $    ; integrator gain in the PID filter (fld id)
       deriv_fld     : 0L, $    ; derivative gain in the PID filetr (fld id)
       lowpass_fld   : 0L, $    ; low-pass cut for the derivative component
       const_fld     : 0L, $    ; constant factor of the g-z-p TF (btn id)
       zero_list     : 0L, $    ; list of zero data id
       add_zero_btn  : 0L, $    ; add a zero to the list (btn id)
       edit_zero_btn : 0L, $    ; edit a zero in the list (btn id)
       del_zero_btn  : 0L, $    ; delete a zero from the list (btn id)
       pole_list     : 0L, $    ; list of zero data id
       add_pole_btn  : 0L, $    ; add a zero to the list (btn id)
       edit_pole_btn : 0L, $    ; edit a zero in the list (btn id)
       del_pole_btn  : 0L, $    ; delete a zero from the list (btn id)
       neg_fb_btn    : 0L, $    ; negative feedback setting (btn id)
       z_coeff_fld   : 0L, $    ; discrete filter coeffs display (fld id)
       hold_on_btn   : 0L, $    ; if set add the HO effect in the plot (btn id)
       redraw_btn    : 0L, $    ; redraw the tranfer function (btn id)
       type_btn      : 0L, $    ; type of filter parametrization (gbtn id)
       dof_sld       : 0L  $    ; current number of degree of freedoms
     }

num = { $                       ; draw panel number
        amp   : 0L, $           ; filter amplitude draw panel
        phase : 0L  $           ; filter phase draw panel
      }

z_coeff = dblarr(max_n_coeff)
z_par = { $
          n_z_num    : 0,       $ ; number of coeffs of the z-domain tf num.
          n_z_den    : 0,       $ ; number of coeffs of the z-domain tf den.
          z_num_coeff: z_coeff, $ ; coeffs of the z-plane filter tf numer.
          z_den_coeff: z_coeff  $ ; coeffs of the z-plane filter tf denom.
        }


n_type_list = n_elements(type_list)
type = par.type

par_save = par
par = replicate(par[0], n_type_list, n_par)

for k=0,n_type_list-1 do par[k,*] = par_save

z_par = replicate(z_par, n_type_list)

for k=0,n_par-1 do begin

    if type[k] ne 0 then begin
        ;; the restored filter type is not a pure integrator.
        ;; a pure integrator is forced to be set in par[0,k]
        par[0,k].s_const  = 1d0
        par[0,k].n_s_zero = 0
        par[0,k].n_s_pole = 1
        par[0,k].s_zero   = 0d0
        par[0,k].s_pole   = 0d0
        par[0,k].type     = 0
    endif
    if type[k] ne 1 then begin
        ;; the restored filter type is not a PID.
        ;; a PID is forced to be set in par[1,k]
        par[1,k].s_const  = 1d0
        par[1,k].n_s_zero = 0
        par[1,k].n_s_pole = 1
        par[1,k].s_zero   = 0d0
        par[1,k].s_pole   = 0d0
        par[1,k].type     = 1
    endif
    if type[k] ne 2 then begin
        ;; the restored filter type is not a generic gain-zero-pole filter.
        ;; a gain-zero-pole filetr is forced to be set in par[2]
        ;; a pure integrator, just to set a default filter
        par[2,k].s_const  = 1d0
        par[2,k].n_s_zero = 0
        par[2,k].n_s_pole = 1
        par[2,k].s_zero   = 0d0
        par[2,k].s_pole   = 0d0
        par[2,k].type     = 2
    endif
endfor

state = { $                      ;; widget state structure
          id         : id,       $ ; widget id structure
          num        : num,      $ ; draw panel numbers
          active_type: type,     $ ; vector of the active type for each d.o.f.
          active_dof : 0,        $ ; index of the active degree of freedom
          z_par      : z_par,    $ ; discrete filter coeff structures
          max_n_coeff: max_n_coeff, $ ; max number of poles/zeros +1
          par        : par       $ ; parameter structures
        }

; compute the discrete filter for the restored/default filter types
; of the current d.o.f.
for k=0,2 do tfl_comp_df, state, TYPE=k

; root base
title = 'Time filter builder GUI'
root_base_id = widget_base(TITLE=title, MODAL=modal, /COL, GROUP_LEADER=group)

; avoid idl to set a draw-widget of this gui as default drawing window
widget_control, /managed, root_base_id

; parameter base
par_base_id = widget_base(root_base_id, /FRAME, /ROW)


; data input base structure
data_base_id = widget_base(par_base_id, /FRAME, /COL)
dummy = widget_label(data_base_id, VALUE='parameters')
data_base_id = widget_base(data_base_id, /FRAME, /COL)

dummy = widget_base(data_base_id) ; TF parametrization panels
tf_base_id[0] = widget_base(dummy, /COL) ; pure integrator
tf_base_id[1] = widget_base(dummy, /COL) ; PID
tf_base_id[2] = widget_base(dummy, /COL) ; generic gain-zero-pole
state.id.tf_base = tf_base_id

; Pure integrator transfer function parametrization
dummy = widget_label(tf_base_id[0], $
                     VALUE='Pure Integrator filter representation (Laplace transform):', $
                     /FRAME)
dummy = widget_label(tf_base_id[0], VALUE='         G ')
dummy = widget_label(tf_base_id[0], VALUE='TF(s) = ---')
dummy = widget_label(tf_base_id[0], VALUE='         s ')
dummy = widget_label(tf_base_id[0] $
                     , VALUE='(sampling time T = ' $
                     +string(adsec.sampling_time, FORMAT='(E12.3)')+'s)')

; constant factor input
dummy = widget_base(tf_base_id[0], /ROW)
state.id.gain_fld = cw_field(dummy $
                              , /FRAME $
                              , /FLOAT $
                              , TITLE='Integrator Gain Factor (G):' $
                              , VALUE=par[0,0].s_const $
                              , /ALL_EVENTS $
                              , UVALUE='tfl_gain_fld')

; PID transfer function parametrization
dummy = widget_label(tf_base_id[1], $
                     VALUE='PID filter representation (Laplace transform):', $
                     /FRAME)
dummy = widget_label(tf_base_id[1] $
                     , VALUE='                     1            A      ')
dummy = widget_label(tf_base_id[1] $
                     , VALUE='TF(s) = K_p + K_i * --- + K_d * ----- * s')
dummy = widget_label(tf_base_id[1] $
                     , VALUE='                     s           s+A     ')
dummy = widget_label(tf_base_id[1] $
                     , VALUE='(sampling time T = ' $
                     +string(adsec.sampling_time, FORMAT='(E12.3)')+'s)')

dummy = widget_base(tf_base_id[1], /COL, /FRAME)

err = gzp2pid(par[1,0].s_const, par[1,0].s_zero, par[1,0].s_pole $
              , kp, ki, kd, A, N_Z=par[1,0].n_s_zero, N_P=par[1,0].n_s_pole)
if err then $
  message, 'Not valid PID filter.'
; proportional factor input
state.id.prop_fld = cw_field(dummy $
                              , /FRAME $
                              , /FLOAT $
                              , TITLE='Proportional (K_p) [N/m]:' $
                              , VALUE=kp $
                              , /ALL_EVENTS $
                              , UVALUE='tfl_prop_fld')
; integrator factor input
;dummy = widget_base(tf_base_id[1], /ROW)
state.id.integ_fld = cw_field(dummy $
                              , /FRAME $
                              , /FLOAT $
                              , TITLE='Integrator (K_i) [N/m*rad/s]:' $
                              , VALUE=ki $
                              , /ALL_EVENTS $
                              , UVALUE='tfl_integ_fld')
; derivative factor input
;dummy = widget_base(tf_base_id[1], /ROW)
state.id.deriv_fld = cw_field(dummy $
                              , /FRAME $
                              , /FLOAT $
                              , TITLE='Derivative (K_d) [N/m*s/rad]:' $
                              , VALUE=kd $
                              , /ALL_EVENTS $
                              , UVALUE='tfl_deriv_fld')
; low-pass frequency input
;dummy = widget_base(tf_base_id[1], /ROW)
state.id.lowpass_fld = cw_field(dummy $
                              , /FRAME $
                              , /FLOAT $
                              , TITLE='Low-pass cut (A) [rad/s]:' $
                              , VALUE=A $
                              , /ALL_EVENTS $
                              , UVALUE='tfl_lowpass_fld')

;;
;; gain-zero-pole transfer function parametrization
;;
dummy = widget_label(tf_base_id[2], $
                     VALUE='Gain-poles-zeros (GPZ) filter representation (Laplace transform):', $
                     /FRAME)
dummy = widget_label(tf_base_id[2] $
                     , VALUE='            (s+z[0])*(s+z[1])*...*(s+z[nz-1])')
dummy = widget_label(tf_base_id[2] $
                     , VALUE='TF(s) = G * ---------------------------------')
dummy = widget_label(tf_base_id[2] $
                     , VALUE='            (s+p[0])*(s+p[1])*...*(s+p[np-1])')
dummy = widget_label(tf_base_id[2] $
                     , VALUE='0<= nz,np <= 4')
dummy = widget_label(tf_base_id[2] $
                     , VALUE='(sampling time T = ' $
                     +string(adsec.sampling_time, FORMAT='(E12.3)')+'s)')

; constant factor input
dummy = widget_base(tf_base_id[2], /ROW)
state.id.const_fld = cw_field(dummy $
                              , /FRAME $
                              , /FLOAT $
                              , TITLE='Constant Factor (G) [N/m]:' $
                              , VALUE=par[2,0].s_const $
                              , /ALL_EVENTS $
                              , UVALUE='tfl_const_fld')

; Panel for editing the zeros
zero_base_id      = widget_base(tf_base_id[2,0], /FRAME, /COL)
dummy = widget_label(zero_base_id, $
                     VALUE='List of ZEROS (z[i]) [rad/s]:')
dummy = widget_base(zero_base_id, /ROW)
; list of zeros
zero_list_base_id = widget_base(dummy, /COL)
state.id.zero_list = widget_list(zero_list_base_id $
                                 , UVALUE='tfl_zero_list' $
                                 , XSIZE=x_list $
                                 , YSIZE=y_list $
                                 , VALUE=tfl_zero2str(par[2,0]))
zero_btn_base_id  = widget_base(dummy, /COL)
; btns for zero list editing
state.id.add_zero_btn = widget_button(zero_btn_base_id $
                                      , VALUE='Add a zero' $
                                      , UVALUE='tfl_add_zero')
state.id.edit_zero_btn = widget_button(zero_btn_base_id $
                                      , VALUE='Edit the zero' $
                                      , UVALUE='tfl_edit_zero')
state.id.del_zero_btn = widget_button(zero_btn_base_id $
                                      , VALUE='Delete the zero' $
                                      , UVALUE='tfl_del_zero')

; Panel for editing the poles
pole_base_id      = widget_base(tf_base_id[2], /FRAME, /COL)
dummy = widget_label(pole_base_id, $
                     VALUE='List of POLES (p[i]) [rad/s]:')
dummy = widget_base(pole_base_id, /ROW)
pole_list_base_id = widget_base(dummy, /COL)
; list of poles
state.id.pole_list = widget_list(pole_list_base_id $
                                 , UVALUE='tfl_zero_list' $
                                 , XSIZE=x_list $
                                 , YSIZE=y_list $
                                 , VALUE=tfl_pole2str(par[2,0]))
pole_btn_base_id  = widget_base(dummy, /COL)
; btns for pole list editing
state.id.add_pole_btn = widget_button(pole_btn_base_id $
                                      , VALUE='Add a pole' $
                                      , UVALUE='tfl_add_pole')
state.id.edit_pole_btn = widget_button(pole_btn_base_id $
                                      , VALUE='Edit the pole' $
                                      , UVALUE='tfl_edit_pole')
state.id.del_pole_btn = widget_button(pole_btn_base_id $
                                      , VALUE='Delete the pole' $
                                      , UVALUE='tfl_del_pole')

;;
;; Negative feedback check box
;;
state.id.neg_fb_btn = CW_BGROUP(data_base_id, /COL, /NONEXCLUSIVE, /NO_REL, $
                                "Filter for negative Feedback", $
                                UVALUE='neg_fb', /FRAME, $
                                SET_VALUE=par[0].negative_fb)

;;
;; Filter type selector
;;
state.id.type_btn = CW_BGROUP(data_base_id, /COL, /EXCLUSIVE, /NO_REL, $
                              type_list, UVALUE='tfl_type', $
                              /FRAME, SET_VALUE=type[0])


;;
;; base for discrete filter coefficients (z_coeff)
;;
z_coeff_base_id = widget_base(data_base_id, /FRAME, /COL)

dummy = widget_label(z_coeff_base_id $
                     , VALUE = 'Recursive Discrete Filter implementation:')
state.id.z_coeff_fld = widget_text(z_coeff_base_id $
                                   , /SCROLL $
                                   , YSIZE=2 $
                                   , VALUE= $
                                   tfl_z_coeff2str(state.z_par[type[0]]) $
                                   , UVALUE='tfl_z_coeff')
dummy = widget_button(z_coeff_base_id $
                      , VALUE = 'Compute now coeffs' $
                      , UVALUE = 'tfl_comp_zcoeff')

;
;
;
; base for TF display
display_base_id = widget_base(par_base_id, /FRAME, /COL)
dummy = widget_label(display_base_id $
                     , VALUE='Filter Tranfer Function Bode Plots')
dummy = widget_label(display_base_id $
                     , VALUE='(Analog Filter Prototype TF: solid line)')
dummy = widget_label(display_base_id $
                     , VALUE='(Discrete Filter TF: dashed line)')
amp_draw_id = widget_draw(display_base_id $
                          , XSIZE = x_draw $
                          , YSIZE = y_draw $
                          , UVALUE='tfl_amp_draw')
phase_draw_id = widget_draw(display_base_id $
                            , XSIZE = x_draw $
                            , YSIZE = y_draw $
                            , UVALUE='tfl_phase_draw')

state.id.hold_on_btn = CW_BGROUP(display_base_id, /COL, /NONEXCLUSIVE, $
                                 /NO_REL, $
                                 "Add the Hold-on (DAC)+CCD integration delay effect", $
                                UVALUE='hold_on', $
                                SET_VALUE=[1])

state.id.redraw_btn = widget_button(display_base_id $
                                    , VALUE='Redraw Filter TF' $
                                    , UVALUE='tfl_redraw')


dof_base_id = widget_base(display_base_id, /FRAME, /COL)
state.id.dof_sld = widget_slider(dof_base_id, $
                                 TITLE = "Current degree of freedom number:", $
                                 /FRAME, MAX = n_par>2, MIN = 1, $
                                 VALUE = 1, $
                                 UVALUE = "tfl_dof")
;
;
; button base for control buttons (standrd buttons)
btn_base_id = widget_base(root_base_id, /FRAME, /ROW)

;dummy     = widget_button(btn_base_id, VALUE="HELP", UVALUE="help")
cancel_id = widget_button(btn_base_id, VALUE="Cancel", UVALUE="cancel")
dummy     = widget_button(btn_base_id, VALUE="Revert", $
                          UVALUE="restore")
save_id   = widget_button(btn_base_id, VALUE="Ok", UVALUE="save")

if keyword_set(modal) then widget_control, save_id,   /DEFAULT_BUTTON
if keyword_set(modal) then widget_control, cancel_id, /CANCEL_BUTTON

;
; Set Visible only the active filter type panel
;
for k=0,n_type_list-1 do $
  widget_control, state.id.tf_base[k], MAP=state.active_type[0] eq k


;
;
; draw the GUI
widget_control, root_base_id, /realize

; get the window numbers associated to the draw widgets
widget_control, amp_draw_id, GET_VALUE=dummy
state.num.amp = dummy
widget_control, phase_draw_id, GET_VALUE=dummy
state.num.phase = dummy

; save the state structure of the GUI in the top base uvalue
widget_control, root_base_id, SET_UVALUE=state

; initialize all the sensitive states
tfl_gui_set, state
if n_par eq 1 then $
    widget_control, state.id.dof_sld, SENSITIVE=0

; draw the filter tranfer function
tfl_draw_filter, state

if sc.debug then xmanager, CATCH=0 else xmanager, /CATCH
xmanager, 'tfl_gui', root_base_id, GROUP_LEADER=group

if error eq adsec_error.ok then the_par=par
return, error
end
