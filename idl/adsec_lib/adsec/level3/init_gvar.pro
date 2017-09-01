; $Id: init_gvar.pro,v 1.29 2004/11/29 14:59:33 labot Exp $

;+
; INIT_GVAR
;
;  Initialize all the global variables/structure used in the adsec software.
;  See adsec_common.pro for a list of them.
;
; HISTORY
;
;  created by A. Riccardi
;  Osservatorio Astrofisico di Arcetri, ITALY
;  <riccardi@arcetri.astro.it>
;
;  13 Mar 2002, AR
;    *The configuration of adsec_model,adsec_prog_ver,adsec_comm,
;     host_comm_default,vme_location and debug are no more defined
;     inside here.
;    *adsec_model='672a' is now supported
;
;  15 Apr 2003, AR
;    *adsec_model = 'P45' is now supported
;    *Odd number of actuators (half DSP) is now well managed (n_dsp computation)
;    *adsec.dummy_act has been introduced
;
;  12 Aug 2003, AR
;    *new adsec fields: n_dummy_act, true_act, n_true_act
;
;  31 Jan 2004, AR
;    *structure conf_gvar and array act2mir_act are restored
;     from ascii files (see read_ascii_structure and
;     read_ascii_array) instead of less portable IDL .sav files
;    *act2mir_act file definition moved from data to conf directory
;
;  03 Feb 2004, AR and M. Xompero
;    *act_wo_pos and act_wo_curr are now set using conf_gvar structure
;    *nominal_gap and curr4bias_mag added in adsec structure. The contents
;     is read from conf_gvar structure
;
;  17 Feb 2004, MX
;    *secure_factor is now set using conf_gvar structure
;
;  10 Mar 2004, AR
;    *new fields in adsec structure: n_bcu, n_board_per_bcu,
;     n_dsp_per_board, n_act_per_dsp. Their content is read from
;     conf_gvar.txt configuration file.
;    *adsec.n_crates is now read from conf_gvar.txt
;    *init_error_code call moved at the beginning of init_gvar
;    *patch structure initialization
;
;   08 Apr 2004, MX
;     *new fields in sc structure: field_port, fp_config pointer
;     *new structure: env_status for the monitoring of the enviroment
;
;  08 Apr 2004, AR
;    *adsec.n_dsp and adsec.last_dsp are computed considering n_act_per_dsp
;    *adsec.all_boards and adsec.all added
;
;   04 May 2004, MX
;     *adsec.ll_debug added in sc structure.
;
;  27 May 2004, AR
;    *structure fields added/modified:
;     max_ngain_step, min_timeout, dac_bits, adc_bits,
;     capsens_vref, capsens_vmax, pu_str, min_volt_val, max_volt_val,
;     fu_str, lu_str, nominal_gap, max_req_delta_lin_dist,
;     offset_delta_pos, A_per_dacbit, dac_offset, d0_sd, c_ratio_sd,
;     max_ngain_step, err_par, sampling_time, tv_sens_unit_list,
;     extra_ct.
;    *computation of force calibration coeffs using send_force_calibration
;    *modified data type to match LBT formats
;
;  14 Jul 2004
;     *pos_sat_force, neg_sat_force,  err_smoothed_step and speed_smoothed_step added
;
;  22 Jul 2004, MX
;     adsec.ff2bias_step field added.
;  28 Jul 2004, MX
;     adsec.curr_threshold filed added.
;  29 jul 2004 D.Zanotti(DZ)
;     adsec.max_amp_pos,max_amp_pos_ramp,max_deltacurr,max_gain_step,
;     min_gain_step,max_curr_tospread,max_iter_integ, thr_perr,max_curr
;  30 jul 2004, DZ
;     fact_reduction_curr added in adsec structure.
;     adsec.gain_to_bbgain is now float (previously was double)
;  03 Aug 2004, MX
;     max_dyn_peak_pos added in adsec. It's max position peak performed in the optimized
;     dynamical response. max_modal_abs_step added in adsec. it's the maximum modal position step.
;  13 Aug 2004, AR
;     tv_curr_unit_list changed (N -> mN)
;  29 Nov 2004,
;     changed default values of temporal filters
;
;-
;================================
; initialize global variables
;================================
pro init_gvar

; common blocks definition
@adsec_common

;==========================================================================
; Environment configurations
;

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; the working directory is the current directory
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
cd, CURRENT=adsec_home

;;;;;;;;;;;;;;;;;;;;;;;
; default directories
;;;;;;;;;;;;;;;;;;;;;;;
adsec_path =                                           $
  {                                                    $
    home: adsec_home,                                  $
    data: filepath(ROOT=adsec_home, SUB=['data'], ''), $ ; data file dir.
    conf: filepath(ROOT=adsec_home, SUB=['conf'], ''), $ ; config. file dir.
    prog: filepath(ROOT=adsec_home, SUB=['prog'], ''), $ ; dsp progs dir
    temp: filepath(ROOT=adsec_home, SUB=['temp'], ''), $ ; dir of temporary files
    meas: filepath(ROOT=adsec_home, SUB=['meas'], '')  $ ; dir of measurements files
  }

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
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

;;=========================================================================
;; error code definition
;;
init_error_codes
;;=========================================================================

;;=========================================================================
;; patch structure loading
;;
filename=filepath(ROOT=adsec_path.conf, "patches.txt")
patch = read_ascii_structure(filename, DEBUG=0)
if n_elements(patch) eq 0 then begin
    message, 'The configuration file "'+filename+'" does not exist or has a wrong format.'
endif
;;=========================================================================

;;=========================================================================
;; conf_gvar structure loading
;;
filename=filepath(ROOT=adsec_path.conf, "conf_gvar.txt")
conf_gvar = read_ascii_structure(filename, DEBUG=0)
if n_elements(conf_gvar) eq 0 then begin
    message, 'The configuration file "'+filename+'" does not exist or has a wrong format.'
endif
;;=========================================================================

;===========================================================================
; variables in structure conf_gvar
;
;;adsec_model=''      ; adaptive secondary model selection
;;adsec_prog_ver=''   ; release of the program running on the DSPs in the AS crates
;;adsec_comm=''       ; realese of the program running on the communication logics in the AS crates
;;host_comm_list = '' ; list of allowed external host communication boards
;;dll_name_list = ''  ; corresponding DLL to use
;;host_comm_default=0 ; index of the default host communication board
;;vme_location=0B     ; location of the VME containing the DMIFB (Used only when SwiftNet communication is selected)
;;debug = 1B          ; execute (1B) or don't execute (0B) the program in debug mode
;;;
;;; LEGEND:
;;; OA = Optical Axis
;;; AR = Actuator Ring
;;; CoC = Center of Curvature
;;; RoC = Radius of Curvature
;;;
;;glass_mass         = 0.0 ;; mass of the glass (thin mirror) [g]
;;mag_mass           = 0.0 ;; mass of one magnet+glass puck+Al ring [g]
;;membrane_ring_mass = 0.0 ;; mass of the outer ring of the central membrane
;;curv_radius        = 0.0 ;; RoC back thin shell [mm]
;;n_rings            = 0   ;; # of AR
;;in_radius          = 0.0 ;; inner radius of the mirror [mm]
;;out_radius         = 0.0 ;; outer radius of the mirror [mm]
;;vertex_thickness   = 0.0 ;; thickness at the vertex [mm]
;;force_sign2pull    = 0   ;; sign of force to pull the shell against the backplate (1 or -1)
;;n_crates           = 0   ;; number of crates for the DSP boards = number of communication boards
;;dummy_act          = [0] ;; dummy actuators (existing DSP channel without real actuators, [-2]=none)
;;weight_curr        = 0   ;; abs(current) to support the weight per actuator
;;n_act_ring         = [0] ;; # of acts per ring
;;angle_ring         = [0.0] ;; angle between the OA and the
;;                      ;; AR centered in the CoC [deg]
;;angle_act0         = [0.0] ;; azimuthal angle of the first act



;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; Communication type selection
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;
; default communication type saved in conf_gvar.host_comm_default
; start the widget for selection of the external host communication board
comm_type = xcomm_select(conf_gvar.host_comm_list, DEF=conf_gvar.host_comm_default)

dll_name = conf_gvar.dll_name_list[comm_type]
host_comm = conf_gvar.host_comm_list[comm_type]

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; Location of the realtime master computer (SwiftNet case only)
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;
if strlowcase(host_comm) ne 'swiftnet' then begin
       sn_node = ""    ;; VME node name
       sn_board = ""   ;; Pentek board name
endif else begin
    case conf_gvar.vme_location of
       0B: begin
         ;; Steward
         sn_node = "vw8"    ;; VME node name
         sn_board = "SSC"   ;; Pentek board name
       end

       1B: begin
         ;; Mirror Lab
         sn_node = "aoccm"
         sn_board = "AMC"
       end

       2B: begin
         ;; MMT
         sn_node = "????"
         sn_board = "AMC"
       end

       else: message, "Unexpected VME location"
    endcase
endelse


;;;;;;;;;;;;;;;;;;;;;;;;;;
; system constants
;;;;;;;;;;;;;;;;;;;;;;;;;;
sc =                        $
  {                         $
    lf        :      [10B], $   ; line feed
    cr        :      [13B], $   ; carriage return
    ctrlc     :       [3B], $   ; control-c
    true      :         1B, $   ; logical true
    false     :         0B, $   ; logical false
    ;;
    debug     :      conf_gvar.debug, $   ; 1B/0B run routines in debug mode if 1B
     ll_debug  :   conf_gvar.ll_debug, $
    name_comm : 'idl_comm', $   ; name of the communication process
    busy_comm :         0B, $   ; freedom of communication flag
     field_port:{          $
     busy:        0B, $   ; flag busy field point communication
     idl_port_id:       0B, $   ; file port opened under idl
     field_com : conf_gvar.field_com, $       ; type of connection with field point
    ip_address: conf_gvar.field_ip, $ ; structure that dfin the socket port
     port: conf_gvar.field_port $
     },                          $
     fp_config:       ptr_new(/alloc),$
    ;;
    model     :conf_gvar.adsec_model, $   ; model of adaptive secondary mirror ("P36","S336","A336","P45","A672")
    prog_ver  :conf_gvar.adsec_prog_ver, $; version of the DSP program for the specified model
    host_comm :  host_comm, $   ; external host computer communication board type ("PCIHotLink", "PCIHotLinkBug", "VMEHotLink", "NoBoard", "SwiftNet")
    adsec_comm: conf_gvar.adsec_comm, $   ; adsec crate communication board type ("L001", "L002", "L003", ...)
    dll_name  :   dll_name, $   ; dll name vector (dummy, PCIHotLink, SwiftNet)
    swiftnet  : {node:  sn_node,  $ ; VME node name
                 board: sn_board},$ ; Pentek board name
    min_timeout: conf_gvar.min_timeout $ ; minimum communication timeout [ms]
  }

selected_acts = [-1]   ;; any actuator currently selected

;
; End of the Environment configurations
;==========================================================================




;===========================================================================
; Definition of the mirror parameters
; LEGEND:
; OA = Optical Axis
; AR = Actuator Ring
; CoC = Center of Curvature
; RoC = Radius of Curvature
;
; radius of each ring (measured on the curved surface)
ring_radius = conf_gvar.curv_radius*(!PI/180.0)*conf_gvar.angle_ring


if (ring_radius[0] lt conf_gvar.in_radius) $
  or (ring_radius[conf_gvar.n_rings-1] gt conf_gvar.out_radius) then begin
    message, "Error in the mirror inner/outer radius definition."
endif


n_true_acts  = fix(total(conf_gvar.n_act_ring))          ;; total number of physical actuators

if conf_gvar.dummy_act[0] lt 0 then begin
    n_dummy_acts = 0
endif else begin
    n_dummy_acts=fix(n_elements(conf_gvar.dummy_act))
endelse

n_acts = n_true_acts+n_dummy_acts             ;; total number of dsp channels

if n_dummy_acts eq 0 then begin
    true_acts=indgen(n_acts)
endif else begin
    err = complement(conf_gvar.dummy_act, indgen(n_acts), true_acts)
endelse


mass = conf_gvar.glass_mass+conf_gvar.mag_mass*n_true_acts+conf_gvar.membrane_ring_mass ;; mirror mass + magnets + central membrane [g] 336

; diagonal of mass matrix computation [g]
M_diag = dblarr(n_acts)
dens = conf_gvar.glass_mass/(conf_gvar.out_radius^2-conf_gvar.in_radius^2)
offset = 0

for i=0,conf_gvar.n_rings-1 do begin
    if i eq 0 then begin
       in_r = conf_gvar.in_radius
       extra_mass = conf_gvar.membrane_ring_mass/conf_gvar.n_act_ring[0] ;; contribution of the membrane ring to the actuator mass
    endif else begin
       in_r = 0.5*(ring_radius[i]+ring_radius[i-1])
       extra_mass = 0.0
    endelse

    if i eq conf_gvar.n_rings-1 then $
       out_r = conf_gvar.out_radius $
    else $
       out_r = 0.5*(ring_radius[i+1]+ring_radius[i])
    act_mass = dens*(out_r^2-in_r^2)/conf_gvar.n_act_ring[i]
    M_diag[offset] = replicate(act_mass, conf_gvar.n_act_ring[i])+conf_gvar.mag_mass+extra_mass
    offset = offset+conf_gvar.n_act_ring[i]
endfor


act_coord = fltarr(2, n_acts)          ;; x,y coords of the acts

n_act_names = 3                         ;; number of formats for the actuator names
                                        ;; 0 = internal numbering (0...35)
                                        ;; 1 = mechanical numbering (1...36)
                                        ;; 2 = RAA format
act_name = strarr(n_acts, n_act_names)  ;; actuator name: "RAA"
                                        ;; R =ring number (0-9)
                                        ;; AA=act number in the ring (00-XX)

if conf_gvar.n_rings gt 10 then $
    ring_format_str='(I2.2)' $
else $
    ring_format_str='(I1.1)'
if conf_gvar.n_act_ring[conf_gvar.n_rings-1] gt 100 then $
    act_format_str='(I3.3)' $
else $
    act_format_str='(I2.2)'

act0 = 0
for nr=0,conf_gvar.n_rings-1 do begin

    ;; azimuthal angle of the acts in the ring nr
    theta = findgen(conf_gvar.n_act_ring[nr])*(360.0/conf_gvar.n_act_ring[nr])+conf_gvar.angle_act0[nr]
    theta = !PI/180.0*transpose(theta)
    ;; act x,y coordinates
    act_coord[0,act0] = ring_radius[nr]*[cos(theta), sin(theta)]

    ;; act names
    act_name[act0,0] = strtrim(indgen(conf_gvar.n_act_ring[nr])+act0, 2)
    act_name[act0,1] = strtrim(indgen(conf_gvar.n_act_ring[nr])+act0, 2)
    act_name[act0,2] = string(nr,FORMAT=ring_format_str) $
      + string(indgen(conf_gvar.n_act_ring[nr]),FORMAT=act_format_str)

    act0 = act0+conf_gvar.n_act_ring[nr]
endfor
if n_dummy_acts ne 0 then begin
    act_name[act0:*,0] = strtrim(indgen(n_dummy_acts)+act0, 2)
    act_name[act0:*,1] = "XX"
    act_name[act0:*,2] = "XX"
endif

;
; End of the Definition of the mirror parameters
;===========================================================================


;===========================================================================
; Definition of main parameters
;
dac_bits = conf_gvar.dac_bits         ; number of bits of current driver DACs
adc_bits = conf_gvar.adc_bits         ; number of bits of capsens ADCs
capsens_vref = conf_gvar.capsens_vref ; [V] PtV voltage of capsens reference signal
capsens_vmax = conf_gvar.capsens_vmax ; cap. sensor max output voltage [volt]
V_per_pcount = float(capsens_vmax/2d0^adc_bits); volts per no-linearized position count [V/pu], pu=adcbit
pu_str = "adcbit"
min_volt_val= 0.0            ; minimum voltage value in internal potential units [pu]
max_volt_val= 2.0^adc_bits-1 ; maximum voltage value in internal potential units [pu]

fu_str = conf_gvar.fu_str
lu_str = conf_gvar.lu_str
nominal_gap = conf_gvar.nominal_gap   ; linearized pcounts at which the mirror usually works

max_req_delta_lin_dist = conf_gvar.max_req_delta_lin_dist ;[um] required maximum linearized gap (only for MMT)

offset_delta_pos = conf_gvar.offset_delta_pos     ; offset to add to the delta position command in order
                                                  ; to reduce the noise induced by the communication

;
; End of the Definition of main parameters
;===========================================================================


;===========================================================================
; Definition of the filenames containing the initialization data

; file containing the feedforward matrix ff_matrix
ff_filename     = filepath(ROOT=adsec_path.data,'ff_matrix.sav')
; file containing the position calibration constants
calib_filename = filepath(ROOT=adsec_path.data,'calibration.sav')
; file containing the force calibration constants
f_calib_filename     = filepath(ROOT=adsec_path.data,'f_calibration.sav')
; file containing the time filter (tfl) data
tfl_data_filename = filepath(ROOT=adsec_path.data,'tfl_data.sav')
; file containing the optical calibration constants
optical_filename = filepath(ROOT=adsec_path.data,'optical.sav')
; file containing the look-up table to convert the DSP channel numbers to
; actuator (aka mechanical or mirror) numbers
act2mir_act_filename = filepath(ROOT=adsec_path.conf,'act2mir_act.txt')

;===========================================================================
; Restoring the look-up table to convert the DSP actuator numbers to
; mirror geometry actuator numbers
;
check = findfile(act2mir_act_filename)
if check[0] eq "" then begin
    message, 'The file '+act2mir_act_filename+ $
      " containing the act to mir_act no. lookup-table does not exist.", /INFO
    print, 'The mirror actuator numbers are set to DSP actuator numbers'
    act2mir_act = indgen(n_acts)
    mir_act2act = indgen(n_acts)
endif else begin
    act2mir_act = read_ascii_array(act2mir_act_filename)
    if test_type(act2mir_act, /INT, DIM=dim) then $
      message, "The act2mir_act look-up table must be short integer"
    if dim[0] ne 1 then $
      message, "The act2mir_act look-up table must be a vector"
    if total(dim eq [1, n_acts]) ne 2 then $
      message, "The act2mir_act look-up table vector must have " $
        +strtrim(n_acts,2)+" elements."

    mir_act2act = fix(sort(act2mir_act))

endelse
;
; End of: Restoring the dsp_ch2act look-up table
;===========================================================================

; Reordering the coordinates following the DSP ordering
act_coord = act_coord[*,act2mir_act]
act_name[*,1:2] = act_name[act2mir_act,1:2]

; reordering the mass per actuator values (DSP ordering)
M_diag = M_diag[act2mir_act]

; definition of the mass matrix
M = dblarr(n_acts,n_acts)
set_diagonal, M, M_diag

; actuators that cannot apply current: act_wo_curr
; (act_wo_curr=[-2] means all actuators can apply current)
;

act_wo_curr = conf_gvar.act_wo_curr

; actuators that cannot apply read the position: act_wo_pos
; (act_wo_pos=[-2] means all actuators can read the position)
;
act_wo_pos = conf_gvar.act_wo_pos

if conf_gvar.dummy_act[0] ne -2 then begin
    if (act_wo_curr[0] eq -2) then act_wo_curr=conf_gvar.dummy_act else act_wo_curr=[act_wo_curr,conf_gvar.dummy_act]
    if (act_wo_pos[0] eq -2) then act_wo_pos=conf_gvar.dummy_act else act_wo_pos=[act_wo_pos,conf_gvar.dummy_act]
endif
act_wo_curr = act_wo_curr[UNIQ(act_wo_curr, SORT(act_wo_curr))]
act_wo_pos = act_wo_pos[UNIQ(act_wo_pos, SORT(act_wo_pos))]

; find the actuators that can apply current
; and store them in act_w_curr
if act_wo_curr[0] ge 0 then begin
    err = complement(act_wo_curr, indgen(n_acts), act_w_curr)
endif else begin
    act_w_curr = indgen(n_acts)
endelse

; find the actuators that can read the position
; and store them in act_w_pos
act_wo_pos = act_wo_pos[uniq(act_wo_pos,sort(act_wo_pos))]
if act_wo_pos[0] ge 0 then begin
    err = complement(act_wo_pos, indgen(n_acts), act_w_pos)
endif else begin
    act_w_pos = indgen(n_acts)
endelse

; find the actuators that can close the loop (i.e. can apply current
; AND can read the position) and store them in act_w_cl
err = intersection(act_w_pos,act_w_curr,act_w_cl)
if err ne 0 then message,"There are no actuator available to close the loop"
err = complement( act_w_cl, indgen(n_acts), act_wo_cl, count)
if (count eq 0) or (err ne 0) then act_wo_cl=[-2]
;
;
;===========================================================================
;



;===========================================================================
; Restoring the feedforward matrix
;
check = findfile(ff_filename)
if check[0] eq "" then begin
    message, 'The file '+ff_filename+ $
      " containing the feedforward matrix doesn't exist", /INFO
    print, 'A zero matrix will be used as feedforward matrix'
    meas_ff_used = 0B
    ff_matrix = fltarr(n_acts,n_acts)
    u = -(identity(n_acts))
    v = -u
    w = replicate(0.0, n_acts)
endif else begin
    ff_matrix = 0 & u = 0 & w = 0 & v = 0 & act_w_ff=0
    restore, ff_filename
    meas_ff_used = 1B
    n_act_w_ff = n_elements(act_w_ff)

    if test_type(ff_matrix, /REAL, DIM=dim) then $
      message, "The feedforward matrix must be real"
    if dim[0] ne 2 then $
      message, "The feedforward must be a 2-D matrix"
    if total(dim eq [2, n_act_w_ff, n_act_w_ff]) ne 3 then $
      message, "The feedforward matrix must be a " $
        +strtrim(n_act_w_ff,2)+"x"+strtrim(n_act_w_ff,2)+" matrix."

    if test_type(u, /REAL, DIM=dim) then $
      message, "The u matrix must be real"
    if dim[0] ne 2 then $
      message, "The u must be a 2-D matrix"
    if total(dim eq [2, n_act_w_ff, n_act_w_ff]) ne 3 then $
      message, "The u matrix must be a " $
        +strtrim(n_act_w_ff,2)+"x"+strtrim(n_act_w_ff,2)+" matrix."

    if test_type(v, /REAL, DIM=dim) then $
      message, "The v matrix must be real"
    if dim[0] ne 2 then $
      message, "The v must be a 2-D matrix"
    if total(dim eq [2, n_act_w_ff, n_act_w_ff]) ne 3 then $
      message, "The v matrix must be a " $
        +strtrim(n_act_w_ff,2)+"x"+strtrim(n_act_w_ff,2)+" matrix."

   if test_type(w, /REAL, DIM=dim) then $
      message, "The v matrix must be real"
    if dim[0] ne 1 then $
      message, "The w must be a vector"
    if total(dim eq [1, n_act_w_ff]) ne 2 then $
      message, "The w must be a " $
        +strtrim(n_act_w_ff,2)+" elements vector."

    if n_elements(act_w_ff) lt n_elements(act_w_cl) then  $
      message, "The # of act in closed loop cannot be greater than the # of act with FF"
    
    is_not_included = complement(act_w_cl, act_w_ff, act_wo_ff, n_act_wo_ff)
    if is_not_included then message, "the list of act with closed loop must be included the list of act with FF"
    if n_act_wo_ff ne 0 then begin
        message, "The actuators in closed loop are less then the FF ones", /INFO
        message, "The FF matrix will be reduced.", /INFO
        idx_list = lonarr(n_act_wo_ff)
        for i=0,n_act_wo_ff-1 do idx_list[i]=where(act_w_ff eq act_wo_ff[i])
        ff_matrix = ff_matrix_reduction(ff_matrix, idx_list)
        act_w_ff = act_w_cl
        svdc, ff_matrix, w, u, v, /DOUBLE

        idx = sort(w)

        ;; the singular vector are ordered from the less to the most "stiff"
        u = u[idx, *]
        v = v[idx, *]
        w = w[idx]

        answ = dialog_message(["The ff_matrix has been changed." $
                              ,"Do you want to save the new one?"], /QUEST)
        if strlowcase(answ) eq "yes" then begin
            filename = dialog_pickfile(FILTER="*.sav", /WRITE, TIT="Select new FF matrix file")
            if filename ne "" then save, ff_matrix, u, v, w, act_w_ff, FILE=filename
        endif
    endif

    dummy_ff_matrix = fltarr(n_acts,n_acts)
    for i=0,n_elements(act_w_cl)-1 do begin
        dummy_ff_matrix[act_w_cl,act_w_cl[i]] = ff_matrix[*,i]
    endfor
    ff_matrix=dummy_ff_matrix

    dummy = dblarr(n_acts,n_acts)
    dummy[0:n_elements(act_w_cl)-1,act_w_cl]=u
    u=dummy

    dummy = dblarr(n_acts,n_acts)
    dummy[0:n_elements(act_w_cl)-1,act_w_cl]=v
    v=dummy

    dummy=dblarr(n_acts)
    dummy[0:n_elements(act_w_cl)-1]=w
    w=dummy

endelse
;
; End of: Restoring the feedforward matrix
;===========================================================================



;===========================================================================
; Restoring the measured force calibrations
;
coil_res_file     = filepath(ROOT=adsec_path.conf, "coil_res.txt")
act_eff_file      = filepath(ROOT=adsec_path.conf, "act_eff.txt")
A_per_dacbit_file = filepath(ROOT=adsec_path.conf, "A_per_dacbit.txt")
dac_offset_file   = filepath(ROOT=adsec_path.conf, "dac_offset.txt")
coil_res     = read_ascii_array(coil_res_file)     ; [Ohm] coil resistance @ 20 Celsius
act_eff      = read_ascii_array(act_eff_file)      ; [N/sqrt(W)] actuator efficiency @ nominal_gap
A_per_dacbit = read_ascii_array(A_per_dacbit_file) ; [A/dacbit] amperes per current driver DAC bit
dac_offset   = read_ascii_array(dac_offset_file)   ; [dacbit] DAC value to give zero current

;;;N_per_ccount = replicate(1.0,n_acts)                ; N per internal force unit (ccount) @ nominal_gap
;;;A_per_ccount = N_per_ccount/(act_eff*sqrt(coil_res)); Ampere per internal force unit (ccount)
;;;
;;;n_f_calib_coeff = 3                ; maximum number of coeff for the force calibration vs distance
;;;f_calib_coeff = fltarr(n_f_calib_coeff, n_acts) ; coefficients of polynomial giving the dependence
;;;                                                ; of N/ccounts vs. linearized distance
;;;                                                ; (N/ccount)[i] = poly(pcounts[i], f_calib_coeff[*,i])
;;;
;;;check = findfile(f_calib_filename)
;;;if check[0] eq "" then begin
;;;    message, 'The file '+f_calib_filename+ $
;;;      " containing the force calibration data doesn't exist", /INFO
;;;    print, 'Default values for f_calib_coeff will be used'
;;;
;;;    f_calib_coeff[0,*] = N_per_ccount  ; constant default values: @ 20 Celsius @ nominal_gap
;;;
;;;endif else begin
;;;
;;;    restore, calib_filename ; restoring the variable f_calib_coeff
;;;
;;;    if test_type(f_calib_coeff, /REAL, DIM=dim) then $
;;;      message, "f_calib_coeff matrix must be real"
;;;    if dim[0] ne 2 then $
;;;      message, "The f_calib_coeff matrix must be a 2-D matrix"
;;;    n_col = n_f_calib_coeff
;;;    if total(dim eq [2, n_col, n_acts]) ne 3 then $
;;;      message, "The f_calib_coeff matrix must be a " $
;;;        +strtrim(n_col,2)+"x"+strtrim(n_acts,2)+" matrix."
;;;
;;;    for i_act=0,n_acts-1 do $
;;;        N_per_ccount[i_act] = poly(nominal_working_distance, f_calib_coeff[*,i_act])
;;;    act_eff = N_per_ccount/A_per_ccount/sqrt(coil_res)
;;;endelse
;
; End of: Restoring the measured calibrations
;==========================================================================


;===========================================================================
; Restoring the capsens calibrations
;
d0_filename         = filepath(ROOT=adsec_path.conf, "d0.txt")
d0_sd_filename      = filepath(ROOT=adsec_path.conf, "d0_sd.txt")
c_ratio_filename    = filepath(ROOT=adsec_path.conf, "c_ratio.txt")
c_ratio_sd_filename = filepath(ROOT=adsec_path.conf, "c_ratio_sd.txt")
d0                  = read_ascii_array(d0_filename)     ;; [m] d0 used for the linearization
d0_sd               = read_ascii_array(d0_sd_filename)  ;; [m] d0 error
c_ratio             = read_ascii_array(c_ratio_filename)    ;; c_stray/c_ref used for the linear.
c_ratio_sd          = read_ascii_array(c_ratio_sd_filename) ;; c_stray/c_ref error
;
; End of: Restoring the capsens calibrations
;==========================================================================



;===========================================================================
; Restoring the time filter data
;
;

gain_to_bbgain=fltarr(n_acts) ; factor to multiply gain to convert in curr_bit/pos_bit
                              ; it is initialized to zero. It will be filled at the end of
                              ; this routine (DC gain)

;
;  filter of position control loop (derivative plus low-pass filter by default)
;
;; compute the default control law coefficients: derivative with a low-pass cut
default_deriv_gain = 100d0    ; default derivative gain in N/(m/s) when
                              ; speed_loop_gain_chX contains 1.0
default_deriv_lowpass_freq1 = 2*!DPI*10d3    ; [rad/s] default low pass cut frequency f
default_deriv_lowpass_freq2 = 2*!DPI*10d3    ; [rad/s] default low pass cut frequency for
                                           ; the derivative loop control filter

tfl_par = tfl_gen_default()          ; returns a structure for filter definition
tfl_max_n_coeff = n_elements(tfl_par.s_zero)
	if n_elements(tfl_par.s_pole) ne tfl_max_n_coeff then $
    message, "Unexpected default par structure. Max #poles have match max #zeros."

tfl_par.s_const = default_deriv_gain*default_deriv_lowpass_freq1*default_deriv_lowpass_freq2;
tfl_par.n_s_pole = 2;1
tfl_par.n_s_zero = 1
tfl_par.s_zero[0] = 0d0
tfl_par.s_pole[0] = default_deriv_lowpass_freq1
tfl_par.s_pole[1] = default_deriv_lowpass_freq2

tfl_par = replicate(tfl_par, n_acts) ; copy the same filter for all actuators
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;tfl_par[30].s_const=default_deriv_gain*2*!DPI*1e3
;tfl_par[30].s_pole[0] = 2*!DPI*1e3
;tfl_par[5].s_const=default_deriv_gain*2*!DPI*1e3
;tfl_par[5].s_pole[0] = 2*!DPI*1e3
;tfl_par[8].s_const=default_deriv_gain*2*!DPI*1.5e3
;tfl_par[8].s_pole[0] = 2*!DPI*1.5e3
;tfl_par[28].s_const=default_deriv_gain*2*!DPI*1.5e3
;tfl_par[28].s_pole[0] = 2*!DPI*1.5e3
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

;
; filter of the position error control loop (simple proportional by default)
;
default_err_gain = 1d6        ; default position error gain in N/m when
                              ; err_loop_gain_chX contains 1.0

err_par = tfl_gen_default()          ; returns a structure for filter definition

err_par.s_const = default_err_gain
err_par.n_s_pole = 0
err_par.n_s_zero = 0

err_par = replicate(err_par, n_acts) ; copy the same filter for all actuators


; check for filter data already saved
check = findfile(tfl_data_filename)
if check[0] eq "" then begin
    message, 'The file '+tfl_data_filename+ $
      " containing the control law data (speed and pos-error) doesn't exist", /INFO
    print, 'Default values for the control law will be used'

endif else begin

    tfl_par=0 & err_par=0
    restore, tfl_data_filename

    if test_type(tfl_par, /STRUC, DIM=dim) then $
      message, "tfl_par must be a vector of structures"
    if dim[0] ne 1 then $
      message, "tfl_par must be a vector"
    if total(dim ne [1, n_acts]) ne 0 then $
      message, "The tfl_par structure vector must have " $
        +strtrim(n_acts,2)+" elements."

    if test_type(err_par, /STRUC, DIM=dim) then $
      message, "err_par must be a vector of structures"
    if dim[0] ne 1 then $
      message, "err_par must be a vector"
    if total(dim ne [1, n_acts]) ne 0 then $
      message, "The err_par structure vector must have " $
        +strtrim(n_acts,2)+" elements."

endelse

; check that the position (no position error!) loop has zero DC gain
dummy_dc_gain = tfl_par.s_const
for i_act=0,n_acts-1 do begin
    for i=0,tfl_par[i_act].n_s_zero-1 do dummy_dc_gain[i_act]=dummy_dc_gain[i_act]*tfl_par[i_act].s_zero[i]
    for i=0,tfl_par[i_act].n_s_pole-1 do dummy_dc_gain[i_act]=dummy_dc_gain[i_act]/tfl_par[i_act].s_pole[i]
endfor
if total(dummy_dc_gain ne 0d0) gt 0 then message, "The position loop filter cannot have DC gain gt 0"

; computes the position error DC gain
tfl_err_gain = err_par.s_const
for i_act=0,n_acts-1 do begin
    for i=0,err_par[i_act].n_s_zero-1 do tfl_err_gain[i_act]=tfl_err_gain[i_act]*err_par[i_act].s_zero[i]
    for i=0,err_par[i_act].n_s_pole-1 do tfl_err_gain[i_act]=tfl_err_gain[i_act]/err_par[i_act].s_pole[i]
endfor

;
; End of: Restoring the time filter data
;==========================================================================



;===========================================================================
; Restoring the wyko parameters
;
interf_type = "wyko"
interf_model = "RTI4100"
;interf_model = "400"

;interf_type = "Durango"
;interf_model = ""

;interf_type = "Fisba"
;interf_model = "DCI2 HD"

case strlowcase(interf_type) of
    "wyko": begin
       case strlowcase(interf_model) of
         "rti4100": begin
          interf_n_pix_x = 736L
          interf_n_pix_y = 480L
         end

         '400': begin
          interf_n_pix_x = 368L
          interf_n_pix_y = 240L
         end

         else: message, interf_type+" "+interf_model+": Unsupported interferometer mode"
       endcase
       interf_aspect = 1.167    ; apsect ratio of the interferometer pixels (y/x)
       interf_wl = 632.8        ; laser wavelength in nm
    end

    "durango": begin
       interf_n_pix_x = 368L
       interf_n_pix_y = 240L
       interf_aspect = 1.167    ; apsect ratio of the interferometer pixels (y/x)
       interf_wl = 632.8        ; laser wavelength in nm
    end

    "fisba": begin
       interf_n_pix_x = 1024L ;???
       interf_n_pix_y = 1024L ;???
       interf_aspect = 1.0      ; apsect ratio of the interferometer pixels (y/x)
       interf_wl = 632.8        ; laser wavelength in nm
    end
endcase

interf_com_type = "socket"
interf_com_port = { $
    ip_address: '193.206.155.206', $
    port:       4660}

;interf_com_type = "serial"
;interf_com_port = "COM1"

offset              = fltarr(2)         ;; (0,0) (CCD) pixel coordinates in mm
scaling             = fltarr(2)         ;; x and y scaling factors from pixel to mm
ang_offset          = 0.0               ;; angular offset of mm coord. sys. from pix coord. sys.
transf              = fltarr(2,2)       ;; rotation matrix to transform from mm to pix coord.
opt_coordinates     = fltarr(2,n_acts)  ;; actuators coordinates in pixels
index               = lonarr(n_acts+1)  ;; index of the starting positions of the capacitor locations in the cap array
cap                 = lonarr(interf_n_pix_x*interf_n_pix_y) ;; list (indexed by index) of the wyko pixels within the capacitor plates
opt_flag            = 0B                ;; flag for the initialization of wyko parameters (0B not initialized)

check = findfile(optical_filename)
if check[0] eq "" then begin
    message, 'The file '+optical_filename+ $
      " containing the optical data doesn't exist", /INFO
    print, "The variables offset, scaling, ang_offset, rot, act_coordinates have not been initialized"
endif else begin

    restore, optical_filename

    if test_type(offset, /REAL, DIM=dim, N_El=n_el) then $
      message, "offset vector must be real"
    if dim[0] ne 1 then $
      message, "Offset must be a vector"
    if n_el ne 2 then $
      message, "Offset must have two values"

    if test_type(scaling, /REAL, DIM=dim, N_El=n_el) then $
      message, "scaling vector must be real"
    if dim[0] ne 1 then $
      message, "scaling must be a vector"
    if n_el ne 2 then $
      message, "scaling must have two values"

    if test_type(ang_offset, /REAL, N_El=n_el) then $
         message, "ang offset must be real"
    if n_el ne 1 then $
        message, "ang offset must have one value"

    if test_type(transf, /REAL, DIM=dim) then $
      message, "transformation matrix vector must be real"
    if total(dim ne [2,2,2]) ne 0 then $
      message, "transformation must be 2x2 matrix"

    if test_type(index, /LONG, DIM=dim) then $
      message, "index must be long"
    if total(dim ne [1,n_acts+1]) ne 0 then $
      message, "index must be a vector"

   if test_type(cap, /LONG, DIM=dim) then $
      message, "cap must be long"
    if total(dim ne [1,interf_n_pix_x*interf_n_pix_y]) ne 0 then $
      message, "cap must be a vector"

    if test_type(opt_coordinates, /REAL, DIM=dim) then $
      message, "act_coordinates matrix vector must be real"
    if total(dim ne [2,2,n_acts]) ne 0 then $
      message, "act_coordinates must be 2xn_acts matrix"

    opt_flag = 1B
endelse
;
; End of: Restoring the wyko parameters
;==========================================================================

;;=========================================================================
;; structure describing the mirror, electronics and system properties
;;
;;
;;
;; Reordering of the actuator coordinates from mirror based ordering to
;; DSP ordering before storing the coordinates in the adsec structure
;;

adsec = $
  { $
    $;;;*** Mirror ***
    $
    n_actuators    :        n_acts, $ ; total number of DSP channels
    n_true_act     :   n_true_acts, $ ; total number of physical actuators
    n_crates       :      conf_gvar.n_crates, $ ; number of crates where the DSP board are collected
    n_bcu          :         conf_gvar.n_bcu, $ ; number of communication boards accessed by diagnostic communication
    n_board_per_bcu:conf_gvar.n_board_per_bcu, $ ;number of DSP boards per communication board (BCU) for diagnostic communication
    n_dsp_per_board:conf_gvar.n_dsp_per_board, $ ;number of DSPs per DSP board
    n_act_per_dsp  : conf_gvar.n_act_per_dsp, $ ; number of actuators per DSP
    n_rings        :       conf_gvar.n_rings, $ ; number of actuator rings
    n_act_ring     :    conf_gvar.n_act_ring, $ ; number of act. per ring
    angle_ring     :    conf_gvar.angle_ring, $ ; angle of the ring wrt the opt. axis [deg]
    angle_act0     :    conf_gvar.angle_act0, $ ; azimuthal angle act0 wrt x+ [deg]
    curv_radius    :   conf_gvar.curv_radius, $ ; radius of curvature (back face) [mm]
    in_radius      :     conf_gvar.in_radius, $ ; inner radius of the mirror [mm]
    out_radius     :    conf_gvar.out_radius, $ ; outer radius of the mirror [mm]
    ring_radius    :   ring_radius, $ ; radius of the rings from the opt.axis [mm]
    act_coordinates:     act_coord, $ ; coordinates of act's [mm],act#0->x+,act#9->y+
    act_name       :      act_name, $ ; act name. Format RAA (see above)
    mass           :          mass, $ ; mirror mass [g]
    act_w_curr     :    act_w_curr, $ ; actuators that can be used to apply current
    act_wo_curr    :   act_wo_curr, $ ; actuators that cannot be used to apply current
    act_w_pos      :     act_w_pos, $ ; actuators that have not problems sensing position
    act_wo_pos     :    act_wo_pos, $ ; actuators that have problems sensing position
    act_w_cl       :      act_w_cl, $ ; actuators available for closing the loop
    act_wo_cl      :     act_wo_cl, $ ; actuators not available for closing the loop
    true_act       :     true_acts, $ ; channels corresponding to physical actuators
    dummy_act      :     conf_gvar.dummy_act, $ ; channels corresponding to no actuators ([-2]=no dummy acts)
    n_dummy_act    :  n_dummy_acts, $ ; number of dummy actuators
    weight_curr    :   conf_gvar.weight_curr, $ ; average current counts per actuator to support weight
    secure_factor  :   conf_gvar.secure_factor,$    ; secure factor to assure the thin shell pressed on the ref.plate
    nominal_gap    :   nominal_gap, $ ; [pcounts] nominal working gap
    curr4bias_mag  :   conf_gvar.curr4bias_mag, $ ; vector of ratio between bias coil currents to balance bias magnet force
    $                                             ;and weight at nominal gap
    max_ngain_step :   conf_gvar.max_ngain_step, $; max allowed step of the normalized gain to be written in one shot
    $
    $;;;*** Capacitive Sensors ***
    $
    in_cap_radius  : conf_gvar.in_cap_radius, $ ; inner radius of the cap. anulus [mm]
    out_cap_radius : conf_gvar.out_cap_radius, $ ; outer radius of the cap. anulus [mm]
    d0             :            d0, $ ; [um] nominal distance where cvar=cref when c_ref=0
    c_ratio        :       c_ratio, $ ; ratio between cstray and cref. CANNOT BE 0.0 for MMT!!!!
    d0_sd          :         d0_sd, $ ; d0 measured std. deviations [um]
    c_ratio_sd     :    c_ratio_sd, $ ; c_ratio measured std. deviations
    max_delta_dist : max_req_delta_lin_dist, $ ; max delta dist for linearization [um]
    max_lin_dist   : dblarr(n_acts), $ ; max dist for linearization [um]
    $
    $;;;*** Control law ***
    $
    ;force_sign2pull: conf_gvar.force_sign2pull, $ ; force sign (1 or -1) to pull the shell against the backplate
    err_smoothed_step  : conf_gvar.err_smoothed_step,  $ ;step of the err_loop_gain during DSP internal gain smoothing
    speed_smoothed_step: conf_gvar.speed_smoothed_step,$ ;step of the speed_loop_gain during DSP internal gain smoothing
    gain_to_bbgain : gain_to_bbgain, $ ; factor to multiply gain to convert in curr_bit/pos_bit (position error loop)
    tfl_err_gain   :   tfl_err_gain, $ ; max DC gain [N/m] of the position error loop
    tfl_max_n_coeff:tfl_max_n_coeff, $ ; maximum number of filter coeffs
    tfl_par:                tfl_par, $ ; data of the position (speed) loop filter
    err_par:                err_par, $ ; data of the position (speed) loop filter
    $
    $;;;*** DSPs ***
    $
    n_dsp      : fix(floor(n_acts/float(conf_gvar.n_act_per_dsp))), $ ; number of dsp
    last_act   :      n_acts-1, $ ; last channel number, first=0
    last_dsp   : fix(floor(n_acts/float(conf_gvar.n_act_per_dsp))-1), $ ; last DSP number, first=0
    all_actuators:         -1L, $ ; code meaning "do on all actuators"
    all_dsp     :          -1L, $ ; code meaning "do on all dsp"
    all_boards  :          -1L, $ ; code meaning "do on all dsp boards"
    all         :          -1L, $ ; code meaning "do on all"
    act2mir_act :  act2mir_act, $ ; dsp actutor to mirror actuator number look-up table
    mir_act2act :  mir_act2act, $ ; mirror actuator number to dsp actuator look-up table
    sampling_time:conf_gvar.sampling_time, $ ; fastest sampling time [s]
    offset_delta_pos: offset_delta_pos, $ ; offset to add to the delta position command in order
    $                                     ; to reduce the noise induced by the communication
    max_seq_samples:conf_gvar.max_seq_samples, $ ; max number of samples in the time sequences
    min_curr_val: fltarr(n_acts), $ ; minimum current value in internal force units [fu]
    max_curr_val: fltarr(n_acts), $ ; maximum current value in internal force units [fu]
    min_volt_val:   min_volt_val, $ ; minimum voltage value in internal potential units [pu]
    max_volt_val:   max_volt_val, $ ; maximum voltage value in internal potential units [pu]
    min_pos_val : fltarr(n_acts), $ ; minimum position value
    max_pos_val : fltarr(n_acts), $ ; maximum position value
    max_dec_val :conf_gvar.max_dec_val, $ ; maximum decimation value
    dac_bits    :       dac_bits, $ ; number of bits of DACs
    adc_bits    :       adc_bits, $ ; number of bits of ADCs
    capsens_vmax:   capsens_vmax, $ ; cap. sensor max output voltage [volt]
    capsens_vref:   capsens_vref, $ ; cap sensor reference voltage (d=d0)[volt]
    coil_res    :       coil_res, $ ; coil resistance, in ohms
    act_efficiency:      act_eff, $ ; actuator efficiency [N/sqrt(W)]
    m_per_pcount: dblarr(n_acts), $ ; meters per linearized position count [m/count]
    min_lin_dist: dblarr(n_acts), $ ; [m] minimum distance that can be obtained by the linearization
    max_lin_output:fltarr(n_acts), $ ; maximum output obtained from the linearization routine [counts]
    A_per_dacbit:   A_per_dacbit, $ ; amperes per dacbit [A/dacbit]
    dac_offset  :     dac_offset, $ ; [dacbit] DAC value to give zero current of current driver
    pos_sat_force: conf_gvar.pos_sat_force, $ ;[fu] default positive saturation force
    neg_sat_force: conf_gvar.neg_sat_force, $ ;[fu] default negative saturation force 
    V_per_pcount:   V_per_pcount, $ ; volts per not-linearized position count [V/count]
    A_per_ccount: fltarr(n_acts), $ ; amperes per current count [A/count]
    N_per_ccount: fltarr(n_acts), $ ; Newton per current count [N/count]
    pu_str      :         pu_str, $
    fu_str      :         fu_str, $
    lu_str      :         lu_str, $
    M           :              M, $ ; Mass matrix [g]
    meas_ff_used:   meas_ff_used, $ ; 1B: a k matrix stored in data dir has been used (0B: eitherwise)
    ff_matrix   :      ff_matrix, $ ; feedforward matrix: n_act X n_act (ccount/pcount)
    ff_f_svec      :           u, $ ; force singular vectors (per columns)
    ff_p_svec      :           v, $ ; position singular vectors (per columns)
    ff_sval        :           w, $ ; singular values (ordered from the lowest to highest) ccount/pcount
    preshaper_len : conf_gvar.preshaper_len, $ ; preshaper length for the current and command rising
    ff2bias_step: conf_gvar.ff2bias_step, $ ; step to set the bias current as sum of initial bias current and ff current 
    weight_times4peak    :   conf_gvar.weight_times4peak, $ ; factor to increase the peak of maximum allowable force.
    curr_threshold       :    conf_gvar.curr_threshold, $; current threshold factor for the dynamical response related to adsec.max_curr_val.
    max_dyn_peak_pos     :    conf_gvar.max_dyn_peak_pos, $ ; position threshold for the dynamical optimized response.
    max_modal_abs_step   :    conf_gvar.max_modal_abs_step, $ ; max positon modal step for the dynamical response.
    max_amp_pos:		conf_gvar.max_amp_pos, $ ;[lu] maximum amplitude of position to apply
    max_amp_pos_ramp:	conf_gvar.max_amp_pos_ramp, $ ;[lu] maximum amplitude of position to apply in ramp_gain
    max_amp_pos_tilt:	conf_gvar.max_amp_pos_tilt, $ ;[lu] maximum amplitude of position to apply in ramp_gain
    max_delta_curr :	conf_gvar.max_delta_curr , $ ;[fu] maximum allowed delta current for  step in ramp_gain
    max_gain_step :		conf_gvar.max_gain_step , $ ;[N/m] maximum delta gain allowed in a single step
    min_gain_step :		conf_gvar.min_gain_step , $ ;[N/m] minimum delta gain allowed in a single step
    max_curr_tospread :	conf_gvar.max_curr_tospread , $ ;[N] maximum current to spread to the bias current of all actuators
    thr_perr:			conf_gvar.thr_perr, $;[m] threshold of position error
    max_iter_integ:		conf_gvar.max_iter_integ, $ ;maximum number of iteration in integrator
    max_curr:			conf_gvar.max_curr, $ ;[N] maximum current threshold  						 
    fact_reduction_curr:	conf_gvar.fact_reduction_curr $ ;factor of reduction current in set_isostatic
  }

;===========================================================================
; definition and initialization of the structure where is saved the
; actual status of the system
;
init_sys_status
;
;===========================================================================

;===========================================================================
; definition and initialization of the structure where is saved the
; actual status of the system ambient and volages
;
env_status={                        $
    temp : conf_gvar.fp_temp,          $
    temp_name : conf_gvar.fp_temp_name,    $
    rh:    conf_gvar.fp_rh,             $
    rh_name:   conf_gvar.fp_rh_name,     $
    volt : conf_gvar.fp_volt,           $
    volt_name : conf_gvar.fp_volt_name     $
}
;
;===========================================================================


;===========================================================================
; definitions for displaying the actuator pattern
;
; dimensions of the position and current panels (pixels), ODD is better
if adsec.n_actuators lt 50 then $
    x_tv = 300 $
else $
    x_tv = 500
y_tv = x_tv+60
charsize=0.9
;theta_tv = 30.0
;x_reflect_tv = 0B
theta_tv = 90.0     ;180.0 matches the screen on Durango for MMT336 at shimulator
x_reflect_tv = 1B
tr = theta_tv/180.0*!PI
rot_mat = [[cos(tr), -sin(tr)], $
           [sin(tr),cos(tr)]]
if x_reflect_tv then rot_mat[*,0] = -rot_mat[*,0]

; scale on the graphic panel [pix/mm]
tv_scale = (min([x_tv, y_tv]-10))/(2*adsec.out_radius)

; # of pixels per sensing zone
npix_cap = tv_scale*2*adsec.out_cap_radius
npix_cap = fix(npix_cap/2)*2+1 > 3 ; odd is better
; # of pixel of the inner diameter of the capsens
npix_in_cap = fix(tv_scale*2*adsec.in_cap_radius)

; pixmap mask (0B/1B) of the single sensing zone
capsens = shift(dist(npix_cap), npix_cap/2, npix_cap/2)
capsens = (capsens lt npix_cap/2) - (capsens lt npix_in_cap/2)

rot_act_coord = rot_mat ## transpose(adsec.act_coordinates)
rot_act_coord = transpose(temporary(rot_act_coord))
act_coord = fix(round(tv_scale*rot_act_coord))

act_coord = act_coord + rebin([x_tv/2, y_tv/2], 2, adsec.n_actuators, /SAMP)
num_coord = act_coord
num_coord[1,*] = num_coord[1,*]-!D.Y_CH_SIZE/4 ; correction for label verical centering

tv_pos_type_list = ["Positions", "Commands", "Max-min position", "Position error", "Capsens out."]
tv_pos_unit_list = [adsec.lu_str, "um", "nm"]
tv_sens_unit_list = [adsec.pu_str, "mV", "V"]

tv_curr_type_list = ["Total Currents", "Bias currents", "Control currents", "Feed-foreward currents"]
tv_curr_unit_list = [adsec.fu_str, "mA", "mN"]

extra_ct = 0.0 ; after color table optimization:
               ; top_color_value = max_value+extra_ct*(max_value-min_value)/2
               ; bottom_color_value = min_value-extra_ct*(max_value-min_value)/2

gr = $
  { $
    x_tv             :            x_tv, $ ; x-dim of position and current panel [pix]
    y_tv             :            y_tv, $ ; y-dim of position, scale and current panel [pix]
    x_scale_tv       :             100, $ ; x-dim of scale panel [pix]
    curr_cuts        :   [-1,1]*5000.0, $ ; color cuts in current panel
    pos_cuts         :[2.0^15,-10000.0], $ ; color cuts in position panel
    extra_ct         :        extra_ct, $ ; see above definition of extra_ct
    sub_pos_offset   :              0B, $ ; 1B: pos-offset is displayed. 0B pos is displayed
    sub_curr_offset  :              0B, $ ; 1B: curr-offset is displayed. 0B curr is displayed
    cl_on_color      :long([255B,0B,0B,0B],0), $ ; color of closed loop on (red)
    cl_off_color     :long([0B,255B,0B,0B],0), $ ; color of closed loop off (green)
    n_act_color      :   !D.TABLE_SIZE, $ ; number of colors for the actuators
    theta_tv         :        theta_tv, $ ; angle of rotation of adsec.act_coordinates pattern 4 displaing
    x_reflect_tv     :    x_reflect_tv, $ ; reflection (1B) or not (0B) of the rotated pattern 4 displaing
    act_coord        :       act_coord, $ ; tv coordinates of act's [pix]
    tv_scale         :        tv_scale, $ ; pix/mm on the panels
    tv_pos_type_list :tv_pos_type_list, $ ; list of the data type to display in the position panel
    tv_curr_type_list:tv_curr_type_list, $ ; list of the data type to display in the position panel
    tv_pos_type_idx  :              0L, $ ; index of the currently active item of tv_pos_type_list
    tv_curr_type_idx :              0L, $ ; index of the currently active item of tv_curr_type_list
    tv_pos_unit_list :tv_pos_unit_list, $ ; list of position units to display in the position panel
    tv_sens_unit_list:tv_sens_unit_list, $ ; list of cap sensor output units to display in the position panel
    tv_curr_unit_list:tv_curr_unit_list, $ ; list of the data type to display in the position panel
    tv_pos_unit_idx  :              0L, $ ; index of the currently active item of tv_pos_type_list
    tv_curr_unit_idx :              0L, $ ; index of the currently active item of tv_curr_type_list
    tv_pos_smooth    :              0B, $ ; do I smooth the position pattern?
    tv_curr_smooth   :              0B, $ ; do I smooth the current pattern?
    num_coord        :       num_coord, $ ; tv coordinates of act num's [pix]
    num_type         :               0, $ ; 0=internal repr.,1=mechanic repr.,2=warren
    charsize         :        charsize, $ ; normalized size of the chars used to label the acts
    position         :bytarr(x_tv,y_tv), $ ; bitmap of position panel
    current          :bytarr(x_tv,y_tv), $ ; bitmap of current panel
    npix_cap         :        npix_cap, $ ; dim of the capsens image
    capsens          :         capsens  $ ; image of a capsens
  }
;
; End of: definitions for displaying the actuator pattern
;===========================================================================

;===========================================================================
; Definition of the interferometer parameters
;
wyko = $
  { $
    type:       interf_type, $    ; interferometer Type
    model:      interf_model, $ ; interferometer Model
    com_type:   interf_com_type, $ ; communication type to the interferometer (socket or serial)
    com_port:   interf_com_port, $ ; connection data used for the serial/socket communication with wyko
    n_pix_x:    interf_n_pix_x, $      ; x-size of wyko CCD in pixels
    n_pix_y:    interf_n_pix_y, $      ; y-size of wyko CCD in pixels
    aspect:     interf_aspect, $       ; apsect ratio of the wyko pixels (y/x)
    wavelength: interf_wl, $   ; laser wavelength in nm
    offset:     offset, $       ; center of the mirror in interferometer coord. (IC, pix)
    scaling:    scaling, $      ; scaling factor from mirror coordinates (MC, mm) to (IC, pix)
    ang_offset: ang_offset, $   ; angle of rotation about the center of the mirror
                                ; to transform from MC to IC
    transf:     transf, $       ; transformation matrix to pass from (MC, mm)
                                ; to (IC, pix). To obtain the total transformation
                                ; wyko.offset must be added to the result
                                ; actuators coordinates in pixels
    act_coordinates: opt_coordinates, $
    index:      index, $
    cap:        cap, $
    flag:       opt_flag $  ;flag to check if the optical calibration paramters have been loaded
  }
;
; END OF: Definition of the interferometer parameters
;===========================================================================


;===========================================================================
; Definition of the wavefront sensor parameters
;
;; no binned format
n_pix_x = 80          ;; number of no-binned pixels in x-direction (full frame)
n_pix_y = 80          ;; number of no-binned pixels in y-direction (full frame)

;; binned format
n_bpix_x = 30         ;; number of binned pixels in x direction returned by the DDS
n_bpix_y = 26         ;; numebr of binned pixels in y direction returned by the DDS
pix_per_bpix = 3      ;; number of pixels (pix) per binned pixel (bpix)

;; lenslet format
n_slopes_1d = 12      ;; number of slopes in x or y direction
bpix_per_slope_1d = 2 ;; number of binned pixel in x or y direction per sensing zone

;; sub-frame format used for slope computation (i.e. the sub-frame containing the pupil)
n_core_bpix_1d = bpix_per_slope_1d*n_slopes_1d ;; size of the binned subframe used for slope computation in binned pixel unit
n_core_pix_1d = n_core_bpix_1d*pix_per_bpix ;; size of the no-binned subframe used for slope computation in no-binned pixel unit

n_pix_offset_x = (n_pix_x - n_core_pix_1d)/2
n_pix_offset_y = (n_pix_y - n_core_pix_1d)/2
n_bpix_offset_x = (n_bpix_x - n_core_bpix_1d)/2
n_bpix_offset_y = (n_bpix_y - n_core_bpix_1d)/2

mask_slope = replicate(1B, n_slopes_1d, n_slopes_1d) ;; mask for slopes (core frame)
dark_field = fltarr(n_core_pix_1d, n_core_pix_1d)    ;; Dark field of the core region in the no binned mode
bdark_field = fltarr(n_core_bpix_1d, n_core_bpix_1d) ;; Dark field of the core region in the binned mode
dark_field_sd = fltarr(n_core_pix_1d, n_core_pix_1d) ;; Sandard deviation of the Dark Field (no binned mode)
bdark_field_sd = fltarr(n_core_bpix_1d, n_core_bpix_1d) ;; Standard Deviation ofthe Dark Field (binned mode)

wfs = $
  { $
    n_pix_x:       n_pix_x, $ ;; x size of the no-binned wfs full-frame
    n_pix_y:       n_pix_y, $ ;; y size of the no-binned wfs full-frame
    n_bpix_x:      n_bpix_x, $;; number of binned pixels in x direction returned by the DDS
    n_bpix_y:      n_bpix_y, $;; number of binned pixels in y direction returned by the DDS

    pix_per_bpix:  pix_per_bpix, $ ;; number of pixels (pix) per binned pixel (bpix)
    n_slopes_1d:   n_slopes_1d, $ ;; number of slopes in x or y direction
    bpix_per_slope_1d: bpix_per_slope_1d, $ ;; number of binned pixel in x or y direction per sensing zone

    mask_slope:     mask_slope, $       ;; mask for slopes (core frame)

    dark_field:     dark_field, $       ;; Dark field of the core region in the no binned mode
    bdark_field:    bdark_field, $      ;; Dark field of the core region in the binned mode
    dark_field_sd:  dark_field_sd, $    ;; Sandard deviation of the Dark Field (no binned mode)
    bdark_field_sd: bdark_field_sd, $   ;; Standard Deviation ofthe Dark Field (binned mode)

    n_pix_offset_x: n_pix_offset_x, $   ;; x offset of first no-binned pixel used for slope computation in the frame from DDS
    n_pix_offset_y: n_pix_offset_y, $   ;; y offset of first no-binned pixel used for slope computation in the frame from DDS
    n_bpix_offset_x: n_bpix_offset_x, $ ;; x offset of first binned pixel used for slope computation in the binned frame returned by DDS
    n_bpix_offset_y: n_bpix_offset_y, $ ;; y offset of first binned pixel used for slope computation in the binned frame returned by DDS

    exp_time: 8.0, $                    ;; exposition time [ms]

    host:   'tilt.as.arizona.edu', $ ;; address of the host used to read the wfs data from
    port:        5100, $ ;; port of the host to connect to (socket connection)
    unit:         -3L  $ ;; logical unit associated to the socket (see dds_open.pro)
  }

;;; Mask definition used by Francois
;;
wfs.mask_slope[0:3,0]=0B
wfs.mask_slope[8:*,0]=0B
wfs.mask_slope[0:3,11]=0B
wfs.mask_slope[8:*,11]=0B
wfs.mask_slope[0,0:3]=0B
wfs.mask_slope[0,8:*]=0B
wfs.mask_slope[11,0:3]=0B
wfs.mask_slope[11,8:*]=0B
wfs.mask_slope[1,1]=0B
wfs.mask_slope[10,10]=0B
wfs.mask_slope[1,10]=0B
wfs.mask_slope[10,1]=0B
wfs.mask_slope[5:6,5:6]=0B


;===========================================================================
; Definition of DSP constants and memory map
;
init_dsp_map
;
; END OF: Definition of DSP constants and memory map
;===========================================================================

err = send_linearization(/no_send2dsp)
if err ne adsec_error.ok then message,"Error computing the linearization parameters"
err = send_force_calibration(/no_send2dsp)
if err ne adsec_error.ok then message,"Error computing the force calibration parameters"

; factor to multiply DC gain to convert it in curr_bit/pos_bit
adsec.gain_to_bbgain=tfl_err_gain/adsec.N_per_ccount*adsec.m_per_pcount

end
