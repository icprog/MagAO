; $Id: update_panels.pro,v 1.13 2008/08/25 15:16:57 labot Exp $
;
;+
; NAME:
;   UPDATE_PANELS
;
; PURPOSE:
;   Update XADSEC GUI with currents values or by reading from the electronics
;
; USAGE:
;   update_panels[, /NO_READ, /OPTIMIZE_CUTS]
;
; INPUT:
;   None.
;
; OUTPUT:
;   None.
;
; KEYWORDS:
;   NO_READ:        no action done in the electronics, only alignment GUI with sys_status
;   OPTIMIZE_CUTS:  optimize color cut in the GUI
;
; HISTORY
;   modifications to match LBT variable types
;   implemented ADC capsens value display
;
;   12 Aug 2004, A. Riccardi (AR)
;     Force display in mN implemented.
;     Empty the grafic buffer before exting.
;   2 Nov 204, D. Zanotti (DZ)
;     adsec:mass,n_true_act,out_radius are changed in adsec_shell: mass,n_true_act,out_radius
;   8 Feb 2005, Marco Xompero(MX)
;     Data type fixed.
;   14 Jun 2005, MX
;     Curr2color call fixed
;   11 july 2006, DZ
;     Canged stdev(obsolate command) with stddev.
;
;-
pro update_panels, NO_READ=no_read, OPTIMIZE_CUTS=optim_cuts

    @adsec_common
    @xadsec_common

    common update_panels_block, uplus_upd, u_upd
;    common pixmap_block, pixmap_pos_num, pixmap_curr_num, pixmap_scale_num
;
;   if n_elements(pixmap_pos_num) eq 0 then begin
;         window, XSIZE=gr.x_tv, YSIZE=gr.y_tv, /PIXMAP, /FREE
;      pixmap_pos_num = !d.window
;      wset, -1
;   endif
;
;   if n_elements(pixmap_curr_num) eq 0 then begin
;         window, XSIZE=gr.x_tv, YSIZE=gr.y_tv, /PIXMAP, /FREE
;      pixmap_curr_num = !d.window
;      wset, -1
;   endif
;
;   if n_elements(pixmap_scale_num) eq 0 then begin
;         window, XSIZE=gr.x_scale_tv, YSIZE=gr.y_tv, /PIXMAP, /FREE
;      pixmap_scale_num = !d.window
;      wset, -1
;   endif

    if (not keyword_set(no_read)) then begin
        err = update_status()
        if err ne adsec_error.ok then begin
            disp_mess, "Error updating the status (err code "+strtrim(err,2)+")."
            return
        endif
    endif

    if keyword_set(optim_cuts) then optimize_cuts

    max_mom2disp = 9.81*adsec_shell.mass/adsec_shell.n_true_act*adsec_shell.out_radius*1e-3  ; [fu*m]
    max_tilt2disp= 1e-6/(rebin(adsec.m_per_pcount[adsec.true_act],1))[0] ; [lu] Peak of tilt at the outer edge
    curr_cuts2disp = gr.curr_cuts
    pos_cuts2disp = gr.pos_cuts

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
            pos2disp_c = sys_status.position - sys_status.command
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

    if gr.tv_pos_type_idx eq n_elements(gr.tv_pos_type_list)-1 then begin
        ; Capsens output case
        case gr.tv_pos_unit_idx of

            0: begin
                ; internal potential units
                pos2disp = pos2disp_c
                ; max_tilt2disp and pos_cuts2disp are already in [pu]
            end

            1: begin
                ; Millivolts
                pos2disp = 1e3*adsec.V_per_pcount*pos2disp_c
                max_tilt2disp= 1e3*adsec.V_per_pcount*max_tilt2disp
                pos_cuts2disp = 1e3*adsec.V_per_pcount*pos_cuts2disp
            end

            2: begin
                ; volts
                pos2disp = adsec.V_per_pcount*pos2disp_c
                max_tilt2disp= adsec.V_per_pcount*max_tilt2disp
                pos_cuts2disp = adsec.V_per_pcount*pos_cuts2disp
            end

            else: begin
                message, "Position unit index not available"
            end
        endcase
        pos_label = gr.tv_sens_unit_list[gr.tv_pos_unit_idx]
    endif else begin
        ; linearized position/command case
        case gr.tv_pos_unit_idx of

            0: begin
                ; internal length units
                pos2disp = pos2disp_c
                ; max_tilt2disp and pos_cuts2disp are already in counts
            end

            1: begin
                ; Microns
                if gr.sub_pos_offset then begin
                    pos2disp = 1e6*adsec.m_per_pcount*pos2disp_c
                    pos_cuts2disp = 1e6*adsec.m_per_pcount*pos_cuts2disp
                endif else begin
                    pos2disp = 1e6*count2meter(pos2disp_c)
                    pos_cuts2disp = 1e6*count2meter(pos_cuts2disp, ACT_LIST=[0,1])
                endelse
                max_tilt2disp = 1e6*adsec.m_per_pcount*max_tilt2disp
            end

            2: begin
                ; nanometers
                if gr.sub_pos_offset then begin
                    pos2disp = 1e9*adsec.m_per_pcount*pos2disp_c
                    pos_cuts2disp = 1e9*adsec.m_per_pcount*pos_cuts2disp
                endif else begin
                    pos2disp = 1e9*count2meter(pos2disp_c)
                    pos_cuts2disp = 1e9*count2meter(pos_cuts2disp, ACT_LIST=[0,1])
                endelse
                max_tilt2disp = 1e9*adsec.m_per_pcount*max_tilt2disp
            end

            else: begin
                message, "Position unit index not available"
            end
        endcase
        pos_label = gr.tv_pos_unit_list[gr.tv_pos_unit_idx]
    endelse

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

    case gr.tv_curr_unit_idx of

        0: begin
            ; Counts
            curr2disp = curr2disp_c
            ; max_mom2disp and curr_cuts2disp are already defined in counts
        end

        1: begin
            ; milliapere
            curr2disp = 1e3*adsec.A_per_ccount*curr2disp_c
            max_mom2disp = 1e3*adsec.A_per_ccount*max_mom2disp
            curr_cuts2disp = 1e3*adsec.A_per_ccount*curr_cuts2disp
        end

        2: begin
            ; milliNewton
            curr2disp = 1e3*adsec.N_per_ccount*curr2disp_c
            max_mom2disp = 1e3*adsec.N_per_ccount*max_mom2disp
            curr_cuts2disp = 1e3*adsec.N_per_ccount*curr_cuts2disp
        end

        else: begin
            message, "Position unit index not available"
        end
    endcase
    curr_label = gr.tv_curr_unit_list[gr.tv_curr_unit_idx]

    f_momentum = [ total(curr2disp[adsec.act_w_curr]*adsec.act_coordinates[1,adsec.act_w_curr]), $
                  -total(curr2disp[adsec.act_w_curr]*adsec.act_coordinates[0,adsec.act_w_curr])]
    f_momentum = f_momentum/max_mom2disp
    mod_f_momentum = sqrt(total(f_momentum^2)) > 1.0
    f_momentum = f_momentum/mod_f_momentum*gr.x_tv/2.0

    x = adsec.act_coordinates[0,adsec.act_w_pos]/adsec_shell.out_radius
    y = adsec.act_coordinates[1,adsec.act_w_pos]/adsec_shell.out_radius
    z = pos2disp[adsec.act_w_pos]
    if n_elements(uplus_upd) eq 0 or n_elements(u_upd) eq 0 then begin

        fit = surf_fit(x, y, z, [1,2,3], COEFF=coeff, UPLUS=temp_uplus_upd, $
                            /ZERN, UMAT=temp_u_upd)
        uplus_upd = temp_uplus_upd
        u_upd = temp_u_upd
    endif else begin
        fit = surf_fit(x, y, z, [1,2,3], COEFF=coeff, UPLUS=uplus_upd, $
                            /ZERN, UMAT=u_upd)
    endelse

    piston = coeff[0]
    tilt_vec = (coeff[1:2])/max_tilt2disp
    mod_tilt_vec = sqrt(total(tilt_vec^2)) > 1.0
    tilt_vec = tilt_vec/mod_tilt_vec*gr.x_tv/2.0

    tr = gr.theta_tv/180.0*!PI
    rot_mat = [[cos(tr), -sin(tr)], $
               [sin(tr),cos(tr)]]
    if gr.x_reflect_tv then rot_mat[*,0]=-rot_mat[*,0]
    f_momentum = rot_mat ## f_momentum
    tilt_vec = rot_mat ## tilt_vec


    temp = gr.position
    draw_rings, temp, pos2color(pos2disp, NOSATURATE=gr.tv_pos_smooth $
                               , CUTS=pos_cuts2disp), SMOOTH=gr.tv_pos_smooth $
                               , LIST=adsec.act_w_pos
    gr.position = temp

    temp = gr.current
    draw_rings, temp, curr2color(curr2disp, NOSATURATE=gr.tv_curr_smooth $
                               , CUTS=curr_cuts2disp), SMOOTH=gr.tv_curr_smooth $
                               , LIST=adsec.act_w_curr
    gr.current = temporary(temp)

    cent_p = [gr.x_tv, gr.y_tv]/2
    color = num2color(sys_status.closed_loop)

    wset, nums.tv_position_num
;    wset, pixmap_pos_num
    tv1, gr.position
    plot_numbers, color, NUM_TYPE=gr.num_type, LIST=adsec.act_w_pos
    plots, [[cent_p-tilt_vec],[cent_p+tilt_vec]], color=gr.cl_off_color, /DEVICE
    plots, cent_p[0]+tilt_vec[0], cent_p[1]+tilt_vec[1], PSYM=2, $
           color=gr.cl_off_color, /DEVICE
    xyouts, 10, 30, "Ave: "+strtrim(piston, 2), /DEVICE
    xyouts, 10, 10, "Rms: "+strtrim(stddev(double(pos2disp[adsec.act_w_pos])), 2), /DEVICE
    xyouts, 10, gr.y_tv-15, "Max: "+strtrim(max(pos2disp[adsec.act_w_pos], MIN=minv), 2), /DEVICE
    xyouts, 10, gr.y_tv-35, "Min: "+strtrim(minv, 2), /DEVICE
;   new_ima = tvrd(TRUE=1)
;   wset, nums.tv_position_num
;   tv, new_ima, TRUE=1

    wset, nums.tv_current_num
;    wset, pixmap_curr_num
    tv1, gr.current
    plot_numbers, color, NUM_TYPE=gr.num_type, LIST=adsec.act_w_curr
    plots, [[cent_p-f_momentum],[cent_p+f_momentum]], color=gr.cl_off_color, /DEVICE
    plots, cent_p[0]+f_momentum[0], cent_p[1]+f_momentum[1], PSYM=2, $
           color=gr.cl_off_color, /DEVICE
    xyouts, 10, 30, "Ave: "+strtrim(mean(double(curr2disp[adsec.act_w_curr])), 2), /DEVICE
    xyouts, 10, 10, "Rms: "+strtrim(stddev(double(curr2disp[adsec.act_w_curr])), 2), /DEVICE
    xyouts, 10, gr.y_tv-15, "Max: "+strtrim(max(curr2disp[adsec.act_w_curr], MIN=minv), 2), /DEVICE
    xyouts, 10, gr.y_tv-35, "Min: "+strtrim(minv, 2), /DEVICE
;   new_ima = tvrd(TRUE=1)
;   wset, nums.tv_current_num
;   tv, new_ima, TRUE=1

    wset, nums.tv_scale_num
;    wset, pixmap_scale_num
    erase
    tv1, rebin(transpose(findgen(gr.y_tv)/(gr.y_tv-1)*(gr.n_act_color-1)),gr.x_scale_tv/3,gr.y_tv), $
        gr.x_scale_tv/3, 0
    xyouts, 15,0, strtrim(pos_cuts2disp(0),2), /DEVICE, CHARSIZE=1.0, ALIGN=0.0, ORIENTATION=90.0
    xyouts, gr.x_scale_tv-10,0,strtrim(curr_cuts2disp(0),2), /DEVICE, CHARSIZE=1.0, ALIGN=0.0, ORIENTATION=90.0
    xyouts, 15,gr.y_tv, strtrim(pos_cuts2disp(1),2), /DEVICE, CHARSIZE=1.0, ALIGN=1.0, ORIENTATION=90.0
    xyouts, gr.x_scale_tv-10,gr.y_tv, strtrim(curr_cuts2disp(1),2), /DEVICE, CHARSIZE=1.0, ALIGN=1.0, ORIENTATION=90.0
    xyouts, 15, gr.y_tv/2, pos_label, /DEVICE, ALIGN=0.5, ORIENTATION=90.0
    xyouts, gr.x_scale_tv-10, gr.y_tv/2, curr_label, /DEVICE, ALIGN=0.5, ORIENTATION=90.0
;   new_ima = tvrd(TRUE=1)
;   wset, nums.tv_scale_num
;   tv, new_ima, TRUE=1
    empty
    wset, -1
    
    widget_control, ids.mirror_cnt_fld, SET_VALUE=STRING(sys_status.mirror_cnt,FORMAT="(I10)")
    widget_control, ids.pending_skip_frame_cnt_fld, SET_VALUE=STRING(sys_status.pending_skip_frame_cnt,FORMAT="(I10)")
    widget_control, ids.safe_skip_frame_cnt_fld, SET_VALUE=STRING(sys_status.safe_skip_frame_cnt,FORMAT="(I10)")
    widget_control, ids.num_fl_crc_err_fld, SET_VALUE=STRING(sys_status.num_fl_crc_err,FORMAT="(I10)")
    widget_control, ids.num_fl_timeout_fld, SET_VALUE=STRING(sys_status.num_fl_timeout,FORMAT="(I10)")

end


