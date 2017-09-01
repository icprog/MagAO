;$Id: optimize_wave.pro,v 1.4 2007/02/07 18:06:08 marco Exp $$
;+
;test to search the best parameters configuration for the signal wave.
; HISTORY
;   2005 Created by D.Zanotti.
;   Jan 2006 
;       Changed the main routine into a pro routine.
;   07 Feb 2007 MX
;       LUT of variable deleted and name refurbished.
;-   
 
pro optimize_wave, act, RESUME_COUNT=resume_count, TEST_TEMP = test_temp, VERB=VERB, FILENAME=filename,DATA=data

@adsec_common
sc.debug=0
path = adsec_path.meas+'/wave/2006_01_11/'
if n_elements(act) eq 0 then act = 11
;verb = 0B
if not keyword_set(resume_count)then resume_count = 0L
if not  keyword_set(test_temp) then test_temp=0 ; if set 1 enable the test on the board temperatures

ext='_a'

data = read_ascii_structure(filepath(ROOT=adsec_path.conf,"wave.txt"))

t_clock = 1d0/adsec.clock_freq

;f_min = 70d3
;f_max = 71d3
;f_step= (f_max-f_min)/6
;wave_ADC_freq_ratio = 1
f_min = 140d3;143500d0;134d3;130d3
f_max = 150d3;145d3;153d3;150d3
f_step= (f_max-f_min)/16
wave_ADC_freq_ratio = 2

; ADC_len_min = adsec.min_ADC_samp_len
; ADC_len_max = adsec.max_ADC_samp_len
; ADC_len_min = 0.5d-6
; ADC_len_max = 0.68d-6
ADC_len_min = 0.56d-6
ADC_len_max = 0.56d-6
; ADC_len_step = (ADC_len_max-ADC_len_min)/3
ADC_len_step = 1d-6

max_dt = 1/f_min/2
max_flat_dt = max_dt-adsec.wave_settling
ADC_pos_step = max_flat_dt/8
ADC_pos_min =0d0; (1/f_max/2)-adsec.wave_settling ;0d0
ADC_pos_max = 10.0

; ADC_pos_min = 0.7d-6
; ADC_pos_max = 0.9d-6
; ADC_pos_step = .05d-6

 capsens_len_min = 1d-6
 ; capsens_len_min = ceil(capsens_len_min/t_clock)*t_clock
 capsens_len_max = 10d0
 capsens_len_step = (max_flat_dt-capsens_len_min)/8
; capsens_len_step = (max_flat_dt-capsens_len_min)/1 ;????????????????????????????????????????????????????????????????
; capsens_len_min = 2.d-6
; capsens_len_max = 4.d-6
; capsens_len_min = 2.6d-6
; capsens_len_max = 2.6d-6
; capsens_len_step = .3d-6

capsens_pos_step = max_flat_dt/10
capsens_pos_min = 0d0
capsens_pos_max = 10d0
; capsens_pos_min = 5.2d-6
; capsens_pos_max = 5.6d-6
; capsens_pos_step = .07d-6

DAC_delay_min=2.6d-6;
DAC_delay_max=13.0d-6;
DAC_delay_step =(DAC_delay_max-DAC_delay_min)/10
;DAC_delay_step =(DAC_delay_max-DAC_delay_min);?????????????????????????????????????????????????????????

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

;    DAC_delay_min =adsec.wc_comp_time+2*t_clock
;    DAC_delay_max =comp_data.sampling_time-adsec.dsp2dac_transfer_time-2*t_clock
;    DAC_delay_step =(DAC_delay_max-DAC_delay_min)/3

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



filename = path+"optimize_wave_"+string(act,FORMAT="(I3.3)")+"_resume_"+strtrim(resume_count,2)+ext+".txt"
openw, unit, filename, /GET

dsp = act/adsec.n_act_per_dsp
addr = dsp_map.float_adc_value+(act mod adsec.n_act_per_dsp)
n_samples = 16000

count = 5L

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
            free_lun, port
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

            free_lun, port
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
            free_lun, port
            return
        endif    


    err = send_wave()
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
    if cc1 ne cc then begin
         message, "dac not applied", /info 
         test_flag += 1
    endif
    if cc_not_ready1 ne cc_not_ready then begin
         message, "dac not properly applied", /info 
         test_flag += 1
    endif    
    if test_flag ne 0 then continue

    err = set_io_buffer(dsp, 0, addr, n_samples, /START)
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
    ;if err ne adsec_error.ok then message, "ERROR!"
    wait, 2*(n_samples*adsec.sampling_time*1.20) > 0.01
    err = read_io_buffer(dsp, 0, n_samples, buf)
    if err ne adsec_error.ok then begin
            err = disable_coils()
            if err ne adsec_error.ok then spawn, '../adsec_lib/adsec_wrapper/sysdown'
            err = clear_dacs(adsec.act_w_cl)
            print, 'Errore nella lettura!'
            spawn, '../adsec_lib/adsec_wrapper/sysdown'
            save, file=filecount, i
            spawn, adsec_path.home+"/sendmail.py"
            free_lun, port
            return
      endif   
     ;if err ne adsec_error.ok then message, "ERROR!"
    
    ave = mean(buf)
    rms = stdev(buf)
    
    print, "AVE [bit]:",ave,"  RMS [bit]:", rms
    wset, 1
    if n_elements(uniq(buf,sort(buf))) eq 1 then plot, buf, PSYM=3 else plothist, buf

    printf, unit, data.desired_wave_freq, data.desired_ADC_samp_len, $
            data.desired_ADC_samp_pos, data.desired_capsens_samp_len, $
            data.desired_capsens_samp_pos, data.desired_delay_dac, ave, rms, FORMAT="(8G30)"

    flush, unit
    flush, -1
    flush, -2
endfor

free_lun, unit
return
end
