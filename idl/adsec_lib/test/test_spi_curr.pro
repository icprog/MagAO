Pro test_spi_curr, name, nsamples=nsample
 
    @adsec_common
    set_plot, 'x'


   ;disable distance checks 
   ;err = set_dist_threshold(sc.all, 4.)

   ;enable coils
  ; err = adam_enable_coils() 
   ;write bias current 
  ; err = write_bias_curr(sc.all, 0.03)
    if n_elements(nsample) eq 0 then nsample = 60.
    spi = fltarr(nsample, adsec.n_actuators)
    for i = 0, nsample-1 do begin
        err= getdiagnvalue(process_list.housekeeper.msgd_name, 'dspcoil*', -1, -1, tim=1, tmp) 
        spi[i, *] = tmp.last
        wait, 1.
        log_print, "sample"+string( i, format='(I3.3)')
    endfor
    spi_m = (-1) * rebin(spi, 1, adsec.n_actuators)
    spi_sd = spi_m*0
    for i = 0, adsec.n_actuators-1 do spi_sd[i] = stddev(spi[*,i])
    if n_elements(name) gt 0 then begin
	    err = get_status(status_save)
	    file_path = meas_path('spi')
	    filename = file_path+name
	    save, file = filename+'.sav', spi, spi_m, spi_sd, status_save
		dosave=1
    endif else begin
		dosave=1
    endelse
    
    window, /free, xs=1024, ys=768
    display, spi_m[adsec.act_w_curr], adsec.act_w_curr, /no_n, /sh, TITLE='MEAN SPI CURRENT', BAR_TITLE='[A]'
    if dosave then write_jpeg, filename+'_spi_m.jpg', tvrd(true=3), true=3, quality=100

    window, /free, xs=1024, ys=768
    display, spi_sd[adsec.act_w_curr], adsec.act_w_curr, /no_n, /sh, TITLE='RMS SPI CURRENT', BAR_TITLE='[A]' 
    if dosave then write_jpeg, filename+'_spi_sd.jpg', tvrd(true=3), true=3, quality=100
end

    
