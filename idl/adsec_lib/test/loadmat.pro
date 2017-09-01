;Load and return a matrix from the adaptive secondary.
;
; dim0: x size
; dim1: y size
; DELAY: if set, load a matrix as the delay line of the secondary mirror.

Function loadmat, addr, dim0,dim1, DELAY=delay
    
    @adsec_common

    if keyword_set(DELAY) then begin

        err = read_seq_dsp(sc.all, addr, delay*long(dim0)*adsec.n_act_per_dsp, buf)
        if err ne adsec_error.ok then return, err
        buf = reform(buf,adsec.n_act_per_dsp,dim0, delay, adsec.n_dsp)
        buf = transpose(temporary(buf), [1,0,3,2])
        buf = reverse(reform(temporary(buf), dim0,dim1, delay))

    endif else begin

        err = read_seq_dsp(sc.all, addr, long(dim0)*adsec.n_act_per_dsp, buf)
        if err ne adsec_error.ok then return, err
        buf = reform(temporary(buf),adsec.n_act_per_dsp,dim0,adsec.n_dsp)
        buf = transpose(buf,[1,0,2])
        buf = reform(buf,dim0,dim1)

    endelse

    return, buf

End
