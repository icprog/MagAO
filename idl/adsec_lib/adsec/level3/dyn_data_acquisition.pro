;;;cose da fare qui:
;ripristinare il write con lo sdram_IO_addr 439
;riprisstinare il wait alla 484

; $Id: dyn_data_acquisition.pro,v 1.17 2007/09/19 09:43:41 labot Exp $

function dyn_data_acquisition, n_rep, index, amp, data, MODAL=modal $
                               , EXCITING_FREQ_RANGE=freq_range $
                               , N_TIME_SAMPLES=n_samples $
                               , ACT2PLOT=act2plot $
                               , DRIFT_POS=drift_pos $
                               , DRIFT_CTRL_CURR=drift_ctrl_curr $
                               , CURR_EXCITE=curr_excite $
                               , BUF_TOT_DIST=buf_tot_dist $
                               , BUF_TOT_ERR=buf_tot_err $
                               , OPTIMIZE_RESPONSE=opt_resp $
                               , SEQPASSED=seqpassed $
                               , GAIN_MEAS= gain_meas $
                               , GAIN_SPEED_MEAS = gain_speed_meas ;$ 
                               ;, FIRST_VALUE = first_value $
                               ;, FIRST_COMM = first_comm


; KEYWORDS not yet implemented
;   CMD_DECIMATION=cmd_dec, READ_DECIMATION=read_dec, $
;                               , SQUARE_WAVE_FREQUENCY=square_freq $
;                               , NOISE_ACQUISITION=noise
; when CMD_DECIMATION will be implemented it should be considered
; the possibility to use the position preshaper to smooth the commands
; bewteen two successive commands in order to avoid a box shaped command.
; A linear interpolation filter could be used.

;+
; DYN_DATA_ACQUISITION
;
; this function measures the zonal/modal closed loop TF.  The mirror
; is supposed to be already set and all the actuators with the control
; enabled. The feed-foreward is not used and the preshaper is disabled
; (the commands settle in 1 loop step).
;
;
; err = dyn_data_acquisition(n_rep, index, amp, data)
;
; INPUTS
;
;   index:        scalar, short int. Mode/actuator index
;                 for which measuring the TF. If the MODAL keyword is
;                 set the index refers to the FF-mode indexes
;                 (adsec.ff_p_svec or adsec.ff_f_svec if a position
;                 or current excitation is performed, see CURR_EXCITE
;                 keyword). If MODAL keyword is not set index refers
;                 to the actuator numbers.
;
;   amp:          scalar, float. Strictly positive.
;                 Rms amplitude of the excitation. The current
;                 needed to perform the requested rms amplitude is
;                 estimatated considering a quasi-static excitation.
;                 An error is returned if the estimated rms amplitude
;                 is greater then 50% of the maximum current that the
;                 system can apply. If the keyword OPTIMIZE_RESPONSE
;                 is set, as output amp contains the amplitude used
;                 after the optimization.
;
; OUTPUTS
;
;   err:          scalar long int. Error code (see adsec_error
;                 structure in init_gvar.pro)
;
; KEYWORDS
;
;   OPTIMIZE_RESPONSE   if set, the excitation is first applied with the input amplitude.
;                       After that the amplitude is scaled to give the max position amplitude
;                       without exceed the maximum cuttent amplitude.
;
;   CURR_EXCITE         if set, the excitation is a current (force) excitation
;                       instead of a position-command excitation
;
;   MODAL:              if set, the feed-foreward modes are used, otherwise
;                       a single-actuator excitation is used.
;
;   OLTF:               if set, the Open Loop Tranfer Function is plotted.
;
;   CLTF:               if set, the Closed Loop Tranfer Function is plotted.
;
;   ETF:                if set, the Error Tranfer Function is plotted.
;
;   PTF:                if set, the Plant Transfer Function is plotted. In this case
;                       "plant" TF means actuator+mirror+sensor TF.
;   GAIN_MEAS:          output proportional gain measured
;   GAIN_SPEED_MEAS:    output derivative gain measured
;
; HISTORY
;
;   Written by A. Riccardi (AR)
;   Osservatorio Asrofisico di Arcetri, ITALY
;   <riccardi@arcetri.astro.it>
;
;   13 Mar 2002, AR
;     Bug fixed in the CONT keyword setting of the MESSAGE call.
;   02 Dec 2003, AR
;     CURR_EXCITE keyword is now correctly implemented
;   28 Jul 2004, M. Xompero (MX)
;     Rewriting and managing to match LBT specs.
;     Added adsec.curr_threshold.
;   09 Aug 2004, AR
;     Core reordering for better reading
;   02 Nov 2004, MX
;     Adsec.all changed in sc.all.
;   10 Dec 2004, MX
;     adsec.sampling_time is now double. Derived variables are casted to float.
;     Fixed circular buffer len problem.
;   8 Feb 2005, MX
;     Now if a sensitive drift is present, it is automatically shown.
;   20 settembre 2006, DZ
;     Change read_addr when curr_exite is set. Added keywords GAIN_MEAS, GAIN_SPEED_MEAS, FIRST_VALUE, FIRST_COMM
;   07 Feb 2007 MX
;       LUT of variable deleted and name refurbished.
;   19 Sep 2007 MX
;       Some force control modified for LBT672 unit
;-

@adsec_common

; index vector must be short int
if test_type(index, /INT, DIM=dim) then begin
    message, "index must be short int", CONT=(sc.debug eq 0B)
    return, adsec_error.input_type
endif
; index must be a scalar
if dim[0] ne 0 then begin
    message, "index must be a scalar", CONT=(sc.debug eq 0B)
    return, adsec_error.input_type
endif
; index must be within 0 and adsec.n_actuators-1
if index ge adsec.n_actuators or index lt 0 then begin
    message, "act/mode index is out of range", CONT=(sc.debug eq 0B)
    return, adsec_error.input_type
endif


; amp must be float or short int
if test_type(amp, /INT, /FLOAT, DIM=dim) then begin
    message, "amp must be short int or float", CONT=(sc.debug eq 0B)
    return, adsec_error.input_type
endif
; amp must be a scalar
if dim[0] ne 0 then begin
    message, "amp must be a scalar", CONT=(sc.debug eq 0B)
    return, adsec_error.input_type
end
; amp must be strictly positive
if amp lt 0 then begin
    message, "amp must be strictly positive", CONT=(sc.debug eq 0B)
    return, adsec_error.input_type
endif

if n_elements(n_samples) eq 0 then begin
    ; default: maximum available samples
    n_samples = adsec.max_seq_samples
endif else begin
    err = test_type(n_samples, /NOFLOAT, N_EL=n_el)
    if err ne adsec_error.ok then begin
        message, "N_TIME_SAMPLES must contains integer numbers", $
          CONT= (sc.debug eq 0B)
        return, adsec_error.input_type
    endif
    if n_el ne 1 then begin
        message, "N_TIME_SAMPLES must be a scalar", $
          CONT= (sc.debug eq 0B)
        return, adsec_error.input_type
    endif
    if (n_samples[0] le 100) or $
      (n_samples[0] gt adsec.max_seq_samples) then begin
        message, "N_TIME_SAMPLES must be within 100 and "+ $
          strtrim(adsec.max_seq_samples,2), $
          CONT= (sc.debug eq 0B)
        return, adsec_error.input_type
    endif

endelse


if n_elements(act2plot) eq 0 then begin
    ; default: do not plot acquired data during the measurement
    show_act_plot = 0B
endif else begin
    err = test_type(act2plot, /INT, /LONG, DIM=dim)
    if err ne adsec_error.ok then begin
        message, "ACT_TO_PLOT must be short or long int", $
          CONT= (sc.debug eq 0B)
        return, adsec_error.input_type
    endif
    if dim[0] ne 0 then begin
        message, "ACT_TO_PLOT must be a scalar", $
          CONT= (sc.debug eq 0B)
        return, adsec_error.input_type
    endif
    if (act2plot lt 0) or (act2plot ge adsec.n_actuators) then begin
        message, "ACT_TO_PLOT out of range", CONT= (sc.debug eq 0B)
        return, adsec_error.input_type
    endif
    ; show the plot of acquired data related to act2plot during
    ; the measuremet
    show_act_plot = 1B
endelse


;; not yet supported variables, initialized to zero for
;; future compatibility
if n_elements(read_dec) eq 0 then read_dec=0
if n_elements(cmd_dec) eq 0 then cmd_dec=0

;; minimum frequency without using decimation
f0 = float(1.0/n_samples[0]/adsec.sampling_time)
;; Nyquist frequency without using decimation
fn = float(0.5/adsec.sampling_time)

if n_elements(freq_range) eq 0 then begin
    ;; default values for the frequency range to excite
    freq_range = [f0, fn]
endif else begin
    err = test_type(freq_range, /REAL, N_EL=n_el)
    if err ne adsec_error.ok then begin
        message, "EXCITING_FREQ_RANGE must contains real numbers", $
          CONT= (sc.debug eq 0B)
        return, adsec_error.input_type
    endif
    if n_el ne 2 then begin
        message, "EXCITING_FREQ_RANGE must contains a 2-elements vector", $
          CONT= (sc.debug eq 0B)
        return, adsec_error.input_type
    endif
    if (freq_range[0] le 0) or (freq_range[1] lt freq_range[0]) then begin
        message, "EXCITING_FREQ_RANGE must be positive and " + $
          "freq[0] le freq[1]", CONT= (sc.debug eq 0B)
        return, adsec_error.input_type
    endif

    if freq_range[1] gt fn then begin
        message, "Higher exciting freq. range outside the allowed limits", $
          CONT= (sc.debug eq 0B)
        return, adsec_error.input_type
    endif
    if freq_range[0] lt f0 then begin
        message, "Lower exciting freq. range outside the allowed limits", $
          CONT= (sc.debug eq 0B)
        return, adsec_error.input_type
    endif

    ;; override to the closest allowed frequencies
    freq_range = round(freq_range/f0)*f0
endelse

;
; refresh the status structure
;
err = update_status(n_samples)
if err ne adsec_error.ok then return, err

cl_act = where(sys_status.closed_loop eq 1, n_cl_act)

if (~ keyword_set(modal)) and ~(keyword_set(curr_excite)) then begin
    ; This is the calculus for the zonal excitation for the efficiency of the position command
    ; and ctrl current.
    ; We compute the efficiency as ratio between position requested and position achieved
    ; calculated by feed forward matrix.
    err = get_gain(index, g_act)
    if err ne adsec_error.ok then return, err
    
    err = get_gain(index, g_act_speed, /speed)
    if err ne adsec_error.ok then return, err
    
    g_act=g_act[0]
    dc_relation = invert(-adsec.ff_matrix/(g_act*adsec.gain_to_bbgain[index])+identity(adsec.n_actuators), /double)
    
    gain_meas = g_act
    gain_speed_meas = g_act_speed
endif

;; check if the maximum required step current is too large
;; (larger then adsec.curr_threshold*adsec.max_curr_val)
if keyword_set(curr_excite) then begin
    if keyword_set(modal) then begin
        max_step_curr = max(abs(amp*adsec.ff_f_svec[index,*]))
    endif else begin
        max_step_curr = amp
    endelse
endif else begin
    if keyword_set(modal) then begin
        ;; ff modes case
                                ; 0Hz gain of the CLTF (estimation)
        err = get_gain(cl_act, g)
        if err ne adsec_error.ok then return, err
        g = mean(g*adsec.gain_to_bbgain[cl_act])
        g0Hz = g/(g+adsec.ff_sval[index])
        max_step_curr =  max(abs(adsec.ff_sval[index]*amp*g0Hz * adsec.ff_f_svec[index,*]))
    endif else begin
	;; single actuator case
        dummy_cmd = fltarr(adsec.n_actuators)
        dummy_cmd[index]=max(abs(amp))
        if total(adsec.ff_matrix) eq 0 then curr_reached = g_act *adsec.gain_to_bbgain[0]* dummy_cmd else $
            curr_reached = adsec.ff_matrix ## dc_relation ## dummy_cmd
        curr_reached_max = max(abs(temporary(curr_reached)))
        max_step_curr =  curr_reached_max
    endelse
endelse
    
if max_step_curr*4. gt min(adsec.curr_threshold*adsec.max_curr_val) then begin
    message, "the passed amplitude gives too large current peaks (" $
                 +strtrim(string(max_step_curr*4.,format="(e10.3)"),2)+")", CONT=(sc.debug eq 0B)
    return, adsec_error.input_type
endif

;; check if the maximum required step position is too large
;; (larger then 1um/adsec.m_per_pcount)
if keyword_set(curr_excite) then begin

	if keyword_set(modal) then $
		message, "Current excitation of (low order) modes can be dangerous. Aborted!" $
		       , CONT=(sc.debug eq 0B)
endif else begin
	if keyword_set(modal) then begin
	    ;;
	    ;; ff modes case
	    ;;
	    ;; 0Hz gain of the CLTF (estimation)
	    err = get_gain(cl_act, g)
	    if err ne adsec_error.ok then return, err
	    
	    err = get_gain(cl_act, g_speed, /speed)
	    if err ne adsec_error.ok then return, err
	    
        g = mean(g*adsec.gain_to_bbgain[cl_act])
	    g0Hz = g/(g+adsec.ff_sval[index])
	    max_step_pos =  max(abs(amp*g0Hz * adsec.ff_p_svec[index,*]))

		if max_step_pos gt adsec.max_modal_abs_step/mean(adsec.m_per_pcount[cl_act]) then begin
		    message, "The passed amplitude gives too large position peaks rms (" $
		      +strtrim(max_step_pos,2)+")", CONT=(sc.debug eq 0B)
		    return, adsec_error.input_type
		endif
        gain_meas = g
        gain_speed_meas = g_speed
    endif else begin
        ;;
        ;; single actuator case
        ;;
        dummy_cmd = fltarr(adsec.n_actuators)
        dummy_cmd[index]=adsec.max_modal_abs_step
        pos_real = dc_relation ## dummy_cmd
        pos_real_max = max(abs(temporary(pos_real)))
        pos_efficiency = pos_real_max/adsec.max_modal_abs_step

        max_step_pos = max(abs(amp))
        ;bisogna vedere se index e cl_Act sono compatibili(prima c'era la media di adsec.m_per_pcount[cl_act])
        if (max_step_pos * pos_efficiency) gt adsec.max_modal_abs_step/(adsec.m_per_pcount[index]) then begin
            message, "The passed amplitude gives too large position peaks rms (" $
                     +strtrim(max_step_pos,2)+")", CONT=(sc.debug eq 0B)
            return, adsec_error.input_type
        endif
    endelse
endelse

; compute the normalized time sequence
;
f_min = freq_range[0] & f_max = freq_range[1]
if n_elements(seqpassed) eq 0 then norm_seq = rand_seq(f_min, f_max, float(adsec.sampling_time*(cmd_dec+1)), $
                                   n_samples, SEED=seed, /ZERO_DC) else norm_seq = seqpassed

;;;;;;;;;;;;;;;;;;;;;;;;;;;;; debug lines for meas
if n_elements(seqpassed) eq 0 then save, norm_seq, file='sequence.sav'
;;;;;;;;;;;;;;;;;;;;;;;;;;;;; end debug lines


; allocate memory for output data
n_dsp = adsec.n_bcu * adsec.n_board_per_bcu * adsec.n_dsp_per_board
n_acts= n_dsp * adsec.n_act_per_dsp
buf_mean_dist = dblarr(n_samples, n_acts)
buf_sd_dist   = dblarr(n_samples, n_acts)
;buf_tot_dist  = fltarr(n_samples, n_acts, n_rep)
buf_tot_dist  = fltarr(n_samples, n_acts)
buf_mean_err  = dblarr(n_samples, n_acts)
buf_sd_err    = dblarr(n_samples, n_acts)
buf_tot_err   = fltarr(n_samples, n_acts)
;buf_tot_err   = fltarr(n_samples, n_acts, n_rep)
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;start debug
;first_value   = lonarr(n_rep, n_acts) ;save the first elements of gc for each acquisition
;first_comm   = fltarr(n_rep, n_acts)
;end debug
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; save the current status of the pos or curr command preshaper
;; in order to restore it at the end of the acquisition
err = get_preshaper(sc.all_actuators, cmd_preshaper_save, FF=keyword_set(curr_excite))
if err ne adsec_error.ok then return, err

;; disable the preshaping of the poc or curr commands
err = set_preshaper(sc.all_actuators, dsp_const.fastest_preshaper_step, FF=keyword_set(curr_excite))
if err ne adsec_error.ok then return, err

for do_opt_resp=keyword_set(opt_resp),0,-1 do begin
	; compute the amplitude for each actuator
	;
	if keyword_set(modal) then begin
	    ;; modal excitation
	    if keyword_set(curr_excite) then begin
	        ;; current excitation
	        amp_act = float(amp*reform(adsec.ff_f_svec[index,*]))
	    endif else begin
	        ;; position excitation
	        amp_act = float(amp*reform(adsec.ff_p_svec[index,*]))
	    endelse
	endif else begin
	    ;; single actuator excitation
	    amp_act = fltarr(adsec.n_actuators)
	    amp_act[index] = amp
	endelse

	static_pos = sys_status.position
	static_ctrl_curr = sys_status.ctrl_current
	if keyword_set(curr_excite) then begin
	    offset = sys_status.bias_current
	    addr = dsp_map.bias_current
	endif else begin
	    offset = sys_status.command
	    addr = dsp_map.pos_command
	endelse

	;; setting of the dataIO buffers:
	;; dataIO_0: sequence of position/current commands to send to ch0/1/2/3
	;; dataIO_1: control current to read from ch0/1/2/3
	;; dataIO_2: linearized position to read from ch0/1/2/3
	;;
	;; trigger and command buffers are set as circular and are trigered
	;; when the global
	;; DSP counter assumes a certain value that will be set after all
	;; the buffer configurations and downloading will be ended.
	;; The first condition (circularity) is due to be able for waiting
	;; the stedy state before performing the measurements. The latter
	;; condition (global counter) assures the sincronization of the
	;; commands for all the actuators.
	;;

    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    ;Generating current or command sequence
    seq = rebin(transpose(amp_act), n_samples, adsec.n_actuators, /SAMP) $
             * rebin(norm_seq, n_samples, adsec.n_actuators, /SAMP);*0;+0.1

    ;pari=indgen(8192)*2
    ;dispari=pari+1
    ;idd = indgen(4096)*4
    ;idd = [idd,idd+1]
    ;seq[idd,22]=0.4

    ;add to the generated sequence the offset given by bias_current or command
    seq = temporary(seq) + rebin(transpose(offset), n_samples, adsec.n_actuators, /SAMP)

    ;uploads the command sequences
    n_buf_words = adsec.n_act_per_dsp * long(n_samples)
    for i_dsp=0,n_dsp-1 do begin
        act_idx0 = i_dsp*adsec.n_act_per_dsp
        act_idx1 = act_idx0+adsec.n_act_per_dsp-1
        err = write_IO_buffer(i_dsp, 0, reform(transpose(seq[*,act_idx0:act_idx1]), n_buf_words, /OVER))
        if err ne adsec_error.ok then return, err
    endfor

    ;set the global counter trigger value (1 sec later)
    trig_delay = 1.0;1.0 ;[s]
    ;time in sampling_time units to wait for starting next acquisition
    time_cycles = ulong(trig_delay/adsec.sampling_time)

    ;reference dsp for the GC reading
    dsp_ref = 0
    ;get the current GC value
    t0 = systime(/SEC)
    err = get_global_counter(dsp_ref, initial_gc)
    if err ne adsec_error.ok then return, err

    ; trigger value of global counter to start the command writing in DSP memory
    gc_trig_val_wr = initial_gc + time_cycles

    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    ;set the command buffer 0 triggered to GC
    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    err = set_IO_buffer(sc.all_dsp, 0, addr, n_samples $
                        , dsp_map.global_counter, gc_trig_val_wr, /WRITE_TO_MEM $
                        , /CIRCULAR, /TRIG_ENABLE, RECORD_SIZE=adsec.n_act_per_dsp, /START)
    if err ne adsec_error.ok then return, err

    ;; n_cycles cycles to wait for the steady stated
    n_cycles = 5
    ;; trigger value of global counter to start the position and ctrl current acquisition
    gc_trig_val_acq = ulong(n_samples*(cmd_dec+1)*n_cycles) + gc_trig_val_wr
   
    if do_opt_resp then begin
         read_addr = [dsp_map.float_dac_value, dsp_map.distance] 
    endif else begin
        if keyword_set(curr_excite) then read_addr = [dsp_map.float_dac_value, dsp_map.distance] else read_addr = [dsp_map.pos_current, dsp_map.distance]
    endelse
    

    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    ;START OF DEBUGGING LINES
    ;read_addr = [dsp_map.pos_command, dsp_map.distance]
    ;read_addr = [dsp_map.global_counter, dsp_map.pos_command]
    ;END OF DEBUGGING LINES
    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    ;START OF DEBUGGING LINES ; 29 Agosto 2006
     ;read_addr = [dsp_map.float_dac_value, dsp_map.distance]
    ;END OF DEBUGGING LINES
    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    
    ;; enable the reading buffers. The trigger is triggered for starting the
    ;; acquisition at the occurrence of the begining of the sequence
    err = set_IO_buffer(sc.all_dsp, [1,2], read_addr, n_samples $
                        , dsp_map.global_counter, gc_trig_val_acq, $
                        /TRIG_ENABLE, RECORD_SIZE=adsec.n_act_per_dsp, /START)
    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    ;; start patch when just one buffer can be used 
    ;err = set_IO_buffer(sc.all_dsp, 1,  read_addr[1], n_samples $
    ;                    , dsp_map.global_counter, gc_trig_val_acq, $
    ;                            ;               /TRIG_ENABLE,
    ;                    RECORD_SIZE=adsec.n_act_per_dsp, /START)
    ;; end patch
    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    if err ne adsec_error.ok then return, err
    t1 = systime(/SEC)
    print, "Delay set [s]:",trig_delay, ", actual delay:",t1-t0
    ;; start debug line
    if t1-t0 gt trig_delay then message, "UNEXPECTED ERROR!", /info
    ;; end debug line

    ;; wait for the end of acquisition.
    ;;
    wait, (gc_trig_val_acq+n_samples*(cmd_dec+1)-initial_gc)*adsec.sampling_time > 0.01

    ;; if optimize response is set, it doesn't repeat the acquisition to increase SNR.
    if do_opt_resp then tmp_n_rep=1 else tmp_n_rep=n_rep

    for i_rep=0,tmp_n_rep-1 do begin

        print, 'Measure ', strtrim(i_rep+1,2), " of ", strtrim(tmp_n_rep,2)
        ; global counter at the end of the acquisition
        gc_trig_val_end = gc_trig_val_acq+n_samples*(cmd_dec+1UL)

        ;; polling
        count=0
        mcount=3
        done=0
        while not done do begin
            err = check_working_IO(sc.all_dsp, 0, aw_circ)
            if err ne adsec_error.ok then return, err
            err = check_working_IO(sc.all_dsp, 1, aw_r2)
            if err ne adsec_error.ok then return, err
            err = check_working_IO(sc.all_dsp, 2, aw_r3)
            if err ne adsec_error.ok then return, err
            
            
            if aw_circ ne 1 then begin
                message,"The circular buffer doesn't work!", CONT=sc.debug ne 1B
                return, adsec_error.buffer_timeout
            endif
            if aw_r2 ne 0 or aw_r3 ne 0 then begin
                count=count+1
                print, 'Polling for the end of acquisition....'
                err = get_global_counter(dsp_ref, actual_gc)
                ;;start debug 
                err = read_seq_dsp(0, dsp_map.DAC_NOT_READY , 1l, dnr , /ul)
                print, dnr
                err = read_seq_dsp(0, dsp_map.DAC_NOT_sent , 1l, dns , /ul)
                print, dns
                ;;end debug
                if actual_gc lt gc_trig_val_end then $
                  wait, (gc_trig_val_end-actual_gc)*adsec.sampling_time > 0.01
                  print, actual_gc
                  print, gc_trig_val_end
                done=0
             
            endif else done=1           
        endwhile
        if count eq mcount then begin
            message, "The acquisition buffers do not stop", CONT=sc.debug ne 1B
            return, adsec_error.buffer_timeout
        endif
        ;; download the acquired data
        
        ;;;;;;;;;;;;; start  debug
        ;print, aw_r2, aw_r3
        ;print, check_working_io(0,[0,1,2,3], w=map)
        ;print, conv2bin(map)
        ;;;;;;;;;;;;; end debug
        
        err = read_IO_buffer(sc.all_dsp, 1, n_buf_words, buf_err_ctrl)
        if err ne adsec_error.ok then return, err
        buf_err_ctrl = reform(buf_err_ctrl, adsec.n_act_per_dsp, n_samples, n_dsp, /OVERWRITE)
        buf_err_ctrl = reform(transpose(temporary(buf_err_ctrl), [1,0,2]), n_samples, n_acts, /OVER)

        err = read_IO_buffer(sc.all_dsp, 2, n_buf_words, buf_dist)
        if err ne adsec_error.ok then return, err
        buf_dist     = reform(buf_dist, adsec.n_act_per_dsp, n_samples, n_dsp, /OVERWRITE)
        buf_dist     = reform(transpose(temporary(buf_dist), [1,0,2]), n_samples, n_acts, /OVER)

        t0 = systime(/SEC)
        ;get the current GC value
        err = get_global_counter(dsp_ref, new_gc)
        if err ne adsec_error.ok then return, err
        ;gc_trig_val_nwr = new_gc+time_cycles ;da togliere
        
        gc_trig_val_acq = ((new_gc-gc_trig_val_wr+time_cycles)/(n_samples*(cmd_dec+1))+1)*(n_samples*(cmd_dec+1)) $
                          +gc_trig_val_wr ;da ripristinare
        
         
        err = set_IO_buffer(sc.all_dsp, [1,2], read_addr, n_samples $
                            , dsp_map.global_counter, gc_trig_val_acq $
                            , /TRIG_ENABLE, RECORD_SIZE=adsec.n_act_per_dsp, /START)
        if err ne adsec_error.ok then return, err
        t1 = systime(/SEC)
        print, "Delay set[s]:",trig_delay, ", actual delay:",t1-t0
        if t1-t0 gt trig_delay then message, 'the actual delay is too long!!', /info;return, adsec_error.generic_error

        if show_act_plot then begin
            set_plot, 'x'
            if i_rep eq 0  then begin
                window, /FREE, retain=2
                act_plot_win = !d.window
            endif
            wset, act_plot_win
            !p.multi = [0,1,2]

            dsp_idx = act2plot/adsec.n_act_per_dsp
            act_idx = act2plot mod adsec.n_act_per_dsp

            t_vec = indgen(n_samples)*adsec.sampling_time*(read_dec+1)
            if keyword_set(curr_excite) then begin
                tmp = max(abs([buf_dist[*,act2plot] - static_ctrl_curr[act2plot] $
                               ,seq[*,act2plot] - offset[act2plot]]))
                plot, t_vec*1e3 $
                      , buf_dist[*, act2plot] $
                      - static_pos[act2plot] $
                      , XSTY=17, YSTY=17, XTIT="[ms]" $
                      , YTICKF="(e10.3)", TITLE="Pos-static pos, act#"+strtrim(act2plot,2) $
                      , SUBTITLE="idx="+strtrim(index,2), PSYM=3
                plot, t_vec*1e3, seq[*,act2plot] - offset[act2plot] $
                      , XSTY=17, YSTY=17, XTIT="[ms]" $
                      , YTICKF="(e10.3)", TITLE="Ctrl curr-static ctrl curr", /LINE $
                      , YR=tmp*[-1,1], PSYM=3
                oplot, t_vec*1e3 $
                       , buf_err_ctrl[*, act2plot] $
                       - static_ctrl_curr[act2plot], COL=255L
            endif else begin
                tmp = max(abs([buf_dist[*, act2plot]- static_pos[act2plot] $
                               ,seq[*,act2plot] - offset[act2plot]]))

                plot, t_vec*1e3, seq[*,act2plot] - offset[act2plot] $
                      , XSTY=17, YSTY=17, XTIT="[ms]" $
                      , YTICKF="(e10.3)", TITLE="Pos-static pos, act#"+strtrim(act2plot,2) $
                      , /LINE, SUBTITLE="idx="+strtrim(index,2), YR=tmp*[-1,1], PSYM=3
                oplot, t_vec*1e3 $
                       , buf_dist[*, act2plot] $
                       - static_pos[act2plot], COL=255L
                plot, t_vec*1e3 $
                      , buf_err_ctrl[*, act2plot] $
                      - static_ctrl_curr[act2plot] $
                      , XSTY=17, YSTY=17, XTIT="[ms]" $
                      , YTICKF="(e10.3)", TITLE="Ctrl curr-static ctrl curr", PSYM=3
            endelse

            !p.multi=0
        endif

        ;; if the reading from program memory give a disturbing
        ;; cross-talk to the DSPs, wait 1s before performing the
        ;; next acquisition
        ;;
        wait, 1.0

        if not do_opt_resp then begin
;            buf_tot_dist[*,*,i_rep]=buf_dist
            buf_tot_dist=buf_dist
;            buf_tot_err[*,*,i_rep]=buf_err_ctrl
            buf_tot_err=buf_err_ctrl
            buf_mean_dist = buf_mean_dist + double(buf_dist)
            buf_sd_dist   = buf_sd_dist + double(buf_dist)^2
            buf_mean_err = buf_mean_err + double(buf_err_ctrl)
            buf_sd_err   = buf_sd_err + double(buf_err_ctrl)^2
            ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
            ;; START DEBUG LINES
            ;first_value[i_rep,*]  = long(buf_err_ctrl[0,*],0,n_acts)
            ;first_comm[i_rep,*]  = buf_dist[0,*]
            ;; END DEBUG LINES
            ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
        endif
        print,'fine giro' 
    wait,  (gc_trig_val_acq+n_samples*(cmd_dec+1)-new_gc)*adsec.sampling_time > 0.01
endfor

    ;; stop the cycling buffers (0 and 1) and wait for their stop
    ;;commento momentaneo!!!!!!!!!!!!!!!!!!!!!!!!!!!1
    err = disable_IO_buffer(sc.all_dsp, [0])
    if err ne adsec_error.ok then return, err

    ;; restore the previous status of the preshaper
    ;;
    err = set_preshaper(sc.all_actuators, cmd_preshaper_save, FF=keyword_set(curr_excite))
    if err ne adsec_error.ok then return, err

    ;; restore the static initial conditions
    ;;
    if keyword_set(curr_excite) then begin
        err = write_bias_curr(sc.all_actuators, offset)
        if err ne adsec_error.ok then return, err
    endif else begin
        err = write_des_pos(sc.all_actuators, offset)
        if err ne adsec_error.ok then return, err
    endelse
    
    wait, time_preshaper()

    if do_opt_resp then begin
        ;
        ; compute the optimal amp giving no more then 8000 current or 1000 position peak
        ave_tmp = rebin(rebin(buf_dist[*,cl_act],1,n_cl_act),n_samples,n_cl_act,/SAMP)
        max_amp_pos = max(abs(buf_dist[*,cl_act]-ave_tmp),idx)
        print, "max pos amp: "+strtrim(max_amp_pos,2)+" @act#"+strtrim(cl_act[idx/n_samples],2)

        ave_tmp = rebin(rebin(buf_err_ctrl[*,cl_act],1,n_cl_act),n_samples,n_cl_act,/SAMP)
        max_amp_cur = max(abs(buf_err_ctrl[*,cl_act]-ave_tmp),idx)
        print, "max cur amp: "+strtrim(max_amp_cur,2)+" @act#"+strtrim(cl_act[idx/n_samples],2)

        amp_fact = min([adsec.max_dyn_peak_pos/max_amp_pos,adsec.weight_curr*adsec.weight_times4peak/max_amp_cur])
        ;amp_fact = 1.95869 ; da togliere
        print, "amp_fact: "+strtrim(amp_fact,2)
        ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;; DEBUG LINE
        ;amp_fact=1.
        ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;; ;;;;;;;;;;;;;;;;;;;;;;;;;

        amp = amp*amp_fact

;        if keyword_set(curr_excite) then  ratio = min(check_vsat(norm_seq*amp))


    endif

    ;; disable the preshaping of the poc or curr commands
    err = set_preshaper(sc.all_actuators, dsp_const.fastest_preshaper_step, FF=keyword_set(curr_excite))
    if err ne adsec_error.ok then return, err

endfor

;; restore the previous status of the preshaper
;;
err = set_preshaper(sc.all_actuators, cmd_preshaper_save, FF=keyword_set(curr_excite))
if err ne adsec_error.ok then return, err


;; wait for the steady state
;;
wait, 1.0

buf_mean_dist = buf_mean_dist/n_rep
buf_sd_dist = (buf_sd_dist/n_rep - buf_mean_dist^2)/n_rep

buf_mean_err = buf_mean_err/n_rep
buf_sd_err = (buf_sd_err/n_rep - buf_mean_err^2)/n_rep

;; fill the data structure with the acquired data
;;

; allocate the structure where storing the data
data = $
  { $
  modal       : keyword_set(modal), $
  curr_excite : keyword_set(curr_excite), $
  index       : index, $
  amp         : amp, $
  n_rep       : n_rep, $
  freq_range  : freq_range, $
  decimation  : read_dec, $
  sequence    : fltarr(n_samples, adsec.n_actuators), $
  position    : fltarr(n_samples, adsec.n_actuators), $
  position_sd : fltarr(n_samples, adsec.n_actuators), $ ;stdev of the mean
  ctrl_current: fltarr(n_samples, adsec.n_actuators), $
  ctrl_current_sd: fltarr(n_samples, adsec.n_actuators) $
  }

;; cmd_dec = read_dec in this implementation, otherwise
;; some problems occurr when the tranfer functions will
;; be computed (frequency points mismatching)
data.sequence = seq
data.position = buf_mean_dist
data.position_sd = buf_sd_dist
data.ctrl_current = buf_mean_err
data.ctrl_current_sd = buf_sd_err

; clear all dacs
;err = clear_dacs(sc.all)
;  if err ne adsec_error.ok then  return, err
  

;; check for drifts from the beginning of the measurement
;;
err = update_status(n_samples)
if err ne adsec_error.ok then return, err

drift_pos = static_pos-sys_status.position
drift_ctrl_curr = static_ctrl_curr-sys_status.ctrl_current
print, "Max position drift    :", max(abs(drift_pos[adsec.act_w_pos]))
print, "Max ctrl current drift:", max(abs(drift_ctrl_curr[adsec.act_w_curr]))

;; ask for displaying the drift shape if the position drift is
;; estimated to be too large (2 times the max-min position during
;; an acquition of n_samples samples per actutor)
;;
if total(abs(drift_pos[adsec.act_w_pos]) gt 2*sys_status.position_sd[adsec.act_w_pos]) then begin
    message, "A sensitive drift in position has been measured", /INFO
    window, /FREE, retain=2
    display, drift_pos[adsec.act_w_pos], adsec.act_w_pos, /NO_NUMBER, /SHOWBAR, TITLE="Position drift"
endif
return, adsec_error.ok
end

