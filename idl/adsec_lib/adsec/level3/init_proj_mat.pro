; $Id: init_proj_mat.pro,v 1.2 2009/11/27 09:10:59 lbusoni Exp $
;+
;   HISTORY
;   01 Dec 2004: Marco Xompero
;    pos_preshaper_len and curr_preshaper_len fields added
;   04 Aug 2005, MX
;    new fast_diag_freq field added.
;   02 July 2007, A. Riccardi (AR)
;    added time_stamo0 and time_stamp1 fields
;   19 Sep 2007, MX
;    ZENITH ANGLE variable added. Shared memory initialization for multiple IDL viewing added. 
;-
Function init_proj_mat, PMHOFORFITS=pmhoforfits, PMFORFITS=pmforfits, PMPOSFITS=pmposfits, PMZFITS=pmzfits

    @adsec_common

    file2restore = pmforfits
    zero_pmforfits = 0
    if ~(file_test(file2restore, /READ, /REGULAR)) then begin
        log_print, 'Projection matrix for forces not found: zero matrix will be used'
        zero_pmforfits = 1
    endif

    file2restore = pmposfits
    zero_pmposfits = 0
    if ~(file_test(file2restore, /READ, /REGULAR)) then begin
        log_print, 'Projection matrix for position not found: zero matrix will be used'
        zero_pmposfits = 1
    endif

    file2restore = pmzfits
    zero_pmzfits = 0
    if ~(file_test(file2restore, /READ, /REGULAR)) then begin
        log_print, 'Projection matrix for zernikes not found: zero matrix will be used'
        zero_pmzfits = 1
    endif

    file2restore = pmhoforfits
    zero_pmhoforfits = 0
    if ~(file_test(file2restore, /READ, /REGULAR)) then begin
        log_print, 'Projection matrix for high order offload not found: zero matrix will be used'
        zero_pmhoforfits = 1
    endif

    if zero_pmforfits then pmfor = dblarr(adsec.n_actuators, adsec.n_actuators) else pmfor = readfits(pmforfits)
    if zero_pmhoforfits then pmhofor = dblarr(adsec.n_actuators, adsec.n_actuators) else pmhofor = readfits(pmhoforfits)
    if zero_pmposfits then pmpos = dblarr(adsec.n_actuators, 22) else pmpos = readfits(pmposfits)
    if zero_pmzfits then pmz = dblarr(22, adsec.n_actuators) else pmz = readfits(pmzfits)

    proj_mat_template =    $
      {                    $
        pmfor    : pmfor,  $
        pmhofor  : pmhofor,$
        pmpos    : pmpos,  $
        pmz      : pmz     $
      }

    pm_offload = proj_mat_template
    !AO_STATUS.pmz = pmzfits
    !AO_STATUS.pmpos = pmposfits
    !AO_STATUS.pmfor = pmforfits
    !AO_STATUS.pmhofor = pmhoforfits

;   if !AO_STATUS.pmz eq "" then begin
;       sys_status.pmz_angle = 0
;   endif else begin
;       ;rotation between acquired zernike reference system and telescope reference system 
;       vv=fltarr(1, 22) & vv[5]=1 
;       tmp=pm_offload.pmz ## vv 
;       display, tmp[adsec.act_w_cl], adsec.act_w_cl, /sh, /no_n, /sm, pos=zimg, /no_plot
;       idx = where(zimg ne median(zimg))
;       szimg = size(zimg, /dim)
;       xx = rebin(mk_vector(szimg[0], -1, 1), szimg[0], szimg[0], /samp)
;       yy = rebin(transpose(mk_vector(szimg[0], -1, 1)), szimg[0], szimg[0], /samp)
;       fit = surf_fit(xx[idx], yy[idx], zimg[idx], [5,6], /zern, coeff=coeff) 
;       sys_status.pmz_angle = atan(coeff[0], coeff[1])/!pi*180 
;   endelse

  
    return, adsec_error.ok
end
