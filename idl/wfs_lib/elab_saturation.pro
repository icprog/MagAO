
; modal_basis = 'KL' or 'mirrorStandard'
; Track_list: array[] of tracking numbers

pro elab_saturation, modal_basis, track_list

; KL

if modal_basis eq 'KL' then begin

; 6 lambda/D
;track_list = ['20090916_173138', '20090916_173221', '20090916_173309', '20090916_174222', '20090916_174257', $
;              '20090916_174324', '20090916_174356', '20090916_174439', '20090916_174528']

; 2 lambda/D

track_list = ['20090917_114240', '20090917_114403', '20090917_114637', '20090917_114842', '20090917_114935', $
              '20090917_115137', '20090917_115317', '20090917_115457', '20090917_115646']

endif

if modal_basis eq 'mirrorStandard' then begin

; mirrorStandard

track_list = ['20090916_174827', '20090916_174921', '20090916_174957', '20090916_175130', '20090916_175213', $
              '20090916_175340', '20090916_175416', '20090916_175528']
;'20090916_175503'

endif

n_mat = n_elements(track_list)

score = fltarr(672, n_mat)
amps = fltarr(672, n_mat)

for i=0,n_mat-1 do begin

    path = '/towerdata/adsec_calib/M2C/'+modal_basis+'/RECs/Intmat_'+track_list[i]+'.fits_saturation.sav'

    print, path
    restore,path,/V

    ; Restore missing data
    in_threshold = -(under_sig_min + over_sig_max) + 1.0

    score[*, i] = in_threshold
    amps[*, i] = amp_modes

    window,i,retain=2
    plot,in_threshold,psym=-4,yrange=[0,1], /ystyle, title="Raw signal saturation", charsize=2
    oplot,over_sig_max,color=255L
    oplot,under_sig_min,color=255L*256L*256L


end

selected = fltarr(672)
amp = fltarr(672)
for i=0,671 do begin
    mm = max(score[i,*], pos)
    selected[i] = mm
    amp[i] = amps[i,pos]
endfor

window,10,retain=2
plot, selected, yrange=[0,1.0], /ystyle, charsize=2
stop

end

