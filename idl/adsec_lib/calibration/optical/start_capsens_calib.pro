;$Id: start_capsens_calib.pro,v 1.1 2009/02/24 13:52:27 labot Exp $
;+
;  NAME:
;   START_POS_CALIB
;
;  PURPOSE:  
;   Data acquisition for the capacitive sendor calibration (single step).
;
;  USAGE:
;   err = START_POS_CALIB(FILENAME=filename, NOASK=noask, ENDFILE=endfile)
;
;  KEYWORDS:
;   FILENAME:    save data into filename.
;   NOASK:       disable all interactive questions.
;   ENDIFLE:     flename with the final status of the mirror to achieve 
;                (flattened_status or sys_status_save variable must be included in the file.)
;   INTERF_WAVE: interferometer wavelemgth (default 632.8e-9 [m])
;
;  DEPENDENCIES:
;   Frames grabber management utilities(GRAB_INIT, GRAB_CAPTURE)
;
;  HISTORY:
;   Created by Marco Xompero on 11 Sept 2006
;   <marco@arcetri.astro.it>
;   07 Feb 2007 MX
;       LUT of variable deleted and name refurbished.
;-

Function start_capsens_calib, CLOSE_STATUS_SAVE=close_status_save, FAR_STATUS_SAVE=far_status_save


    @adsec_common

    interf_wave = 632.8d-9
    ;update start conditions
    err = update_status()
    if err ne adsec_error.ok then begin
        message, "Calibration aborted.", /CONT
        return, err
    endif
    sys_status_start = sys_status

    
    curr_cmd  = sys_status.command[adsec.act_w_cl]
    curr_gap  = mean(curr_cmd)
    
    restore, close_status_save
    close_cmd = status_save.sys_status.command[status_save.adsec.act_w_cl]
    close_gap = mean(close_cmd)

    restore, far_status_save
    far_cmd  = status_save.sys_status.command[status_save.adsec.act_w_cl]
    far_gap  = mean(far_cmd)

    
    T4D = 0.035                     ;tempo di campionamento del 4D
    Nphi = 5                        ;numero di campioni per ogni periodo della sinusoide
    Novs = 5                        ;numero di campioni di posizione per ogni periodo della
    Lambda = 632.8d-9               ;lunghezza d'onda interferometro
    Ns = uint(2L^16-2)              ;numero massimo di campioni disponibili nel buffer diagnostico -1 per usare bene la dist
    
    TASM = T4D/Novs                                                 ;massimo tempo di campionamento della posizione del secondario adattivo
    TASM_d = floor(TASM/adsec.sampling_time)*adsec.sampling_time    ;TASM discretizzato a clock di frequenza dell'ASAM
    fASM_d = 1/TASM_d                                               ;frequenza campionamento ASM
    opt_dec = tASM_d/adsec.sampling_time -1                         ;decimazione dati ASM (da verificare il -1....)
    
    AmpMax =  1d/(Nphi*T4D)*lambda/16*Ns*tASM_d

    dc = final_cmd-first_cmd
    
    RequestedAmp = max(abs(dc))/2.
    if RequestedAmp gt AmpMax then begin
        message, "ERROR: max amplitude exceeded.", /INFO
        return, err
    endif
    
    ;Re-sampling using full range
    NewNphi = AmpMax/RequestedAmp*Novs_d
    ns_shift = round(ns/2d*(curr_gap-close_gap)/(far_gap-close_gap))
    dHist = shift(dist(ns,1)/(ns/2d), ns_shift)
    dHistReb = rebin(dHist - dHist[0], ns, adsec.n_actuators, /SAMPLE)  
 
    dHistReb[0:ns_shift-1,*] =  dHistReb[0:ns_shift-1,*] * rebin(abs(curr_cmd - close_cmd), ns, adsec.n_actuators, /SAMPLE)
    dHistReb[ns_shift:*,*]   =  dHistReb[ns_shift:*,*] * rebin(abs(final_cmd - curr_cmd), ns, adsec.n_actuators, /SAMPLE)
    Hist = rebin(curr_cmd,ns,adsec.n_actuators, /SAMPLE) + dHistReb 


    ;;UPLOAD THE TIME HISTORY ON EACH ACTUATOR
    n_dsp = adsec.n_bcu * adsec.n_board_per_bcu * adsec.n_dsp_per_board
    n_acts= n_dsp * adsec.n_act_per_dsp
    n_buf_words = adsec.n_act_per_dsp * long(ns)

    for i_dsp=0,n_dsp-1 do begin

        act_idx0 = i_dsp*adsec.n_act_per_dsp
        act_idx1 = act_idx0+adsec.n_act_per_dsp-1
        err = write_IO_buffer(i_dsp, 0, reform(transpose(Hist[*,act_idx0:act_idx1]), n_buf_words, /OVER))
        if err ne adsec_error.ok then begin
            message, "Calibration aborted.", /CONT
            return, err
        endif

    endfor
    
    ;new_preshaper_len = round((float(dsp_const.fastest_preshaper_step)-opt_dec-1)/float(opt_dec+1))
    new_preshaper_len =  round(float(dsp_const.fastest_preshaper_step)/float(opt_dec+1))
    shape_preshaper = findgen(adsec.preshaper_len)/(adsec.preshaper_len-1)
        

    ;;; SAVE THE CURRENT STATUS OF THE POS COMMAND PRESHAPER
    ;;; IN ORDER TO RESTORE IT AT THE END OF THE ACQUISITION
    err = get_preshaper(sc.all_actuators, cmd_preshaper_save)
    if err ne adsec_error.ok then return, err

    ;;; DISABLE THE PRESHAPING OF THE POS COMMANDs
    err = set_preshaper(sc.all_actuators, new_preshaper_len)
    if err ne adsec_error.ok then return, err

    ;;; SET THE NEW PRESHAPER SHAPE
    err = set_shape_preshaper(shape_preshaper)
    if err ne adsec_error.ok then return, err
    
    ;;BUFFER WRITING AND START ACQUISITION
    ;;SET THE GLOBAL COUNTER TRIGGER VALUE (0.5 SEC LATER)
    ;;REFERENCE DSP FOR THE GC READING

    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    ;;SET THE COMMAND BUFFER 0 TRIGGERED TO GC
    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    addr = dsp_map.pos_command
    err = set_IO_buffer(sc.all_dsp, 0, addr, ns , /WRITE_TO_MEM, RECORD_SIZE=adsec.n_act_per_dsp, /START, DEC=opt_dec)
    if err ne adsec_error.ok then begin
        message, "Calibration aborted.", /CONT
        return, err
    endif

;; TODO: forza in scrittura a richiesta....


    print, "samples ", ns
    addr2save = dsp_map.float_adc_value
    err = set_IO_buffer(sc.all_dsp, 1, addr2save, ns, RECORD_SIZE=adsec.n_act_per_dsp, /START, DEC=opt_dec)
    if err ne adsec_error.ok then begin
        message, "Calibration aborted.", /CONT
        return, err
    endif


    err = read_io_buffer_ch(sc.all_dsp, 1, ns, buf_volt)


    ;;; disable the preshaping of the poc or curr commands
    err = set_preshaper(sc.all_actuators, cmd_preshaper_save)
    if err ne adsec_error.ok then return, err

    ;buf_time construction
    volt_time = dindgen(ns)*adsec.sampling_time*(opt_dec+1)

    ;temporary save
    if n_elements(filename) gt 0 then begin
        print, format='("Wait for saving ...",$)'
        adsec_save = adsec
        save, frames, timef, buf_volt, time_hist, opt_dec, volt_time, time_delay, adsec_save, interf_wave, file=filename
        print, string(" done.", format='')
    endif

    ;;DATA FILTERING: DATA PROJECTION
    ;chiedi le procedure di fit al pinna

    ;;DATA FITLERING: CAPSENS AREA FEATURES OR DEFAULT
    return, 0
    ;;BYE
End
