;path = "/home/labot/Desktop/noise/2008_02_04/"
;files = strarr(6);file_search(path+'*.sav')
;files[0] = "/home/labot/Desktop/noise/2008_02_04/noise_lin_p0.06_d0.3_p0.01_d0.2_ext_00deg_80um_.sav"
;files[1] = "/home/labot/Desktop/noise/2008_02_04/noise_lin_p0.13_d0.3_p0.06_d0.25_ext_00deg_.sav"
;files[2] = "/home/labot/Desktop/noise/2008_02_04/noise_nolin_p0.06_d0.3_p0.01_d0.2_ext_00deg_80um_.sav"
;files[3] = "/home/labot/Desktop/noise/2008_02_04/noise_nolin_p0.13_d0.3_p0.06_d0.25_ext_00deg_.sav"
;files[4] = "/home/labot/Desktop/noise/2008_02_04/noise_spectrum_00deg_80um.sav"
;files[5] = "/home/labot/Desktop/noise/2008_02_04/noise_spectrum_00deg.sav"

path = "/media/disk/adsec672a/meas/noise/2008_01_23/"
files = strarr(6);file_search(path+'*.sav')
files[0] = path+"noise_lin_60deg_p0.09_d0.3.sav"
files[1] = "";/home/labot/Desktop/noise/2008_02_04/noise_lin_85deg_p0.09_d0.3.sav"
files[2] = "" ;/home/labot/Desktop/noise/2008_02_04/noise_nolin_p0.06_d0.3_p0.01_d0.2_ext_00deg_80um_.sav"
files[3] = "" ;/home/labot/Desktop/noise/2008_02_04/noise_nolin_p0.13_d0.3_p0.06_d0.25_ext_00deg_.sav"
files[4] = path+"noise_spectrum60deg_p0.09_d0.3.sav"
files[5] = "";/home/labot/Desktop/noise/2008_02_04/noise_spectrum85deg_p0.09_d0.3.sav"



do_jpeg = 1
loadct, 12
xs = 1024
ys = 768
set_plot_defaults, /WHITE

;Posizione e corrente a 60um
strgap = ' @60um mean gap'
strdeg = '30deg_'
strgap = ' @30deg az angle'

if files[1] ne "" then begin
    restore, files[1], /ver
    col = comp_colors(6)
    col_a = rebin(col, 672, /SAMP)
    window, /FREE, XS=xs, YS=ys
    plot, status_save.sys_status.position[adsec_save.act_w_cl]*1e6,sigma_pos_reb[adsec_save.act_w_cl]*1e9            $
        , /NODATA, TITLE='Position oise RMS VS Gap' + strgap, XTIT='[!4l!3m]', YTIT='[nm]'  , YR=[0,8], YTICKF='(f5.1)', XTICKF='(f5.1)'
    for i=0, n_elements(adsec_save.act_w_cl)-1 do $
        plots,  status_save.sys_status.position[adsec_save.act_w_cl[i]]*1e6                                          $
             ,  sigma_pos_reb[adsec_save.act_w_cl[i]]*1e9, col=col_a[adsec_save.act_w_cl[i]], psym=4
    if do_jpeg then write_jpeg, path+strdeg+'plot_NoiseRMSVSPosition_60um_zenith.jpg', tvrd(true=3), true=3, quality=100


    window, /FREE, XS=xs, YS=ys
    display, status_save.sys_status.position[adsec_save.act_w_cl]*1e6, adsec_save.act_w_cl, OUT_VAL='FFFFFF'xl, /no_n, /sh  $
           , TITLE='Mirror Shape'+strgap, BAR_TITLE='[!4l!3m]', YTICKF_BAR='(f5.2)', XTIT='[mm]', YTIT='[mm]'   
    if do_jpeg then write_jpeg, path+strdeg+'display_mirror_shape_60um_zenith.jpg', tvrd(true=3), true=3, quality=100


    window, /FREE, XS=xs, YS=ys
    display, status_save.sys_status.current[adsec_save.act_w_cl], adsec_save.act_w_cl, /no_n, /sh, OUT_VAL='FFFFFF'xl $
           , TITLE='Mirror Forces'+strgap, BAR_TITLE='[N]', YTICKF_BAR='(f6.3)', XTIT='[mm]', YTIT='[mm]' 
    if do_jpeg then write_jpeg, path+strdeg+'display_mirror_forces_60um_zenith.jpg', tvrd(true=3), true=3, quality=100


    ;Noise a 60 um
    window, /FREE, XS=xs, YS=ys
    display, sigma_pos_reb[adsec_save.act_w_cl]*1e9, adsec_save.act_w_cl, /no_n, /sh , OUT_VAL='FFFFFF'xl  $
           , TITLE='Noise RMS'+strgap, BAR_TITLE='[nm]', YTICKF_BAR='(f5.2)', XTIT='[mm]', YTIT='[mm]' 
    if do_jpeg then write_jpeg, path+strdeg+'display_mirror_rms_lin_60um_zenith.jpg', tvrd(true=3), true=3, quality=100

endif

if files[3] ne "" then begin

    restore, files[3]
    window, /FREE, XS=xs, YS=ys
    plot, status_save.sys_status.voltage[adsec_save.act_w_cl],sigma_pos_reb[adsec_save.act_w_cl], /NODATA          $
        , XTICKF='(I5)', TITLE='Voltage Noise RMS VS Capsens Voltage' + strgap, XTIT='[bit]', YTIT='[bit]'  , YR=[0,2.5], YTICKF='(f5.1)'
    for i=0, n_elements(adsec_save.act_w_cl)-1 do $
        plots,  status_save.sys_status.voltage[adsec_save.act_w_cl[i]]                                          $
             ,  sigma_pos_reb[adsec_save.act_w_cl[i]], col=col_a[adsec_save.act_w_cl[i]], psym=4
    if do_jpeg then write_jpeg, path+'plot_NoiseRMSVSVoltage_60um_zenith.jpg', tvrd(true=3), true=3, quality=100

    window, /FREE, XS=xs, YS=ys
    display, sigma_pos_reb[adsec_save.act_w_cl], adsec_save.act_w_cl, /no_n, /sh , OUT_VAL='FFFFFF'xl  $
           , TITLE='Noise RMS @60[!4l!3m] mean gap', BAR_TITLE='[bit]', YTICKF_BAR='(f5.2)', XTIT='[mm]', YTIT='[mm]' 
    if do_jpeg then write_jpeg, path+strdeg+'display_mirror_rms_nolin_60um_zenith.jpg', tvrd(true=3), true=3, quality=100

endif


if files[5] ne "" then begin

    restore, files[5]
    ;fft_buf_mean = rebin(fft_buf, max(size(fft_buf, /DIM)))
    act_w_cl = adsec_save.act_w_cl
    dim0 = max(size(fft_buf, /DIM))
    dim1 = max(size(buf, /DIM))
    fft_buf_mean = sqrt(rebin(fft_buf^2, dim0))
    v0 = total(fft_buf^2, 1, /double)
    v1 = v0*0
    for i=0, 662 do v1[i] = variance(buf[*,i])   ;il metodo rapido non e' preciso
    AA = v1/v0
    BB = total(v1) / total(v0)
    print, 'Error on relative normalization in variance between data and spectrum' + strtrim(bb,2)

    window, /FREE, XS=xs, YS=ys
    bin = fvec[1]-fvec[0]
    plot, fvec, fft_buf_mean*1e9 $
           , TITLE='Average position noise Spectrum'+strgap +' (BIN='+string(bin, format='(f5.2)')+'[Hz])', YTITLE='[nm]/sqrt(bin)', XTITLE='[Hz]', YTICKFORMAT='(f5.3)'
    if do_jpeg then write_jpeg, path+strdeg+'plot_mean_spectrum_60um_zenith.jpg', tvrd(true=3), true=3, quality=100
endif

;-----------------------------------------------------------------------------------------------------------
;Posizione e corrente a 80um
strgap = ' @80um mean gap'
strgap = ' 30deg az angle'
if files[0] ne "" then begin

    restore, files[0], /ver
    window, /FREE, XS=xs, YS=ys
    plot, status_save.sys_status.position[adsec_save.act_w_cl]*1e6,sigma_pos_reb[adsec_save.act_w_cl]*1e9            $
        , /NODATA, TITLE='Position Noise RMS VS Gap' + strgap, XTIT='[!4l!3m]', YTIT='[nm]'  , YR=[0,8], YTICKF='(f5.1)', XTICKF='(f5.1)'
    for i=0, n_elements(adsec_save.act_w_cl)-1 do $
        plots,  status_save.sys_status.position[adsec_save.act_w_cl[i]]*1e6                                          $
             ,  sigma_pos_reb[adsec_save.act_w_cl[i]]*1e9, col=col_a[adsec_save.act_w_cl[i]], psym=4
    if do_jpeg then write_jpeg, path+strdeg+'plot_NoiseRMSVSPosition_80um_zenith.jpg', tvrd(true=3), true=3, quality=100


    window, /FREE, XS=xs, YS=ys
    display, status_save.sys_status.position[adsec_save.act_w_cl]*1e6, adsec_save.act_w_cl, /no_n, /sh  , OUT_VAL='FFFFFF'xl  $
           , TITLE='Mirror Shape'+strgap, BAR_TITLE='[!4l!3m]', YTICKF_BAR='(f5.2)' , XTIT='[mm]', YTIT='[mm]'   
    if do_jpeg then write_jpeg, path+strdeg+'display_mirror_shape_80um_zenith.jpg', tvrd(true=3), true=3, quality=100


    window, /FREE, XS=xs, YS=ys
    display, status_save.sys_status.current[adsec_save.act_w_cl], adsec_save.act_w_cl, /no_n, /sh  , OUT_VAL='FFFFFF'xl  $
           , TITLE='Mirror Force'+strgap, BAR_TITLE='[N]', YTICKF_BAR='(f6.3)' , XTIT='[mm]', YTIT='[mm]' 
    if do_jpeg then write_jpeg, path+strdeg+'display_mirror_forces_80um_zenith.jpg', tvrd(true=3), true=3, quality=100


    ;Noise a 80 um
    window, /FREE, XS=xs, YS=ys
    display, sigma_pos_reb[adsec_save.act_w_cl]*1e9, adsec_save.act_w_cl, /no_n, /sh , OUT_VAL='FFFFFF'xl  $
           , TITLE='Noise RM'+strgap, BAR_TITLE='[nm]', YTICKF_BAR='(f5.2)', XTIT='[mm]', YTIT='[mm]' 
    if do_jpeg then write_jpeg, path+strdeg+'display_mirror_rms_lin_80um_zenith.jpg', tvrd(true=3), true=3, quality=100

endif


if files[2] ne "" then begin

    restore, files[2]
    window, /FREE, XS=xs, YS=ys
    plot, status_save.sys_status.voltage[adsec_save.act_w_cl],sigma_pos_reb[adsec_save.act_w_cl], /NODATA          $
        , XTICKF='(I5)', TITLE='Voltage Noise RMS VS Capsens Voltage' + strgap, XTIT='[bit]', YTIT='[bit]'  , YR=[0,2.5], YTICKF='(f5.1)'
    for i=0, n_elements(adsec_save.act_w_cl)-1 do $
        plots,  status_save.sys_status.voltage[adsec_save.act_w_cl[i]]                                          $
             ,  sigma_pos_reb[adsec_save.act_w_cl[i]], col=col_a[adsec_save.act_w_cl[i]], psym=4
    if do_jpeg then write_jpeg, path+strdeg+'plot_NoiseRMSVSVoltage_80um_zenith.jpg', tvrd(true=3), true=3, quality=100

    window, /FREE, XS=xs, YS=ys
    display, sigma_pos_reb[adsec_save.act_w_cl], adsec_save.act_w_cl, /no_n, /sh , OUT_VAL='FFFFFF'xl  $
           , TITLE='Noise RMS @80[!4l!3m] mean gap', BAR_TITLE='[bit]', YTICKF_BAR='(f5.2)', XTIT='[mm]', YTIT='[mm]' 
    if do_jpeg then write_jpeg, path+strdeg+'display_mirror_rms_nolin_80um_zenith.jpg', tvrd(true=3), true=3, quality=100

endif


if files[4] ne "" then begin

    restore, files[4]
    act_w_cl = adsec_save.act_w_cl
    dim0 = max(size(fft_buf, /DIM))
    dim1 = max(size(buf, /DIM))
    fft_buf_mean = sqrt(rebin(fft_buf^2, dim0))
    v0 = total(fft_buf^2, 1, /double)
    v1 = v0*0
    for i=0, 662 do v1[i] = variance(buf[*,i])   ;il metodo rapido non e' preciso
    AA = v1/v0
    BB = total(v1) / total(v0)
    print, 'Error on relative normalization in variance between data and spectrum' + strtrim(bb,2)

    window, /FREE, XS=xs, YS=ys
    bin = fvec[1] - fvec[0]
    plot, fvec, fft_buf_mean*1e9 $
           , TITLE='Average position noise Spectrum'+strgap+' (BIN='+string(bin, format='(F5.2)')+'[Hz])', YTITLE='[nm/sqrt(bin)]', XTITLE='[Hz]', YTICKFORMAT='(f5.3)'
    if do_jpeg then write_jpeg, path+strdeg+'plot_mean_spectrum_80um_zenith.jpg', tvrd(true=3), true=3, quality=100

endif



end




