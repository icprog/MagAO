pro make_step_plot, pattern, ext, ovs_v, ste_v, set_v, mod_v, jpeg=jpeg_path

    @adsec_common
    set_plot_defaults, /WHITE
    loadct, 3
    if n_elements(ext) eq 0 then ext=""
    fn = file_search(pattern)
    nfn = n_elements(fn)
    ovs_v = fltarr(nfn)  ; OVERSHOOT
    ste_v = fltarr(nfn)  ; STATIC ERROR
    set_v = fltarr(nfn)  ; SETTLING TIM
    mod_v = fltarr(nfn)  ; MODES NUMBER
  
    for i=0, nfn-1 do begin
        
        restore, fn[i]
        ns = n_elements(mod_com)
        ovs_v[i] = overshoot*100
        ste_v[i] = (steady_pos - mod_com[ns-1])/abs(mod_com[ns-1])*100
        mod_v[i] = number
        set_v[i] = set_time

    endfor
    
    col = ['0000ff'xl, '00ff00'xl, '00ffff'xl]
    window, /free, xs=1024, ys=768
    plot, mod_v, set_v, TITLE='Modal step responses '+ext, psym=4, yr=[0.5, 1.2], xs = 17, ys=17, XTITLE='Mode #', YTITLE='Settling time [ms]'
    oplot, [-1, 673], [1, 1], col=col[0], /line
    oplot, [-1, 673], [0.7, 0.7], col=col[1], /line
    oplot, [-1, 673], [0.637, 0.637], col=col[2], /line
    items = ['BASE SPEC', 'GOAL SPEC', 'PRESHAPER CURR']
    legend, items, col=col[0:n_elements(items)-1], linestyle=0, /right, /center, box=0
    if keyword_set(jpeg_path) then write_jpeg, jpeg_path+'settling_time.jpg', tvrd(true=3), true=3, quality=100

    
    window, /free, xs=1024, ys=768
    plot, mod_v, ovs_v, TITLE='Modal step responses '+ext, psym=4, yr=[0, 11], xs = 17, ys=17, XTITLE='Mode #', YTITLE='Overshoot [%]'
    oplot, [-1, 673], [10, 10], col='0000ff'xl, /line
    if keyword_set(jpeg_path) then write_jpeg, jpeg_path+'overshoot.jpg', tvrd(true=3), true=3, quality=100


    window, /free, xs=1024, ys=768
    rr = (max(abs(ste_v)))*1.1
    plot, mod_v, ste_v, TITLE='Modal step responses '+ext, psym=4, yr = [-rr, rr] $
        , xs = 17, ys=17, XTITLE='Mode #', YTITLE='Static error [%]', /line
    if keyword_set(jpeg_path) then write_jpeg, jpeg_path+'static_error.jpg', tvrd(true=3), true=3, quality=100

    restore, file_dirname(pattern)+'/adsec_save.sav', /ver
    idmax_st = where(set_v eq max(set_v))
    idmax_os = where(ovs_v eq max(ovs_v))

    restore, fn[idmax_st], /ver
    mod_pos *= 1e6
    mod_com *= 1e6
    moden = fix(stregex(file_basename(fn),'[0-9][0-9][0-9]',/ex))
    number= moden[idmax_st]
    np=n_elements(mod_com)
    c0 = mod_com[0]
    c1 = mod_com[np-1]
    window, /free, xs=1024, ys=768
    plot,time,mod_pos,psym=pos_psym,xr=[-1, 10], tit='Mode #'+strtrim(number,2)+' Settling time '+string(set_v[idmax_st], format='(F4.2)')+' [ms]'$
             , YSTY=17 , YR=[0-c1/50., c1+c1/8], xtitle = 'Time - ms', ytitle = 'Modal Amplitude - um', ytickformat = '(F4.2)'

    oplot, time, mod_com, LINE=3;, col=0
    oplot,minmax(time),replicate(c0+1.1*(c1-c0), 2), /LINE , col=255l
    oplot,minmax(time),replicate(c0+0.9*(c1-c0), 2), /LINE , col=255l
    oplot,[1,1], [-1, 1], col=255l, /LINE
    display, adsec_save.ff_p_svec[number[0], adsec_save.act_w_cl], adsec_save.act_w_cl, /sh, /SMOOTH, /NONUM, /NO_PLOT, POS=ima, OUT_VAL=0
    px = !x.window * !d.x_vsize ;Get position of plot window in device units
    py = !y.window * !d.y_vsize
    swx = px(1)-px(0)       ;Size in x in device units
    swy = py(1)-py(0)       ;Size in y
    s=min([swx/4, swy/4])
    sima=(size(ima))[1]
    if sima gt s then begin
       dd=fix(sima/s)
      idd=dd
      while(sima mod idd ne 0) do idd +=  1
          ima=rebin(ima, sima/idd, sima/idd,/SAMP)
    endif else begin
          ima=rebin(ima, sima*round(s/sima), sima*round(s/sima),/SAMP)
    endelse
    sima=(size(ima))[1]
    if keyword_set(same_cuts) then begin
        tvscl1, [[ima_com],[ima]], px[1]-sima*1.1, py[0]+swy/2-1.2*s, /DEVICE
    endif else begin
        tvscl1, ima, px[1]-sima*1.1, py[0]+swy/2, /DEVICE
    endelse
    if keyword_set(jpeg_path) then write_jpeg, jpeg_path+'max_settling_time.jpg', tvrd(true=3), true=3, quality=100


    restore, fn[idmax_os], /ver
    mod_pos *= 1e6
    mod_com *= 1e6
    moden = fix(stregex(file_basename(fn),'[0-9][0-9][0-9]',/ex))
    number= moden[idmax_os]
    np=n_elements(mod_com)
    c0 = mod_com[0]
    c1 = mod_com[np-1]
    window, /free, xs=1024, ys=768
    plot,time,mod_pos,psym=pos_psym,xr=[-1, 10], tit='Mode #'+strtrim(number,2)+' Overshoot '+string(ovs_v[idmax_os], format='(F4.2)')+' %'$
             , YSTY=17 , YR=[0-c1/50., c1+c1/8], xtitle = 'Time - ms', ytitle = 'Modal Amplitude - um', ytickformat = '(F4.2)'
    oplot, time, mod_com, LINE=3;, col=0
    oplot,minmax(time),replicate(c0+1.1*(c1-c0), 2), /LINE , col=255l
    oplot,minmax(time),replicate(c0+0.9*(c1-c0), 2), /LINE , col=255l
    oplot,[1,1], [-1, 1], col=255l, /LINE
    display, adsec_save.ff_p_svec[number[0], adsec_save.act_w_cl], adsec_save.act_w_cl, /sh, /SMOOTH, /NONUM, /NO_PLOT, POS=ima, OUT_VAL=0
    px = !x.window * !d.x_vsize ;Get position of plot window in device units
    py = !y.window * !d.y_vsize
    swx = px(1)-px(0)       ;Size in x in device units
    swy = py(1)-py(0)       ;Size in y
    s=min([swx/4, swy/4])
    sima=(size(ima))[1]
    if sima gt s then begin
       dd=fix(sima/s)
      idd=dd
      while(sima mod idd ne 0) do idd +=  1
          ima=rebin(ima, sima/idd, sima/idd,/SAMP)
    endif else begin
          ima=rebin(ima, sima*round(s/sima), sima*round(s/sima),/SAMP)
    endelse
    sima=(size(ima))[1]
    if keyword_set(same_cuts) then begin
        tvscl1, [[ima_com],[ima]], px[1]-sima*1.1, py[0]+swy/2-1.2*s, /DEVICE
    endif else begin
        tvscl1, ima, px[1]-sima*1.1, py[0]+swy/2, /DEVICE
    endelse
    if keyword_set(jpeg_path) then write_jpeg, jpeg_path+'max_overshoot_step.jpg', tvrd(true=3), true=3, quality=100


	for i=0, n_elements(fn)-1 do begin
		restore, fn[i], /ver
			mod_pos *= 1e6
			mod_com *= 1e6
			moden = fix(stregex(file_basename(fn),'[0-9][0-9][0-9]',/ex))
			number= moden[i]
			np=n_elements(mod_com)
			c0 = mod_com[0]
			c1 = mod_com[np-1]
			window, /free, xs=1024, ys=768
			plot,time,mod_pos,psym=pos_psym,xr=[-1, 10], tit='Mode #'+strtrim(number,2)+' Settling time '+string(set_v[i], format='(F4.2)')+' [ms]' +' Overshoot '+string(ovs_v[i], format='(F4.2)')+' %'$
             , YSTY=17 , YR=[0-c1/50., c1+c1/8], xtitle = 'Time - ms', ytitle = 'Modal Amplitude - um', ytickformat = '(F4.2)'
			oplot, time, mod_com, LINE=3;, col=0
			oplot,minmax(time),replicate(c0+1.1*(c1-c0), 2), /LINE , col=255l
			oplot,minmax(time),replicate(c0+0.9*(c1-c0), 2), /LINE , col=255l
			oplot,[1,1], [-1, 1], col=255l, /LINE
			display, adsec_save.ff_p_svec[number[0], adsec_save.act_w_cl], adsec_save.act_w_cl, /sh, /SMOOTH, /NONUM, /NO_PLOT, POS=ima, OUT_VAL=0
			px = !x.window * !d.x_vsize ;Get position of plot window in device units
			py = !y.window * !d.y_vsize
			swx = px(1)-px(0)       ;Size in x in device units
			swy = py(1)-py(0)       ;Size in y
			s=min([swx/4, swy/4])
			sima=(size(ima))[1]
			if sima gt s then begin
			   dd=fix(sima/s)
			  idd=dd
			  while(sima mod idd ne 0) do idd +=  1
				  ima=rebin(ima, sima/idd, sima/idd,/SAMP)
			endif else begin
				  ima=rebin(ima, sima*round(s/sima), sima*round(s/sima),/SAMP)
			endelse
			sima=(size(ima))[1]
			if keyword_set(same_cuts) then begin
				tvscl1, [[ima_com],[ima]], px[1]-sima*1.1, py[0]+swy/2-1.2*s, /DEVICE
			endif else begin
				tvscl1, ima, px[1]-sima*1.1, py[0]+swy/2, /DEVICE
			endelse
			if keyword_set(jpeg_path) then write_jpeg, jpeg_path+'mode_'+string(number[0],format='(I3.3)')+'_step.jpg', tvrd(true=3), true=3, quality=100


	endfor


end

