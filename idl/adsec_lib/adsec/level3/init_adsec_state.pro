; $Id: init_adsec_state.pro,v 1.3 2006/03/16 18:08:07 labot Exp $$
;
;+
;  NAME:
;   INIT_ADSEC_STATE
;
;  PURPOSE:
;   CREATE AND LOAD THE STRUCTURE OF THE CURRENT SET CONFIGURATION FILES.
;
;  USAGE:
;   err = INIT_ADSEC_STATE(state,STRUCT=struct, NOFILL=nofill)
;
;  INPUT:
;   state  : set the working state:  
;                       4 : Set/A0 Adaptive Optics operating mode, 68 um;
;                       5 : Set/SL Seeing Limited operating mode, 68um;
;                       6 : Set/Chop Chopping operating mode, 100um.
;            If not set the default state MBO(Manual Breaker off) is used.                           
;
;
;  OUTPUT:
;   error               : error code.
;
;  KEYWORDS:
;   STRUCT              : structure with parameters loaded and calculated.
;   NOFILL              : do not fill the adsec_state structure.
;
;  COMMON BLOCKS:
;   ADSEC_STATE        : adsec_state base common block. It will be filled here.
;
;  PACKAGE:
;   LBT Adsec Libraries.
;
;  HISTORY
;   Created by Daniela Zanotti(DZ) on January 2006.
;   Marco Xompero(MX), Added the keywords  REC_MAT_A_FILENAME,B_DELAY_A_FILENAME,                                 
;   G_GAIN_A_FILENAME,REC_MAT_B_FILENAME,B_DELAY_B_FILENAME,G_GAIN_B_FILENAME,A_DELAY_FILENAME, M2C_FILENAME
;
;-



Function init_adsec_state, set_state,   STRUCT=struct, NOFILL=nofill, $
                                        REC_MAT_A_FILENAME=rec_mat_a_filename, B_DELAY_A_FILENAME=b_delay_a_filename, $
                                        G_GAIN_A_FILENAME=g_gain_a_filename, REC_MAT_B_FILENAME=rec_mat_b_filename, $
                                        B_DELAY_B_FILENAME=b_delay_b_filename,  G_GAIN_B_FILENAME=g_gain_b_filename, $
                                        A_DELAY_FILENAME=a_delay_filename, M2C_FILENAME= m2c_filename ,$
                                        VERBOSE=verbose

; common blocks definition
@adsec_common



;numbering of the sates for ASM
MBO = 0         ; Manual-Breaker-off state
Power_off = 1   ; Power off state
Power_on = 2    ; Power on state
Operating = 3   ; Operating state
Set_AO = 4      ; Set/A0 Adaptive Optics operating mode, 68 um;
Set_SL = 5      ; Set/SL Seeing Limited operating mode, 68um;
Set_Chop = 6    ; Set/Chop Chopping operating mode, 100um.
Running_AO = 7  ; Running AO state
Running_Chop = 8; Running Chopping state 

;;=========================================================================
;; data structure loading
;;

if n_elements(set_state) eq 0 then default_state = 0 else default_state = set_state[0]
if total(default_state eq [MBO, set_ao, set_sl, set_chop]) ne 1 then $
     message, 'The Set State does not exist or has a wrong format.'
 
filestate = "configuration_"+strtrim(default_state,2)+".txt"
filename=filepath(ROOT=adsec_path.conf, filestate)


default = read_ascii_structure(filename, DEBUG=0)
if n_elements(default) eq 0 then begin
    message, 'The configuration file "'+filename+'" does not exist or has a wrong format.'
endif


;Defining the parameters to set
;mirror set configuration file .txt.
if keyword_set(mirror_set_filename) then mirror_set_par=mirror_set_filename $
                                    else mirror_set_par = adsec_path.conf+default.mirror_set  
;flatten position file .sav.
if keyword_set(flatten2apply_filename) then flatten2apply_par = flatten2apply_filename $
                                       else flatten2apply_par = adsec_path.data+default.flatten2apply 
;proportinal and derivative gain file .txt.
if keyword_set(ramp_gain_filename) then ramp_gain_par = ramp_gain_filename $
                                   else ramp_gain_par = adsec_path.data+default.ramp_gain 
;modes to command matrix file.
if keyword_set(m2c_filename) then m2c_par = m2c_filename $
                             else m2c_par = adsec_path.data+default.m2c_matrix
;file containing the B0 matrix.
if keyword_set(rec_mat_a_filename) then b0_a_matrix_par = rec_mat_a_filename $
                                   else b0_a_matrix_par = adsec_path.data+default.b0_a_matrix
;file containing the B0 matrix.
if keyword_set(rec_mat_b_filename) then b0_b_matrix_par = rec_mat_b_filename $
                                   else b0_b_matrix_par = adsec_path.data+default.b0_b_matrix
;file containing the A delay matrix.
if keyword_set(a_delay_filename) then a_delay_matrix_par = a_delay_filename $
                                 else a_delay_matrix_par = adsec_path.data+default.a_delay_matrix
;file containing the B delay matrix.    
if keyword_set(b_delay_a_filename) then b_delay_a_matrix_par = b_delay_a_filename $
                                   else b_delay_a_matrix_par = adsec_path.data+default.b_delay_a_matrix
;file containing the B delay matrix.    
if keyword_set(b_delay_b_filename) then b_delay_b_matrix_par = b_delay_b_filename $
                                   else b_delay_b_matrix_par = adsec_path.data+default.b_delay_b_matrix
;file containing the optical loop gain.
if keyword_set(g_gain_a_filename) then g_gain_a_par = g_gain_a_filename $
                                  else g_gain_a_par = adsec_path.data+default.g_gain_a
;file containing the optical loop gain.
if keyword_set(g_gain_b_filename) then g_gain_b_par = g_gain_b_filename $
                                  else g_gain_b_par = adsec_path.data+default.g_gain_b
;force calibration file.
if keyword_set(force_calib_filename) then force_calib_par = force_calib_filename $
                                     else force_calib_par = adsec_path.conf+default.force_calib
;sys_status at defined gap file.
if keyword_set(base_status_filename) then base_status_par = base_status_filename $
                                     else base_status_par = adsec_path.data+default.base_status
 

;;=========================================================================

struct =                                        $
    {                                           $
    state_current       : default_state,        $ ;Current Set state:(0) Set/A0,(1) Set/SL,(2) Set/Chop.
    mirror_set          : mirror_set_par,       $ ;mirror set configuration file .txt.
    flatten2apply       : flatten2apply_par,    $ ;flatten position file .sav.
    ramp_gain           : ramp_gain_par,        $ ;proportinal and derivative gain file .txt.
    m2c_matrix          : m2c_par,       $ ;modes to command matrix file.
    b0_a_matrix         : b0_a_matrix_par,      $ ;file containing the B0 matrix.
    b0_b_matrix         : b0_b_matrix_par,      $ ;file containing the B0 matrix.
    a_delay_matrix      : a_delay_matrix_par,   $ ;file containing the A delay matrix.
    b_delay_a_matrix    : b_delay_a_matrix_par, $ ;file containing the B delay matrix.    
    b_delay_b_matrix    : b_delay_b_matrix_par, $ ;file containing the B delay matrix.    
    g_gain_a            : g_gain_a_par,         $ ;file containing the optical loop gain.
    g_gain_b            : g_gain_b_par,         $ ;file containing the optical loop gain.
    force_calib         : force_calib_par,      $ ;force calibration file.
    base_status         : base_status_par,      $ ;sys_status at defined gap file.
    MBO                 : mbo,                  $ ;Manual-Breaker-off state
    Power_off           : power_off,            $ ;Power off state
    Power_on            : power_on,             $ ;Power on state
    Operating           : operating,            $ ;Operating state
    Set_AO              : set_ao,               $ ;Set/A0 Adaptive Optics operating mode, 68 um;
    Set_SL              : set_sl,               $ ;Set/SL Seeing Limited operating mode, 68um;
    Set_Chop            : set_chop,             $ ;Set/Chop Chopping operating mode, 100um.
    Running_AO          : running_ao,           $ ;Running AO state
    Running_Chop        : running_chop          $ ;Running Chopping state 

}

;===========================================================================
if ~ keyword_set(nofill) then begin
    adsec_state.mirror_set       = mirror_set_par        ;mirror set configuration file .txt.
    adsec_state.flatten2apply    = flatten2apply_par     ;flatten position file .sav.
    adsec_state.ramp_gain        = ramp_gain_par         ;proportinal and derivative gain file .txt.
    adsec_state.m2c_matrix       = m2c_par        ;modes to command matrix file.
    adsec_state.b0_a_matrix      = b0_a_matrix_par         ;file containing the B0 matrix.
    adsec_state.b0_b_matrix      = b0_b_matrix_par         ;file containing the B0 matrix.
    adsec_state.a_delay_matrix   = a_delay_matrix_par    ;file containing the A delay matrix.
    adsec_state.b_delay_a_matrix = b_delay_a_matrix_par    ;file containing the B delay matrix.
    adsec_state.b_delay_b_matrix = b_delay_b_matrix_par    ;file containing the B delay matrix.
    adsec_state.g_gain_a         = g_gain_a_par         ;file containing the optical loop gain.
    adsec_state.g_gain_b         = g_gain_b_par         ;file containing the optical loop gain.
    adsec_state.force_calib      = force_calib_par       ;force calibration file.
    adsec_state.base_status      = base_status_par       ;sys_status at defined gap file.

endif

return, adsec_error.ok

End
