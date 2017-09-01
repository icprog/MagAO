

function get_img4d, NMEAS=nmeas, OUT=out , BASETIME=basetime

        @adsec_common
        if (n_elements(nmeas) eq 0) then begin
            nacq=1
        endif else begin
            nacq=nmeas
        endelse
        
        default_file=adsec_path.conf+'scripts/4d_opt_intmat_default.txt'
        def=read_ascii_structure(default_file)
;        opd2read='$ADOPT_MEAS/adsec_calib/4d/'+def.flat4d
        namefold=def.flat4d
        opd2read='$ADOPT_MEAS/adsec_calib/4d/'+def.folder4d+namefold
;        opd2read='$ADOPT_MEAS/adsec_calib/4d/'+def.folder4d+namefold+'/'+def.foldh5
        while file_test(opd2read) eq 1 do begin
            file_delete, opd2read, /recursive
        endwhile
 

;       if file_test(opd2read) eq 1 then begin 
;                file_delete, opd2read, /recursive
;                log_print, 'Deleting old files'
;        endif
        nstep2ave = 1
        time2wait=0
        if n_elements(basetime) ne 0 then begin
            nstep2ave=10.
            time2wait=basetime/nstep2ave
        endif
        
        for kk=0, nstep2ave-1 do begin
            while file_test(opd2read) eq 1 do begin
                file_delete, opd2read, /recursive
            endwhile    
            err = idl_4d_init()
            if err ne adsec_error.ok then message, "ERROR!!"
            log_print, 'Acquiring RAW frames from 4D interferometer'
            print, idl_4d_capture(nacq, namefold)
            print, idl_4d_produce(namefold)
            log_print, '4D Capturing and Processing completed'
            err = idl_4d_finalize()
            if err ne adsec_error.ok then message, "ERROR!!"
        
            wait, 5
            flist=file_search(opd2read, '*.h5')
            nfile=n_elements(flist)
            n2ave=nfile

            for i=0, nfile-1 do begin
            print, 'reading file'+flist[i]
            opd=read_opd(flist[i], bad=mask, type='4d')
            if i eq 0 then begin
                totopd=opd*0.
                totmask=mask*0.+1
                nn=n_elements(opd)
            endif
            idx=where(mask eq 1B)
            if n_elements(idx)/float(nn) lt 0.1 then begin
                log_print, '4D Laser dimmed during the acquisition, this file will be discarted'
                n2ave -= 1
            endif else begin
                totopd  += opd
                totmask *= mask
            endelse
        endfor
        opd         = totopd/n2ave
        out         = totmask
        if kk eq 0 then begin
            result_opd=opd 
            result_out=out
        endif else begin
            result_opd=result_opd+opd
            result_out=result_out*out
        endelse
        wait,time2wait
        log_print, 'Waiting for iteration n'+string(kk)
        
        endfor
        opd=result_opd/nstep2ave
        out=result_out
        return, opd

    end
