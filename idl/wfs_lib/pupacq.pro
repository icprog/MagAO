; pupacq, 'test2_slopes.fits', 'test2_frame.fits', INDPUP_OUT='indpup', slopex_out='slopex', slopey_out='slopey'
;
; Perform a pupil acquisition using a signal rms threshold.
;
; An opt.loop data in open loop (with the mirror kept flat) is needed.
;
; OUTDIR: where to save the resulting data
; USEPUP: pupils used in the opt. loop data save (optional, if not present, the wfs header is used)
; BINNING: binning (optional, if not present, the wfs header is used)
; THRESHOLD: signal threshold (optional). If not present, starts from 4e-7 and then asks interactively
; WAITKEY: wait for a keypress when done

pro pupacq, tracknum, OUTDIR = OUTDIR, USEPUP = USEPUP, BINNING = BINNING, THRESHOLD = THRESHOLD, WAITKEY = WAITKEY

dir = '/towerdata/adsec_data/'+strmid(tracknum, 0, 8)+'/'
pupdir = GETENV('ADOPT_ROOT')+'/calib/wfs/current/ccd39/LUTs'
slopes_file = dir + 'Data_'+tracknum+'/Slopes_'+tracknum+'.fits'
frames_file = dir + 'Data_'+tracknum+'/Frames_'+tracknum+'.fits'
wfs_file = dir + 'Data_'+tracknum+'/wfs.fits'

slopes = readfits(slopes_file)

frame_sequence = readfits(frames_file)
frame = total(float(frame_sequence),3) / n_elements(frame_sequence[0,0,*])
frame = fix(frame)

if keyword_set(USEPUP) then USEPUP = pupdir + USEPUP

if (not keyword_set(USEPUP)) or (not keyword_set(BINNING)) then begin
    wfs = readfits( wfs_file, hdr)
    if not keyword_set(USEPUP) then USEPUP = pupdir + get_fits_keyword( hdr, 'sc.PUPILS')
    if not keyword_set(BINNING) then BINNING = fix(get_fits_keyword(hdr, 'ccd39.BINNING'))
endif

;USEPUP = pupdir + '/bin1/00big_20090626-144840'

pup = read_ascii(USEPUP+'/slopex')
tx = pup.field1

indpup = read_ascii(USEPUP+'/indpup')
indpup = indpup.field1

pupdata_file = USEPUP+'/pupdata.txt'

n_subap = n_elements(indpup)/4
intensity = fltarr(n_subap)
for i=0,n_subap-1 do begin
	intensity[i] =     frame[ indpup[i + n_subap*0]] + $
                       frame[ indpup[i + n_subap*1]] + $
                       frame[ indpup[i + n_subap*2]] + $
                       frame[ indpup[i + n_subap*3]]
endfor


;n_slopes = n_elements(slopes[*,0])
n_slopes = n_elements(intensity)*2

; Recalc slopes
n_frames = n_elements(frame_sequence[0,0,*])
slopes = fltarr(n_slopes, n_frames)
for i=0,n_frames-1 do begin
    f = frame_sequence(*,*,i)
    sx = (f[ indpup[n_subap*0:n_subap*1-1]] + f[ indpup[n_subap*1:n_subap*2-1]]) - $
         (f[ indpup[n_subap*2:n_subap*3-1]] + f[ indpup[n_subap*3:n_subap*4-1]])
    sy = (f[ indpup[n_subap*0:n_subap*1-1]] + f[ indpup[n_subap*2:n_subap*3-1]]) - $
         (f[ indpup[n_subap*1:n_subap*2-1]] + f[ indpup[n_subap*3:n_subap*4-1]]) 
    sx /= (intensity / n_subap)
    sy /= (intensity / n_subap)
    slopes[0:*:2, i] = sx
    slopes[1:*:2, i] = sy
endfor


avg = fltarr(n_slopes)
for i=0,n_slopes-1 do begin
	if i mod 2 eq 0 then avg[i] = intensity[i/2]
	if i mod 2 eq 1 then avg[i] = intensity[i/2]
endfor

dev = fltarr(n_slopes)
for i=0,n_slopes-1 do dev[i] = stddev(slopes[i,*])

;devavg = dev/avg^2

devavg = max(avg)-avg

indx = where (tx ne -1)

dim = 80/BINNING
dim_rebin = dim*4

frame_dev = fltarr(dim,dim)
frame_dev[indx] = dev[tx[indx]]

frame_avg = fltarr(dim,dim)
frame_avg[indx] = avg[tx[indx]]

frame_devavg = fltarr(dim,dim)
frame_devavg[indx] = devavg[tx[indx]]

if not keyword_set(THRESHOLD) then soglia = 4e-7 $
else soglia = THRESHOLD

while 1 do begin   ; interactive loop



window,0,retain=2, xsize=dim_rebin, ysize=dim_rebin
tvscl,rebin(frame_dev, dim_rebin,dim_rebin,/SAMPLE)

window,1,retain=2, xsize=dim_rebin, ysize=dim_rebin
tvscl,rebin(frame_avg, dim_rebin,dim_rebin,/SAMPLE)

window,2,retain=2, xsize=dim_rebin, ysize=dim_rebin
tvscl,rebin(frame_devavg, dim_rebin,dim_rebin,/SAMPLE)


; Mostra il cut

cuty = mean(indx / dim)
print,'Cuty=',cuty
plotsize = dim_rebin*2
if plotsize lt 400 then plotsize = 400
window,4,retain=2, xsize=plotsize, ysize=plotsize
mm = minmax(frame_devavg[ where(frame_devavg)])
plot,frame_devavg[*,cuty],psym=-4, yrange=mm, charsize=2
oplot,[0,150],[soglia,soglia],color=255L

; Esegui il cut

tocut = where(frame_devavg gt soglia, count)   ; Slope index in slopex

; Mostra il risultato sulle slopes
newframe_devavg = frame_devavg
if count gt 0 then newframe_devavg[tocut] = 0
window,3,retain=2, xsize=dim_rebin, ysize=dim_rebin
tv,bytscl( rebin(newframe_devavg, dim_rebin,dim_rebin,/SAMPLE))

newindpup = indpup
if count gt 0 then begin
    tocut_nslope = tx[tocut]		; Slope numbers to cut (x and y)

    for i=0,n_elements(tocut_nslope)-1 do begin	        ; Cut away from indpup
	    subap = tocut_nslope[i]/2
	    newindpup[ subap + n_subap*0]=0
	    newindpup[ subap + n_subap*1]=0
	    newindpup[ subap + n_subap*2]=0
	    newindpup[ subap + n_subap*3]=0
    endfor
endif

indpup_indx = where(newindpup ne 0, count)
if count lt 1 then begin
    print,'Problem: no subapertures selected!!!!'
    return
endif


newindpup = newindpup[indpup_indx]

newsubaps = n_elements(newindpup)/4
print,'New subap no.',newsubaps

; Indpup now only has the good subapertures
newframe = fltarr(dim,dim)
newframe[newindpup] = 1
window,5,retain=2, xsize=dim_rebin, ysize=dim_rebin
tvscl,rebin(newframe, dim_rebin, dim_rebin, /SAMPLE)

if not keyword_set(THRESHOLD) then begin
	print, 'Soglia attuale: ',soglia
	read, soglia, prompt = "Nuova soglia (0 to accept)"
	if soglia eq 0 then break
endif else begin
	print, newsubaps
	break
endelse

endwhile

if keyword_set(OUTDIR) then begin

    pup = newindpup[ newsubaps*0 : newsubaps*1-1]
    writefits,OUTDIR + '/pup1.fits', pup

    pup = newindpup[ newsubaps*1 : newsubaps*2-1]
    writefits,OUTDIR + '/pup2.fits', pup

    ; These two are inverted because in makePupils.generateLUT() pupils 3 and 4 end up inverted
    ; - and it must remain so.
    pup = newindpup[ newsubaps*2 : newsubaps*3-1]
    writefits,OUTDIR + '/pup4.fits', pup

    pup = newindpup[ newsubaps*3 : newsubaps*4-1]
    writefits,OUTDIR + '/pup3.fits', pup

    FILE_COPY, pupdata_file, OUTDIR+'/pupdata.txt', /OVERWRITE

endif 

if keyword_set(WAITKEY) then begin
        dummy = ''
	read, dummy, prompt='Press Enter to finish'
endif

end

