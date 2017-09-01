

pro psf_focus, tracknum, n_images, peak, w, CX = CX, CY = CY, MASK = MASK

path = getenv('ADOPT_MEAS')+'/autoFocus47/'+tracknum

p = fltarr(n_images)
w = fltarr(n_images)
z = fltarr(n_images)

for i=0,n_images-1 do begin

    filename = path+'/zpos_'+strtrim(i,2)+'.fits' 

    if not keyword_set(cx) then begin
        a = readfits(filename, hdr)
        mm = max(a, pos)
        cx = pos mod 1024
        cy = pos / 1024
    endif

    
    psf_fwhm, filename,  FWHM = FWHM, ENERGY = ENERGY, MASK = MASK, CX = CX, CY=CY, PEAK = PEAK

    z[i] = float( get_fits_keyword( hdr, 'stagez.POSITION'))
    p[i] = peak
    w[i] = FWHM

    print, 'Immagine ',i,': FWHM=', fwhm,'  PEAK = ',peak


endfor

z = findgen(n_images)

window,0, retain=2
plot,z,p, psym=-4

window,1, retain=2
plot,z,w, psym=-4

pos_p = where(p eq max(p))
pos_w = where(w eq min(w))

peak = mean([pos_p, pos_w])

end
