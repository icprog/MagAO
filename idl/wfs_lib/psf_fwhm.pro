function psfmax, imasmall, FWHM = FWHM, ENERGY = ENERGY, PEAK = PEAK
    res = gauss2dfit(imasmall,a, /TILT)

    window_w = n_elements(imasmall[*,0])
    px = a[4]
    py = a[5]

    if (px lt 0) or (py lt 0) or (px ge n_elements(imasmall[*,0])) or (py ge n_elements(imasmall[0,*])) then begin
        FWHM = 0
        ENERGY =1
        PEAK = 1
        return, 0
    endif

    ;psf_energy = total(imasmall-a[0])
    psf_energy = total(res - a[0])

    aave = (a(2) + a(3))/2 * 2.35
    window,0, xs=600, retain=2
    plot_io, imasmall(px,*)+1, psym=-4
    oplot, res(px,*), color = 255, psym=-2
    window,1, xs=600, retain=2
    tvscl, rebin(imasmall, window_w*2, window_w*2); alog10(imasmall)

    print, 'results of fitting'
    print, a
    print, 'averaged FWHM of airy PSF....', aave,' px '
    aave_ms = (aave*13.)/( 45*0.80)
    print, 'FWHM in lambda/D: ', aave_ms
    psfmax = a[1]/psf_energy
    print, 'Peak: ', a[1], '  -  Peak/energy:', psfmax
    print,''

    FWHM = aave_ms
    ENERGY =  (a[1] / psf_energy) * FWHM^2
    PEAK = a[1]
    print, 'Total energy:', ENERGY
    return, psfmax
end

pro psf_fwhm, filename, WINDOW=WINDOW, FWHM = FWHM, ENERGY = ENERGY, MASK = MASK, CX = CX, CY=CY, PEAK = PEAK
window, xs=512,ys=512, retain = 2

if not keyword_set(MASK) then MASK = 200
if not keyword_set(CX) then cx = 420
if not keyword_set(CY) then cy = 510

ima = float(readfits(filename))
s = size(ima)
if s[0] eq 3 then s = total(s,3) / s[3]
if keyword_set(MASK) then begin
    ima = ima[cx-MASK/2:cx+MASK/2-1, cy-MASK/2:cy+MASK/2-1]
endif

ima_max = max(ima, h)
width = n_elements(ima[*,0])
xc = h mod width
yc = h / width

mm = psfmax(ima, FWHM = FWHM, ENERGY = ENERGY, PEAK = PEAK)


print,energy

end

