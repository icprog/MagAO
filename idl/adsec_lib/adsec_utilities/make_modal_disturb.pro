;+
; MAKE_MODAL_DISTURB
;
;NB: if PASSEDBASE is set, the "base passed" has to be 1 RMS normalized and in the form [nmodes x n_actuators]
;EXAMPLES:
;print, make_modal_disturb(0, 100, /SEND, /SAV, EXT='pippo')    -> ./meas/disturbance/%date%/disturb_modal_000_100_pippo.sav
;print, make_modal_disturb(0, 100, /SEND, /SAV, /ZONAL)    -> ./meas/disturbance/%date%/disturb_zonal_000_100.sav

Function make_modal_disturb, from, to,  HIST=fhist, TEMPLATE=template, ZONAL=zonal, AMP=amp $
                           , SEND=send, EXT=ext, SAVEFILE=savefile, SCALE_FACTOR_AMP=scale_factor_amp, KL=kl, PASSEDBASE=passedbase $
                           , FORCE_FACTOR_AMP=force_factor_amp, NOCHECK=nocheck, DEC_FACTOR = dec_factor, TRIGG_AMP=trigg_amp, INI_0=ini_0, END_0=end_0, MAX_RMS=max_rms, TRACK=track

    @adsec_common

    amp_trigger = trigg_amp
    n_heading_zeros = ini_0
    n_trailing_zeros = end_0
    trigger_mode = 5

    if n_elements(from) eq 0 then message, 'starting modal index has not been specified'

    if n_elements(to) eq 0 then begin
        to  = from
    endif
    
    if n_elements(dec_factor) eq 0 then begin
        dec_factor =1
    endif
    if n_elements(max_rms) eq 0 then   max_rms = 150e-9
    max_peak_force = 0.3
    if n_elements(force_factor_amp) ne 0 then max_peak_force *= force_factor_amp
    if max_peak_force gt 0.7 then begin
        message, "TOO LARGE FORCE (limit is 0.7N): ",max_peak_force 
    endif
    if n_elements(ext) eq 0 then ext0='' else ext0='_'+ext

    zonal_mat = identity(adsec.n_actuators)
;    THIS IS THE DEFINITION OF THE BAAD LIST USED FOR ZONAL IF
;    list1 = adsec.mir_act2act ;OLD BAD
;    mir_cl = adsec.act2mir_act[adsec.act_w_cl]
;    err= intersection(list1, mir_cl, outl)
;    mir_cl_list = adsec.mir_act2act[outl]   ;OLD BAAD
;    zonal_mat = zonal_mat[mir_cl_list,*]    ;OLD BAAD
;    ------ END------
    list1 = adsec.act2mir_act ;new definition
    mir_cl = adsec.act2mir_act[adsec.act_w_cl]
    err= intersection(list1, mir_cl, outl)
    zonal_mat = zonal_mat[outl,*]            ;new definition
    ncl = n_elements(adsec.act_w_cl)

    if keyword_set(zonal) then begin
        cmd_mat = zonal_mat
        type_str = '_zonal_'
    endif else begin
        if keyword_set(kl) then begin
            ;kl_mat = readfits(filepath(ROOT=adsec_path.data,'20090817_fittedKLs_02.fits'))
        ;kl_mat = readfits('/home/labot/idl/adsec672a/data/20090817_fittedKLs_02.fits')
        kl_file = '/local/M2C/KL_v4/m2c.fits'
        kl_file = '/towerdata/adsec_calib/M2C/20110311_full_m2v.fits'
        kl_file = '/towerdata/adsec_calib/M2C/KL_MAG585_20110706/m2c.fits'
        ;kl_file= '/adsec_calib/m2c/KL_v5/m2c.fits'
        kl_mat = readfits(kl_file)
        log_print, 'Using KL base!!'
        log_print, 'Base restored: '+kl_file
            cmd_mat = kl_mat[0:ncl-1, *]
            type_str = '_klfit_'
        endif else begin
            if keyword_set(PASSEDBASE) then begin
                cmd_mat = passedbase
                type_str = '_passedbase_'
            endif else begin
                cmd_mat = adsec.ff_p_svec[0:ncl-1, *]
                type_str = '_modal_'
            endelse
        endelse
    endelse
        

    if keyword_set(passedbase) then nmodes = (size(passedbase, /DIM))[0] else nmodes=ncl
    amp = fltarr(nmodes)
    ptv_pos = fltarr(nmodes)
    max_force = fltarr(nmodes)
    ;AMPLITUDE GENERATION AND FORCE SCALING
    for i=0, nmodes-1 do begin
        if keyword_set(zonal) then begin
            max_amp = max_rms
            mode2apply = max_amp*zonal_mat[i,*] ;; actuator eith CL!!!
        endif else begin
            if keyword_set(kl) then begin
                max_amp = 800d-9*((i+1)/10.0)^(-0.66)
                max_amp = max_amp < max_rms
;                max_amp = max_amp > 90d-9  comment instrted by RB 20101221
                mode2apply = max_amp*kl_mat[i,*]
            endif else begin
                if keyword_set(PASSEDBASE) then begin
                    max_amp = (800d-9*sqrt(n_elements(adsec.act_w_cl)))*((i+1)/10.0)^(-0.66)
                    max_amp = max_amp < (max_rms*sqrt(n_elements(adsec.act_w_cl)))
                    ;max_amp = max_amp > (90d-9*sqrt(n_elements(adsec.act_w_cl)))
                    mode2apply = max_amp*passedbase[i,*]
                endif else begin
                    max_amp = (800d-9*sqrt(n_elements(adsec.act_w_cl)))*((i+1)/10.0)^(-0.66)
                    max_amp = max_amp < (max_rms*sqrt(n_elements(adsec.act_w_cl)))
                    ;max_amp = max_amp > (90d-9*sqrt(n_elements(adsec.act_w_cl)))
                    mode2apply = max_amp*adsec.ff_p_svec[i,*]
                endelse
            endelse
        endelse

        mode2apply = float(mode2apply)
        force2apply = float(adsec.ff_matrix ## mode2apply)
        maxf = max(abs(force2apply))
        if maxf ne 0.0 then begin
            scale = (max_peak_force/maxf) < 1.0
            amp[i] = scale*max_amp
            mode2apply = scale*mode2apply
            force2apply = scale*force2apply
            maxf *= scale
        endif else begin
            amp[i] = max_amp
        endelse
        max_force[i] = maxf
        ptv_pos[i] = max(mode2apply)-min(mode2apply)
    endfor
    
    if n_elements(scale_factor_amp) ne 0 then begin
     amp *= scale_factor_amp
     max_force *= scale_factor_amp
     ptv_pos *= scale_factor_amp
    endif


    ;FIXED amplitude pattern: 5 zero commands, +1 trefoil, 4 zero commands
    prend=(n_heading_zeros+n_trailing_zeros+1)*dec_factor
    prefix = fltarr(prend, adsec.n_actuators) 
    n_empty=n_elements(prend)
    prefix[n_heading_zeros*dec_factor:n_heading_zeros*dec_factor+dec_factor-1, *] = rebin(adsec.ff_p_svec[trigger_mode, *],dec_factor,adsec.n_actuators, /sampl) * amp_trigger  
    
    if n_elements(template) eq 0 then begin
        template = [1, -1, 1]
    endif
    if dec_factor gt 1 then begin
        template_dec = rebin(template, dec_factor * n_elements(template), /SAMP)
    endif else begin
        template_dec=template
    endelse

    if n_elements(rep) eq 0 then begin
        rep=1
    endif

    ;DUMMY CODE FOR THE APMPLITUDE SCALING
    ;ho preso il seguente file
    ;restore, '/home/labot/work/AO/idl/adsec672a/meas/flat/2009_03_31/flat_data_60um_05deg_000_652.sav'
    ;ho verificato che l'intersezione delle maskere fosse ok (a vista)
    ;mask = data.r_mask_cube
    ;mf = total(mask, 1)

    ;aa = float(mf eq 653)
    ;bb = float(mf gt 0)
    ;image_show, reform(bb-aa, 124, 124), /as, /sh
    ;ho preso le ampiezze dalla struttura data e le ho divise per due (perche' nell'"acquire" vengono salvate doppie)
    ;amp = extract_diagonal(data.applied_mode)/2d
    
    ;scalo opportunamente i modi di ff per queste ampiezze
    nm = to-from+1
    nt = n_elements(template_dec)
    if ~keyword_set(nocheck) then begin
        if double(rep)*nm*nt/float(dec_factor) gt 10000L then begin
            print, "OUT OF INTERFEROMETER SINGLE PACK MEASUREMENT SET"
            return, -1
        endif
    endif

    index = indgen(nm)+from
    ;mhist = rebin(adsec.ff_p_svec[index,*], nm*nt, adsec.n_actuators, /SAMP)
    mhist = rebin(cmd_mat[index,*], nm*nt, adsec.n_actuators, /SAMP)
    mamp  = rebin(amp[index], nm*nt, /SAMP)
    dummy = reform(transpose(rebin(transpose(template_dec), nm, nt, /SAMP)), nm*nt) * mamp
    modif = rebin(dummy, nm*nt, adsec.n_actuators, /SAMP)
    hist  = float(modif * mhist)
    fhist = fltarr(rtr.disturb_len, adsec.n_actuators)

    ;INITIAL PADDING
    fhist[0:prend-1, *] = prefix
    ;temporay patch for removing initial trefoil block
;    prend=0

    
    ;filling
    if nm*nt+prend gt rtr.disturb_len then begin
        print, "OUT of disturbance length :"+string(rtr.disturb_len, format="(I4.4)")
        return, -1
    endif
    fhist[prend:nm*nt-1+prend, *]=hist
    ;REPETITION (NOT WELL IMPLEMENTED YET)
    for i=1,rep-1 do begin
        fhist[i*nm*nt:(i+1)*nm*nt-1,*]=hist
    endfor


;window, 0
;plot, index, ptv_pos[index]*1e9, XTIT='mode idx', YTIT='abs(amplitude) and PtV(red) [nm]'
;oplot, index, amp[index]*1e9, COLOR=255L
;window, 1
;plot, max_force[index],   XTIT='mode idx', YTIT='max abs(force) [N]'
if keyword_set(KL) or keyword_set(PASSEDBASE) then $
    amp_modal = minmax((amp[index])[where(ptv_pos[index])])  $
else $
    amp_modal = minmax((amp[index])[where(ptv_pos[index])])/sqrt(n_elements(adsec.act_w_cl))

ptv = minmax((ptv_pos[index])[where(ptv_pos[index])])  

print, "ptv [nm], minmax: ", 1e9*ptv
print, "modal amp [nm], minmax: ", 1e9*minmax((amp[index])[where(ptv_pos)])
print, "peak force [N], minmax: ", minmax((max_force[index])[where(ptv_pos)])

if max(abs(ptv)) gt 1.6e-6  and (keyword_set(PASSEDBASE) or keyword_set(KL))then begin
    log_print, "Provided base gives a PTV too large (bad normalization?)"
    return, adsec_error.generic_error
endif

    if keyword_set(send) then begin
        err = set_disturb(/reset, /counter)
        if err ne adsec_error.ok then return,err
        err = set_disturb(fhist)
        if err ne adsec_error.ok then return,err
    endif
  
    if keyword_set(SAVEFILE) then begin
        adsec_save = adsec
        amp = amp[index]
        filename=meas_path('disturbance')
        ww=strsplit(filename, '/', /ex)
        track=ww[n_elements(ww)-1]
        filename= filename+'disturb'+type_str+string(from, FORMAT='(I3.3)')+'_'+string(to, FORMAT='(I3.3)')+ext0+'.sav'
        save, file=filename,  adsec_save, fhist, template, amp, from, to , type_str, n_heading_zeros, n_trailing_zeros, trigger_mode, amp_trigger, template_dec, dec_factor, cmd_mat; mod RB @magellan. added saving cmd_mat
    endif

    return, adsec_error.ok
    
    
end
     
