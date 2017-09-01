; $Id: proc_startup.pro,v 1.53 2009/12/21 10:55:08 marco Exp $$
;+
;   NAME:
;    PROC_STARTUP
;
;   PURPOSE:
;    Configuration and cleaning dsp boards, crateBcu and SwitchBcu.
;
;   USAGE:
;    err = PROC_STARTUP([prog_file, NO_ASK=no_ask])
;   
;   INPUT:
;    prog_file: program file list to download (DSP, BCU, SWITCH)
;
;   OUTPUT:
;    Error code.
;   
;   KEYWORDS:
;    NO_ASK: automatic DSPs synchronization.
;   
;   HISTORY
;    ReWritten by M. Xompero (MX) 26 Jan 2007
;    Osservatorio Asrofisico di Arcetri, ITALY
;    <marc0@arcetri.astro.it>
;    27 Feb 2007 MX
;       Sleep4Update call added and FF_matrix initialization has been moved here.
;    07 Mar 2007 MX
;       Initial disabling of master diagnostic before download programs reset.
;    14 Mar 2007 MX
;       Reload default parameters for SIGGEN waveform
;    20 Mar 2007, MX & DZ
;       Disable/enable interrupt added before/after the program loading.
;    03 April 2007, MX & DZ
;       Insert the disable Timeout frames, the cleaning of delay line filter, offload commands and forces.
;    18 Apr 2007, MX
;       Added/changed parameters for programs path.
;    17 May 2007, MX
;       Added test firmware/dsp program versions and fastlink alignment.
;    18 Mar 2008, MX
;       Added distance treshold configuration for dummy acts
;    01 Oct 2008, MX
;       NO_OPEN_COMMUNICATION and COMM_ONLY keywords removed, ASK for
;       the system startup removed.
;-


Function proc_startup, prog_file_vec, NO_ASK=no_ask, NOFLTEST=nofltest, PROGRESS=progress

    @adsec_common

    if xregistered('xadsec') gt 0 then  xrx = 1 else xrx=0
    if n_elements(progress) eq 0 then progress=[0,100]

    cpr = progress[0]+fix(0./(progress[1]-progress[0]))
    err = update_progress(cpr)
    if err ne adsec_error.ok then return, err


    if n_elements(prog_file_vec) eq 0 then                      $
        prog_file_vec=[sc.prog_ver+'DspMainProgram.ldr',        $
                       sc.switch_prog_ver+'SwitchBCU.ldr',      $
                       sc.bcu_prog_ver+'CrateBCU.ldr',          $
                       sc.accel_prog_ver+'AccAcquisition.ldr' ]    

    if n_elements(prog_file_vec) ne 4 then begin
        message, "The input must be a string vector of 3 elements.", CONT=(sc.debug eq 0B)
        return, adsec_error.input_type
    endif

    if test_type(prog_file_vec, /STRING) then begin
        message, "The input must be a string vector.", CONT=(sc.debug eq 0B)
        return, adsec_error.input_type
    endif

    

;   ans = dialog_message("Do you want to init the system?", /QUEST)
;   if ans eq "No" then return, adsec_error.cancel


; ---------------------------------------------------------------------------------------------------
; Sleep for update the pointers and the program

    err=set_diagnostic(over=0.0)
    if err ne adsec_error.ok then begin
        message, 'New register initialization failed, and no start is ready!', CONT = (sc.debug eq 0)
        return, err
    endif

    err=set_diagnostic(MASTER=0)
    if err ne adsec_error.ok then begin
        message, 'New register initialization failed, and no start is ready!', CONT = (sc.debug eq 0)
        return, err
    endif

; ---------------------------------------------------------------------------------------------------
;Disable DSP watchdog for clearing
     disp_mess, 'Disabling DSP WATCHDOG...',/APPEND
    err = disable_watchdog(/DSP)
    if err ne adsec_error.ok then begin
         disp_mess, '... error on  DSP watchdog disabling.', /APPEND
        return, err
    endif
     disp_mess, '... done.', /APPEND

;------------------------------------------------------------------------------------------------------ OK


    err=sleep4update()
    if err ne adsec_error.ok then begin
        message, 'New register initialization failed, and no start is ready!', CONT = (sc.debug eq 0)
        return, err
    endif

    err = manage_interrupt(/DISABLE)
    if err ne adsec_error.ok then begin
        message, 'Interrupt disabling fails!', CONT = (sc.debug eq 0)
        return, err
    endif
; ---------------------------------------------------------------------------------------------------

    disp_mess, 'System startup:'
    
;
; ---------------------------------------------------------------------------------------------------
    ;WaveForm default load in common structures
    disp_mess, 'Default WaveForm loading...',/APPEND
    
    err = init_adsec_wave()
    if err ne adsec_error.ok then return, err
;
;---------------------------------------------------------------------------------------------------
    cpr = progress[0]+fix(0.1*(progress[1]-progress[0]))
    err = update_progress(cpr)
    if err ne adsec_error.ok then return, err

 
    ;FASTLink Connection Setting
    if ~keyword_set(NO_ASK) then answ = dialog_message("Do you want to perform FastLink fiber test?", /QUEST) else answ = 'yes'
        
            if keyword_set(NOFLTEST) or strlowcase(answ) eq 'no' then begin
                 disp_mess, 'FastLink connection test skipped.',/APPEND
                 disp_mess, 'FastLinkTest skipped by user.'
                log_print, "FastLink test skipped by user"
            endif else begin
                     disp_mess, 'FastLink connection testing...',/APPEND
                    old_debug=sc.debug
                    sc.debug=0
                    ;GET STATUS OF HOUSEKEEPER VARIABLES TO BE CHANGED
                    var_list = ['CHECKDEWPOINT', 'DEWPOINT', 'EXTERNALHUMIDITY', 'EXTERNALTEMPERATURE', 'FLUXRATEIN', 'WATERCOLDPLATEINLET', 'WATERCOLDPLATEOUTLET', 'WATERMAININLET', 'WATERMAINOUTLET']
                    err = getdiagnparam(process_list.HOUSEKEEPER.MSGD_NAME,  strjoin(var_list, ' '), 0, 0, bb)
                    actual_status = bb.enabled
                    ;TEMPORARY SET THEM TO NOT ENABLED
                    err = setdiagnparam(process_list.HOUSEKEEPER.MSGD_NAME,  strjoin(var_list, ' '), 0, 0, ENABLED=0)
                    ;FASTLINK TEST
                    err = fastlink_alignment(/VER, TEST=5, /STOP) ;mod RB 
                    ;RESTORE HOUSEKEEPER VARIABLE PREVIOUS STATUS
                    for i=0, n_elements(var_list)-1 do begin
                        err2 = setdiagnparam(process_list.HOUSEKEEPER.MSGD_NAME,  var_list[i], 0, 0, ENABLED=actual_status[i])
                    endfor
        
                    if err ne adsec_error.ok then begin
                        err1 = read_seq_dsp(sc.all, '8000'xl, 20L, bb, /UL)
                        if err1 ne adsec_error.ok then begin
                        log_print, "Error trying to read data for fastlink error. "
                        return, err1
                    endif
                    cc = bb - rebin(bb[*,0], 20, adsec.n_dsp_per_board * adsec.n_board_per_bcu * adsec.n_crates, /sam)
                    id = where(cc ne 0)
                    if id[0] eq -1 then begin
                        log_print, "Fastlink test failed but unable to find the DSP which failed. "
                        return, adsec_error.IDL_FAST_LINK_ERROR
                        vv = -1
                    endif else begin
                        idb = id / 20 
                        idu = uniq(idb)
                        bd = strjoin(string(idb[idu], FORMAT='(I3, " ")'))
                        vv = idb[idu]
                        log_print, "Fastlink test failed on DSP#: "+bd
                    endelse
                    save, file = add_timestamp(filepath(ROOT=getenv("ADOPT_LOG"), '')+'fastlink_alignment_status.sav'), vv

                    return, err
                endif
                 disp_mess, 'Done', /APPEND
            endelse
        ;    if err ne adsec_error.ok then begin
        ;        answ=dialog_message("THE FASTLINK TEST FAILED and THE SYSTEM IS NOT SAFE. CONTINUE ANYWAY?", /QUEST)
        ;        if answ eq "No" then return, err
        ;    endif

    ;==========================================================================
    ; Calculating dac e adc calibations parameters
    names_sram = strupcase(tag_names(adsec))
    id_sram = where(names_sram eq strupcase('ADC_spi_offset'))
    if id_sram eq -1 then nofill_sram = 1 else nofill_sram = 0
    err=init_adsec_sram(ST=sram_data, NOFILL=nofill_sram)
    if err ne adsec_error.ok then message,"The adsec ADC e DAC calibrations fields are not initialized"
    if id_sram eq -1 then adsec=create_struct(adsec,sram_data)

    ; End calculating force calibrations parameters
    ;==========================================================================


    cpr = progress[0]+fix(0.2*(progress[1]-progress[0]))
    err = update_progress(cpr)
    if err ne adsec_error.ok then return, err



     disp_mess, "DSPBoards and SIGGEN CONFIGURATION START:", /APPEND
    err = proc_startup_dsp(prog_file_vec[0], NO_ASK=no_ask)
    if err ne adsec_error.ok then return, err
     disp_mess, "DONE", /APPEND
    cpr = progress[0]+fix(0.3*(progress[1]-progress[0]))
    err = update_progress(cpr)
    if err ne adsec_error.ok then return, err

     disp_mess, "SwitchBCU CONFIGURATION START:", /APPEND
    err = proc_startup_switch(prog_file_vec[1], NO_ASK=no_ask)
    if err ne adsec_error.ok then return, err
     disp_mess, "DONE", /APPEND
    cpr = progress[0]+fix(0.4*(progress[1]-progress[0]))
    err = update_progress(cpr)
    if err ne adsec_error.ok then return, err

     disp_mess, "CrateBCU CONFIGURATION START:", /APPEND
    err = proc_startup_bcu(prog_file_vec[2], NO_ASK=no_ask)
    if err ne adsec_error.ok then return, err
     disp_mess, "DONE", /APPEND

    cpr = progress[0]+fix(0.6*(progress[1]-progress[0]))
    err = update_progress(cpr)
    if err ne adsec_error.ok then return, err

    if prog_file_vec[3] eq 'AccAcquisition.ldr' then begin
         disp_mess, "WARNING: no accelerometer boards found.", /APPEND
    endif else begin
         disp_mess, "ACCELEROMETER CONFIGURATION START:", /APPEND
        err = proc_startup_accel(prog_file_vec[3], NO_ASK=no_ask)
        if err ne adsec_error.ok then return, err
         disp_mess, "DONE", /APPEND
    endelse
    cpr = progress[0]+fix(0.7*(progress[1]-progress[0]))
    err = update_progress(cpr)
    if err ne adsec_error.ok then return, err

;------------------------------------------------------------------------------------------------------

    ;if ~keyword_set(NO_ASK) then answ = dialog_message("Do you want to perform reconstruction configuration?", /QUEST) else answ = 'yes'

    ;if answ eq 'yes' then begin
    ;Configuration reconstructor parameters
     disp_mess, "CONFIGURE RECONSTRUCTOR PARAMETERS IN ALL BOARDS:", /APPEND

     disp_mess, 'Clearing the reconstructor memory ...',/APPEND
    err = clear_rtr_mem()
    if err ne adsec_error.ok then begin
         disp_mess, '... error on writing.', /APPEND
        return, err
    endif
     disp_mess, '... done.', /APPEND 

     disp_mess, 'Setting the reconstructor parameters ...',/APPEND
    err = set_reconstructor_par()
    if err ne adsec_error.ok then begin
         disp_mess, '... error on writing.', /APPEND
        return, err
    endif
     disp_mess, '... done.', /APPEND

     disp_mess, 'Setting the matrix address pointers ...',/APPEND
    err = set_matrix_ptr()
    if err ne adsec_error.ok then begin
         disp_mess, '... error on writing.', /APPEND
        return, err
    endif
     disp_mess, '... done.', /APPEND
;---ok
     disp_mess, 'Setting the address pointers ...',/APPEND
    if not(sc.host_comm eq "Dummy") then begin 
        err = set_all_ptr()
        if err ne adsec_error.ok then begin
             disp_mess, '... error on writing.', /APPEND
            return, err
        endif
         disp_mess, '... done.', /APPEND
    endif

     disp_mess, 'Setting the diagnostic pointers and sampling...',/APPEND
    err = set_diagnostic(/CONFIGURE)
    if err ne adsec_error.ok then begin
         disp_mess, '... error on writing.', /APPEND
        return, err
    endif
     disp_mess, '... done.', /APPEND

     disp_mess, 'Setting the disturbance pointers and vectors...',/APPEND
    err = set_disturb(/RESET)
    if err ne adsec_error.ok then begin
         disp_mess, '... error on writing.', /APPEND
        return, err
    endif
     disp_mess, '... done.', /APPEND

     disp_mess, "DONE", /APPEND

    cpr = progress[0]+fix(0.8*(progress[1]-progress[0]))
    err = update_progress(cpr)
    if err ne adsec_error.ok then return, err

;------------------------------------------------------------------------------------------------------ OK

    ;Configuring all reconstruction/filters matrices
     disp_mess, "UPLOADING RECONSTRUCTION/FILTERS MATRICES:", /APPEND

     disp_mess, 'Sending default matrix B0 block A  ...',/APPEND
    err = set_b0_matrix(*rtr.b0_a_matrix)
    if err ne adsec_error.ok then begin
         disp_mess, '... error on writing.', /APPEND
        return, err
    endif
     disp_mess, '... done.', /APPEND
     disp_mess, 'Sending default matrix B0 block B  ...',/APPEND
    err = set_b0_matrix(*rtr.b0_b_matrix, /block)
    if err ne adsec_error.ok then begin
         disp_mess, '... error on writing.', /APPEND
        return, err
    endif
     disp_mess, '... done.', /APPEND

     disp_mess, 'Sending default matrix B_delay block A  ...',/APPEND
    err = set_b_delay_matrix(*rtr.b_delay_a_matrix)
    if err ne adsec_error.ok then begin
         disp_mess, '... error on writing.', /APPEND
        return, err
    endif
     disp_mess, '... done.', /APPEND


     disp_mess, 'Sending default matrix B_delay block B  ...',/APPEND
    err = set_b_delay_matrix(*rtr.b_delay_b_matrix, /block)
    if err ne adsec_error.ok then begin
         disp_mess, '... error on writing.', /APPEND
        return, err
    endif
     disp_mess, '... done.', /APPEND

     disp_mess, 'Sending default matrix A_delay   ...',/APPEND
    err = set_a_delay_matrix(*rtr.a_delay_matrix)
    if err ne adsec_error.ok then begin
         disp_mess, '... error on writing.', /APPEND
        return, err
    endif
     disp_mess, '... done.', /APPEND

     disp_mess, 'Sending default matrix M2C   ...',/APPEND
    err = set_m2c_matrix(*rtr.m2c_matrix)
    if err ne adsec_error.ok then begin
         disp_mess, '... error on writing.', /APPEND
        return, err
    endif
     disp_mess, '... done.', /APPEND

    if adsec.meas_ff_used then begin
         disp_mess, 'Uploading the measured feedforward matrix...', /APPEND
    endif else begin
        if max(abs(adsec.ff_matrix)) ne 0.0 then begin
             disp_mess, '... measured ff-matrix not availabe, but non-zero ff-matrix is going to be load...', /APPEND
             disp_mess, '... This is not allowed. A zero ff-matrix is needed...', /APPEND
             disp_mess, '... error on uploading the ff matrix.', /APPEND
            return, err
        endif
         disp_mess, 'Uploading a zero feedforward matrix...', /APPEND
    endelse
    err = send_ff_matrix(adsec.ff_matrix)
    if err ne adsec_error.ok then begin
         disp_mess, '... error on uploading the ff matrix.', /APPEND
        return, err
    endif
     disp_mess, '... done.', /APPEND

;------------------------------------------------------------------------------------------------------ OK

    ;reset optical loop gain
     disp_mess, 'Cleaning Optical Gain   ...',/APPEND
    err = set_g_gain(/reset_all)
    if err ne adsec_error.ok then begin
         disp_mess, '... error on reseting optical gain.', /APPEND
        return, err
    endif
     disp_mess, '... done.', /APPEND


;------------------------------------------------------------------------------------------------------ OK

    ;clear delay lines
     disp_mess, 'Cleaning Delay line filter    ...',/APPEND
    err = clear_dl()
    if err ne adsec_error.ok then begin
         disp_mess, '... error on clearing filter delay line.', /APPEND
        return, err
    endif
     disp_mess, '... done.', /APPEND

;------------------------------------------------------------------------------------------------------ OK

   ;clear modes offload parameters
     disp_mess, 'Cleaning Offload lines ...',/APPEND
    err = set_offload_cmd(/RESET, /NOCHECK)
    if err ne adsec_error.ok then begin
         disp_mess, '... error on cleaning offload commands.', /APPEND
        return, err
    endif
     disp_mess, '... done.', /APPEND

    err = set_offload_force(/RESET, /NOCHECK)
    if err ne adsec_error.ok then begin
         disp_mess, '... error on cleaning offload forces.', /APPEND
        return, err
    endif
     disp_mess, '... done.', /APPEND


    ;endif else begin
    ;    return, adsec_error.generic_error
    ;endelse
;------------------------------------------------------------------------------------------------------ OK

  ;  if ~keyword_set(NO_ASK) then answ = dialog_message("Do you want to enable controll loop interrupt?", /QUEST) else answ = 'yes'
  ;  if answ eq 'yes' then begin

    cpr = progress[0]+fix(0.9*(progress[1]-progress[0]))
    err = update_progress(cpr)
    if err ne adsec_error.ok then return, err

    err = manage_interrupt(/ENABLE)
    if err ne adsec_error.ok then begin
        message, 'Interrupt disabling fails!', CONT = (sc.debug eq 0)
        return, err
    endif

;------------------------------------------------------------------------------------------------------ OK

    ;setting rtr_sleep to 0 and setting all the starts to 0 waiting for a start_rtr.
    err=wakeup4update()
    if err ne adsec_error.ok then begin
        message, 'New register initialization failed, and no start is ready!', CONT = (sc.debug eq 0)
        return, err
    endif

;------------------------------------------------------------------------------------------------------ OK
    ;Test software/firmware versions
    err = get_sw_ver()
    if err ne adsec_error.ok then return, err

   ;Test software/firmware versions
    ;err = get_sw_ver(/FWCHECK)    
    err = get_sw_ver(/FWCHECK, /DISPL, PVR=pvr)
    if err ne adsec_error.ok then return, err
    prog_vers = pvr
    ;endif else begin
    ;    return, adsec_error.generic_error
    ;endelse

;------------------------------------------------------------------------------------------------------ OK

    ;if ~keyword_set(NO_ASK) then answ = dialog_message("Do you want to perform accelerometer configuration?", /QUEST) else answ = 'yes'

    ;if answ eq "yes" then begin
    ;Start ACCELEROMETER program
     disp_mess, 'Starting the ACCELEROMETER program...',/APPEND
    err = start_program(5, /SIGGEN)
    if err ne adsec_error.ok then begin
         disp_mess, '... error on starting the ACCELEROMETER program.', /APPEND
        return, err
    endif
     disp_mess, '... done.', /APPEND

    ;Start DSP program
     disp_mess, 'Starting the DSP program...',/APPEND
    err = start_program(sc.all)
    if err ne adsec_error.ok then begin
         disp_mess, '... error on starting the DSP program.', /APPEND
        return, err
    endif
     disp_mess, '... done.', /APPEND
    ;endif else begin
    ;    return, adsec_error.generic_error
    ;endelse

    if not keyword_set(no_ask) then begin
        answ = dialog_message(["Do you want to syncronize DSPs?"], /QUESTION)
    endif else answ='yes'

    if strlowcase(answ) eq "yes" then begin
         disp_mess, 'Syncronizing the DSPs (it will take a while)...',/APPEND
        err = sync_dsp()
        if err ne adsec_error.ok then begin
             disp_mess, '... error on Syncronizing DSPs.', /APPEND
            ans = dialog_message(["The DSPs cannot be syncronized.", $
                                  "Procedures that suppose the DSP syncronized", $
                                  "cannot be used."])
        endif
         disp_mess, '... done.', /APPEND
    endif

    ;Start BCU program
     disp_mess, 'Starting the BCU program...',/APPEND
    err = start_program(indgen(adsec.n_bcu), /BCU)
    if err ne adsec_error.ok then begin
         disp_mess, '... error on starting the DSP program.', /APPEND
        return, err
    endif
     disp_mess, '... done.', /APPEND

    ;Start SWITCH BCU program
     disp_mess, 'Starting the BCU program...',/APPEND
    err = start_program(0, /SWITCH)
    if err ne adsec_error.ok then begin
         disp_mess, '... error on starting the DSP program.', /APPEND
        return, err
    endif
     disp_mess, '... done.', /APPEND
;------------------------------------------------------------------------------------------------------;OK
;Enable DSP watchdog 
     disp_mess, 'Enabling DSP WATCHDOG...',/APPEND
    err = enable_watchdog(/DSP)
    if err ne adsec_error.ok then begin
         disp_mess, '... error on  DSP watchdog enabling.', /APPEND
        return, err
    endif
     disp_mess, '... done.', /APPEND

;------------------------------------------------------------------------------------------------------ OK
    ;Test dsp versions
    ;err = get_sw_ver(/CHECK)   
    err = get_sw_ver(/CHECK, /DISPL)
    if err ne adsec_error.ok then return, err

;------------------------------------------------------------------------------------------------------ OK
; Enable the storage of the diagnostic data in the DSP SDRAM
   disp_mess, 'Enabling the storage of the diagnostic data in DSP SDRAM...',/APPEND

  err=dspstorage() ;TO DEBUG 
 
  if err ne adsec_error.ok then begin
         disp_mess, '... error setting storage of diagnostic data.', /APPEND
        return, err
  endif



;-------------------------------------------------------------------------------------------------------    
;Set thresholds for dummy acts
     disp_mess, 'Enable distance thresholds and exclude dummy acts...', /APPEND

    if adsec.dummy_act[0] gt 0 then begin

        err = set_dist_threshold(adsec.dummy_act, 4., 1L)
   
        if err ne adsec_error.ok then begin
             disp_mess, '... error setting distance threshold on dummy act.', /APPEND
            return, err
        endif

    endif
    if adsec.act_wo_pos[0] gt 0 then begin

        err = set_dist_threshold(adsec.act_wo_pos, 4., 1L)
        if err ne adsec_error.ok then begin
             disp_mess, '... error setting distance threshold on act_wo_pos.', /APPEND
            return, err
        endif

    endif
    err = set_dist_threshold(adsec.act_w_pos, 125e-6, 15L, /CLEAN)
    
    if err ne adsec_error.ok then begin
         disp_mess, '... error on setting distance threshold on good act.', /APPEND
        return, err
    endif

     disp_mess, 'Done.', /APPEND
    
     disp_mess, 'Enable Pending Frame Safety....', /APPEND
    err = enable_pending()
   if err ne adsec_error.ok then begin
         disp_mess, '... error on enabling pending safety.', /APPEND
        return, err
    endif


;     disp_mess, 'Configuring Fast Diagnostics....', /APPEND
;    config_diagnostics
;    if err ne adsec_error.ok then begin
;         disp_mess, '... error on enabling pending safety.', /APPEND
;        return, err
;    endif


;-------------------------------------------------------------------------------------------------------    
    ;Enabling coils
     disp_mess, 'Enabling all coils...',/APPEND
    err = enable_coils()
    if err ne adsec_error.ok then begin
         disp_mess, '... error on enabling coils.', /APPEND
        return, err
    endif

    if adsec.act_wo_curr[0] gt 0 then begin
        err = disable_coils(adsec.act_wo_curr)
        if err ne adsec_error.ok then begin
              disp_mess, '... error on enabling coils.', /APPEND
             return, err
         endif
     endif
 
    cpr = progress[1]
    err = update_progress(cpr)
    if err ne adsec_error.ok then return, err


    return, adsec_error.ok

End
