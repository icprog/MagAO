; $Id: xdsp_mem_browser.pro,v 1.4 2004/11/02 16:34:47 marco Exp $
;+
; HISTORY
;   02 Nov 2004, MX
;     Adsec.all changed in sc.all.
;-


pro xdsp_mem_browser_event, event

    @adsec_common 
    common dsp_map_block, dsp_map
    common dsp_mb_block, field_ids, dsp_mb_data

    widget_control, event.id, GET_UVALUE = uvalue

    case uvalue of
        'dsp_mb_var_list': begin

            if dsp_mb_data.selected_var_idx lt 0 then $
                widget_control, dsp_mb_data.update_btn_id, SENSITIVE=1
            dsp_mb_data.selected_var_idx = event.index
            widget_control, dsp_mb_data.addr_text_id, $
                    SET_VALUE=string(dsp_map.(dsp_mb_data.selected_var_idx), FORMAT='("0x",(Z4.4))')
        end

        'dsp_mb_update': begin
            if dsp_mb_data.selected_var_idx ge 0 then begin
                dsp_addr = dsp_map.(dsp_mb_data.selected_var_idx)
                widget_control, dsp_mb_data.pm_ch1_id, GET_VALUE=btn_vals
                ;if btn_vals[0] then set_pm=1B else set_pm=0B
                if btn_vals[1] then dsp_addr = dsp_addr+dsp_const.ch1_offset
                err = read_seq_dsp(sc.all_dsp, dsp_addr, 1, buffer)
                for dsp_count=0,adsec.n_dsp-1 do $
                    widget_control, field_ids[dsp_count], SET_VALUE=strtrim(buffer(dsp_count),2)
            end
        end

        'dsp_mb_cancel': begin
            widget_control, event.top, /DESTROY
        end

        'pm_ch1': begin
        end

        ;else: begin
        ;end
    endcase

end



pro xdsp_mem_browser, GROUP=group, MODAL=modal

    @adsec_common 
    common dsp_mb_block, field_ids, dsp_mb_data

    dsp_mb_data = {selected_var_idx: -1L, update_btn_id: 0L, addr_text_id: 0L, pm_ch1_id: 0L}

    root_dsp_mb = widget_base(TITLE = 'DSP memory browser', /COLUMN, MODAL=modal)
    temp_base = widget_base(root_dsp_mb, /ROW)
    dsp_mb_list_base = widget_base(temp_base, /COLUMN)
    dsp_mb_base = widget_base(temp_base, /COLUMN, /FRAME)
    buttons_dsp_mb = widget_base(root_dsp_mb, /ROW)

    dsp_mb_data.selected_var_idx = -1
    var_names = tag_names(dsp_map)
    dummy = widget_list(dsp_mb_list_base, VALUE=var_names, UVALUE="dsp_mb_var_list", YSIZE=9)
    dsp_mb_data.pm_ch1_id = CW_BGROUP(dsp_mb_list_base, ['Program memory','Add ch1 offset'] $
                                      , /NONEXCLUSIVE, UVALUE='pm_ch1')
    addr_base=widget_base(dsp_mb_list_base, /ROW)
    dummy = widget_label(addr_base, VALUE="Addr.:")
    dsp_mb_data.addr_text_id = widget_text(addr_base, XSIZE=2+4+1)

    fields_per_row = 14
    n_field_rows = (adsec.n_dsp-1)/fields_per_row+1
    field_row_base = lonarr(n_field_rows)
    field_ids = lonarr(adsec.n_dsp)

    for row_count=0,n_field_rows-1 do $
        field_row_base[row_count] = widget_base(dsp_mb_base,/ROW)

    for dsp_count=0,adsec.n_dsp-1 do begin
        field_base = widget_base(field_row_base[dsp_count/fields_per_row],/COLUMN)
        dummy = widget_label(field_base, VALUE="DSP #"+string(dsp_count,format='(I3.3)'))
        field_ids[dsp_count] = widget_text(field_base,XSIZE=6+1)
    endfor


    dsp_mb_data.update_btn_id = widget_button(buttons_dsp_mb, VALUE='Update', UVALUE='dsp_mb_update')
    dummy = widget_button(buttons_dsp_mb, VALUE='Cancel', UVALUE='dsp_mb_cancel')

    widget_control, root_dsp_mb, /REALIZE

    widget_control, dsp_mb_data.update_btn_id, SENSITIVE=0

    xmanager, 'xdsp_mem_browser', root_dsp_mb, GROUP_LEADER=group, /NO_BLOCK

end

