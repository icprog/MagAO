;function rtr_dsp_map, n_slope,  STRUCT=struct, NOFILL=nofill
;+
; RTR_DSP_MAP
;
;  Create  the map memory of reconstrtructor parameters
;   and fill it in a structure.
;
;
;  USAGE:
;   err = RTR_DSP_MAP(N_SL=n_sl, SL_DL=sl_dl, DP_DL=dp_dl, STRUCT=struct)
;
;   INPUT:
;       None.
;   OUTPUT:
;    Error code.
;
;   KEYWORD:
;     N_SL : if set is the slope number used, otherwise it used the rtr.n_slope.
;     SL_DL : if set is the slope delay number used, otherwise it used the rtr.n_slope_delay.
;     DP_DL : if set is the modes delay number used, otherwise it used the rtr.n_modes_delay.
;     STRUCT : structure creates in the file in which the reconstructor map is saved.
;
;
; HISTORY
;
;  created by D. Zanotti
;  Osservatorio Astrofisico di Arcetri, ITALY
;  <zanotti@@arcetri.astro.it>
;
;   03 Aug 2005, Marco Xompero (MX)
;       Fast diagnostic record len configurion added.
;   29 Aug 2005, Armando Riccardi
;       Error handling fixed.
;   29 Nov 2005, MX
;       use of safe address now permitted and configurable.
;   07 Feb 2007 MX
;       LUT of variable deleted and name refurbished.
;       Updated with new variables.
;   20 Feb 2007 MX
;       Memory map fixed.
;   26 Feb 2007 MX
;       Matrix_K and M2C now are in fixed position.
;       NB REMOVE PATCH on UPDATE_FF AFTER LOGIC FIXING
;   18 Apr 2007, MX
;       Added/changed parameters for programs path.
;-
function rtr_dsp_map, N_SL=n_sl, SL_DL=sl_dl, DP_DL=dp_dl, DIAG_LEN=diag_len $
                               , STRUCT=struct, SAVEDFILE=savedfile

    @adsec_common

    ;DSP MEMORY MAP
    if not keyword_set(n_sl) then n_slope=rtr.n_slope else n_slope=n_sl
    if not keyword_set(sl_dl) then n_slope_delay=rtr.n_slope_delay $
        else n_slope_delay=sl_dl
    if not keyword_set(dp_dl) then n_modes_delay=rtr.n_modes_delay $
        else n_modes_delay=dp_dl
    if not keyword_set(diag_len) then block_diagn=rtr.diag_len $
        else block_diagn = diag_len

    ;n_slope=ulong(ceil(n_slope/4.)*4) ;??blocchi di 4??
    id_matrices_bank = 2
    id_safeptr_bank = 1


    ;block_diagn=44

    ;memory bank #1
    ;addr_slope_vector = dsp_map.speed_coeff+(dsp_const.max_filters_coeff+(dsp_const.max_filters_coeff-1))*adsec.n_act_per_dsp ;'24'xul
    addr_slope_vector = '89000'xul
    ;addr_start_rtr = addr_slope_vector+n_slope
    ;addr_mirror_frames_counter = addr_start_rtr+1ul
    ;addr_wfs_frames_counter = addr_mirror_frames_counter+1ul
    ;addr_param_selector = addr_wfs_frames_counter+1ul

    addr_wfs_frames_counter = addr_slope_vector+n_slope
    addr_param_selector = addr_wfs_frames_counter+1ul
    addr_start_rtr = addr_param_selector+1ul
    addr_mirror_frames_counter = addr_start_rtr+1ul

    addr_modes_vector = addr_mirror_frames_counter+1ul
    addr_start_mm = addr_modes_vector+ulong(rtr.n_modes2correct)
    if (addr_start_mm+1) mod 4ul eq 0 then not_used = 0 else $
             not_used=4ul-((addr_start_mm+1) mod 4ul)

    addr_command_vector = addr_start_mm+1ul+not_used
    addr_update_cmd = addr_command_vector+adsec.n_actuators
    if (addr_update_cmd+1) mod 4ul eq 0 then not_used = 0 else $
             not_used=4ul-((addr_update_cmd+1) mod 4ul)

    addr_ff_command_vector = addr_update_cmd+1l+not_used
    addr_start_ff = addr_ff_command_vector+adsec.n_actuators
    if (addr_start_ff+1) mod 4ul eq 0 then not_used = 0 else $
             not_used=4ul-((addr_start_ff+1) mod 4ul)

    addr_start_dl = addr_start_ff+1ul+not_used
    if (addr_start_dl+1) mod 4ul eq 0 then not_used = 0 else $
             not_used=4ul-((addr_start_dl+1) mod 4ul)

    addr_update_ff= addr_start_dl+1ul+not_used      + 36ul  ;;;;;; PATCH TO BE REMOVED AFTER LOGIC FIXING

    if (addr_update_ff+1) mod 4ul eq 0 then not_used = 0 else $
             not_used=4ul-((addr_update_ff+1) mod 4ul)

    addr_slope_delay = addr_update_ff+1UL+not_used
    addr_modes_delay=addr_slope_delay+ulong(n_slope*n_slope_delay)
    addr_diagnostic_buffer=ulong(ceil((addr_modes_delay+ulong(rtr.n_modes2correct*n_modes_delay))/4)*4)
    if (addr_diagnostic_buffer+1) mod 4ul eq 0 then not_used = 0 else $
             not_used=4ul-((addr_diagnostic_buffer+1) mod 4ul)
    
    ;addr_disturb_buffer=addr_diagnostic_buffer+1ul+not_used
    addr_disturb_buffer='8B000'xul

    ;set test_address and safe location for critical pointers
    end_mem = dsp_datasheet.data_block_offset*id_safeptr_bank+dsp_datasheet.data_mem_size-1
    if addr_disturb_buffer+rtr.disturb_len gt end_mem then begin
       message, "Too large reconstructor or too many filter coefficients or too much disturb data. No enough memory.", $
                 CONT=sc.debug eq 0B
        return, adsec_error.IDL_OUT_DSP_MEM
    endif

    ;addr_safe_update_ff = end_mem -1
    ;addr_safe_update_cmd = addr_safe_update_ff -1
    ;addr_safe_start_ff = addr_safe_update_cmd -1
    ;addr_safe_start_mm = addr_safe_start_ff -1
    ;addr_safe_start_rtr = addr_safe_start_mm -1
    addr_test_address = end_mem 

    ;set the matrices on bank #2
    ;addr_matrix_b0a = dsp_datasheet.data_block_offset*id_matrices_bank
    addr_matrix_k = dsp_datasheet.data_block_offset*id_matrices_bank
    addr_matrix_m2c = addr_matrix_k+adsec.n_actuators*adsec.n_act_per_dsp
    addr_matrix_b0a = addr_matrix_m2c+rtr.n_modes2correct*adsec.n_act_per_dsp
    addr_matrix_b0b = addr_matrix_b0a+adsec.n_act_per_dsp*n_slope
    addr_matrix_ba_delay = addr_matrix_b0b+adsec.n_act_per_dsp*n_slope
    addr_matrix_bb_delay = addr_matrix_ba_delay+adsec.n_act_per_dsp*n_slope*n_slope_delay
    addr_matrix_a_delay = addr_matrix_bb_delay+adsec.n_act_per_dsp*n_slope*n_slope_delay
    ;addr_matrix_k = addr_matrix_a_delay+rtr.n_modes2correct*adsec.n_act_per_dsp*n_modes_delay
    ;addr_matrix_m2c = addr_matrix_k+adsec.n_actuators*adsec.n_act_per_dsp
    ;end_area = addr_matrix_m2c-1ul+rtr.n_modes2correct*adsec.n_act_per_dsp
    end_area = addr_matrix_a_delay-1ul+rtr.n_modes2correct*n_modes_delay*adsec.n_act_per_dsp
    if end_area gt id_matrices_bank*dsp_datasheet.data_block_offset+dsp_datasheet.data_mem_size  then begin
        message, "Too large reconstructor or too many filter coefficients. No enough memory.", $
                 CONT=sc.debug eq 0B
        return, adsec_error.IDL_OUT_DSP_MEM
    endif

    ;CRATE BCU MEMORY MAP
    bcu_ReplyVector = '80000'xul
    bcu_DiagVector = '100000'xul
    

    ;SWITCH BCU CONFIGURATION
    SCSlopeVector = '80000'xul
    SCStartRTR = SCSlopeVector+rtr.n_slope
    Header = SCStartRTR+4
    SlopeVector = Header+4
    StartRTR = SlopeVector+rtr.n_slope
    ModesVector = StartRTR+4
    StartMM = ModesVector+rtr.n_modes2correct
    FFCommandVector = StartMM+4
    StartFF = FFCommandVector+adsec.n_actuators
    ReplyVector = StartFF+4

;;=========================================================================

    struct = {                                                          $
            ;DSP BCU RECONSTRUCTOR ADDRESSES
            slope_vector            : long(addr_slope_vector),          $
            wfs_frames_counter      : long(addr_wfs_frames_counter),    $
            param_selector          : long(addr_param_selector),        $
            start_rtr               : long(addr_start_rtr),             $
            mirror_frames_counter   : long(addr_mirror_frames_counter), $
            modes_vector            : long(addr_modes_vector),          $
            start_mm                : long(addr_start_mm),              $
            command_vector          : long(addr_command_vector),        $
            update_cmd              : long(addr_update_cmd),            $
            ff_command_vector       : long(addr_ff_command_vector),     $
            start_ff                : long(addr_start_ff),              $
            start_dl                : long(addr_start_dl),              $
            update_ff               : long(addr_update_ff),             $
            slope_delay             : long(addr_slope_delay),           $
            modes_delay             : long(addr_modes_delay),           $
            diagnostic_buffer       : long(addr_diagnostic_buffer),     $
            disturb_buffer          : long(addr_disturb_buffer),        $
            test_address            : long(addr_test_address),          $
            matrix_K                : long(addr_matrix_k),              $
            matrix_M2C              : long(addr_matrix_m2c),            $
            matrix_B0a              : long(addr_matrix_b0a),            $
            matrix_B0b              : long(addr_matrix_b0b),            $
            matrix_Ba_delay         : long(addr_matrix_ba_delay),       $
            matrix_Bb_delay         : long(addr_matrix_bb_delay),       $
            matrix_A_delay          : long(addr_matrix_a_delay),        $
            ;CRATE BCU RECONSTRUCTOR ADDRESSES
            bcu_ReplyVector         : long(bcu_ReplyVector),           $
            bcu_DiagVector          : long(bcu_DiagVector),            $
            ;SWITCH BCU RECONSTRUCTOR ADDRESSES
            switch_SCSlopeVector    : long(SCSlopeVector),             $
            switch_SCStartRTR       : long(SCStartRTR),                $
            switch_Header           : long(Header),                    $
            switch_SlopeVector      : long(SlopeVector),               $
            switch_StartRTR         : long(StartRTR),                  $
            switch_ModesVector      : long(ModesVector),               $
            switch_StartMM          : long(StartMM),                   $
            switch_FFCommandVector  : long(FFCommandVector),           $
            switch_StartFF          : long(StartFF),                   $
            switch_ReplyVector      : long(ReplyVector)                $
       }


    if keyword_set(savedfile) then begin
        root_dir=sc.prog_ver
        namefile = filepath(ROOT=root_dir,"curr_rtr_map.txt")
        write_structure_map, struct, filename=namefile
    endif


    return, adsec_error.ok

end
