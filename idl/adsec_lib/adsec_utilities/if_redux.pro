
function if_redux, tracknum, TEST=test, FORCE_FIRST_FRAME=force_q $
                , USE_PATCH=use_patch, NOISE_MEASUREMENT=noise_measurement, ZONAL=do_zonal, EXT=ext $
                , SAVE_ONLY_MODE_LIST=only_mode_list, KL=do_modal_kl, MODAL_MIRROR=do_modal_mirror, CONF_FILE=conf_file , NORM=norm, TEMPLATE=template
;
; save opd and mask of Influence Functions from optical data. opd in 2*surface deformation of the mirror in meters
;!!!in case NORMALIZE is used, the OPD is normalized to surface amplitude
  @adsec_common
    ;  if not keyword_set(NOISE_MEASUREMENT) then histdir    = adsec_path.meas+'disturbance/'+dist_tracknu
    hist_file    = file_search(adsec_path.meas+'disturbance/'+tracknum, 'disturb*.sav')
    comminfo=strsplit(hist_file, '/',/ex)
    comminfo=comminfo[n_elements(comminfo)-1]
    a=strsplit(comminfo, '_',/ex)
    first_mode=fix(a[2])
    last_mode=fix(a[3])
    restore,hist_file
    if n_elements(cmd_mat) ne 0 then begin
 command_matrix = cmd_mat 
endif else begin 
        command_matrix = adsec_save.ff_p_svec; mod RB @magellan for slaved basis 
endelse
    writefits, adsec_path.meas+'if_functions/'+tracknum+'/command_matrix.fits', command_matrix
    default_file=adsec_path.conf+'scripts/4d_opt_intmat_default.txt'
    def0=read_ascii_structure(default_file)
    h5dir=adsec_path.meas+'4d/'+def0.folder4d+tracknum+'/'+def0.foldh5
    four_sight_conf=h5dir+'/'+def0.conf4d 

    if n_elements(conf_file) eq 0 then conf_file = adsec_path.meas+'if_functions/'+tracknum+'/'+'info.txt'
    def=read_ascii_structure(conf_file)
    n_heading_zeros  = def.ini_zeros
    n_trailing_zeros = def.end_zeros
    amp_trigger      = def.trigmode_amp
    form             = def.template
    if (n_elements(template) ne 0) then form = template
    
    check_sum  = keyword_set(do_zonal)+keyword_set(do_modal_mirror) $
    +keyword_set(do_modal_kl)+keyword_set(noise_measurement)
    if check_sum eq 0 then message, 'Type of actuation has not been specified (KEWORDS: ZONAL,MOADAL_MIRROR,KL)'
    if check_sum gt 1 then message, 'Only a single keyword can be set umong: ZONAL,MOADAL_MIRROR,KL'
    case 1B of
        keyword_set(do_zonal): type='_zonal_'
        keyword_set(do_modal_mirror): type='_modal_'
        keyword_set(do_modal_kl): type='_klfit_'
        keyword_set(noise_measurement): type='_noise_'
    endcase

    if (n_elements(ext) eq 0) then ext0='' else ext0='_'+ext

    imgname='img'
;  pattern    = imgname+'*.h5'
;  resdir     = dir0

; file_search seems to have problems with disks mounted with Samba. I will use the obsolete findfile routine  ;; then we came back to file_search as in this version of samba it works better
; flist      = findfile(filepath(ROOT=dir, imgname+'*.h5'), COUNT=nfiles)
;flist      = file_search(dir, imgname+'*.h5', COUNT=nfiles)
    flist      = file_search(filepath(ROOT=h5dir, imgname+'*.h5'), COUNT=nfiles)
    if nfiles eq 0 then message, "NO H5 FILES FOUND IN: "+h5dir
    missed_modes=' Missing modes:'
    num=indgen(nfiles)
    num=string(num, format='(i4.4)')
    files=h5dir+'/'+imgname+'_'+num+'.h5'
    opd0  = read_opd(files[0], bad=mask0, type='4d',wedge=1.)
    if n_elements(opd0) eq 1 then message, "NO OPD FILE FOUND: "+flist[0]
    missed_modes=' Missing modes:'
    ss=size(opd0,/dim)
    idx0 = where(mask0)
    xx = rebin(findgen(ss[0]),ss)
    yy = rebin(findgen(1,ss[1]),ss)
    maxv=max(xx[idx0],MIN=minv)
    xx -= (minv+maxv)/2
    xx /= (maxv-minv)/2
    maxv=max(yy[idx0],MIN=minv)
    yy -= (minv+maxv)/2
    yy /= (maxv-minv)/2

    opd1  = read_opd(files[1], bad=mask1, type='4d',wedge=1.)
    if n_elements(opd1) eq 1 then message, "NO OPD FILE FOUND: "+flist[1]

    mask0 *= mask1
    id=where(mask0)
; tilt removal added at LBTO because of large vibrations that impede to identify trefoil frame
    opd0[id] -= surf_fit(xx[id],yy[id],opd0[id],[1,2,3])
    opd1[id] -= surf_fit(xx[id],yy[id],opd1[id],[1,2,3])

    rms0=stddev(opd0[id]-opd1[id])
    image_show, (opd0-opd1), /as, /sh, title='OPD1-OPD0'
    if not keyword_set(noise_measurement) then begin

            ;given the configuration file, there is no more need to extract the trigger information
            ;www = rebin(fhist[0:39,*]^2,40,1)
            ;iii = where(www ne 0)
            ;n_heading_zeros = iii[0]
            ;n_trailing_zeros = iii[1]-iii[0]-1
             ; modRB ;  if stddev(fhist[n_heading_zeros,*]) eq 0 then message, "UNEXPECTED TRIGGER POSITION"
        mode_ref=amp_trigger*adsec_save.ff_p_svec[trigger_mode,adsec_save.act_w_cl]
        rms_ref=2*rms(mode_ref) ; times to to match the interferometer measurement with respect to surface
        step       = n_elements(form)
        if rms0 gt rms_ref/3 then message, "trigger level too low with respect to noie"
        rr=-1
        q=-1
        window, 0
        while (rr lt 1/3.) do begin
            q=q+1
            print, q
            opd=read_opd(flist[q], bad=mask, type='4d', wedge=1.)
            opdtest=opd-opd0
            id=where(mask*mask0)
            opdtest[id] -= surf_fit(xx[id],yy[id],opdtest[id],[1,2,3])
            ri=rms(opdtest[id])
            rr=ri/rms_ref
            image_show, mask*mask0*opdtest, /as, /sh, title='TRIGGER. rms [nm]:'+strtrim(ri*1e9,2)+' rms ratio:'+strtrim(rr,2)
            opd0=opd
            mask0=mask
            print, rr
        endwhile

        first=q+1+n_trailing_zeros
        if keyword_set(use_patch) then q -= 1

        nmodes=last_mode-first_mode+1
        nmodes_max=(nfiles-first+1)/step
        if nmodes gt nmodes_max then message, "NO ENOUGH FILES TO PROCESS THE REQUESTED NUMBER OF MODES"
    endif else begin
        q=0
        first=0
        ;form = [1,-1,1]
        ;step = n_elements(form)
        nmodes = nfiles
        step=1
        first_mode=0
    endelse

    if n_elements(force_q) ne 0 then first=force_q
          ;  if (step ne n_elements(form)) then print, 'ERROR, wrong algorithm provided!'
          ;  if ((nfiles mod step) ne 0) then begin
          ;    print, 'ERROR, wrong files number!'
          ;  endif else begin
        n_only_mode_list = n_elements(only_mode_list)
        if n_only_mode_list ne 0 then do_only_mode_list =1B else do_only_mode_list =0B
        tt = fltarr(2,nmodes)

        save_dir =  file_dirname( filepath('dummy', ROOT='$ADOPT_MEAS/adsec_calib',SUB="if_functions/"+tracknum+'/'+ext0))
        file_mkdir, save_dir
        new4d_conffile='$ADOPT_MEAS/adsec_calib/if_functions/'+tracknum+'/'+def0.conf4d
        if file_test(new4d_conffile) eq 1 then file_delete, new4d_conffile
        file_copy, four_sight_conf, new4d_conffile
        window, 1
        window, 2
        window, 3
        mask_intersection=replicate(1B, ss[0], ss[1])
        mask_union=bytarr(ss[0], ss[1])

    for i=first,nmodes*step+first-1,step do begin
        mode = (first_mode+((i-first)/step))
        if do_only_mode_list then begin
            iii=where(only_mode_list eq mode, count_iii)
            if count_iii eq 0 then do_save_mode=0B else do_save_mode=1B
            endif else do_save_mode=1B

            if do_save_mode then begin
            opdtemp=dblarr(ss[0],ss[1],step)
            mask=replicate(1B, ss[0], ss[1])
            for p=0, step-1 do begin
                n=string(i+p, format='(i4.4)')
                f=h5dir+'/'+imgname+'_'+n+'.h5'
                opdtemp[*,*,p]=read_opd(f, bad=maskd, type='4d',wedge=1.)
                maskd=float(maskd)
                mask=mask*maskd
            endfor

            ;if laser dims out, no measurement is taken! discard the measurement and go on...
            if total(mask) eq 0. then begin
                modelost=strtrim(fix(mode),2)
                message, '4D laser dimmed!!! Fucked up measurement of mode number: '+modelost, /info
                missed_modes=missed_modes+' '+modelost
                print, missed_modes 
                continue
            endif

            opdtemp *= rebin(mask,ss[0],ss[1],step,/SAMP)
            id=where(mask)
            opd=dblarr(ss[0],ss[1])
            opd_check=dblarr(ss[0],ss[1])
            for p=1, step-1 do begin
                opd += opdtemp[*,*,p]*form[p]+opdtemp[*,*,p-1]*form[p-1]
                opd_check += opdtemp[*,*,p]+opdtemp[*,*,p-1]
            endfor
            opd /=(step-1)   ;averaging the number of push-pull couples
            opd /= 4. ;IMPORTANT!!! Divide by 2 due to retroreflector, and by 2 to export data in surface!
            opd_check /=(step-1)
            opd_check /= 4.

;		lines below were inverting the sign of the OPD!!! what a f#@*ck!!!!!
;        opd*=-1.
;        opd_check*=-1.

            if not keyword_set(noise_measurement) then begin
                if keyword_set(norm) then    opd /=(amp[mode-first_mode])
                applied_amp = amp[mode-first_mode]
            endif

            opd *=mask
            opd_check *=mask
            opd_check[id] -= mean(opd_check[id])
            mask_intersection *= mask
            mask_union = mask_union OR mask
            ;idx_check = where(mask*mask_prev)
            ;rms_check = stdev(opd_check[idx_check]-opd_check_prev[idx_check])

        print, 'Mode:'+strtrim(mode,2)+' dir: '+h5dir
        smode=string(mode,FORMAT="(I3.3)")
        if i eq first then begin
            if (not keyword_set(noise_measurement)) and keyword_set(use_patch) then opd *= 4.0/3.0
        endif
        id = where(mask)

        data_ver=2
        ;data_ver = 2 corresponds to opd = surface : 20100531
        ; data_ver = 1 corresponds to opd = 2* surface
        if keyword_set(noise_measurement) then begin
            dummy = surf_fit(xx[id],yy[id],opd[id], [1,2,3],COEFF=coeff,/ZERN)
        endif else begin
            dummy = surf_fit(xx[id],yy[id],opd_check[id], [1,2,3],COEFF=coeff,/ZERN)
        endelse
        tt[*,(i-first)/step]=coeff[1:2]
        if keyword_set(noise_measurement) then opd[id]-=coeff[0]
        if keyword_set(noise_measurement) then begin
             save, filename=filepath(ROOT=save_dir,'/mode_'+smode+'.sav'),opd, mask,data_ver
        endif else begin
            if keyword_set(norm) then mode_normalized_to_amp = 1b else mode_normalized_to_amp = 0b
            save, filename=filepath(ROOT=save_dir,'/mode_'+smode+'.sav'),opd, mask, applied_amp, $
            	mode_normalized_to_amp, data_ver
        endelse
        if keyword_set(noise_measurement) then begin
            wset,1 & image_show, opd*mask, /AS, /SH, /EQ, title='mode '+smode+'. Noise on differential measurement'
        endif else begin
            wset,1 & image_show, opd*mask, /AS, /SH, title='mode '+smode+'. differential actuation'
        endelse
        wset,2 & image_show, opd_check*mask, /AS, /SH, title='mode '+smode+'. Bias shape'
        wset,3 & image_show, mask_union-mask_intersection,title='mode '+smode+' union-intersection of masks', /AS, /SH
    endif
endfor

if ~do_only_mode_list then save, tt, data_ver, filename=filepath(ROOT=save_dir,'/tt_vector.sav')
ao_status = ptr2value(!AO_STATUS)
save, adsec_save, filename=filepath(ROOT=save_dir,'/adsec_save.sav')
save, ao_status, filename=filepath(ROOT=save_dir,'/ao_status.sav')
window, 3
if keyword_set(noise_measurement) then begin
    tit_str = 'Tip-Tilt coeffs hist on differential measurement'

endif else begin
    tit_str = 'Tip-Tilt coeff hist on bias measurement'
endelse
plot, tt[0,*]*4*1e9, TITLE=tit_str, YTIT='Zern-Noll Tilt amp [nm]'
oplot, tt[1,*]*4*1e9, COLOR=255L
tt_tot=sqrt(tt[0,*]^2+tt[1,*]^2)
print, "Max ptv tilt (surface) [nm]:", max(tt_tot)*4*1e9
print, "RMS tilt coeff (surface) [nm]:", sqrt(mean(tt_tot^2))*4*1e9
fft1,tt[0,*],3./25,FSPEC=fspec, PSPECTRUM=pspectrum, PSD=psd,/noplot
window, 4
plot, fspec,psd*fspec*1e18,TIT="tip-tilt PSD", XTIT='freq [Hz]', YTIT='PSF*freq [nm^2]',/XLOG,PSYM=10
fft1,tt[1,*],3./25,FSPEC=fspec, PSPECTRUM=pspectrum, PSD=psd,/noplot
oplot, fspec,psd*fspec*1e18, color=255L, PSYM=10

save, filename=filepath(ROOT=save_dir,'/mask_intersection.sav'),mask_intersection
iddiff=where(mask_union-mask_intersection)
idgood=where(mask_intersection)
n_good = (float(n_elements(iddiff))/n_elements(idgood))


print, add4dlog(tracknum, 'IF_Redux'+string(n_good)+missed_modes)

end

;pupil_xyre=[496.846,      496.849,      436.529,   -0.0917443]
;coord = pupilfit(mask, /circle, /DISP, GUESS=[248, 238, 229, 0.1])
;  out=7
;  x0=fix(coord[0]-coord[2]-out)
;  xf=fix(coord[0]+coord[2]+out)
;  y0=fix(coord[1]-coord[2]-out)
;  yf=fix(coord[1]+coord[2]+out)
;  ss1=[xf-x0,yf-y0]
;opd0=read_opd(flist[0], bad=mask, type='4d',wedge=1.)&rr=dblarr(21)&for i=0, 20 do begin& opd=read_opd(flist[i], bad=mask, type='4d',wedge=1.)&opd=opd-opd0&id=where(mask)&rr[i]=rms(opd[id]) &image_show, opd, /as, /sh, /eq&wait, 1&   endfor
