
function telemetry_file, from, to, DOSAVE=dosave, Night=night, asma=asma
    
    base='$ADOPT_MEAS/telemetry_out/'
    families=['ChDistAverage', 'ChCurrAverage', 'ChDistRMS','ChCurrRMS','elevation'];,'Windspeed', 'FluxRateIn',$
     ;         'HubTemp','WaterColdplateIn','WaterColdplateOu','CPTemp']
    night_fam=[ 'FluxRateIn','elevation','Windspeed','HubTemp','WaterColdplateIn','WaterColdplateOu']
    ran=[[8,11], [0,90],[0,8],[-10,10],[-10,10],[-10,10]     ]
    unit=['L/s','deg', 'm/s',replicate('C',4)]
    if keyword_set(asma) then families=['ChDistAverage', 'ChCurrAverage', 'WaterColdplateIn','WaterColdplateOu', 'ExternalHumidity', 'DewPoint']
    nfam=n_elements(families)
     ext=''
    telem=create_struct('from',from,'to',to)
    
    if keyword_set(night) then begin 
         nfam=n_elements(night_fam)
         ext='_overnight'
         families=night_fam
    endif        
        for i=0, nfam -1 do begin
            val=telemetry(families[i], from=from, to=to)
            telem=create_struct(telem, families[i],val) 
            nval=n_elements(val)
            if keyword_set(night) then  begin
               if i eq 0 then np=n_elements(val)
               xarr=findgen(nval)*np/nval
                    
                    ;val=resample(val, np)
                    ;list=resample(list, np)
                    ;id=where(list eq 1)
               ;endelse
               ;plot, xarr,val, title=night_fam[i], xtitle='Minutes since start',ytitle=unit[i], yr=ran[*,i], psym=1
               if keyword_set(dosave) then write_jpeg,base+from+night_fam[i]+'.jpg', tvrd(true=3), true=3, quality=100
            endif    
        endfor
        act=last_status(telem.from)  
        telem=create_struct(telem,'ACT',act) 
        if keyword_set(dosave) then    save, telem, filename='$ADOPT_MEAS/adsec_calib/telemetry_out/telemetry-'+from+'-'+to+ext+'.sav'
    return, telem
end
