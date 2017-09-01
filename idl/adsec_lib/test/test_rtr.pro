;script per testare il ricostruttore
pro test_rtr, fly=fly,  block=block
@adsec_common
slc=0
nstep=20
if ~keyword_set(fly) then begin
    block=0
    err = clear_dl()

    ;setting block
    err= set_block(block)
    if err then message, 'Il blocco non è stato cambiato'
endif
                                ;set delay line casuali
sdl = randomn(seed,rtr.n_slope*rtr.n_slope_delay) 
ddl = randomn(seed,adsec.n_actuators*rtr.n_modes_delay)
err = write_same_dsp(sc.all, rtr_map.slope_delay, sdl)
err = write_same_dsp(sc.all, rtr_map.modes_delay, ddl)

                                ;set matrice b0 casuale
b0 = randomn(seed,rtr.n_slope, adsec.n_actuators)
err = set_b0_matrix(b0, block=block)

                                ;set matrici bi casuali
;bi = randomn(seed,rtr.n_slope, adsec.n_actuators, rtr.n_slope_delay) 
bi = fltarr(rtr.n_slope, adsec.n_actuators, rtr.n_slope_delay)+1 
err = set_b_delay_matrix(bi, block=block)

                                ;set matrici ai casuali
;ai = randomn(seed,adsec.n_actuators, adsec.n_actuators, rtr.n_slope_delay)
;ai = fltarr(adsec.n_actuators, adsec.n_actuators, rtr.n_slope_delay)
ai=rebin(diagonal_matrix(fltarr(adsec.n_actuators)+0.1),adsec.n_actuators, adsec.n_actuators, rtr.n_slope_delay)
err = set_a_delay_matrix(ai)

                                ;set gains casuali
gs = randomn(seed,adsec.n_actuators)*0+1e-5
err = set_g_gain(sc.all, gs, block=block)
;err = set_nendof()

for i = 0,nstep-1 do begin 
    
    if slc then begin
                                ;smread qualcosa
                                ;slope = ....
    endif else begin

        slope = randomn(seed, rtr.n_slope)
                                ;scrivo la slope sul ricostruttore
        err = write_same_dsp(sc.all,rtr_map.slope_vector, slope)
        
                                ;start_rtr
        err = start_rtr()
        ;wait, 0.1

                                ;scrivo i modi sul mode_vector
        err = read_seq_ch(sc.all,dsp_map.modes, 1l,mf_orig)
                                ;a tutti i passi tranne allo zero somma i d ed e correnti  

        err = write_same_dsp(sc.all,rtr_map.modes_vector, mf_orig)
        
                                ;start_mm
        err = start_mm()
        ;wait, 0.1
                                ;start_ff
        err = start_ff()


    endelse
                                ;a tutti i passi tranne allo zero somma i d ed e correnti
        if i gt 0 then begin
            result = filtro_rtr(gs,b0,slope,d,e, /idl)
            err = read_seq_ch(sc.all,dsp_map.modes, 1l,mf)
            perc = abs(mf - double(result))/abs(double(mf))
            ok = perc gt 1e-2
            print, minmax(mf)
            print, minmax(result)
            print, minmax(perc)
            if total(ok) eq 0 then print, 'Step #'+string(i, format='(I4.4)')+' ok.' else message, 'Filter fails! Test broken.'
        endif

;calcolo i nuovi d ed e, li leggo e li confronto
    
    wait, 0.1    ;ci vuole perche' altrimenti non funziona.....!!!!
    undefine, d, e
    err = read_seq_dsp(0,'180181'xl, 1l, c_slope, /ul)
    err = read_seq_dsp(0,'180182'xl, 1l, c_modes, /ul)
    counter_slope = ((c_slope - rtr_map.slope_delay) / rtr.n_slope)
    counter_modes = ((c_modes - rtr_map.modes_delay) / adsec.n_actuators)
    while (counter_slope gt 3) or (counter_slope lt 0) do begin
        err = read_seq_dsp(0,'180181'xl, 1l, c_slope, /ul)
        counter_slope = ((c_slope - rtr_map.slope_delay) / rtr.n_slope)
    endwhile
    while (counter_modes gt 3) or (counter_modes lt 0) do begin
        err = read_seq_dsp(0,'180182'xl, 1l, c_modes, /ul)
        counter_modes = ((c_modes - rtr_map.modes_delay) / adsec.n_actuators)
    endwhile

    print, 'Counter slope, '+string(counter_slope, format='(i2.2)')
    print, 'Counter deltapos, '+string(counter_modes, format='(i2.2)')
    err = calcola_de(d,e, counter_slope, counter_modes, /idl)


endfor

end

