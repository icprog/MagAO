
pro fwhm_multi, dark, prefix, first, last, 

    for i=first,last do begin

        filename = prefix+strtrim(i,2)+'.fits'
        fwhm_single, filename, dark

    endfor

pro fwhm_single, filename, dark, fwhm_h, fwhm_w


    img = readfits(filename)
    darkimg = readfits(dark)
    img -= darkimg

w = 50

window, 0, retain=2
plot,[0,50],[0,0],yrange=[-1000,13000], /ystyle, background= 'FFFFFF'XL, color=0
window, 1, retain=2
plot,[0,50],[0,0],yrange=[-1000,13000], /ystyle, background = 'FFFFFF'XL, color=0

for i=0,n_elements(files)-1 do begin


    img = readfits(files[i]) - readfits(dark)

    peak = max(img)
    pos_peak = where( img eq peak)
    dx = n_elements(img[*,0])
    dy = n_elements(img[0,*])
    x = pos_peak mod dx
    y = pos_peak / dx

    line_h = img[x-w/2:x+w/2,y]
    line_w = img[x,y-w/2:y+w/2]

    wset, 0
    oplot, line_h, psym=-4, color= colors[i]
    fwhm_h[i] = calc_fwhm( line_h)
   
    wset,1 
    oplot, line_w, psym=-4, color = colors[i]
    fwhm_w[i] = calc_fwhm( line_w)

    print, files[i], fwhm_h[i], fwhm_w[i]

    ; Gaussfit

    yfit = gauss2dfit( img[x-w/2:x+w/2, y-w/2:y+w/2], coeff, /TILT) 

    wset, 0
    oplot, yfit[*,w/2], psym=-4, color = 255L

    wset, 1
    oplot, yfit[w/2,*], psym=-4, color = 255L

    print, coeff
    stop
    

endfor

end



function calc_fwhm, psf_profile

    peak = max(psf_profile)
    hm   = peak/2.0

    ; Cerca la prima intersezione
    p=0
    while psf_profile[p] le hm do p +=1

    x1 = float(p)-1
    x2 = float(p)
    y1 = float(psf_profile[x1])
    y2 = float(psf_profile[x2])
    y = float(hm)

    x_prev = (y-y1)/(y2-y1)*(x2-x1) + x1
   
    ; Cerca la seconda intersezione 
    while psf_profile[p] ge hm do p +=1

    x1 = float(p-1)
    x2 = float(p)
    y1 = float(psf_profile[x1])
    y2 = float(psf_profile[x2])
    y = float(hm)

    x_next = (y-y1)/(y2-y1)*(x2-x1) + x1
  
    return, x_next - x_prev
end 




