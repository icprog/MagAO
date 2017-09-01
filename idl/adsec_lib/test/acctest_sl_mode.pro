;test_id='test3-day1'
;after_load_shape_timestamp = '20091214_125410'
;before_aogain_timestamp = '20091214_125610'
;after_aogain_timestamp = '20091214_125655'
;end_aogain_timestamp = '20091214_125907'
;filename='/backup/logs/aospare/FASTDGN00_TELEMETRY/FASTDGN00_TELEMETRY.00001260795148.log'

test_id='test2-day1'
after_load_shape_timestamp = '20091214_125959'
before_aogain_timestamp = '20091214_130202'
after_aogain_timestamp = '20091214_130233'
end_aogain_timestamp = '20091214_130434'
filename='/backup/logs/aospare/FASTDGN00_TELEMETRY/FASTDGN00_TELEMETRY.00001260795148.log'

;test_id='test1-day1'
;after_load_shape_timestamp = '20091214_130807'
;before_aogain_timestamp = '20091214_131011'
;after_aogain_timestamp = '20091214_130233'
;end_aogain_timestamp = '20091214_130434'
;filename='/backup/logs/aospare/FASTDGN00_TELEMETRY/FASTDGN00_TELEMETRY.00001260795148.log'

;test_id='test2-day2'
;after_load_shape_timestamp = '20091215_103710'
;before_aogain_timestamp = '20091215_103917'
;after_aogain_timestamp = '20091215_103954'
;end_aogain_timestamp = '20091215_104213'
 

;test_id='test3.1-day2'
;after_load_shape_timestamp = '20091215_102548'
;before_aogain_timestamp = '20091215_102749'
;after_aogain_timestamp = '20091215_102818'
;end_aogain_timestamp = '20091215_103034'

;test_id='test3.2-day2'
;after_load_shape_timestamp = '20091215_103150'
;before_aogain_timestamp = '20091215_103349'
;after_aogain_timestamp = '20091215_103421'
;end_aogain_timestamp = '20091215_103629'

;test_id='test3.3-day2'
;after_load_shape_timestamp = '20091215_104426'
;before_aogain_timestamp = '20091215_104710'
;after_aogain_timestamp = '20091215_104746'
;end_aogain_timestamp = '20091215_105047'


;test_id='test3.1-day3'
;after_load_shape_timestamp = '20091216_095539'
;before_aogain_timestamp = '20091216_095730'
;after_aogain_timestamp = '20091216_095855'
;end_aogain_timestamp = '20091216_100100'

;test_id='test3.2-day3'
;after_load_shape_timestamp = '20091216_100300'
;before_aogain_timestamp = '20091216_100500'
;after_aogain_timestamp = '20091216_100540'
;end_aogain_timestamp = '20091216_100800'

;test_id='test3.3-day3'
;after_load_shape_timestamp = '20091216_101240'
;before_aogain_timestamp = '20091216_101558'
;after_aogain_timestamp = '20091216_101633'
;end_aogain_timestamp = '20091216_101834'

;test_id='test3.4-day3'
;after_load_shape_timestamp = '20091216_101925'
;before_aogain_timestamp = '20091216_102125'
;after_aogain_timestamp = '20091216_102145'
;end_aogain_timestamp = '20091216_102345'


;test_id='test2-day2'
;after_load_shape_timestamp = '20091215_103711'
;before_aogain_timestamp = '20091215_103917'
;after_aogain_timestamp = '20091215_103955'
;end_aogain_timestamp = '20091215_104213'

;flat_data_wfs_calibrated.sav
;after_load_shape_timestamp = '20091208_122216'
;before_aogain_timestamp = '20091208_122458'
;after_aogain_timestamp = '20091208_122513'
;end_aogain_timestamp = '20091208_122609'

;flat_data_wfs_calibrated.sav
;after_load_shape_timestamp = '20091208_164840'
;before_aogain_timestamp = '20091208_164948'
;after_aogain_timestamp = '20091208_165112'
;end_aogain_timestamp = '20091208_165345'

;600_modes_2009_08_10.sav
;after_load_shape_timestamp = '20091208_170010'
;before_aogain_timestamp = '20091208_170120'
;after_aogain_timestamp = '20091208_170153'
;end_aogain_timestamp = '20091208_170400'

;600_modes_2009_08_10_after_jump.sav. Failed actuator force filtering
;after_load_shape_timestamp = '20091208_170811'
;before_aogain_timestamp = '20091208_171045'
;after_aogain_timestamp = '20091208_171100'
;end_aogain_timestamp = '20091208_171524'
print, 'Using ref: '+test_id
print, 'Retrieving OL data from internal telemetry...'
data0 = telemetry('ChDistAverage' $
    , FROM=after_load_shape_timestamp $
    , TO=before_aogain_timestamp, /NOPLOT,filename=filename)
data0cur =  telemetry('ChCurrAverage' $
    , FROM=after_load_shape_timestamp $
    , TO=before_aogain_timestamp, /NOPLOT, filename=filename)
 
print, '...retreived number of DistAverage samples: ', strtrim(n_elements(data0.value[0,*]),2)
print, 'Retrieving CL data from internal telemetry...'
data1 = telemetry('ChDistAverage' $
    , FROM=after_aogain_timestamp $
    , TO=end_aogain_timestamp, /NOPLOT, filename=filename)
data1cur =  telemetry('ChCurrAverage' $
   , FROM=after_aogain_timestamp $
    , TO=end_aogain_timestamp, /NOPLOT, filename=filename)

print, '...retreived number of DistAverage samples: ', strtrim(n_elements(data1.value[0,*]),2)

s0 = size(data0.value, /DIM)
ave0 = rebin(double(data0.value),s0[0],1)
std0 = sqrt(rebin(double(data0.value)^2,s0[0],1)-ave0^2)

s1 = size(data1.value, /DIM)
ave1 = rebin(double(data1.value),s1[0],1)
std1 = sqrt(rebin(double(data1.value)^2,s1[0],1)-ave1^2)

restore, /V, '/towerdata/adsec_calib/PhaseMaps/MMmatrix_20090811_setg1.sav'

ave1all = dblarr(adsec.n_actuators)
ave1all[adsec.act_w_pos]=ave1

ave0all = dblarr(adsec.n_actuators)
ave0all[adsec.act_w_pos]=ave0

mm = transpose(mm2c)##transpose(ave1all-ave0all)
wfe = 2*mmmatrix##mm
wfe -= mean(wfe)
xx = rebin((findgen(dpix)-(dpix-1.0)/2)/((dpix-1.0)/2),dpix,dpix,/SAMP)
yy = transpose(xx)
fit = surf_fit(xx[idx_mask],yy[idx_mask],wfe,[1,2,3,4],/ZERN,COEFF=zcoeff)
wfe_ttf_removed = wfe-fit
wfe_map = fltarr(dpix,dpix)
wfe_map[idx_mask] = wfe_ttf_removed

std_wfe = stddev(wfe_ttf_removed)

loadct,3
window, 0
!P.BACKGROUND='FFFFFF'XL
!P.COLOR='000000'XL
!X.THICK=3.0
!Y.THICK=3.0
!P.THICK=3.0
!X.CHARSIZE=1.4 
!Y.CHARSIZE=1.4 
!P.CHARSIZE=1.4 
!P.CHARTHICK=2.0
!X.STYLE=19
!Y.STYLE=19
!X.MARGIN = [11,3]
!Y.MARGIN = [5,3]

image_show, wfe_map*1e9, /AS, /SH, TIT='WFE(=2*surf). sigma=' $
    +strtrim(fix(std_wfe*1e9),2)+'nm' $
    , BAR_TIT='[nm]', XAX=0.911/2*[-1,1], YAX=0.911/2*[-1,1] $
    , XTIT='[m]', YTIT='[m]'

print, 'WFE stdev [nm]:',strtrim(fix(std_wfe*1e9),2)
print, '2*capsens stdev OL [nm]:',strtrim(fix(2*sqrt(mean(std0^2))*1e9),2)
print, '2*capsens stdev CL [nm]:',strtrim(fix(2*sqrt(mean(std1^2))*1e9),2)

filename = '/towerdata/adsec_calib/Shapes/'+test_id+'result.sav'
save, data0, data1,data0cur,data1cur, after_load_shape_timestamp, before_aogain_timestamp, after_aogain_timestamp, end_aogain_timestamp, filename=filename

print, 'data saved in file: ',filename

end

