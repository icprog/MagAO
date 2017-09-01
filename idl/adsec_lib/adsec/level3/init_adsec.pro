
;$Id: init_adsec.pro,v 1.28 2009/12/21 10:55:08 marco Exp $$
;
;+
; INIT_ADSEC
;
;  Initialize all the global variables/structure used in the adsec software.
;  See adsec_common.pro for a list of them.
;
; HISTORY
;
;  created by A. Riccardi
;  Osservatorio Astrofisico di Arcetri, ITALY
;  <riccardi@arcetri.astro.it>
;
;  13 Mar 2002, AR
;    *The configuration of adsec_model,adsec_prog_ver,adsec_comm,
;     host_comm_default,vme_location and debug are no more defined
;     inside here.
;    *adsec_model='672a' is now supported
;
;  15 Apr 2003, AR
;    *adsec_model = 'P45' is now supported
;    *Odd number of actuators (half DSP) is now well managed (n_dsp computation)
;    *adsec.dummy_act has been introduced
;
;  12 Aug 2003, AR
;    *new adsec fields: n_dummy_act, true_act, n_true_act
;
;  31 Jan 2004, AR
;    *structure conf_gvar and array act2mir_act are restored
;     from ascii files (see read_ascii_structure and
;     read_ascii_array) instead of less portable IDL .sav files
;    *act2mir_act file definition moved from data to conf directory
;
;  03 Feb 2004, AR and M. Xompero
;    *act_wo_pos and act_wo_curr are now set using conf_gvar structure
;    *nominal_gap and curr4bias_mag added in adsec structure. The contents
;     is read from conf_gvar structure
;
;  17 Feb 2004, MX
;    *secure_factor is now set using conf_gvar structure
;
;  10 Mar 2004, AR
;    *new fields in adsec structure: n_bcu, n_board_per_bcu,
;     n_dsp_per_board, n_act_per_dsp. Their content is read from
;     conf_gvar.txt configuration file.
;    *adsec.n_crates is now read from conf_gvar.txt
;    *init_error_code call moved at the beginning of init_gvar
;    *patch structure initialization
;
;   08 Apr 2004, MX
;     *new fields in sc structure: field_port, fp_config pointer
;     *new structure: env_status for the monitoring of the enviroment
;
;  08 Apr 2004, AR
;    *adsec.n_dsp and adsec.last_dsp are computed considering n_act_per_dsp
;    *adsec.all_boards and adsec.all added
;
;   04 May 2004, MX
;     *adsec.ll_debug added in sc structure.
;
;  27 May 2004, AR
;    *structure fields added/modified:
;     max_ngain_step, min_timeout, dac_bits, adc_bits,
;     capsens_vref, capsens_vmax, pu_str, min_volt_val, max_volt_val,
;     fu_str, lu_str, nominal_gap, max_req_delta_lin_dist,
;     offset_delta_pos, A_per_dacbit, dac_offset, d0_sd, c_ratio_sd,
;     max_ngain_step, err_par, sampling_time, tv_sens_unit_list,
;     extra_ct.
;    *computation of force calibration coeffs using send_force_calibration
;    *modified data type to match LBT formats
;
;  14 Jul 2004
;     *pos_sat_force, neg_sat_force,  err_smoothed_step and speed_smoothed_step added
;
;  22 Jul 2004, MX
;     adsec.ff2bias_step field added.
;  28 Jul 2004, MX
;     adsec.curr_threshold filed added.
;  29 jul 2004 D.Zanotti(DZ)
;     adsec.max_amp_pos,max_amp_pos_ramp,max_deltacurr,max_gain_step,
;     min_gain_step,max_curr_tospread,max_iter_integ, thr_perr,max_curr
;  30 jul 2004, DZ
;     fact_reduction_curr added in adsec structure.
;     adsec.gain_to_bbgain is now float (previously was double)
;  03 Aug 2004, MX
;     max_dyn_peak_pos added in adsec. It's max position peak performed in the optimized
;     dynamical response. max_modal_abs_step added in adsec. it's the maximum modal position step.
;  13 Aug 2004, AR
;     tv_curr_unit_list changed (N -> mN)
;  3 Nov 2004, MX & DZ
;     init_gvar was changed in init_adsec.Its main functions have been distributed
;     to various specific modules.
;  01 Dec 2004, AR
;     added init_adsec_wave section, moved init_dsp_map ina different file position,
;     added init_adsec_forces and init_adsec_lin sections,
;     corrected configuration default filenames.
;  Feb/March 2005, DZ
;     added init_rtr_par and created a new rtr structure.
;     added init_rtr_b0, init_rtr_a_delay, init_rtr_b_delay, init_rtr_m2c.
;     added init_rtr_map and created a new rtrt_map structure.
;  29 Nov 2005, MX
;     new reconstructur parameter saved each call.
;  January, 2006 DZ
;    Now the comunication is opened at the end of the initialization.
;    Added the new structure adsec_state containing all the configuration file and
;     the current state of the ASM(Adaptive Secondary Mirror).
;    Updated the rtr structure, with a fied for the matrices of each memory block.
;    and the new optical loop gain field.
;    The RTR matrix file are now .fits
;   March 2006, MX
;    Added the clbuf structures.
;   Jun 2006, MX
;    Added the calibration parameters initialization.
;   Jan 2007, MX
;    Init_hw_map call changed.
;   March 2007, DZ
;     Added init_adsec_flash after the communication open.
;   March 2007, DZ
;     substituted  the init_adsec_flash with init_adsec_sram
;       after the communication open.
;   18 Apr 2007, MX
;    Added/changed parameters for programs path. Get_sw_ver function used.
;   19 Sep 2007, MX
;    ADAM configuration added.
;
;-

;================================
; initialize global variables
;================================
function init_adsec, NO_ASK=no_ask, REG_NAME=reg_name

    ; common blocks definition
    @adsec_common

;    catch, error_status
;    IF error_status NE 0 THEN BEGIN 
;        idl_error_status=1
;        return, !AO_ERROR
;    endif

;==========================================================================
; Environment configurations
;

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; the working directory is the current directory
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    adsec_home_src = filepath(ROOT=getenv('ADOPT_IDL_SOURCE'), SUB=['idl'],'')
    adsec_home     = filepath(ROOT=getenv('ADOPT_ROOT'), SUB=['idl'],'')
    cd, adsec_home



        ;;;;;;;;;;;;;;;;;;;;;;;
        ; default directories
        ;;;;;;;;;;;;;;;;;;;;;;;
        adsec_path =                                           $
          {                                                    $
            home: adsec_home,                                  $
            data: filepath(ROOT=getenv('ADOPT_ROOT'), SUB=['calib', 'adsec', !AO_CONST.unit, 'data'], ''), $ ; config. file dir.
            conf: filepath(ROOT=getenv('ADOPT_ROOT'), SUB=['conf', 'adsec', !AO_CONST.unit], ''), $ ; config. file dir.
            commons: filepath(ROOT=getenv('ADOPT_ROOT'), SUB=['conf', 'adsec', 'commons'], ''), $ ; config. file dir.
            prog: filepath(ROOT=getenv('ADOPT_ROOT'), SUB=['conf', 'adsec', 'prog'], ''), $ ; dsp progs dir
;           temp: filepath(ROOT=adsec_home, SUB=['temp'], ''), $ ; dir of temporary files
            meas: filepath(ROOT=getenv('ADOPT_MEAS'), SUB=['adsec_calib'], '')  $ ; dir of measurements files
            ;calib: filepath(ROOT=getenv('ADOPT_ROOT'), SUB=['calib', !AO_CONST.telescope_side, 'adsec'],'')  $ ; dir of measurements files
          }

        ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
        ; check if the default directories exist
        ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
        if not is_a_dir(adsec_path.data) then begin
            log_message, 'The AdSec data directory "'+adsec_path.data+'" does not exist.', ERR=-1
        endif
        if not is_a_dir(adsec_path.conf) then begin
            log_message, 'The AdSec data directory "'+adsec_path.conf+'" does not exist.', ERR=-1
        endif
        if not is_a_dir(adsec_path.prog) then begin
            log_message, 'The AdSec data directory "'+adsec_path.prog+'" does not exist.', ERR=-1
        endif
;        if not is_a_dir(adsec_path.temp) then begin
;            log_message, 'The AdSec directory "'+adsec_path.temp+'" does not exist.', ERR=-1
;        endif

        ;;=========================================================================
        ;; error code definition
        ;;
        init_error_codes
        ;;=========================================================================
    
        
        ;;=========================================================================
        ;; syntax check
        err = check_routines(bad)
        if err ne adsec_error.ok then begin
            log_print, "ERROR ON COMPILING ROUTINES: "+strjoin(bad, ' '), log_lev = !AO_CONST.log_lev_fatal
            ;if !AO_CONST.side_master then exit
            return, err
        endif
        ;;=========================================================================
        
        ;===========================================================================
        ; Definition of process_list structure and checks for all process up
        ;
        filename=filepath(Root=getenv('ADOPT_ROOT'), SUB=['conf','adsec', 'current', 'processConf'], "processList.conf")
        
        log_print, 'Examining filename: '+filename
        err = init_process_list(filename)
        if err ne adsec_error.ok then log_message,"The process list init and checking failed", ERR=err
        ;
        ; END OF: Definition of process list strucure
        ;========================================================================



        ;;=========================================================================
        ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
        ;;adsec_shell structure: Definition of the mirror parameters ;
        ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
        
        filename=filepath(ROOT=adsec_path.conf, "configuration.txt")
        log_print, 'Examining filename: '+filename
        str = read_ascii_structure(filename)
        !AO_CONST.shell = str.shell

        filename=filepath(ROOT=adsec_path.commons, SUB=[!AO_CONST.shell],"shell.txt")
        log_print, 'Examining filename: '+filename
        err=init_adsec_shell(filename)
        if err ne adsec_error.ok then log_message,"The adsec_shell structure was not initialized", ERR=adsec_error.idl_invalid_file


        ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
        ; Communication type selection  ;
        ;                               ;
        ; & system constants            ;
        ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

        ;
        ; End of the Environment configurations
        ;==========================================================================



        ;===========================================================================
        ; Definition of main parameters
        ;===========================================================================

        ;;===========================================================================
        ;; Restoring the look-up table to convert the DSP actuator numbers to
        ;; mirror geometry actuator numbers

        filename=filepath(ROOT=adsec_path.conf, SUB=[!AO_CONST.shell], "elec.txt")
        log_print, 'Examining filename: '+filename
        err=init_adsec_elec(filename, ACT2MIR = act2mir_act_filename, ST=elec, /NOFILL)
        if err ne adsec_error.ok then log_message,"The adsec elec structure was not initialized", ERR=adsec_error.idl_invalid_file
        adsec=elec

        ;;===========================================================================

        ;===========================================================================
        err = get_sw_ver()
        if err ne adsec_error.ok then log_message,"The software version loading cannot be done.", ERR=adsec_error.idl_invalid_file
        ;===========================================================================

        ;===========================================================================
        filename=filepath(ROOT=adsec_path.conf, "sys_const.txt")
        log_print, 'Examining filename: '+filename
        err=init_sys_const(NO_ASK=no_ask, REG_NAME=reg_name)
        if err ne adsec_error.ok then log_message,"The system constants structure was not initialized", ERR=adsec_error.idl_invalid_file
        selected_acts = [-1]   ;; any actuator currently selected
        sc.debug=0B;
        ;===========================================================================

        ;===========================================================================
        ; Definition of the filenames containing the initialization data

        ; file containing the feedforward matrix ff_matrix
        ff_filename       = filepath(ROOT=adsec_path.data, str.ff_matrix)
        ; actuator (aka mechanical or mirror) numbers
        act2mir_act_filename = filepath(ROOT=adsec_path.conf,SUB='general', 'act2mir_act.txt')
        ; file containing the rtr_map addresses
        rtr_map_filename = filepath(ROOT=adsec_path.prog, SUB=[sc.prog_ver], "curr_rtr_map.txt")
        ; file containing the projection matrices
        pmz_filename = filepath(ROOT=adsec_path.data, str.pmz)
        pmpos_filename = filepath(ROOT=adsec_path.conf,SUB=['processConf', 'fastdiagn'], str.pmpos)
        pmfor_filename = filepath(ROOT=adsec_path.conf,SUB=['processConf', 'fastdiagn'], str.pmfor)
        pmhofor_filename = filepath(ROOT=adsec_path.data, str.pmhofor)
        ;===========================================================================

        ;;===========================================================================
        ;; Restoring and computing data related to reference signal generation
        ;;

        err=init_adsec_wave(ST=wave_struct, /NOFILL)
        if err ne adsec_error.ok then log_message,"The wave section of adsec structure was not initialized", ERR=adsec_error.idl_invalid_file
        adsec=create_struct(adsec,wave_struct)

        ;;===========================================================================


        ;===========================================================================
        ; Restoring the feedforward matrix
        ;
        err=init_adsec_ff(FF_FILE=ff_filename, ST=ff_struct, /NOFILL)
        if err ne adsec_error.ok then log_message,"The adsec feedforward  structure was not initialized", ERR=adsec_error.idl_invalid_file
        adsec=create_struct(adsec,ff_struct)
        ;
        ; End of: Restoring the feedforward matrix
        ;===========================================================================


        ;===========================================================================
        ; Restoring the measured calibrations & Restoring the capsens calibrations

        filename=filepath(ROOT=adsec_path.conf, SUB=!AO_CONST.shell, "cals.txt")
        act_eff_path = filepath(ROOT=adsec_path.conf,  SUB=!AO_CONST.shell, 'act_eff_calib.txt')
        d0_path = filepath(ROOT=adsec_path.conf,  SUB=!AO_CONST.shell, 'd0_calib.txt')
        d0_sd_path = filepath(ROOT=adsec_path.conf, SUB=!AO_CONST.shell,'d0_sd_calib.txt')
        if file_search(act_eff_path) ne act_eff_path then undefine, act_eff_path
        if file_search(d0_path) ne d0_path then undefine, d0_path
        if file_search(d0_sd_path) ne d0_sd_path then undefine, d0_sd_path
        err=init_adsec_cals(filename, ST=calib, /NOFILL, ACT_EFF_PATH=act_eff_path, DO_PATH=d0_path, DO_SD_PATH=d0_sd_path)
        if err ne adsec_error.ok then log_message,"The adsec calibration  structure was not initialized", ERR=adsec_error.idl_invalid_file
        adsec=create_struct(adsec,calib)
        ;
        ; End of: Restoring the capsens calibrations  & Restoring the capsens calibrations
        ;==========================================================================


        ;==========================================================================
        ; Calculating force calibations parameters
        err=init_adsec_forces(ST=forces, /NOFILL, SAFE=0)
        if err ne adsec_error.ok then log_message,"The adsec forces calibrations fields are not initialized", ERR=adsec_error.idl_invalid_file
        adsec=create_struct(adsec,forces)

        ; End calculating force calibrations parameters
        ;==========================================================================


        ;==========================================================================
        ; Calculating linearizations parameters
        err = init_adsec_lin(ST=lin, /NOFILL, SAFE=0)
        if err ne adsec_error.ok then log_message,"Error computing the linearization parameters", ERR=adsec_error.idl_invalid_file
        adsec=create_struct(adsec,lin)
        ; End calculating linearizations calibrations parameters
        ;==========================================================================

        ;===========================================================================
        ; Definition of DSP constants and memory map
        ;
        err = init_dsp_map()
        ; END OF: Definition of DSP constants and memory map
        ;========================================================================

        ;===========================================================================
        ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
        ; Restoring the time filter data
        ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

        filename=filepath(ROOT=adsec_path.conf, SUB='general', "filters.txt")
        err=init_adsec_filters(filename, TFL = tfl_data_filename, ST=tfl_struct, /NOFILL)
        if err ne adsec_error.ok then log_message,"The adsec filter  structure was not initialized", ERR=adsec_error.idl_invalid_file
        adsec=create_struct(adsec,tfl_struct)

        ;; End of: Restoring the time filter data
        ;==========================================================================


        ;==========================================================================
        ; Definition and initialization of the structure where is saved the
        ; actual status of the system
        ;
        init_sys_status
        ;
        ;===========================================================================


        ;===========================================================================
        ; Creating rtr sruct
        ;
        err=init_rtr_par(ST=rtr_struct, /NOFILL)
        if err ne adsec_error.ok then log_message,"The rtr parameters was not created", ERR=adsec_error.idl_invalid_file
        rtr=rtr_struct
        ;
        ; End of: Creating rtr struct
        ;===========================================================================


        ;calcolo la nuova suddivisione in memoria
        err=init_rtr_map(/sav)
        if err ne adsec_error.ok then log_message, 'The new rtr_map structure was not created', ERR=adsec_error.idl_invalid_file

        ;===========================================================================
        ; Restoring the B0 matrix gain
        ;
        err=init_rtr_b0(B0_A_FILE=b0_a_filename,B0_B_FILE=b0_b_filename, ST=b0_struct, /NOFILL)
        if err ne adsec_error.ok then log_message,"The adsec B0  structure was not initialized", ERR=adsec_error.idl_invalid_file
        rtr=create_struct(rtr,b0_struct)
        ;
        ; End of: Restoring the B0 matrix gain
        ;===========================================================================

        ;===========================================================================
        ; Restoring the A_delay  matrix gain
        ;
        err=init_rtr_a_delay(A_DELAY_FILE=a_delay_filename, ST=a_mtrx_struct, /NOFILL)
        if err ne adsec_error.ok then log_message,"The A_delay mtrx structure was not initialized", ERR=adsec_error.idl_invalid_file
        rtr=create_struct(rtr,a_mtrx_struct)
        ;
        ; End of: Restoring the A_delay matrix
        ;===========================================================================
        ;===========================================================================
        ; Restoring the B_delay matrix gain
        ;
        ierr=init_rtr_b_delay(B_DELAY_A_FILE=b_delay_a_filename,B_DELAY_B_FILE=b_delay_b_filename, ST=b_mtrx_struct, /NOFILL)
        if err ne adsec_error.ok then log_message,"The B_delay  mtrx  structure was not initialized", ERR=adsec_error.idl_invalid_file
        rtr=create_struct(rtr,b_mtrx_struct)
        ;
        ; End of: Restoring the  B_delay matrix gain
        ;===========================================================================

        ;===========================================================================
        ; Restoring the M2C matrix gain
        ;
        err=init_rtr_m2c(M2C_FILENAME=m2c_filename, ST=m2c_struct, /NOFILL)
        if err ne adsec_error.ok then log_message,"The m2C    structure was not initialized", ERR=adsec_error.idl_invalid_file
        rtr=create_struct(rtr,m2c_struct)
        ; End of: Restoring the  M2C matrix
        ;==========================================================================

        ;===========================================================================
        ; Restoring the optical loop gain
        ;
        err=init_rtr_g_gain(G_GAIN_A_FILE=g_gain_a_filename, G_GAIN_B_FILE=g_gain_b_filename, ST=g_gain_struct, /NOFILL)
        if err ne adsec_error.ok then log_message,"The optical loop gains was not initialized", ERR=adsec_error.idl_invalid_file
        rtr=create_struct(rtr,g_gain_struct)
        ; End of: Restoring the Optical loop gains
        ;==========================================================================

        ;;===========================================================================
        ; Initialization of optical loop data queues
        init_cl_buffer

        ; End of initialization optical loop data queue
        ;==========================================================================


        ;===========================================================================
        ; Initialization of adsec_state structure
        ;
;        err=init_adsec_state(ST=state_struct, /NOFILL)
;        if err ne adsec_error.ok then log_message,"The adsec_state structure was not initialized", ERR=adsec_error.idl_invalid_file
;        adsec_state=state_struct


        ;===========================================================================
        ; Open communication

            err = config_network()
            if err ne adsec_error.ok then begin
                log_message, 'The '+ sc.host_comm +' communication initialization FAILED!', ERR=err
            endif else print, '... done.'

        ;===========================================================================

        ;===========================================================================
        err = get_sw_ver()
        if err ne adsec_error.ok then log_message,"The software version loaded are different from the exspected.", ERR=err
        *!AO_STATUS.firmware = prog_vers
        ;===========================================================================


;        shmmap, 'SHM_ADSEC_'+strupcase(!AO_CONST.telescope_side), TEMPLATE=ADSEC, /DESTROY_SEGMENT, /SYSV
;        shmmap, 'SHM_SYS_STATUS_'+strupcase(!AO_CONST.telescope_side), TEMPLATE=SYS_STATUS, /DESTROY_SEGMENT, /SYSV

    if sc.name_comm ne "Dummy" then begin

        if !AO_CONST.side_master then begin

;            shmmap, 'SHM_ADSEC_'+strupcase(!AO_CONST.telescope_side), TEMPLATE=ADSEC, /DESTROY_SEGMENT
;            shmmap, 'SHM_SYS_STATUS_'+strupcase(!AO_CONST.telescope_side), TEMPLATE=SYS_STATUS, /DESTROY_SEGMENT
            log_print, "IDL SESSION MASTER: full control of unit allowed"
;            tmp = adsec
;            undefine, adsec
;            adsec = shmvar('SHM_ADSEC_'+strupcase(!AO_CONST.telescope_side))
;            adsec[0] = tmp[0]
;
;            shm_sys_status = shmvar('SHM_SYS_STATUS_'+strupcase(!AO_CONST.telescope_side))
;            shm_sys_status[0] = sys_status[0]

        endif else begin

;            shmmap, 'SHM_ADSEC_'+strupcase(!AO_CONST.telescope_side), TEMPLATE=ADSEC
;            shmmap, 'SHM_SYS_STATUS_'+strupcase(!AO_CONST.telescope_side), TEMPLATE=SYS_STATUS

            log_print, "IDL SESSION SLAVE: only read permission allowed"
;            adsec = shmvar('SHM_ADSEC_'+strupcase(!AO_CONST.telescope_side))
;            shm_sys_status = shmvar('SHM_SYS_STATUS_'+strupcase(!AO_CONST.telescope_side))

        endelse

    endif

    ;===========================================================================
    ; definition and initialization of the structure where is saved the
    ; actual status of the system ambient and volages
    ;

;    filename=filepath(Root=adsec_path.conf, "fp.txt")
;    err=init_lab_fp(filename)
;    if err ne adsec_error.ok then log_message,"The env_status  structure was not initialized", ERR=err

    ;===========================================================================


    ;===========================================================================
    ; definitions for displaying the actuator pattern

    filename=filepath(Root=adsec_path.conf, SUB='general', "graph.txt")
    log_print, 'Examining filename: '+filename
    err=init_adsec_gr(filename)
    if err ne adsec_error.ok then log_message,"The gr  structure was not initialized", ERR=err
    ;
    ; End of: definitions for displaying the actuator pattern
    ;===========================================================================

        ;===========================================================================
        ; Creating projection matrices structure
        ;
        err =init_proj_mat(pmz = pmz_filename, pmfor = pmfor_filename, pmpos = pmpos_filename, pmhofor=pmhofor_filename) 
        if err ne adsec_error.ok then log_message,"The projection matrices were not initialized", ERR=adsec_error.idl_invalid_file
        ;
        ; End of: Creating projetion matrices structure
        ;===========================================================================

    ;===========================================================================
    ; Definition of enviroment structure
    ;
    init_env_templates
    ;
    ; END OF: Definition of enviroment structure
    ;========================================================================


    ;;===========================================================================
    ;;ADAM status initialization
    err = init_adam()
    if err ne adsec_error.ok then return, err
;    if err ne adsec_error.ok then begin
;        log_print,"The ADAM connection cannot be established. Skipped"
;        err1 = init_adam(/NOCONNECT)
;        if err1 ne adsec_error.ok then log_message,"The ADAM connection cannot be initialized..", ERR=err1
;        for i=0, n_elements(tag_names(adam_out))-1 do adam_out.(i)=0
;        for i=0, n_elements(tag_names(adam_in))-1 do adam_in.(i)=0
;        log_print, "Only base ADAM structures are built."
;    endif
    ;;===========================================================================

    ;;===========================================================================
    ;;FLAT table initialization
    err = update_flat_table(/READTABLE)
    if err ne adsec_error.ok then log_message,"The FLAT table cannot be established.", ERR=err
    ;;===========================================================================

    err = getconftn(struct)
    if err ne adsec_error.ok then log_message,"Some problem in the TN recognition occured.", ERR=err
    *!AO_STATUS.configuration = struct

;    msg =""
;        err= update_popmsg(msg, 'NWACT', /CLEAR)
;        err= update_popmsg(msg, 'CONTAMINATION', /CLEAR)

    !AO_STATUS.ACT_W_POS[adsec.act_w_pos]=1
    !AO_STATUS.ACT_W_CURR[adsec.act_w_curr]=1
    return, update_rtdb()

end
