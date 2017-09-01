;$Id: drift_test.pro,v 1.4 2009/06/11 11:08:40 labot Exp $
sc.debug = 0
no_status = 0
save_sys_status = 1
save_flat_status = 0

dir = meas_path('drift')
base_name = dir+'drift_test'
;update_panels, /opt
n_samples = 1000;8000;20; 600;; 4115;4115;4115 ;4115
delay =  0.1;7;14.;14;14

final = n_samples*(delay+1)

window, /free, title='ENVIRONMENT TEMPERATURE AND DEW POINT '
win0 = !D.WINDOW
window, /free, title='ZOOM LAST HOUR :ENVIRONMENT TEMPERATURE AND DEW POINT '
win1 = !D.WINDOW

window, /free, title='ALL SYSTEM(CP,RP, STRUCTURE, WATER) '
win2 = !D.WINDOW

window, /free, title='ZOOM LAST HOUR : ALL SYSTEM(CP,RP, STRUCTURE, WATER)'
win3 = !D.WINDOW

;window, /free, title='WATER FLUX '
;win4 = !D.WINDOW

;window, /free, title='ZOOM LAST HOUR :WATER FLUX '
;win5 = !D.WINDOW

window, /free, title='INTERNAL TEMPERATURE (STRATIX BCU, DSP, SWITCH BCU,POWER BP) '
win6 = !D.WINDOW

window, /free, title='ZOOM LAST HOUR : INTERNAL TEMPERATURE (STRATIX BCU, DSP, SWITCH BCU,POWER BP) '
win7 = !D.WINDOW

;temperature variables
cold_plate = dblarr(9, n_samples)
ref_plate  = dblarr(3, n_samples)
other_temp = dblarr(2, n_samples)
power_back = dblarr(6, n_samples)
water      = dblarr(3, n_samples); inlet, outlet, flux
temp_hum   = dblarr(2, n_samples)
dew_hum    = dblarr(2, n_samples) 

stratix_bcu = dblarr(6, n_samples)
driver_dsp   =dblarr(6,n_samples)
stratix_switch = dblarr(n_samples)


time_vect  = dblarr(n_samples) 

tcol       = comp_colors(40)    

thour      = ulong(60.*60./delay)
; end variables
slow_diag_name = process_list.housekeeper.msgd_name
;stop
temp_name=[ $
"BCUCoolerIn0Temp","BCUCoolerIn1Temp","BCUCoolerOut0Temp","BCUCoolerOut1Temp",$
"BCUCoolerPressure","BCUHumidity"]

time0 = systime(/sec)
day=systime()


if ~no_status then begin
    err = get_status(sys0)
    if err then message, 'WARNING: ERROR READING GET_STATUS: '+string(err), /INFO
endif


err = GetDiagnValue(slow_diag_name, 'BCUCooler* BCUH*', -1, -1, cooler,tim=5)
if err ne adsec_error.ok then print, 'Diagnostic failed!'

err = GetDiagnValue(slow_diag_name, '*Stratix* *PowerTemp* *DspsTemp* *DriverTemp*', -1, -1, temp_board,tim=5)
if err ne adsec_error.ok then print, 'Diagnostic failed!'

err = GetDiagnValue(slow_diag_name, '*Current* *Voltage*', -1, -1,power ,tim=5)
if err ne adsec_error.ok then print, 'Diagnostic failed!'
adsec_save = adsec


save, file=base_name+'_conf.sav', cooler, temp_board, power, sys0, time0, no_status, adsec_save, day


for i=0, n_samples-1 do begin

 
    time = systime(/sec)
    if ~no_status then begin
        err = update_status()
        if err then message, 'WARNING: ERROR READING UPDATE_STATUS: '+string(err), /INFO
        ;update_panels, /no_read
    endif
    
    err = GetDiagnValue(slow_diag_name, 'BCUCooler* BCUH*', -1, -1, cooler,tim=5)
    if err ne adsec_error.ok then print, 'Diagnostic failed!'

    err = GetDiagnValue(slow_diag_name, '*Stratix* *PowerTemp* *DspsTemp* *DriverTemp*', -1, -1, temp_board,tim=5)
    if err ne adsec_error.ok then print, 'Diagnostic failed!'
    
    err = GetDiagnValue(slow_diag_name, '*Current* *Voltage*', -1, -1,power ,tim=5)
    if err ne adsec_error.ok then print, 'Diagnostic failed!'
    
    t_sens0   = cooler[20].last
    t_sens1   = cooler[23].last
    hr_sens0  = cooler[32].last 
    hr_sens1  = cooler[35].last
        
    
    if t_sens0 gt 0.0 then begin 
        svp_sens0 = svp(t_sens0)*(hr_sens0/100.) 
    endif else begin 
        svp_sens0 = svp(t_sens0, /ice)*(hr_sens0/100.) 
    endelse
    
    dew_sens0 = dew_point(svp_sens0)
    frost_sens0 = dew_point(svp_sens0, /frost)
    dew_p0 = dew_sens0>frost_sens0
    
    if t_sens1 gt 0.0 then begin 
        svp_sens1 = svp(t_sens1)*(hr_sens1/100.) 
    endif else begin
        svp_sens1 = svp(t_sens1, /ice)*(hr_sens1/100.) 
    endelse 

    dew_sens1 = dew_point(svp_sens1)
    frost_sens1 = dew_point(svp_sens1, /frost)
    
    dew_p1 = dew_sens1>frost_sens1 
    

 

    if ~save_sys_status then begin 
        save, file=base_name+'_'+string(i, format='(i5.5)')+'.sav', cooler, temp_board, power, sys_status, dew_p0, dew_p1, time, no_status, save_sys_status
    endif else begin
        ;update_panels, /no_read, /opt
        pos_save = sys_status.position
        curr_save = sys_status.current
        ff_curr = sys_status.ff_current
        save, file=base_name+'_'+string(i, format='(i5.5)')+'.sav', cooler, temp_board, power, sys_status, dew_p0, dew_p1, time, no_status, pos_save, curr_save, save_sys_status, ff_curr
    endelse


    if save_sys_status then begin
        err = update_status()
        ;update_panels, /no_read, /opt ;ONLY FOR SCREW
        pos_save = sys_status.position
        curr_save = sys_status.current
        ff_curr = sys_status.ff_current
        save, file = base_name+'_status_'+string(i, format='(I5.5)')+'.sav',  pos_save, curr_save, ff_curr
    endif


    if save_flat_status then begin
        err = idl_4d_init()
        if err ne adsec_error.ok then print, 'OPD init failed!'
        name_radix = "tmp0"
        err = get_m4d(name_radix, opd0, mask0)
        if err ne adsec_error.ok then message, "OPD acquisition failed!"
        save, file = base_name+'_flat_'+string(i, format='(I5.5)')+'.sav',  opd0, mask0
        err = idl_4d_finalize()
        if err ne adsec_error.ok then print, 'OPD finalize failed!'
    endif

        
    

;if (dew_p1) gt t_sens1 then begin
;    spawn, 'play siren.wav'
;    message, 'WARNING:ATTENTION THE DEW POINT IS OVER THE TEMPERATURE IN SENSOR #1!!!!!', /info
;endif

;if dew_p0 gt t_sens0 then begin 
;    spawn, 'play siren.wav'
;    message, 'WARNING:ATTENTION THE DEW POINT IS OVER THE TEMPERATURE IN SENSOR #0!!!!!', /info
;endif

; fill variables
    cold_plate[0,i] = cooler[6].last  ;in3
    cold_plate[1,i] = cooler[12].last ;no  
    cold_plate[2,i] = cooler[18].last ;no 
    cold_plate[3,i] = cooler[7].last  ;out3
    cold_plate[4,i] = cooler[13].last ;in8
    cold_plate[5,i] = cooler[19].last ;out9
    cold_plate[6,i] = cooler[9].last  ;center
    cold_plate[7,i] = cooler[10].last ;extern 
    cold_plate[8,i] = cooler[11].last ;middle
    
    power_back[0,i] = cooler[0].last
    power_back[1,i] = cooler[1].last
    power_back[2,i] = cooler[2].last
    power_back[3,i] = cooler[3].last
    power_back[4,i] = cooler[4].last
    power_back[5,i] = cooler[5].last
    
    ref_plate[0,i]  = cooler[8].last 
    ref_plate[1,i]  = cooler[15].last 
    ref_plate[2,i]  = cooler[17].last
    
    other_temp[0,i]  = cooler[21].last ; steel structure 
    other_temp[1,i]  = cooler[22].last ; swing arm
    
    water[0,i]  = cooler[14].last ; water inlet  
    water[1,i]  = cooler[16].last ; water outlet 
    water[2,i]  = cooler[31].last ; water flux 
    
    temp_hum[0,i]  = cooler[20].last 
    temp_hum[1,i]  = cooler[23].last 
    
    dew_hum[0,i]  =  dew_p0  
    dew_hum[1,i]  =  dew_p1
    
    idx_temp = [6,12,18,7,13,19,9,10,11,8,15,17,21,22,14,16]
    min_temp = min ((cooler.last)[idx_temp])
    
    if (dew_p1) gt min_temp then begin
        spawn, 'play siren.wav'
        message, 'WARNING:ATTENTION THE DEW POINT IS OVER THE TEMPERATURE!!!!!', /info
    endif
    
    
    stratix_bcu[*,i] = (temp_board.last)[6:11]
    driver_dsp[*,i]   = rebin((temp_board.last)[12:95], 6)
    ids = where(temp_board.family eq 'SWITCHSTRATIXTEMP')

    stratix_switch[i] = temp_board[ids].last

;vect = [stratix_bcu, stratix_switch, driver_dsp, power_back]
    
    time_vect[i]  = time - time0
    
;endfill
    
    
; plotting
    if i eq 1 then thour=ulong(round(60.*60./(time_vect[1]-time_vect[0])))
    if i gt thour then j =i-thour else  j = 0   
;max_temp = max(temp_hum, cold_plate, ref_plate, other_temp, water[0,*], water[0,*])
    yr_temp = [-40.,40.]
    yr_int  = [-30., 70.]
    yr_temp0 = [-50.,40.]
    wset, win0
    plot, time_vect[0:i], temp_hum[0,0:i], yr = yr_temp0, xs=17
    xyouts, delay, 0, '- Sens#0'
    oplot, time_vect[0:i], temp_hum[1,0:i], col = tcol[1] 
    xyouts, delay, -2, '- Sens#1',  col = tcol[1]
                                ;   oplot, time_vect[0:i], dew_hum[0,0:i], col = tcol[10]
                                ;   xyouts, delay, 2, '- Dew#0',col = tcol[10]
    oplot, time_vect[0:i], dew_hum[1,0:i], col = tcol[25]
    xyouts, delay, 4, '- Dew#1', col = tcol[25]
    
    wset, win1
    plot, time_vect[j:i], temp_hum[0,j:i], yr= yr_temp0, xs=17

    xyouts, time_vect[j]+delay, 0, '- Sens#0'
    oplot, time_vect[j:i], temp_hum[1,j:i], col = tcol[1]
    
    xyouts, time_vect[j]+delay, -2, '- Sens#1',  col = tcol[1]
    oplot, time_vect[j:i], dew_hum[0,j:i], col = tcol[10]
    xyouts, time_vect[j]+delay, 2, '- Dew#0',col = tcol[10]
    oplot, time_vect[j:i], dew_hum[1,j:i], col = tcol[25]
    xyouts, time_vect[j]+delay, 4, '- Dew#1',col = tcol[25]
    
    wset, win2
    plot, time_vect[0:i], cold_plate[0,0:i],col= tcol[0], yr = yr_temp, xs=17
    xyouts, delay, 0, '- CP Sens#2',col = tcol[0]
    ;oplot, time_vect[0:i], cold_plate[1,0:i], col = tcol[1]
    ;oplot, time_vect[0:i], cold_plate[2,0:i], col = tcol[2]
    oplot, time_vect[0:i], cold_plate[3,0:i], col = tcol[3]
    oplot, time_vect[0:i], cold_plate[4,0:i], col = tcol[4]
    oplot, time_vect[0:i], cold_plate[5,0:i], col = tcol[5]
    oplot, time_vect[0:i], cold_plate[6,0:i], col = tcol[6]
    oplot, time_vect[0:i], cold_plate[7,0:i], col = tcol[7]
    oplot, time_vect[0:i], cold_plate[8,0:i], col = tcol[8]
    xyouts, delay, -2, '- CP Sens#22',col = tcol[8]
    
    oplot, time_vect[0:i], ref_plate[0,0:i], col = tcol[10]
    xyouts, delay, -4, '- RP Sens#10',col = tcol[10]
    oplot, time_vect[0:i], ref_plate[1,0:i], col = tcol[11]
    oplot, time_vect[0:i], ref_plate[2,0:i], col = tcol[12]
    xyouts, delay, -6, '- RP Sens#23',col = tcol[12]
    
    oplot, time_vect[0:i], other_temp[0,0:i], col = tcol[20]
    xyouts, delay, 2, '- EXTERNAL TEMPERATURE',col = tcol[20]
    oplot, time_vect[0:i], other_temp[1,0:i], col = tcol[21]
    xyouts, delay, 4, '- AL STRUCT',col = tcol[21]
    
    
    oplot, time_vect[0:i], water[0,0:i], col = tcol[30]
    xyouts, delay, 6, '- WATER INLET',col = tcol[30]
    oplot, time_vect[0:i], water[1,0:i], col = tcol[31]
    xyouts, delay, 8, '- WATER OUTLET',col = tcol[31]
    oplot, time_vect[0:i], dew_hum[1,0:i]
    xyouts, delay, -8, '- Dew#1'
    wset, win3
    plot, time_vect[j:i], cold_plate[0,j:i],col= tcol[0], yr = yr_temp, xs=17
    xyouts, time_vect[j]+delay, 0, '- CP inlet',col = tcol[0]
    
    ;oplot, time_vect[j:i], cold_plate[1,j:i], col = tcol[1]
    ;oplot, time_vect[j:i], cold_plate[2,j:i], col = tcol[2]
    oplot, time_vect[j:i], cold_plate[3,j:i], col = tcol[3]
    oplot, time_vect[j:i], cold_plate[4,j:i], col = tcol[4]
    oplot, time_vect[j:i], cold_plate[5,j:i], col = tcol[5]
    oplot, time_vect[j:i], cold_plate[6,j:i], col = tcol[6]
    oplot, time_vect[j:i], cold_plate[7,j:i], col = tcol[7]
    oplot, time_vect[j:i], cold_plate[8,j:i], col = tcol[8]
    xyouts, time_vect[j]+delay, -2, '- CP middle',col = tcol[8]
    
    
    oplot, time_vect[j:i], ref_plate[0,j:i], col = tcol[10]
    xyouts, time_vect[j]+delay, -4, '- RP Sens#10',col = tcol[10]
    oplot, time_vect[j:i], ref_plate[1,j:i], col = tcol[11]
    
    oplot, time_vect[j:i], ref_plate[2,j:i], col = tcol[12]
    xyouts, time_vect[j]+delay, -6, '- RP Sens#23',col = tcol[12]

    oplot, time_vect[j:i], other_temp[0,j:i], col = tcol[20]
    xyouts, time_vect[j]+delay, 2, '- EXTERNAL TEMPERATURE',col = tcol[20]
    oplot, time_vect[j:i], other_temp[1,j:i], col = tcol[21]
    xyouts, time_vect[j]+delay, 4, '- AL STRUCT',col = tcol[21]

    oplot, time_vect[j:i], water[0,j:i], col = tcol[30]
    xyouts, time_vect[j]+delay, 6, '- WATER INLET',col = tcol[30]
    oplot, time_vect[j:i], water[1,j:i], col = tcol[31]
    xyouts, time_vect[j]+delay, 8, '- WATER OUTLET',col = tcol[31]
    oplot, time_vect[0:i], dew_hum[1,0:i]
    xyouts, time_vect[j]+delay, -8, '- Dew#1'

;wset, win4
;  plot, time_vect[0:i], water[2,0:i], ytitle='Flux [liter/min]'

;wset, win5
;  plot, time_vect[j:i], water[2,j:i],ytitle='Flux [liter/min]' 
    
    wset, win6
    plot, time_vect[0:i], stratix_bcu[0,0:i],col= tcol[1], yr = yr_int, xs=17 
    xyouts, delay, 0, '- STRATIX BCU #0 ',col = tcol[1]
    oplot, time_vect[0:i], stratix_bcu[1,0:i],col= tcol[2] 
    oplot, time_vect[0:i], stratix_bcu[2,0:i],col= tcol[3] 
    oplot, time_vect[0:i], stratix_bcu[3,0:i],col= tcol[4] 
    oplot, time_vect[0:i], stratix_bcu[4,0:i],col= tcol[5] 
    oplot, time_vect[0:i], stratix_bcu[5,0:i],col= tcol[6] 
    xyouts, delay, -3, '- STRATIX BCU #5 ',col = tcol[6]
    
    oplot, time_vect[0:i], driver_dsp[0,0:i],col= tcol[10] 
    xyouts, delay, -6, '- DRIVER DSP #0 ',col = tcol[10]
    
    oplot, time_vect[0:i], driver_dsp[1,0:i],col= tcol[11] 
    oplot, time_vect[0:i], driver_dsp[2,0:i],col= tcol[12] 
    oplot, time_vect[0:i], driver_dsp[3,0:i],col= tcol[13] 
    oplot, time_vect[0:i], driver_dsp[4,0:i],col= tcol[14] 
    oplot, time_vect[0:i], driver_dsp[5,0:i],col= tcol[15]
    xyouts, delay, -9, '- DRIVER DSP #5 ',col = tcol[15]
    
    oplot, time_vect[0:i], stratix_switch[0:i],col= tcol[34]
    xyouts, delay, 3, '- STRATIX SWITCH ',col = tcol[34]
    
    oplot, time_vect[0:i], power_back[0,0:i],col= tcol[20]
    xyouts, delay, 6, '- POWER BACK #0 ',col = tcol[20]
    
    oplot, time_vect[0:i], power_back[1,0:i],col= tcol[21] 
    oplot, time_vect[0:i], power_back[2,0:i],col= tcol[22] 
    oplot, time_vect[0:i], power_back[3,0:i],col= tcol[23] 
    oplot, time_vect[0:i], power_back[4,0:i],col= tcol[24] 
    oplot, time_vect[0:i], power_back[5,0:i],col= tcol[25]
    xyouts, delay, 9, '- POWER BACK #5 ',col = tcol[25]
    oplot, time_vect[0:i], dew_hum[1,0:i]
    xyouts, delay, 12, '- Dew#1'
    
    wset, win7
    
    plot, time_vect[j:i], stratix_bcu[0,j:i],col= tcol[1], yr = yr_int, xs=17
    xyouts, time_vect[j]+delay, 0, '- STRATIX BCU #0 ',col = tcol[1]
    oplot, time_vect[j:i], stratix_bcu[1,j:i],col= tcol[2]
    oplot, time_vect[j:i], stratix_bcu[2,j:i],col= tcol[3]
    oplot, time_vect[j:i], stratix_bcu[3,j:i],col= tcol[4]
    oplot, time_vect[j:i], stratix_bcu[4,j:i],col= tcol[5]
    oplot, time_vect[j:i], stratix_bcu[5,j:i],col= tcol[6]
    xyouts, time_vect[j]+delay, -3, '- STRATIX BCU #5 ',col = tcol[6]
    
    oplot, time_vect[j:i], driver_dsp[0,j:i],col= tcol[10]
    xyouts, time_vect[j]+delay, -6, '- DRIVER DSP #0 ',col = tcol[10]
    oplot, time_vect[j:i], driver_dsp[1,j:i],col= tcol[11]
    oplot, time_vect[j:i], driver_dsp[2,j:i],col= tcol[12]
    oplot, time_vect[j:i], driver_dsp[3,j:i],col= tcol[13]
    oplot, time_vect[j:i], driver_dsp[4,j:i],col= tcol[14]
    oplot, time_vect[j:i], driver_dsp[5,j:i],col= tcol[15]
    xyouts, time_vect[j]+delay, -9, '- DRIVER DSP #5 ',col = tcol[15]
    
    oplot, time_vect[j:i], stratix_switch[j:i],col= tcol[34]
    xyouts, time_vect[j]+delay, 3, '- STRATIX SWITCH ',col = tcol[34]
    
    oplot, time_vect[j:i], power_back[0,j:i],col= tcol[20]
    xyouts, time_vect[j]+delay, 6, '- POWER BACK #0 ',col = tcol[20]
    
    
    oplot, time_vect[j:i], power_back[1,j:i],col= tcol[21]
    oplot, time_vect[j:i], power_back[2,j:i],col= tcol[22]
    oplot, time_vect[j:i], power_back[3,j:i],col= tcol[23]
    oplot, time_vect[j:i], power_back[4,j:i],col= tcol[24]
    oplot, time_vect[j:i], power_back[5,j:i],col= tcol[25]
    xyouts, time_vect[j]+delay, 9, '- POWER BACK #5 ',col = tcol[25]
    oplot, time_vect[0:i], dew_hum[1,0:i]
    xyouts, time_vect[j]+delay, 12, '- Dew#1'
    
    
; end plotting
    
    
    id0 = reform(rebin(indgen(4)*6, 4, 6, /samp), 24) + reform(rebin(indgen(1,6), 4, 6, /samp), 24)
    id = [id0, [31, 32, 35]]
    
    print, "#1  Power backplane rear plate (U15)", cooler[id[0]].last ;, cooler[id[0]].family, cooler[id[0]].index
    print, "#2  Cooling IN3 on ColdPlate", cooler[id[1]].last ;, cooler[id[1]].family, cooler[id[1]].index
;    print, "#3  Cooling IN(down) on ColdPlate", cooler[id[2]].last ;, cooler[id[2]].family, cooler[id[2]].index
;    print, "#4  Cooling IN1 on ColdPlate", cooler[id[3]].last ;, cooler[id[3]].family, cooler[id[3]].index
    print, "#5  Power backplane rear plate (U15)", cooler[id[4]].last ;, cooler[id[4]].family, cooler[id[4]].index
    print, "#6  Cooling OUT3 on ColdPlate", cooler[id[5]].last ;, cooler[id[5]].family, cooler[id[5]].index
    print, "#7  Cooling OUT9 on ColdPlate", cooler[id[6]].last ;, cooler[id[6]].family, cooler[id[6]].index
    print, "#8  Cooling IN8 on ColdPlate", cooler[id[7]].last ;, cooler[id[7]].family, cooler[id[7]].index
    print, "#9  Power backplane rear plate (U15)", cooler[id[8]].last ;, cooler[id[8]].family, cooler[id[8]].index
    print, "#10 Reference plate near guide B", cooler[id[9]].last ;, cooler[id[9]].family, cooler[id[9]].index
    print, "#11 Water main inlet", cooler[id[10]].last ;, cooler[id[10]].family, cooler[id[10]].index
    print, "#12 Temperature of Humidity Sensor #0", cooler[id[11]].last ;, cooler[id[11]].family, cooler[id[11]].index
    print, "#13 Power backplane rear plate (U15)", cooler[id[12]].last ;, cooler[id[12]].family, cooler[id[12]].index
    print, "#14 Cold plate center", cooler[id[13]].last ;, cooler[id[13]].family, cooler[id[13]].index
    print, "#15 Reference plate near guide A", cooler[id[14]].last ;, cooler[id[14]].family, cooler[id[14]].index
    print, "#16 EXTERNAL TEMPERATURE", cooler[id[15]].last ;, cooler[id[15]].family, cooler[id[15]].index
    print, "#17 Power backplane rear plate (U15)", cooler[id[16]].last ;, cooler[id[16]].family, cooler[id[16]].index
    print, "#18 Cold plate extern", cooler[id[17]].last ;, cooler[id[17]].family, cooler[id[17]].index
    print, "#19 Water main outlet", cooler[id[18]].last ;, cooler[id[18]].family, cooler[id[18]].index
    print, "#20 AL STRUCTURE ", cooler[id[19]].last ;, cooler[id[19]].family, cooler[id[19]].index
    print, "#21 Power backplane rear plate (U15)", cooler[id[20]].last ;, cooler[id[20]].family, cooler[id[20]].index
    print, "#22 Cold plate middle", cooler[id[21]].last ;, cooler[id[21]].family, cooler[id[21]].index
    print, "#23 Reference plate near guide C", cooler[id[22]].last ;, cooler[id[22]].family, cooler[id[22]].index
    print, "#24 Temperature of Humidity Sensor #1", cooler[id[23]].last ;, cooler[id[23]].family, cooler[id[23]].index
    
    
    
    print, "Humidity    Water flux (liter/min)", cooler[id[24]].last ;, cooler[id[24]].family, cooler[id[24]].index
    print, "Humidity    Humidity Sensor #0", cooler[id[25]].last ;, cooler[id[25]].family, cooler[id[25]].index
    print, "Humidity    Humidity Sensor #1", cooler[id[26]].last ;, cooler[id[26]].family, cooler[id[26]].index
    print, "Dew point Sensor #1", dew_p1
    
    wait, delay
    print, "time:", time-time0, ' sec'
endfor

end

