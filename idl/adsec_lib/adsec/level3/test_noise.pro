;+
; HISTORY:
;   11 july 2006 
;       Daniela Zanotti(DZ)
;       changed the obsolate command stdev with stddev.
;   07 Feb 2007 MX
;       LUT of variable deleted and name refurbished.
;-
 
function test_noise, times, max_min, sigma, coeff, covar, skewness, buf $
         , WAIT_TIME=dt, SAMPLES=samples, NO_LINEARIZED=no_lin

@adsec_common

if n_elements(dt) eq 0 then dt=0.5
if n_elements(samples) eq 0 then nsamp = dsp_const.max_data_IO_len else nsamp=samples
;stop master diagnostic
err = get_master_diagn_freq(ENABLE=enable_fast)
if err ne adsec_error.ok then return, err
old_ovs = rtr.oversampling_time
if enable_fast ne 0 then begin
    err = set_diagnostic(OVER=0.0)
    if err ne adsec_error.ok then return, err
    err = set_diagnostic(MASTER=0.0)
    if err ne adsec_error.ok then return, err
endif
wait, 2*old_ovs

sigma = dblarr(adsec.n_actuators, times)
max_min = dblarr(adsec.n_actuators, times)
covar = dblarr(adsec.n_actuators, adsec.n_actuators, times)
skewness = dblarr(adsec.n_actuators, times)

if keyword_set(no_lin) then $
    addr = [dsp_map.float_adc_value] $
else $
    addr = [dsp_map.distance]

for i=0,times-1 do begin
    print, strtrim(i+1,2)+"/"+strtrim(times,2)

    err = get_ch_data_buf(addr, nsamp, buf)
    if err ne adsec_error.ok then return, err
    
    buf = reform(temporary(buf))
    wait, dt
    for act=0,adsec.n_actuators-1 do begin
        sigma[act, i] = (stddev(buf[*,act],/double))
        skewness[act, i] = ((moment(buf[*,act], /double))[2])
        maxv = max(buf[*,act], MIN=minv)
        max_min[act, i] = maxv-minv
    endfor
    mean = reform(total(buf, 1)/nsamp)
    covar[*,*,i] = correlate(transpose(buf), /COVAR, /DOUBLE)
endfor

if enable_fast ne 0 then begin
    err = set_diagnostic(/master)
        if err ne adsec_error.ok then return, err
    err = set_diagnostic(over=old_ovs)
        if err ne adsec_error.ok then return, err
endif

return, adsec_error.ok
end


