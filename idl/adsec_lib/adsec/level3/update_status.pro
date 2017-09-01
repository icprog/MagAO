; $Id: update_status.pro,v 1.23 2009/09/18 08:25:02 marco Exp $
;+
;   NAME:
;    UPDATE_STATUS
;
;   PURPOSE:
;    This function perfoms the refresh of the IDL internal variable by reading the
;    values from the phisycal system.
;
;   USAGE:
;    err = update_status(ndata2ave)
;
;   INPUT:
;    ndata2ave:     number of data to average.
;                   Optional input, if not defined ndata2ave=sys_status.ndata2ave is used.
;                   This parameter has to be long or ulong.
;
;   OUTPUT:
;    Error code.
;
;   KEYWORD:
;    None.
;
;   NOTE:
;    None.
;
; HISTORY
;   28 May 2004 Armando Riccardi (AR)
;       Completely rewritten to match the LBT format.
;   27 Jul 2004 Marco Xompero (MX)
;       Help added.
;       dsp_const.max_data_acc_len used in the checks.
;   30 Jul 2004 AR and D. Zanotti
;       fixed test on ndata2ave value
;   02 Nov 2004, MX
;       Adsec.all changed in sc.all.
;   01 Dec 2004, MX
;       new pos_preshaper_len and curr_preshaper_len fields managed
;   04 Aug 2005, MX
;       New fast diagnostic frequency now managed.
;   07 Feb 2007 MX
;       LUT of variable deleted and name refurbished.
;   03 April 2007 MX
;       In the dsp code there is a numerical error between the update current and average current.
;   02 July 2007 AR
;       Added a circular buffer to store some sys_status history
;       Added time-stamps in the sys_status structure:
;       sys_status.time_stamp0: time stamp at the beginning of the data downloading
;       sys_status.time_stamp1: time stamp at the end of the data downloading
;       see systime(1) for the format of the time-stamp
;   04 Nov 2007, MX
;       Added fast diagnostic integration. Added bias force/command
;       data.
;   14 Aug 2008, AR
;       added handling of new sys_status fields: fast_diag_data,
;       mirror_cnt, pending_skip_frame_cnt, safe_skip_frame_cnt,
;       num_fl_crc_err, num_fl_timeout.
;       Moved the reading of full_bias_current and full_bias_command
;       before the reading of fast diagnostic record.
;
;-
function update_status, ndata2ave

    @adsec_common
    @xadsec_common
    common sys_buffer_len_block, sys_buffer_len 
    if n_elements(sys_buffer) eq 0 then begin
        sys_buffer_len = 100
        sys_buffer = replicate(sys_status, sys_buffer_len)
        sys_buffer_idx = 0
        sys_buffer.time_stamp0=0.0
         sys_buffer.time_stamp1=0.0
    endif

    if (rtr.oversampling_time gt 0) then  usefast_xadsec = 1B else  usefast_xadsec = 0B
    usefast = usefast_xadsec

    if n_elements(ndata2ave) eq 0 then begin
        samples = sys_status.ndata2ave
    endif else begin
        if test_type(ndata2ave, /NOFLOAT, N_EL=n_el) then begin
            message, "ndata2ave must be a long or unsigned long.", CONT=(sc.debug eq 0B) 
            return, adsec_error.input_type
        endif
        if n_el ne 1 then begin
            message, "ndata2ave must be a scalar.", CONT=(sc.debug eq 0B)
            return, adsec_error.input_type
        endif
        if ndata2ave[0] le 0 or ndata2ave[0] ge long(dsp_const.max_data_acc_len) then begin
            message, "ndata2ave must be positive and less then dsp_const.max_data_acc_len.", CONT=(sc.debug eq 0B)
            return, adsec_error.input_type
        endif
        samples = ndata2ave[0]
    endelse


    sys_status.time_stamp0 = systime(1)

    err = read_seq_ch(sc.all_actuators, dsp_map.fullbias_curr, 1L, fb_curr)
    if err ne adsec_error.ok then return, err
    sys_status.full_bias_current = fb_curr

    err = read_seq_ch(sc.all_actuators, dsp_map.fullbias_cmd, 1L, fb_cmd)
    if err ne adsec_error.ok then return, err
    sys_status.full_bias_command = fb_cmd

         
        if usefast then begin
            err = get_fast_diagnostic(fastdata)
            if err ne adsec_error.ok then return, err
            sys_status.position = fastdata.DistAverage
            sys_status.current = fastdata.CurrAverage
            sys_status.position_sd = fastdata.DistRMS
            sys_status.current_sd = fastdata.CurrRMS
            sys_status.ff_current = fastdata.FFPureCurrent+fastdata.IntControlCurrent+sys_status.full_bias_current
            sys_status.command = (fastdata.NewDeltaCommand+fastdata.FFCommand+sys_status.full_bias_command)
            sys_status.fast_diag_data = 1B
            err = test_skip_frame(MIRROR_CNT=mirrc, PENDING_SKIP_FRAME_CNT=pendc, SAFE_SKIP_FRAME_CNT=safec $
                                , FL_CRC_ERR=fl_crc, FL_TIMEOUT=fl_tim, /NOVERB)
            if err ne adsec_error.ok then return, err
            sys_status.mirror_cnt = mirrc
            sys_status.pending_skip_frame_cnt = pendc
            sys_status.safe_skip_frame_cnt = safec
            sys_status.num_fl_crc_err = fl_crc
            sys_status.num_fl_timeout = fl_tim

        endif else begin

            if !ao_const.side_master then begin
                err = get_ave_pc(pos, curr, min_pos, max_pos, SAMPLES=samples, DELAY=0L)
                if err ne adsec_error.ok then return, err

                sys_status.position = pos
                sys_status.current = curr
                sys_status.max_pos = max_pos
                sys_status.min_pos = min_pos
                sys_status.position_sd = max_pos-min_pos
                sys_status.current_sd = 0

                err = get_preshaper(sc.all_actuators, presh_curr, /FF)
                if err ne adsec_error.ok then return, err
                sys_status.curr_preshaper_len = presh_curr

                err = get_preshaper(sc.all_actuators, presh_pos)
                if err ne adsec_error.ok then return, err
                sys_status.pos_preshaper_len = presh_pos
                sys_status.fast_diag_data = 0B

            endif
        endelse


        err = read_seq_ch(sc.all, dsp_map.distance, 1l, pos)
        if err ne adsec_error.ok then return, err
        err = read_seq_ch(sc.all, dsp_map.float_dac_value, 1l, curr)
        if err ne adsec_error.ok then return, err

        sys_status.position = pos
        sys_status.current = curr
        sys_status.max_pos = pos
        sys_status.min_pos = pos
        sys_status.position_sd = pos*0
        sys_status.current_sd = curr*0

        err = get_preshaper(sc.all_actuators, presh_curr, /FF)
        if err ne adsec_error.ok then return, err
        sys_status.curr_preshaper_len = presh_curr

        err = get_preshaper(sc.all_actuators, presh_pos)
        if err ne adsec_error.ok then return, err
        sys_status.pos_preshaper_len = presh_pos
        sys_status.fast_diag_data = 0B

;   endif 

    err = get_commands(sc.all, olcmd, /OFFLOADCMD)
    if err ne adsec_error.ok then return, err
    sys_status.offload_cmd = olcmd

    err = get_commands(sc.all, olcurr, /CURR)
    if err ne adsec_error.ok then return, err
    sys_status.offload_curr = olcurr

    err = read_seq_ch(sc.all_actuators, dsp_map.float_adc_value, 1L, voltage)
    if err ne adsec_error.ok then return, err
    sys_status.voltage = voltage

    err = get_commands(sc.all_actuators, commands)
    if err ne adsec_error.ok then return, err
    sys_status.command = commands

    err = read_seq_ch(sc.all_actuators, dsp_map.bias_current, 1L, bias_curr)
    if err ne adsec_error.ok then return, err
    sys_status.bias_current = bias_curr

    err = read_seq_ch(sc.all_actuators, dsp_map.ff_ud_current, 1L, ff_curr)
    if err ne adsec_error.ok then return, err
    sys_status.ff_current = ff_curr

    ;err = read_seq_ch(sc.all_actuators, dsp_map.fullbias_curr, 1L, fb_curr)
    ;if err ne adsec_error.ok then return, err
    ;sys_status.full_bias_current = fb_curr

    err = check_control(sc.all_actuators, ctrl_status)
    if err ne adsec_error.ok then return, err
    sys_status.closed_loop = ctrl_status eq 1B

    err = read_seq_ch(sc.all_actuators, dsp_map.bias_command, 1L, b_cmd)
    if err ne adsec_error.ok then return, err
    sys_status.bias_command = b_cmd

    err = get_gain(sc.all, pgain)
    if err ne adsec_error.ok then return, err
    sys_status.pgain = pgain

    err = get_gain(sc.all, dgain, /SPEED)
    if err ne adsec_error.ok then return, err
    sys_status.dgain = dgain

    sys_status.ctrl_current = float(sys_status.current $
                                    - sys_status.bias_current $
                                    - sys_status.ff_current)

    if err ne adsec_error.ok then return, err
    sys_status.time_stamp1 = systime(1)


    sys_buffer[sys_buffer_idx]=sys_status
    sys_buffer_idx = (sys_buffer_idx+1) mod sys_buffer_len

    return, update_rtdb()

end
