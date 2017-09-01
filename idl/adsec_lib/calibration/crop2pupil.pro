Function crop2pupil, img, xyre, SAMPLING=sampling, MASK=mask

    if n_elements(xyre) eq 0 then xyre =  pupilfit(img, guess = [250, 250, 250, 0], /circle)

    x0 = xyre[0]
    y0 = xyre[1]
    r  = xyre[2]

    img1 = img[floor(x0-r):ceil(x0+r), floor(y0-r):ceil(y0+r)]
    simg1 = size(img1, /DIM)
    
    if n_elements(sampling) eq 0 then retimg = img1 $
    else begin
        sx = sampling[0]
        sy = sampling[1]
        x0n = simg1[0]/2.
        y0n = simg1[1]/2.
        rn  = simg1[1]/2.
        xx  = mk_vector(sx, 0, 2*rn)
        yy  = mk_vector(sy, 0, 2*rn)
        retimg= interpolate(img1, xx, yy, /GRID)
    endelse

    if keyword_set(MASK) then return, float(retimg eq 1) else return, retimg

End
