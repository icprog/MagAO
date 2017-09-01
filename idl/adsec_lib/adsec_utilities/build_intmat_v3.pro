function build_intmat_v3, tracknum, ACT_SAMPLING=act_sampling, N2REMOVE=zern2remove

    ;ricordarsi di rimuovere le correzioni
    @adsec_common
    tracknum = ['20100525_161704', '20100525_161850','20100525_162017', '20100525_162214', '20100525_162343', '20100525_162538', '20100525_162634', '20100525_162747', '20100525_162908', '20100525_163238', '20100525_163356', '20100525_164156']
    tracknum = ['20100525_161704', '20100525_161850','20100525_162017', '20100525_162214', '20100525_162343', '20100525_162538', '20100525_162634', '20100525_162747', '20100525_162908']
    tracknum = ['20100525_161704', '20100525_162214']
    cl_act=adsec.act_w_cl
    if n_elements(act_sampling) eq 0 then pix_per_act=4 else if (act_sampling lt 4) then pix_per_act=4
    if n_elements(zern2remove) eq 0 then zern2fit=[1,2,3]

    mainfolder = filepath(ROOT=getenv('ADOPT_MEAS'), sub=['adsec_calib', 'if_functions'], '')
    for i=0, n_elements(tracknum)-1 do begin
        currdir = filepath(ROOT=mainfolder , SUB=tracknum[i], '')
        if i eq 0 then names = file_search(currdir+'*') else names = [names, file_search(currdir+'*') ]
    endfor
    newnames= stregex(names, '.*mode.*.sav', /ex)
    file2read = newnames[where(newnames ne "")]
    idw = fix(strmid(file_basename(file2read), 5, 3))
    nmodes = n_elements(uniq(idw[sort(idw)]))
    nfiles = n_elements(file2read)
   
    for i=0, nfiles-1 do begin
        restore, file2read[i]
        print, 'restoring file'+file2read[i]
                if (i eq 0) then begin
                    ss=size(opd, /dim)
                    size_ini=ss
                    xx = rebin(findgen(ss[0]),ss)
                    yy = rebin(findgen(1,ss[1]),ss)
                    pupil_xyre=pupilfit(mask, /circ, guess=[ss[0]/2, ss[1]/2, ss[0]/2, 0.1],/disp)
;                   this part has been written by RB to implement an arbitrary rebinning of the frame. a series of tests revealed that in 
;                   the final rebinned image the pupil was not perfectly tangent to the image. modification starts here
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

                    opdseries   = fltarr(nmodes, n_meta_pix_x* n_meta_pix_y)
                    maskseries  = opdseries
                    mode_ampl   = fltarr(nmodes, nmodes)
                
                    mask_inters = double(intarr(n_meta_pix_x, n_meta_pix_y)+1)

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
                if total(abs(maskseries[idw[i],*])) eq 0 then maskseries[idw[i],*]=float(r_mask2cube) else maskseries[idw[i],*] *= float(r_mask2cube)
                
                r_opd = rebin(opd_dummy, n_meta_pix_x, n_meta_pix_y)
                idx = where(r_mask gt 0)
                r_opd[idx] = r_opd[idx]/r_mask[idx]
                image_show, r_opd,/as,/sh, title='Attuation of mode #'+strtrim(idw[i],2)
                if total(abs(opdseries[idw[i],*])) eq 0 then opdseries[idw[i],*]=r_opd[*] else opdseries[idw[i],*] += r_opd[*]
                mask_inters=double((mask_inters*r_mask) gt 0)
        
                if total(abs(mode_ampl[idw[i],idw[i]])) eq 0 then mode_ampl[idw[i],idw[i]]=applied_amp else mode_ampl[idw[i],idw[i]]+=applied_amp 
                
            endfor
            hist=histogram(idw, bin=1) 
            bins = findgen(n_elements(hist))+min(idw)
            hist = transpose(float(hist))
            opdseries /= rebin(hist, nmodes, n_elements(hist)) 
            tmp = extract_diagonal(mode_ampl) / hist
            for i=0, nmodes-1 do mode_ampl[i,i]=tmp[i] 
            if n_elements(list2ranges(idw)) ne 2 then return, "Error: Some modes are missing. The covered ranges are: "+string(list2ranges(idw), FORMAT='(I3.3)')

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
    path = meas_path('flat')
    save, path+'flat_data_'+strtrim(n_modes,2)+'modes.sav'$
    , data, cl_act, n_modes, n_pix_x, n_pix_y, n_meta_pix_x, n_meta_pix_y $
    , x0,x1,y0,y1,pupil_xyre, mask_cx, mask_cy, min_pix, intersect_mask_idx, max_rms, size_ini, data_ver, tracknum
    
    return, adsec_error.ok
end
