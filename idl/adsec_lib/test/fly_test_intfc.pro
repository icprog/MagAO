;test corrente di controllo integrata, con cambiamento al volo del blocco memoria
;set Bf0, Bf1, Af1
;
;imposto le matrici e pulisco i guadagni
test=4

block=0
test_fast, block=block
sl = findgen(1600)

;setting block
err= set_block(block)
if err then message, 'Il blocco non è stato cambiato'

;cleaning of all gain block
err = set_fc_gain(/all)
if err then message, 'I Guadagni non sono stati puliti'

;++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
;clean all current    
    err = set_delta_position_vector(fltarr(adsec.n_actuators), /ch)
    err = set_position_vector(fltarr(adsec.n_actuators), /ch)
    err = write_same_ch(sc.all, dsp_map.ff_current, 0l, /ch) 
    err = write_same_ch(sc.all, dsp_map.ff_pure_current, 0l, /ch)  
    err = write_same_ch(sc.all, dsp_map.int_control_current, 0l, /ch)
    err = write_same_ch(sc.all, dsp_map.mean_control_current, 0l, /ch)
    err = write_same_ch(sc.all, dsp_map.bias_current, 0l, /ch)
    err = write_same_dsp(sc.all, dsp_map.curr_accumulator, fltarr(8), /ch)
    err = set_g_gain(/reset_all)
    err = write_same_dsp(sc.all, rtr_map.slope_delay, fltarr(rtr.n_slope*rtr.n_slope_delay))
    err = write_same_dsp(sc.all, rtr_map.modes_delay, fltarr(rtr.n_modes_delay*adsec.n_actuators))
    err = write_same_dsp(sc.all, rtr_map.modes_vector, fltarr(adsec.n_actuators))
    err = write_same_dsp(sc.all, rtr_map.slope_vector, fltarr(rtr.n_slope))
    err = write_same_dsp(sc.all, dsp_map.modes, fltarr(adsec.n_act_per_dsp))
    err= write_same_ch(-1, dsp_map.curr_average, 0l)
    err=clear_dacs(sc.all) 
;err= write_same_ch(-1, dsp_map.curr_accumulator, 0l)

 err = write_same_ch(sc.all, dsp_map.ff_ud_current, 0l, /ch)
;++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
err=read_seq_dsp(sc.all, dsp_map.ff_current, 4l,deltaff_curr0)
if err then message, 'Errore in lettura'
err=read_seq_dsp(sc.all, dsp_map.ff_pure_current, 4l,ff_curr0)
if err then message, 'Errore in lettura'
err=read_seq_dsp(sc.all, dsp_map.int_control_current, 4l,ffc_curr0)
if err then message, 'Errore in lettura'
;err=read_seq_dsp(sc.all, dsp_map.curr_accumulator, 4l,acc_curr0)
err=read_seq_dsp(sc.all, dsp_map.curr_average, 4l,acc_curr0)
if err then message, 'Errore in lettura'
err=read_seq_dsp(sc.all, dsp_map.bias_current, 4l,bias_curr0)
if err then message, 'Errore in lettura'
err=read_seq_dsp(sc.all, dsp_map.mean_control_current, 4l, mctrl_curr0)
if err then message, 'Errore in lettura'
err=read_seq_dsp(sc.all, dsp_map.ff_ud_current, 4l, fud_curr0)
if err then message, 'Errore in lettura'

err=set_preshaper(sc.all, dsp_const.fastest_preshaper_step)
err=set_preshaper(sc.all, dsp_const.fastest_preshaper_step, /ff)
;print, 'Setting the nendof to 1UL ...'
;err = set_nendof()  
err=set_acc(128,100)
new_ff=diagonal_matrix(fltarr(48))
new_ff=diagonal_matrix(fltarr(48)+1.)
;print, send_ff_matrix(new_ff)
print, send_ff_matrix(adsec.ff_matrix)
;wait, 1
;ccf2 = randomn(seed,adsec.n_actuators)*1e-6

case test of


    0: begin
       ;bf0, bf1, af1 nulli controllo che tutte le correnti siano costanti
            test_const = 0.0        
            delta_const=0.0
            ff_const=0.0
            ffc_const=0.0
            acc_const=0.0
            bias_const=0.0
            mctrl_const=0.0
            fud_const=0.0
            ;ccf2 = randomn(seed,adsec.n_actuators)
            for ii=0,2 do begin           
            
            ;err = set_acc(128, 0)
            ;err = start_acc()
            ;if err then message, 'Gli accumulatori non sono partiti'

            err = start_rtr()
            err=read_seq_dsp(sc.all, rtr_map.start_rtr, 1l,nertr)
            if total(nertr) ne 0.0 then  wait, 0.1
            
            err = start_mm()
            err=read_seq_dsp(sc.all, rtr_map.start_mm, 1l,nemm)
            if total(nemm) ne 0.0 then  wait, 0.1
            ccf = randomn(seed,adsec.n_actuators)*1e-7
            err = write_same_dsp(sc.all, rtr_map.ff_command_vector, ccf, /ch)
            ;err = write_same_dsp(sc.all, rtr_map.ff_command_vector, fltarr(adsec.n_actuators)+1e-6, /ch)
            ; err = write_same_dsp(sc.all, rtr_map.ff_command_vector, fltarr(adsec.n_actuators), /ch)
            err=read_seq_ch(sc.all, dsp_map.curr_average, 1l,acc_curr0)
            ;err=read_seq_dsp(sc.all, dsp_map.curr_accumulator, 4l,acc_curr0)
            if err then message, 'Errore in lettura'
            
            err = start_ff()

            err=read_seq_dsp(sc.all, rtr_map.start_ff, 1l,neff)

            if total(neff) ne 0.0 then  wait, 0.2
            wait, 1

            err=read_seq_ch(sc.all, dsp_map.ff_current, 1l,deltaff_curr1)
            if err then message, 'Errore in lettura'
            err=read_seq_ch(sc.all, dsp_map.ff_pure_current, 1l,ff_curr1)
            if err then message, 'Errore in lettura'
            err=read_seq_ch(sc.all, dsp_map.int_control_current, 1l,ffc_curr1)
            if err then message, 'Errore in lettura'
            wait, 1 ;bisogna attendere che l'accumulatore finisca per poter leggere il valore medio
            err=read_seq_ch(sc.all, dsp_map.curr_average, 1l,acc_curr1)
            ;err=read_seq_dsp(sc.all, dsp_map.curr_accumulator, 4l,acc_curr1)
            if err then message, 'Errore in lettura'
            err=read_seq_ch(sc.all, dsp_map.bias_current, 1l,bias_curr1)
            if err then message, 'Errore in lettura'
            err=read_seq_ch(sc.all, dsp_map.mean_control_current, 1l, mctrl_curr1)
            if err then message, 'Errore in lettura'
            err=read_seq_ch(sc.all, dsp_map.ff_ud_current, 1l, fud_curr1)
            if err then message, 'Errore in lettura'
           
            ;delta_const=(deltaff_curr1-deltaff_curr0);+delta_const
            ff_const=(ff_curr1-ff_curr0);+ff_const
            ffc_const=(ffc_curr1-ffc_curr0);+ffc_const
            acc_const=(acc_curr1-acc_curr0);+acc_const
            bias_const=(bias_curr1-bias_curr0);+bias_const
            mctrl_const=(mctrl_curr1-mctrl_curr0);+mctrl_const
            fud_const=(fud_curr1-fud_curr0);+fud_const
            fc_calc=acc_curr1-bias_curr1-ff_curr0
            print, 'totale diff tra la f media di ctrl misurata e calcolata',total(mctrl_curr1-fc_calc)  
         
            ;err = write_same_dsp(sc.all,dsp_map.slope_vector, sl, /ch)
        
            print, 'Delta FeedForward',minmax(deltaff_curr1)
            print, 'Delta FeedForwad Applied',minmax(ff_const)
            print, 'FeedForwad Applied PURE',minmax(ff_curr1)
            print, 'Contributo integrale di controllo',total(ffc_curr1)
            print, 'forza average', minmax(acc_curr1)
            print, 'forza media calcolata', minmax(acc_curr0-bias_curr1-ff_curr0)
            print, 'forza di bias',minmax(bias_curr1)
            ;print, 'delta forza media di controllo',minmax(mctrl_const)
            print, 'forza media di controllo',minmax(mctrl_curr1)
            ;print, total(mctrl_curr1)
            print, 'nuovo comando di forza UD_CURRENT',minmax(fud_curr1)
            test_const=test_const+total(ffc_curr1)
            ;deltaff_curr0=deltaff_curr1
            ff_curr0=ff_curr1
            ffc_curr0=ffc_curr1
            ;acc_curr0=acc_curr1
            bias_curr0=bias_curr1
            mctrl_curr0=mctrl_curr1
            fud_curr0=fud_curr1
            
            err=change_block()
            if err then message, 'Impossibile cambiare blocco'

            err = read_seq_dsp(sc.all, rtr_map.param_selector, 1l,pb, /ul)
            block=pb[0]
            print, 'block',pb[0]
       
        endfor
        err_mac=(machar()).eps
        if test_const le err_mac then print, "test ok int ctrl current e' costante" else $
          print, "test ko!!"
    end
 1: begin
       ;Bf0, Bf1 nulle, controlla fFC(f) sia uguale ad ogni passo a Af1##fFC(f-1)
 
      err = write_same_ch(sc.all, dsp_map.int_control_current, 1e-5, /ch)
       ;err = write_same_ch(sc.all, dsp_map.int_control_current, 0.0, /ch)
       err=read_seq_ch(sc.all, dsp_map.int_control_current, 1l,ffc_curr0)
        err = write_same_ch(sc.all, dsp_map.ff_current, 0.0, /ch)
;err=set_io_buffer(sc.all,0,rtr_map.param_selector,32000,/start)
       err=set_fc_gain(/all_reset)
       ffc_int=ffc_curr0
       for ii=0,2 do begin    
            af00 = randomn(seed,adsec.n_actuators)
            ;af00=fltarr(48)+1.0
            err = write_same_dsp(sc.all, rtr_map.ff_command_vector, fltarr(adsec.n_actuators), /ch)
            err=set_fc_gain(af00,/af1, block=block)
            ;err=set_fc_gain(af00,/af1, /block)
            if err then message, 'Set matrix af1 fallita'
       ;for ii=0,4 do begin   
            ;err = read_seq_dsp(sc.all, dsp_map.param_selector, 1l,pb)
            ;printhex, pb
            fc_pre=ffc_curr0
           
            
            ffc_as=mamano(diagonal_matrix(af00),reform(fc_pre))
            ;ffc_as=reform(fc_pre)*af00
            err = start_rtr()
            err=read_seq_dsp(sc.all, rtr_map.start_rtr, 1l,nertr)
            if total(nertr) ne 0.0 then  wait, 0.1
            
            err = start_mm()
            err=read_seq_dsp(sc.all, rtr_map.start_mm, 1l,nemm)
            if total(nemm) ne 0.0 then  wait, 0.1
            
            err = write_same_dsp(sc.all, rtr_map.ff_command_vector, fltarr(adsec.n_actuators), /ch)
            err = start_ff()
            ;wait, 1
            err=read_seq_dsp(sc.all, rtr_map.ff_command_vector,48,bb)
            err=read_seq_dsp(sc.all, rtr_map.start_ff, 1l,neff)
            if total(neff) ne 0.0 then  wait, 0.4
            
            err=read_seq_ch(sc.all, dsp_map.ff_current, 1l,deltaff_curr1)
            if err then message, 'Errore in lettura'
            wait,1
            err=read_seq_ch(sc.all, dsp_map.ff_pure_current, 1l,ff_curr1)
            if err then message, 'Errore in lettura'
            
            err=read_seq_ch(sc.all, dsp_map.int_control_current, 1l,ffc_curr1)
            if err then message, 'Errore in lettura'
            
            err=read_seq_ch(sc.all, dsp_map.curr_average, 1l,acc_curr1)
            if err then message, 'Errore in lettura'
            err=read_seq_ch(sc.all, dsp_map.bias_current, 1l,bias_curr1)
            if err then message, 'Errore in lettura'
            err=read_seq_ch(sc.all, dsp_map.mean_control_current, 1l, mctrl_curr1)
            if err then message, 'Errore in lettura'
            err=read_seq_ch(sc.all, dsp_map.ff_ud_current, 1l, fud_curr1)
            if err then message, 'Errore in lettura'
            ;ffc_int=ffc_int+ffc_curr1
            ffc_p=ffc_curr1
            print,"ff_ud_current(f_f): ", minmax(fud_curr1)
            print, "ff_pure_current(f_ff): ",minmax(ff_curr1)
            print, "int_control_current:",minmax(ffc_curr1)
            print,"ff_current (delta_ff): " ,minmax(deltaff_curr1)
            print,"ff_command_vector:", minmax(bb[*,0])
            
            ok= total(ffc_as-reform(ffc_p))
            if ok ne 0.0 then begin 
                print, ok
                message, 'Test calcolo Af1 fallito!'
            endif

            print, 'ok:',ok
            deltaff_curr0=deltaff_curr1
            ff_curr0=ff_curr1
            ffc_curr0=ffc_curr1
            acc_curr0=acc_curr1
            bias_curr0=bias_curr1
            mctrl_curr0=mctrl_curr1
            fud_curr0=fud_curr1
            wait, 0.2    
            ;change block
            err=change_block()
            if err then message, 'Impossibile cambiare blocco'

            err = read_seq_dsp(sc.all, rtr_map.param_selector, 1l,pb, /ul)
            block=pb[0]
            print, 'block',block
        endfor
wait,1
;err=read_io_buffer(sc.all,0,32000,pbbuf,/ulong)
 
    end
 2: begin
     ;Bf1, Af1 nulle, controlla fFC(f) sia uguale ad ogni passo a Bf0##fC(f)
 
       ;err = write_same_ch(sc.all, dsp_map.mean_control_current, 1e-3, /ch)
       err=read_seq_ch(sc.all, dsp_map.int_control_current, 1l,ffc_curr0)
       err=read_seq_ch(sc.all, dsp_map.mean_control_current, 1l, mctrl_curr0)
       err=read_seq_ch(sc.all, dsp_map.ff_pure_current, 1l, ff_curr0)
       err=set_fc_gain(/all)
       for ii=0,5 do begin    
            err = write_same_ch(sc.all, dsp_map.curr_average, 1e-3, /ch)
            bf00 = randomn(seed,adsec.n_actuators)
            bf01 = randomn(seed,adsec.n_actuators)
            ;err = write_same_dsp(sc.all, dsp_map.ff_command_vector, fltarr(adsec.n_actuators), /ch)
            err=set_fc_gain(bf00,/bf0, block=block)
            ;err=set_fc_gain(bf00,/bf1, block=block)
            if err then message, 'Set matrix af1 fallita'
            ;err = read_seq_dsp(sc.all, dsp_map.param_selector, 1l,pb)
            ;printhex, pb
            
            err = start_rtr()
            err=read_seq_dsp(sc.all, rtr_map.start_rtr, 1l,nertr)
            if total(nertr) ne 0.0 then  wait, 0.1
            
            err = start_mm()
            err=read_seq_dsp(sc.all, rtr_map.start_mm, 1l,nemm)
            if total(nemm) ne 0.0 then  wait, 0.1
            
            err = write_same_dsp(sc.all, rtr_map.ff_command_vector, fltarr(adsec.n_actuators), /ch)
            err=read_seq_ch(sc.all, dsp_map.curr_average, 1l,acc_curr0)
            if err then message, 'Errore in lettura'
            err = start_ff()
            err=read_seq_dsp(sc.all, rtr_map.start_ff, 1l,neff)
            if total(neff) ne 0.0 then  wait, 0.2
            
            err=read_seq_ch(sc.all, dsp_map.mean_control_current, 1l, mctrl_curr1)
            if err then message, 'Errore in lettura'
            ffc_as=bf00*reform(mctrl_curr1)
            err=read_seq_ch(sc.all, dsp_map.int_control_current, 1l,ffc_curr1)
            if err then message, 'Errore in lettura'
            
            ffc_p=ffc_curr1
            ok= total(ffc_as-reform(ffc_p))
            if ok ne 0.0 then begin 
                print, ok
                message, 'Test calcolo BF0 fallito!'
            endif
            ;print, 'ff_curr0', total(ff_curr0)
            ;print, 'err_acc_mean', total(acc_curr0-mctrl_curr1)
            ;print, 'err_ud_int', total(fud_curr1-ffc_curr1)
            print, ok
            wait,0.5
            ;change block
            err=change_block()
            if err then message, 'Impossibile cambiare blocco'

            err = read_seq_dsp(sc.all, rtr_map.param_selector, 1l,pb, /ul)
            block=pb[0]
            print, 'block',block
        
        endfor
 
    end
 3: begin 
       ;Bf0, Af1 nulle, controlla fFC(f) sia uguale ad ogni passo a Bf1##fC(f-1)
       ;err = write_same_ch(sc.all, dsp_map.mean_control_current, 1e-3, /ch)
       err=read_seq_ch(sc.all, dsp_map.int_control_current, 1l,ffc_curr0)
       err=read_seq_ch(sc.all, dsp_map.mean_control_current, 1l, mctrl_curr0)
       err=read_seq_ch(sc.all, dsp_map.ff_pure_current, 1l, ff_curr0)
       err=set_fc_gain(/all)
       for ii=0,5 do begin    
            err = write_same_ch(sc.all, dsp_map.curr_average, 1e-3, /ch)
            bf00 = randomn(seed,adsec.n_actuators)
            bf01 = randomn(seed,adsec.n_actuators)
            ;err = write_same_dsp(sc.all, dsp_map.ff_command_vector, fltarr(adsec.n_actuators), /ch)
            ;err=set_fc_gain(bf00,/bf0, block=block)
            err=set_fc_gain(bf00,/bf1, block=block)
            if err then message, 'Set matrix bf1 fallita'
            ;err = read_seq_dsp(sc.all, dsp_map.param_selector, 1l,pb)
            ;printhex, pb
            
            err = start_rtr()
            err=read_seq_dsp(sc.all, rtr_map.start_rtr, 1l,nertr)
            if total(nertr) ne 0.0 then  wait, 0.1
            
            err = start_mm()
            err=read_seq_dsp(sc.all, rtr_map.start_mm, 1l,nemm)
            if total(nemm) ne 0.0 then  wait, 0.1
            
            err = write_same_dsp(sc.all, rtr_map.ff_command_vector, fltarr(adsec.n_actuators), /ch)
            err=read_seq_ch(sc.all, dsp_map.curr_average, 1l,acc_curr0)
            if err then message, 'Errore in lettura'
            err = start_ff()
            err=read_seq_dsp(sc.all, rtr_map.start_ff, 1l,neff)
            if total(neff) ne 0.0 then  wait, 0.2
            
            err=read_seq_ch(sc.all, dsp_map.mean_control_current, 1l, mctrl_curr1)
            if err then message, 'Errore in lettura'
            
            ffc_as=bf00*reform(mctrl_curr0)
            err=read_seq_ch(sc.all, dsp_map.int_control_current, 1l,ffc_curr1)
            if err then message, 'Errore in lettura'
            
            ffc_p=ffc_curr1
            ok= total(ffc_as-reform(ffc_p))
            if ok ne 0.0 then begin 
                print, ok
                message, 'Test calcolo BF0 fallito!'
            endif
            ;print, 'ff_curr0', total(ff_curr0)
            ;print, 'err_acc_mean', total(acc_curr0-mctrl_curr1)
            ;print, 'err_ud_int', total(fud_curr1-ffc_curr1)
            print, ok, 'Test ok!'
            mctrl_curr0=mctrl_curr1
            wait,0.5
            ;change block
            err=change_block()
            if err then message, 'Impossibile cambiare blocco'

            err = read_seq_dsp(sc.all, rtr_map.param_selector, 1l,pb, /ul)
            block=pb[0]
            print, 'block',block
        
        endfor
 
    end
 4: begin
       ;Af1 nulla, controlla fFC(f) sia uguale ad ogni passo a Bf0##fC(f)+Bf1##fC(f-1)
       ;err = write_same_ch(sc.all, dsp_map.mean_control_current, 1e-3, /ch)
       err=read_seq_ch(sc.all, dsp_map.int_control_current, 1l,ffc_curr0)
       err=read_seq_ch(sc.all, dsp_map.mean_control_current, 1l, mctrl_curr0)
       err=read_seq_ch(sc.all, dsp_map.ff_pure_current, 1l, ff_curr0)
       err=set_fc_gain(/all)
        bf00 = randomn(seed,adsec.n_actuators)
        bf01 = randomn(seed,adsec.n_actuators)
        err=set_fc_gain(bf00,/bf0, block=block)
        err=set_fc_gain(bf01,/bf1, block=block)
        rm=randomn(seed, 1)
        err = write_same_ch(sc.all, dsp_map.curr_average, rm, /ch)
        for ii=0,5 do begin    
                        
            err = start_rtr()
            err=read_seq_dsp(sc.all, rtr_map.start_rtr, 1l,nertr)
            if total(nertr) ne 0.0 then  wait, 0.1
            
            err = start_mm()
            err=read_seq_dsp(sc.all, rtr_map.start_mm, 1l,nemm)
            if total(nemm) ne 0.0 then  wait, 0.1
            
            err = write_same_dsp(sc.all, rtr_map.ff_command_vector, fltarr(adsec.n_actuators), /ch)
            
            err = start_ff()
            err=read_seq_dsp(sc.all, rtr_map.start_ff, 1l,neff)
            if total(neff) ne 0.0 then  wait, 0.2
            
            err=read_seq_ch(sc.all, dsp_map.mean_control_current, 1l, mctrl_curr1)
            if err then message, 'Errore in lettura'
            
            ffc_as=bf01*reform(mctrl_curr0)+bf00*reform(mctrl_curr1)
            err=read_seq_ch(sc.all, dsp_map.int_control_current, 1l,ffc_curr1)
            if err then message, 'Errore in lettura'
            
            ffc_p=ffc_curr1
            ok= total(ffc_as-reform(ffc_p))
            if ok ne 0.0 then begin 
                print, ok
                message, 'Test calcolo BF0,Bf1 fallito!'
            endif 
            ;print, 'ff_curr0', total(ff_curr0)
            ;print, 'err_acc_mean', total(acc_curr0-mctrl_curr1)
            ;print, 'err_ud_int', total(fud_curr1-ffc_curr1)
            print, ok,"Test ok!"
            mctrl_curr0=mctrl_curr1
            wait,0.5
            
           ;change block
            err=change_block()
            if err then message, 'Impossibile cambiare blocco'

            err = read_seq_dsp(sc.all, rtr_map.param_selector, 1l,pb, /ul)
            block=pb[0]
            print, 'block',block
        
            bf00 = randomn(seed,adsec.n_actuators)
            bf01 = randomn(seed,adsec.n_actuators)
            err=set_fc_gain(bf00,/bf0, block=block)
            err=set_fc_gain(bf01,/bf1, block=block)
        
        endfor
 
    end
     
else: break 
    
endcase

end
