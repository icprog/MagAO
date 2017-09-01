;wv:[tx, ty, focus, cx, cy]
function hexalign, TRACK=track, APPLY=apply 

    @adsec_common
    
    if n_elements(track) eq 0 then begin
        flist=file_search(adsec_path.meas+'hexapod', 'hexapod_tf_mat.sav')
        file2restore=flist[n_elements(flist)-1]
    endif else begin
        file2restore=adsec_path.meas+"hexapod/"+track+"/hexapod_tf_mat.sav"
    endelse
    print, file2restore
    restore, file2restore
    verbose=1

    if n_elements(average) eq 0 then average = 1
    if average eq 1 then acquire4D = 1 else acquire4D=0
    if average gt 1 then averaged = 1 else averaged=0
    if acquire4d then begin
        err = idl_4d_init()
        name_radix = "wowowow"
        err = get_m4d(name_radix, opd2flatten, mask2flatten)
        if err ne adsec_error.ok then message, "ERROR!!"
        err = idl_4d_finalize()

        sopd = size(opd2flatten, /DIM)
        idx_m = where(mask2flatten)
    
        fit = surf_fit(xx[idx_m], yy[idx_m], double(opd2flatten[idx_m]), indgen(nz)+1, COEFF=coeff, /ZERN)
        cnorm = coeff
        ;normalized coefficient reordering to match the 4D order
        cnorm = transpose(cnorm[[1,2,3,6,7]])
    
    endif
   if averaged then begin
        opd2flatten=get_img4d(nmeas=3, out=mask2flatten)
        sopd = size(opd2flatten, /DIM)
        ;xx = rebin(findgen(sopd[0]),sopd)
        ;yy = rebin(findgen(1,sopd[1]),sopd)
        idx_m = where(mask2flatten)

        fit = surf_fit(xx[idx_m], yy[idx_m], double(opd2flatten[idx_m]), indgen(nz)+1, COEFF=coeff, /ZERN)
        cnorm = coeff
        ;normalized coefficient reordering to match the 4D order
        cnorm = transpose(cnorm[[1,2,3,6,7]])

    endif





    dpar = -(tf_mat ## cnorm) ; was -0.9*

    if keyword_set(VERBOSE) then print, "HXP MOVEMENT", transpose(dpar)

    if keyword_set(APPLY) then begin
        err = hexgetpos(actual_pos)
        if err ne adsec_error.ok then return, adsec_error.generic_error
        newpos = dpar+actual_pos
        if total(abs(newpos[[0,1, 2]]) gt [5000, 5000, 5000]) gt 0 then begin
            return, adsec_error.generic_error
        endif
        if total(abs(newpos[[3, 4]]) gt [900, 900]) gt 0 then begin
            return, adsec_error.generic_error
        endif
        
        err = hexmoveby([dpar[0], dpar[1], dpar[2], dpar[3], dpar[4], 0.0])
        if err ne 0 then return, err
    endif

 
    return, dpar

end



      
