
pro fwhm_multi, dark, prefix, first, last, min_fwhm, xcorr, ycorr

    fwhm = fltarr(last-first+1)
    posx = fltarr(last-first+1)
    posy = fltarr(last-first+1)

    for i=first,last do begin

        filename = prefix+strtrim(i,2)+'.fits'
        fwhm_single, filename, dark, coeff, fwhm_1, posx1, posy1

        fwhm[i-first] = fwhm_1
        posx[i-first] = posx1
        posy[i-first] = posy1
    endfor

    for i=first,last do begin
        print, 'PSF FWHM :', fwhm[i-first]
    endfor

    plot,fwhm,psym=-4

    print, 'Minimum FWHM:', min(fwhm)
    pos = where(fwhm eq min(fwhm))
    print, 'X,Y corrections: ', posx[pos], posy[pos]


    min_fwhm = min(fwhm)
    xcorr = xpos[pos]
    ycorr = ypos[pos]

end

pro fwhm_single, filename, dark, coeff, fwhm, xcorr, ycorr, DISPLAY = DISPLAY


    w = 50

    if keyword_set(DISPLAY) then begin
        window, 0, retain=2
        plot,[0,50],[0,0],yrange=[-1000,13000], /ystyle, background= 'FFFFFF'XL, color=0
        window, 1, retain=2
        plot,[0,50],[0,0],yrange=[-1000,13000], /ystyle, background= 'FFFFFF'XL, color=0
    endif

    img = readfits(filename) - readfits(dark)

    peak = max(img)
    pos_peak = where( img eq peak)
    dx = n_elements(img[*,0])
    dy = n_elements(img[0,*])
    x = pos_peak mod dx
    y = pos_peak / dx

    line_h = img[x-w/2:x+w/2,y]
    line_w = img[x,y-w/2:y+w/2]

    if keyword_set(DISPLAY) then begin
        wset, 0
        oplot, line_h, psym=-4, color= 0
   
        wset,1 
        oplot, line_w, psym=-4, color = 0
    endif

    ; Non servono, sostituite dal gaussfit
    ;fwhm_h = calc_fwhm( line_h)
    ;fwhm_w = calc_fwhm( line_w)
    ;print, fwhm_h, fwhm_w

    ; Gaussfit

    yfit = gauss2dfit( img[x-w/2:x+w/2, y-w/2:y+w/2], coeff, /TILT) 

    if keyword_set(DISPLAY) then begin
        wset, 0
        oplot, yfit[*,w/2], psym=-4, color = 255L
        wset, 1
        oplot, yfit[w/2,*], psym=-4, color = 255L
    endif

    print, coeff

    fwhm = (coeff[2]+coeff[3])/2 * sqrt(2*alog(2)) * 2

    xcorr = round(coeff[4]) - coeff[4]
    ycorr = round(coeff[5]) - coeff[5]

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




function airy_disk, npoints, lambda, diam, i0, gamma, DOSHIFT = DOSHIFT
pixel_scale = gamma*lambda/diam
pixel_scale1 = pixel_scale/ 10
npoints1 = npoints  *10
radius = shift(dist(npoints1),npoints1/2,npoints1/2)
x = 2 * !pi * diam/lambda * radius * pixel_scale1+1e-6

Result = (2*BESELJ(X, 1 ,/DOUBLE)/ x )^2 *i0
if keyword_set(DOSHIFT) then result = shift( result, DOSHIFT*10, 0)
result1 = rebin(result,npoints,npoints)
print, 'pixel scale => [arcsec]', pixel_scale/4.848e-6
return, result1
end
