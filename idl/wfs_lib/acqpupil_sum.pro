
pro acqpupil_sum, img_filename, dim, SPLIT=SPLIT, OUTFILE=OUTFILE, TH1=TH1, TH2=TH2, BINNING=BINNING

if not keyword_set(BINNING) then BINNING=1

immagine = float(readfits(img_filename))
dim_imm=size(immagine)

immagine[*,0] = 0
immagine[*,dim_imm[2]-1] =0
immagine[0,*] = 0
immagine[dim_imm[1]-1,*] =0

if keyword_set(SPLIT) then begin

    cx=fix(dim_imm[1]/2)
    cy=fix(dim_imm[2]/2)

    ;fix ccd39. Only reduce on the x axis.
    cx1 = cx-0/BINNING
    cy1 = cy-0/BINNING

    dx = cx1-cx
    dy = cy1-cy
    dim = min([cx-abs(dx), cy-abs(dy)])

    a = immagine[cx1-dim:cx1-1, cy1-dim:cy1-1]
    b = immagine[cx1:cx1+dim-1, cy1-dim:cy1-1]
    c = immagine[cx1-dim:cx1-1, cy1:cy1+dim-1]
    d = immagine[cx1:cx1+dim-1, cy1:cy1+dim-1]  

    side = 40 / BINNING
    ;stop
    ; Sum the four images with a shift

    a += shift( b, dim-side, 0)
    a += shift( c, 0, dim-side)
    a += shift( d, dim-side, dim-side)

    writefits, '/tmp/pippo4.fits', a

    analizza, a , m, SOGL = TH1, S2_P2V = TH2, /QUIET, /NOSAVE
 
    center = fltarr(4,2)
    diam = m[0]*2
    center[2,*] = m[1:2] + [cx1-dim, cy1-dim]
    center[1,*] = center[2,*] + [side, side]
    center[0,*] = center[2,*] + [0,side]
    center[3,*] = center[2,*] + [side,0]

    print,'Summed pupils:'
    print, 'diameter: ',diam
    print,center

    if keyword_set(OUTFILE) then begin
        openw, unit, OUTFILE, /GET_LUN
        for pup=0,3 do printf, unit, diam, center[pup,0], center[pup,1], 0.0
        close, unit
        free_lun, unit
    endif

endif 

end
