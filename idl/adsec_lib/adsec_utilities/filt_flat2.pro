;file2 = '/towerdata/adsec_calib/CMD/shape/flat_sbagliato_dopo.sav'
;file1 = '/towerdata/adsec_calib/CMD/shape/flat_sbagliato_prima.sav'
;Pro appunti
;    @adsec_common
;file1 = '/towerdata/adsec_calib/CMD/shape/refshape.sav'
;file2 = '/towerdata/adsec_calib/CMD/shape/flat_data_OAT_calibrated.sav'
;file2 = filepath(ROOT=adsec_path.conf, SUB='data', '600modes_2009_08_10.sav')
;file1 = filepath(ROOT=adsec_path.conf, SUB='data', 'flat_data_OAT_calibrated.sav')

;file2 = filepath(ROOT=adsec_path.conf, SUB='data', 'flat_data_wfs_calibrated.sav')
function filt_flat2, sys_status_flat, sys_status_ini, DISPL=disp_plot, INIT=file1, FLAT=file2, ACT_FILTERED=act_removed_list

    @adsec_common
    if n_elements(file1) gt 0 then begin
        restore, file1, /ver
        flattened_status=sys_status_ini
        f1 = flattened_status
        restore, file2, /ver
        f2 = flattened_status
    endif else begin
        f1 = sys_status_ini
        f2 = sys_status_flat
    endelse

    disp=keyword_set(disp_plot)
    disp=0
    wxs = 640
    wys = 480

    cl1 = where(f1.closed_loop)
    cl2 = where(f2.closed_loop)
    err = intersection(cl1, cl2, clact)
    
    ;IPOTESI: non c'e' stato cambi odi attuatori?? da pensare se funziona con meno attuatori....



    ;SET TO FLAT STATUS THE SAME PISTON IN POSITION AND FORCES OF PRESENT STATUS 
    ;POSITION
    f2_nopist = f2
    pist_pos=replicate(1.0,1,adsec.n_actuators)
    mm = transpose(adsec.ff_p_svec) ## pist_pos
    newpist = adsec.ff_p_svec[0:8,*] ## mm[0,0:8]
    deltapist = mean(f2.position[clact]) - mean(f1.position[clact]) 
    f2_nopist.position[clact] -= (deltapist*newpist[clact])
    ;FORCE: to be implemented with first forces modes in order to correct gravity effect (instead of simple force piston)
    ;to be understood if needed or if the procedure does it by itself
    f2_nopist.current[clact] += (-mean(f2_nopist.current[clact]) + mean(f1.current[clact]))

    dcurr = fltarr(adsec.n_actuators)
    dpos = fltarr(adsec.n_actuators)
    ;QUI CI VA L'EVENTUALE RIMANEGGIAMENTO DELLE FORZE di F2 DOVUTO ALLA matrice di FF CAMBIATA...

    dcurr[clact]= (f2_nopist.current[clact]-f1.current[clact])
    dpos[clact]= (f2_nopist.position[clact]-f1.position[clact])
    
    fcmd = dpos
    opt_fcmd = fcmd
    df = adsec.ff_matrix ## fcmd
    ftot = df + f1.current
;    idact = where(abs(ftot) gt (abs(f2_nopist.current)))
    act_removed = 0
    dlist = clact
    aa = fltarr(672)
        
    while (max(ftot) gt (max(f2_nopist.current))) or $
          (min(ftot) lt (min(f2_nopist.current))) do begin
    
        idact = where(abs(ftot[dlist]) eq max(abs(ftot[dlist])))
        log_print, "Found actuator DSP# "+string(dlist[idact], FORMAT='(I3.3)') + " with force [N] " $ 
               +string(f2_nopist.current[dlist[idact]], FORMAT='(F5.2)') + " and tot force [N] " $
               +string(f2_nopist.current[dlist[idact]]+ftot[dlist[idact]], FORMAT='(F5.2)')
        

        if act_removed eq 0 then begin
            act_removed_list = dlist[idact]
        endif else begin
            act_removed_list = [act_removed_list, dlist[idact]]
        endelse
        dummy = complement(dlist[idact], dlist, tmp_dlist)

        ;ITERATION IN THE NEW LIST SET
        idbad = [act_removed_list, adsec.act_wo_cl]
        dummy = complement(idbad, indgen(adsec.n_actuators), idnotbad)
        dummy = ff_matrix_reduction(adsec.ff_matrix, idbad, dx, k00, k01, k10, k11)

        df1 = transpose(df[idnotbad])
        dc1 = transpose(fcmd[idnotbad])
        df0 = transpose(dcurr[idbad])
        dc0 = pseudo_invert(k00) ## (df0 - k01 ## dc1)

        fcmd = fltarr(adsec.n_actuators)
        fcmd[idbad] = dc0
        fcmd[idnotbad] = dc1

        ;OPTIMUM FLAT DECOMPOSITION OF DELTA: REMOVE VERY HIGH ORDER MODES with 3 nm RMS
;------------------------------------
        thr=1d-9
        mode_idx = indgen(n_elements(clact))
        n_mode_idx = n_elements(clact)
        fcmd_mod = transpose(adsec.ff_p_svec[mode_idx,*])##transpose(fcmd)
        comm_filt_vec = fltarr(adsec.n_actuators,n_mode_idx)
        
        for i=0,n_mode_idx-1 do begin
            dummy_p = adsec.ff_p_svec[mode_idx[i],*]*fcmd_mod[i]
            if i eq 0 then begin
                comm_filt_vec[*,i]=dummy_p
            endif else begin
                comm_filt_vec[*,i]=comm_filt_vec[*,i-1]+dummy_p
            endelse
        endfor
        
        ;NOTE: res_rms and force_rms are computed WITHOUT removing the average value.
        res_filt_vec = comm_filt_vec-rebin(fcmd, adsec.n_actuators, n_mode_idx)
        res_rms = sqrt(total(res_filt_vec[clact,*]^2,1)/n_elements(clact))
        
        dd = min(abs(res_rms-thr),idx)
        max_mode = idx

        opt_fcmd= comm_filt_vec[*,max_mode]
    ;    log_print, "max mode used: "+string(max_mode, format='(I3.3)')
;-----------------------------------
        df = adsec.ff_matrix ## opt_fcmd
   ;     log_print, "pistone di forza f1 "+string(mean(f1.current[clact]), FORMAT='(f12.9)')
   ;     log_print, "pistone di forza ftot "+string(mean(ftot[clact]), FORMAT='(f12.9)')
   ;     log_print, "pistone di forza df "+string(mean(df[clact]), FORMAT='(f12.9)')
   ;     log_print, "pistone di posizione fcmd "+string(mean(opt_fcmd[clact]), FORMAT='(f12.9)')
        ftot = df + f1.current
        act_removed +=1
        dlist = tmp_dlist
        if act_removed gt 50 then begin
            log_print, "TOO much actuators removed. Stopped force feedback procedure"
            log_print, "No force feedback will be used."
            return, dpos
        endif
    endwhile


    
        if disp then begin
             
            loadct, 12

            window, /free, xs=wxs, ys=wys
            display, dpos[clact], clact, /sh, /no_n, TITLE="ORIGINAL DCMD REQUEST FOR FLAT"
            window, /free, xs=wxs, ys=wys
            display, dcurr[clact], clact, /sh, /no_n, TITLE="ORIGINAL DCURR REQUEST FOR FLAT"

            window, /free, xs=wxs, ys=wys
            display, fcmd[clact], clact, /sh, /no_n, TITLE="DELTA COMMAND COMPUTED FOR FLAT"
            window, /free, xs=wxs, ys=wys
            newff = adsec.ff_matrix ## fcmd
            display, newff[clact], clact, /sh, /no_n, TITLE="FORCE COMPUTED FOR FLAT"

            mostra, act_removed_list,  TITLE="IDENTIFIED BAD ACTUATORS"

            window, /free, xs=wxs, ys=wys
            display, (adsec.ff_matrix ## fcmd + f1.current)[clact], clact, /sh, /no_n, TITLE="NEW TOTAL FORCE FOR FLAT"
            window, /free, xs=wxs, ys=wys
            display, (f2_nopist.current)[clact], clact, /sh, /no_n, TITLE="ORIGINAL FORCE SAVED INTO FLAT FILE"

            window, /free, xs=wxs, ys=wys
            plot,  dpos[act_removed_list], ps=-4, TITLE='COMMANDS ON BAD ACTUATORS'
            oplot, fcmd[act_removed_list], col='0000ff'xl, ps=-4 
;            oplot, fcmd_null[act_removed_list], ps=-4, col='00ffff'xl 

        endif
    return, fcmd
end
