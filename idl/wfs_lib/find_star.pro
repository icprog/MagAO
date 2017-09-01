;
; Find one or more stars into a frame, using the find.pro procedure
;
; INPUT:
;
; fitsfile:   file .fits with the image
; threshold:  background luminosity threshold (anything lower than that is not counted)
; fwhm:       estimated FWHM of the star in pixels
;
; OUTPUT:
;
; nstars   :  no. of stars found
; x        :  x coordinate vector (one for each star)
; y        :  y coordinate vector (one for each star)
; flux     :  flux vector (one for each star)
; sharp    :  sharpness vector (one for each star)
; round    :  roundness vector (one for each star)
;
; Stars are ordered in decreasing flux value

pro find_star, fitsfile, threshold, fwhm, nstars, x, y, flux, sharp, round, USE_FIT = USE_FIT, FIT_TH = FIT_TH

    image = readfits(fitsfile)

    if keyword_set(USE_FIT) then begin
        fit = gauss2dfit(smooth(image,2),par)
        flux = [par[1]]
        x = [par[4]]
        y = [par[5]]
        print, flux, x, y, fit_th
        if flux gt fit_th then begin
            nstars=1
            sharp=[1]
            round=[1]
        endif else begin
            nstars=0
        endelse
        return
    endif

  
    nstars = 0
    x = -1 
 
    find, image, x, y, flux, sharp, round, threshold, fwhm, [-1.0,1.0], [0.2,1.0], /SILENT

    if n_elements(x) eq 1 then begin
	if x[0] eq -1 then begin
		nstars=0
		return
	endif
    endif

    nstars = n_elements(x)
    s = reverse(sort(flux))
    x = x[s]
    y = y[s]
    flux = flux[s]
    sharp = sharp[s]
    round = round[s]

end

