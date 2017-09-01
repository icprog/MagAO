;$Id: set_diagnostic.pro,v 1.25 2009/07/03 13:25:32 labot Exp $$
;+
;   NAME:
;    SET_DIAGNOSTIC
;
;   PURPOSE:
;   Initialized the all RECONSTRUCOR address pointers to the respective address.
;
;   USAGE:
;    err = set_diagnostic([CONFIGURE, MASTER, OVERSAMPLING_TIME, AUTODIAGNOSTIC_TIME, 
;                          DECIMATION_MASTER, DECIMATION_SAVE, SWITCHB, BCU])
;
;   INPUT:
;    None.
;
;   OUTPUT:
;    Error code.
;
;   KEYWORD:
;    CONFIGURE: set up all parameters loaded in the rtr_map structure. If no other keyword is added, the default value will
;               be updated into SWITCH BCU and CRATE BCU 
;               (Master diagnostic OFF, Oversampling OFF, Decimation Master NONE, Decimation SDRAM NONE)
;               If some keyword is passed, the configuration will be the Default as said before, with exceptions
;               for the fields changed in the keyword. All the other keywords can be call singularly.
;               If nor SWITCHB and BCU are specified, the modifications will be applied for both SWITCH BCU and CRATE BCU.
;               The counters in default are reset.
;    SWITCHB:   apply modification to SWITCH BCU.
;    BCU:       apply modification to SWITCH BCU.
;    DECIMATION_MASTER: decimation of diagnostic  frames in BCU Master Mode.
;    DECIMATION_SAVE: decimation of diagnostic frames in SDRAM.
;    OVERSAMPLING_TIME: oversampling time [s]
;    MASTER: enable master bcu bit in SRAM
;    RESET_COUNTERS: reset the swb_SafeSkipFrameCnt, swb_PendinfSkipFrameCnt, swb_MirrFramesCounter and swb_WFSGlobalTimeout
;                    -1 reset all
;                    2 reset swb_SafeSkipFrameCnt
;                    4 reset swb_PendingSkipFrameCnt
;                    8 reset swb_MirrFramesCounter
;                    16 reset swb_WFSGlobalTimeout
;    
;   NOTE:
;    None.
; HISTORY
;   Created on 07 Feb 2007, Marco Xompero
;       marco@arcetri.astro.it
;   20 Feb 2007
;       General fixing
;   26 Feb 2007
;       Fixed error on NumDSPBoard variable in BCU memory map.
;       Fixed error setting up decimation in master bcu mode.
;   06 Mar 2007
;       Now MASTER and OVERSAMPLING keywords are used with value 0 to disaable the MASTER BCU mode and OVERSAMPLING sampling
;   09 Mar 2007
;       Test on accumulator settings added before set up the oversampling time
;   20 Mar 2007, MX and DZ
;       Fastlink data collecting from DSP boards updated for multicrate operations.
;   3 April 2007, MX and DZ
;       Updating fastlink map address and cleaning of   InvOvsFrameRate, TimeoutFrameRate and ParamSelector 
;       before the writing in Switch bcu memory.
;   04 Nov 2007, MX
;       Fixed sequence on oversampling_time write.
;   01 Feb 2008, MX
;       Added the timeout_fast.
;   14 Feb 2008,MX
;       Corrected the bug of the name C.ADSEC.OVS_P
;   01 Oct 2008, MX
;       Changed call for OVS_P variable
;   09 Apr 2009, MX
;       Added update to RTDB variables.
;
;-

Function set_diagnostic, MASTER=master, CONFIGURE=configure, OVERSAMPLING_TIME=oversampling_time $
                       , DECIMATION_MASTER=decimation_master, DECIMATION_SAVE=dacimation_save    $
                       , SWITCHB=switchb, BCU=bcu, RESET_COUNTERS=reset_counters $
                       , TIMEOUT_FAST=timeout_fast, FREEZE_TIMEOUT=freeze_timeout
    @adsec_common

    if (n_elements(oversampling_time) gt 0) then $
      if (oversampling_time eq 0) and (n_elements(timeout_fast) eq 0) then timeout_fast = 0
    if n_elements(timeout_fast) eq 0 then timeout_fast = 1000.
    first_dsp = 0
    last_dsp = adsec.n_actuators / adsec.n_act_per_dsp -1

    if keyword_set(CONFIGURE) then begin

        if ~keyword_set(reset_counters) then reset_counters=-1
;----------------------------------------------------------------------------------------------------
        if keyword_set(DECIMATION_MASTER) then begin
            ;;;;sistema il valore del decimation al valore passato
            rtr.bcu_decimation_master = decimation_master
            rtr.switch_decimation_master = decimetion_master
            decimation_master = 0
        endif else begin
            rtr.bcu_decimation_master = 0
            rtr.switch_decimation_master = 0
        endelse

        if keyword_set(DECIMATION_SAVE) then begin
            rtr.bcu_decimation_save = decimation_save
            rtr.switch_decimation_save = decimation_save
            decimation_save = 0
        endif else begin
            rtr.bcu_decimation_save = 0
            rtr.switch_decimation_save = 0
        endelse

        if keyword_set(MASTER) then begin
            rtr.enable_master_diag = 1
            master = 0
        endif else rtr.enable_master_diag = 0

        if keyword_set(OVERSAMPLING_TIME) then begin
            err = get_acc(sc.all, samples, delay, /SINGLE)
            if err ne adsec_error.ok then return, err
            if (samples+delay)*1.1*adsec.sampling_time gt oversampling_time then begin
                message, "The accumulator settings don't allow the requested oversampling_time.", CONT = (sc.debug eq 0)
                return, adsec_error.input_type
            endif
            rtr.oversampling_time = (oversampling_time) < ('7ffffffe'xl * 4.1176e-9)
            oversampling_time = 0
        endif else rtr.oversampling_time = 0

;----------------------------------------------------------------------------------------------------

        ;DSP CONFIGURATION
        ; setting diagnostic_ptr pointer to the diagnostic address
        ; DSP memory
        addr_diag = rtr_map.diagnostic_buffer
        err = write_same_dsp(sc.all_dsp, dsp_map.diagnostic_ptr, addr_diag, /CHECK)
        if err ne adsec_error.ok then begin
            print, 'Error writing address of diagnostic area.'
            return, err
        endif

        ; SRAM memory
        err = write_same_board(sc.all, sram_map.diagnostic_record_len, $
                        long(rtr.diag_len), set_pm = dsp_datasheet.sram_mem_id)
        if err ne adsec_error.ok then return, err

        err = write_same_board(sc.all, sram_map.diagnostic_record_ptr, rtr_map.diagnostic_buffer $
                       , SET_PM=dsp_datasheet.sram_mem_id, /CHECK)
        if err ne adsec_error.ok then return, err

        err = write_same_board(sc.all, sram_map.rd_diagnostic_record_ptr, 0L $
                       , SET_PM=dsp_datasheet.sram_mem_id, /CHECK)
        if err ne adsec_error.ok then return, err

        err = write_same_board(sc.all, sram_map.wr_diagnostic_record_ptr, 0L $
                       , SET_PM=dsp_datasheet.sram_mem_id, /CHECK)
        if err ne adsec_error.ok then return, err


;----------------------------------------------------------------------------------------------------

        ;CRATE BCU CONFIGURATION
        ; DSP memory
        err = write_same_dsp(sc.all, bcu_map.ReplyVectorPtr, bcu_map.ReplyVector, /CHECK, /BCU)
        if err ne adsec_error.ok then return, err

        err = write_same_dsp(sc.all, bcu_map.DiagVectorPtr, bcu_map.DiagVector, /CHECK, /BCU)
        if err ne adsec_error.ok then return, err

        err = write_same_dsp(sc.all, bcu_map.NumDSPBoards, ulong(adsec.n_board_per_bcu)*adsec.n_dsp_per_board, /CHECK, /BCU)
        if err ne adsec_error.ok then return, err

        err = write_same_dsp(sc.all, bcu_map.FullDiagRecordSize, ulong(rtr.diag_len), /CHECK, /BCU)
        if err ne adsec_error.ok then return, err

        err = write_same_dsp(sc.all, bcu_map.SavedDiagRecordSize, (ulong(rtr.diag_len)-12)/4, /CHECK, /BCU)
        if err ne adsec_error.ok then return, err

        err = write_same_dsp(sc.all, bcu_map.DiagnosticFrameDec, ulong(rtr.bcu_decimation_save), /CHECK, /BCU)
        if err ne adsec_error.ok then return, err
        
        ;Fastlink
        num_dsps_crt = adsec.n_dsp_per_board*adsec.n_board_per_bcu 
        dummy = ulonarr(8)
        dummy[0] = 2 * 65536ul+8704
        dummy[1] = 0ul*16777216+(num_dsps_crt-1)*4096ul+first_dsp
        dummy[2] = rtr_map.start_dl
;        dummy[3] = 2*(last_dsp-first_dsp+1)*65536ul + $
;                   floor(rtr_map.bcu_ReplyVector/524288.)*32768ul+(rtr_map.bcu_ReplyVector-524288)/2

        dummy[3] = 2*(num_dsps_crt)*65536ul + $
                   floor(rtr_map.bcu_ReplyVector/524288.)*32768ul+(rtr_map.bcu_ReplyVector-524288)/2
        
        dummy[4] = rtr.diag_len * 65536ul+8704
        dummy[5] = 1ul*16777216+(num_dsps_crt-1)*4096ul+first_dsp
        dummy[6] = rtr_map.diagnostic_buffer
        dummy[7] = rtr.diag_len*(num_dsps_crt)*65536ul + $
                   floor(rtr_map.bcu_ReplyVector/524288.)*32768ul+(rtr_map.bcu_ReplyVector-524288)/2

        dsp2crate_collect_conf=dummy
        err = write_same_dsp(sc.all, bcu_map.Bus32LinkCmd, dummy, /CHECK, /BCU)
        if err ne adsec_error.ok then return, err

        ; SRAM memory
        err = write_same_board(sc.all, bcu_sram_map.diagnostic_record_ptr, $
                        rtr_map.bcu_DiagVector, set_pm = bcu_datasheet.sram_mem_id, /CHECK, /BCU)
        if err ne adsec_error.ok then return, err

        err = write_same_board(sc.all, bcu_sram_map.diagnostic_record_len, $
                        rtr.bcu_diag_len, set_pm = bcu_datasheet.sram_mem_id, /CHECK, /BCU)
        if err ne adsec_error.ok then return, err

        err = read_seq_board(0, bcu_sram_map.enable_master_diag, 1l, dummy, set_pm = bcu_datasheet.sram_mem_id,/BCU, /UL)
        if err ne adsec_error.ok then return, err

        dummy = uint(dummy,0,2)
        dummy[0] = uint(rtr.enable_master_diag)
        dummy = ulong(dummy,0)

        err = write_same_board(sc.all, bcu_sram_map.enable_master_diag, $
                        dummy, set_pm = bcu_datasheet.sram_mem_id, /CHECK, /BCU)
        if err ne adsec_error.ok then return, err

        err = read_seq_board(0, bcu_sram_map.decimation_factor, 1l, dummy, set_pm = bcu_datasheet.sram_mem_id,/BCU, /UL)
        if err ne adsec_error.ok then return, err

        dummy = uint(dummy,0,2)
        dummy[1] = uint(rtr.bcu_decimation_master)
        dummy = ulong(dummy,0)

        err = write_same_board(sc.all, bcu_sram_map.decimation_factor, $
                        dummy, set_pm = bcu_datasheet.sram_mem_id, /CHECK, /BCU)
        if err ne adsec_error.ok then return, err

        ;Annoying stuff for configuration
        for i = 0, adsec.n_bcu-1 do begin
            combuf = compress_rd(rtr.bcu_remote_mac_address[i], rtr.bcu_remote_ip_address[i], rtr.bcu_remote_udp_port[i])
            err = write_same_board(i, bcu_sram_map.remote_mac_address, combuf, set_pm = bcu_datasheet.sram_mem_id, /CHECK, /BCU)
            if err ne adsec_error.ok then return, err
        endfor

        err = write_same_board(sc.all, bcu_sram_map.rd_diagnostic_record_ptr, 0L $
                               , set_pm = bcu_datasheet.sram_mem_id, /CHECK, /BCU)
        if err ne adsec_error.ok then return, err

        err = write_same_board(sc.all, bcu_sram_map.wr_diagnostic_record_ptr, 0L $
                       , SET_PM=bcu_datasheet.sram_mem_id, /BCU, /CHECK)
        if err ne adsec_error.ok then return, err


     
;----------------------------------------------------------------------------------------------------

        ;SWITCH BCU CONFIGURATION
        ; DSP memory

        ;Nulling before write
        err = write_same_dsp(0, switch_map.InvOvsFrameRate, 0., /CHECK, /SWITCH)
        if err ne adsec_error.ok then return, err
        err = write_same_dsp(0, switch_map.TimeoutFrameRate, 0., /CHECK, /SWITCH)
        if err ne adsec_error.ok then return, err

        err =  write_same_dsp(sc.all, switch_map.paramselector, 'C0'xl, /CHECK, /SWITCH)
        if err ne adsec_error.ok then return, err

        ;tmt = ulong(floor(rtr.oversampling_time/4.1176e-9, /L64))
        tmt =  ulong(106.25/7*16*1e6*rtr.oversampling_time) 
        if rtr.oversampling_time eq 0 then dmt = 0. else dmt = 1./tmt
        err = write_same_dsp(0, switch_map.InvOvsFrameRate,dmt, /CHECK, /SWITCH)
        if err ne adsec_error.ok then return, err
        err = write_same_dsp(0, switch_map.TimeoutFrameRate, tmt, /CHECK, /SWITCH)
        if err ne adsec_error.ok then return, err
        err = write_same_dsp(sc.all, switch_map.DiagnosticFrameDec,rtr.switch_decimation_save , /CHECK, /SWITCH)
        if err ne adsec_error.ok then return, err

        ;SRAM Memory
        err = write_same_board(0, switch_sram_map.diagnostic_record_ptr, $
                            rtr_map.switch_Header, set_pm = switch_datasheet.sram_mem_id, /CHECK, /SWITCH)
        if err ne adsec_error.ok then return, err

        err = write_same_board(0, switch_sram_map.diagnostic_record_len, $
                        rtr.switch_diag_len, set_pm = switch_datasheet.sram_mem_id, /CHECK, /SWITCH)
        if err ne adsec_error.ok then return, err

        err = read_seq_board(0, switch_sram_map.enable_master_diag, 1l, dummy, set_pm = bcu_datasheet.sram_mem_id,/SWITCH, /UL)
        if err ne adsec_error.ok then return, err

        dummy = uint(dummy,0,2)
        dummy[0] = uint(rtr.enable_master_diag)
        dummy = ulong(dummy,0)

        err = write_same_board(0, switch_sram_map.enable_master_diag, $
                        dummy, set_pm = switch_datasheet.sram_mem_id, /CHECK, /SWITCH)
        if err ne adsec_error.ok then return, err

        err = read_seq_board(0, switch_sram_map.decimation_factor, 1l, dummy, set_pm = bcu_datasheet.sram_mem_id,/SWITCH, /UL)
        if err ne adsec_error.ok then return, err

        dummy = uint(dummy,0,2)
        dummy[1] = uint(rtr.switch_decimation_master)
        dummy = ulong(dummy,0)

        err = write_same_board(0, switch_sram_map.decimation_factor, $
                        dummy, set_pm = switch_datasheet.sram_mem_id, /CHECK, /SWITCH)
        if err ne adsec_error.ok then return, err

        combuf = compress_rd(rtr.switch_remote_mac_address, rtr.switch_remote_ip_address, rtr.switch_remote_udp_port)
        err = write_same_board(0, bcu_sram_map.remote_mac_address, set_pm = switch_datasheet.sram_mem_id, combuf, /CHECK, /SWITCH)
        if err ne adsec_error.ok then return, err

        err = write_same_board(0, switch_sram_map.rd_diagnostic_record_ptr, 0L $
                            , set_pm = switch_datasheet.sram_mem_id, /CHECK, /SWITCH)
        if err ne adsec_error.ok then return, err

        err = write_same_board(0, switch_sram_map.wr_diagnostic_record_ptr, 0L $
                       , SET_PM=switch_datasheet.sram_mem_id, /SWITCH, /CHECK)
        if err ne adsec_error.ok then return, err



    endif

    if n_elements(OVERSAMPLING_TIME) gt 0 then begin
        if oversampling_time gt 0 then begin
            err = get_acc(sc.all, samples, delay, /SINGLE)
            if err ne adsec_error.ok then return, err
            if (samples+delay)*1.00*adsec.sampling_time gt oversampling_time then begin
                message, "The accumulator settings don't allow the requested oversampling_time.", CONT = (sc.debug eq 0)
                return, adsec_error.input_type
            endif
        endif

        rtr.oversampling_time = (oversampling_time) < ('7ffffffe'xl * 4.1176e-9)
        ;tmt = ulong(floor(rtr.oversampling_time/4.1176e-9, /L64))
        tmt = ulong(106.25d/7*16*1e6*rtr.oversampling_time) 

        ;Nulling before write
        err = write_same_dsp(0, switch_map.TimeoutFrameRate, 0., /CHECK, /SWITCH)
        if err ne adsec_error.ok then return, err
        err = write_same_dsp(0, switch_map.InvOvsFrameRate, 0., /CHECK, /SWITCH)
        if err ne adsec_error.ok then return, err


        if rtr.oversampling_time eq 0 then dmt = 0. else dmt = 1./tmt
        err = write_same_dsp(0, switch_map.InvOvsFrameRate,dmt, /CHECK, /SWITCH)
        if err ne adsec_error.ok then return, err
        err = write_same_dsp(0, switch_map.TimeoutFrameRate, tmt, /CHECK, /SWITCH)
        if err ne adsec_error.ok then return, err
        
        if sc.name_comm ne "Dummy" then begin
;            if ~keyword_set(FREEZE_TIMEOUT) then begin
                val = long(timeout_fast)
                ;err = write_scalar_var_wrap(strupcase(strmid(!AO_CONST.telescope_side,0,1))+'.ADSEC.OVS_P', long(long(dmt gt 0)*timeout_fast), 1001, 1000L)
                err = write_scalar_var_wrap(strupcase(strmid(!AO_CONST.telescope_side,0,1))+'.ADSEC.OVS_P', val, 1001, 1000L)
                if err ne adsec_error.ok then return, err
;            endif 
        endif


    endif

    if n_elements(DECIMATION_SAVE) gt 0 then begin
        
        id = keyword_set(BCU) + 2*keyword_set(SWITCHB)
        case id of
            1:begin
            ;BCU
                rtr.bcu_decimation_save = decimation_save
                err = write_same_dsp(sc.all, bcu_map.NumDSPBoards+3, rtr.bcu_decimation_save, /CHECK, /BCU)
                if err ne adsec_error.ok then return, err
            end
            2:begin
            ;SWITCH
                rtr.switch_decimation_save = decimation_save
                err = write_same_dsp(sc.all, switch_map.DiagnosticFrameDec,rtr.switch_decimation_save , /CHECK, /SWITCH)
                if err ne adsec_error.ok then return, err
            end
            else:begin
                ;tutti
                rtr.bcu_decimation_save = decimation_save
                rtr.switch_decimation_save = decimation_save
                err = write_same_dsp(sc.all, bcu_map.NumDSPBoards+3, rtr.bcu_decimation_save, /CHECK, /BCU)
                if err ne adsec_error.ok then return, err
                err = write_same_dsp(sc.all, switch_map.DiagnosticFrameDec,rtr.switch_decimation_save , /CHECK, /SWITCH)
                if err ne adsec_error.ok then return, err
            end
        endcase
    endif 

    if n_elements(MASTER) gt 0  then begin
        rtr.enable_master_diag = master gt 0
        id = keyword_set(BCU) + 2*keyword_set(SWITCHB)
        case id of
            1:begin
            ;BCU
                err = read_seq_board(0, bcu_sram_map.enable_master_diag, 1l, dummy, set_pm = bcu_datasheet.sram_mem_id,/BCU, /UL)
                if err ne adsec_error.ok then return, err
                dummy = uint(dummy,0,2)
                dummy[0] = uint(rtr.enable_master_diag)
                dummy = ulong(dummy,0)
                err = write_same_board(sc.all, bcu_sram_map.enable_master_diag, $
                                dummy, set_pm = bcu_datasheet.sram_mem_id, /CHECK, /BCU)
                if err ne adsec_error.ok then return, err
            end
            2:begin
            ;SWITCH
                err = read_seq_board(0, switch_sram_map.enable_master_diag, 1l, dummy, set_pm = bcu_datasheet.sram_mem_id,/SWITCH, /UL)
                if err ne adsec_error.ok then return, err
                dummy = uint(dummy,0,2)
                dummy[0] = uint(rtr.enable_master_diag)
                dummy = ulong(dummy,0)
                err = write_same_board(0, switch_sram_map.enable_master_diag, $
                                dummy, set_pm = bcu_datasheet.sram_mem_id, /CHECK, /SWITCH)
                if err ne adsec_error.ok then return, err
            end
            else:begin
                ;tutti
                err = read_seq_board(0, bcu_sram_map.enable_master_diag, 1l, dummy, set_pm = bcu_datasheet.sram_mem_id,/BCU,/UL)
                if err ne adsec_error.ok then return, err
                dummy = uint(dummy,0,2)
                dummy[0] = uint(rtr.enable_master_diag)
                dummy = ulong(dummy,0)
                err = write_same_board(sc.all, bcu_sram_map.enable_master_diag, $
                                dummy, set_pm = bcu_datasheet.sram_mem_id, /CHECK, /BCU)
                if err ne adsec_error.ok then return, err

                err = read_seq_board(0, switch_sram_map.enable_master_diag, 1l, dummy, set_pm = bcu_datasheet.sram_mem_id,/SWITCH, /UL)
                if err ne adsec_error.ok then return, err
                dummy = uint(dummy,0,2)
                dummy[0] = uint(rtr.enable_master_diag)
                dummy = ulong(dummy,0)
                err = write_same_board(0, switch_sram_map.enable_master_diag, $
                                dummy, set_pm = bcu_datasheet.sram_mem_id, /CHECK, /SWITCH)
                if err ne adsec_error.ok then return, err
            end
        endcase
            
    endif

    if n_elements(DECIMATION_MASTER) gt 0 then begin
        id = keyword_set(BCU) + 2*keyword_set(SWITCHB)
        case id of
            1:begin
            ;BCU
                rtr.bcu_decimation_master = decimation_master
                err = read_seq_board(0, bcu_sram_map.decimation_factor, 1l, dummy, set_pm = bcu_datasheet.sram_mem_id,/BCU, /UL)
                if err ne adsec_error.ok then return, err
                dummy = uint(dummy,0,2)
                dummy[0] = uint(rtr.bcu_decimation_master)
                dummy = ulong(dummy,0)
                err = write_same_board(sc.all, bcu_sram_map.decimation_factor, $
                                        dummy, set_pm = bcu_datasheet.sram_mem_id, /CHECK, /BCU)
                if err ne adsec_error.ok then return, err
            end
            2:begin
            ;SWITCH
                rtr.switch_decimation_master = decimation_master
                err = read_seq_board(0, switch_sram_map.decimation_factor, 1l, dummy, set_pm = bcu_datasheet.sram_mem_id,/SWITCH, /UL)
                if err ne adsec_error.ok then return, err
                dummy = uint(dummy,0,2)
                dummy[1] = uint(rtr.switch_decimation_master)
                dummy = ulong(dummy,0)
                err = write_same_board(sc.all, switch_sram_map.decimation_factor, $
                                dummy, set_pm = switch_datasheet.sram_mem_id, /CHECK, /SWITCH)
                if err ne adsec_error.ok then return, err
                
            end
            else:begin
                ;tutti
                rtr.bcu_decimation_master = decimation_master
                rtr.switch_decimation_master = decimation_master
                
                err = read_seq_board(0, bcu_sram_map.decimation_factor, 1l, dummy, set_pm = bcu_datasheet.sram_mem_id,/BCU, /UL)
                if err ne adsec_error.ok then return, err
                dummy = uint(dummy,0,2)
                dummy[1] = uint(rtr.bcu_decimation_master)
                dummy = ulong(dummy,0)
                err = write_same_board(sc.all, bcu_sram_map.decimation_factor, $
                                        dummy, set_pm = bcu_datasheet.sram_mem_id, /CHECK, /BCU)
                if err ne adsec_error.ok then return, err

                err = read_seq_board(0, switch_sram_map.decimation_factor, 1l, dummy, set_pm = bcu_datasheet.sram_mem_id,/SWITCH, /UL)
                if err ne adsec_error.ok then return, err
                dummy = uint(dummy,0,2)
                dummy[1] = uint(rtr.switch_decimation_master)
                dummy = ulong(dummy,0)
                err = write_same_board(sc.all, switch_sram_map.decimation_factor, $
                                        dummy, set_pm = switch_datasheet.sram_mem_id, /CHECK, /SWITCH)
                if err ne adsec_error.ok then return, err
            end
        endcase
            
    endif

    if keyword_set(RESET_COUNTERS) then begin
        if reset_counters eq -1 then id = 32+16+8+4+2 else id =reset_counters

        if (id and 32) eq 32 then begin
            err = write_same_board(sc.all, sram_map.rd_diagnostic_record_ptr,0L , /CHECK, SET_PM=dsp_datasheet.sram_mem_id)
            if err ne adsec_error.ok then return, err
            err = write_same_board(sc.all, sram_map.wr_diagnostic_record_ptr,0L , /CHECK, SET_PM=dsp_datasheet.sram_mem_id)
            if err ne adsec_error.ok then return, err
        endif

        if (id and 16) eq 16 then begin
            err = write_same_dsp(0, switch_map.WFSGlobalTimeout,0L , /CHECK, /SWITCH)
            if err ne adsec_error.ok then return, err
        endif
            
        if (id and 8) eq 8 then begin 
            err = write_same_dsp(0, switch_map.MirrFramesCounter,0L , /CHECK, /SWITCH)
            if err ne adsec_error.ok then return, err
        endif

        if (id and 4) eq 4 then begin
            err = write_same_dsp(0, switch_map.PendingSkipFrameCnt,0L , /CHECK, /SWITCH)
            if err ne adsec_error.ok then return, err
        endif

        if (id and 2) eq 2 then begin
            err = write_same_dsp(0, switch_map.SafeSkipFrameCnt,0L , /CHECK, /SWITCH)
            if err ne adsec_error.ok then return, err
        endif           

    endif

    return, update_rtdb()
;    return, adsec_error.ok


end
