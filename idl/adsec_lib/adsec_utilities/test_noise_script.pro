; $Id: test_noise_script.pro,v 1.4 2009/06/11 12:49:21 labot Exp $$
;
;+
; NAME:
;       TEST_NOISE_SCRIPT
;
; PURPOSE:
;       The script to test the electronic noise.
;
; CATEGORY:
;       General utilities.
;
; CALLING SEQUENCE:
;
; INPUTS:
;      None
;
; OUTPUTS:
;       NONE.
; HISTORY
;   2005 Marco Xompero(MX)
;
;-

dosave=1
samples = 2048
times = 10

lin = 0
tn=tracknum()
dir = meas_path('noise', date=tn)
ext=''

name = dir+'noise' +ext+ '.sav'
name2 = dir+tn+'_noise' +ext+ '.jpg'
pump = adam_out
print, name
;;;;pompa;;;;
;pump.out5 = 1
;no_pump.out5 = 0

;ans = dialog_message("hai riacceso e spento il camion?", /QUEST)
;if ans eq "No" then stop
;print, adam_send(no_pump)
;print, "POMPA SPENTA"
;wait, 10
;;;;pompa;;;;

no_lin =~lin
;updating the status of IDL registers.
err = update_status()

if err ne adsec_error.ok then begin
    message, "Error updating the status", CONT=(sc.debug eq 0B)
;    return, err
endif

;Control the closed loop actuators
cl_act = adsec.act_w_cl;where(sys_status.closed_loop, n_cl_act)

n_cl_act = n_elements(cl_act)
if n_cl_act eq 0 then begin
    message, "No actuator in closed loop", CONT=(sc.debug eq 0B)
;    return, err
endif

if times ne 0 then begin
	err = test_noise(times, max_min, sigma_pos, coeff, covar,sk,buf,SAMPLES=samples, NO_LIN=no_lin)
    if err ne adsec_error.ok then begin
        message, 'Unable to test the noise for the covariance matrix retrieval.', CONT=(sc.debug eq 0B)
;        return, err
    endif

	;; compute the covariance matrix of the mean
	if (size(covar))[0] eq 3 then $
	    m_covar=total(covar,3)/(long(times)) $;*samples) $
	else $
	    m_covar=covar;/samples
       
       m_covar[adsec.act_wo_pos,*]=0
       m_covar[*,adsec.act_wo_pos]=0
	window, 1, retain=2
	dummy=m_covar[*,cl_act]
	dummy=dummy[cl_act,*]
	min_v=min(dummy)
	max_v=max(dummy)
	image_show, m_covar, /AS, /SH, max_v=max_v, min_v=min_v, title= 'Mean covariance matrix'

    window, 2, retain=2, XS=1024, ys=768
    sigma_pos_reb = reform(rebin(sigma_pos,adsec.n_actuators, 1))
    if ~lin then begin
        display, sigma_pos_reb[adsec.act_w_cl], adsec.act_w_cl, /sh, title= 'Mean RMS on all realizations', BAR_TIT='[bit]', YTICKF_BAR='(f10.2)', XTITLE='[mm]', YTITLE='[mm]'
    endif else begin
        display, sigma_pos_reb[adsec.act_w_cl]*1e9, adsec.act_w_cl, /sh, title= 'Mean RMS on all realizations', BAR_TIT='[nm]', YTICKF_BAR='(f10.2)', XTITLE='[mm]', YTITLE='[mm]'
    endelse
    write_jpeg,  dir+'noise' +ext+ '.jpg', tvrd(true=3), true=3, quality=100
    write_jpeg,  name2, tvrd(true=3), true=3, quality=100

    window, 3, retain=2
    m_covar_mir = m_covar[adsec.mir_act2act,*]
    m_covar_mir = m_covar_mir[*,adsec.mir_act2act]
    image_show, m_covar_mir, /AS, /SH, max_v=max_v, min_v=min_v, title= 'Mean covarianve matrix mir-ordered'
adsec_save = adsec
err = get_status(status_save)
if err ne adsec_error.ok then message, "error", /INFO
log_print, name
if dosave then save, file=name, m_covar, m_covar_mir, sigma_pos, sigma_pos_reb, covar, adsec_save, status_save, max_min, buf

;;;pompa;;;
;print, adam_send(pump)
;print, "POMPA accesa"
;;;pompa;;;
endif
;
end
