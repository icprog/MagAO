function build_int_mat_v2, tracknum, ACT_SAMPLING=act_sampling, N2REMOVE=zern2remove, LIST=LIST
;ricordarsi di rimuovere le correzioni
   @adsec_common
   cl_act=adsec.act_w_cl
    if n_elements(act_sampling) eq 0 then pix_per_act=4 $
        else if (act_sampling lt 4) then pix_per_act=4
    if n_elements(zern2remove) eq 0 then zern2fit=[1,2,3]

    mainfolder='$ADOPT_MEAS/adsec_calib/if_functions/'+tracknum
    restore, mainfolder[0]+'/adsec_save.sav'
    ;fold2read=file_search(mainfolder)
    nfold=n_elements(tracknum)
    file2read=''
    for i=0, nfold-1 do begin
        filefold=mainfolder[i]+'/mode*.sav' 
        flist=file_search(filefold)
        ;if n_elements(flist) lt 1 then begin
        ;    log_print, 'Not enough Images found in folder'
        ;endif else begin 
        file2read=[file2read,flist]
    endfor
    
    file2read=file2read[1:*]
    n_modes=n_elements(file2read)
    nfiles=n_elements(file2read)
    imodes=intarr(n_modes)
    modesnames=strmid(file2read, /rev, 6,3)
    modesnumber=fix(modesnames)
    modeslist=sort(modesnumber)
    lastmode=(modesnumber[modeslist])[n_elements(modesnumber)-1] 
stop
;    list= where(strmid(list, /rev, 6,3) eq string(i, format='(i3.3)'))
    
    for i=0, nfiles-1 do begin
        if not keyword_set(list) then  begin
            id=where(modesnames eq string(i, format='(i3.3)'))
            if n_elements(id) gt 1 then begin
                message, 'Warning: more than 1 file found for mode:'+string(i)
            endif
        endif else begin
            id =i
        endelse
        
        restore, file2read[id]
        print, 'restoring file'+file2read[id]
                if (i eq 0) then begin
                    ss=size(opd, /dim)
                    size_ini=ss
                    xx = rebin(findgen(ss[0]),ss)
                    yy = rebin(findgen(1,ss[1]),ss)
                    ;pup=[234.348,      244.324   ,   219.549  ,   0.102942]                   
                    pup=pupilfit(mask, /circ, guess=[ss[0]/2, ss[1]/2, ss[0]/2, 0.1],/disp)
                    pupil_xyre=pup

                    n_pix_diam_x = pupil_xyre[2]*2
                    n_pix_diam_y = pupil_xyre[2]*2
                    nx_pix4d = (size(mask, /DIM))[0]
                    ny_pix4d = (size(mask, /DIM))[1]
                    xx = rebin(findgen(nx_pix4d), nx_pix4d, ny_pix4d, /SAMP)
                    yy = rebin(findgen(1, ny_pix4d), nx_pix4d , ny_pix4d, /SAMP)
                    mask_cx = total(mask*xx)/total(mask)
                    mask_cx = pupil_xyre[0]
                    mask_cy = pupil_xyre[1]
                    xx = (xx-mask_cx)/ceil(n_pix_diam_x/2)
                    yy = (yy-mask_cy)/ceil(n_pix_diam_y/2)

                    x0 = round(mask_cx - 1.1*n_pix_diam_x/2) > 0
                    x1 = round(mask_cx + 1.1*n_pix_diam_x/2) < (nx_pix4d-1)
                    y0 = round(mask_cy - 1.1*n_pix_diam_y/2) > 0
                    y1 = round(mask_cy + 1.1*n_pix_diam_y/2) < (ny_pix4d-1)

                    n_meta_pix_diam = pix_per_act*(2*adsec_shell.out_radius)/((adsec_shell.out_radius-adsec_shell.in_radius)/adsec_shell.n_rings)

                    pix_per_meta_pix_x = round(n_pix_diam_x/n_meta_pix_diam) > 1
                    pix_per_meta_pix_y = round(n_pix_diam_y/n_meta_pix_diam) > 1

                    n_pix_x = ceil(float(x1-x0+1)/pix_per_meta_pix_x)*pix_per_meta_pix_x
                    n_pix_y = ceil(float(y1-y0+1)/pix_per_meta_pix_y)*pix_per_meta_pix_y

                    n_meta_pix_x = n_pix_x/pix_per_meta_pix_x
                    n_meta_pix_y = n_pix_y/pix_per_meta_pix_y

                    opdseries   = fltarr(nfiles, n_meta_pix_x* n_meta_pix_y)
                    maskseries  = opdseries
                    mode_ampl   = fltarr(nfiles, nfiles)
                
                    mask_inters = intarr(n_meta_pix_x, n_meta_pix_y)+1

                endif
                id=where(mask)
                mask = mask[x0:x1, y0:y1]
                opd=opd[x0:x1,y0:y1]
                opd_dummy = fltarr(n_pix_x, n_pix_y)
                opd_dummy[0,0] = opd
                
                mask_dummy= fltarr(n_pix_x, n_pix_y)
                mask_dummy[0,0] = mask
                opd_dummy=opd_dummy*mask_dummy
                r_mask = rebin(mask_dummy, n_meta_pix_x, n_meta_pix_y)
                r_mask2cube=r_mask ge 0.3
                maskseries[i,*]=r_mask2cube            
                
                r_opd = rebin(opd_dummy, n_meta_pix_x, n_meta_pix_y)
                idx = where(r_mask gt 0)
                r_opd[idx] = r_opd[idx]/r_mask[idx]
                image_show, r_opd,/as,/sh, title='Attuation of mode #'+strtrim(i,2)
                opdseries[i,*]=r_opd
                mask_inters=mask_inters*r_mask
        
                ;opd=rebin(opd, n_meta_pix_x,n_meta_pix_y) ; implemented in opt_int_mat_reduce
                ;opdseries[i, *] = opd[*]
                ;mask=mask[x0:x1,y0:y1]
                ;mask_inters=mask_inters*mask 
                ;mask=rebin(mask, n_meta_pix_x, n_meta_pix_y)  ; implemented in opt_int_mat_reduce
                ;mask_inters=mask_inters*mask
                
                
;               maskseries[i,*]=mask[*]
                mode_ampl[i,i]=applied_amp   ;was *2.  but why?????? as data from if_redux.pro are in wavefront=2*surface
                
            endfor

    data  =  {r_opd_cube   :  opdseries,   $
              r_mask_cube  :  maskseries,  $     
              applied_mode :  mode_ampl   $
             }  
   ;Here, additional variables are created to be stored: this, in order to be compliant with the routine opt_int_mat_reduce
    is_new    =  1
    max_rms   = max(mode_ampl)
    min_pix   = total(mask_inters)
    intersect_mask_idx=where(mask_inters)
    data_ver=2    
    ;data_ver=2 corresponds to  mode_ampl[i,i]=applied_amp... if_redux modified as well : 20100531    
    ;data_ver=1corresponds to  mode_ampl[i,i]=applied_amp*2. 
    flatdir = '$ADOPT_MEAS/adsec_calib/flat/'+tracknum[0]
    if file_test(flatdir) eq 0 then  file_mkdir, flatdir
    filename = flatdir+'/'+'flat_data_'+strtrim(n_modes,2)+'modes.sav'
    if (file_info(filename)).exists eq 1 then ff = add_timestamp(filename) else ff=filename

    save,  filename=ff,data $
        ,  cl_act, n_modes, n_pix_x, n_pix_y, n_meta_pix_x, n_meta_pix_y $
        , x0,x1,y0,y1,pupil_xyre, mask_cx, mask_cy, min_pix, intersect_mask_idx, max_rms, size_ini, data_ver, adsec_save, tracknum
;print, add4dlog(tracknum, 'Build_IntMat')


return, 0
end