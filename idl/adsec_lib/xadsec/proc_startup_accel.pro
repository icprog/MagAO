; $Id: proc_startup_accel.pro,v 1.3 2009/12/21 10:55:08 marco Exp $$
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


Function proc_startup_accel, prog_file, NO_ASK=no_ask, NOFILT=nofilt

    @adsec_common

    idacc = where(prog_vers.crate[*].siggen[0].is_acc)
    ;;;;; configuro solo una scheda!!!! ;;;;;
    idacc = idacc[2]

    disp_mess, 'accBoard startup procedure:', /APPEND

    if not keyword_set(no_ask) then begin
        ans = dialog_message(['The DSPs will be soft-reset and the program reloded.', $
                          'Click on "Ok" to continue, "Cancel" to stop here.'], /CANCEL)
    endif else if keyword_set(comm_only) then ans='cancel' else ans='ok'

    if strlowcase(ans) eq 'cancel' then begin
            return, adsec_error.cancel
    endif
    
    ;DSP stop and reset
    disp_mess, 'Resetting all the DSPs...', /APPEND
    err = force_dsp_idle(idacc, /SIGGEN)
    if err ne adsec_error.ok then begin
        disp_mess, 'Resetting FAILED.', /APPEND
        return, err
    endif
    disp_mess, '...resetting done.', /APPEND

    ;Communication test
;    disp_mess, 'Executing the communication test...', /APPEND
;    if accel_datasheet.data_mem_size gt 0 then begin
;        ; Communication test using WR_SAME in data memory
;        data_len = dsp_const.test_len
;        err = communication_test(0, 0, 0, rtr_map.test_address-dsp_const.test_len, $
;                                 data_len, WORD_WRONG=n_ww, ITER=20, /RETRY, /VERBOSE, /ACCEL)
;        if n_ww ne 0 then begin
;            disp_mess, '...Write Same Test in data memory FAILED. ('+strtrim(n_ww,2) $
;                       +' of '+strtrim(data_len*adsec.n_dsp,2)+')', /APPEND
;            return, adsec_error.communication
;        endif
;        disp_mess, '... Write same (DM) test done...', /APPEND

;        ; Communication test using WR_SEQ in data memory
;        err = communication_test(1, 0, 0, \, $
;                                 data_len, WORD_WRONG=n_ww, ITER=20, /RETRY, /VERBOSE, /ACCEL)
;        if n_ww ne 0 then begin
;            disp_mess, '...Write Seq Test in data memory FAILED. ('+strtrim(n_ww,2) $
;                       +' of '+strtrim(data_len*adsec.n_dsp,2)+')', /APPEND
;            return, adsec_error.communication
;        endif
;        disp_mess, '... Write seq (DM) test done...', /APPEND
;    endif

;    if accel_datasheet.prog_mem_size gt 0 then begin
;        ; Communication test using WR_SAME in program memory
;        data_len = dsp_const.test_len
;        err = communication_test(0, 0, 0, dsp_map_pm.test_address-dsp_const.test_len, $
;                                 data_len, WORD_WRONG=n_ww, ITER=20, /SET_PM, /RETRY, /VERBOSE, /SIGGEN)
;        if n_ww ne 0 then begin
;            disp_mess, '...Write Same Test in program memory FAILED. ('+strtrim(n_ww,2) $
;                       +' of '+strtrim(data_len*adsec.n_dsp,2)+')', /APPEND
;            return, adsec_error.communication
;        endif
;        disp_mess, '... Write same (PM) test done...', /APPEND

;        ; Communication test using WR_SEQ in program memory
;        err = communication_test(1, 0, 0, dsp_map_pm.test_address-dsp_const.test_len, $
;                                 data_len, WORD_WRONG=n_ww, ITER=20, /SET_PM, /RETRY, /VERBOSE, /SIGGEN)
;        if n_ww ne 0 then begin
;            disp_mess, '...Write Seq Test in program memory FAILED. ('+strtrim(n_ww,2) $
;                       +' of '+strtrim(data_len*adsec.n_dsp,2)+')', /APPEND
;            return, adsec_error.communication
;        endif
;        disp_mess, '... Write seq (PM) test done...', /APPEND
;    endif
;    disp_mess, '... communication test done.', /APPEND

    ;DOWNLOAD PROGRAM
    if n_elements(prog_file) eq 0 then begin
        filter = strsplit(dsp_const.prog_name, ".", /EXTRACT)
        filter = "*."+filter[n_elements(filter)-1]
        prog_file = dialog_pickfile(PATH=adsec_path.prog, FILTER=filter, $
                                    TITLE="Select a accelerometer program:", /MUST_EXIST)
    endif
    disp_mess, 'Uploading the DSP program: '+prog_file+'...',/APPEND
    err = download_program(idacc,idacc,prog_file, /NORESET, /NOSTART, /VERB, /SIGGEN, /ACCEL)
    if err ne adsec_error.ok then begin
        disp_mess, '... error uploading the DSP program in accelerometer board', /APPEND
        return, err
    endif
    disp_mess, '... accBoard done.', /APPEND
    disp_mess, 'DSP program uploaded.',/APPEND

    disp_mess, 'Executing the communication test...', /APPEND
    if accel_datasheet.data_mem_size gt 0 then begin
        ; Communication test using WR_SAME in data memory
        data_len = dsp_const.test_len
        err = communication_test(0, 0, 0, rtr_map.test_address-dsp_const.test_len, $
                                 data_len, WORD_WRONG=n_ww, ITER=20, /RETRY, /VERBOSE, /SIGGEN)
        if n_ww ne 0 then begin
            disp_mess, '...Write Same Test in data memory FAILED. ('+strtrim(n_ww,2) $
                       +' of '+strtrim(data_len*adsec.n_dsp,2)+')', /APPEND
            return, adsec_error.communication
        endif
        disp_mess, '... Write same (DM) test done...', /APPEND

        ; Communication test using WR_SEQ in data memory
        err = communication_test(1, 0, 0, rtr_map.test_address-dsp_const.test_len, $
                                 data_len, WORD_WRONG=n_ww, ITER=20, /RETRY, /VERBOSE, /SIGGEN)
        if n_ww ne 0 then begin
            disp_mess, '...Write Seq Test in data memory FAILED. ('+strtrim(n_ww,2) $
                       +' of '+strtrim(data_len*adsec.n_dsp,2)+')', /APPEND
            return, adsec_error.communication
        endif
        disp_mess, '... Write seq (DM) test done...', /APPEND
    endif

    if accel_datasheet.prog_mem_size gt 0 then begin
        ; Communication test using WR_SAME in program memory
        data_len = dsp_const.test_len
        err = communication_test(0, 0, 0, dsp_map_pm.test_address-dsp_const.test_len, $
                                 data_len, WORD_WRONG=n_ww, ITER=20, /SET_PM, /RETRY, /VERBOSE, /SIGGEN)
        if n_ww ne 0 then begin
            disp_mess, '...Write Same Test in program memory FAILED. ('+strtrim(n_ww,2) $
                       +' of '+strtrim(data_len*adsec.n_dsp,2)+')', /APPEND
            return, adsec_error.communication
        endif
        disp_mess, '... Write same (PM) test done...', /APPEND

        ; Communication test using WR_SEQ in program memory
        err = communication_test(1, 0, 0, dsp_map_pm.test_address-dsp_const.test_len, $
                                 data_len, WORD_WRONG=n_ww, ITER=20, /SET_PM, /RETRY, /VERBOSE, /SIGGEN)
        if n_ww ne 0 then begin
            disp_mess, '...Write Seq Test in program memory FAILED. ('+strtrim(n_ww,2) $
                       +' of '+strtrim(data_len*adsec.n_dsp,2)+')', /APPEND
            return, adsec_error.communication
        endif
        disp_mess, '... Write seq (PM) test done...', /APPEND
    endif
    disp_mess, '... communication test done.', /APPEND

; ------------ CONFIGURAZIONE SRAM DA RIMUOVERE DA QUI              -----------------------------
    
;filt = [5.607474753155e-005, 1.682242425947e-004, 1.682242425947e-004, 5.607474753155e-005, 1.000000000000e+000 $
;        ,-2.842956089969e+000,   2.698002335056e+000, -8.545976471062e-001, 0.0, 5.581914158198e-013, 1.116382831640e-012 $
;        ,5.581914158198e-013, 0.0, 1.000000000000e+000, -1.996001300368e+000, 9.960092792312e-001]

        ;PATCH because the read_ascii_structure has a bug
        cd, current=cc
        ccdir = filepath(ROOT=adsec_path.conf,SUB='general','')
        cd, ccdir
        def=read_ascii_structure(filepath(ROOT=adsec_path.conf,SUB='general', 'accel_filter_coeff.txt'))
        cd, cc
         ;modified RB to implement Agapito's coefficients
;        def.proj[*,*]*= !pi/6.
;        def.proj[*,0]=cos(def.proj[*,0])
;        def.proj[*,1]=sin(def.proj[*,1])
;        def.proj[*,2]=cos(def.proj[*,2])


    if not keyword_set(nofilt) then begin
        err = set_accelerometer(filter_vec=def.filter_coeff,/CONFIGURE, /START_STORAGE, /START_ACQ, proj_mat=def.proj, ACCGAIN=def.gain)
        print, 'Accelerometers started, filter applied'
    endif else begin
        err = set_accelerometer(/CONFIGURE, /START_STORAGE, proj_mat=def.proj, /START_ACQ, ACCGAIN=gain)
        print, 'Accelerometer started, unit filter and proj matrix applied'
    endelse
   

;    err = write_same_board(1,'181a4'xl ,'80010'xl , /CHECK, SET_PM=dsp_datasheet.sram_mem_id, /SIGGEN)
;    err = write_same_board(1,'181a5'xl ,'16' , /CHECK, SET_PM=dsp_datasheet.sram_mem_id, /SIGGEN)


    
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
