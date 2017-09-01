;GET INTERFEROGRAM FROM 4D with a WEDGE of 0.5 respect to the OPD SAVED (SURFACE)

Function get_m4d, name_radix, opd, mask, MIN_PIX=min_pix, VALUES=values, BURST=burst, TAKE_MEAS=take_meas, READ_FILE=read_file

    @adsec_common
    n=1
    nrep=0
    out = 0 
    if n_elements(maxiter) eq 0 then maxiter=3
    path = filepath(root=adsec_path.meas, sub=['4d','Zcopy'], '')
    if ~keyword_set(TAKE_MEAS) and ~keyword_set(READ_FILE) then begin
        take_meas=1
        read_file=1
    endif


    if keyword_set(BURST) then begin
            nb=burst > 10
            fullname = idl_4d_get_burst(nb,name_radix)
            str_fullname = strtrim(string(fullname),2)
            if strlen(str_fullname) eq 0 or strmid(str_fullname, 0,1) eq '-' then begin
                print, "Error returned from 4d: ", fullname
                if str_fullname eq "" then return, adsec_error.generic_error else return, fullname
            endif
            
            for i=0, nb-1 do begin

                name_opd = path+fullname+'/MEAS_'+string(i, FORMAT='(I4.4)')+'.h5'
                opd_tmp = read_opd(name_opd, BAD_PIXEL_MASK=mask_tmp, WEDGE=0.5, /FLIPY, type='4D', VALUES=values)
                if i eq 0 then begin
                    opd = opd_tmp
                    mask = mask_tmp
                endif else begin
                    opd += opd_tmp
                    mask = mask and mask_tmp
                endelse
            
            endfor
            opd /= float(nb)
            opd *= mask
            
            return, adsec_error.ok

    endif else begin
        repeat begin

            if keyword_set(take_meas) then begin
                out = 1
                fullname = idl_4d_get_measure(name_radix)
                str_fullname = strtrim(string(fullname),2)
                if strlen(str_fullname) eq 0 or strmid(str_fullname, 0,1) eq '-' then begin
                    print, "Error returned from 4d: ", fullname
                    if str_fullname eq "" then return, adsec_error.generic_error else return, fullname
                endif
            endif
            if keyword_set(read_file) then begin
                out = 1
                fullname = 'SINGLE_MEAS/hdf5/'+name_radix+'_0000.h5'
                name_opd = stregex(path+fullname, '.*\.', /ex)+'h5'

                opd = read_opd(name_opd, BAD_PIXEL_MASK=mask, WEDGE=0.5, /FLIPY, type='4D', VALUES=values)
            
                if n_elements(min_pix) ne 0 then begin
                    if total(mask) lt min_pix then begin
                        nrep +=1
                        out=0
                    endif
                endif 
            endif

        endrep until (out) or (nrep gt maxiter)
        if nrep gt maxiter then return,adsec_error.generic_error else return,adsec_error.ok
    endelse

        
End

