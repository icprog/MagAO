; $Id: xtemp_anal_setup.pro,v 1.10 2008/02/05 11:05:08 labot Exp $
;+
; HYSTORY
;   21 Jul 2004:
;       modifications to match LBT data type specifications
;       buffer managed using get_ch_data_buf function
;   02 Nov 2004, MX
;       Adsec.all changed in sc.all.
;   30 Nov 2004, AR
;       Some slider initialization data modified
;   01 Dec 2004, AR
;       Decimation correctly handled
;   07 Feb 2007 MX
;       LUT of variable deleted and name refurbished.
;-
pro xtemp_update_fields

    @adsec_common
    common tanal_block, field_ids

    widget_control, field_ids.decimation_id, GET_VALUE=value
    decimation = value-1L
    widget_control, field_ids.ndata2store_id, GET_VALUE=value
    ndata2store = value

    str_unit = ["us", "ms", "s", "min"]
    str_mult = [1e-6, 1e-3, 1.0, 60.0]
    sampling_time = (decimation+1L)*adsec.sampling_time
    range_time = ndata2store*sampling_time
    str_idx = optim_unit([sampling_time], str_unit, str_mult)
    widget_control, field_ids.sampling_time_id, $
        SET_VALUE=strtrim(sampling_time/str_mult[str_idx])+" "+str_unit[str_idx]
    str_idx = optim_unit([range_time], str_unit, str_mult)
    widget_control, field_ids.range_time_id, $
        SET_VALUE=strtrim(range_time/str_mult[str_idx])+" "+str_unit[str_idx]
end

pro xtemp_anal_setup_event, event

    @adsec_common
    common tanal_block, field_ids

    widget_control, event.id, GET_UVALUE = uvalue

    case uvalue of

        'freeze': begin

            old_ovs = rtr.oversampling_time
            err = set_diagnostic(over=0.0, MASTER=0)
            if err ne adsec_error.ok then begin
                message, "ERROR ON DIAGNOSTIC SETTING (error code: "+strtrim(err,2)+").", $
                         CONT=(sc.debug eq 0B)
                return
            endif
            wait, old_ovs > 0.01
            widget_control, field_ids.decimation_id, GET_VALUE=value
            decimation = value-1L
            widget_control, field_ids.ndata2store_id, GET_VALUE=value
            ndata2store = value
            widget_control, field_ids.lin_pos_id, GET_VALUE=value
            if value[0] then voltage_output=0L else voltage_output=1L

            if voltage_output then begin
                addr_list = dsp_map.float_adc_value
            endif else begin
                addr_list = dsp_map.distance
            endelse
            addr_list = [addr_list, dsp_map.float_dac_value]
            buf_num_list = [0,1]
            err = get_ch_data_buf(addr_list, uint(ndata2store), data, DEC=uint(decimation))
;            err = set_IO_buffer(sc.all_dsp, buf_num_list, addr_list, $
;                                ndata2store, dsp_map.dataIO_0, 1, DEC=decimation)
            if err ne adsec_error.ok then begin
                message, "error in setting the dataIO buffers (error code: "+strtrim(err,2)+").", $
                         CONT=(sc.debug eq 0B)
                return
            endif
            if old_ovs gt 0 then begin
                err = set_diagnostic(over=old_ovs, /MASTER)
                if err ne adsec_error.ok then begin
                    disp_mess, 'TEMPORAL ANALISYS FAILED, MIRROR UNPROTECTED!', /APPEND
                    return
                endif
            endif

;            err = write_same_dsp(sc.all_dsp, dsp_map.dataIO_0, 1)
;            if err ne adsec_error.ok then begin
;                message, "error in clearing the trigger value (error code: "+strtrim(err,2)+").", $
;                         CONT=(sc.debug eq 0B)
;                return
;            endif

;            err = enable_IO_buffer(sc.all_dsp, buf_num_list)
;            if err ne adsec_error.ok then begin
;                message, "error in enabling the dataIO buffers (error code: "+strtrim(err,2)+").", $
;                         CONT=(sc.debug eq 0B)
;                return
;            endif
;            ; wait for the end of the acquisition
;            wait, adsec.sampling_time*(decimation+1L)*ndata2store*1.10 > 0.01

;            old_idx = 0
;            break = 0B
;            repeat begin
;                err = read_seq_dsp(sc.all_dsp, dsp_map.dataIO_process, 1, buffer)
;                if err ne adsec_error.ok then begin
;                    message, "error in enabling the dataIO buffers (error code: "+strtrim(err,2)+").", $
;                             CONT=(sc.debug eq 0B)
;                    return
;                endif
;                if total(buffer and 2^buf_num_list) ne 0 then begin
;                    err = read_seq_dsp(sc.all_dsp, dsp_map.dataIO_curridx0, 1, buffer)
;                    if err ne adsec_error.ok then begin
;                        message, "error in reading the dataIO buffer indexes (error code: "+strtrim(err,2)+").", $
;                                 CONT=(sc.debug eq 0B)
;                        return
;                    endif
;                    idx = min(buffer)
;                    if old_idx eq idx then begin
;                        message, "The buffer storing seems to be freezed.", $
;                                 CONT=(sc.debug eq 0B)
;                        return
;                    endif
;                    if idx lt ndata2store then begin
;                        wait, adsec.sampling_time*(decimation+1L)*(ndata2store-idx) > 0.01
;                    endif
;                endif else begin
;                    break = 1B
;                endelse
;            endrep until break


;            data = intarr(ndata2store, 2, adsec.n_actuators)
;            for i_dsp=0,adsec.n_dsp-1 do begin
;                err = read_seq_dsp(i_dsp, dsp_map.dataIO_0, ndata2store, buffer)
;                data[*, 0, i_dsp*2] = buffer
;                err = read_seq_dsp(i_dsp, dsp_map.dataIO_1, ndata2store, buffer)
;                data[*, 1, i_dsp*2] = buffer
;                err = read_seq_dsp(i_dsp, dsp_map_pm.dataIO_3, ndata2store, buffer, /SET_PM)
;                data[*, 0, i_dsp*2+1] = buffer
;                err = read_seq_dsp(i_dsp, dsp_map_pm.dataIO_4, ndata2store, buffer, /SET_PM)
;                data[*, 1, i_dsp*2+1] = buffer
;            endfor

            tanal_panel, decimation, ndata2store, voltage_output, data, group=event.top

        end

        'tanal_setup_cancel': begin

            widget_control, event.top, /DESTROY
        end

        'saving_decimation_slider': begin
            xtemp_update_fields
        end

        'ndata2store_slider': begin
            xtemp_update_fields
        end

        'lin_pos': begin
        end

    endcase

end



pro xtemp_anal_setup, GROUP=group

    @adsec_common
    common tanal_block, field_ids

    root_tanal_setup = widget_base(TITLE = 'Temporal analisys', /COLUMN, /MODAL, GROUP=group)

    buttons_tanal_setup = widget_base(root_tanal_setup, /ROW)

    field_ids = {   decimation_id:  0L, $
                    ndata2store_id: 0L, $
                    sampling_time_id:   0L, $
                    range_time_id:  0L, $
                    lin_pos_id:0L}

    field_ids.decimation_id = cw_lslider(root_tanal_setup, $
                        TITLE="Decimation", $
                        /DRAG, $
                        /EDIT, $
                        /FRAME, $
                        SCROLL=5, $
                        MINIMUM=1L, $
                        MAXIMUM=100000L, $
                        VALUE=sys_status.saving_decimation+1L, $
                        UVALUE="saving_decimation_slider")

    field_ids.sampling_time_id = cw_field(root_tanal_setup, $
                        TITLE="Sampling time", $
                        /NOEDIT)

    field_ids.ndata2store_id = cw_lslider(root_tanal_setup, $
                        TITLE="Num. of data values to store", $
                        /DRAG, $
                        /EDIT, $
                        /FRAME, $
                        SCROLL=10, $
                        MINIMUM=2L, $
                        MAXIMUM='FFFF'XL, $
                        VALUE=16384L, $
                        UVALUE="ndata2store_slider")

    field_ids.range_time_id = cw_field(root_tanal_setup, $
                        TITLE="Sampling range", $
                        /NOEDIT)

    field_ids.lin_pos_id = cw_bgroup(root_tanal_setup, ["Linearized position"] $
        , /COLUMN $
        , /NONEXCLUSIVE $
        , UVALUE='lin_pos')

    dummy = widget_button(buttons_tanal_setup, VALUE='Freeze now', UVALUE='freeze')
    dummy = widget_button(buttons_tanal_setup, VALUE='Cancel', UVALUE='tanal_setup_cancel')

    widget_control, field_ids.lin_pos_id, SET_VALUE=[1]
    xtemp_update_fields

    widget_control, root_tanal_setup, /REALIZE

    xmanager, 'xtemp_anal_setup', root_tanal_setup, GROUP_LEADER=group

end

