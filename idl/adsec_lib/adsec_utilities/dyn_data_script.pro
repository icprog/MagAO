; $Id: dyn_data_script.pro,v 1.2 2007/12/12 14:11:19 labot Exp $
;+
; HISTORY
;   Ago 2004, M. Xompero (MX)
;     modifications to match LBT secifications.
;     configuration file conf/dyn_data_defalt.txt for input parameters.
;
;   09 Ago 2004, A. Riccardi (AR)
;     code reordering for better reading
;   02 Dec 2004, MX
;     modified format of plot labels
;-
loadct, 12
tpt = systime(/sec)
;restoring default data
default=read_ascii_structure(filepath(ROOT=adsec_path.conf,SUB="scripts", "dyn_data_default.txt"))
;setting up...
;file on output
;dir = filepath(ROOT=adsec_path.meas, SUB=[default.path_0,default.path_1], '')
dir = meas_path('dyn')
ext = default.ext
 aostatus = ptr2value(!AO_STATUS)


;save the data if set
do_save = default.do_save
if do_save then begin
    err = get_status(status_save)
    if err ne adsec_error.ok then message, "Error"
    adsec_save = adsec
    save, adsec_save, status_save, aostatus,FILE=filepath(ROOT=dir, "adsec_save"+ext+".sav")
endif

;print, 'Checking the syncronization of DSPs...'
;err = sync_dsp(N_CYCLES=default.n_cycles_sync, /CHECK)
;if err ne adsec_error.ok then begin
;    message, "The DSPs are not syncronized. Procedure aborted."  ;, CONT= (sc.debug eq 0B)
;;    return, adsec_error.sync_dsp_fail
;endif

;------------------------------------------------
;stop master diagnostic
;err = get_master_diagn_freq(ENABLE=enable_fast)

;if err ne adsec_error.ok then message, 'Error: ', err
old_ovs = rtr.oversampling_time
;if enable_fast ne 0 then begin
    err = set_diagnostic(OVER=0.0)
    if err ne adsec_error.ok then  message, 'Error: ', err
    err = set_diagnostic(MASTER=0.0)
    if err ne adsec_error.ok then  message, 'Error: ', err
;endif
wait, 2*old_ovs
;-----------------------------------------------

curr_excite = default.curr_excite
n_rep = default.n_rep
n_samples = default.n_samples

f0 = float(1.0/n_samples/adsec.sampling_time)  ; frequency resolution bin
fn = float(0.5/adsec.sampling_time)            ; Nyquist frequency
min_freq = (round(default.min_freq/f0)*f0) > f0
freq_step = (round(default.freq_step/f0)*f0) > f0
max_freq = ((round(default.max_freq/f0)*f0) > f0) < fn
n_step = round((default.max_freq-default.min_freq)/default.freq_step) > 1

do_optimize = default.do_optimize

if n_step eq 1 then begin
    max_freq = round(max_freq/f0)*f0
    freq_step = max_freq-min_freq
    n_step = 1
endif else begin
    max_freq = min_freq+n_step*freq_step < fn
endelse

;updating the status of IDL registers.
err = update_status()
if err ne adsec_error.ok then message, "ERROR" ;return, err

cl_act = fix(where(sys_status.closed_loop, n_cl_act))
; check if all the actuators are set with the control enabled
if n_cl_act ne n_elements(adsec.act_w_cl) then begin
    answ = dialog_message(["Some of the actuators for which it is possible" $
                          , "to close the loop are in open loop.", "Do you want to continue?"] $
                          , /QUEST)
    if strlowcase(answ) eq "no" then message, "Script aborted." ;return, adsec_error.ctrl_disabled
endif

position = mean(sys_status.command[cl_act])
if position ge 0 then strsign="+" else strsign="-"

err = get_gain(cl_act, act_gain)
if err ne adsec_error.ok then message, "ERROR" ;return, err

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; modal excitation
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
modal = 1B

mod_list = default.mod_list
if mod_list[0] eq sc.all then mod_list = fix(where(adsec.ff_sval ne 0.0))
if mod_list[0] ge 0 then begin
    tmp= intersection(fix(where(adsec.ff_sval ne 0.0)),mod_list,out_mod)
    if n_elements(out_mod) ne n_elements(mod_list) then message,'Wrong mode selection. Script aborted!'
endif

if (mod_list[0] ge 0) then begin
    if curr_excite then message,"No modal current command allowed."

    strmodal = "m"
    max_amp = default.max_amp_modal
    stramp = strtrim(string(max_amp,format="(e8.1)"),2)

    gain = mean(act_gain*adsec.gain_to_bbgain[cl_act]) ;; appossimation of modal gain
    fact = mean((adsec.N_per_ccount/adsec.m_per_pcount)[cl_act])

    for i_fstep=default.restorecycle,n_step-1 do begin

        freq_range = (min_freq+(i_fstep+[0.0,1.0])*freq_step) < max_freq
        print, "Step "+strtrim(i_fstep+1,2)+"/"+strtrim(n_step,2)
        
        if i_fstep eq default.restorecycle then irec=default.restoremode else irec=0
        for i=irec,n_elements(mod_list)-1 do begin
            index = mod_list[i]
            print, "Mode #", strtrim(index,2)

            ;; estimation of CL 0Hz gain
            cl_dc_gain = gain/(gain+adsec.ff_sval[index])
            ;; estimation of CL bandwidth
            cl_bw = sqrt((gain+adsec.ff_sval[index])*fact/(adsec_shell.mass/n_cl_act))/(2*!PI)
            if freq_range[1] gt cl_bw then freq1 = cl_bw else freq1 = freq_range[1]
            if freq_range[0] gt cl_bw then begin
                message, "Excitation frequency is outside the mode CL bandwidth", /INFO
                ratio=1.0
            endif else begin
                freq0 = freq_range[0]
                if freq0 eq freq1 then message, "No excitation can apply!!!because frequency range is 0"
                ratio = sqrt((freq_range[1]-freq_range[0])/(freq1-freq0))/cl_dc_gain < 1.0
            endelse

            amp = (max_amp*ratio)
            max_curr_amp = (amp*adsec.ff_sval[index]) < adsec.weight_curr * adsec.weight_times4peak
            amp = float(max_curr_amp/adsec.ff_sval[index] * (gain+adsec.ff_sval[index])/gain)
            dummy = max(abs(adsec.ff_p_svec[index,*]),act2plot)

            err = dyn_data_acquisition(n_rep, index, amp, data, MODAL=modal $
                                       , EXCITING_FREQ_RANGE=freq_range $
                                       , N_TIME_SAMPLES=n_samples $
                                       , DRIFT_POS=drift_pos $
                                       , DRIFT_CTRL_CURR=drift_ctrl_curr $
                                       , BUF_TOT_DIST=buf_tot_dist,BUF_TOT_ERR=buf_tot_err $
                                       , ACT2PLOT=act2plot,OPTIMIZE=do_optimize $       
                                       , GAIN_MEAS = gain_meas, GAIN_SPEED_MEAS = gain_speed_meas) ;, FIRST_VALUE = first_value,  FIRST_COMM = first_comm $
            if err ne adsec_error.ok then message, "ERROR" ;return, err

            err = dyn_data_elab(data, fft_data, MODAL=modal, MODE_INDEX=index)
            if err ne adsec_error.ok then message, "ERROR" ;return, err

            wdel
            if (freq_range[1]-freq_range[0]) ne 0 then begin
                err = dyn_comp_tf(fft_data, /PLOT_TF, INDEX=index, OLTF=oltf, CLTF=cltf, PTF=ptf)
                if err ne adsec_error.ok then message, "ERROR"
            endif

            strpos=strtrim(string(position,format="(e8.1)"),2)

            if do_save then begin
                filename="dyn_"+strsign+strpos+"_"+stramp+"_"+strmodal+string(index,format='(I3.3)')$
                     +"_"+ext+string(i_fstep,FORMAT="(I2.2)")+'.sav'
                save, FILE=filepath(ROOT=dir, filename), n_rep, index, amp, data, modal, $
                      freq_range, n_samples, drift_pos, drift_ctrl_curr, act_gain,gain_meas, gain_speed_meas; $, first_value,first_comm
            endif
        endfor
    endfor
endif


;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; zonal excitation
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
modal = 0B

act_list = default.act_list
if act_list[0] eq sc.all then act_list = cl_act
if act_list[0] ge 0 then begin
    tmp = intersection(cl_act, act_list,out_int)
    if n_elements(out_int) ne n_elements(act_list) then message,'Actuators range is not valid! Script aborted!'
endif

if (act_list[0] ge 0) then begin
    strmodal = "a"
    max_amp = default.max_amp_zonal
    stramp = strtrim(string(max_amp,format="(e8.1)"),2)

    if check_channel(act_list, /NOALL) then begin
        message, "Unexpected list of actuators." ;,CONT=sc.debug eq 0B
        ;return, adsec_error.input_type
    endif

    for i_fstep=default.restorecycle,n_step-1 do begin

        freq_range = (min_freq+(i_fstep+[0.0,1.0])*freq_step) < max_freq
        print, "Step "+strtrim(i_fstep+1,2)+"/"+strtrim(n_step,2)
        
        if i_fstep eq default.restorecycle then irec=default.restoremode else irec=0
        
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
        undefine, seqpassed
;restore, 'sequence.sav', /ver
;seqpassed = norm_seq
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
        
        for i=irec,n_elements(act_list)-1 do begin
            index = act_list[i]
            print, "act #", strtrim(index,2)
            amp = max_amp
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
            if curr_excite then begin
                err = get_gain(index, pgain)
                err = get_gain(index, sgain, /SPEED)
                err = set_gain(index, 0.0)
                err = set_gain(index, 0.0, /SPEED)
            endif
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
            err = dyn_data_acquisition(n_rep, index, amp, data, MODAL=modal $
                                       , EXCITING_FREQ_RANGE=freq_range $
                                       , N_TIME_SAMPLES=n_samples $
                                       , DRIFT_POS=drift_pos $
                                       , DRIFT_CTRL_CURR=drift_ctrl_curr $
                                       , CURR_EXCITE=curr_excite $
                                       , BUF_TOT_DIST=buf_tot_dist,BUF_TOT_ERR=buf_tot_err $
                                       , ACT2PLOT=index, SEQPASSED=seqpassed $
                                       , GAIN_MEAS = gain_meas, GAIN_SPEED_MEAS = gain_speed_meas)
            if err ne adsec_error.ok then message, "ERROR" ;return, err

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
            if curr_excite then begin
                err = ramp_gain(index, sgain, /SPEED)
                err = ramp_gain(index, pgain)
            endif
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
            
            err = dyn_data_elab(data, fft_data, MODAL=modal, MODE_INDEX=index)
            if err ne adsec_error.ok then message, "ERROR" ;return, err
            
            wdel
            if (freq_range[1]-freq_range[0]) ne 0 then begin
                if curr_excite then $
                  err = dyn_comp_tf(fft_data, /PLOT_TF, INDEX=index, PTF=ptf,  CLTF=cltf) $
                else $
                  err = dyn_comp_tf(fft_data, /PLOT_TF, INDEX=index, OLTF=oltf, CLTF=cltf, PTF=ptf)
            endif
            if err ne adsec_error.ok then message, "ERROR" ;return, err
            ;;;;;;;;;;;;;;;;;;;;;;;;;;;;; debug line for meas
            restore, 'sequence.sav', /ver
            seqpassed = norm_seq
            ;;;;;;;;;;;;;;;;;;;;;;;;;;;;; debug_ line for meas
            if do_save then begin

                strpos=strtrim(string(position,format="(e8.1)"),2)
                
                filename="dyn_"+strsign+strpos+"_"+stramp+"_"+strmodal+string(index,format='(I3.3)')$
                         +"_"+ext+string(i_fstep,FORMAT="(I2.2)")+'.sav'
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;debug lines for meas;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
                save, FILE=filepath(ROOT=dir, filename), n_rep, index, amp, data, modal, $
                      freq_range, n_samples, drift_pos, drift_ctrl_curr, act_gain, seqpassed
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;debug lines for meas;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;                save, FILE=filepath(ROOT=dir, filename), n_rep, index, amp, data, modal, $
;                      freq_range, n_samples, drift_pos, drift_ctrl_curr, act_gain, seqpassed
            endif
        endfor
        undefine, seqpassed
    endfor
endif


;if enable_fast ne 0 then begin
    ;err = rip(/XADSEC)
     ;   if err ne adsec_error.ok then  message, 'Error: ', err
    err = set_diagnostic(/master)
        if err ne adsec_error.ok then  message, 'Error: ', err
    err = set_diagnostic(over=old_ovs)
        if err ne adsec_error.ok then  message, 'Error: ', err
;endif

tpt1 = systime(/sec)
if  (xregistered('xadsec') gt 0) then update_panels, /OPT
end
