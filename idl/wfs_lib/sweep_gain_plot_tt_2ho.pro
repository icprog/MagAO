 
pro sweep_gain_plot_tt_2ho, GAINSFILE = GAINSFILE, SLOPESFILE = SLOPESFILE, GAINSDIR = GAINSDIR, GAINV=GAINV, GAINTT = GAINTT, GAINHO1 = GAINHO1, GAINHO2 = GAINHO2, RMSMIN = RMSMIN, NOSAVE = NOSAVE, TARGET = TARGET, MIDDLE_HO = MIDDLE_HO, QUIET = QUIET

if not keyword_set(GAINSFILE) then GAINSFILE = 'gains.fits'
if not keyword_set(SLOPESFILE) then SLOPESFILE = 'slopes.fits'
if not keyword_set(GAINSDIR) then GAINSDIR = '/towerdata/adsec_calib/M2C/KL_v7/gain'

;1) caricare il ricostruttore in uso
;2) misurare le slopes ai vari gain
;3) calcolare la decomposizione modale residua dalle slopes
;4) calcolarsi il yilt rms dai coefficiento di tip e tilt  sqrt(t12+T22)
;5) calcolarsi o rm dei modi corretti residui sqrt(sum_1_n mode_i2) 


gains = readfits( GAINSFILE, hdr)
slopes = readfits( SLOPESFILE)

cycles = get_fits_keyword(hdr,'CYCLES')
steps_ramp = get_fits_keyword(hdr, 'STEPS_RAMP')
modalbasis = get_fits_keyword(hdr, 'M2C')

idx = where(slopes[*,0,0] ne 0)
n_frames = n_elements(slopes[0,*,0])
n_steps  = n_elements(slopes[0,0,*])
; Carica il rec e calcola i modi residui
recfile = "/towerdata/adsec_calib/M2C/"+get_fits_keyword(hdr, 'REC')
rec = readfits(recfile, hhh)
modesi = fltarr(n_elements(rec[0,*]), n_frames, n_steps)
for i=0,n_steps-1 do modesi[*,*,i] = transpose(rec ## transpose(reform(slopes[*,*,i])))
n_modes = long(get_fits_keyword(hhh, 'IM_MODES'))
modes = modesi[0:n_modes-1, *,*]

ratio = gains[1,0] / gains[0,0]
; Detect if tt or ho were used
mmtt = minmax(gains[0,*])
mmho1 = minmax(gains[1,*])
mmho2 = minmax(gains[2,*])

nn = max(idx)
rms = fltarr(n_frames, n_steps)
ttrms = fltarr(n_frames, n_steps)
horms = fltarr(n_frames, n_steps)
horms1 = fltarr(n_frames, n_steps)
horms2 = fltarr(n_frames, n_steps)

; Calcola rms dei modi residui (generale, solo tip-tilt, modi alti)

if MIDDLE_HO gt n_modes then MIDDLE_HO = n_modes

for i=0,n_steps-1 do for j=0,n_frames-1 do rms[j,i]   = sqrt(total(modes[*,j,i]^2))
for i=0,n_steps-1 do for j=0,n_frames-1 do ttrms[j,i] = sqrt(total(modes[0:1,j,i]^2))
for i=0,n_steps-1 do for j=0,n_frames-1 do horms[j,i] = sqrt( total(modes[2:*,j,i]^2))
for i=0,n_steps-1 do for j=0,n_frames-1 do horms1[j,i] = sqrt( total(modes[2:MIDDLE_HO-1,j,i]^2))
if MIDDLE_HO lt n_modes then begin
    for i=0,n_steps-1 do for j=0,n_frames-1 do horms2[j,i] = sqrt( total(modes[MIDDLE_HO:*,j,i]^2))
endif
    

; Seleziona quale usare come funzione di merito
if (TARGET eq 'tt') then use_rms = ttrms	
if (TARGET eq 'ho1') then use_rms = horms1
if (TARGET eq 'ho2') then use_rms = horms2
if (TARGET eq 'both') then use_rms = horms
if (TARGET eq 'all') then use_rms = rms

if (TARGET eq 'tt') then usegain = gains[0,*]	
if (TARGET eq 'ho1') then usegain = gains[1,*]
if (TARGET eq 'ho2') then usegain = gains[2,*]
if (TARGET eq 'both') then usegain = gains[1,*]
if (TARGET eq 'all') then usegain = gains[0,*]

rmst = total(use_rms,1)/n_frames
;rmst = smooth(rmst,2)
x = usegain

;; Catch plot errors in case X display is not set
catch, error_plot

if not keyword_set(QUIET) then begin
    if error_plot eq 0 then begin
       window,1,retain=2
       plot,x, rmst,psym=4,/ystyle,yrange=[0,0.5e-7]
    endif else begin
       catch, /cancel
    endelse
    ;for i=0,n_steps-1 do xyouts, usegain[i], rmst[i]+0.01, strtrim(i,2) ; debug output
endif

;LB 120916 la mediana e' piu' robusta della media
;rms_avg = fltarr(steps_ramp)
rms_avg_v = fltarr(cycles, steps_ramp)

for cycle=0,cycles-1 do begin
	for step=0,steps_ramp-1 do begin
		;rms_avg[step] += rmst[ cycle*steps_ramp + step]
		rms_avg_v[cycle,step] = rmst[ cycle*steps_ramp + step]
	endfor
endfor
;rms_avg /= cycles
rms_avg = median(rms_avg_v, dim=1)
;LB end


if (not keyword_set(QUIET)) and (error_plot eq 0) then oplot, x, rms_avg, color=255L, thick=2

xmin = min(x)
xmax = max(x)
npoints=100
xc = findgen(npoints)/npoints * (xmax-xmin)+xmin
rms_avg_extended=interpol(rms_avg, npoints)
curve = smooth(rms_avg_extended, 5, /edge_truncate)
if (not keyword_set(QUIET)) and (error_plot eq 0) then oplot, xc, curve, color=255L*256*256, thick=2

mm = where(curve eq min(curve), count)
;mm = mm[count-1]
nn = n_elements(curve)
if curve[mm] gt curve[nn-1]*0.85 then mm = nn-1

; Recalc gain array
gg = fltarr(n_elements(gains[*,0]), npoints)
for i=0, n_elements(gains[*,0])-1 do begin
    gmin = min(gains[i,*])
    gmax = max(gains[i,*])
    gg[i,*] = findgen(npoints)/npoints * (gmax-gmin)+gmin
endfor

gaintt = gg[0,mm]
gainho1 = gg[1,mm]
gainho2 = gg[2,mm]


if not keyword_set(QUIET) then print, 'Minimum found at step: ', mm, ' rms: ',rmst
if not keyword_set(QUIET) then print, 'gains: ', gaintt, gainho1, gainho2

gainv = fltarr(672)
gainv[0:1] = gaintt
gainv[2:MIDDLE_HO-1] = gainho1
gainv[MIDDLE_HO:*] = gainho2
if not keyword_set(NOSAVE) then writefits, GAINSDIR + path_sep() + 'gain_sweep.fits', float(gainv)
end

