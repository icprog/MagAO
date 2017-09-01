;+
;  NAME:
;   GET_OPT_GHOSTS
;
;  USAGE:
;  get_opt_ghosts, act_list, conf, date, nstep,RMS_ACQ=rms_acq, P_RMS=p_rms, NBIN_OTH=nbin_oth
;
;  PURPOSE:
;   N acquisitions of an actuator sensor are sent, 
;   rms and peak_rms(get_opt_ghosts) are calculated.
;   
;
;  INPUT:
;   act_list  : name of actuator
;   conf : number of configuration
;   date : data of configuration
;   nstep: number of acquisitions
;  OUTPUT:
;   Error code.
;
;  KEYWORD:
;   RMS_ACQ=rms_acq : rms for N acquisitions.
;   P_RMS=p_rms  : rms the only peaks of fft_data.
;   NBIN_OTH = nbin_oth : numbers of bins over the fixed threeshold.
;   TEST:     if set when an error occurs the shutdown is applied, it is useful.
;   DECIMATION: decimation factor
;  HISTORY:
;   Created by Daniela Zanotti(DZ) on Dec 2004.
;   
;   28 Dec 2004, A. Riccardi (AR):
;     File containing the spectrum is saved on request (no longer by default,
;     in order to save disk space). Order of input parameters changed.
;   January 2006, DZ
;     Added keyword TEST.
;   Aug 2007, MX and AR
;     FFT normalization fixed. (RMS^2 = total(FFT_BUF^2))
;-
;



Function get_opt_ghosts_v2, act_list, nstep  $
        , RMS_ACQ=rms_acq, P_RMS=p_rms, NBIN_OTH=nbin_oth, SAVE_DATA=save_data, FREQ_THRESHOLD=freq_thr, TEST = test, EXT_NEW=ext, $
        AVE_RMS=rms_ave, NO_DISPLAY=no_display, XADSEC=xadsec, no_lin=no_lin, RMS_TH = rms_th, FBAND = fband, MIN_FBAND = min_fband, DECIMATION=dec

@adsec_common

if (n_elements(dec) eq 0) then dec=0
if n_elements(rms_th) eq 0 then rms_fid = 0. else rms_fid = rms_th/10.

if act_list[0] eq sc.all then act_list=adsec.true_act
nact = n_elements(act_list)
if keyword_set(no_lin) then addr = dsp_map.float_adc_value else addr = dsp_map.distance

if n_elements(freq_thr) eq 0 then freq_thr=1.0
thrsh = 4 ;threeshold for peak in the fft_buf( thrsh*alfa)
if ~keyword_set(fband) then fband = 14000.0/(dec+1)

buflen='8000'xu ; 'FFFF'xu;'8000'xu
buf = fltarr(buflen, n_elements(act_list), nstep)
fft_buf = fltarr(buflen, nact)

fvec = (findgen(buflen/2-1)+1) / ((buflen*adsec.sampling_time)*(dec+1))
rms_tot= dblarr(nact)

if ~keyword_set(min_fband) then min_fband = fvec[0]

;------------------------------------------------
;stop master diagnostic
err = get_master_diagn_freq(ENABLE=enable_fast)
if err ne adsec_error.ok then message, 'Error: ', err
old_ovs = rtr.oversampling_time
if enable_fast ne 0 then begin
    err = set_diagnostic(OVER=0.0)
    if err ne adsec_error.ok then  message, 'Error: ', err
    err = set_diagnostic(MASTER=0.0)
    if err ne adsec_error.ok then  message, 'Error: ', err
endif
wait, 2*old_ovs
;-----------------------------------------------



for i = 0, nstep-1 do begin
    
    err = get_ch_data_buf(addr, buflen, temp_buf, DEC=dec) 
    if err ne adsec_error.ok then return, err

    print, 'Iteration '+ strtrim(string(i+1),2) +" of "+ strtrim(nstep,2) + ' done.'

    buf[*,*,i] = reform(temp_buf[*,0,act_list])

    for j=0, nact-1 do fft_buf[*,j] += abs(fft(buf[*,j,i]))^2
    ;fft_buf += abs(fft(buf[*,*,i], DIM=1))^2

    for ww=0, nact-1 do rms_tot[ww] = stddev(buf[*,ww,i], /double)+rms_tot[ww]
    ;rms_tot[ww] = 
    
    if keyword_set(xadsec) then begin
        err = update_status()
            if err ne adsec_error.ok then begin
                disp_mess, '... error on updating panels.', /APPEND
                return, err
            endif
        update_panels, /no_read,/opt
    endif


endfor

fft_buf /= nstep
fft_buf = sqrt(fft_buf)

fft_buf = sqrt(2) * fft_buf[1:buflen/2-1,*]

rms_vec = dblarr(nact)
p_rms=fltarr(nact)
nbin_oth=fltarr(nact)
rms_ave=rms_tot/nstep
max_peak_val = fltarr(n_elements(act_list))
max_peak_id_freq = fltarr(n_elements(act_list))
max_peak_val_band = fltarr(n_elements(act_list))
max_peak_id_freq_band = fltarr(n_elements(act_list))
id_fb = (where(fvec ge fband))[0]
id_fb_min = (where(fvec ge min_fband))[0]

for ii =0, nact-1 do begin
    
    rms_vec[ii] = stddev(buf[*,ii,*], /double)
    A = rms_vec[ii]^2/(total(fft_buf(*,ii)^2))
    alfa = sqrt(total(fft_buf(*,ii)^2)/n_elements(fft_buf(*,ii))) ;
    idv = where((fft_buf(*,ii) lt thrsh*alfa) or (fvec lt freq_thr), cv)
   ; if idv eq -1 then idv=0
    nbin_oth[ii] = n_elements(fft_buf(*,ii))-cv
    fft_buf_th = sqrt(A)*fft_buf(*,ii) 
    fft_buf_th(idv) = 0  
    p_rms[ii] = sqrt(total(fft_buf_th^2))  

    max_peak_val[ii] = max(fft_buf[*,ii], dummy)
    max_peak_id_freq[ii] = dummy

    max_peak_val_band[ii] = max(fft_buf[id_fb_min:id_fb,ii], dummy_band)
    max_peak_id_freq_band[ii] = dummy_band

    if ~keyword_set(no_display) then begin 
        print, 'RMS peak '+ strtrim(string(act_list[ii]),2) + ': ', p_rms[ii]
        print, 'Number of bin over threeshold act '+ strtrim(string(act_list[ii]),2) + ': ', nbin_oth[ii]
        print, 'RMS act '+ strtrim(string(act_list[ii]),2) + ': ', stddev(buf[*,ii,*], /double)
        print, 'Average RMS act  '+ strtrim(string(act_list[ii]),2) + ': ', rms_ave[ii]
        print, 'Mean act '+ strtrim(string(act_list[ii]),2) + ': ', mean(buf[*,ii,*])
    endif
endfor

dummy_min = min(max_peak_val, id_min)
dummy_max = max(max_peak_val, id_max)
dummy_max_band = max(max_peak_val_band, id_max_band)

window, /FREE, RETAIN=2, xs = 1024, ys=600
win_one = !d.window
window, /FREE, RETAIN=2,  xs = 1024, ys=600
win_two = !d.window
window, /FREE, RETAIN=2,  xs = 1024, ys=600
win_three = !d.window
window, /FREE, RETAIN=2,  xs = 1024, ys=600
win_four = !d.window


wset, win_one
plot, fvec, fft_buf[*,id_min], title = "BEST: ACT#"+strtrim(act_list[id_min],2) + " RMS[nm]:"+string(rms_ave[id_min]*1e9, format = '(f7.2)'), xs=17
oplot, minmax(fvec), [1,1]*rms_fid, COL=255L

wset, win_two
plot, fvec, fft_buf[*,id_max], title = "WORST: ACT#"+strtrim(act_list[id_max],2)+ " RMS[nm]:"+string(rms_ave[id_max]*1e9, format='(f7.2)'), xs=17
oplot, minmax(fvec), [1,1]*rms_fid, COL=255L

wset, win_three
mean_spec = rebin(fft_buf, n_elements(fft_buf[*,0]),1)
plot, fvec, mean_spec, title = "Mean spectrum on act_list", xs=17
oplot, minmax(fvec), [1,1]*rms_fid, COL=255L

wset, win_four
plot, fvec, fft_buf[*,id_max_band], title = "WORST IN BAND ["+strtrim(fix(min_fband),2)+","+strtrim(fix(fband),2)+"]: ACT#"+strtrim(act_list[id_max_band],2)+ " RMS[nm]:"+string(rms_ave[id_max_band]*1e9, format='(f7.2)'), xs=17
oplot, minmax(fvec), [1,1]*rms_fid, COL=255L

print, "MAX RMS: ", strtrim(max(rms_ave, ppp),2) , " on act# ", act_list[ppp], " at freq [Hz]", fvec[where(fft_buf[*,ppp] eq max(fft_buf[*,ppp]))]
print, "MAX PEAK: ", strtrim(dummy_max) , " on act# ", act_list[id_max], " at freq [Hz]", fvec[where(fft_buf[*,id_max] eq max(fft_buf[*,id_max]))]
print, "MAX PEAK IN BAND: ", strtrim(dummy_max_band) , " on act# ", act_list[id_max_band], " at freq [Hz]", fvec[where(fft_buf[*,id_max_band] eq max(fft_buf[*,id_max_band]))]

rms_acq=rms_vec

err = get_status(status_save)
if err ne adsec_error.ok then return, err

res_freq = sqrt(total(status_save.pgain[adsec.act_w_cl])*1e9/adsec_shell.mass)/2./!pi
print, "Resonance freq [Hz]: ", res_freq

if keyword_set(save_data) then begin
   save, buf, fvec, fft_buf, act_list, nstep, addr, status_save,freq_thr,thrsh,   $
        file=meas_path('noise','noise_spectrum.sav')
endif

;err = dspstorage()
;if err ne adsec_error.ok then return, err

if enable_fast ne 0 then begin
    ;err = rip(/XADSEC)
     ;   if err ne adsec_error.ok then  message, 'Error: ', err
    err = set_diagnostic(/master)
        if err ne adsec_error.ok then  message, 'Error: ', err
    err = set_diagnostic(over=old_ovs)
        if err ne adsec_error.ok then  message, 'Error: ', err
endif




return, adsec_error.ok

end
