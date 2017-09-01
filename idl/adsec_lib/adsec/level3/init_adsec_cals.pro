;+
;   NAME:
;    INIT_ADSEC_CALS    
;
;   PURPOSE:
;    Restore the measured force calibrations
;    Restore the capsens calibrations
;
;   USAGE:
;    err = init_adsec_cals(filename, COIL_RES_PATH=coil_res_path, ACT_EFF_PATH=act_eff_path,$
;                          A_PER_DACBIT_PATH=A_per_dacbit_path, DAC_OFFSET_PATH=dac_offset_path,$
;                          DO_PATH=d0_path,DO_SD_PATH=d0_sd_path,C_RATIO_PATH=c_ratio_path,$
;                          C_RATIO_PATH=c_ratio_sd_path)
;
;
;   INPUT:
;
;    filename : name of the configuration file with the calibration data (default file for LBT672).
;
;   OUTPUT:
;    Error code.
;
;   KEYWORD:
;    STRUCT              : structure with parameters loaded and calculated.
;    NOFILL              : do not fill the adsec structure.
;
;  COMMON BLOCKS:
;   ADSEC               : secondary adaptive base common block. A part of it will be filled.
;
;  PACKAGE:
;   LBT Adsec Libraries.
;
;  HISTORY
;   Created on 28 Oct 2004 by Daniela Zanotti (DZ).
;   
;   01/12/2004 Marco Xompero (MX)
;    Deleted from procedure the filling of adsec structure about forces calibrations and linearizations of position sensors.
;   27/05/2005 MX
;    Added alpha_pos configuration file for calibrating the position sensors
;
;   NOTE:
;    None.
;-
                                                                                                                             
function init_adsec_cals, filename,STRUCT=struct,NOFILL=nofill, COIL_RES_PATH=coil_res_path, ACT_EFF_PATH=act_eff_path,$
                          A_PER_DACBIT_PATH=A_per_dacbit_path, DAC_OFFSET_PATH=dac_offset_path,$
                          DO_PATH=d0_path,DO_SD_PATH=d0_sd_path,C_RATIO_PATH=c_ratio_path,$
                          C_RATIO_PATH=c_ratio_sd_path, ALPHA_POS_PATH=alpha_pos_path,FORCE_GAIN_PATH=force_gain_path,$
                          MAG_EFF_PATH=mag_eff_path     
                                                                                                                             
@adsec_common

if n_elements(filename) eq 0 then filename=filepath(Root=adsec_path.conf, SUB=!AO_CONST.shell, "cals.txt")
                                                                                                                             

                                                                                                                             
if file_search(filename) ne filename then begin
    message,'The configuraton file "'+filename+'" does not exist.', CONT = (sc.debug eq 0B)
    return, adsec_error.input_type
endif
                                                                                                                             
cals = read_ascii_structure(filename, DEBUG=0)

if n_elements(cals) eq 0 then begin
    message, 'The configuration file "'+filename+'" does not exist or has a wrong format.', CONT = (sc.debug eq 0B)
    return, adsec_error.input_type
endif

;======================================================================================================================

n_acts  = adsec.n_actuators
ring_radius = adsec_shell.ring_radius

; diagonal of mass matrix computation [g]
M_diag = dblarr(n_acts)
dens = adsec_shell.glass_mass/(adsec_shell.out_radius^2-adsec_shell.in_radius^2)
offset = 0
                                                                                                                             
for i=0,adsec_shell.n_rings-1 do begin
    if i eq 0 then begin
       in_r = adsec_shell.in_radius
       extra_mass = adsec_shell.membrane_ring_mass/adsec_shell.n_act_ring[0] ;; contribution of the membrane ring to the actuator mass
    endif else begin
       in_r = 0.5*(ring_radius[i]+ring_radius[i-1])
       extra_mass = 0.0
    endelse
                                                                                                                             
    if i eq adsec_shell.n_rings-1 then $
       out_r = adsec_shell.out_radius $
    else $
       out_r = 0.5*(ring_radius[i+1]+ring_radius[i])
    
    act_mass = dens*(out_r^2-in_r^2)/adsec_shell.n_act_ring[i]
    M_diag[offset] = replicate(act_mass, adsec_shell.n_act_ring[i])+adsec_shell.mag_mass+extra_mass
    offset = offset+adsec_shell.n_act_ring[i]
endfor

                                                                                                                             



; reordering the mass per actuator values (DSP ordering)
M_diag = M_diag[adsec.act2mir_act]
                                                                                                                             
; definition of the mass matrix
M = dblarr(n_acts,n_acts)
set_diagonal, M, M_diag

;===========================================================================
; Restoring the measured force calibrations
;

if keyword_set(coil_res_path) then begin
    coil_res_file = coil_res_path
endif else begin
coil_res_file     = filepath(ROOT=adsec_path.conf,SUB=!AO_CONST.shell, "coil_res.txt")
endelse

if keyword_set(act_eff_path) then begin
    act_eff_file  = act_eff_path
 endif else begin 
act_eff_file      = filepath(ROOT=adsec_path.conf,SUB=!AO_CONST.shell, "act_eff.txt")
endelse

if keyword_set(A_per_dacbit_path) then begin
    A_per_dacbit_file = A_per_dacbit_path
 endif else begin
A_per_dacbit_file = filepath(ROOT=adsec_path.conf,SUB=['general'], "A_per_dacbit.txt")
endelse

if keyword_set(dac_offset_path) then begin
    dac_offset_file = dac_offset_path 
endif else begin
dac_offset_file   = filepath(ROOT=adsec_path.conf,SUB=!AO_CONST.shell, "dac_offset.txt")
endelse

if keyword_set(force_gain_path) then begin
    force_gain_file = force_gain_path
endif else begin
    force_gain_file     = filepath(ROOT=adsec_path.conf,SUB=!AO_CONST.shell, "force_gain.txt")
endelse

if keyword_set(mag_eff_path) then begin
    mag_eff_file = mag_eff_path
endif else begin
    mag_eff_file     = filepath(ROOT=adsec_path.commons,SUB=!AO_CONST.shell, "mag_eff.txt")
endelse



coil_res     = read_ascii_array(coil_res_file)     ; [Ohm] coil resistance @ 20 Celsius
if n_elements(coil_res) eq 0 then begin
    message, 'The configuration file "'+coil_res_file+'" does not exist or has a wrong format.', CONT = (sc.debug eq 0B)
    return, adsec_error.input_type
endif

act_eff      = read_ascii_array(act_eff_file)      ; [N/sqrt(W)] actuator efficiency @ nominal_gap
if n_elements(act_eff) eq 0 then begin
    message, 'The configuration file "'+act_eff_file+'" does not exist or has a wrong format.', CONT = (sc.debug eq 0B)
    return, adsec_error.input_type
endif

A_per_dacbit = read_ascii_array(A_per_dacbit_file) ; [A/dacbit] amperes per current driver DAC bit nominal
if n_elements(A_per_dacbit) eq 0 then begin
    message, 'The configuration file "'+A_per_dacbit_file+'" does not exist or has a wrong format.', CONT = (sc.debug eq 0B)
    return, adsec_error.input_type
endif

dac_offset   = read_ascii_array(dac_offset_file)   ; [dacbit] DAC value to give zero current
if n_elements(dac_offset) eq 0 then begin
    message, 'The configuration file "'+dac_offset_file+'" does not exist or has a wrong format.', CONT = (sc.debug eq 0B)
    return, adsec_error.input_type
endif

force_gain   = read_ascii_array(force_gain_file)   ; [N/A] gain forces calibration 
if n_elements(force_gain) eq 0 then begin
    message, 'The configuration file "'+force_gain_file+'" does not exist or has a wrong format.', CONT = (sc.debug eq 0B)
    return, adsec_error.input_type
endif
                                                                                                                             
mag_efficiency   = read_ascii_array(mag_eff_file)   ; [N/A] gain forces calibration 
if n_elements(mag_efficiency) eq 0 then begin
    message, 'The configuration file "'+mag_eff_file+'" does not exist or has a wrong format.', CONT = (sc.debug eq 0B)
    return, adsec_error.input_type
endif
                                                                                                                             
;===========================================================================
; Restoring the capsens calibrations
;
if keyword_set(d0_path) then begin
    d0_filename  = d0_path
endif else begin
d0_filename         = filepath(ROOT=adsec_path.conf,SUB=!AO_CONST.shell, "d0.txt")
endelse

if keyword_set(d0_sd_path) then begin
    d0_sd_filename  = d0_sd_path
endif else begin
d0_sd_filename      = filepath(ROOT=adsec_path.conf,SUB=!AO_CONST.shell, "d0_sd.txt")
endelse

if keyword_set(c_ratio_path) then begin
    c_ratio_filename  = c_ratio_path
endif else begin
c_ratio_filename    = filepath(ROOT=adsec_path.conf,SUB=!AO_CONST.shell, "c_ratio.txt")
endelse

if keyword_set(c_ratio_sd_path) then begin
    c_ratio_sd_filename  = c_ratio_sd_path
endif else begin
c_ratio_sd_filename = filepath(ROOT=adsec_path.conf,SUB=!AO_CONST.shell, "c_ratio_sd.txt")
endelse

if keyword_set(alpha_pos_path) then begin
    alpha_pos_filename  = alpha_pos_path
endif else begin
alpha_pos_filename = filepath(ROOT=adsec_path.conf,SUB=!AO_CONST.shell, "alpha_pos.txt")
endelse


d0                  = read_ascii_array(d0_filename)     ;; [m] d0 used for the linearization
if n_elements(d0) eq 0 then begin
    message, 'The configuration file "'+d0_filename+'" does not exist or has a wrong format.', CONT = (sc.debug eq 0B)
    return, adsec_error.input_type
endif

d0_sd               = read_ascii_array(d0_sd_filename)  ;; [m] d0 error
if n_elements(d0_sd) eq 0 then begin
    message, 'The configuration file "'+d0_sd_filename+'" does not exist or has a wrong format.', CONT = (sc.debug eq 0B)
    return, adsec_error.input_type
endif

c_ratio             = read_ascii_array(c_ratio_filename)    ;; c_stray/c_ref used for the linear.
if n_elements(c_ratio) eq 0 then begin
    message, 'The configuration file "'+c_ratio_filename+'" does not exist or has a wrong format.', CONT = (sc.debug eq 0B)
    return, adsec_error.input_type
endif

c_ratio_sd          = read_ascii_array(c_ratio_sd_filename) ;; c_stray/c_ref error
if n_elements(c_ratio_sd) eq 0 then begin
    message, 'The configuration file "'+c_ratio_sd_filename+'" does not exist or has a wrong format.',CONT = (sc.debug eq 0B)
    return, adsec_error.input_type
endif

alpha_pos          = read_ascii_array(alpha_pos_filename) ;; position calibration coeff
if n_elements(alpha_pos) eq 0 then begin
    message, 'The configuration file "'+alpha_pos_filename+'" does not exist or has a wrong format.',CONT = (sc.debug eq 0B)
    return, adsec_error.input_type
endif
;
;
;===============================================================================================================
c4bm = read_ascii_array(filepath(Root=adsec_path.conf, SUB=!AO_CONST.shell, "curr4bias_mag.txt"))

struct = $
  { $
    d0 : d0, $ ; [um] nominal distance where cvar=cref when c_ref=0
    c_ratio : c_ratio ,$ ; ratio between cstray and cref. CANNOT BE 0.0 for MMT!!!!
    d0_sd : d0_sd ,$ ; d0 measured std. deviations [um]
    c_ratio_sd : c_ratio_sd ,$ ; c_ratio measured std. deviations
    alpha_pos : alpha_pos, $  ; coefficient for position calibration
    max_delta_dist : cals.max_req_delta_lin_dist ,$ ; max delta dist for linearization [um]
    mag_ref : float(cals.mag_ref_efficiency) ,$ ; reference magnet efficiency [Ohm/sqrt(W)]
    coil_res : coil_res ,$        ; coil resistance, in ohms
    act_efficiency : act_eff ,$ ; actuator efficiency [N/sqrt(W)]
    A_per_dacbit : A_per_dacbit ,$ ; amperes per dacbit [A/dacbit]
    dac_offset : dac_offset ,$ ; [dacbit] DAC value to give zero current of current driveri
    force_gain : force_gain,$  ; [N/A] force gain to compute the DAC_N2A_gain
    M : M ,$   ; Mass matrix [g] 
    curr4bias_mag  :   c4bm ,$ ; vector of ratio between bias coil currents to balance bias magnet force
    mag_efficiency  :   mag_efficiency ,$ ; vector of magnets efficiency [Ohm/sqrt(W)]
    max_ngain_step :   cals.max_ngain_step $ ; max allowed step of the normalized gain to be written in one shot
 }

;==================adsec calibration initizialzation==========================
;
; End of: Restoring the capsens calibrations
;*
    ;;;*** Capacitive Sensors ***
    
if not(keyword_set(nofill)) then begin

    adsec.d0 = d0 ; [um] nominal distance where cvar=cref when c_ref=0
    adsec.c_ratio = c_ratio ; ratio between cstray and cref. CANNOT BE 0.0 for MMT!!!!
    adsec.d0_sd = d0_sd ; d0 measured std. deviations [um]
    adsec.c_ratio_sd = c_ratio_sd ; c_ratio measured std. deviations
    adsec.alpha_pos = alpha_pos ; coefficient for calibration
    adsec.max_delta_dist = cals.max_req_delta_lin_dist ; max delta dist for linearization [um]
    adsec.mag_ref = float(cals.mag_ref_efficiency) ; reference magnet efficiency [Ohm/sqrt(W)]
;;;*** DSPs ***

    adsec.coil_res = coil_res        ; coil resistance, in ohms
    adsec.act_efficiency = act_eff ; actuator efficiency [N/sqrt(W)]
    adsec.A_per_dacbit = A_per_dacbit ; amperes per dacbit [A/dacbit]
    adsec.dac_offset = dac_offset ; [dacbit] DAC value to give zero current of current driver
    adsec.force_gain =force_gain ; [N/A] force gain to compute the DAC_N2A_gain
    adsec.M = M   ; Mass matrix [g]
    adsec.curr4bias_mag  =   c4bm ; vector of ratio between bias coil currents to balance bias magnet force
    adsec.max_ngain_step =   cals.max_ngain_step ; max allowed step of the normalized gain to be written in one shot
    adsec.mag_efficiency =   mag_efficiency      ;magnets efficiency [Ohm/sqrt(W)]
endif

return, adsec_error.ok
end
