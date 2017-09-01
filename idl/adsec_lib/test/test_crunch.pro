;$Id: test_crunch.pro,v 1.2 2008/02/05 09:51:01 labot Exp $
;Pro test_crunch, bufr
;print, set_diagnostic(over =0, master=0)
;err = write_same_dsp(0, rtr_map.switch_SCStartRTR, [0l,0l], /SWITCH, /CHECK)
;if err ne adsec_error.ok then return, err

;    @adsec_common
    dir = meas_path('crunch')
    len = 32768u
    ;file0 = dir+'float_ADC_value_11_12_07_a'
    file0 = dir+'distance_h'
;    file1 = 'command_pending_c'
;    file1 = '8FFF0_f'
    ;file1 = dir+'dac_not_sent_11_12_07_a'
    file1 = dir+'float_dac_value_h'
    ;file2 = dir+'dac_not_ready_11_12_07_a'
    ;file2 = dir+'pos_command_11_12_07_a'
;    file2 = '180358_f'
;    addr = [dsp_map.float_ADC_value, dsp_map.command_pending, '180358'xul]
    ;addr = [dsp_map.float_ADC_value, dsp_map.dac_not_sent, dsp_map.dac_not_ready]
    addr = [dsp_map.distance, dsp_map.float_dac_value];, dsp_map.pos_command]
    err = write_same_dsp(sc.all,rtr_map.test_address, 0ul)
    err = dspstorage(/DIS)
    wait, 1
    trig_addr = rtr_map.test_address
    trig_val = 1l
    ;print, set_io_buffer(sc.all, [0], addr[0], len, trig_addr, trig_val, RECORD_SIZE=4, /TRIG, /START, /NO_CH)
    ;print, set_io_buffer(sc.all, [1], addr[1], len, trig_addr, trig_val, /TRIG, /START, /NO_CH)
    ;print, set_io_buffer(sc.all, [2], addr[2], len, trig_addr, trig_val, /TRIG, /START, /NO_CH)
    err = write_same_dsp(sc.all,rtr_map.test_address, 1ul)
    print, set_io_buffer(sc.all, [0, 1], addr, len, RECORD_SIZE=4, /START, /NO_CH)
    wait, 1
    err = dspstorage()
    
    ;err = read_io_buffer_ch(sc.all, [0], len, buf0, /UL)
    ;err = read_io_buffer(sc.all, [1], len, buf1, /UL)
    ;err = read_io_buffer(sc.all, [2], len, buf2, /UL)
    err = read_io_buffer_ch(sc.all, [0,1], len, bufr)
    
    data1 = reform(bufr[*,0,*])
    data2 = reform(bufr[*,1,*])
   ; data3 = reform(bufr[*,2,*])
    ;data1 = reform(buf0)
    ;data2 = reform(buf1)
    ;data3 = reform(buf2)
    openw, unit, file0+'.dat', /GET_LUN
    printf, unit, ulong(reform(data1, len*672ul))
    free_lun, unit
    openw, unit, file1+'.dat', /GET_LUN
    printf, unit, ulong(reform(data2, len*672ul))
    ;printf, unit, ulong(reform(data2, len*168ul))
    ;free_lun, unit
    ;openw, unit, file2+'.dat', /GET_LUN
    ;printf, unit, ulong(reform(data3, len*672ul))
    ;printf, unit, ulong(reform(data3, len*168ul))
    free_lun, unit
    spawn, 'gzip -c '+file0+'.dat > '+file0+'.zip'
    spawn, 'gzip -c '+file1+'.dat > '+file1+'.zip'
    ;spawn, 'gzip -c '+file2+'.dat > '+file2+'.zip'
    ;return
    save, file=file0+'.sav', data1
    save, file=file1+'.sav', data2

End
