;$Id: filter_mode_recons.pro,v 1.1 2006/03/20 10:30:15 labot Exp $
;
; NAME:
;   FILTER_MODE_RECONS
;
; PURPOSE:
;   Filter throught the reconstructor matrix the chosen mode in the secondary mirror
;   command. 
;
; CATEGORY:
;   General math.
;
; CALLING SEQUENCE:
;   result = filter_mode_recons(mode, REC_FILE=rec_file, M2C_FILE=m2c_file)
;
; INPUTS:
;   mode = the mode to filter, if not set the piston command is filtered.
;          The mode must be normalized to 1. 
;
; KEYWORD PARAMETERS:
;   REC_FILE = File name of the .fits reconstruction matrix. 
;       
;   M2C_FILE = File name of the .fits mode to commands matrix.
;
;
; OUTPUTS:
;   result: The new reconstruction matrix computes filtering the input mode.
;
; RESTRICTIONS:
;   None.
;
; MODIFICATION HISTORY:
;   13 March 2006
;    Created by Daniela Zanotti(DZ)
;    zanotti@arcetri.astro.it
;
;-

function filter_mode_recons, mode, REC_FILE=rec_file, M2C_FILE=m2c_file  

@adsec_common

 eps = machar()
 n_act = adsec.n_actuators
 n_true_act = n_elements(adsec.true_act) 
 n_dummy = n_act - n_elements(adsec.true_act)
 
 if ~keyword_set(rec_file) then begin 
    R = readfits('../../Supervisor/config/reconstructors/rec_2006-03-06_2V_45.fits')
    print, 'A default Reconstructor file is used.'
 endif else begin
    R = readfits(rec_file)
 endelse

 if ~keyword_set(rec_file) then begin 
    M = readfits(adsec_path.data+'m2c.fits') 
    print, 'A default modes to command matrix file is used.'
 endif else begin
    M = readfits(m2c_file)
 endelse

 

 if n_elements(mode) eq 0 then mode =  dblarr(n_true_act)+1./sqrt(n_true_act)
 if n_elements(mode) ne n_true_act then begin 
    print, 'Error in input mode.'
    return, adsec_error.datalenght_error
 endif

 if float(total(mode*mode)) ne 1.0 then begin
    print, 'Mode is not normalized'
    print, 'Error in input mode.'
    return, adsec_error.IDL_INPUT_TYPE
 endif       

 M_filt = M[*,adsec.true_act]
 M_filt = M_filt[0:n_true_act-1, *]
; test
 m_m = max((mode##M_filt), p_m)

 R_filt = R[*,0:n_true_act-1]

 ;cc45 = dblarr(n_act-3)+1./sqrt(n_act-3)
 err=complement([p_m], indgen(n_true_act), dd)
 MM_filt = dblarr(n_true_act, n_true_act)
 MM_filt[1:n_true_act-1,*] = M_filt[dd,*]
 MM_filt[0,*] = mode
 

 Btr=MM_filt

 P=make_orto_modes(Btr)

 ;=======================================
 D=diagonal_matrix([0,dblarr(44)+1.])
 RR = float(invert(M_filt)##P##D##transpose(P)##M_filt##R_filt)

;  svdc, R_filt, WR45, UR45, VR45
 ;test


;m145 = R_filt##vR45
;c145 = M_filt##m145

;m1145 = RR##vR45
;c1145 = M_filt##m1145
 R_new = R*0.0
 R_new[*,0:n_true_act-1] = RR
;stop
 return, R_new
end
