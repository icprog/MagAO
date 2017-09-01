
pro processMVR, tracknum, PREFIX=PREFIX

	if not keyword_set(PREFIX) then PREFIX='' $
        else PREFIX += '/'

	n_steps = 15
        step_size = 22.5

        positions = findgen(n_steps)*step_size		
	sx = fltarr(n_steps)
	sy = fltarr(n_steps)
        modulo = fltarr(n_steps)

	k = 45*800e-9 * 1e6	; Raggio della PSF in micron

        for i=0,n_steps-1 do begin

		filename = PREFIX + tracknum+'/ccd39_fr'+strtrim(i,2)+'.fits'
		img = readfits(filename)
		w = n_elements(img[*,0])
		h = n_elements(img[0,*])
                soglia = median(img[0:5,0:5])
		
		f1 = img[0:w/2-1, 0:h/2-1]
		f2 = img[w/2:*, 0:h/2-1]
		f3 = img[0:w/2-1, h/2:*]
		f4 = img[w/2:*, h/2:*]
		f1 = mean( f1[where(f1 gt soglia)])
		f2 = mean( f2[where(f2 gt soglia)])
		f3 = mean( f3[where(f3 gt soglia)])
		f4 = mean( f4[where(f4 gt soglia)])
	
		sx[i] = ((f2+f4)-(f1+f3)) /(f1+f2+f3+f4) 
		sy[i] = ((f1+f2)-(f3+f4))/(f1+f2+f3+f4) 
                modulo[i] = sqrt(sx[i]^2 + sy[i]^2)

	endfor


	window,0,retain=2
	plot, modulo, psym=-4, title='Tip-tilt signal', yrange = minmax([sx, sy, modulo])
	oplot, sx
	oplot, sy

	window,1,retain=2
	plot,sx*k,sy*k, /ISO, title="PSF movement", ytitle="micron", xtitle="micron"
	
end


