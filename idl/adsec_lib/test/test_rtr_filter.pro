;Script to test the m calcolo, e calcolo and d calcolo
;pulizia G,b0,Bi,Ai
block = 0
test  = 4


;setting block
err= set_block(block)
if err then message, 'Il blocco non è stato cambiato'


print, 'Cleaning default matrix B0 block A...'
err = set_b0_matrix(fltarr(rtr.n_slope, adsec.n_actuators))
print, '... done.'

print, 'Cleaning default matrix B0 block B...'
err = set_b0_matrix(fltarr(rtr.n_slope, adsec.n_actuators), /block)
print, '... done.'

print, 'Cleaning default matrix B_delay block A...'
err = set_b_delay_matrix(*rtr.b_delay_matrix)
print, '... done.'


print, 'Cleaning default matrix B_delay block B...'
err = set_b_delay_matrix(*rtr.b_delay_matrix, /block)
print, '... done.' 

print, 'Cleaning default matrix A_delay...'
err = set_a_delay_matrix(*rtr.a_delay_matrix)
print, '... done.' 

print, 'Cleaning default matrix M2C...'
err = set_m2c_matrix(*rtr.m2c_matrix)
print, '... done.' 
    
print, 'Cleaning ff_command_vector...'
err = set_delta_position_vector(fltarr(48), /ch)
print, '... done.' 

print, 'Cleaning command_vector...'
err = set_position_vector(fltarr(48), /ch)
print, '... done.' 

print, 'Cleaning ff_command_vector...'
err = write_same_ch(sc.all, dsp_map.ff_pure_current, 0l, /ch)  
print, '... done.' 

print, 'Cleaning int_control_current...'
err = write_same_ch(sc.all, dsp_map.int_control_current, 0l, /ch)
print, '... done.' 

print, 'Cleaning G gain...'
err = set_g_gain(/reset_all)
print, '... done.'

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;;;;;;;;;;;;;;;;;;;;; da creare una funzione che pulisce tutto prima del
;nuovo bin e controlla che il loop sia aperto

err = clear_dl()
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;print, 'Setting the nendof to 1UL ...'
;err = set_nendof()


case test of

    0: begin
;f- G=0, a1=0,e controllo che rimanga cosi' 
        print, 'Test 00: set G nulli, Ai tutti nulli, S(f) random: Verifica m(f) rimanga nullo.'
        
        g_t00 = fltarr(adsec.n_actuators)
        err = set_g_gain(sc.all, g_t00, block=block)
        Sf_t00 = randomn(seed,rtr.n_slope)

        ;annullo Ai
        ad_t00 = fltarr(adsec.n_actuators, adsec.n_actuators, rtr.n_modes_delay)
        err = set_a_delay_matrix(ad_t00)
        err = write_same_dsp(sc.all,rtr_map.slope_vector, sf_t00)
        tmp = fltarr(adsec.n_actuators)
        
        for j=0, 99 do begin
            ;print, 'Setting the nendof to 1UL ...'
            ;err = set_nendof()
            print, 'Starting RTR #'+strtrim(string(j),2)
            err = start_rtr()
            print, 'Starting MM #'+strtrim(string(j),2)
            err = start_mm()
            print, 'Starting FF #'+strtrim(string(j),2)
            err = start_ff()
            cc=1
            err = read_seq_dsp(sc.all, rtr_map.start_ff, 1l, nendof_ff)
            while (total(abs(nendof_ff)) ne 0) or cc eq 10 do begin
                cc +=1
                err = read_seq_dsp(sc.all, rtr_map.start_ff, 1l, nendof_ff)
            endwhile
            if cc eq 10 then message, 'Conto modi non ancora finito' else print, 'Conto modi finito in '+strtrim(string(cc),2)+' tentativi'
            err = read_seq_ch(sc.all,dsp_map.modes,1l, mf_t00)
            tmp = tmp+mf_t00
        endfor
        
        err = read_seq_ch(sc.all,dsp_map.modes,1l, mf_t00)
        print, 'Test 00: set G nulli, Ai tutti nulli, S(f) random: Verifica m(f) rimanga nullo.'
        if total(abs(mf_t00)) eq 0 then $
          print, 'Test 00 passed.'  else message, 'Test 00 failed!'
        break
    end
    
;
;a- G=1, b0 random, S(f) random
;   start rtr e verifica che il modo sia corretto
    1: begin

        print, 'Test 01: set G a 1, B0 random, S(f) random verifica calcolo del modo corretto.'
        err = set_g_gain(sc.all, fltarr(adsec.n_actuators)+1, block=block)
        b0_t01 = randomn(seed,rtr.n_slope, adsec.n_actuators)
        Sf_t01 = randomn(seed,rtr.n_slope)
        err = set_b0_matrix(b0_t01, block=block)
        err = write_same_dsp(sc.all,rtr_map.slope_vector, sf_t01)
        ;annullo Ai
        ad_t01 = fltarr(adsec.n_actuators, adsec.n_actuators, rtr.n_modes_delay)
        err = set_a_delay_matrix(ad_t01)
        ;annullo Bi
        bd_t01 = fltarr(rtr.n_slope, adsec.n_actuators, rtr.n_slope_delay)
        err = set_b_delay_matrix(bd_t01)

        print, 'Starting RTR'
        err = start_rtr()
        print, 'Starting MM'
        err = start_mm()
        print, 'Starting FF'
        err = start_ff()
        cc=1
        err = read_seq_dsp(sc.all, rtr_map.start_ff, 1l, nendof_ff)
        while (total(abs(nendof_ff)) ne 0) or cc eq 10 do begin
            cc +=1
            err = read_seq_dsp(sc.all, rtr_map.start_ff, 1l, nendof_ff)
        endwhile
        if cc eq 10 then message, 'Conto modi non ancora finito' else print, 'Conto modi finito in '+strtrim(string(cc),2)+' tentativi'
        err = read_seq_ch(sc.all,dsp_map.modes, 1l, mf_t01)
        mf = mamano(b0_t01, sf_t01)
        ;relerr = (double(mf_t01)-dmf)/double(mf_t01)
        ;roundoff error tra sqrt(n) e n precisione di macchina, con n numero di operazioni
        ok = mf ne mf_t01
        if total(ok) eq 0 then print, 'Test 01: passed.' $
        else message, 'Test 01: failed!'
    end
    
    2: begin
;b- idem a sopra ma con guadagni random
        print, 'Test 02: set G casuale, B0 random, S(f) random verifica calcolo del modo corretto.'
        g_t02 = randomn(seed,adsec.n_actuators)
        err = set_g_gain(sc.all, g_t02, block=block)
        b0_t02 = randomn(seed,rtr.n_slope, adsec.n_actuators)
        Sf_t02 = randomn(seed,rtr.n_slope)
        err = set_b0_matrix(b0_t02, block=block)
        err = write_same_dsp(sc.all,rtr_map.slope_vector, sf_t02)
        ;annullo Ai
        ad_t02 = fltarr(adsec.n_actuators, adsec.n_actuators, rtr.n_modes_delay)
        err = set_a_delay_matrix(ad_t02)
        ;annullo Bi
        bd_t02 = fltarr(rtr.n_slope, adsec.n_actuators, rtr.n_slope_delay)
        err = set_b_delay_matrix(bd_t02)

        print, 'Starting RTR'
        err = start_rtr()
        print, 'Starting MM'
        err = start_mm()
        print, 'Starting FF'
        err = start_ff()
        cc=1
        err = read_seq_dsp(sc.all, rtr_map.start_ff, 1l, nendof_ff)
        while (total(abs(nendof_ff)) ne 0) or cc eq 10 do begin
            cc +=1
            err = read_seq_dsp(sc.all, rtr_map.start_ff, 1l, nendof_ff)
        endwhile
        if cc eq 10 then message, 'Conto modi non ancora finito' else print, 'Conto modi finito in '+strtrim(string(cc),2)+' tentativi'
        err = read_seq_ch(sc.all,dsp_map.modes, 1l, mf_t02)
        mf2 = mamano(b0_t02, sf_t02)
        mfg2 = g_t02 * mf2
        ok = mfg2 ne mf_t02
        if total(ok) eq 0 then print, 'Test 02: passed.' $
        else message, 'Test 02: failed!'

    end


;c- G=1, b0 nulla, slope casuale, b1 non nulla
;   start rtr 2 volte e verifico che m(f) sia uguale al mio d(f-1)
;d- b2 non nulla. start rtr 2 volte e verifico su m(f)
    3: begin
        print, 'Test 03: set G random, B0 nulla, B1 casuale, S(f) random verifica m(f) uguale d(f).'
        ;debug line
        ;sl_ran= randomn(seed,6400)
        ;err= write_same_dsp(sc.all, rtr_map.slope_delay, sl_ran, /ch)
        if err then print, 'slope vector delay non sono state scritte corretttamente'
        for i=0, rtr.n_slope_delay-1 do begin
            g_t03 = randomn(seed,adsec.n_actuators)
            err = set_g_gain(sc.all, g_t03, block=block)
            b0_t03 = fltarr(rtr.n_slope, adsec.n_actuators)
            Sf_t03 = randomn(seed,rtr.n_slope)
            tmp = randomn(seed,rtr.n_slope, adsec.n_actuators)
            bd_t03 = fltarr(rtr.n_slope, adsec.n_actuators, rtr.n_slope_delay)
            ;bd_t03 = randomn(seed,rtr.n_slope, adsec.n_actuators, rtr.n_slope_delay)
            bd_t03[*,*,i] = tmp
            err = set_b0_matrix(b0_t03, block=block)
            err = set_b_delay_matrix(bd_t03, block=block)
            err = write_same_dsp(sc.all,rtr_map.slope_vector, sf_t03)
            ;annullo Ai
            ad_t02 = fltarr(adsec.n_actuators, adsec.n_actuators, rtr.n_modes_delay)
            err = set_a_delay_matrix(ad_t02)

            for j=0, i+1 do begin
;                err = write_same_dsp(sc.all, dsp_map.modes_delay, fltarr(192))
                ;print, 'Setting the nendof to 1UL ...'
                ;err = set_nendof()
                print, 'Starting RTR #'+strtrim(string(j),2)
                err = start_rtr()
                err = read_seq_dsp(sc.all, rtr_map.start_rtr, 1l, nendof_rtr)
                ;while (total(abs(nendof_rtr)) ne 0) or cc eq 10 do begin
                ;    cc +=1
                ;    err = read_seq_dsp(sc.all, rtr_map.start_rtr, 1l, nendof_rtr)
                ;endwhile
                ;if cc eq 10 then message, 'Conto modi non ancora finito' else print, 'Conto modi finito in '+strtrim(string(cc),2)+' tentativi'
                print, 'Starting MM #'+strtrim(string(j),2)
                err = start_mm()
                print, 'Starting FF #'+strtrim(string(j),2)
                err = start_ff()
                cc=1
                err = read_seq_dsp(sc.all, rtr_map.start_ff, 1l, nendof_ff)
                while (total(abs(nendof_ff)) ne 0) or cc eq 10 do begin
                    cc +=1
                    err = read_seq_dsp(sc.all, rtr_map.start_ff, 1l, nendof_ff)
                    printhex, nendof_ff
                    err = read_seq_dsp(sc.all, rtr_map.start_rtr, 1l, nendof_rtr)
                    printhex, nendof_rtr
                    err = read_seq_dsp(sc.all, rtr_map.start_mm, 1l, nendof_mm)
                    printhex, nendof_mm
                endwhile
                if cc eq 10 then message, 'Conto modi non ancora finito' else print, 'Conto modi finito in '+strtrim(string(cc),2)+' tentativi'
print, read_seq_dsp(sc.all, '180181'xl, 1l, bb) & printhex, bb
            endfor

            err = read_seq_ch(sc.all,dsp_map.modes,1l, mf_t03)
            mf3 = mamano(tmp, sf_t03, /REVERSE)
            mfg3 = g_t03 *  mf3
            ok = mfg3 ne mf_t03
            print, minmax(mf_t03)
            if total(ok) eq 0 then print, 'Test 03, delay'+strtrim(string(i+1),2)+': passed.' $
            else message, 'Test 03: failed!'
        endfor
    end
    
    4: begin
;e- continuo come d finche' non finisco la linea di ritardo
        print, 'Test 04: set G nullo?, A0 casuale , A1 nulla ecc, S(f) random verifica m(f) uguale d(f), via via per tutte.'
        
        for i=0, rtr.n_modes_delay-1 do begin
            g_t04 = randomn(seed,adsec.n_actuators)
            err = set_g_gain(sc.all, g_t04, block=block)
            Sf_t04 = randomn(seed,rtr.n_slope)
            tmp = randomn(seed,adsec.n_actuators, adsec.n_actuators)
            ad_t04 = fltarr(adsec.n_actuators, adsec.n_actuators, rtr.n_modes_delay)
            ad_t04[*,*,i] = tmp
            err = set_a_delay_matrix(ad_t04)
            b0_t04 = randomn(seed,rtr.n_slope, adsec.n_actuators)
            err = set_b0_matrix(b0_t04, block=block)
            err = write_same_dsp(sc.all,rtr_map.slope_vector, sf_t04)

            for j=0, i+1 do begin 

               ;print, 'Setting the nendof to 1UL ...'
               ; err = set_nendof()
               print, 'Starting RTR #'+strtrim(string(j),2)
                err = start_rtr()
                if j eq 0 then begin 
                    err = set_b0_matrix(b0_t04, block=block)
                    err = read_seq_ch(sc.all,dsp_map.modes, 1l,mf_orig)
                    err = write_same_dsp(sc.all,rtr_map.modes_vector, mf_orig)
                    err = set_g_gain(sc.all, g_t04*0, block=block)
                endif 
            

                print, 'Starting MM #'+strtrim(string(j),2)
                err = start_mm()
                print, 'Starting FF #'+strtrim(string(j),2)
                err = start_ff()
                cc=1
                err = read_seq_dsp(sc.all, rtr_map.start_ff, 1l, nendof_ff)
                while (total(abs(nendof_ff)) ne 0) or cc eq 10 do begin
                    cc +=1
                    err = read_seq_dsp(sc.all, rtr_map.start_ff, 1l, nendof_ff)
                endwhile
                if cc eq 10 then message, 'Conto ff non ancora finito' else print, 'Conto ff finito in '+strtrim(string(cc),2)+' tentativi'
                err = read_seq_ch(sc.all,dsp_map.modes, 1l,mf_t04)
            endfor
            
            err = read_seq_ch(sc.all,dsp_map.modes, 1l,mf_t04)
            mf4 = mamano(tmp, mf_orig, /reverse)
            ok = mf4 ne mf_t04
            print, minmax(mf_t04)
            if total(ok) eq 0 then print, 'Test 04, delay '+strtrim(string(i+1),2)+': passed.' $
            else message, 'Test 04: failed!'

        endfor
        

    end
    else: break 
    
endcase
end
