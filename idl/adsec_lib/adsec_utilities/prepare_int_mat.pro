function prepare_int_mat, tracknum
    
    n_acq=n_elements(tracknum)
    
    for i=0, n_acq-1 do begin
        print, 'Creating int mat number: ',tracknum[i]
        print, if_redux(tracknum[i], /MODAL)
    endfor
    
    print, build_int_mat_v2(tracknum)
    return, 0
end
