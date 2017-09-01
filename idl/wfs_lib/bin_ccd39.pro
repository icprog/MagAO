;
; bin CCD39 frame.
; binning can be 1,2,3,4,5
; output is 80x80, 40x40, 26x26, 20x20, 16x16
; 
; In the binning=3 case, the first external column and row of each quadrant is discarded
;

function bin_ccd39, frame, binning, INPUT_FILE = INPUT_FILE, OUTPUT_FILE = OUTPUT_FILE

  if keyword_set(INPUT_FILE) then frame = readfits(INPUT_FILE)

  if ( (size(frame))[1] ne 80 ) or ( (size(frame))[2] ne 80 ) then begin
    print, 'CCD39 must be 80x80'
    return, -1L
  endif

  if (binning eq 3) then begin
    quadsz = 39
    tobin = frame[1:78, 1:78]
  endif else begin
    quadsz = 40
    tobin = frame
  endelse
  
  quad1 = tobin[0:quadsz-1,        0:quadsz-1] 
  quad2 = rotate(tobin[quadsz:2*quadsz-1, 0:quadsz-1]       ,3) 
  quad3 = rotate(tobin[0:quadsz-1,        quadsz:2*quadsz-1],1) 
  quad4 = rotate(tobin[quadsz:2*quadsz-1, quadsz:2*quadsz-1],2)  
  
  quad1r = long(rebin(float(quad1), quadsz/binning, quadsz/binning) * binning^2) 
  quad2r = long(rebin(float(quad2), quadsz/binning, quadsz/binning) * binning^2) 
  quad3r = long(rebin(float(quad3), quadsz/binning, quadsz/binning) * binning^2) 
  quad4r = long(rebin(float(quad4), quadsz/binning, quadsz/binning) * binning^2) 
  
  qbsz = quadsz/binning
  binned = lonarr(2*qbsz,2*qbsz)
  binned[0:qbsz-1,        0:qbsz-1]      = quad1r
  binned[qbsz:2*qbsz-1,   0:qbsz-1]      = rotate(quad2r,1)
  binned[0:qbsz-1,        qbsz:2*qbsz-1] = rotate(quad3r,3)
  binned[qbsz:2*qbsz-1,   qbsz:2*qbsz-1] = rotate(quad4r,2)
  
  if keyword_set(OUTPUT_FILE) then writefits, OUTPUT_FILE ,binned
  
  return, binned
end

pro bintest

a = read_ascii('indpup')
frame = fltarr(80,80)
indpup = a.field1 

; Scommenta queste per avere lato=37 invece di 36
;indpup[0:703] += 0
;indpup[704:1407] += 1
;indpup[704*2:704*3-1] += 80+1
;indpup[704*3:*] += 80

frame[indpup] = 1
f2 = bin_ccd39(frame,2)
f3 = bin_ccd39(frame,3)
f4 = bin_ccd39(frame,4)
window,0,xsize=320*4,ysize=320*2,retain=2
tvscl,rebin(frame,320,320,/SAMPLE),0
tvscl,rebin(f2,320,320,/SAMPLE),1
tvscl,rebin(f3,312,312,/SAMPLE),2
tvscl,rebin(f4,320,320,/SAMPLE),3

frame = shift(frame,-3,-3)
f2 = bin_ccd39(frame,2)
f3 = bin_ccd39(frame,3)
f4 = bin_ccd39(frame,4)
tvscl,rebin(frame,320,320,/SAMPLE),4
tvscl,rebin(f2,320,320,/SAMPLE),5
tvscl,rebin(f3,312,312,/SAMPLE),6
tvscl,rebin(f4,320,320,/SAMPLE),7

end


