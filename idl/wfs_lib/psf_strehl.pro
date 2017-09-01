pro psf_strehl, img1, img2, MASK = MASK

psf_fwhm, img1, fwhm = f1, energy = e1, MASK = MASK
psf_fwhm, img2, fwhm = f2, energy = e2, MASK = MASK

; Riordina per avere f1 piu' grande
if f2 gt f1 then begin
    temp = f2
    f2 = f1
    f1 = temp
endif
    
f_res2 = f1^2-f2^2

sr = 1.0/ (1.0 + f_res2)
sigma2 = -alog(sr)
sigma2_h = sigma2*(850.0/1650.0)^2
sigma2_k = sigma2*(850.0/2200.0)^2
sr_h = exp(-sigma2_h)
sr_k = exp(-sigma2_k)
print,'SR:i @ 850 ',sr
print,'SR:i @ 1650 ',sr_h
print,'SR:i @ 2200 ',sr_k
print,'E1/E2: ', e1/e2

end




