function accel_spec, acc, np, ITER=iter, ALL=all, GET=get

        strbase=3
        dt=1/4000.
        orient=['Hor','Null','Vert','Hor','Ver','Hor', 'Ver', 'Null']
        work_acc=[0,2,3,4,5,6]
        err=load_diag_data_acc(np, red, dat,/readfc)
        fft1, red.(strbase+acc), dt, fspec=freq, psd=psd, /noplo
        if n_elements(iter)eq 0 then iter=0
        for i=0, iter do begin
            plot, psym=10, freq, psd, xr=[0,100], title='Accelerometer #'+strtrim(acc,2)+', '+orient[acc], xtitle='Freq (Hz)'
            wait, 0.05
        endfor
        
        if keyword_set(get) then begin
            h=red.acc_0+red.acc_2+red.acc_6
            v=red.acc_1+red.acc_3+red.acc_5
            fft1, h, dt, fspec=freqh, psd=psdh, /noplo
            fft1, v, dt, fspec=freqv, psd=psdv, /noplo
            acc=[freqh, psdh, psdv]

        endif
                


        if keyword_set(all) then begin
        wsize=[600,400]
        
        for i=0, n_elements(work_acc)-1 do window, i, xsize=wsize[0], ysize=wsize[1];, xpos=i*wsize[0], ypos=i*wsize[1]
        for i=0, n_elements(work_acc)-1 do begin
                wset, i
                fft1, red.(strbase+work_acc[i]), dt, fspec=freq, psd=psd, /noplo
                plot, psym=10, freq, psd, xr=[0,100], title='Accelerometer #'+strtrim(work_acc[i],2)+', '+orient[work_acc[i]], xtitle='Freq (Hz)'
        endfor
        endif
   return, 0       
end
