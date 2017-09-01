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

Function circle_merit, p

    common data_pupilfit_block, data_pupilfit
    common display_block,fit_display
    sdata = size(data_pupilfit, /dim)

    pupil = mk_pupil(p[0],p[1],p[2],p[2],sdata[0],sdata[1], EPS=p[3])
    if fit_display then begin
        tvscl, data_pupilfit + 10*mean(data_pupilfit)*pupil
        print, "Merit Fcn:"+strtrim(1/(correlate(data_pupilfit, pupil, /COVAR)),2)
    endif
    ;return, 1/(correlate(data, pupil, /COVAR))
    if correlate(data_pupilfit,pupil, /COVAR) eq 0 then return, 1e6 else $
       return, abs(1d/correlate(data_pupilfit, pupil, /COVAR))

End


Function ellipse_merit, p

    common data_pupilfit_block, data_pupilfit
    common display_block,fit_display
    sdata = size(data_pupilfit, /dim)

    pupil = mk_pupil(p[0],p[1],p[2],p[3],sdata[0],sdata[1], EPS=p[4])
    if fit_display then begin
        tvscl, data_pupilfit + 10*mean(data_pupilfit)*pupil
        print, "Merit Fcn:"+strtrim(1/(correlate(data_pupilfit, pupil, /COVAR)),2)
    endif
    ;return, 1/(correlate(data, pupil, /COVAR))
    if correlate(data_pupilfit,pupil, /COVAR) eq 0 then return, 1e6 else $
       return, abs(1d/correlate(data_pupilfit, pupil, /COVAR))

End


Function pupilfit, image, GUESS=guess, SCALE=scale, DISPLAY=display, MASK=mask, CIRCLE=circle

    common data_pupilfit_block, data_pupilfit
    common display_block, fit_display

    fit_display = keyword_set(display)
    if fit_display then begin
        window, /free, xs = (size(image,/DIM))[0], ys = (size(image,/DIM))[1]
    endif

    ;image noise levelling
    box = image[0:10, 0:10]
    noise = mean(box)
    data_pupilfit = ((image-noise)^2)

    ;data_pupilfit = data_pupilfit[*]

    t0 = systime(/sec)
    sdata = size(data_pupilfit, /dim)
    if keyword_set(CIRCLE) then begin

        if n_elements(GUESS) gt 0 then begin
            par = guess
        endif else begin
            par = dblarr(4)
            par[0] = 496
            par[1] = 499
            par[2] = 435
            par[3] = 0.1
        endelse
        if n_elements(SCALE) eq 0 then SCALE=[100,100,100,0.5]
        yfit = amoeba((machar()).eps, FUNCTION_NAME='circle_merit', NMAX=2000, P0=par, SCALE=scale )
        mask =  mk_pupil(yfit[0],yfit[1],yfit[2],yfit[2],sdata[0],sdata[1], EPS=yfit[3]) gt 0.5d

    endif else begin
        if n_elements(GUESS) gt 0 then begin
            par = guess
        endif else begin
            par = dblarr(5)
            par[0] = 350
            par[1] = 240
            par[2] = 200
            par[3] = 200
            par[4] = 0.3
        endelse
        if n_elements(scale) eq 0 then SCALE=[100,100,100,100,0.5]
        yfit = amoeba((machar()).eps, FUNCTION_NAME='ellipse_merit', NMAX=2000, P0=par, SCALE=scale )
        mask =  mk_pupil(yfit[0],yfit[1],yfit[2],yfit[3],sdata[0],sdata[1], EPS=yfit[4]) gt 0.5d
    endelse


    dt = systime(/sec)-t0
    print, "Ellapsed time: " + strtrim(dt, 2)



    return,yfit

End

