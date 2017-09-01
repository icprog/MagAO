function filter_m2c, m2c, j_vec, max_abs, THRESHOLD_RATIO=thr
@adsec_common
if n_elements(thr) eq 0 then thr=1e-3
m2c_filt = m2c*0.0
j_vec = lonarr(adsec.n_actuators)
max_abs = fltarr(adsec.n_actuators)
mm = transpose(adsec.ff_p_svec)##m2c
idx = where(total(m2c^2,2) ne 0, count)
for i=0,count-1 do begin
    j1=i
    j2=adsec.n_actuators-1
    v1 = adsec.ff_p_svec[j1:*,*]##mm[idx[i],j1:*]
    v2 = adsec.ff_p_svec[j2:*,*]##mm[idx[i],j2:*]
    tv0 = total(m2c[idx[i],*]^2)
    tv1 = total(v1^2)
    tv2 = total(v2^2)
    if sqrt(tv1/tv0) gt thr then begin
        repeat begin
            jm = (j1+j2)/2
            vm = adsec.ff_p_svec[jm:*,*]##mm[idx[i],jm:*]
            tvm = total(vm^2)
            if sqrt(tvm/tv0) lt thr then begin
                v2 = vm
                j2 = jm
                tv2 = tvm
            endif else begin
                v1 = vm
                j1 = jm
                tv1 = tvm
            endelse
        endrep until j2-j1 le 1
    endif
    print, i, tv0, j1
    m2c_filt[idx[i],*]=adsec.ff_p_svec[0:j1,*]##mm[idx[i],0:j1]
    j_vec[idx[i]]=j1
    max_abs[idx[i]]=max(abs(m2c[idx[i],*]-m2c_filt[idx[i],*]))
endfor
return, m2c_filt
end
        
