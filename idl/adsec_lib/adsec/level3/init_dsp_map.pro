; $Id: init_dsp_map.pro,v 1.14 2009/09/18 08:25:01 marco Exp $

;+
; init_hw_map
;
; load structures with data related to HARDWARE(DSP, CRATE BCU, SWITCH BCU) operations
;
; HISTORY
;   Jan 2007: ReWritten by M.Xompero (MX)
;   marco@arcetri.astro.it
;   18 Apr 2007, MX
;    Added/changed parameters for programs path.
;-
Function init_dsp_map

    @adsec_common

    ; read dsp_datasheet structure
    ;
    filename = filepath(ROOT=adsec_path.commons, SUB=['datasheet'], "dsp_datasheet.txt")
    dsp_datasheet = read_ascii_structure(filename)
    if test_type(dsp_datasheet, /STRUCT) then $
        message, "Fix the file format of dsp_datasheet.txtand restart!"

    filename = filepath(ROOT=adsec_path.commons, SUB=['datasheet'], "siggen_datasheet.txt")
    siggen_datasheet = read_ascii_structure(filename)
    if test_type(siggen_datasheet, /STRUCT) then $
        message, "Fix the file format of siggen_datasheet.txt and restart!"

    filename = filepath(ROOT=adsec_path.commons, SUB=['datasheet'], "accel_datasheet.txt")
    accel_datasheet = read_ascii_structure(filename)
    if test_type(siggen_datasheet, /STRUCT) then $
        message, "Fix the file format of siggen_datasheet.txt and restart!"

    ;; max_read_buffer_len:  max len of comm. buffer when reading [bytes].
    ;;   It must be larger then or equal to 4*n_dsp. Zero means no limitations.
    ;; max_write_buffer_len: max len of comm. buffer when writing [bytes].
    ;;   It must be larger then or equal to 4*n_dsp+header_len. Zero means no limitations.
    ;; No limitations in  this case
    max_read_buffer_len = 0L
    max_write_buffer_len = 0L

    filename = filepath(ROOT=sc.prog_ver, "dsp_const.txt")
    dsp_const = read_ascii_structure(filename)
    if test_type(dsp_const, /STRUCT) then begin
        message, "Fix file format of dsp_const.txt and restart!", CONT=(sc.debug ne 1)
        return, adsec_error.invalid_file
    endif

    ; read DSP address map (data_memory) and store it in dsp_map structure
    filename = filepath(ROOT=sc.switch_prog_ver, "switch_dm_map.txt")
    switch_map = read_ascii_structure(filename)
    if test_type(switch_map, /STRUCT) then $
        message, "Fix file format and restart!"

    ; read DSP address map (data_memory) and store it in dsp_map structure
    filename = filepath(ROOT=sc.bcu_prog_ver, "bcu_dm_map.txt")
    bcu_map = read_ascii_structure(filename)
    bcu_map = create_struct(bcu_map, {DiagVector: '100000'xl, ReplyVector: '80000'xl })
    if test_type(bcu_map, /STRUCT) then $
        message, "Fix file format and restart!"

   ; read ACCELEROMETER address map (data_memory) and store it in acc_map structure
    filename = filepath(ROOT=sc.accel_prog_ver, "acc_dm_map.txt")
    acc_map=read_ascii_structure(filename)
    if test_type(acc_map, /STRUCT) then $
        message, "Fix file format and restart!"

    if accel_datasheet.sram_mem_size ne 0 then begin
        ; read SRAM address map and store it in sdram_map structure
        filename = filepath(ROOT=sc.accel_prog_ver, "acc_sram_map.txt")
        acc_sram_map = read_ascii_structure(filename)
        if test_type(acc_sram_map, /STRUCT) then $
            message, "Fix file format and restart!"
    endif


    ; read DSP address map (data_memory) and store it in dsp_map structure
    filename = filepath(ROOT=sc.prog_ver, "dsp_dm_map.txt")
    dsp_map = read_ascii_structure(filename)
    if test_type(dsp_map, /STRUCT) then $
        message, "Fix file format and restart!"

    ; Check if Program Memory exists
    if dsp_datasheet.prog_mem_size ne 0 then begin
        ; read DSP address map (program memory) and store it in dsp_map_pm structure
        filename = filepath(ROOT=sc.prog_ver, "dsp_pm_map.txt")
        dsp_map_pm = read_ascii_structure(filename)
        if test_type(dsp_map_pm, /STRUCT) then $
            message, "Fix file format and restart!"
    endif

    ; Check if SDRAM Memory exists
    if dsp_datasheet.sdram_mem_size ne 0 then begin
        ; read SDRAM address map and store it in sdram_map structure
        filename = filepath(ROOT=sc.prog_ver, "sdram_map.txt")
        sdram_map = read_ascii_structure(filename)
        if test_type(sdram_map, /STRUCT) then $
            message, "Fix file format and restart!"
    endif

    ;; Check if SRAM Memory exists
    if dsp_datasheet.sram_mem_size ne 0 then begin
        ; read SRAM address map and store it in sdram_map structure
        filename = filepath(ROOT=sc.prog_ver, "sram_map.txt")
        sram_map = read_ascii_structure(filename)
        if test_type(sram_map, /STRUCT) then $
            message, "Fix file format and restart!"
    endif

    ; Check if FPGAbuf Memory exists
    if dsp_datasheet.fpgabuf_mem_size ne 0 then begin
        ; read FPGAbuf address map and store it in fpgabuf_map structure
        filename = filepath(ROOT=sc.prog_ver, "fpgabuf_map.txt")
        fpgabuf_map = read_ascii_structure(filename)
        if test_type(fpgabuf_map, /STRUCT) then $
            message, "Fix file format and restart!"
    endif

    ; Check if FPGAsiggen Memory exists
    if dsp_datasheet.fpgasig_mem_size ne 0 then begin
        ; read FPGAsig address map and store it in fpgasig_map structure
        filename = filepath(ROOT=sc.prog_ver, "fpgasig_map.txt")
        fpgasig_map = read_ascii_structure(filename)
        if test_type(fpgasig_map, /STRUCT) then $
            message, "Fix file format and restart!"
    endif

;-----------------------------------------------------------------------------------------

    ;CRATE BCU CONFIGURATION
    filename = filepath(ROOT=adsec_path.commons, SUB=['datasheet'], "bcu_datasheet.txt")
    bcu_datasheet = read_ascii_structure(filename)
    if test_type(bcu_datasheet, /STRUCT) then $
        message, "Fix the file format of bcu_datasheet.txt and restart!"

    ; Check if SDRAM Memory exists
    if bcu_datasheet.sdram_mem_size ne 0 then begin
        ; read SDRAM address map and store it in sdram_map structure
        filename = filepath(ROOT=sc.bcu_prog_ver, "bcu_sdram_map.txt")
        bcu_sdram_map = read_ascii_structure(filename)
        if test_type(sdram_map, /STRUCT) then $
            message, "Fix file format and restart!"
    endif

    ;; Check if SRAM Memory exists
    if bcu_datasheet.sram_mem_size ne 0 then begin
        ; read SRAM address map and store it in sdram_map structure
        filename = filepath(ROOT=sc.bcu_prog_ver, "bcu_sram_map.txt")
        bcu_sram_map = read_ascii_structure(filename)
        if test_type(sram_map, /STRUCT) then $
            message, "Fix file format and restart!"
    endif

    ; Check if FPGAbuf Memory exists
    if bcu_datasheet.fpgabuf_mem_size ne 0 then begin
        ; read FPGAbuf address map and store it in fpgabuf_map structure
        filename = filepath(ROOT=sc.bcu_prog_ver, "bcu_fpgabuf_map.txt")
        bcu_fpgabuf_map = read_ascii_structure(filename)
        if test_type(fpgabuf_map, /STRUCT) then $
            message, "Fix file format and restart!"
    endif

    ; Check if FPGAsiggen Memory exists
    if bcu_datasheet.fpgasig_mem_size ne 0 then begin
        ; read FPGAsig address map and store it in fpgasig_map structure
        filename = filepath(ROOT=sc.bcu_prog_ver, "bcu_fpgasig_map.txt")
        bcu_fpgasig_map = read_ascii_structure(filename)
        if test_type(fpgasig_map, /STRUCT) then $
            message, "Fix file format and restart!"
    endif

;-----------------------------------------------------------------------------------------

    ;SWITCH BCU CONFIGURATION
    filename = filepath(ROOT=adsec_path.commons, SUB=['datasheet'], "switch_datasheet.txt")
    switch_datasheet = read_ascii_structure(filename)
    if test_type(switch_datasheet, /STRUCT) then $
        message, "Fix the file format of switch_datasheet.txt and restart!"

    ; Check if SDRAM Memory exists
    if switch_datasheet.sdram_mem_size ne 0 then begin
        ; read SDRAM address map and store it in sdram_map structure
        filename = filepath(ROOT=sc.switch_prog_ver, "switch_sdram_map.txt")
        switch_sdram_map = read_ascii_structure(filename)
        if test_type(sdram_map, /STRUCT) then $
            message, "Fix file format and restart!"
    endif

    ;; Check if SRAM Memory exists
    if switch_datasheet.sram_mem_size ne 0 then begin
        ; read SRAM address map and store it in sdram_map structure
        filename = filepath(ROOT=sc.switch_prog_ver, "switch_sram_map.txt")
        switch_sram_map = read_ascii_structure(filename)
        if test_type(sram_map, /STRUCT) then $
            message, "Fix file format and restart!"
    endif

    ; Check if FPGAbuf Memory exists
    if switch_datasheet.fpgabuf_mem_size ne 0 then begin
        ; read FPGAbuf address map and store it in fpgabuf_map structure
        filename = filepath(ROOT=sc.switch_prog_ver, "switch_fpgabuf_map.txt")
        switch_fpgabuf_map = read_ascii_structure(filename)
        if test_type(fpgabuf_map, /STRUCT) then $
            message, "Fix file format and restart!"
    endif

    ; Check if FPGAsiggen Memory exists
    if switch_datasheet.fpgasig_mem_size ne 0 then begin
        ; read FPGAsig address map and store it in fpgasig_map structure
        filename = filepath(ROOT=sc.switch_prog_ver, "switch_fpgasig_map.txt")
        switch_fpgasig_map = read_ascii_structure(filename)
        if test_type(fpgasig_map, /STRUCT) then $
            message, "Fix file format and restart!"
    endif


    return, adsec_error.ok

end
