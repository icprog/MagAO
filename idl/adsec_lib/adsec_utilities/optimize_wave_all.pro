
;main
;test to search the best parameters configuration for the signal wave.
; HISTORY
; 2005 Created by D.Zanotti.
; Jan 2006 
;  Changed the main routine into a pro routine.
;    
 
pro optimize_wave_all, act, RESUME_COUNT=resume_count, TEST_TEMP = test_temp, VERB=VERB, PATH_SAVE = path_save, FILENAME=filename, CONF_FILE=conf_file,$
EXT = ext

@adsec_common
sc.debug=0
if keyword_set(path_save) then path = path_save else path = adsec_path.meas+'wave/2006_01_11/'
if ~keyword_set(CONF_FILE) then conf_file=adsec_path.conf+'conf_opt_wave.txt'
if n_elements(act) eq 0 then message, 'No Actuators was chosen!!'
if act eq -1 then act=findgen(adsec.n_actuators)
;verb = 0B
if not keyword_set(resume_count) then resume_count = 0L
if not  keyword_set(test_temp) then test_temp=0 ; if set 1 enable the test on the board temperatures

;ext='_a'

data = read_ascii_structure(filepath(ROOT=adsec_path.conf,"wave.txt"))
t_clock = 1d0/adsec.clock_freq
params = read_ascii_structure(conf_file)


if n_elements(params) eq 0 then begin 
;f_min = 70d3
;f_max = 71d3
;f_step= (f_max-f_min)/6
;wave_ADC_freq_ratio = 1
    f_min = 140d3;143500d0;134d3;130d3
    f_max = 150d3 ;145d3;153d3;150d3
    f_step= (f_max-f_min)/32.
    wave_ADC_freq_ratio = 2

; ADC_len_min = adsec.min_ADC_samp_len
; ADC_len_max = adsec.max_ADC_samp_len
; ADC_len_min = 0.5d-6
    ADC_len_max = 0.68d-6 ;*******************
    ADC_len_min = 0.56d-6
;ADC_len_max = 0.56d-6
; ADC_len_step = (ADC_len_max-ADC_len_min)/3
    ADC_len_step = 1d-6

    max_dt = 1/f_min/2
    max_flat_dt = max_dt-adsec.wave_settling
    ADC_pos_step = max_flat_dt/8
    ADC_pos_min =0d0; (1/f_max/2)-adsec.wave_settling ;0d0
    ADC_pos_max = 10d-6;**************10.0

; ADC_pos_min = 0.7d-6
; ADC_pos_max = 0.9d-6
; ADC_pos_step = .05d-6

    capsens_len_min = 1d-6
 ; capsens_len_min = ceil(capsens_len_min/t_clock)*t_clock
    capsens_len_max = 10d-6;**********10d0
    capsens_len_step = (max_flat_dt-capsens_len_min)/8
; capsens_len_step = (max_flat_dt-capsens_len_min)/1 ;????????????????????????????????????????????????????????????????
; capsens_len_min = 2.d-6
; capsens_len_max = 4.d-6
; capsens_len_min = 2.6d-6
; capsens_len_max = 2.6d-6
; capsens_len_step = .3d-6

    capsens_pos_step = max_flat_dt/10
    capsens_pos_min = 0d;*************0d0
    capsens_pos_max = 10d-6;*************10d0
; capsens_pos_min = 5.2d-6
; capsens_pos_max = 5.6d-6
; capsens_pos_step = .07d-6

    DAC_delay_min=2.6d-6;
    DAC_delay_max=13.0d-6;
    DAC_delay_step =(DAC_delay_max-DAC_delay_min)/10
;DAC_delay_step =(DAC_delay_max-DAC_delay_min);?????????????????????????????????????????????????????????
endif else begin
    f_min = params.f_min
    f_max =  params.f_max
    f_step= (f_max-f_min)/params.f_n_step
    wave_ADC_freq_ratio = params.wave_ADC_freq_ratio

; ADC_len_min = adsec.min_ADC_samp_len
; ADC_len_max = adsec.max_ADC_samp_len
; ADC_len_min = 0.5d-6
    ADC_len_max = params.ADC_len_max ;*******************
    ADC_len_min = params.ADC_len_min
;ADC_len_max = 0.56d-6
; ADC_len_step = (ADC_len_max-ADC_len_min)/3
    ADC_len_step = params.ADC_len_step

    max_dt = 1/f_min/2
    max_flat_dt = max_dt-adsec.wave_settling
    ADC_pos_step = max_flat_dt/params.ADC_pos_n_step  
    ADC_pos_min =params.ADC_pos_min; (1/f_max/2)-adsec.wave_settling ;0d0
    ADC_pos_max = params.ADC_pos_max;**************10.0

; ADC_pos_min = 0.7d-6
; ADC_pos_max = 0.9d-6
; ADC_pos_step = .05d-6

    capsens_len_min = params.capsens_len_min
 ; capsens_len_min = ceil(capsens_len_min/t_clock)*t_clock
    capsens_len_max = params. capsens_len_max;**********10d0
    capsens_len_step = (max_flat_dt-capsens_len_min)/params.capsens_len_step  
; capsens_len_step = (max_flat_dt-capsens_len_min)/1 ;????????????????????????????????????????????????????????????????
; capsens_len_min = 2.d-6
; capsens_len_max = 4.d-6
; capsens_len_min = 2.6d-6
; capsens_len_max = 2.6d-6
; capsens_len_step = .3d-6

    capsens_pos_step = max_flat_dt/params.n_capsens_pos_step 
    capsens_pos_min = params.capsens_pos_min;*************0d0
    capsens_pos_max = params.capsens_pos_max;*************10d0
; capsens_pos_min = 5.2d-6
; capsens_pos_max = 5.6d-6
; capsens_pos_step = .07d-6

    DAC_delay_min = params.DAC_delay_min
    DAC_delay_max= params.DAC_delay_max
    DAC_delay_step =(DAC_delay_max-DAC_delay_min)/params.n_DAC_delay_step
;DAC_delay_step =(DAC_delay_max-DAC_delay_min);?????????????????????????????????????????????????????????

endelse
count = 0L

block_size = 10000
test_points = dblarr(6,block_size)

;parametri di controllo sulla temperatura
    mem_frames = 0
    th_bcu = 60
    th_sig = 45
    th_dsp = 40
    boolean = intarr(9)
    filecount = path+"count2res.sav"
;

for freq=f_min,f_max,f_step do begin

    dt = 1/freq/2
    data.desired_wave_freq = freq
    data.desired_ADC_samp_pos = dt/2
    data.desired_capsens_samp_pos = dt/2
    data.desired_ADC_samp_len = ADC_len_min
    data.desired_capsens_samp_len = dt/4
    data.desired_delay_dac = DAC_delay_min
    data.wave_ADC_freq_ratio = wave_ADC_freq_ratio

    err = init_adsec_wave(data, STRUC=comp_data, /NOFILL)
    if err ne adsec_error.ok then message, "ERROR!"
    dt = 1/comp_data.wave_freq/2
    flat_dt = dt-adsec.wave_settling

     if DAC_delay_min lt adsec.wc_comp_time+2*t_clock then DAC_delay_min = adsec.wc_comp_time+2*t_clock
;    DAC_delay_max =comp_data.sampling_time-adsec.dsp2dac_transfer_time-2*t_clock
    DAC_delay_step =(DAC_delay_max-DAC_delay_min)/params.n_DAC_delay_step

    data.desired_delay_dac = DAC_delay_min

    for ADC_len=ADC_len_min,min([flat_dt-2*t_clock,ADC_len_max]),ADC_len_step do begin
        a=1
        for ADC_pos=max([2*t_clock+ADC_len,ADC_pos_min]),min([dt-2*t_clock,ADC_pos_max]),ADC_pos_step do begin
            a=1
            for DAC_delay_pos=DAC_delay_min, DAC_delay_max, DAC_delay_step do begin
                a=1
	        for capsens_len=capsens_len_min,min([flat_dt-t_clock,capsens_len_max]), capsens_len_step do begin
                    a=1
	            for capsens_pos=max([t_clock+capsens_len,capsens_pos_min]),min([flat_dt,capsens_pos_max]),capsens_pos_step do begin
                       data.desired_ADC_samp_pos = ADC_pos
                       data.desired_capsens_samp_pos = capsens_pos
                       data.desired_ADC_samp_len = ADC_len
                       data.desired_capsens_samp_len = capsens_len
                       data.desired_delay_dac = DAC_delay_pos

                       err = init_adsec_wave(data, STRUC=comp_data, /NOFILL, VERB=verb)
                       if err ne adsec_error.ok then message, "ERROR!"

                       if count gt (size(test_points,/DIM))[1]-1 then begin
                           test_points = [[temporary(test_points)],[dblarr(6,block_size)]]
                       endif
                       test_points[*,count]=[freq, ADC_len, ADC_pos, capsens_len, capsens_pos, DAC_delay_pos]
                       count += 1L
                   endfor
                endfor
            endfor
        endfor
    endfor
endfor

test_points = (temporary(test_points))[*,0:count-1]
;stop
n_act = n_elements(act)
    
    filename = path+"optimize_wave_"+string(n_act,FORMAT="(I3.3)")+"acts_resume_"+strtrim(resume_count,2)+ext+".txt"
openw, unit, filename, /GET
dsp=-1
;dsp = act/adsec.n_act_per_dsp
;addr = dsp_map.sensor_ch0+(act mod adsec.n_act_per_dsp)
addr=dsp_map.sensor_ch0
n_samples = 16000

;count = 5L

window, 0
window, 1
for i=resume_count,count-1 do begin
; monitoring temperature**********************************
    if test_temp then begin
        err = get_slow_diagnostic(gd)
        if err ne adsec_error.ok then begin
            err = disable_coils()
            if err ne adsec_error.ok then spawn, '../adsec_lib/adsec_wrapper/sysdown'
            err = clear_dacs(adsec.act_w_cl)
            print, 'Errore comunicazione!'
            spawn, '../adsec_lib/adsec_wrapper/sysdown'
            save, file=filecount, i
            spawn, adsec_path.home+"/sendmail.py"
            return
        endif
        boolean[0] = total(gd.bcu.stratix_temp gt th_bcu)
        boolean[1] = total(gd.bcu.power_temp gt th_bcu)
        boolean[2] = total(gd.siggen.stratix_temp gt th_sig)
        boolean[3] = total(gd.siggen.power_temp gt th_sig)
        ;boolean[4] = total(gd.siggen.dsps_temp gt th_sig)
        boolean[5] = total(gd.dsp.stratix_temp gt th_dsp)
        boolean[6] = total(gd.dsp.power_temp gt th_dsp)
        boolean[7] = total(gd.dsp.dsps_temp gt th_dsp)
        boolean[8] = total(gd.dsp.driver_temp gt th_dsp)
    
        print, 'Troppo caldi ', total(boolean)


        if total(boolean) gt 0 then begin
            err = disable_coils()
            if err ne adsec_error.ok then spawn, '../adsec_lib/adsec_wrapper/sysdown'
            err = clear_dacs(adsec.act_w_cl)
            spawn, '../adsec_lib/adsec_wrapper/sysdown'
            print, 'Troppo Caldo!'
            save, file=filecount, i
            spawn, adsec_path.home+"/sendmail.py"
            return
        endif
    endif
;end monitoring temperature****************************************************

    print, "ITERATION:", strtrim(i+1,2), " of ", strtrim(count,2)
    data.desired_wave_freq        = test_points[0,i]
    data.desired_ADC_samp_pos     = test_points[2,i]
    data.desired_capsens_samp_pos = test_points[4,i]
    data.desired_ADC_samp_len     = test_points[1,i]
    data.desired_capsens_samp_len = test_points[3,i]
    data.desired_delay_dac        = test_points[5,i]
    
    wset, 0
    err = init_adsec_wave(data, STRUC=comp_data, /VERB)
    if err ne adsec_error.ok then begin
            err = disable_coils()
            if err ne adsec_error.ok then spawn, '../adsec_lib/adsec_wrapper/sysdown'
            err = clear_dacs(adsec.act_w_cl)
            print, "Errore nell'inizializzazione dell'onda!"
            spawn, '../adsec_lib/adsec_wrapper/sysdown'
            save, file=filecount, i
            spawn, adsec_path.home+"/sendmail.py"

            return
        endif    

    err = stop_control(adsec.act_w_cl)
    err = send_wave()
    err = start_control(adsec.act_w_cl)
    if err ne adsec_error.ok then begin
            err = disable_coils()
            if err ne adsec_error.ok then spawn, '../adsec_lib/adsec_wrapper/sysdown'
            err = clear_dacs(adsec.act_w_cl)
            print, 'Errore di scrittura!'
            spawn, '../adsec_lib/adsec_wrapper/sysdown'
            save, file=filecount, i
            spawn, adsec_path.home+"/sendmail.py"
            return   
        endif
    
   
    wait, 0.05
    err= get_dac_ctrl(dsp, cc)
    err= get_dac_ctrl(dsp, cc_not_ready,/not_ready)

    wait, 0.05

    err= get_dac_ctrl(dsp, cc1)
    err= get_dac_ctrl(dsp, cc_not_ready1,/not_ready)
    
    test_flag=0
    if total(cc1 ne cc) ne 0 then begin
         message, "dac not applied", /info 
         test_flag += 1
    endif
    if total(cc_not_ready1 ne cc_not_ready) ne 0 then begin
         message, "dac not properly applied", /info 
         test_flag += 1
    endif    
    if test_flag ne 0 then continue
    
    buf = fltarr(n_samples, n_elements(act))
    ;err = set_io_buffer(dsp, 0, addr, n_samples, /START)
    err = get_ch_data_buf(addr, n_samples, temp_buf, DEC=0)
    if err ne adsec_error.ok then begin
            err = disable_coils()
            if err ne adsec_error.ok then spawn, '../adsec_lib/adsec_wrapper/sysdown'
            err = clear_dacs(adsec.act_w_cl)
            print, 'Errore setting buffer!'
            spawn, '../adsec_lib/adsec_wrapper/sysdown'
            save, file=filecount, i
            spawn, adsec_path.home+"/sendmail.py"
            return
        endif    
    buf=reform(temp_buf)
    ;if err ne adsec_error.ok then message, "ERROR!"
    ;wait, 2*(n_samples*adsec.sampling_time*1.20) > 0.01
    ;err = read_io_buffer(dsp, 0, n_samples, buf)
    ;if err ne adsec_error.ok then begin
     ;       err = disable_coils()
      ;      if err ne adsec_error.ok then spawn, '../adsec_lib/adsec_wrapper/sysdown'
       ;     err = clear_dacs(adsec.act_w_cl)
        ;    print, 'Errore nella lettura!'
         ;   spawn, '../adsec_lib/adsec_wrapper/sysdown'
          ;  save, file=filecount, i
           ; spawn, adsec_path.home+"/sendmail.py"
            ;return
     ; endif   
     ;if err ne adsec_error.ok then message, "ERROR!"
    ave = dblarr(n_act)
    rms = dblarr(n_act)
    ave = total(buf,1)/n_samples
    for aa = 0,n_act-1 do  rms[aa] = stddev(buf[*,aa])
    nc = 6+n_act*2
    format_txt ="("+strtrim(nc,1)+"G30)"
    
    ;print, "AVE [bit]:",ave,"  RMS [bit]:", rms
    wset, 1
    if n_act eq adsec.n_actuators then begin
        ctrl_ave = where(ave[adsec.true_act] eq min(ave[adsec.true_act]),ind_plot)
        act_plot = adsec.true_act[ctrl_ave[0]]
    endif else act_plot=act[0] 
        
    if n_elements(uniq(buf[*,act_plot],sort(buf[*,act_plot]))) eq 1 then plot, buf[*,act_plot], PSYM=3 else plothist, buf[*,act_plot]

    printf, unit, data.desired_wave_freq, data.desired_ADC_samp_len, $
            data.desired_ADC_samp_pos, data.desired_capsens_samp_len, $
            data.desired_capsens_samp_pos, data.desired_delay_dac, ave, rms, FORMAT=format_txt

    flush, unit
    flush, -1
    flush, -2
endfor

free_lun, unit
return
end
