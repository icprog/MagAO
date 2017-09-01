
; Applies the pupilfit algorithm to a single image

function wfs_pupilfit_single, img, binning, QUIET = QUIET

    dx = n_elements(img[*,0])
    dy = n_elements(img[0,*])
    guess_cx = dx/2.0
    guess_cy = dy/2.0
    guess_rad = 15.0 / binning

    if keyword_set(QUIET) then DISPLAY =0 $
    else DISPLAY = 1
    par = pupilfit( img , GUESS=[ guess_cx, guess_cy, guess_rad, guess_rad, 0], SCALE=[100,100,100,100,0.5], mask = mask, DISPLAY = DISPLAY)

    return, par
end

function dist2d, x1, y1, x2, y2
    return, sqrt( (x1-x2)*(x1-x2) + (y1-y2)*(y1-y2))
end

; Pupil fitting (WFS version).
;
; Fits a pupil using the IDL amoeba algorithm
;
; filename: fits file with the input image
; OUTFILE: if set, txt file where to save the result
; SPLIT  : if set, compute four pupils, otherwise only one.
; QUIET  : if set, do not display anything during computation.

pro wfs_pupilfit, filename, binning, OUTFILE = OUTFILE, SPLIT = SPLIT, QUIET=QUIET

    img = readfits(filename)

    if keyword_set(SPLIT) then begin

        dx = n_elements(img[*,0])
        dy = n_elements(img[0,*])

        par = fltarr(5,4)
        side = fltarr(4)
        par[*,0] = wfs_pupilfit_single( img[0:dx/2-1, 0:dy/2-1], binning, QUIET = QUIET)    
        par[*,1] = wfs_pupilfit_single( img[0:dx/2-1, dy/2:*], binning, QUIET = QUIET)    
        par[*,2] = wfs_pupilfit_single( img[dx/2:*, dy/2:*], binning, QUIET = QUIET)    
        par[*,3] = wfs_pupilfit_single( img[dx/2:*, 0:dy/2-1], binning, QUIET = QUIET)    

        par[1,1] += dy/2
        par[1,2] += dy/2
        par[0,2] += dx/2
        par[0,3] += dx/2

        side[0] = dist2d( par[0,0], par[1,0], par[0,1], par[1,1])
        side[1] = dist2d( par[0,1], par[1,1], par[0,2], par[1,2])
        side[2] = dist2d( par[0,2], par[1,2], par[0,3], par[1,3])
        side[3] = dist2d( par[0,3], par[1,3], par[0,0], par[1,0])
    
        if keyword_set(OUTFILE) then begin
            reorder = [1,2,0,3]
            openw, unit, OUTFILE, /GET_LUN
            for i=0,3 do begin
                ii = reorder[i]
                printf, unit, par[0,ii], par[1,ii], par[2,ii], par[3,ii], par[4,ii], side[ii]
            endfor
            close, unit
            free_lun, unit
        endif

    endif else begin

        par = wfs_pupilfit_single( img, binning, QUIET = QUIET)

        if keyword_set(OUTFILE) then begin
            openw, unit, OUTFILE, /GET_LUN
            printf, unit, par[0], par[1], par[2], par[3], par[4], 0
            close, unit
            free_lun, unit
        endif
    endelse

end
