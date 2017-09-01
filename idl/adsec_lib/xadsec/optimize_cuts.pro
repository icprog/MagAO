; $Id: optimize_cuts.pro,v 1.5 2004/07/21 14:16:34 riccardi Exp $
;+
; HISTORY
;  21 Jul 2004: A. Riccardi
;    gr.pos_cuts and gr.curr_cuts are used now.
;    ADC sensor output is handled correctly.
;-
pro optimize_cuts

@adsec_common

    case gr.tv_pos_type_idx of

        0: begin
            ; Positions
            if gr.sub_pos_offset then begin
                pos2disp_c = sys_status.position-sys_status.offset_pos
            endif else begin
                pos2disp_c = sys_status.position
            endelse
        end

        1: begin
            ; Commands
            if gr.sub_pos_offset then begin
                pos2disp_c = sys_status.command-sys_status.offset_comm
            endif else begin
                pos2disp_c = sys_status.command
            endelse
        end

        2: begin
            ; Max-min position
            pos2disp_c = sys_status.position_sd
        end

        3: begin
            ; Position error
            pos2disp_c = sys_status.position-sys_status.command
        end

        4: begin
            ; Sensor output
            if gr.sub_pos_offset then begin
                pos2disp_c = sys_status.voltage-sys_status.offset_volt
            endif else begin
                pos2disp_c = sys_status.voltage
            endelse
        end

        else: begin
            message, "Position type index not available"
        end
    endcase


    case gr.tv_curr_type_idx of

        0: begin
            ; current
            if gr.sub_curr_offset then begin
                curr2disp_c = sys_status.current-sys_status.offset_curr
            endif else begin
                curr2disp_c = sys_status.current
            endelse
        end

        1: begin
            ; bias current
            if gr.sub_curr_offset then begin
                curr2disp_c = sys_status.bias_current-sys_status.offset_bias_curr
            endif else begin
                curr2disp_c = sys_status.bias_current
            endelse
        end

        2: begin
            ; Conrol current
            if gr.sub_curr_offset then begin
                curr2disp_c = sys_status.ctrl_current-sys_status.offset_ctrl_curr
            endif else begin
                curr2disp_c = sys_status.ctrl_current
            endelse
        end

        3: begin
            ; Conrol current
            if gr.sub_curr_offset then begin
                curr2disp_c = sys_status.ff_current-sys_status.offset_ff_curr
            endif else begin
                curr2disp_c = sys_status.ff_current
            endelse
        end

        else: begin
            message, "Current type index not available"
        end
    endcase

pos_act_list = adsec.act_w_pos
;idx = where(sys_status.position ne 32767, count)
;if count ne 0 then begin
;    is_empty = intersection(pos_act_list, idx, new_idx)
;    if not is_empty then pos_act_list = new_idx
;endif

cur_act_list = adsec.act_w_curr

min_pos = min(pos2disp_c[pos_act_list], MAX=max_pos)
min_curr = min(curr2disp_c[cur_act_list], MAX=max_curr)
dpos  = gr.extra_ct*(max_pos-min_pos)/2
dcurr = gr.extra_ct*(max_curr-min_curr)/2

gr.pos_cuts=[(min_pos-dpos), (max_pos+dpos)]
gr.curr_cuts=[(min_curr-dcurr), (max_curr+dcurr)]

end


