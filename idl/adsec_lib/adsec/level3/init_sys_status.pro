; $Id: init_sys_status.pro,v 1.14 2009/11/24 16:57:08 marco Exp $
;+
;   HISTORY
;   01 Dec 2004: Marco Xompero
;    pos_preshaper_len and curr_preshaper_len fields added
;   04 Aug 2005, MX
;    new fast_diag_freq field added.
;   02 July 2007, A. Riccardi (AR)
;    added time_stamo0 and time_stamp1 fields
;   19 Sep 2007, MX
;    ZENITH ANGLE variable added. Shared memory initialization for multiple IDL viewing added. 
;-
pro init_sys_status

    @adsec_common

    sys_status_template = $
      { $
        $;;;*** communication stuff ***
        $
        sys_is_open:           0B, $ ; 1B/0B the system is open(initialized)/closed
        hl_num_device:          0, $ ; HotLink device number
        hl_receive_channel:    1B, $ ; HotLink receive channel (0B/1B = A/B)
        hl_start_of_frame:     1B, $ ; HotLink start of frame byte
        adam_connected:     0B, $ ; HotLink start of frame byte
        $
        $;;;***
        $
        bad_actuators:      bytarr(adsec.n_actuators), $ ; 0B 1B 2B se l'attuatore e'
        $                           ; OK, attua solo corrente,  o e' morto rispettivamente
        cont_update:        0B, $   ; vero se abilitato l'update continuo
        monitor:            0B, $   ; vero se abilitato il monitoring durante l'update continuo
        next_update:       1.0, $   ; intervallo in secondi da il prossimo update
        ndata2ave:        128L, $   ; numero di dati su cui fare la media per determinare lo stato
        saving_decimation:  0L, $   ; save data every (saving_decimation+1)*adsec.sampling_time
        fast_diag_freq:   -1.0, $   ; real frequency of fast diagnostic
        closed_loop:        bytarr(adsec.n_actuators), $ ; is the loop closed? data for each act.
        position:           fltarr(adsec.n_actuators), $ ; position vector. Init to 0
        offset_pos:         fltarr(adsec.n_actuators), $ ; offset position vector. Init to 0
        current:            fltarr(adsec.n_actuators), $ ; current vector. Init to 0
        offset_curr:        fltarr(adsec.n_actuators), $ ; offset position vector. Init to 0
        position_sd:        fltarr(adsec.n_actuators), $ ; max-min position vector
        min_pos:            fltarr(adsec.n_actuators), $ ; min position vector
        max_pos:            fltarr(adsec.n_actuators), $ ; max position vector
        voltage:            fltarr(adsec.n_actuators), $ ; position vector in voltage counts. Init to 0
        offset_volt:        fltarr(adsec.n_actuators), $ ; offset position vector in voltage counts. Init to 0
        current_sd:         fltarr(adsec.n_actuators), $ ; current stdev vector
        command:            fltarr(adsec.n_actuators), $ ; command vector
        offset_comm:        fltarr(adsec.n_actuators), $ ; offset command vector. Init to 0
        ctrl_current:       fltarr(adsec.n_actuators), $ ; control current vector in curr. counts. Init to 0
        offset_ctrl_curr:   fltarr(adsec.n_actuators), $ ; control current vector in curr. counts. Init to 0
        bias_current:       fltarr(adsec.n_actuators), $ ; bias current vector in curr. counts. Init to 0
        full_bias_current:  fltarr(adsec.n_actuators), $ ; bias current vector in curr. counts in Optical loop operation
        bias_command:       fltarr(adsec.n_actuators), $ ; bias command vector in pos. counts. Init to 0
        full_bias_command:  fltarr(adsec.n_actuators), $ ; bias command vector in pos. counts in Optical loop operation
        offset_bias_curr:   fltarr(adsec.n_actuators), $ ; 
        ff_current:         fltarr(adsec.n_actuators), $ ; feedforeward current vector in curr. counts. Init to 0
        offset_ff_curr:     fltarr(adsec.n_actuators), $ ; feedforeward current vector in curr. counts. Init to 0
        offload_cmd:        fltarr(adsec.n_actuators), $ ; offload commad values
        offload_curr:       fltarr(adsec.n_actuators), $ ; offload current values
        flat_offload_cmd:   fltarr(adsec.n_actuators), $ ; offload commad values
        flat_offload_curr:  fltarr(adsec.n_actuators), $ ; offload current values
        cflat_offload_cmd:  fltarr(adsec.n_actuators), $ ; current flat offload command values
        cflat_offload_curr:  fltarr(adsec.n_actuators), $ ; current flat offload current values
        pos_preshaper_len:  fltarr(adsec.n_actuators), $ ; positions preshaper length vector
        curr_preshaper_len: fltarr(adsec.n_actuators), $ ; currents preshaper length vector
        pgain:              fltarr(adsec.n_actuators), $ ; proportional gain
        dgain:              fltarr(adsec.n_actuators), $ ; dericative gain
        icc_dcgain:         fltarr(adsec.n_actuators), $ ; dericative gain
        icc_cutfreq:        fltarr(adsec.n_actuators), $ ; dericative gain
        elevation_angle:    90d0                     , $ ; current elevation angle from zenith in DEGREE
        ;pmz_angle:          0d0                      , $ ; current rotation angle between zernike calibrated reference system and zernike use at the telescope
        time_stamp0:        0d0                      , $ ; time stamp at the beginning of data download (systime(1) format)
        time_stamp1:        0d0                      , $ ; time stamp at the end of data download (systime(1) format)
        fast_diag_data:     0B                       , $ ; 1B if sys_status contains data from fast diagnostics
        mirror_cnt:         0UL                      , $ ; mirror frame counter
        pending_skip_frame_cnt:0UL                   , $ ; counter of skipped frames for accumulator pending error
        safe_skip_frame_cnt:0UL                      , $ ; counter of skipped frames for safe range test on modes, commands or positions
        num_fl_crc_err:     0UL                      , $ ; counter of fault communications due to fast-link crc error
        num_fl_timeout:     0UL                        $ ; counter of fault communications due to fast-link timeout
      }

      sys_status= sys_status_template
  
;    if (sc.name_comm ne "Dummy") then begin
        
        ;help, /shared, OUTPUT=out
        ;if ~total(strmatch(out, '*SHM_SYS_STATUS_'+strupcase(!AO.telescope_side)+'*')) then $
        ;    shmmap,  "shm_sys_status_"+!AO.telescope_side
        ;sys_status = sys_status_template
        ;shm_sys_status = shmvar("shm_sys_status_"+telescope_side, TEMPLATE=sys_status_template) 

;    endif
    ;stop

end
