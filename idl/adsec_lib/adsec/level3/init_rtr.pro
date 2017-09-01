;$Id: init_rtr.pro,v 1.3 2007/02/07 17:21:41 marco Exp $$
;
;+
; INIT_RTR
;
;  Initialize all the global variables/structure used in the adsec software for the RTR map settings.
;  See adsec_common.pro for a list of them.
;
; HISTORY
;    Updated the rtr structure, with a fied for the matrices of each memory block.
;    and the new optical loop gain field.
;   07 Feb 2007 MX
;       LUT of variable deleted and name refurbished.
;-
;
;================================
; initialize global variables
;================================
pro init_rtr, N_SLOPE=n_slope, N_SL_DL=n_sl_dl, N_DP_DL=n_dp_d

; common blocks definition
@adsec_common

;==========================================================================
; Environment configurations
;

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; the working directory is the current directory
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
cd, CURRENT=adsec_home

; check if the default directories exist
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
if not is_a_dir(adsec_path.data) then begin
    message, 'The AdSec data directory "'+adsec_path.data+'" does not exist.'
endif
if not is_a_dir(adsec_path.conf) then begin
    message, 'The AdSec data directory "'+adsec_path.conf+'" does not exist.'
endif
if not is_a_dir(adsec_path.prog) then begin
    message, 'The AdSec data directory "'+adsec_path.prog+'" does not exist.'
endif
if not is_a_dir(adsec_path.temp) then begin
    message, 'The AdSec directory "'+adsec_path.temp+'" does not exist.'
endif

;=========================================================================
b0_a_filename     = filepath(ROOT=adsec_path.data,'b0_a_matrix.sav')
b0_b_filename     = filepath(ROOT=adsec_path.data,'b0_b_matrix.sav')
; file containing the A_delay matrix
a_delay_filename     = filepath(ROOT=adsec_path.data,'a_delay_matrix.sav')
; file containing the B_delay matrix
b_delay_a_filename     = filepath(ROOT=adsec_path.data,'b_delay_a_matrix.sav')
b_delay_b_filename     = filepath(ROOT=adsec_path.data,'b_delay_b_matrix.sav')
; file containing the M2C matrix
m2c_filename     = filepath(ROOT=adsec_path.data,'m2c_matrix.sav')
; file containing the G_GAIN matrix
g_gain_a_filename     = filepath(ROOT=adsec_path.data,'g_gain_a_matrix.sav')
g_gain_b_filename     = filepath(ROOT=adsec_path.data,'g_gain_b_matrix.sav')
;===========================================================================

;===========================================================================
; Creating rtr sruct
;
err=init_rtr_par(ST=rtr_struct, /NOFILL,N_SLOPE=n_slope, N_SL_DL=n_sl_dl, N_DP_DL=n_dp_d )
if err ne adsec_error.ok then message,"The rtr structure was not initialized"
rtr=rtr_struct
;
; End of: Creating rtr struct
;===========================================================================
;===========================================================================
; Restoring the B0 matrix gain
;
err=init_rtr_b0(B0_A_FILE=b0_a_filename,B0_B_FILE=b0_b_filename, ST=b0_struct, /NOFILL)
if err ne adsec_error.ok then message,"The rtr B0  structure was not initialized"
rtr=create_struct(rtr,b0_struct)
;
; End of: Restoring the B0 matrix gain
;===========================================================================

;===========================================================================
; Restoring the A_delay  matrix gain
;
err=init_rtr_a_delay(A_DELAY_FILE=a_delay_filename, ST=a_mtrx_struct, /NOFILL)
if err ne adsec_error.ok then message,"The A_delay mtrx structure was not initialized"
rtr=create_struct(rtr,a_mtrx_struct)
;
; End of: Restoring the A_delay matrix
;===========================================================================
;===========================================================================
; Restoring the B_delay matrix gain
;
ierr=init_rtr_b_delay(B_A_DELAY_FILE=b_delay_a_filename,B_B_DELAY_FILE=b_delay_b_filename, ST=b_mtrx_struct, /NOFILL)
if err ne adsec_error.ok then message,"The B_delay  mtrx  structure was not initialized"
rtr=create_struct(rtr,b_mtrx_struct)
;
; End of: Restoring the  B_delay matrix gain
;===========================================================================

;===========================================================================
; Restoring the M2C matrix gain
;
err=init_rtr_m2c(M2C_FILENAME=m2c_filename, ST=m2c_struct, /NOFILL)
if err ne adsec_error.ok then message,"The m2C    structure was not initialized"
rtr=create_struct(rtr,m2c_struct)
; End of: Restoring the  M2C matrix
;===========================================================================
;===========================================================================
; Restoring the optical loop gain
;
err=init_rtr_g_gain(G_GAIN_A_FILE=g_gain_ai_filename, G_GAIN_B_FILE=g_gain_b_filename, ST=g_gain_struct, /NOFILL)
if err ne adsec_error.ok then message,"The optical loop gains was not initialized"
rtr=create_struct(rtr,g_gain_struct)
; End of: Restoring the Optical loop gains


; Setting all pointers
err = set_all_ptr()
if err ne adsec_error.ok then message,"The reconstructor was not initialized"
end
