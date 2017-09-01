Pro script_cal_data_analize, COMPUTATION=computation, SINGLE=single, LARGE=large, JPG=jpg, FILE=filef, d0=d0_pack, cr=cr_pack
;SCRIPT PER ANALIZZARE UN SET DI DATI CON LO STESSO NOME

steps=3
path = '~/calibrazione/'
@adsec_common

!p.background='ffffffff'xul
!P.THICK = 3
!X.THICK = 3
!Y.THICK = 3
!P.CHARSIZE = 2
!P.CHARTHICK = 2
!p.color = 0
!p.color = 0
if keyword_set(LARGE) then begin
    xs = 1280
    ys = 1024
    psymd = 7
endif else begin
    xs = 800
    ys = 600
    psymd = 3
endelse

    print, "File Selection..."
    if n_elements(filef) eq 0 then template = dialog_pickfile(TITLE="Please Select a File TEMPLATE:") else template = filef

    if template eq "" then begin
        print, "NO FILE SELECTED!. Returning."
        return
    endif

    base = file_basename(template)
    dir = file_dirname(template)
    names_plus = file_search(dir, stregex(base,'.*_', /extract)+'*')
    if keyword_set(single) then begin
        names[0] = template
        names=names[0]
    endif

;names = names[0:8]

if keyword_set(computation) then begin

    base = file_basename(template)
    dir = file_dirname(template)
    names = file_search(dir, stregex(base,'.*_', /extract)+'*')
    if keyword_set(single) then begin
        names[0] = template
        names=names[0]
    endif

    for k=0, n_elements(names)-1 do begin

        print, "NOW ANALIZING FILE: "+ names[k]
        dummy = multiple_cal_data_analize(DATA=names[k], ACT_COORD=act_coord )
        base = file_basename(names[k])
        step_filter = stregex(base,'.*step.', /extract)
        new_name = strmid(step_filter, 0,strlen(step_filter)-6) + '-step-'
        meas_filter  = stregex(base,'_meas_.*', /extract)
        name_neg = file_search(dir, new_name+'*'+meas_filter)
        name_neg=name_neg[0]
        dummy = multiple_cal_data_analize(DATA=name_neg, ACT_COORD=act_coord )
 
        ;TODO:::: sistemare la selezione dei file
        
    endfor


endif else begin


d0_pack = dblarr(n_elements(names), n_elements(adsec.true_act))
cr_pack = dblarr(n_elements(names), n_elements(adsec.true_act))

window, 0,  xs=xs, ys=ys
window, 1,  xs=xs, ys=ys

col = comp_colors(n_elements(names))

For k=0, n_elements(names)-1 do begin

    print, "Restoring file: "+names[k]
    restore, names[k]
    d0_pack[k,*] = params[0,*]
    cr_pack[k,*] = params[1,*]


    wset, 0
    if k eq 0 then  plot, x_over_real, residue*1e9, psym=psymd, xs=17$
                        ,TITLE="FIT RESIDUE OF ALL MEASURES", XTITLE="Normalized Voltage [V]", YTITLE="[nm]" , /NODATA
    oplot,  x_over_real, residue*1e9, psym=psymd, col=col[k]


    wset, 1
    if k eq 0 then  plot, x_over_real[id_res[1]:id_res[2]-1], residue[id_res[1]:id_res[2]-1]*1e9, psym=psymd, xs=17$
                        ,TITLE="FIT RESIDUE OF ALL MEASURES ACT 1", XTITLE="Normalized Voltage [V]", YTITLE="[nm]" , /NODATA
    xx =  x_over_real[id_res[1]:id_res[2]-1]
    yy = residue[id_res[1]:id_res[2]-1]*1e9
    
    oplot, xx, yy, col=col[k], thick=1

    if k eq -1 then  begin
        window, 2
        wset, 2
        plot, x_over_real, residue*1e9, psym=psymd, xs=17$
                        ,TITLE="FIT RESIDUE OF MEASURES 0 ALL ACTS", XTITLE="Normalized Voltage [V]", YTITLE="[nm]" , /NODATA
        cpp = comp_colors(45)
        for i=0, 43 do begin
            xx =  x_over_real[id_res[i]:id_res[i+1]-1]
            yy = residue[id_res[i]:id_res[i+1]-1]*1e9
            oplot, xx, yy,  thick=1, col=cpp[i]
            wait, 0.3
            print, i
        endfor
        xx =  x_over_real[id_res[i]:*]
        yy = residue[id_res[i]:*]*1e9
        oplot, xx, yy,  thick=1, col=cpp[i]
        type="_residue"
        if keyword_set(JPG) then begin
            img_name = path+strmid(file_basename(names[k]),0,strlen(base)-4)
            write_jpeg, img_name+type+'.jpg', tvrd(true=3), quality=100, true=3
        endif

    endif

    print, k


EndFor

    wset, 0
    type="_residue_dot"
    if keyword_set(JPG) then begin
        img_name = path+strmid(file_basename(names[0]),0,strlen(base)-4)
        write_jpeg, img_name+type+'.jpg', tvrd(true=3), quality=100, true=3
    endif

    wset, 1
    type="_residue_act1_time"
    if keyword_set(JPG) then begin
        img_name = path+strmid(file_basename(names[0]),0,strlen(base)-4)
        write_jpeg, img_name+type+'.jpg', tvrd(true=3), quality=100, true=3
    endif

;adsec = adsec_save
cr_pack = abs(cr_pack)
d0_pack = abs(d0_pack)

d0_mean = rebin(d0_pack,1,n_elements(adsec.true_act))
cr_mean = rebin(cr_pack,1,n_elements(adsec.true_act))

d0_sd = dblarr(n_elements(adsec.true_act))
cr_sd = dblarr(n_elements(adsec.true_act))
d0_displ = dblarr(n_elements(adsec.true_act))
cr_displ = dblarr(n_elements(adsec.true_act))
for i=0,  n_elements(adsec.true_act)-1 do begin
    d0_sd[i] = stddev(d0_pack[*,i])
    cr_sd[i] = stddev(cr_pack[*,i])
    d0_displ[i] = max(abs(d0_pack[*,i]-d0_mean[i]))
    cr_displ[i] = max(abs(cr_pack[*,i]-cr_mean[i]))
endfor

;d0_displ = max(abs(d0_pack-rebin(d0_mean,n_elements(names), n_elements(adsec.true_act), /SAMP)), dim=1)
;cr_displ = max(abs(cr_pack-rebin(cr_mean,n_elements(names), n_elements(adsec.true_act), /SAMP)), dim=1)

d02w = dblarr(1,adsec.n_actuators)
d02w[adsec.true_act] = d0_mean
cr2w = dblarr(1,adsec.n_actuators)
cr2w[adsec.true_act] = cr_mean
loadct, 12
    openw, unit, adsec_path.temp+"d0_mean_calibrated.txt" ,/GET_LUN
    printf, unit, "float 48"
    printf, unit, d02w
    close, unit
    openw, unit, adsec_path.temp+"cr_mean_calibrated.txt" ,/GET_LUN
    printf, unit, "float 48"
    printf, unit, cr2w
    close, unit
    print, "d0_mean_calibrated.txt and cs_mean_calibrated.txt write into temporary folder."
 
window, /free, xs=xs, ys=ys
display, d0_mean*1e6, adsec.true_act, TITLE="Mean value of d0", /sh, BAR_TITLE="[!7l!3m]", max_v=52, YTICKF_BAR='(F5.2)' ;**

type="_mean_d0_value"
if keyword_set(JPG) then begin
    img_name = path+strmid(file_basename(names[0]),0,strlen(base)-4)
    write_jpeg, img_name+type+'.jpg', tvrd(true=3), quality=100, true=3
endif

window, /free, xs=xs, ys=ys
display, d0_sd*1e6, adsec.true_act, TITLE="RMS on d0 values", /sh, BAR_TITLE="[!7l!3m]", YTICKF_BAR='(F5.2)'

window, /free, xs=xs, ys=ys
display, abs(d0_displ/d0_mean)*100, adsec.true_act, TITLE="MAX d0 Deviation", /sh, BAR_TITLE="[%]", YTICKF_BAR='(F5.2)'


window, /free, xs=xs, ys=ys
display, cr_mean, adsec.true_act, TITLE="Mean value of C-Ratio", /sh, BAR_TITLE="[-]",  YTICKF_BAR='(F6.3)';**

type="_mean_cr_value"
if keyword_set(JPG) then begin
    img_name = path+strmid(file_basename(names[0]),0,strlen(base)-4)
    write_jpeg, img_name+type+'.jpg', tvrd(true=3), quality=100, true=3
endif

window, /free, xs=xs, ys=ys
display, cr_sd, adsec.true_act, TITLE="RMS on C-Ratio values", /sh, BAR_TITLE="[-]",  YTICKF_BAR='(F6.3)'

window, /free, xs=xs, ys=ys
display, abs(cr_displ/cr_mean)*100, adsec.true_act, TITLE="MAX C-Ratio Deviation", /sh, BAR_TITLE="[%]", YTICKF_BAR='(F5.2)'

window, /free, xs=xs, ys=ys
display, abs((abs(adsec.c_ratio[adsec.true_act])-abs(cr_mean))/abs(cr_mean))*100, adsec.true_act, TITLE="Deviation WRT actual C-ratio", /sh, BAR_TITLE="[%]", YTICKF_BAR='(F5.2)'

window, /free, xs=xs, ys=ys
display, abs((adsec.d0[adsec.true_act]-abs(d0_mean))/d0_mean)*100, adsec.true_act, TITLE="Deviation WRT actual d0", /sh, BAR_TITLE="[%]", YTICKF_BAR='(F5.2)'

window, /free, xs=xs, ys=ys
display, d0_mean/d0_sd*sqrt(n_elements(names)), adsec.true_act, TITLE="SNR d0", /sh, BAR_TITLE="[!7l!3m]", YTICKF_BAR='(F5.1)';**

    type="_SNR_d0"
    if keyword_set(JPG) then begin
        img_name = path+strmid(file_basename(names[0]),0,strlen(base)-4)
        write_jpeg, img_name+type+'.jpg', tvrd(true=3), quality=100, true=3
    endif

window, /free, xs=xs, ys=ys
display, abs(cr_mean)/cr_sd*sqrt(n_elements(names)), adsec.true_act, TITLE="SNR C-ratio", /sh, BAR_TITLE="[!7l!3m]",  YTICKF_BAR='(F5.1)';**

    type="_SNR_cr"
    if keyword_set(JPG) then begin
        img_name = path+strmid(file_basename(names[0]),0,strlen(base)-4)
        write_jpeg, img_name+type+'.jpg', tvrd(true=3), quality=100, true=3
    endif

stop

endelse
return

End

