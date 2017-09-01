; $Id: lbt_xadsec.pro,v 1.2 2008/08/01 07:59:07 labot Exp $

;+
; lbt_xadsec
;
;   Engeneering grafical interface to adaptive secondary
;
;   CATEGORY:
;    Supervisor IDL function.                                                   ;
;
;   CALLING SEQUENCE:
;    err = LBT_XADSEC(GROUP = group, MODAL = modal, ASK=ASK, COMM_ONLY=comm_only)
;
;   INPUTS:
;    None.
;
;   OUTPUTS:
;    err : Error code.
;
;   RESTRICTIONS:
;    None.
;
; HISTORY
;
; ?? ??? ????: written by A. Riccardi
;
; 31 Jan 2004: by AR
;   *changed path from which restoring the DSP program
;
; 21 Jul 2004: by AR
;   *implemented an additional menu for the choice of the units for
;    the capsens output
; 02 Nov 2004, MX
;    Adsec.all changed in sc.all.
; 30 Nov 2004, AR & MX
;    reload_wave_param, reload_ff_matrix and reload_ctrl_filters
;    events added and handled.
;    default configuration filename changed (filters.txt).
;    Wait depending on preshaper setting added.
; 20 Dic 2004, MX
;    bug on different wave files fixed.
; Dicember & January 2005, Daniela Zanotti(DZ)
;    Modify to a Supervisor IDL function.
; 21 November 2007, DZ
;    Updated the full procedure.Introduced the use of master diagnostic.   
; 
;-
PRO lbt_xadsec_event, event
                                ; This is the event handler for adsec control.

@adsec_common
@xadsec_common

if (event.id eq event.handler) then begin ; A top level base timer event

    if (sys_status.cont_update) then begin
        widget_control, event.id, TIMER=sys_status.next_update
       if sys_status.monitor then prev_sys_status = sys_status
       err = update_status()
        if err ne adsec_error.ok then begin
            disp_mess, '... error on updating panels.', /APPEND
            return
        endif
        update_panels, /NO_READ
        if sys_status.monitor then proc_monitor, prev_sys_status
    endif

endif else begin

    WIDGET_CONTROL, event.id, GET_UVALUE = uvalue
    CASE uvalue OF

        'exit': begin
            if (total(sys_status.current ne 0)) then dummy=proc_sysdown()
            WIDGET_CONTROL, event.top, /DESTROY
        end

        'rip': begin
             err = rip(/XADSEC)
        end

        'select_all': begin
            selected_acts = lindgen(adsec.n_actuators)
            disp_mess, 'All actuators are selected.'
        end

        'startup_sys': begin
            err = proc_startup( /NO_OPEN)
            if err ne adsec_error.ok then begin
                disp_mess, 'Startup procedure interrupted!', /APPEND
                sys_status.sys_is_open = 0B
                return
            endif
            sys_status.sys_is_open = 1B
            err = update_status()
                if err ne adsec_error.ok then begin
                    disp_mess, '... error on updating panels.', /APPEND
                    return
                endif
            update_panels, /NO_READ

         end

        'load_program': begin
            ;REMOVED
        end


        'reload_ff_matrix': begin
            default_dir = filepath(ROOT=adsec_path.data, "")
            default_filename = "ff_matrix.sav"
            filename = dialog_pickfile(FILTER="*.sav", /MUST_EXIST $
                                      , FILE=default_filename, PATH=default_dir $
                                      , TITLE="Select file with FF matrix to load")
            if filename eq "" then begin
                disp_mess, 'No FF matrix reloaded!'
                return
            endif else begin
                disp_mess, 'FF initialization...'
                disp_mess, 'FF file: '+filename, /APPEND
                err = init_adsec_ff(FF_FILENAME=filename)
                if err ne adsec_error.ok then begin
                    disp_mess, 'FF initialization failed!', /APPEND
                    return
                endif
                old_ovs = rtr.oversampling_time
                err = set_diagnostic(over=0.0)
                if err ne adsec_error.ok then begin
                    disp_mess, 'FF initialization failed, MIRROR UNPROTECTED!', /APPEND
                    return
                endif
                disp_mess, 'Uploading FF matrix...', /APPEND
                err = send_ff_matrix(adsec.ff_matrix)
                if err ne adsec_error.ok then begin
                    disp_mess, 'FF uploading failed!', /APPEND
                    return
                endif
                err = set_diagnostic(over=old_ovs)
                if err ne adsec_error.ok then begin
                    disp_mess, 'FF initialization failed, MIRROR UNPROTECTED!', /APPEND
                    return
                endif
                disp_mess, '... done.', /APPEND
            endelse
        end


        'reload_ctrl_filters': begin
            default_dir = filepath(ROOT=adsec_path.data, "")
            default_filename = "tfl_data.sav"
            filename = dialog_pickfile(FILTER="*.sav", /MUST_EXIST $
                                      , FILE=default_filename, PATH=default_dir $
                                      , TITLE="Select filter definition file (cancel for defaults)")

            disp_mess, 'Control filters initialization...'
            if filename eq "" then begin
                disp_mess, 'Default filters will be reloaded!',/APPEND
                undefine, filename
            endif
            config_filename = filepath(ROOT=adsec_path.conf, "filters.txt")
            err = init_adsec_filters(config_filename, TFL=filename)
            if err ne adsec_error.ok then begin
                disp_mess, 'Control filters initialization failed!', /APPEND
                return
            endif
            disp_mess, 'Uploading control filters...', /APPEND
            err = send_ctrl_coeff(adsec.act_w_cl)
            if err ne adsec_error.ok then begin
                disp_mess, 'Control filters uploading failed!', /APPEND
                return
            endif
            disp_mess, '... done.', /APPEND
        end

        'reload_wave_param': begin
            default_dir = filepath(ROOT=adsec_path.conf, "")
            default_filename = "wave.txt"
            filename = dialog_pickfile(FILTER="*.txt", /MUST_EXIST $
                                      , FILE=default_filename, PATH=default_dir $
                                      , TITLE="Select filter definition file (cancel for defaults)")

            disp_mess, 'Reference wave paramenters initialization...'
            if filename eq "" then begin
                disp_mess, 'Default wave parameters will be reloaded!',/APPEND
                undefine, filename
            endif
            err = init_adsec_wave(filename, /VER)
            if err ne adsec_error.ok then begin
                disp_mess, 'Reference wave parameters initialization failed!', /APPEND
                return
            endif
            disp_mess, 'Uploading reference wave paramenters...', /APPEND
            err = send_wave()
            if err ne adsec_error.ok then begin
                disp_mess, 'Reference wave parameters initialization failed!', /APPEND
                return
            endif
            disp_mess, '... done.', /APPEND

            disp_mess, 'Uploading the calibration parameters..', /APPEND
            err =  init_adsec_cals()
            if err ne adsec_error.ok then begin
                disp_mess, 'Calibration failed!', /APPEND
                return
            endif
            disp_mess, '... done.', /APPEND

            disp_mess, 'Computing and uploading the linearization coefficents...', /APPEND
             err = send_linearization()
             if err ne adsec_error.ok then begin
                 disp_mess, '... error on uploading the linearization coeffs.', /APPEND
                 return
             endif
             disp_mess, '... done.', /APPEND

 
        end

        'set_cuts': begin
            xcuts_setup, GROUP=ids.base_root_id
            update_panels, /NO_READ
        end

        'optimize_cuts': begin
            optimize_cuts
            update_panels, /NO_READ
        end

        'update_panels': begin
            err = update_status()
                if err ne adsec_error.ok then begin
                     disp_mess, '... error on updating panels.', /APPEND
                 return
                endif
            update_panels, /NO_READ
        end


        "offset_pos": begin
            ;err=update_status()
            sys_status.offset_pos=sys_status.position
            sys_status.offset_volt=sys_status.voltage
            sys_status.offset_comm=sys_status.command
            sys_status.offset_curr=sys_status.current
            sys_status.offset_bias_curr=sys_status.bias_current
            sys_status.offset_ctrl_curr=sys_status.ctrl_current
            sys_status.offset_ff_curr=sys_status.ff_current
            widget_control, ids.tv_pos_offset_id, SENSITIVE=1
            widget_control, ids.tv_curr_offset_id, SENSITIVE=1
            widget_control, ids.tv_pos_offset_id, GET_VALUE=sub_pos_offset
            widget_control, ids.tv_curr_offset_id, GET_VALUE=sub_curr_offset
            gr.sub_pos_offset = sub_pos_offset[0]
            gr.sub_curr_offset = sub_curr_offset[0]
            update_panels, /NO_READ
        end

        "reset_offset_pos": begin
            sys_status.offset_pos=0
            sys_status.offset_pos=0
            sys_status.offset_volt=0
            sys_status.offset_comm=0
            sys_status.offset_curr=0
            sys_status.offset_bias_curr=0
            sys_status.offset_ctrl_curr=0
            sys_status.offset_ff_curr=0
            widget_control, ids.tv_pos_offset_id, SENSITIVE=0
            widget_control, ids.tv_curr_offset_id, SENSITIVE=0
            gr.sub_pos_offset = 0B
            gr.sub_curr_offset = 0B
            update_panels, /NO_READ
        end

        "tv_pos_offset": begin
            widget_control, event.id, GET_VALUE=is_set
            gr.sub_pos_offset = is_set[0]
            update_panels, /NO_READ
        end

        "tv_curr_offset": begin
            widget_control, event.id, GET_VALUE=is_set
            gr.sub_curr_offset = is_set[0]
            update_panels, /NO_READ
        end

        "tv_pos_smooth": begin
            widget_control, event.id, GET_VALUE=is_set
            gr.tv_pos_smooth = is_set[0]
            gr.position = 0
            update_panels, /NO_READ
        end

        "tv_curr_smooth": begin
            widget_control, event.id, GET_VALUE=is_set
            gr.tv_curr_smooth = is_set[0]
            gr.current=0
            update_panels, /NO_READ
        end

        'start_update': begin
            sys_status.cont_update=1B
            sys_status.monitor=0B
            widget_control, event.handler, TIMER=sys_status.next_update
            disp_mess, "Continuous panel updating started"
        end

        'start_monitor': begin
            sys_status.cont_update=1B
            sys_status.monitor=1B
            widget_control, event.handler, TIMER=sys_status.next_update
            disp_mess, "Continuous panel updating started"
        end

        'stop_update': begin
            sys_status.cont_update=0B
            sys_status.monitor=0B
            disp_mess, "Continuous panel updating stopped"
        end

        'temporal_analisys': begin
            xtemp_anal_setup, GROUP=ids.base_root_id
        end

        'mirror_set': begin
            err = xmirror_set()
            if err eq adsec_error.ok then begin
              err = update_status()
                if err ne adsec_error.ok then begin
                    disp_mess, '... error on updating panels.', /APPEND
                    return
                endif
             update_panels, /NO_READ
            endif else begin
                disp_mess, "Mirror setting failed!", /APPEND
            endelse
        end

        'stop_control': begin
            disp_mess, "Stopping control on all actuators..."
            err = stop_control(sc.all_actuators)
            if (err ne 0L) then begin
                disp_mess, "Failed!", /APPEND
                return
            endif
            sys_status.closed_loop = 0B
            disp_mess, "Done!", /APPEND
            err = update_status()
            if err ne adsec_error.ok then begin
                disp_mess, '... error on updating panels.', /APPEND
                return
            endif
            update_panels, /NO_READ

        end

        'set_same_curr': begin
            xcurr_setup, /SAME, GROUP=ids.base_root_id
            selected_acts = [-1]
        end

        'clear_dacs': begin
            disp_mess, "Clearing all DACs..."
            err = clear_dacs(sc.all_dsp)
            if (err ne 0L) then begin
                disp_mess, "Failed!", /APPEND
                return
            endif
            disp_mess, "Done!", /APPEND
            dum = time_preshaper(FF=tff) 
            wait,max(tff)
            err = update_status()
            if err ne adsec_error.ok then begin
                disp_mess, '... error on updating panels.', /APPEND
                return
            endif
            update_panels, /NO_READ
        end

        'tv_position': begin
            if (event.type eq 0) then begin

                                ; start position setting panel if true
                if(proc_pos_click(event)) then begin
                    xpos_setup, selected_acts, GROUP=ids.base_root_id
                    selected_acts = [-1]
                endif
            endif
        end

        'tv_current': begin
            if (event.type eq 0) then begin
                ; start current setting panel if true
                if(proc_pos_click(event)) then begin
                    xcurr_setup, selected_acts, GROUP=ids.base_root_id
                    selected_acts = [-1]
                endif
            endif
        end
        "tv_pos_type": begin
            sens_idx = n_elements(gr.tv_pos_type_list)-1
            if ((gr.tv_pos_type_idx eq sens_idx) or (event.index eq sens_idx)) $
               and (gr.tv_pos_type_idx ne event.index) then $
                change_list=1B $
            else $
                change_list=0B
            gr.tv_pos_type_idx = event.index
            if change_list then begin
                if gr.tv_pos_type_idx eq sens_idx then begin
                    ; sensor output display
                    widget_control, ids.tv_pos_unit_id, SET_VALUE=gr.tv_sens_unit_list
                endif else begin
                    ; position/command display
                    widget_control, ids.tv_pos_unit_id, SET_VALUE=gr.tv_pos_unit_list
                endelse
                gr.tv_pos_unit_idx = 0
            endif
            update_panels, /NO_READ
        end

        "tv_curr_type": begin
            gr.tv_curr_type_idx = event.index
            update_panels, /NO_READ
        end

        "tv_pos_unit": begin
            idx = event.index
            gr.tv_pos_unit_idx = idx
            update_panels, /NO_READ
        end

        "tv_curr_unit": begin
            gr.tv_curr_unit_idx = event.index
            update_panels, /NO_READ
        end

        "view_opt": begin
            xview_opt_setup, GROUP=event.top
        end

        "act_label_dsp": begin
            gr.num_type = 0
            update_panels, /NO_READ
        end

        "act_label_mirror": begin
            gr.num_type = 1
            update_panels, /NO_READ
        end

        "act_label_warren": begin
            gr.num_type = 2
            update_panels, /NO_READ
        end

        "act_label_no_numbers": begin
            gr.num_type = -1
            update_panels, /NO_READ
        end

        'xdsp_mem_browser': begin
            xdsp_mem_browser, GROUP=event.top
        end

        'xloadct': begin
            xloadct, GROUP=event.top
        end

        'xpalette': begin
            xpalette, GROUP=event.top
        end

        else: begin
            disp_mess, 'The event ' + uvalue + ' is non handled at the moment.'
        end

    endcase
endelse
END




pro lbt_xadsec, GROUP = group, MODAL = modal, ASK=ASK, COMM_ONLY=comm_only

    @adsec_common
    @xadsec_common

    ; Global variable initialization is done in the startup.pro not needed here
    ;init_gvar

    ;Check if there is another xadsec instance opened.
    if xregistered('lbt_xadsec') gt 0 then begin
        print, "There is another xadsec instance opened. Use it!"
        return
    endif

    ; Local variable initialization
    mes_field_xchars = 50
    mes_field_lines = 20;8

    ids = {base_root_id    : 0L, $
           tv_position_id  : 0L, $
           tv_scale_id     : 0L, $
           tv_current_id   : 0L, $
           tv_pos_type_id  : 0L, $
           tv_pos_unit_id  : 0L, $
           tv_curr_type_id : 0L, $
           tv_curr_unit_id : 0L, $
           message_field_id: 0L, $
           tv_pos_offset_id: 0L, $
           tv_curr_offset_id:0L, $
           tv_pos_smooth_id: 0L, $
           tv_curr_smooth_id:0L}

    nums = {tv_position_num:0L, $
            tv_scale_num   :0L, $
            tv_current_num :0L}

    ; initialize position and current drawing panels
    loadct, 12


    ids.base_root_id = widget_base(TITLE = 'Adaptive Secondary Control Interface' $
                        , /COLUMN, MBAR=base_bar, MODAL=modal)
    WIDGET_CONTROL, /MANAGED, ids.base_root_id

    ; menu bar initialization
    init_menu, base_bar

    ; base of the position and control panels
    base = widget_base(ids.base_root_id, /ROW)

    ; position control base
    base_position = widget_base(base, /COLUMN, /FRAME)
    ; position control base
    base_scale = widget_base(base, /COLUMN, /FRAME)
    ; current control base
    base_current = widget_base(base, /COLUMN, /FRAME)

    ; position control definitions
    dummy = WIDGET_LABEL(base_position, VALUE = 'Position Control Panel')
    ids.tv_position_id = WIDGET_DRAW(base_position, $
        /BUTTON_EVENTS, $       ;generate events when buttons pressed
        /FRAME, $
        UVALUE = 'tv_position', $
        XSIZE = gr.x_tv, $
        YSIZE = gr.y_tv, RETAIN=2)
    sub_base = widget_base(base_position, /COL)
    ids.tv_pos_type_id = widget_droplist(sub_base, $
                                         VALUE=gr.tv_pos_type_list, $
                                         UVALUE="tv_pos_type", $
                                         TITLE="Values to display:")
    ids.tv_pos_unit_id = widget_droplist(sub_base, $
                                         VALUE=gr.tv_pos_unit_list, $
                                         UVALUE="tv_pos_unit", $
                                         TITLE="Units:")
    ids.tv_pos_offset_id = cw_bgroup(sub_base, ["Subtract the offset"], $
                                     UVALUE="tv_pos_offset", $
                                     SET_VALUE = [1], $
                                     /NONEXCLUSIVE)
    ids.tv_pos_smooth_id = cw_bgroup(sub_base, ["Smooth"], $
                                     UVALUE="tv_pos_smooth", $
                                     SET_VALUE = [0], $
                                     /NONEXCLUSIVE)




    dummy = WIDGET_LABEL(base_scale, VALUE = 'Color scale')
    ids.tv_scale_id = WIDGET_DRAW(base_scale, /FRAME, XSIZE = gr.x_scale_tv, YSIZE = gr.y_tv)

    dummy = WIDGET_LABEL(base_current, VALUE = 'Current Control Panel')
    ids.tv_current_id = WIDGET_DRAW(base_current, $
        /BUTTON_EVENTS, $       ;generate events when buttons pressed
        /FRAME, $
        UVALUE = 'tv_current', $
        XSIZE = gr.x_tv, $
        YSIZE = gr.y_tv)
    sub_base = widget_base(base_current, /COL)
    ids.tv_curr_type_id = widget_droplist(sub_base, $
                                         VALUE=gr.tv_curr_type_list, $
                                         UVALUE="tv_curr_type", $
                                         TITLE="Values to display:")
    ids.tv_curr_unit_id = widget_droplist(sub_base, $
                                         VALUE=gr.tv_curr_unit_list, $
                                         UVALUE="tv_curr_unit", $
                                         TITLE="Units:")
    ids.tv_curr_offset_id = cw_bgroup(sub_base, ["Subtract the offset"], $
                                     UVALUE="tv_curr_offset", $
                                     SET_VALUE = [1], $
                                     /NONEXCLUSIVE)
    ids.tv_curr_smooth_id = cw_bgroup(sub_base, ["Smooth"], $
                                      UVALUE="tv_curr_smooth", $
                                      SET_VALUE = [0], $
                                      /NONEXCLUSIVE)


    dummy = widget_label(ids.base_root_id, VALUE='Messages:')
    ids.message_field_id = WIDGET_TEXT(ids.base_root_id, $
        VALUE = ' ', $
        XSIZE = mes_field_xchars, $
        YSIZE = mes_field_lines, $
        SCROLL = mes_field_lines-1, $
        /FRAME)

;   button example
;   frun_id = WIDGET_BUTTON(base_right_mid, $
;           UVALUE = 'frun', $
;           VALUE = 'Free run')

    ; Realize the widgets:
    WIDGET_CONTROL, ids.base_root_id, /REALIZE

    widget_control, ids.tv_position_id, GET_VALUE=tv_position_num
    widget_control, ids.tv_scale_id   , GET_VALUE=tv_scale_num
    widget_control, ids.tv_current_id , GET_VALUE=tv_current_num
    nums.tv_position_num = tv_position_num
    nums.tv_scale_num    = tv_scale_num
    nums.tv_current_num  = tv_current_num


;    ans = dialog_message(['Do you want to initialize the system?', $
;                          'Click on "No" to skip initialization'], $
;                          /QUESTION)
;
;    if strlowcase(ans) eq 'yes' then begin
    if sc.busy_comm then no_open=1 else no_open=0
    if keyword_set(ask) then begin 
        err = proc_startup(no_open=no_open)
    endif else begin
        err = proc_startup(/no_ask, COMM =comm_only,no_open=no_open)
     endelse

        if err ne adsec_error.ok then begin
            disp_mess, 'Startup procedure interrupted!', /APPEND
            sys_status.sys_is_open = 0B
        endif else begin
            sys_status.sys_is_open = 1B
            err = update_status()
                if err ne adsec_error.ok then begin
                    disp_mess, '... error on updating panels.', /APPEND
                    return
                endif
            update_panels, /NO_READ
       endelse
;    endif



;   activate or unactivate the menu items
    widget_control, ids.tv_pos_offset_id, SENSITIVE=0
    widget_control, ids.tv_curr_offset_id, SENSITIVE=0
    widget_control, item_ids.load_program_id, SENSITIVE=0
    widget_control, item_ids.set_mirror_id, SENSITIVE=0
;   widget_control, close_sys_id, SENSITIVE=0
;   widget_control, frun_id, SENSITIVE=0
;   widget_control, stop_id, SENSITIVE=0
;   widget_control, shot_id, SENSITIVE=0
;   widget_control, set_tv_a_id, SENSITIVE=0
;    widget_control, sys_setup_id, SENSITIVE=1

    ; Hand off control of the widget to the XMANAGER:
    XMANAGER, CATCH=(sc.debug eq 0B) ; xmamager does't catch errors if debugging
    XMANAGER, 'lbt_xadsec', ids.base_root_id, /NO_BLOCK, GROUP_LEADER = group

end
