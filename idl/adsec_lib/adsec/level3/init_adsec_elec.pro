; $Id: init_adsec_elec.pro,v 1.14 2009/09/18 08:25:01 marco Exp $$
;
;+
;  NAME:
;   INIT_ADSEC_ELEC
;
;  PURPOSE:
;   Load filters parameter in the adsec structure.
;
;  USAGE:
;   err = INIT_ADSEC_ELEC(filename,ACT2MIR=act2mir_act_filename,STRUCT=struct, NOFILL=nofill)
;
;  INPUT:
;   filename(optional)  : filename of data to load. Default adsec_path.conf+'filters_LBT672.txt'.
;
;  OUTPUT:
;   error               : error code.
;
;  KEYWORDS:
;   ACT2MIR             : filename con fullpath of a .sav file with tfl_par and err_par structure.
;   STRUCT              : structure with parameters loaded and calculated.
;   NOFILL              : do not fill the adsec structure.
;
;  COMMON BLOCKS:
;   ADSEC               : secondary adaptive base common block. A part of it will be filled.
;
;  PACKAGE:
;   LBT Adsec Libraries.
;
;  HISTORY
;   Created on 28 Oct 2004 by Marco Xompero (MX).
;   marco@arcetri.astro.it
;   02 Dec 2004, AR & DZ
;     removed sampling_time and capsens_vref fields. Added min_ADC_samp_len
;     and max_ADC_samp_len fields.
;   13 Dec 2004, DZ
;     Added dsp2dac_transfer_time, dac_settling,wave_settling,
;     wc_comp_time, wave_clock_decimation,  DSP_clock_decimation fields.
;   Feb 2005 DZ
;   Added n_slope,n_slope_delay,n_modes_delay
;    20 May 2005 Marco Xompero(MX)
;      n_slope,n_slope_delay,n_modes_delay moved to rtr.
;-



Function init_adsec_elec, filename, ACT2MIR=act2mir_act_filename,STRUCT=struct, NOFILL=nofill

; common blocks definition
@adsec_common


;;=========================================================================
;; data structure loading
;;
if n_elements(filename) eq 0 then filename=filepath(ROOT=adsec_path.conf, SUB=!AO_CONST.shell, "elec.txt")
data = read_ascii_structure(filename, DEBUG=0)
if n_elements(data) eq 0 then begin
    message, 'The configuration file "'+filename+'" does not exist or has a wrong format.'
endif
;;=========================================================================

if data.dummy_act[0] lt 0 then begin
    n_dummy_acts = 0
endif else begin
    n_dummy_acts=fix(n_elements(data.dummy_act))
endelse

n_acts = adsec_shell.n_true_act+n_dummy_acts             ;; total number of dsp channels

if n_dummy_acts eq 0 then begin
    true_acts=indgen(n_acts)
endif else begin
    err = complement(data.dummy_act, indgen(n_acts), true_acts)
endelse

act_coord = fltarr(2, n_acts)          ;; x,y coords of the acts

;n_act_names = 3                         ;; number of formats for the actuator names
                                        ;; 0 = internal numbering (0...35)
                                        ;; 1 = mechanical numbering (1...36)
                                        ;; 2 = RAA format
;act_name = strarr(n_acts, n_act_names)  ;; actuator name: "RAA"
                                        ;; R =ring number (0-9)
                                        ;; AA=act number in the ring (00-XX)

;if adsec_shell.n_rings gt 10 then $
;    ring_format_str='(I2.2)' $
;else $
;    ring_format_str='(I1.1)'
;if adsec_shell.n_act_ring[adsec_shell.n_rings-1] gt 100 then $
;    act_format_str='(I3.3)' $
;else $
;    act_format_str='(I2.2)'
;
act0 = 0
for nr=0,adsec_shell.n_rings-1 do begin

    ;; azimuthal angle of the acts in the ring nr
    theta = findgen(adsec_shell.n_act_ring[nr])*(360.0/adsec_shell.n_act_ring[nr])+adsec_shell.angle_act0[nr]
    theta = !PI/180.0*transpose(theta)
    ;; act x,y coordinates
    act_coord[0,act0] = shift(adsec_shell.ring_radius[nr]*[cos(theta), sin(theta)],0,nr+1)

;    act_name[act0,0] = strtrim(indgen(adsec_shell.n_act_ring[nr])+act0, 2)
;    act_name[act0,1] = strtrim(indgen(adsec_shell.n_act_ring[nr])+act0, 2)
;    act_name[act0,2] = string(nr,FORMAT=ring_format_str) $
;      + string(indgen(adsec_shell.n_act_ring[nr]),FORMAT=act_format_str)

    act0 = act0+adsec_shell.n_act_ring[nr]
endfor
;matching rigid M2 specification
new_act_coord = [-act_coord[1,*], act_coord[0,*]]

;if n_dummy_acts ne 0 then begin
;    act_name[act0:*,0] = strtrim(indgen(n_dummy_acts)+act0, 2)
;    act_name[act0:*,1] = "XX"
;    act_name[act0:*,2] = "XX"
;endif

;===========================================================================
; Definition of main parameters
;
dac_bits = data.dac_bits         ; number of bits of current driver DACs
adc_bits = data.adc_bits         ; number of bits of capsens ADCs
capsens_vmax = data.capsens_vmax ; cap. sensor max output voltage [volt]
V_per_pcount = float(capsens_vmax/2d0^adc_bits); volts per no-linearized position count [V/pu], pu=adcbit
min_volt_val= 0.0            ; minimum voltage value in internal potential units [pu]
max_volt_val= 2.0^adc_bits-1 ; maximum voltage value in internal potential units [pu]

nominal_gap = data.nominal_gap   ; linearized pcounts at which the mirror usually works

offset_delta_pos = data.offset_delta_pos     ; offset to add to the delta position command in order to reduce the noise induced by the communication

; actuator (aka mechanical or mirror) numbers
if n_elements(act2mir_act_filename) eq 0 then act2mir_act_filename = filepath(ROOT=adsec_path.conf, SUB='general', 'act2mir_act.txt')

;===========================================================================
; Restoring the look-up table to convert the DSP actuator numbers to
; mirror geometry actuator numbers
;
check = file_search(act2mir_act_filename)
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
;act_coord = act_coord[*,act2mir_act]
;new_act_coord = act_coord[*,act2mir_act]
new_act_coord = new_act_coord[*,act2mir_act]
;act_name[*,1:2] = act_name[act2mir_act,1:2]

; actuators that cannot apply current: act_wo_curr
; (act_wo_curr=[-2] means all actuators can apply current)
;
act_wo_curr = data.act_wo_curr

; actuators that cannot apply read the position: act_wo_pos
; (act_wo_pos=[-2] means all actuators can read the position)
;
act_wo_pos = data.act_wo_pos

if data.dummy_act[0] ne -2 then begin
    if (act_wo_curr[0] eq -2) then act_wo_curr=data.dummy_act else act_wo_curr=[act_wo_curr,data.dummy_act]
    if (act_wo_pos[0] eq -2) then act_wo_pos=data.dummy_act else act_wo_pos=[act_wo_pos,data.dummy_act]
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
n_dsp = fix(floor(n_acts/float(data.n_act_per_dsp)))        ;number of dsp
last_dsp = fix(floor(n_acts/float(data.n_act_per_dsp))-1)   ;last DSP number, first=0

struct =                                           $
  {                                                $
    n_actuators         : n_acts,                  $ ;total number of DSP channels
    n_crates            : data.n_crates,           $ ;number of crates where the DSP board are collected
    n_bcu               : data.n_bcu,              $ ;number of communication boards accessed by diag communication
    n_board_per_bcu     : data.n_board_per_bcu,    $ ;number of DSP boards per communication board (BCU)
                                                   $ ;for diagnostic communication
    n_dsp_per_board     : data.n_dsp_per_board,    $ ;number of DSPs per DSP board
    n_act_per_dsp       : data.n_act_per_dsp,      $ ;number of actuators per DSP
    act_coordinates     : new_act_coord,           $ ;coordinates of act's [mm],act#0->x+,act#9->y+
;    act_name            : act_name,                $ ;act name. Format RAA (see above)
    act_w_curr          : act_w_curr,              $ ;actuators that can be used to apply current
    act_wo_curr         : act_wo_curr,             $ ;actuators that cannot be used to apply current
    act_w_pos           : act_w_pos,               $ ;actuators that have not problems sensing position
    act_wo_pos          : act_wo_pos,              $ ;actuators that have problems sensing position
    act_w_cl            : act_w_cl,                $ ;actuators available for closing the loop
    act_wo_cl           : act_wo_cl,               $ ;actuators not available for closing the loop
    act_wo_cl_on_set    : data.act_wo_cl_on_set,   $ ;actuators in open loop during the mirror set
    act_wo_icc          : data.act_wo_icc      ,   $ ;actuators without ICC filter active
    act_w_ffb_on_flat   : data.act_w_ffb_on_flat,  $ ;actuators with forced force feedback during flattening
    true_act            : true_acts,               $ ;channels corresponding to physical actuators
    dummy_act           : data.dummy_act,          $ ;channels corresponding to no actuators ([-2]=no dummy acts)
    n_dummy_act         : n_dummy_acts,            $ ;number of dummy actuators
    weight_curr         : data.weight_curr,        $ ;average current counts per actuator to support weight
    secure_factor       : data.secure_factor,      $ ;secure factor to assure the thin shell pressed on the ref.plate
    time_secure_factor  : data.time_secure_factor, $ ;secure factor to assume the time operations as done
    nominal_gap         : nominal_gap,             $ ;[pcounts] nominal working gap
    err_smoothed_step   : data.err_smoothed_step,  $ ;step of the err_loop_gain during DSP internal gain smoothing
    speed_smoothed_step : data.speed_smoothed_step,$ ;step of the speed_loop_gain during DSP internal gain smoothing
    n_dsp               : n_dsp,                   $ ;number of dsp
    last_act            : n_acts-1,                $ ;last channel number, first=0
    last_dsp            : last_dsp,                $ ;last DSP number, first=0
    act2mir_act         : act2mir_act,             $ ;dsp actutor to mirror actuator number look-up table
    mir_act2act         : mir_act2act,             $ ;mirror actuator number to dsp actuator look-up table
    clock_freq          : data.clock_freq,         $ ;[Hz] frequncy clock of status machine
    wave_dac_bits       : data.wave_dac_bits,      $ ;number of DAC bits for reference wave generation
    wave_max_ptv        : data.wave_max_ptv,       $ ;[V] maximum peak-to-valley available for reference wave
    min_ADC_samp_len    : data.min_ADC_samp_len,   $ ;[s] minimum temporal length of ADC sample
    max_ADC_samp_len    : data.max_ADC_samp_len,   $ ;[s] maximum temporal length of ADC sample
    wave_settling       : data.wave_settling,      $;[s] time of settling of the reference square wave
    dsp2dac_transfer_time: data.dsp2dac_transfer_time,  $;[s] transfer time of dac data
    wc_comp_time        : data.wc_comp_time,       $;[s] end of computation time from main interrupt (worst case)
    adc_acq_time        : data.adc_acq_time,       $;[s]total acquisition time of ADC
    dac_settling        : data.dac_settling,       $;[s] settling time of DAC

    DSP_clock_decimation: data.DSP_clock_decimation, $;
    wave_clock_decimation: data.wave_clock_decimation, $;

    offset_delta_pos    : offset_delta_pos,        $ ;offset to add to the delta position command in order
                                                   $ ;to reduce the noise induced by the communication
    max_seq_samples     : data.max_seq_samples,    $ ;max number of samples in the time sequences
    min_volt_val        : min_volt_val,            $ ;minimum voltage value in internal potential units [pu]
    max_volt_val        : max_volt_val,            $ ;maximum voltage value in internal potential units [pu]
    max_dec_val         : data.max_dec_val,        $ ;maximum decimation value
    dac_bits            : dac_bits,                $ ;number of bits of DACs
    adc_bits            : adc_bits,                $ ;number of bits of ADCs
    capsens_vmax        : capsens_vmax,            $ ;cap. sensor max output voltage [volt]
    pos_sat_force       : data.pos_sat_force,      $ ;[fu] default positive saturation force
    neg_sat_force       : data.neg_sat_force,      $ ;[fu] default negative saturation force
    V_per_pcount        : V_per_pcount,            $ ;volts per not-linearized position count [V/count]
    preshaper_len       : data.preshaper_len,      $ ;preshaper length for the current and command rising
    ff2bias_step        : data.ff2bias_step,       $ ;step to set the bias current as sum of
                                                   $ ;initial bias current and ff current
    weight_times4peak   : data.weight_times4peak,  $ ;factor to increase the peak of maximum allowable force.
    curr_threshold      : data.curr_threshold,     $ ;current threshold factor for the dynamical response
                                                   $ ;related to adsec.max_curr_val.
    max_dyn_peak_pos    : data.max_dyn_peak_pos,   $ ;position threshold for the dynamical optimized response.
    max_modal_abs_step  : data.max_modal_abs_step, $ ;max positon modal step for the dynamical response.
    max_amp_pos         : data.max_amp_pos,        $ ;[lu] maximum amplitude of position to apply
    max_amp_pos_ramp    : data.max_amp_pos_ramp,   $ ;[lu] maximum amplitude of position to apply in ramp_gain
    max_amp_pos_tilt    : data.max_amp_pos_tilt,   $ ;[lu] maximum amplitude of position to apply in ramp_gain
    max_delta_curr      : data.max_delta_curr,     $ ;[fu] maximum allowed delta current for  step in ramp_gain
    max_gain_step       : data.max_gain_step,      $ ;[N/m] maximum delta gain allowed in a single step
    min_gain_step       : data.min_gain_step,      $ ;[N/m] minimum delta gain allowed in a single step
    max_curr_tospread   : data.max_curr_tospread,  $ ;[N]maximum current to spread to the bias current of all actuators
    thr_perr            : data.thr_perr,           $ ;[m]threshold of position error
    max_iter_integ      : data.max_iter_integ,     $ ;maximum number of iteration in integrator
    max_curr            : data.max_curr,           $ ;[N] maximum current threshold
    fact_reduction_curr : data.fact_reduction_curr $ ;factor of reduction current in set_isostatic
  }

;===========================================================================
if not keyword_set(nofill) then begin
adsec.n_actuators         = n_acts                   ;total number of DSP channels
adsec.n_crates            = data.n_crates            ;number of crates where the DSP board are collected
adsec.n_bcu               = data.n_bcu               ;number of communication boards accessed by diag communication
adsec.n_board_per_bcu     = data.n_board_per_bcu     ;number of DSP boards per communication board (BCU)
                                                      ;for diagnostic communication
adsec.n_dsp_per_board     = data.n_dsp_per_board     ;number of DSPs per DSP board
adsec.n_act_per_dsp       = data.n_act_per_dsp       ;number of actuators per DSP
adsec.act_coordinates     = act_coord                ;coordinates of act's [mm],act#0->x+,act#9->y+
;adsec.act_name            = act_name                 ;act name. Format RAA (see above)
adsec.act_w_curr          = act_w_curr               ;actuators that can be used to apply current
adsec.act_wo_curr         = act_wo_curr              ;actuators that cannot be used to apply current
adsec.act_w_pos           = act_w_pos                ;actuators that have not problems sensing position
adsec.act_wo_pos          = act_wo_pos               ;actuators that have problems sensing position
adsec.act_w_cl            = act_w_cl                 ;actuators available for closing the loop
adsec.act_wo_cl           = act_wo_cl                ;actuators not available for closing the loop
adsec.act_wo_cl_on_set    = data.act_wo_cl_on_set    ;actuators in open loop during the mirror set
adsec.act_wo_icc          = data.act_wo_icc          ;actuators without ICC filter active
adsec.act_w_ffb_on_flat   = data.act_w_ffb_on_flat   ;actuators with forced force feedback during flattening
adsec.true_act            = true_acts                ;channels corresponding to physical actuators
adsec.dummy_act           = data.dummy_act           ;channels corresponding to no actuators ([-2]=no dummy acts)
adsec.n_dummy_act         = n_dummy_acts             ;number of dummy actuators
adsec.weight_curr         = data.weight_curr         ;average current counts per actuator to support weight
adsec.secure_factor       = data.secure_factor       ;secure factor to assure the thin shell pressed on the ref.plate
adsec.time_secure_factor  = data.time_secure_factor  ;secure factor to assume the time operations as done
adsec.nominal_gap         = nominal_gap              ;[pcounts] nominal working gap
adsec.err_smoothed_step   = data.err_smoothed_step   ;step of the err_loop_gain during DSP internal gain smoothing
adsec.speed_smoothed_step = data.speed_smoothed_step ;step of the speed_loop_gain during DSP internal gain smoothing
adsec.n_dsp               = n_dsp                    ;number of dsp
adsec.last_act            = n_acts-1                 ;last channel number, first=0
adsec.last_dsp            = last_dsp                 ;last DSP number, first=0
adsec.act2mir_act         = act2mir_act              ;dsp actutor to mirror actuator number look-up table
adsec.mir_act2act         = mir_act2act              ;mirror actuator number to dsp actuator look-up table
adsec.clock_freq          = data.clock_freq          ;[Hz] frequncy clock of status machine
adsec.wave_dac_bits       = data.wave_dac_bits       ;number of DAC bits for reference wave generation
adsec.wave_max_ptv        = data.wave_max_ptv        ;[V] maximum peak-to-valley available for reference wave
adsec.wave_settling       = data.wave_settling      ;[s] time of settling of the reference square wave
adsec.dsp2dac_transfer_time = data.dsp2dac_transfer_time  ;[s] transfer time of dac data
adsec.wc_comp_time        = data.wc_comp_time      ;[s] end of computation time from main interrupt (worst case)
adsec.adc_acq_time        = data.adc_acq_time      ;[s] total acquisition time of ADC
adsec.dac_settling        = data.dac_settling       ;[s] settling time of DAC

adsec.DSP_clock_decimation= data.DSP_clock_decimation ;
adsec.wave_clock_decimation=data.wave_clock_decimation;

adsec.offset_delta_pos    = offset_delta_pos         ;offset to add to the delta position command in order
                                                     ;to reduce the noise induced by the communication
adsec.min_ADC_samp_len    = data.min_ADC_samp_len    ;[s] minimum temporal length of ADC sample
adsec.max_ADC_samp_len    = data.max_ADC_samp_len    ;[s] maximum temporal length of ADC sample
adsec.max_seq_samples     = data.max_seq_samples     ;max number of samples in the time sequences
adsec.min_volt_val        = min_volt_val             ;minimum voltage value in internal potential units [pu]
adsec.max_volt_val        = max_volt_val             ;maximum voltage value in internal potential units [pu]
adsec.max_dec_val         = data.max_dec_val         ;maximum decimation value
adsec.dac_bits            = dac_bits                 ;number of bits of DACs
adsec.adc_bits            = adc_bits                 ;number of bits of ADCs
adsec.capsens_vmax        = capsens_vmax             ;cap. sensor max output voltage [volt]
adsec.pos_sat_force       = data.pos_sat_force       ;[fu] default positive saturation force
adsec.neg_sat_force       = data.neg_sat_force       ;[fu] default negative saturation force
adsec.V_per_pcount        = V_per_pcount             ;volts per not-linearized position count [V/count]
;adsec.pu_str              = pu_str
;adsec.fu_str              = fu_str
;adsec.lu_str              = lu_str
adsec.preshaper_len       = data.preshaper_len       ;preshaper length for the current and command rising
adsec.ff2bias_step        = data.ff2bias_step        ;step to set the bias current as sum of
                                                      ;initial bias current and ff current
adsec.weight_times4peak   = data.weight_times4peak   ;factor to increase the peak of maximum allowable force.
adsec.curr_threshold      = data.curr_threshold      ;current threshold factor for the dynamical response
                                                      ;related to adsec.max_curr_val.
adsec.max_dyn_peak_pos    = data.max_dyn_peak_pos    ;position threshold for the dynamical optimized response.
adsec.max_modal_abs_step  = data.max_modal_abs_step  ;max positon modal step for the dynamical response.
adsec.max_amp_pos         = data.max_amp_pos         ;[lu] maximum amplitude of position to apply
adsec.max_amp_pos_ramp    = data.max_amp_pos_ramp    ;[lu] maximum amplitude of position to apply in ramp_gain
adsec.max_amp_pos_tilt    = data.max_amp_pos_tilt    ;[lu] maximum amplitude of position to apply in ramp_gain
adsec.max_delta_curr      = data.max_delta_curr      ;[fu] maximum allowed delta current for  step in ramp_gain
adsec.max_gain_step       = data.max_gain_step       ;[N/m] maximum delta gain allowed in a single step
adsec.min_gain_step       = data.min_gain_step       ;[N/m] minimum delta gain allowed in a single step
adsec.max_curr_tospread   = data.max_curr_tospread   ;[N]max current to spread to the bias current of all actuators
adsec.thr_perr            = data.thr_perr            ;[m]threshold of position error
adsec.max_iter_integ      = data.max_iter_integ      ;maximum number of iteration in integrator
adsec.max_curr            = data.max_curr            ;[N] maximum current threshold
adsec.fact_reduction_curr = data.fact_reduction_curr  ;factor of reduction current in set_isostatic

endif

return, adsec_error.ok

End
