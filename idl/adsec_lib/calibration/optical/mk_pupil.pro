;Function to make a pupil:
;xc, yc: center coordinates(pixels)
;r: radius lengths (pixels)
;eps: central obstruction in fraction of radius

Function mk_pupil, xc, yc, r0, r1, imgx, imgy, EPS=eps, AR=aspect_ratio, SMOOTH=smooth

    if n_elements(imgx) eq 0 then imgx = 2*max([r0,r1])
    if n_elements(imgy) eq 0 then imgy = 2*max([r0,r1])
    if n_elements(aspect_ratio) eq 0 then aspect_ratio=1d
    if r0 ne r1 then aspect_ratio = double(r1)/double(r0)

    if double(aspect_ratio) eq 1d then begin
        img = dist(imgx, imgy)
        imgs = fshift(img, xc, yc)
    endif else begin

        vx = dist(imgx, 1)
        vy = dist(imgy, 1)
        xx = rebin(vx, imgx, imgy)
        yy = rebin(vy/aspect_ratio, imgy, imgx)
        img = sqrt(xx^2+transpose(yy^2))
        ;img /= (max(img)/max(dist(imgx, imgy)))

        ;e' sbagliato lo sjift con l'aspect ratio
        imgs = fshift(img, xc, yc)
    endelse



    if n_elements(eps) eq 0 then eps=0d
    return, (imgs lt r0) and (imgs gt abs(eps)*r0)

End
