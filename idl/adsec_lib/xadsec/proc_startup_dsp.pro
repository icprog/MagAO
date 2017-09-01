; $Id: proc_startup_dsp.pro,v 1.5 2008/10/02 14:01:48 labot Exp $$

;+
;   NAME:
;    PROC_STARTUP_DSP
;
;   PURPOSE:
;    Configuration and cleaning dsp boards.
;
;   USAGE:
;    err = PROC_STARTUP_DSP([prog_file, NO_ASK=no_ask])
;   
;   INPUT:
;    prog_file: program file to download
;
;   OUTPUT:
;    Error code.
;   
;   KEYWORDS:
;    NO_ASK: automatic DSPs synchronization.
;   
;   HISTORY
;    Written by A. Riccardi (AR)
;    Osservatorio Asrofisico di Arcetri, ITALY
;    <riccardi@arcetri.astro.it>
;
;    13 Mar 2002, AR
;      n_dsp > 256 is now handled.
;      The procedure handles the "Dummy" communication correctly, now.
;    14 July 2004, AR
;      request of hard-reset removed. Software-reset has been implemented.
;      communication opening code has been moved in open_communication function
;      various modifications for matching LBT specs
;      added code to configure new LBT parameters:
;       set_smoothed_gain_step, set_sat_force, set_shape_preshaper,
;       start_program.
;    04 Aug 2004, AR and M. Xompero
;      setting default configuration for accumulators
;    02 Nov 2004, MX
;      Adsec.all changed in sc.all.
;    01 Dec 2004, AR
;      uploading of reference waveform and timing parameters added
;    13 Dec 2004, MX
;      Reordered startup initialization.
;    14 Feb 2005, Daniela Zanotti(DZ)
;      Initialized to a zero matrix B0, A_delay, B_delay.
;      Inizialized all the pointers address.
;      Initialized ti 1UL the all nendof parameters.
;    06 Apr 2005, AR
;      removed obsolete keyword NODIV in set_acc
;    25 May 2005, MX
;      Dsp_map updated to rtr_map.
;    03 Aug 2005, MX
;      Position vector, delta_position vector, precalc_modes and precalc_slope cleaning added.
;    04 Aug 2005, MX
;      Added a field to set the MsgRTDB variables. Now only the BCUXXDIAG is set to reset the
;      MirrorCtrl frames counter.
;    07 Oct 2005, MX
;      RTR matrices now managed by pointers.
;    30 Nov 2005, MX
;      General reorganizing of the current and filter cleaning. wake4update added. NO_ASK kwd added.
;    11 Dec 2005, MX
;      Three reconstructor start now added at the end of initialization.
;    23 Jan 2006, DZ
;      Updated the initialization with the changes of rtr structure.
;    26 Jan 2007, MX
;       Function name changed and help, updated.
;    07 Feb 2007 MX
;       LUT of variable deleted and name refurbished. The previous name of this procedure is "proc_startup"
;    26 Feb 2007 MX
;       FF_Matrix initialization moved to proc_startup procedure. Some
;       unused line removed.
;    01 Oct 2008, MX
;       NO_OPEN_COMMUNICATION and COMM_ONLY keywords removed. 
;-


function proc_startup_dsp, prog_file, NO_ASK=no_ask

    @adsec_common

    disp_mess, 'dspBoard startup procedure:', /APPEND

;    if not keyword_set(no_open) then begin
;        disp_mess, 'Initializing '+ sc.host_comm +' communication...', /APPEND

;       err = open_communication(NO_ASK=no_ask)
;       if err ne adsec_error.ok then begin
;         disp_mess, '... initialization FAILED!', /APPEND
;         return, err
;       endif
;       disp_mess, '... done.', /APPEND
;   endif
;   wait, 0.1

    if not keyword_set(no_ask) then begin
        ans = dialog_message(['The DSPs will be soft-reset and the program reloded.', $
                              'Click on "Ok" to continue, "Cancel" to stop here.'], /CANCEL)
        if strlowcase(ans) eq 'cancel' then begin
            return, adsec_error.cancel
        endif
    endif ;else if keyword_set(comm_only) then ans='cancel' else ans='ok'


    disp_mess, 'Resetting all the DSPs...', /APPEND
    err = force_dsp_idle(sc.all)
    if err ne adsec_error.ok then begin
        disp_mess, 'Resetting FAILED.', /APPEND
        return, err
    endif
    disp_mess, '...resetting done.', /APPEND


    disp_mess, 'Executing the communication test...', /APPEND
    if dsp_datasheet.data_mem_size gt 0 then begin
        ; Communication test using WR_SAME in data memory
        data_len = dsp_const.test_len
        err = communication_test(0, 0, adsec.last_dsp, rtr_map.test_address-dsp_const.test_len, $
                                 data_len, WORD_WRONG=n_ww, ITER=20, /RETRY, /VERBOSE)
        if n_ww ne 0 then begin
            disp_mess, '...Write Same Test in data memory FAILED. ('+strtrim(n_ww,2) $
                       +' of '+strtrim(data_len*adsec.n_dsp,2)+')', /APPEND
            return, adsec_error.communication
        endif
        disp_mess, '... Write same (DM) test done...', /APPEND

        ; Communication test using WR_SEQ in data memory
        err = communication_test(1, 0, adsec.last_dsp, rtr_map.test_address-dsp_const.test_len, $
                                 data_len, WORD_WRONG=n_ww, ITER=20, /RETRY, /VERBOSE)
        if n_ww ne 0 then begin
            disp_mess, '...Write Seq Test in data memory FAILED. ('+strtrim(n_ww,2) $
                       +' of '+strtrim(data_len*adsec.n_dsp,2)+')', /APPEND
            return, adsec_error.communication
        endif
        disp_mess, '... Write seq (DM) test done...', /APPEND
    endif

    if dsp_datasheet.prog_mem_size gt 0 then begin
        ; Communication test using WR_SAME in program memory
        data_len = dsp_const.test_len/2
        err = communication_test(0, 0, adsec.last_dsp, dsp_map_pm.test_address-dsp_const.test_len, $
                                 data_len, WORD_WRONG=n_ww, ITER=20, /SET_PM, /RETRY, /VERBOSE)
        if n_ww ne 0 then begin
            disp_mess, '...Write Same Test in program memory FAILED. ('+strtrim(n_ww,2) $
                       +' of '+strtrim(data_len*adsec.n_dsp,2)+')', /APPEND
            return, adsec_error.communication
        endif
        disp_mess, '... Write same (PM) test done...', /APPEND

        ; Communication test using WR_SEQ in program memory
        err = communication_test(1, 0, adsec.last_dsp, dsp_map_pm.test_address-dsp_const.test_len, $
                                 data_len, WORD_WRONG=n_ww, ITER=20, /SET_PM, /RETRY, /VERBOSE)
        if n_ww ne 0 then begin
            disp_mess, '...Write Seq Test in program memory FAILED. ('+strtrim(n_ww,2) $
                       +' of '+strtrim(data_len*adsec.n_dsp,2)+')', /APPEND
            return, adsec_error.communication
        endif
        disp_mess, '... Write seq (PM) test done...', /APPEND
    endif
    disp_mess, '... communication test done.', /APPEND

   if n_elements(prog_file) eq 0 then begin
        filter = strsplit(dsp_const.prog_name, ".", /EXTRACT)
        filter = "*."+filter[n_elements(filter)-1]
        prog_file = dialog_pickfile(PATH=adsec_path.prog, FILTER=filter, $
                                    TITLE="Select a DSP porgram:", /MUST_EXIST)
    endif
    disp_mess, 'Uploading the DSP program: '+prog_file+'...',/APPEND
    n_dsp_per_crate = adsec.n_dsp/adsec.n_crates
    for nc=0,adsec.n_crates-1 do begin
        err = download_program(n_dsp_per_crate*nc, n_dsp_per_crate*(nc+1)-1, prog_file, /NORESET, /NOSTART, /VERB)
        if err ne adsec_error.ok then begin
            disp_mess, '... error uploading the DSP program in crate #'+strtrim(nc,2), /APPEND
            return, err
        endif
        disp_mess, '... crate #'+strtrim(nc,2)+' done.', /APPEND
    endfor
    disp_mess, 'DSP program uploaded.',/APPEND

    disp_mess, 'Executing the communication test...', /APPEND
    if dsp_datasheet.data_mem_size gt 0 then begin
        ; Communication test using WR_SAME in data memory
        data_len = dsp_const.test_len
        err = communication_test(0, 0, adsec.last_dsp, rtr_map.test_address-dsp_const.test_len, $
                                 data_len, WORD_WRONG=n_ww, ITER=20, /RETRY, /VERBOSE)
        if n_ww ne 0 then begin
            disp_mess, '...Write Same Test in data memory FAILED. ('+strtrim(n_ww,2) $
                       +' of '+strtrim(data_len*adsec.n_dsp,2)+')', /APPEND
            return, adsec_error.communication
        endif
        disp_mess, '... Write same (DM) test done...', /APPEND

        ; Communication test using WR_SEQ in data memory
        err = communication_test(1, 0, adsec.last_dsp, rtr_map.test_address-dsp_const.test_len, $
                                 data_len, WORD_WRONG=n_ww, ITER=20, /RETRY, /VERBOSE)
        if n_ww ne 0 then begin
            disp_mess, '...Write Seq Test in data memory FAILED. ('+strtrim(n_ww,2) $
                       +' of '+strtrim(data_len*adsec.n_dsp,2)+')', /APPEND
            return, adsec_error.communication
        endif
        disp_mess, '... Write seq (DM) test done...', /APPEND
    endif

    if dsp_datasheet.prog_mem_size gt 0 then begin
        ; Communication test using WR_SAME in program memory
        data_len = dsp_const.test_len/2.
        err = communication_test(0, 0, adsec.last_dsp, dsp_map_pm.test_address-dsp_const.test_len, $
                                 data_len, WORD_WRONG=n_ww, ITER=20, /SET_PM, /RETRY, /VERBOSE)
        if n_ww ne 0 then begin
            disp_mess, '...Write Same Test in program memory FAILED. ('+strtrim(n_ww,2) $
                       +' of '+strtrim(data_len*adsec.n_dsp,2)+')', /APPEND
            return, adsec_error.communication
        endif
        disp_mess, '... Write same (PM) test done...', /APPEND

        ; Communication test using WR_SEQ in program memory
        err = communication_test(1, 0, adsec.last_dsp, dsp_map_pm.test_address-dsp_const.test_len, $
                                 data_len, WORD_WRONG=n_ww, ITER=20, /SET_PM, /RETRY, /VERBOSE)
        if n_ww ne 0 then begin
            disp_mess, '...Write Seq Test in program memory FAILED. ('+strtrim(n_ww,2) $
                       +' of '+strtrim(data_len*adsec.n_dsp,2)+')', /APPEND
            return, adsec_error.communication
        endif
        disp_mess, '... Write seq (PM) test done...', /APPEND
    endif
    disp_mess, '... communication test done.', /APPEND
    
    disp_mess, 'Reset bias current...', /APPEND
    err = write_bias_curr(sc.all, 0.0, /CHECK)
    if err ne adsec_error.ok then begin
        disp_mess, '... ERROR resetting bias position commands!', /APPEND
        return, err
    endif
    disp_mess, 'Done.', /APPEND

    disp_mess, 'Resetting bias position command...', /APPEND
    err = write_bias_cmd(0.0, /CHECK)
    if err ne adsec_error.ok then begin
        disp_mess, '... ERROR resetting bias position commands!', /APPEND
        return, err
    endif

    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    ; Upload the reference waveform params
    disp_mess, 'Uploading the reference signal parameters...',/APPEND
    err= send_wave()
    if err ne adsec_error.ok then begin
        disp_mess, '... ERROR uploading the reference signal parameters!', /APPEND
        return, err
    endif
    disp_mess, '... done.', /APPEND

    disp_mess, 'Setting the "board_number" DSP variables...',/APPEND
    err = set_who_is()
    if err ne adsec_error.ok then begin
        disp_mess, '... error on writing.', /APPEND
        return, err
    endif
    disp_mess, '... done.', /APPEND

    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    ;Setting loop configuration and calibration

    disp_mess, 'Sending default accumulator configuration...',/APPEND
    err = set_acc(1024L,0L)
    if err ne adsec_error.ok then begin
        disp_mess, '... error on writing.', /APPEND
        return, err
    endif
    disp_mess, '... done.', /APPEND

    disp_mess, 'Setting default value of gain step ...', /APPEND
    err = set_smoothed_gain_step(sc.all_actuators, adsec.err_smoothed_step)
    if err ne adsec_error.ok then begin
        disp_mess, '... error on setting default value of gain step (err loop).', /APPEND
        return, err
    endif
    err = set_smoothed_gain_step(sc.all_actuators, adsec.speed_smoothed_step, /SPEED)
    if err ne adsec_error.ok then begin
        disp_mess, '... error on setting default value of gain step (speed loop).', /APPEND
        return, err
    endif
    disp_mess, '... done.', /APPEND

    disp_mess, 'Setting to zero the loop gains...', /APPEND
    err = set_gain(sc.all_actuators, 0.0)
    if err ne adsec_error.ok then begin
        disp_mess, '... error on setting to zero the gain of the position error loop.', /APPEND
        return, err
    endif
    err = set_gain(sc.all_actuators, 0.0, /SPEED)
    if err ne adsec_error.ok then begin
        disp_mess, '... error on setting to zero the gain of the position (for speed ctrl) loop.', /APPEND
        return, err
    endif
    disp_mess, '... done.', /APPEND

    disp_mess, 'Computing and uploading the linearization coefficents...', /APPEND
    err = send_linearization()
    if err ne adsec_error.ok then begin
        disp_mess, '... error on uploading the linearization coeffs.', /APPEND
        return, err
    endif
    disp_mess, '... done.', /APPEND

    disp_mess, 'Computing and uploading the force calibration coeffs...', /APPEND
    err = send_force_calibration()
    if err ne adsec_error.ok then begin
        disp_mess, '... error on uploading the force calibration coeffs.', /APPEND
        return, err
    endif
    disp_mess, '... done.', /APPEND

    disp_mess, 'Setting the force saturation values...', /APPEND
    err = set_sat_force(sc.all, adsec.pos_sat_force, adsec.neg_sat_force)
    if err ne adsec_error.ok then begin
        disp_mess, '... error on setting the force saturation values.', /APPEND
        return, err
    endif
    disp_mess, '... done.', /APPEND

    disp_mess, 'Computing and uploading the control coefficents...', /APPEND
    err = send_ctrl_coeff(sc.all_actuators)
    if err ne adsec_error.ok then begin
        disp_mess, '... error on uploading the control coeffs.', /APPEND
        return, err
    endif
    disp_mess, '... done.', /APPEND

    disp_mess, 'Setting the command and ff preshaper shape...',/APPEND
    err=set_shape_preshaper(sc.all)
    if err ne adsec_error.ok then begin
        disp_mess, '... error on setting the command preshaper shape.', /APPEND
        return, err
    endif

    err=set_shape_preshaper(sc.all,/FF)
    if err ne adsec_error.ok then begin
        disp_mess, '... error on setting the FF preshaper shape.', /APPEND
        return, err
    endif
    disp_mess, '... done.', /APPEND

    disp_mess, 'Setting the command and ff preshapers to 1 (slow)...',/APPEND
    err=set_preshaper(sc.all_actuators,1UL)
    if err ne adsec_error.ok then begin
        disp_mess, '... error on setting the command preshaper.', /APPEND
        return, err
    endif

    err=set_preshaper(sc.all_actuators,1UL,/FF)
    if err ne adsec_error.ok then begin
        disp_mess, '... error on setting the FF preshaper.', /APPEND
        return, err
    endif
    disp_mess, '... done.', /APPEND

    disp_mess, 'Resetting the ff currents and ff filter coefficents...',/APPEND
    err = clear_ff(/RESET_FILTER)
    if err ne adsec_error.ok then begin
        disp_mess, '... error on resetting ff.', /APPEND
        return, err
    endif
    disp_mess, '... done.', /APPEND

;;;;;;;;;;;;;; debug lines ;;;;;;;;;;;;;;;;;;;;;;;;
;    disp_mess, 'Cleaning position vector and delta position vector...'
;    err = set_position_vector(fltarr(adsec.n_actuators), /ch)
;    if err ne adsec_error.ok then begin
;        disp_mess, '... error on cleaning position vector.', /APPEND
;        return, err
;    endif
;;;;;;;;;;;;;;;; end debug lines ;;;;;;;;;;;;;;;;;;;;;;;;;

    ;Clearing accelerometers coefficients
    ;;err=clear_accelerometers()
    ;if err ne adsec_error.ok then begin
    ;    message, 'Error on clearing disturbance buffer.!', CONT = (sc.debug eq 0)
    ;    return, err
    ;endif

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    disp_mess, 'Setting the msgdrdb variables....',/APPEND
;    err = set_msgd_var(sc.msgd_diag_name, 1L)      ;to reset the frame counter in the MirrCtrl
;    if err ne adsec_error.ok then begin
;        disp_mess, '... error on setting the command preshaper.', /APPEND
;        return, err
;    endif


    return, adsec_error.ok
end
