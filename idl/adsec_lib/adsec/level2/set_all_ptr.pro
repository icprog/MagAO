;+
;   NAME:
;    SET_ALL_PTR
;
;   PURPOSE:
;   Initialized the all RECONSTRUCOR address pointers to the respective address.
;
;   USAGE:
;    err = set_all_ptr()
;
;   INPUT:
;    None.
;
;   OUTPUT:
;    Error code.
;
;   KEYWORD:
;    None
;
;   NOTE:
;    None.
; HISTORY
;   Created by 4 february 2005: D. Zanotti(DZ)
;   20 May 2005 Marco Xompero (MX)
;    Dsp_map updated to rtr_map. Diagnostic pointer to the diagnostic area added.
;   03 Aug 2005, MX
;    Added new configuration for the diagnostic pointers on SRAM.
;   07 Feb 2007 MX
;       LUT of variable deleted and name refurbished.
;       Switch BCU and Crate BCU configuration added.
;   20 Feb 2007 MX
;       Fastlink configuration for CRATE 1
;       Start RTR address fixed
;   20 Mar 2007, MX and DZ
;       Fastlink configuration updated for multicrate operations.
;      ;PATCH TO BE REMOVED AFTER LOGIC FIXING on FASTLINK ADDRESSING
;
;-

Function set_all_ptr

    @adsec_common

;                                            RECONSTRUCTOR
;----------------------------------------------------------------------------------------------------

    ;DSP CONFIGURATION
    ; setting slope_vector pointer to the slope_vector address
    addr_slope_vector =   rtr_map.slope_vector
    err = write_same_dsp(sc.all_dsp, dsp_map.slope_vector_ptr, addr_slope_vector, /CHECK)

    if err ne adsec_error.ok then begin
        print, 'Error writing address of slope_vector'
        return, err
    endif

    ; setting start_rtr pointer to the start_rtr address
    addr_start_rtr = rtr_map.start_rtr
    
    err = read_seq_dsp(sc.all_dsp, addr_start_rtr, 1l, buf, /UL)
    if (err ne 0) then return, err
            
    if total(double(buf)) gt 0 and sc.name_comm ne "Dummy" then begin
        message, 'The new start_rtr mem location is not cleared. Returning without move the pointer.' $
                , CONT = (sc.debug eq 0)
        return, adsec_error.idl_garbage
    endif

    err = write_same_dsp(sc.all_dsp, dsp_map.start_rtr_ptr, addr_start_rtr -2ul, /CHECK)

    if err ne adsec_error.ok then begin
        print, 'Error writing address of start_rtr'
        return, err
    endif


    ; setting modes_vector pointer to the modes_vector address
    addr_modes_vector = rtr_map.modes_vector
    err = write_same_dsp(sc.all_dsp, dsp_map.modes_vector_ptr, addr_modes_vector, /CHECK)

    if err ne adsec_error.ok then begin
        print, 'Error writing address of modes_vector'
    return, err
    endif


    ; setting start_mm  pointer to the start_mm address
    addr_start_mm = rtr_map.start_mm

    err = read_seq_dsp(sc.all_dsp, addr_start_mm, 1l, buf, /UL)
    if (err ne 0) then return, err
        
    if total(double(buf)) gt 0 and sc.name_comm ne "Dummy" then begin
        message, 'The new start_mm mem location is not cleared. Returning without move the pointer.' $
                , CONT = (sc.debug eq 0)
        return, adsec_error.idl_garbage
    endif

    err = write_same_dsp(sc.all_dsp, dsp_map.start_mm_ptr, addr_start_mm, /CHECK)
    if err ne adsec_error.ok then begin
        print, 'Error writing address of start_mm'
        return, err
    endif


    ; setting command_vector pointer to the command_vector address
    addr_command_vector = rtr_map.command_vector    ;command_vector
    err = write_same_dsp(sc.all_dsp, dsp_map.command_vector_ptr, addr_command_vector, /CHECK)

    if err ne adsec_error.ok then begin
        print, 'Error writing address of command_vector'
        return, err
    endif


    ; setting update_command pointer to the update_command address
    addr_update_cmd = rtr_map.update_cmd

    err = read_seq_dsp(sc.all_dsp, addr_update_cmd, 1l, buf, /UL)
    if (err ne 0) then return, err
        
    if total(double(buf)) gt 0 and sc.name_comm ne "Dummy" then begin
        message, 'The new update_cmd mem location is not cleared. Returning withour move the pointer.' $
                , CONT = (sc.debug eq 0)
        return, adsec_error.idl_garbage
    endif

    err = write_same_dsp(sc.all_dsp, dsp_map.update_cmd_ptr, addr_update_cmd, /CHECK)

    if err ne adsec_error.ok then begin
        print, 'Error writing address of update_cmd'
        return, err
    endif


    ; setting ff_command_vector pointer to the ff_command_vector address
    addr_ff_command_vector = rtr_map.ff_command_vector ;ff_command_vector
    err = write_same_dsp(sc.all_dsp, dsp_map.ff_cmd_vector_ptr, addr_ff_command_vector, /CHECK)

    if err ne adsec_error.ok then begin
        print, 'Error writing address of ff_command_vector'
        return, err
    endif

    ; setting start_ff  pointer to the start_ff address
    addr_start_ff = rtr_map.start_ff

    err = read_seq_dsp(sc.all_dsp, addr_start_ff, 1l, buf, /UL)
    if (err ne 0) then return, err
    if total(double(buf)) gt 0 and sc.name_comm ne "Dummy" then begin
        message, 'The new start_ff mem location is not cleared. Returning withour move the pointer.' $
                , CONT = (sc.debug eq 0)
        return, adsec_error.idl_garbage
    endif

    err = write_same_dsp(sc.all_dsp, dsp_map.start_ff_ptr, addr_start_ff, /CHECK)

    if err ne adsec_error.ok then begin
        print, 'Error writing address of start_ff'
        return, err
    endif

    ; setting start_dl  pointer to the start_dl address
    addr_start_dl = rtr_map.start_dl

    err = read_seq_dsp(sc.all_dsp, addr_start_dl, 1l, buf, /UL)
    if (err ne 0) then return, err
    if total(double(buf)) gt 0 and sc.name_comm ne "Dummy" then begin
        message, 'The new start_dl mem location is not cleared. Returning without move the pointer.' $
                , CONT = (sc.debug eq 0)
        return, adsec_error.idl_garbage
    endif

    err = write_same_dsp(sc.all_dsp, dsp_map.start_dl_ptr, addr_start_dl, /CHECK)

    if err ne adsec_error.ok then begin
        print, 'Error writing address of start_dl'
        return, err
    endif


    ; setting update_ff pointer to the update_ff address
    addr_update_ff = rtr_map.update_ff

    err = read_seq_dsp(sc.all_dsp, addr_update_ff, 1l, buf, /UL)
    if (err ne 0) then return, err
    if total(double(buf)) gt 0 and sc.name_comm ne "Dummy" then begin
        message, 'The new update_ff mem location is not cleared. Returning withour move the pointer.' $
                , CONT = (sc.debug eq 0)
        return, adsec_error.idl_garbage
    endif

    err = write_same_dsp(sc.all_dsp, dsp_map.update_ff_ptr, addr_update_ff, /CHECK)

    if err ne adsec_error.ok then begin
        print, 'Error writing address of update_ff'
        return, err
    endif


    ; setting slope_delay pointer to the slope_delay address
    addr_slope_delay = rtr_map.slope_delay
    err = write_same_dsp(sc.all_dsp, dsp_map.slope_delay_ptr, addr_slope_delay, /CHECK)

    if err ne adsec_error.ok then begin
        print, 'Error writing address of slope_delay'
        return, err
    endif

    ; setting modes_delay pointer to the modes_delay address
    addr_modes_delay = rtr_map.modes_delay
    err = write_same_dsp(sc.all_dsp, dsp_map.modes_delay_ptr, addr_modes_delay, /CHECK)

    if err ne adsec_error.ok then begin
        print, 'Error writing address of modes_delay'
        return, err
    endif

;    ;disturbance pointers ??????
;    addr_diag = [rtr_map.disturb_start, rtr_map.disturb_start+rtr.disturb_len+1l, rtr_map.disturb_start]
;    
;    err = write_same_dsp(sc.all_dsp, dsp_map.diagnostic_ptr, addr_diag, /CHECK)
;    if err ne adsec_error.ok then begin
;        print, 'Error writing address of disturbance area.'
;        return, err
;    endif

;----------------------------------------------------------------------------------------------------
    
    ;CRATE BCU CONFIGURATION
    ;ReplyVector configuration
    err = write_same_dsp(sc.all, bcu_map.ReplyVectorPtr, rtr_map.bcu_ReplyVector, /BCU)
    if err ne adsec_error.ok then begin
        print, 'Error writing address of ReplyVector area.'
        return, err
    endif
    
;----------------------------------------------------------------------------------------------------

    ;SWITCH BCU CONFIGURATION
    
    dummy = [rtr_map.switch_SCSlopeVector, rtr_map.switch_SCStartRTR, $
             rtr_map.switch_Header, rtr_map.switch_SlopeVector,       $
             rtr_map.switch_StartRTR, rtr_map.switch_ModesVector,     $
             rtr_map.switch_StartMM, rtr_map.switch_FFCommandVector,  $
             rtr_map.switch_StartFF, rtr_map.switch_ReplyVector]
    
    err = write_same_dsp(sc.all, switch_map.SCSlopeVectorPtr, dummy, /SWITCH)
    if err ne adsec_error.ok then begin
        print, 'Error writing address of ReplyVector area.'
        return, err
    endif

    
    ;fastlink (PATCH FOR P45)
    if adsec.n_crates eq 1 then begin
        first_crt = 1 
        last_crt = 1
        mode_last_crt = 1
    endif else begin
        first_crt = 0
        last_crt = adsec.n_crates - 1 
        num_dsps_crt = adsec.n_dsp_per_board*adsec.n_board_per_bcu 
        mode_last_crt = ceil(float(rtr.n_modes2correct) / (adsec.n_dsp_per_board * adsec.n_act_per_dsp*adsec.n_board_per_bcu)) - 1
    endelse
    num_dsps_crt = adsec.n_dsp_per_board*adsec.n_board_per_bcu 
    
    first_dsp = 0
    last_dsp = ceil((adsec.n_actuators / float(adsec.n_act_per_dsp))/2)*2  - 1
    mode_last_dsp = ceil((rtr.n_modes2correct/float(adsec.n_act_per_dsp))/2)*2-1
    ;last_dsp_single_crt = (last_dsp+1)/(last_crt+1) - 1
    last_dsp_single_crt = (last_dsp-(last_crt-first_crt)*num_dsps_crt)
    mode_last_dsp_single_crt = mode_last_dsp-(mode_last_crt-first_crt)*num_dsps_crt
    ;mode_last_dsp_single_crt = (mode_last_dsp+1)/(mode_last_crt+1) - 1
    flink = ulonarr(40)
    
    ;rd_seq da tutti i DSP di command_pending (con irq) in ReplyVector
    flink[0] = 2ul*65536ul+8704
    ;flink[1] = 0ul*16777216+last_dsp*4096ul+first_dsp
    flink[1] = 0ul*16777216+last_crt*1048576ul+last_dsp_single_crt*4096ul+first_crt*256ul+first_dsp
    flink[2] = dsp_map.command_pending
    flink[3] = 2ul*(last_dsp-first_dsp+1)*65536+floor(rtr_map.switch_ReplyVector/524288.)*32768ul+(rtr_map.switch_ReplyVector-524288ul)/2


    ;wr_same a tutti i DSP di slopes + start_rtr (con irq) + frames_counters + mirrfc+param
    flink[4] = (rtr.n_slope+4)*65536ul+8192
    ;flink[5] = 1ul*16777216+last_dsp*4096ul+first_dsp
    flink[5] = 1ul*16777216+last_crt*1048576ul+last_dsp_single_crt*4096ul+first_crt*256ul+first_dsp
    flink[6] = rtr_map.slope_vector
    flink[7] = (rtr.n_slope+4)*65536ul+floor(rtr_map.switch_SlopeVector/524288.)*32768ul+(rtr_map.switch_SlopeVector-524288ul)/2


    ;rd_seq da tutti i DSP di start_rtr (con irq) in ReplyVector
    flink[8] = 2ul*65536+8704
    ;flink[9] = 2ul*16777216+mode_last_dsp*4096ul+first_dsp
    flink[9] = 2ul*16777216+mode_last_crt*1048576ul+mode_last_dsp_single_crt*4096ul+first_crt*256ul+first_dsp
    flink[10] = rtr_map.start_rtr 
    flink[11] = 2ul*(mode_last_dsp-first_dsp+1)*65536+floor(rtr_map.switch_ReplyVector/524288.)*32768ul+(rtr_map.switch_ReplyVector-524288)/2

    ;rd_seq da tutti i DSP di modes (con irq)
    flink[12] = adsec.n_act_per_dsp*65536ul+8704
    ;flink[13] = 3ul*16777216+mode_last_dsp*4096ul+first_dsp
    flink[13] = 3ul*16777216+mode_last_crt*1048576ul+mode_last_dsp_single_crt*4096ul+first_crt*256ul+first_dsp
    flink[14] = dsp_map.modes
    flink[15] = adsec.n_act_per_dsp*ulong(mode_last_dsp-first_dsp+1)*65536ul+floor(rtr_map.switch_ModesVector/524288.)*32768ul+(rtr_map.switch_ModesVector-524288)/2

    ;wr_same a tutti i DSP di modes_vector + start_mm (con irq)
    flink[16] = (rtr.n_modes2correct+4)*65536ul+8192      ;MARIO MODIFICATION SUGGESTION (non e' allineato a 128 bit e potrebbe essere meglio
    ;flink[16] = (rtr.n_modes2correct+2)*65536ul+8192
    ;flink[17] = 4ul*16777216+last_dsp*4096ul+first_dsp
    flink[17] = 4ul*16777216+last_crt*1048576ul+last_dsp_single_crt*4096ul+first_crt*256ul+first_dsp
    flink[18] = rtr_map.modes_vector
    flink[19] = (rtr.n_modes2correct+4)*65536ul+floor(rtr_map.switch_ModesVector/524288.)*32768ul+(rtr_map.switch_ModesVector-524288)/2 ;MARIO MODIFICATION SUGGESTION
    ;flink[19] = (rtr.n_modes2correct+2)*65536ul+floor(rtr_map.switch_ModesVector/524288.)*32768ul+(rtr_map.switch_ModesVector-524288)/2

    ;rd_seq da tutti i DSP di start_mm (con irq) in ReplyVector
    flink[20] = 2ul*65536+8704
    ;flink[21] = 5*16777216+last_dsp*4096ul+first_dsp
    flink[21] = 5ul*16777216+last_crt*1048576ul+last_dsp_single_crt*4096ul+first_crt*256ul+first_dsp
    flink[22] = rtr_map.start_mm
    flink[23] = 2ul*(last_dsp-first_dsp+1)*65536+FLOOR(rtr_map.switch_ReplyVector/524288.)*32768ul+(rtr_map.switch_ReplyVector-524288)/2

    ;rd_seq da tutti i DSP di ff_command (con irq)
    flink[24] = adsec.n_act_per_dsp*65536ul+8704
    ;flink[25] = 6ul*16777216+last_dsp*4096ul+first_dsp
    flink[25] = 6ul*16777216+last_crt*1048576ul+last_dsp_single_crt*4096ul+first_crt*256ul+first_dsp
    flink[26] = dsp_map.ff_command
    flink[27] = adsec.n_act_per_dsp*ulong(last_dsp-first_dsp+1)*65536ul+FLOOR(rtr_map.switch_FFCommandVector/524288.)*32768ul+(rtr_map.switch_FFCommandVector-524288)/2

    ;wr_same a tutti i DSP di ff_command_vector + start_ff (con irq)
    flink[28] = (adsec.n_actuators+4)*65536ul+8192                 ;MARIO MODIFICATION SUGGESTION
    ;flink[28] = (adsec.n_actuators+2)*65536ul+8192
    ;flink[29] = 7ul*16777216+last_dsp*4096ul+first_dsp
    flink[29] = 7ul*16777216+last_crt*1048576ul+last_dsp_single_crt*4096ul+first_crt*256ul+first_dsp
    flink[30] = rtr_map.ff_command_vector
    flink[31] = (adsec.n_actuators+4)*65536ul+FLOOR(rtr_map.switch_FFCommandVector/524288.)*32768ul+(rtr_map.switch_FFCommandVector-524288)/2 ;MARIO MODIFICATION SUGGESTION
    ;flink[31] = (adsec.n_actuators+2)*65536ul+FLOOR(rtr_map.switch_FFCommandVector/524288.)*32768ul+(rtr_map.switch_FFCommandVector-524288)/2

    ;rd_seq da tutti i DSP di start_ff (con irq) in ReplyVector
    flink[32] = 2ul*65536+8704
    ;flink[33] = 8ul*16777216+last_dsp*4096ul+first_dsp
    flink[33] = 8ul*16777216+last_crt*1048576ul+last_dsp_single_crt*4096ul+first_crt*256ul+first_dsp
    flink[34] = rtr_map.start_ff
    flink[35] = 2ul*(last_dsp-first_dsp+1)*65536+FLOOR(rtr_map.switch_ReplyVector/524288.)*32768ul+(rtr_map.switch_ReplyVector-524288)/2

    ;wr_same a tutti i DSP di start_dl appoggiato su ReplyVector (con irq, l'irq allo slave è fatto dal codice)
    ;flink[36] = 4ul*65536+8192
    ;flink[37] = 9ul*16777216+last_dsp*4096ul+first_dsp
    ;flink[38] = rtr_map.start_dl
    ;flink[39] = 4ul*65536+FLOOR(rtr_map.switch_ReplyVector/524288.)*32768ul+(rtr_map.switch_ReplyVector-524288)/2

    ;PATCH TO BE REMOVED AFTER LOGIC FIXING
    flink[36] = 40ul*65536+8192
    ;flink[37] = 9ul*16777216+last_dsp*4096ul+first_dsp
    flink[37] = 9ul*16777216+last_crt*1048576ul+last_dsp_single_crt*4096ul+first_crt*256ul+first_dsp
    flink[38] = rtr_map.start_dl
    flink[39] = 40ul*65536+FLOOR(rtr_map.switch_ReplyVector/524288.)*32768ul+(rtr_map.switch_ReplyVector-524288)/2


    ;SWITCH BCU accelerometer fastlink configuration and pointer configuration
;    num_acc = 8
;    last_crt = 5
;    acc_dsp = 248
;    dummy = ulonarr(4)
;    dummy[0] = num_acc * 65536ul+8704
;    dummy[1] = 10ul*16777216+(last_crt)*1048576ul + acc_dsp*4096ul + last_crt*256 +acc_dsp
;    dummy[2] = acc_map.AccFiltered
   ;va spostata nella inizializzazione di tutta la fibra!!!!!
;    dummy[3] = num_acc *65536ul+floor(switch_map.AccData/524288.)*32768ul+(switch_map.AccData)/2
;    err = write_same_dsp(sc.all, switch_map.FastlinkCmd+40, dummy, /SWITCH, /CHECK)

;ACCELEROMETER (from 1.11)
    ver = float(prog_vers.switchbcu.dsp) ge 1.11
    if ver then begin
        err = write_seq_dsp(sc.all, switch_map.AccModePtr, rtr_map.switch_ModesVector+adsec.n_actuators-3, /SWITCH, /CHECK)
        if err ne adsec_error.ok then return, err
    endif


    crate2switch_collect_conf = flink

    err = write_same_dsp(0, switch_map.FastlinkCmd, flink, /SWITCH)
    if err ne adsec_error.ok then begin
        print, 'Error writing address of ReplyVector area.'
        return, err
    endif

;----------------------------------------------------------------------------------------------------

    return, adsec_error.ok

end
