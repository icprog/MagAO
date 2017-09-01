;$Id: data_cut.pro,v 1.3 2009/05/22 16:38:33 labot Exp $
;+
;   NAME:
;    DATA_CUT
;
;   PURPOSE:
;    Cut a data set by using a smooth derivative filter.
;
;   USAGE:
;    output = data_cut(data, MASK=mask)
;
;   INPUT:
;    data:      2D or 3D data array to cut. The last dimension is temporal sampling.
;
;   OUTPUT:
;    output:    fitted parameters
;
;   KEYWORD:
;    MASK:      2D byte array with 1 on pixels to analize
;
; HISTORY
;   Oct 2006:   written by M.Xompero (MX)
;               marco@arcetri.astro.it
;-



Function data_cut, phase, MASK=mask, TURB_RMS=turb_rms
    
;    tt = reform(phase[*,0,0] - mean( phase[*,0,0]))
;    for i=0,499 do aa2[i] = total(tt[0:i])
    cut = 31


 phi = dblarr(500, 672)
for i=0, 671 do phi[*,i] = reform(atan((phase[*,i,3]-phase[*,i,1]),(phase[*,i,2]-phase[*,i,0])))


    for i=0, adsec.n_actuators-1 do begin

        data = double(reform(phase[*,i,0] - mean( phase[*,i,0])))

        dummy = smooth(deriv(data),5)
        dummy = dummy[cutd:n_elements(dummy)-1-cutd]

        thr = 0.5
        id = where(abs(dummy) gt thr*max(abs(dummy)), cc)
        id_min[i] = id[0]+cutd
        id_max[i] = id[cc-1]+cutd
    
    endfor






























    if n_elements(mask) ne 0 then begin
        idx = where(mask)
        sdata_old = size(data_old, /DIM)
        data_in = reform(temporary(data_old),long(sdata_old[0])*sdata_old[1],sdata_old[2])
        sdata = size(data_in, /DIM)
    end else begin
        data_in = temporary(data_old)
        sdata = size(data_in, /DIM)
        idx = indgen(sdata[1])
    end

    if n_elements(idx) gt 32767 then begin
        idx_id = round(mk_vector(32767,0,n_elements(idx)-1))    
        idx = idx[idx_id]
    endif

    id_min = lonarr(sdata[1] <32767)
    id_max = lonarr(sdata[1] <32767)
    cutd=4

    for i=0, n_elements(idx)-1 do begin


        data = double(data_in[*,idx[i]]) - double(mean(data_in[*,idx[i]]))

        dummy = smooth(deriv(data),5)
        dummy = dummy[cutd:n_elements(dummy)-1-cutd]

        thr = 0.5
        id = where(abs(dummy) gt thr*max(abs(dummy)), cc)
        id_min[i] = id[0]+cutd
        id_max[i] = id[cc-1]+cutd
    
    endfor

    if n_elements(mask) gt 0 then data_old = reform(data_in, sdata_old[0], sdata_old[1], sdata_old[2]) $
                             else data_old = data_in

    tmp = max(histogram(id_min, LOC=hlmin), h1)
    tmp = max(histogram(id_max, LOC=hlmax), h2)

    final_min = hlmin[h1]
    final_max = hlmax[h2]

    turb_rms = dblarr(n_elements(idx))
    for i=0, n_elements(idx)-1 do turb_rms[i] = stddev(data_old[i:final_min,i])/2+$
                                                stddev(data_old[final_max:*,i])/2

    return, [final_min, final_max]
End
