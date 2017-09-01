;+
;INIT_OL (BETA VERSION)
;
;  Initialize all the global variables/structure used to close the optical loop.
;
;HISTORY
; 5 April 2005 Created.
;   03 Aug 2005 Marco Xompero
;     New update to initialize open loop operations.
;   11 Aug 2005 A. Riccardi (AR)
;     Added setting of preshapers.
;     Automatic commutation of accumulator settings.
;   07 Feb 2007 MX
;       LUT of variable deleted and name refurbished.
;
;-
pro init_ol, BIN_CH = bin_ch

; common blocks definition
@adsec_common

wfs_sampling_time = 8.0e-3            ; [s] WFS sampling time [s]
desired_cmd_preshaper_sec = 6.0e-3    ; [s] preshaper setting time (command)
desired_cur_preshaper_sec = 6.0e-3    ; [s] preshaper setting time (force)

cmd_preshaper = time2step_preshaper(desired_cmd_preshaper_sec, APPLIED=cmd_preshaper_sec, /VERB)
cur_preshaper = time2step_preshaper(desired_cur_preshaper_sec, APPLIED=cur_preshaper_sec, /VERB)

preshaper_sec = max([cmd_preshaper_sec,cur_preshaper_sec])
max_settling_time = preshaper_sec

;!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
;NB ricordarsi di cambiare i valori nella diagnostica veloce
;!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
acc_delay = ceil(max_settling_time/adsec.sampling_time)
acc_len   = long(wfs_sampling_time/adsec.sampling_time-acc_delay)

if acc_delay lt 0 then message, "ACC_DELAY_TIME sbagliato!"
if acc_len lt 1 then message, "ACC_LEN_TIME troppo corto!"

print, "ACC_DELAY: ", strtrim(acc_delay,2)," (",strtrim(acc_delay*adsec.sampling_time,2),"s)"
print, "ACC_LEN  : ", strtrim(acc_len,2)," (",strtrim(acc_len*adsec.sampling_time,2),"s)"

;!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
;CONCORDARE CON ALFIO COME BLOCCARE LA CHIUSURA DEL LOOOP
;!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
; check open optical loop
;err=check_ol()

;set preshapers
err = set_preshaper(sc.all, cmd_preshaper)
err = set_preshaper(sc.all, cur_preshaper, /FF)

;writes to zero modes and delay line
err = clear_dl()
if err then  message, 'The delay lines were not cleaned'
err = write_same_dsp(sc.all, dsp_map.modes, fltarr(adsec.n_act_per_dsp))
if err then message, 'The modes  were not cleaned'
err = write_same_dsp(sc.all, rtr_map.modes_vector, fltarr(adsec.n_actuators), /ch)
if err then message,'The mode vector was not cleaned'
err = write_same_dsp(sc.all, rtr_map.slope_vector, fltarr(rtr.n_slope), /ch)
if err then message,'The slope vector was not cleaned'

;writes the current command in the bias command
err = set_cmd2bias()
if err ne adsec_error.ok then message, '!!!!!!!!!!'

;==============================================================================
if keyword_set(bin_ch) then begin
    ;BIN SETUP:solo se necessario!
    ;creo la nuova struttura rtr con il nuovo numero di slopes
    
    ;!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!    
    ;;leggo nella shared memory il numero di slope: DA IMPLEMENTARE
    if n_elements(n_slope) ne 0 then undefine, n_slope
    ;;leggo nella shared memory il numero di step di slope delay line: DA IMPLEMENTARE
    if n_elements(n_sl_dl) ne 0 then undefine, n_sl_dl
    ;;leggo nella shared memory il numero di step di mode delay line: DA IMPLEMENTARE
    if n_elements(n_dp_dl) ne 0 then undefine, n_dp_dl
    ;!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

    ;lancio il calcolo della nuova struttura rtr
    init_rtr, N_SL=n_slope, N_SL_DL=n_sl_dl, N_DP_DL=n_dp_dl
    ;if err ne adsec_error.ok then message, 'The new rtr structure was not created'

    ;calcolo la nuova suddivisione in memoria
    err=init_rtr_map()
    if err ne adsec_error.ok then message, 'The new rtr_map structure was not created'

    ;pulisco ben bene test_address
    err=clear_rtr_mem()
    if err ne adsec_error.ok then message, 'The memory of reconstructor data was not cleaned'

    ;SET POINTERS==========================================================================================
    ;setting all the pointers 
    disp_mess, 'Setting the reconstructor parameters ...',/APPEND
    err = set_reconstructor_par()
    if err ne adsec_error.ok then begin
        message, '... error on writing.', /APPEND
    ;return, err
    endif
    disp_mess, '... done.', /APPEND

    disp_mess, 'Setting the matrix address pointers ...',/APPEND
    err = set_matrix_ptr()
    if err ne adsec_error.ok then begin
        message, '... error on writing.', /APPEND
    endif
    disp_mess, '... done.', /APPEND
    
    
    disp_mess, 'Setting the address pointers ...',/APPEND
    err = set_all_ptr()
    if err ne adsec_error.ok then begin
        message, '... error on writing.', /APPEND
    endif
    disp_mess, '... done.', /APPEND

;    disp_mess, 'Setting the nendof to 1UL ...',/APPEND
;    err = set_nendof()
;    if err ne adsec_error.ok then begin
;        message, '... error on writing.', /APPEND
;    endif
;    disp_mess, '... done.', /APPEND

endif

;=================================================================================================

;riempio la memoria, matrici di ricostruzione e di delay lines 
err=set_matrix()
if err ne adsec_error.ok then message, 'The filter matrix were not written'

;send M2C
err=set_m2c_matrix(rtr.m2c_matrix)
if err then message, 'Reconstructor matrix not sent'

;genero il file di testo con i nuovi indirizzi del secondario per la comunicazione veloce
;===================================================================================================
;!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
;
;!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
;Close optical loop
;set PBS per la diagnostica veloce...
; reset of the diagnostic record pointer to the beginning of SDRAM (addr=0)
err = write_same_board(sc.all, sram_map.wr_diagnostic_record_ptr, 0L $
                       , SET_PM=bcu_datasheet.sram_mem_id)

; set to true the saving of fast diagnostics in SDRAM (bit 6 of
; param-block selector)
word = 2ul^6
err = read_seq_dsp(sc.all, rtr_map.param_selector, 1l, bb, /ul)
bb = bb or word
err = write_same_dsp(sc.all, rtr_map.param_selector, word)
print, 'Settata la configurazione con diagnostica veloce salvata in SDRAM.'

err = set_acc(acc_len,acc_delay)
wait, 0.1
; update the accumulators to the current state
err = start_acc(/WAIT)

;??????????????????
;err = close_ol()

end
