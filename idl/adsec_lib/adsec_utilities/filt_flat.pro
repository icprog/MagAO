;file1 = '/towerdata/adsec_calib/CMD/shape/flat_sbagliato_dopo.sav'
;file1 = '/towerdata/adsec_calib/CMD/shape/flat_sbagliato_prima.sav'
;Pro appunti
;    @adsec_common
;file1 = '/towerdata/adsec_calib/CMD/shape/flat_sbagliato_dopo2.sav'
;    file2 = filepath(ROOT=adsec_path.meas, "/flat/2009_04_07/2009_04_07_20deg_optical_flat_with_100modes_opt_mat_2009_03_31_05deg.sav")

function filt_flat, sys_status_flat, sys_status_ini, DISPL=disp_plot, INIT=file1, FLAT=file2

    @adsec_common
    if n_elements(file1) gt 0 then begin
        restore, file1, /ver
        f1 = flattened_status
        restore, file2, /ver
        f2 = flattened_status
    endif else begin
        f1 = sys_status_ini
        f2 = sys_status_flat
    endelse

    disp=keyword_set(disp_plot)
    wxs = 640
    wys = 480

    cl1 = where(f1.closed_loop)
    cl2 = where(f2.closed_loop)
    print, "displ is "+string(disp)
    err = intersection(cl1, cl2, clact)
    
    ;IPOTESI: non c'e' stato cambi odi attuatori?? da pensare se funziona con meno attuatori....
        dcurr= (f2.current-f1.current)
        dpos= (f2.position-f1.position)
        old_idout = clact

        vv=(((adsec.ff_matrix ## dpos)-dcurr)[clact])

        h  = double(histogram(vv, loc=loc, nbin=100., REV=rev))

        gfit  = gaussfit(double(loc), h, gc, NTERMS=4)

;SOGLIA AUTOMATICA intersezione di %attuatori che dovrebbero essere fuori e %che sono fuori
;curva a sigma teorico sugli attuatori
        nel = 1000
        nclact = n_elements(clact)
        filt_vv = vv - gc[1]

        fitc = fltarr(nel)
        for i=0, nel-1 do begin 
            fact = 0.1*i 
            dummy  = where(abs(filt_vv) gt fact*gc[2], cc) 
            fitc[i] = cc/float(nclact) 
        endfor
        tmp = (findgen(nel))*0.1*gc[2]
        fitg= 1-(gauss_pdf(tmp/gc[2])-0.5)*2

        mm = (fitc-fitg)
        dummy = max(mm, idmax)
        th = tmp[idmax]*3

        idout  = where(abs(filt_vv ) gt th , nidout)

        if idout[0] eq -1   then begin
            log_print, "All ok, no reduction needed."
            return, dpos
        endif

        log_print, "MISCALIBRATED ACTUATORS:  "+strjoin(string(nidout, FORMAT='(" ",I3.3)'))

        err =  complement(clact[idout], clact, out_list)

        ;ITERATION IN THE NEW LIST SET
        idbad = [clact[idout], adsec.act_wo_cl]
        dummy = complement(idbad, indgen(adsec.n_actuators), idnotbad)

        dummy = ff_matrix_reduction(adsec.ff_matrix, idbad, dx, k00, k01, k10, k11)

        df = adsec.ff_matrix ## dpos

        df1 = transpose(df[idnotbad])
        dc1 = transpose(dpos[idnotbad])
        df0 = transpose(dcurr[idbad])
        dc0 = pseudo_invert(k00) ## (df0 - k01 ## dc1)
        df0_null = df0*0
        dc0_null =  pseudo_invert(k00) ## (df0_null - k01 ## dc1)

        fcmd = fltarr(adsec.n_actuators)
        fcmd[idbad] = dc0
        fcmd[idnotbad] = dc1

        fcmd_null = fltarr(adsec.n_actuators)
        fcmd_null[idbad] = dc0_null
        fcmd_null[idnotbad] = dc1


        if disp then begin

            window, /free, xs=wxs, ys=wys
            plot, loc, h, psym=10
            oplot, loc, gfit, psym=-4, col='ff'xl

            window, /free, xs=wxs, ys=wys
            display, vv, clact, /sh, /no_n, TITLE="DELTA FORCES FF-DCURR"

            window, /free, xs=wxs, ys=wys
            display, vv[out_list], clact[out_list], /sh, /no_n, TITLE="DELTA FORCE FILTERED"


            mostra, clact[idout],  TITLE="IDENTIFIED BAD ACTUATORS"

            window, /free, xs=wxs, ys=wys
            display, fcmd[clact], clact, /sh, /no_n, TITLE="NEW DELTA COMMAND FOR FLAT"
            window, /free, xs=wxs, ys=wys
            display, (adsec.ff_matrix ## fcmd)[clact], clact, /sh, /no_n, TITLE="NEW DELTA FORCE FOR FLAT"
            window, /free, xs=wxs, ys=wys
            display, (adsec.ff_matrix ## fcmd + f1.current)[clact], clact, /sh, /no_n, TITLE="NEW TOTAL FORCE FOR FLAT"
            window, /free, xs=wxs, ys=wys
            display, (adsec.ff_matrix ## fcmd_null + f1.current)[clact], clact, /sh, /no_n, TITLE="NEW TOTAL FORCE(NULL) FOR FLAT"

            window, /free, xs=wxs, ys=wys
            plot,  dpos[clact[idout]], ps=-4, TITLE='COMMANDS ON BAD ACTUATORS'
            oplot, fcmd[clact[idout]], col='0000ff'xl, ps=-4 
            oplot, fcmd_null[clact[idout]], ps=-4, col='00ffff'xl 

        endif
    return, fcmd
end
