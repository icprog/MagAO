; $Id: proc_startup_bcu.pro,v 1.4 2008/03/18 14:54:33 labot Exp $$
;+
;   NAME:
;    PROC_STARTUP_BCU
;
;   PURPOSE:
;    Configuration and cleaning BCU boards.
;
;   USAGE:
;    err = PROC_STARTUP_DSP([prog_file])
;   
;   INPUT:
;    prog_file: program file to download
;
;   OUTPUT:
;    Error code.
;   
;   KEYWORDS:
;    None.
;   
;   HISTORY
;    Written by M.Xompero (MX) 26 Jan 2007
;    Osservatorio Asrofisico di Arcetri, ITALY
;    <marco@arcetri.astro.it>
;    20 Mar 2007, MX & DZ
;       Multi crate configuration fixed.
;-


Function proc_startup_bcu, prog_file, NO_ASK=no_ask

    @adsec_common

    disp_mess, 'crateBCU startup procedure:', /APPEND

    if not keyword_set(no_ask) then begin
        ans = dialog_message(['The DSPs will be soft-reset and the program reloded.', $
                          'Click on "Ok" to continue, "Cancel" to stop here.'], /CANCEL)
    endif else if keyword_set(comm_only) then ans='cancel' else ans='ok'

    if strlowcase(ans) eq 'cancel' then begin
            return, adsec_error.cancel
    endif
    
    ;DSP stop and reset
    disp_mess, 'Resetting all the DSPs...', /APPEND
    err = force_dsp_idle(sc.all, /BCU)
    if err ne adsec_error.ok then begin
        disp_mess, 'Resetting FAILED.', /APPEND
        return, err
    endif
    disp_mess, '...resetting done.', /APPEND

    ;Communication test
    disp_mess, 'Executing the communication test...', /APPEND
    if switch_datasheet.data_mem_size gt 0 then begin
        ; Communication test using WR_SAME in data memory
        data_len = dsp_const.test_len
        err = communication_test(0, 0, 0, rtr_map.test_address-dsp_const.test_len, $
                                 data_len, WORD_WRONG=n_ww, ITER=20, /RETRY, /VERBOSE, /BCU)
        if n_ww ne 0 then begin
            disp_mess, '...Write Same Test in data memory FAILED. ('+strtrim(n_ww,2) $
                       +' of '+strtrim(data_len*adsec.n_dsp,2)+')', /APPEND
            return, adsec_error.communication
        endif
        disp_mess, '... Write same (DM) test done...', /APPEND

        ; Communication test using WR_SEQ in data memory
        err = communication_test(1, 0, 0, rtr_map.test_address-dsp_const.test_len, $
                                 data_len, WORD_WRONG=n_ww, ITER=20, /RETRY, /VERBOSE, /BCU)
        if n_ww ne 0 then begin
            disp_mess, '...Write Seq Test in data memory FAILED. ('+strtrim(n_ww,2) $
                       +' of '+strtrim(data_len*adsec.n_dsp,2)+')', /APPEND
            return, adsec_error.communication
        endif
        disp_mess, '... Write seq (DM) test done...', /APPEND
    endif

    if switch_datasheet.prog_mem_size gt 0 then begin
        ; Communication test using WR_SAME in program memory
        data_len = dsp_const.test_len
        err = communication_test(0, 0, 0, dsp_map_pm.test_address-dsp_const.test_len, $
                                 data_len, WORD_WRONG=n_ww, ITER=20, /SET_PM, /RETRY, /VERBOSE, /BCU)
        if n_ww ne 0 then begin
            disp_mess, '...Write Same Test in program memory FAILED. ('+strtrim(n_ww,2) $
                       +' of '+strtrim(data_len*adsec.n_dsp,2)+')', /APPEND
            return, adsec_error.communication
        endif
        disp_mess, '... Write same (PM) test done...', /APPEND

        ; Communication test using WR_SEQ in program memory
        err = communication_test(1, 0, 0, dsp_map_pm.test_address-dsp_const.test_len, $
                                 data_len, WORD_WRONG=n_ww, ITER=20, /SET_PM, /RETRY, /VERBOSE, /BCU)
        if n_ww ne 0 then begin
            disp_mess, '...Write Seq Test in program memory FAILED. ('+strtrim(n_ww,2) $
                       +' of '+strtrim(data_len*adsec.n_dsp,2)+')', /APPEND
            return, adsec_error.communication
        endif
        disp_mess, '... Write seq (PM) test done...', /APPEND
    endif
    disp_mess, '... communication test done.', /APPEND

    ;DOWNLOAD PROGRAM
    if n_elements(prog_file) eq 0 then begin
        filter = strsplit(dsp_const.prog_name, ".", /EXTRACT)
        filter = "*."+filter[n_elements(filter)-1]
        prog_file = dialog_pickfile(PATH=adsec_path.prog, FILTER=filter, $
                                    TITLE="Select a SWITCH BCU program:", /MUST_EXIST)
    endif
    disp_mess, 'Uploading the DSP program: '+prog_file+'...',/APPEND
    err = download_program(0,adsec.n_crates-1,prog_file, /NORESET, /NOSTART, /VERB, /BCU)
    if err ne adsec_error.ok then begin
        disp_mess, '... error uploading the DSP program in crate #'+strtrim(nc,2), /APPEND
        return, err
    endif
    disp_mess, '... SWITCH BCU done.', /APPEND
    disp_mess, 'DSP program uploaded.',/APPEND

    disp_mess, 'Executing the communication test...', /APPEND
    if switch_datasheet.data_mem_size gt 0 then begin
        ; Communication test using WR_SAME in data memory
        data_len = dsp_const.test_len
        err = communication_test(0, 0, 0, rtr_map.test_address-dsp_const.test_len, $
                                 data_len, WORD_WRONG=n_ww, ITER=20, /RETRY, /VERBOSE, /BCU)
        if n_ww ne 0 then begin
            disp_mess, '...Write Same Test in data memory FAILED. ('+strtrim(n_ww,2) $
                       +' of '+strtrim(data_len*adsec.n_dsp,2)+')', /APPEND
            return, adsec_error.communication
        endif
        disp_mess, '... Write same (DM) test done...', /APPEND

        ; Communication test using WR_SEQ in data memory
        err = communication_test(1, 0, 0, rtr_map.test_address-dsp_const.test_len, $
                                 data_len, WORD_WRONG=n_ww, ITER=20, /RETRY, /VERBOSE, /BCU)
        if n_ww ne 0 then begin
            disp_mess, '...Write Seq Test in data memory FAILED. ('+strtrim(n_ww,2) $
                       +' of '+strtrim(data_len*adsec.n_dsp,2)+')', /APPEND
            return, adsec_error.communication
        endif
        disp_mess, '... Write seq (DM) test done...', /APPEND
    endif

    if switch_datasheet.prog_mem_size gt 0 then begin
        ; Communication test using WR_SAME in program memory
        data_len = dsp_const.test_len
        err = communication_test(0, 0, 0, dsp_map_pm.test_address-dsp_const.test_len, $
                                 data_len, WORD_WRONG=n_ww, ITER=20, /SET_PM, /RETRY, /VERBOSE, /BCU)
        if n_ww ne 0 then begin
            disp_mess, '...Write Same Test in program memory FAILED. ('+strtrim(n_ww,2) $
                       +' of '+strtrim(data_len*adsec.n_dsp,2)+')', /APPEND
            return, adsec_error.communication
        endif
        disp_mess, '... Write same (PM) test done...', /APPEND

        ; Communication test using WR_SEQ in program memory
        err = communication_test(1, 0, 0, dsp_map_pm.test_address-dsp_const.test_len, $
                                 data_len, WORD_WRONG=n_ww, ITER=20, /SET_PM, /RETRY, /VERBOSE, /BCU)
        if n_ww ne 0 then begin
            disp_mess, '...Write Seq Test in program memory FAILED. ('+strtrim(n_ww,2) $
                       +' of '+strtrim(data_len*adsec.n_dsp,2)+')', /APPEND
            return, adsec_error.communication
        endif
        disp_mess, '... Write seq (PM) test done...', /APPEND
    endif
    disp_mess, '... communication test done.', /APPEND
    
;  --------------------------------------------------------------------------------------------------------

;    disp_mess, 'Clear and disable disturbance '
;    err = set_disturb(sc.all, 0.0, /CHECK)
;    if err ne adsec_error.ok then begin
;        disp_mess, '... ERROR configuring fastlink connection!', /APPEND
;        return, err
;    endif
;    disp_mess, 'Done.', /APPEND

    return, adsec_error.ok
end
