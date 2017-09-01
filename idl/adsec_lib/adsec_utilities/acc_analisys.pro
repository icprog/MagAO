function acc_analisys, acc, FFT_ANAL=fft_anal


fpa = 1d0 ;[Hz] 
accfreq = 3999.887d0
;gain = 3.65d-6
;offset = 32768L
ns = n_elements(acc)
;acc -= lonarr(ns) + offset
;acc *= gain
;pa_num = [0d0,0d0,1d0]
;pa_den = [(2*!dpi*fpa)^2,2*2*!dpi*fpa, 1d0]
;tustin, pa_num, pa_den, accfreq, pa_numd,pa_dend , /dou
;step1 = recursive_df(double(acc[*,0])-mean(double(acc[*,0])),pa_numd,pa_dend)


if keyword_set(fft_anal) then begin
    step0 = (acc - mean(acc))*9.81
    step1 = filter_sequence(step0, 1d/accfreq, [4d, 40d])
    

    tustin, 1d0, [0d, 1d], accfreq, i_numd, i_dend, /dou
    step2 = recursive_df(step1-mean(step1),i_numd,i_dend)
    step3 = recursive_df(step2-mean(step2),i_numd,i_dend)

endif else begin
    print, 'filter'
    blnum = double([1,1,-1,-1])
    blden = double([1,-2.97827,2.95671,-0.978433])
    bhnum = double(0.728178e-5*[1,-1,-1,1])
    bhden = double([1,-2.9421,2.88698,-0.944835])
; bhden=reverse(bhden)

   ; blnum = double([5.607474753155e-005, 1.682242425947e-004, 1.682242425947e-004, 5.607474753155e-005])
   ; blden = double([1.000000000000e+000, -2.842956089969e+000, 2.698002335056e+000, -8.545976471062e-001])
   ; bhnum = double([5.581914158198e-013, 1.116382831640e-012, 5.581914158198e-013])
   ; bhden = double([1.000000000000e+000, -1.996001300368e+000, 9.960092792312e-001])
    
    step2 = recursive_df(double(acc),blnum, blden)
    step3 = recursive_df(double(step2), bhnum, bhden)

endelse

return, step3-mean(step3)
 

End
