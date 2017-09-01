


pro lbto_noise, READ=read, ind=ind, type=type

    @adsec_common
    todo  = intarr(3)
    todo[type]=1
    ghost = todo[0] 
    telem = todo[1]
    optic = todo[2]
    jsave = 1
    
    measbase = '/home/aoacct/meas/adsec_calib/'
    jbase    = measbase + 'pic/'
    
;******************************** DEFINITION OF DATA FILES *********************************
    ghostfile = measbase + 'noise/'     + ['20100528_203346/noise_spectrum_full_gain_hbs-on.sav','20100528_203424/noise_spectrum_09_gain_hbs-on.sav','20100528_203458/noise_spectrum_08_gain_hbs-on.sav',$
                                           '20100528_155944/noise_spectrum_full_gain_set3.sav','20100528_160031/noise_spectrum_09_gain_set3.sav','20100528_160118/noise_spectrum_08_gain_set3.sav',$
                                           '20100528_203346/noise_spectrum_full_gain_hbs-on.sav','20100528_203424/noise_spectrum_09_gain_hbs-on.sav','20100528_203458/noise_spectrum_08_gain_hbs-on.sav']
    telemfile = measbase + 'telemetry/' + ['telemetry_20100525_224700.sav', 'data_20100526_170000.sav']
    opticfile = measbase + '4d/Zcopy/'  + ['20100528_3house_off3','20100528_hbs_on1', 'bearing_off', 'chiller_on', 'chiller_off1'] + '/hdf5'
    comment=['full gain-hb on ','0.9 gain-hb on','0.8 gain-hb on','full gain-hb off','0.9 gain-hb off','0.8 gain-hb off','full gain ','0.9 gain','0.8 gain']
    gx    = ind
    tx    = 0
    ox    = ind
    ext=comment[ind]
;********************************* DEFINITION OF FITTING PARAMETERS AND PUPIL **************    
    cir=[233.988  ,    244.367  ,    219.618  , -0.0945944]
    ss=[494,500]
    x=(findgen(ss[0])-cir[0])/cir[2]
    y=(findgen(1,ss[1])-cir[1])/cir[2]
    xx=rebin(x, ss, /samp)
    yy=rebin(y, ss, /samp)
    undefine, x,y
    
    x=adsec.act_coordinates[0,*]/455.
    y=adsec.act_coordinates[1,*]/455.
    psvec= adsec.ff_p_svec
    idc  = adsec.act_w_cl
    nidc = n_elements(idc)
   ;!P.BACKGROUND='FFFFFF'XL
   ;!P.COLOR='000000'XL
   ;!P.CHARSIZE=1
   ;!P.CHARTHICK=1.2
   ;!P.THICK=2
   ;!X.STYLE = 19
   ;!Y.STYLE = 19
   ;!X.THICK = 2
   ;!Y.THICK = 2
   
    
    if ghost eq 1 then begin
        
        if keyword_set(read) then begin
            restore, ghostfile[gx]    ;buf, fvec, fft_buf
        endif else begin
            print, get_opt_ghosts_v2(adsec.act_w_cl, /save, ext_new='_LBTOall')
        endelse
        decim=20    
        np = n_elements(fvec)
        nmodes2fit= 20
        mirmodes=dblarr(nmodes2fit,np/decim+1)
        buf -= rebin(buf[0,*], np, nidc, /sample)        
        j=0
        for i=0, np -1, decim do begin
            mirmodes[*,j]=transpose(psvec[0:nmodes2fit-1,adsec.act_w_cl])##(buf[i,*])
            ;plot, mirmodes[*,j], title='Mirror modes decomposition of actual position'+string(i), psym=-4
            j+=1
            wait, 0.005
        endfor
        dt=1/(fvec[np-1]/decim)
        startp=200
        fft1, mirmodes[0,startp:*],dt, fspec=freq, psd=psd, /noplot
        fbin=freq[1]-freq[0]
        window, /free
    
        plot,(freq), psd,/nodata,yr=[0,5e-17], title='Mirror modes freq spectra'+ext, xtitle='Freq (Hz)', psym=10, xr=[0,200]
        m2fft=5
        coltable=comp_colors(m2fft)

        modestring=''
        peak=dblarr(m2fft, 2)
        for i=0, m2fft-1 do begin
;            mirmodes[i,*]-=replicate(mean(mirmodes[i,*]), 1,np/decim)
            fft1, mirmodes[i,startp:*], dt, fspec=freq, psd=psd, /noplot
            a=max(psd, x)
            peak[i,*]=[sqrt(a*fbin),freq[x]]
            oplot, (freq), psd, col=coltable[i]
            modestring=[modestring,string(i)]
            
         endfor
        legend, /right, col=coltable[*], modestring[1:*], linestyle=2
        ;window, 1
        write_jpeg, jbase+ext+'.jpg', tvrd(true=3), true=3, quality=100
        ;plot, peak[*,0], title='Max peak rms'
    
    print, ghostfile[gx]
    print, peak
    endif
    if telem eq 1 then begin

    endif

    if optic eq 1 then begin
        window, 0
        window, 1
        flist=file_search(opticfile[ox], '*.h5')
        nfile=n_elements(flist)
        opd0 = read_opd(flist[0], bad=mask0, type='4d')
        term =[1,2,3,4,5,6,7,8]
        term0=[1,2,3]    
        dt =1/28.5
        zern = dblarr(nfile-1, n_elements(term))
        rr = dblarr(nfile-1,3)
        for i=1, nfile-2 do begin
            opd=read_opd(flist[i], bad=mask, type='4d')
            mask=mask*mask0
            idx=where(mask)
            no=where(mask eq 0)
            opd=opd-opd0
            opdcheck=opd
            rr[i,0] = rms(opd[idx])
            fit0=surf_fit(xx[idx],yy[idx],opd[idx],term0, /zern, coeff=coeff)
            opdcheck[idx]-=fit0
            rr[i,1]= rms(opdcheck[idx])
            wset, 0
            opdcheck[no]=min(opdcheck[idx])
            image_show, opdcheck, /as,/sh, title='diff image, 3 terms removed, rms= '+string(rr[i,1])
           
            fit=surf_fit(xx[idx],yy[idx],opd[idx],term, /zern, coeff=coeff)
            zern[i,*]=coeff
            opd[idx]-=fit
            rr[i,2] = rms(opd[idx])
            wset, 1          
            opd[no]=min(opd[idx])
            image_show, /as, /sh, opd, title='diff image, 8 terms removed, rms= '+string(rr[i,2])
           
        endfor
        window, 4
        cc=comp_colors(n_elements(term))
        peakz=dblarr(n_elements(term),2)
        fft1, zern[*,0], dt,fspec=freq, psd=psd, /noplot
        window, /free
        plot, (freq), psd, /noda, title='Zern modes optical rms', psym=10, xtitle='Freq (Hz)'
        fbin=freq[1]-freq[0]
        for i=0, n_elements(term)-1 do begin
            fft1, zern[*,i], dt, fspec=freq, psd=psd, /noplot
            oplot,(freq), psd, col=cc[i]
            a=max(psd, x)
            peakz[i,*]=[sqrt(fbin*a),freq[x]]
        endfor
        legend, /right, linestyle=2, col=cc[*], ['1','2','3','4','5','6','7','8']
        write_jpeg, jbase+'opt_data_spectrum.jpg', tvrd(true=3), true=3, quality=100

       ; window, 3
       ; plot, peakz[*,0], title='Max peak optical rms'

    endif
    stop

;write_jpeg, name, tvrd=(true=3, true=3, quality=100)
end 
