;$Id: time_hist_reduce.pro,v 1.8 2008/03/12 14:56:33 marco Exp $
;+
;HISTORY
; written by M.Xompero(MX)
;-
;$Id: time_hist_reduce.pro,v 1.8 2008/03/12 14:56:33 marco Exp $
;+
;HISTORY
; written by M.Xompero(MX)
;-


Pro time_hist_reduce, FULL=full, SLAVES=slaves, PATH=path

    @adsec_common
    if n_elements(path) eq 0 then path = '~/adopt/measures/adsec_calib/disturbance/20120530_155122/'
    path = '~/adopt/measures/adsec_calib/disturbance/20120530_155122/'
    ;path = '/media/disk/adsec672a/meas/disturbance/2008_01_30/'
    file0 = path+'telescope_CR_90deg_disturb_1000hz_TS5.sav'
	if keyword_set(slaves) then begin
		print, "sono slaves"
		ext='slaves_' 
	endif else ext='' 
	ffout = path+ext+'fe_modes_reduced_10deg_icc.sav'
	print, ffout
    restore, file0,  /ver

    ; quando e' preso questo sys_status???
    ss = status_save.sys_status
    fb = transpose(ss.full_bias_command)
    bi = transpose(ss.bias_command)
    ;, togliere subito gli offset giusti!!!
    ndc =  transpose(data_reduced.new_delta_command)
    ffc =  transpose(data_reduced.ff_command)
    pos =  transpose(data_reduced.dist_average)
    curr = transpose(data_reduced.curr_average)
    bc = transpose(ss.bias_current)
    ;total_cmd = ndc + rebin(transpose(status_save.sys_status.command), nel0, na_tot)
    tvc = data_reduced_sw.timestamp

    nel0  = max(size(pos, /DIM)) ; number of acquired temporal samples
    nelth  = max(size(time_hist, /DIM)) ; number of temporal samples in the WF history
    na_tot = status_save.adsec.n_actuators

    skip_test = data_reduced.mirror_frames_counter[1:nel0-1]-data_reduced.mirror_frames_counter[0:nel0-2]
    if total(skip_test ne 1) ne 0 then message, "ERROR: some mirror frame has been skipped", /INFO
    
    cfc = data_reduced.mirror_frames_counter
    sfc = data_reduced_sw.mirror_frames_counter

    ;id1 = where(cfc eq sfc[0])
    ;if id1[0] eq -1 then 
    id1=0

    id  = where(ndc[*,0] eq 0.0)
    if id[0] eq -1 then id = nel0

    nel = id[0]-id1[0]

    ndc  =  ndc[id1:nel-1,  *]
    ffc  =  ffc[id1:nel-1,  *]
    pos  =  pos[id1:nel-1,  *]
    curr =  curr[id1:nel-1, *]
    tvc  =  tvc[id1:nel-1]
    cfc  =  cfc[id1:nel-1]
    sfc  =  sfc[0:nel-1]

    tvc -= tvc[0]
    undefine, data_reduced
    undefine, data_reduced_sw
    mismatch_test = sfc - cfc
    if total(mismatch_test) gt 0 then begin
        message, "ERROR: mismatch between switchBCU and crateBCU data", /INFO
        if total(tvc[1:*]-tvc[0:nel-2]-mean(tvc[1:*]-tvc[0:nel-2]) gt 10e-6 ) gt 0 then message, "Excessive jitter in timestap vector", /INFO
        tvc = indgen(nel)*mean(tvc[1:*]-tvc[0:nel-2])

    endif

    rms_pos = fltarr(na_tot)
    rms_curr = fltarr(na_tot)
   
    ;time history phasing
    id = where(time_hist[*,0] eq ndc[0,0]) & print, id
    th_ph = shift(time_hist, -id, 0)
    col = comp_colors(6)
    
    ndc_fb0  = ndc + rebin(fb-bi, nel, na_tot, /SAMP)
    pos_fb0  = pos - rebin(bi, nel, na_tot, /SAMP)
    curr_fb0 = curr - rebin(bc, nel, na_tot, /SAMP)

    act_w_cl = status_save.adsec.act_w_cl
	if keyword_set(slaves) then begin
print, "sono slaves"
	;patch for slaves
    tmp1 = adsec.mir_act2act[indgen(total(adsec_shell.n_act_ring[0:3]))]
	notcl = [adsec.act_wo_cl, tmp1]
	slavid = notcl[uniq(notcl, sort(notcl))]
	print,complement(notcl, indgen(672), masid)
 	na = n_elements(masid)
	
	ffred = ff_matrix_reduction(adsec.ff_matrix, slavid, dx, k00, k01, k10, k11)
	svdc, k11, wred, ured, vred, /double
	idred = sort(wred)
	ured = ured[idred, *]
	vred = vred[idred, *]
	wred = wred[idred]
	tmp = dblarr(na, 672)
	uff = dblarr(672, 672)
	tmp[*,masid] = ured 
	uff[0:na-1, *] = tmp
	vff = dblarr(672, 672)
	tmp[*,masid] = vred 
	vff[0:na-1, *] = tmp
	fff = dblarr(672, 672)
	tmp[*,masid] = ffred 
	fff[masid, *] = tmp
	print,"cambiata uff e vff"
;patch for slaves
	endif else begin
		na = n_elements(act_w_cl)
	endelse

    
    ;;;; OCCHIO A CONSIDERARE BENE IL RITARDO!!!!
    if keyword_set(FULL) then begin
        strfull = 'tps_'
        pos_fb0_full = temporary(curr_fb0)
        ax = status_save.adsec.act_coordinates[0,*]
        ay = status_save.adsec.act_coordinates[1,*]
        for i=0, nel-1 do begin
            full_pos = min_curve_surf(pos_fb0[i,act_w_cl], ax[act_w_cl], ay[act_w_cl] ,/DOUBLE, /TPS $
                                     , XOUT=ax, YOUT=ay)
        endfor
        pos_err = (pos_fb0_full - ndc_fb0)  ;; =pos-ndc_fb
    endif else begin
        strfull = ''
        pos_err = (pos_fb0 - ndc_fb0)  ;; =pos-ndc_fb
     

       
        rms_pos_err_v = float(sqrt(total(pos_err[*,act_w_cl]^2, 2, /DOUBLE)/na $
                     -(total(pos_err[*,act_w_cl], 2, /DOUBLE)/na)^2))
        rms_pos_fb0_v = float(sqrt(total(pos_fb0[*,act_w_cl]^2, 2, /DOUBLE)/na $
                     -(total(pos_fb0[*,act_w_cl], 2, /DOUBLE)/na)^2))
        rms_ndc_fb0_v = float(sqrt(total(ndc_fb0[*,act_w_cl]^2, 2, /DOUBLE)/na $
                     -(total(ndc_fb0[*,act_w_cl], 2, /DOUBLE)/na)^2))
        rms_curr_v = float(sqrt(total(curr[*,act_w_cl]^2, 2, /DOUBLE)/na $
                     -(total(curr[*,act_w_cl], 2, /DOUBLE)/na)^2))
        peak_curr_v = max(abs(curr[*,act_w_cl]),DIM=2)
        rms_pos_err= float(sqrt(total(pos_err[*,act_w_cl]^2, 1, /DOUBLE)/nel $
                     -(total(pos_err[*,act_w_cl], 1, /DOUBLE)/nel)^2))
        rms_pos_fb0= float(sqrt(total(pos_fb0[*,act_w_cl]^2, 1, /DOUBLE)/nel $
                     -(total(pos_fb0[*,act_w_cl], 1, /DOUBLE)/nel)^2))
        rms_ndc_fb0= float(sqrt(total(ndc_fb0[*,act_w_cl]^2, 1, /DOUBLE)/nel $
                     -(total(ndc_fb0[*,act_w_cl], 1, /DOUBLE)/nel)^2))
        peak_curr=max(abs(curr[*,act_w_cl]),DIM=1)
        rms_curr = float(sqrt(total(curr[*,act_w_cl]^2, 1, /DOUBLE)/nel $
                     -(total(curr[*,act_w_cl], 1, /DOUBLE)/nel)^2))
        rms_pos_err0=sqrt(mean(rms_pos_err_v^2))
    ;rms_pos0=sqrt(mean(rms_pos_v^2))
    ;rms_ndc0=sqrt(mean(rms_ndc_v^2))
        rms_curr0=sqrt(mean(rms_curr_v^2))
        peak_curr0=max(peak_curr_v)
    
                               ;tolgo il comando di bias dalla
;posizione perche' non ho la posizione di full bias e la decompongo in
;modi (E' GIUSTO TOGLIERE SOLO IL BIAS, IL COMANDO DI OFFLOAD NON DEVE
;ESSERE TOLTO (???))
if keyword_set(slaves) then begin
		print, "sono slaves"
        md_pos_fb0 = transpose(vff[0:na-1,*]) ## pos_fb0
        md_pos_fb0_spec = sqrt(total(md_pos_fb0^2, 1, /DOUBLE)/nel $
                            -(total(md_pos_fb0, 1, /DOUBLE)/nel)^2) / sqrt(na_tot)
        md_cmd = transpose(vff[0:na-1,*]) ## ndc_fb0
        md_cmd_spec =sqrt(total(md_cmd^2, 1, /DOUBLE)/nel $
                        -(total(md_cmd, 1, /DOUBLE)/nel)^2) / sqrt(na_tot)
        md_pos_err_spec =sqrt(total((md_pos_fb0-md_cmd)^2, 1, /DOUBLE)/nel $
                         -(total(md_pos_fb0-md_cmd, 1, /DOUBLE)/nel)^2) / sqrt(na_tot)
        md_curr = transpose(uff[0:na-1,*]) ## curr
    
endif else begin
    ;DECOMPOSIZIONE POSIZIONE
        md_pos_fb0 = transpose(status_save.adsec.ff_p_svec[0:na-1,*]) ## pos_fb0
        md_pos_fb0_spec = sqrt(total(md_pos_fb0^2, 1, /DOUBLE)/nel $
                            -(total(md_pos_fb0, 1, /DOUBLE)/nel)^2) / sqrt(na_tot)

    ;DECOMPOSIZIONE COMANDO
        md_cmd = transpose(status_save.adsec.ff_p_svec[0:na-1,*]) ## ndc_fb0
        md_cmd_spec =sqrt(total(md_cmd^2, 1, /DOUBLE)/nel $
                        -(total(md_cmd, 1, /DOUBLE)/nel)^2) / sqrt(na_tot)
    
    ;DECOMPOSIZIONE ERRORE DI POSIZIONE
        md_pos_err_spec =sqrt(total((md_pos_fb0-md_cmd)^2, 1, /DOUBLE)/nel $
                         -(total(md_pos_fb0-md_cmd, 1, /DOUBLE)/nel)^2) / sqrt(na_tot)
    
    
    ;vec = dblarr(672) & for i=0,671 do vec[i] = sqrt(kolm_covar(i+2, i+2)) * (8.254/(5e-7/(0.65/206265.)))^(5./6.) * 5e-7 /2

    ;DECOMPOSIZIONE FORZA
    ;tolgo dalle forze la SOLA forza di bias (LA FULL BIAS = OFFLOAD FORZA la tengo)
        md_curr = transpose(status_save.adsec.ff_f_svec[0:na-1,*]) ## curr
endelse

        md_pos_err_rms = fltarr(na+1)  ;;; primo elemento del vettore nessuna correzione
        md_cmd_rms = fltarr(na+1)  ;;; primo elemento del vettore nessuna correzione
        md_curr_rms = fltarr(na+1)  ;;; primo elemento del vettore nessuna correzione
        md_curr_peak = fltarr(na+1)  ;;; primo elemento del vettore nessuna correzione
        temp_cmd = fltarr(nel, na_tot)
        temp_pos_fb0 = fltarr(nel, na_tot)
        temp_curr = fltarr(nel, na_tot)

    ;CASO BASE: a zero modi corretti, ho tutto l'RMS del comando come ERRORE DI POSIZIONE
    ;mentre l'RMS del comando e' zero
        md_pos_err_rms[0]=sqrt(mean(total(ndc_fb0[*,act_w_cl]^2, 1, /DOUBLE)/nel $
                          -(total(ndc_fb0[*,act_w_cl], 1, /DOUBLE)/nel)^2))
    
        for i=0,na-1 do begin
        
            print, "   "+strtrim(i,2)+"/"+strtrim(na-1, 2)
			if keyword_set(slaves) then begin
					if i eq 0 then print, "SLAVES!!"
					temp_pos_fb0 += vff[i,*]##md_pos_fb0[*,i]
					temp_curr += uff[i,*]##md_curr[*,i]
					temp_cmd += vff[i,*]##md_cmd[*,i]
			endif else begin
					temp_pos_fb0 += status_save.adsec.ff_p_svec[i,*]##md_pos_fb0[*,i]
					temp_curr += status_save.adsec.ff_f_svec[i,*]##md_curr[*,i]
					temp_cmd += status_save.adsec.ff_p_svec[i,*]##md_cmd[*,i]
			endelse
            temp_err = temp_pos_fb0-ndc_fb0
            md_pos_err_rms[i+1]=sqrt(mean(total(temp_err[*,act_w_cl]^2, 1, /DOUBLE)/nel $
                                -(total(temp_err[*,act_w_cl], 1, /DOUBLE)/nel)^2))
            md_curr_rms[i+1]=sqrt(mean(total(temp_curr[*,act_w_cl]^2, 1, /DOUBLE)/nel $
                             -(total(temp_curr[*,act_w_cl], 1, /DOUBLE)/nel)^2))
            md_curr_peak[i+1]=max(abs(temp_curr[*,act_w_cl]))
            md_cmd_rms[i+1] = sqrt(mean(total(temp_cmd[*,act_w_cl]^2, 1, /DOUBLE)/nel $
                              -(total(temp_cmd[*,act_w_cl], 1, /DOUBLE)/nel)^2))

        end   
        save, file=ffout, rms_pos_err_v, rms_pos_fb0_v, rms_ndc_fb0_v,  rms_curr_v $
                , peak_curr_v, rms_pos_err, rms_pos_fb0, rms_ndc_fb0, peak_curr, rms_curr $
                , md_pos_err_rms, md_curr_rms, md_curr_peak, tvc, md_cmd_rms, ndc_fb0, curr_fb0, pos_err, pos_fb0  $
                , md_cmd, md_pos_fb0, md_curr, md_pos_err_spec, md_cmd_spec, md_pos_fb0_spec, act_w_cl, vff, uff, ffred
        
    
    ;mode_dec_ndc = transpose(status_save.adsec.ff_p_svec) ## ndc_fb
    ;mode_dec_pos = transpose(status_save.adsec.ff_p_svec) ## ndc

    ;    for i =0, n_elements(adsec.act_w_cl) do begin
    ;        print, i
    ;        
    ;    endfor
    endelse

end

Pro time_hist_reduce_plot, SLAVES=slaves, path=path

    if n_elements(path) eq 0 then path = '~/adopt/measures/adsec_calib/disturbance/20120530_155122/'
    if keyword_set(slaves) then begin
		ext='slaves_' 
		restore, path+ext+'fe_modes_reduced_10deg_icc.sav', /ver
    	mv = indgen(500)+1
	endif else begin
		ext=''
		restore, path+ext+'fe_modes_reduced_10deg_icc.sav', /ver
    	mv = indgen(n_elements(act_w_cl))+1
	endelse
    ;restore, path+'act_w_cl.sav'
    set_plot_defaults, /WHITE
    xs=1024;800
    ys=768;600
    do_jpeg = 1
    act=0
    strgap = ' @60[!4l!3m] mean gap'
    ;restore, path+'caos_par.sav'
    ;path = path + '10deg_icc_fixed_'

    ;TIME_HISTORY tracking
    window, /FREE, xs=xs, ys = ys
    plot, tvc, 1e6*ndc_fb0[*, act], XTIT="[s]", YTIT="[!4l!3m]" $
          , TIT="Time history and displacement act#"+strtrim(act,2)
    ;oplot, tvc, 1e6*pos_fb0[*, act],COL=255L
    oplot, tvc, 1e6*pos_err[*, act], COL=255L
    if do_jpeg then write_jpeg, path+ext+'plot_TypicalActHist.jpg', tvrd(true=3), true=3, quality=100

    window, /FREE, xs=xs, ys=ys
    plot,  tvc, 1e9*pos_err[*, act], XTIT="[s]", YTIT="[nm]" $
          , TIT="Position error act#"+strtrim(act,2)
    if do_jpeg then write_jpeg, path+ext+'plot_TypicalActPosErr.jpg', tvrd(true=3), true=3, quality=100
    
    window, /FREE, XS=xs, YS=ys
    plot, tvc, 1e9*rms_pos_err_v, XTIT="[s]", YTIT="[nm]", TITLE='Position Error RMS'+strgap
    if do_jpeg then write_jpeg, path+ext+'plot_PosErrVsTime.jpg', tvrd(true=3), true=3, quality=100
    

    window, /FREE, XS=xs, YS=ys
    plot, mv, md_curr_peak, PSYM=10, YR=minmax([md_curr_peak, md_curr_rms]), YTICKF='(f5.2)'  $
        , XTIT='# corrected modes', YTIT='[N]', TIT='Peak/RMS Force VS CorrectedModes'+strgap
    oplot, mv, md_curr_rms, PSYM=10, COL=255l
    xyouts, 500, 0.075, "RMS", COL=255l 
    xyouts, 500, 0.47, "Peak"
    if do_jpeg then write_jpeg, path+ext+'plot_ForceVSCorrectedModes.jpg', tvrd(true=3), true=3, quality=100
    

;QUESTO NON VA
    window, /FREE, XS=xs, YS=ys
    plot, mv, md_cmd_spec*1e6, PSYM=10, YR=minmax([md_cmd_spec, md_pos_err_spec])*1e6  $
        , XTIT='# corrected modes', YTIT='[!4l!3m]', TIT='Modal Spectrum'+strgap, /YLOG, /XLOG
    oplot, mv, md_pos_err_spec*1e6, PSYM=10 , COL=255l
    xyouts, 500, 3e-4, "PosErr", COL=255l 
    xyouts, 500, 2e-3, "Cmd"
    if do_jpeg then write_jpeg, path+ext+'plot_CmdPosErrModalSpectrum.jpg', tvrd(true=3), true=3, quality=100
    

; QUESTO NON FUNZIONA!!!
    window, /FREE, XS=xs, YS=ys
    col = [255L, 255L*256L]
    plot, md_pos_err_rms*1e6 *2, /YLOG, YTICKF = '(f6.3)'    $
        , XTIT="# of corrected modes", YTIT="[!4l!3m]" $
        , TIT="WF Position error RMS"+strgap +         $
        '(MIN: '+string(min(md_pos_err_rms)*1e9*2, format='(f5.2)')+' [nm])'
    oplot, [-10, 680], [50e-9, 50e-9]*1e6, COL=col[0], LINE=2
    oplot, [-10, 680], [28e-9, 28e-9]*1e6, COL=col[1], LINE=2
    xyouts, 500, 52.1e-9*1e6, "SPEC", COL=col[0]
    xyouts, 500, 30e-9*1e6, "GOAL", COL=col[1]
    md_0 = (where(md_pos_err_rms*1e6*2 lt 50e-9*1e6))[0]
    md_1 = (where(md_pos_err_rms*1e6*2 lt 28e-9*1e6))[0]
    oplot, [md_0, md_0], [1e-2, 10], COL=col[0], LINE=1
    oplot, [md_1, md_1], [1e-2, 10], COL=col[1], LINE=1
    xyouts, md_0+10, 2, strtrim(md_0, 2), COL=col[0]
    xyouts, md_1+10, 2, strtrim(md_1, 2), COL=col[1]
    
    if do_jpeg then write_jpeg, path+ext+'plot_WFPosErrVSModes.jpg', tvrd(true=3), true=3, quality=100

    ids = reverse(sort(md_cmd_spec))
    turb = dblarr(n_elements(act_w_cl))
	r0fqp = 0.5e-6/(0.8d/206265)
	lafqp = 0.5e-6
    for i =0, n_elements(act_w_cl)-1 do turb[i] = sqrt(kolm_covar(i+2, i+2)) * (6.5/r0fqp)^(5./6.) * lafqp /2

   ; for i =0, n_elements(act_w_cl)-1 do turb[i] = sqrt(kolm_covar(i+2, i+2)) * (lbt.D/lbt.r0)^(5./6.) * lbt.lambda /3.

    window, /FREE, XS=xs, YS=ys
    plot, mv, md_cmd_spec[ids]*1e6, PSYM=10, YR=minmax([md_cmd_spec, md_pos_err_spec])*1e6, /XLOG, /YLOG $
        , XTIT='# corrected modes', YTIT='[!4l!3m]', TIT='Modal Spectrum RMS Ordered'+strgap
    oplot, mv, md_pos_err_spec[ids]*1e6, PSYM=10 , COL=255l
    oplot, mv, turb*1e6, PSYM=10 , COL=255l*256L
    xyouts, 500, 3e-4, "PosErr", COL=255l 
    xyouts, 500, 2e-3, "Cmd"
    xyouts, 500, 3e-3, "Kolm", COL=255l*256 
    if do_jpeg then write_jpeg, path+ext+'plot_CmdPosErrModalSpectrumOrdered.jpg', tvrd(true=3), true=3, quality=100


    window, /FREE, XS=xs, YS=ys
    display, abs(peak_curr), act_w_cl, /SH, /NO_N, TIT="Peak Abs(Force)"+strgap, YTICKF_BAR='(f5.2)'  $
           , OUT_VAL='FFFFFF'xl, BAR_TITLE='[N]', XTIT='[mm]', YTIT='[mm]'
    if do_jpeg then write_jpeg, path+ext+'display_ForcePeak.jpg', tvrd(true=3), true=3, quality=100

    window, /FREE, XS=xs, YS=ys
    display, rms_curr, act_w_cl, /SH, /NO_N, TIT="Force RMS"+strgap, YTICKF_BAR='(f5.2)'  $
           , OUT_VAL='FFFFFF'xl, BAR_TITLE='[N]', XTIT='[mm]', YTIT='[mm]'
    if do_jpeg then write_jpeg, path+ext+'display_ForceRMS.jpg', tvrd(true=3), true=3, quality=100
    
     ;DISPLAY PtV COMMAND
    dmin = (min(ndc_fb0, dim=1))[act_w_cl]
    dmax = (max(ndc_fb0, dim=1))[act_w_cl]
    window, /FREE, XS=xs, YS=ys
    ;display, 1e6*rms_ndc_fb0, act_w_cl, /SH, /NO_N, TIT="command rms [um]"
    display, 1e6*(dmax-dmin), act_w_cl, /SH, /NO_N, BAR_TIT="[!4l!3m]", YTICKF_BAR='(f5.2)' $
           , TITLE='Command PtV'+strgap, XTIT='[mm]', YTIT='[mm]', OUT_VAL='ffffff'xl
    if do_jpeg then write_jpeg, path+ext+'display_commandPtV.jpg', tvrd(true=3), true=3, quality=100

    window, /FREE, XS=xs, YS=ys
    display, 1e9*rms_pos_err, act_w_cl, /SH, /NO_N, BAR_TIT="[nm]", YTICKF_BAR='(f5.1)'     $
           , XTIT='[mm]', YTIT='[mm]', TITLE='Tracking error'+strgap, OUT_VAL='ffffff'xl
    if do_jpeg then write_jpeg, path+ext+'display_trackingError.jpg', tvrd(true=3), true=3, quality=100
    
    
    
end
