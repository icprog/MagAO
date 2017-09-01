;$ID$$;
; pro to reorder the configuration file
 
pro gen_act2mirr_act, array2write, INPUT_FILE = INPUT_FILE ,  OUTPUT_FILE = OUTPUT_FILE, $
    ONLY_ORDER = only_order

@adsec_common
; read configuration file
    if keyword_set(input_file) then file2conv = input_file else $ 
         file2conv = dialog_pickfile(filter='*.txt',title='Pick Configuration file ')
 
    gen_array = read_ascii_array(file2conv)

; read_order
    file_serial = adsec_path.conf+'temp/serialNumber.txt'
    serial_array = read_ascii_array(file_serial)
    ;file_act = adsec_path.conf+''
    act2order =  [[serial_array],[gen_array]]
    
    file_ordered =  adsec_path.conf+'dsp_conf.txt'
    order_act_mat =  read_ascii_array(file_ordered) 
    order_act = order_act_mat[5,*]
    
    n_o = n_elements(order_act)
    n_g = n_elements(gen_array)
    array2write = fltarr(n_o)
    
    for i=0,n_o-1 do begin
        cc = where(serial_array eq order_act[i])
       ;TO  DEBUG the number of some coils are missing so if the serial numberis over 2000 the last value of the gen_array will be given [?16,?0,?590]:
       if (order_act[i] ge 2000) or (order_act[i] eq 16) or (order_act[i] eq 0) or (order_act[i] eq 590) $
       or (order_act[i] eq 25) or (order_act[i] eq 247) or (order_act[i] eq 582) or (order_act[i] eq 1500) $
       then cc = n_g-1 
      ;TO DEBUG  
        array2write[i] = gen_array[cc]
    endfor

;    array2write = gen_array[order] 

; write new
    if ~keyword_set(only_order) then begin 
        if keyword_set(output_file) then file_path = adsec_path.conf+output_file else $
        file_path = dialog_pickfile(title='Configuration file ') 
    
        write_array_ascii, array2write , FILENAME=file_path
    
        print, 'The array was written in ', file_path
    endif
end
