; $Id: xmirror_set.pro,v 1.1.1.1 2002/03/12 11:02:37 riccardi Exp $

pro xmirror_set_set_sensitive, state

if state.par.opt_curr then begin
	widget_control, state.id.iter_num, SENSITIVE=1
endif else begin
	widget_control, state.id.iter_num, SENSITIVE=0
endelse

end

;;;;;;;;;;;;;;;;;;;;;;;
; xmirror_set event loop
;;;;;;;;;;;;;;;;;;;;;;;
;
pro xmirror_set_event, event

common error_block, error
@adsec_common 

; Handle a kill request. It is considered as a cancel event.
; The right error returning is guaranteed only if GROUP_LEADER keyword
; is set to a valid parent id in the xmirror_set call.
if tag_names(event, /STRUCTURE_NAME) eq 'WIDGET_KILL_REQUEST' then begin
    widget_control, event.top, GET_UVALUE=state
    error = adsec_error.cancel
    widget_control, event.top, /DESTROY
endif

; Handle other events.
; Get the user value of the event sender
widget_control, event.id, GET_UVALUE = uvalue

case uvalue of
                                ; handle event from standard save button
    'ok': begin
                                ; restore all the parameter values
        widget_control, event.top, GET_UVALUE=state

     	; each actuator can be chosen only once
        act_hist = histogram(state.par.act)
        if check_channel(state.par.act, /NOALL) then begin
            dummy = dialog_message(["An actuator cannot be selected more then once"], $
                                   DIALOG_PARENT = event.top, $
                                   TITLE = "Multiple act. selection", /ERROR)
            return
        endif

		cl_act = state.par.act
		cl_pos = state.par.pos
		curr = state.par.curr
		if state.par.opt_curr then $
			opt_curr_iter=state.par.iter_num $
		else $
			opt_curr_iter=0

		error = mirror_set(curr, cl_act, cl_pos, OPT_CURR_ITER=opt_curr_iter)

        widget_control, event.top, /DESTROY
    end

                                ; handle event from standard help button:
    'help' : begin
    	dummy = dialog_message("Help is not supported, yet")
    end

                                ; handle event from standard restore button:
                                ; restore the default parameter values
;    'restore': begin
;                                ; restore all the parameter values
;        widget_control, event.top, GET_UVALUE=state
;
;       par = 0                 ; restore the default par. file
;        restore, state.def_file
;                                ; update the current module number
;        par.module.n_module = state.par.module.n_module
;                                ; set the default values for all the widgets
;        widget_control, state.id.bla1, SET_VALUE=par.bla1
;                                ; write the reseted state structure
;        widget_control, state.id.bla2, SET_VALUE=par.bla2
;                                ; write the reseted state structure
;        if par.method eq 'bla1' then method=0 else method=1
;        widget_control, state.id.method, SET_VALUE=method
;                                ; write the reseted state structure
;        state.par = par
;                                ; reset the sensitive status
;        xmirror_set_set_sensitive, state
;                                ; write the GUI state structure
;        widget_control, event.top, SET_UVALUE=state
;    end

                                ; exit without saving
    'cancel'  : begin
        error = adsec_error.cancel
        widget_control, event.top, /DESTROY
    end

    'pos0': begin
                                ; read the GUI state structure
        widget_control, event.top, GET_UVALUE=state
                                ; ge the value
        widget_control, event.id, GET_VALUE=pos_val
                                ; set the value in the state structure
        state.par.pos[0] = pos_val
                                ; write the state structure
        widget_control, event.top, SET_UVALUE=state
    end

     'pos1': begin
                                ; read the GUI state structure
        widget_control, event.top, GET_UVALUE=state
                                ; ge the value
        widget_control, event.id, GET_VALUE=pos_val
                                ; set the value in the state structure
        state.par.pos[1] = pos_val
                                ; write the state structure
        widget_control, event.top, SET_UVALUE=state
    end

    'pos2': begin
                                ; read the GUI state structure
        widget_control, event.top, GET_UVALUE=state
                                ; ge the value
        widget_control, event.id, GET_VALUE=pos_val
                                ; set the value in the state structure
        state.par.pos[2] = pos_val
                                ; write the state structure
        widget_control, event.top, SET_UVALUE=state
    end

    'act0': begin
                                ; read the GUI state structure
        widget_control, event.top, GET_UVALUE=state
                                ; ge the value
        widget_control, event.id, GET_VALUE=act_val
                                ; set the value in the state structure
        state.par.act[0] = act_val
                                ; write the state structure
        widget_control, event.top, SET_UVALUE=state
    end

     'act1': begin
                                ; read the GUI state structure
        widget_control, event.top, GET_UVALUE=state
                                ; ge the value
        widget_control, event.id, GET_VALUE=act_val
                                ; set the value in the state structure
        state.par.act[1] = act_val
                                ; write the state structure
        widget_control, event.top, SET_UVALUE=state
    end

    'act2': begin
                                ; read the GUI state structure
        widget_control, event.top, GET_UVALUE=state
                                ; ge the value
        widget_control, event.id, GET_VALUE=act_val
                                ; set the value in the state structure
        state.par.act[2] = act_val
                                ; write the state structure
        widget_control, event.top, SET_UVALUE=state
    end

    'curr': begin
                                ; read the GUI state structure
        widget_control, event.top, GET_UVALUE=state
                                ; ge the value
        widget_control, event.id, GET_VALUE=curr_val
                                ; set the value in the state structure
        state.par.curr = curr_val
                                ; write the state structure
        widget_control, event.top, SET_UVALUE=state
    end

                                ; handle the bla event
    'opt_curr': begin
                                ; read the GUI state structure
        widget_control, event.top, GET_UVALUE=state
                                ; get the bla1 parameter value
        widget_control, event.id, GET_VALUE=opt_curr_val
                                ; set the value in the state structure
                                ; put here control procedure for the parameter
        state.par.opt_curr = opt_curr_val
                                ; update the sensitive status
        xmirror_set_set_sensitive, state
                                ; write the state structure
        widget_control, event.top, SET_UVALUE=state
    end

    'iter_num': begin
                                ; read the GUI state structure
        widget_control, event.top, GET_UVALUE=state
                                ; get the bla2 parameter value
        widget_control, event.id, GET_VALUE=dummy
                                ; set the value in the state structure
                                ; put here control procedure for the parameter
        state.par.iter_num = dummy
                                ; write the state structure
        widget_control, event.top, SET_UVALUE=state
    end

	else: begin
		message, "Message not handled: "+uvalue, /CONTINUE
	end
endcase

end

;;;;;;;;;;;;;;;;;;;;;;;;;
; GUI generation code
;;;;;;;;;;;;;;;;;;;;;;;;;
;
function xmirror_set, ACT=act, POS=pos, CURR=curr, OPT_CURR=opt_curr, ITER_NUM=iter_num, $
			GROUP_LEADER=group

; error status from the event handler procedure
common error_block, error
; define adsec common blocks
@adsec_common 

error = adsec_error.ok

; default params values
par = { $
       act:		[16,22,28], $	; actuator numbers
       pos:		[-25000,-25000,-25000], $ ; actuator positions
       curr:	-1800, $		; common current
       opt_curr:1B, $			; optimize current
       iter_num:3}				; number of optimization iterations

def_num_act = n_elements(par.act)

if n_elements(act) ne 0 then begin
	if check_channel(act, /NOALL) or n_elements(act) ne def_num_act then begin
		dummy = dialog_message("Act vector not valid. Resetted to default values.")
	endif else begin
		par.act = act
	endelse
endif

if n_elements(pos) ne 0 then begin
	if n_elements(pos) ne def_num_act then begin
		dummy = dialog_message("Pos length doesn't match act length. Resetted to default values.")
	endif else begin
		par.pos = pos
	endelse
endif

if n_elements(curr) ne 0 then begin
	if n_elements(curr) ne n_elements(par.curr) then begin
		dummy = dialog_message("Curr length mismatching. Resetted to default values.")
	endif else begin
		par.curr = curr
	endelse
endif

if n_elements(opt_curr) ne 0 then begin
	par.opt_curr = opt_curr ne 0
endif

if n_elements(iter_num) ne 0 then begin
	par.iter_num = (iter_num < 10) > 1
endif

id = { $                      ; widget id structure
       act:		[0L,0L,0L], $ ; actuator number ids
       pos:		[0L,0L,0L], $ ; actuator position ids
       curr:	0L, $         ; common current id
       opt_curr:0L, $         ; optimize current id
       iter_num:0L}           ; number of optimization iterations id

state = { $                     ; widget state structure
          id: id, $             ; widget id structure
          par: par}             ; parameter structure


; root base
modal = n_elements(group) ne 0	; set modal only if a group leader is defined
title = 'Mirror Setting Procedure'
root_base_id = widget_base(TITLE=title, MODAL=modal, /COL, GROUP_LEADER=group)

; parameter base
par_base_id = widget_base(root_base_id, /FRAME, /COL)

; button base for control buttons
btn_base_id = widget_base(root_base_id, /FRAME, /ROW)
ok_id = widget_button(btn_base_id, VALUE="Ok", UVALUE="ok")
if modal then widget_control, ok_id, /DEFAULT_BUTTON
dummy = widget_button(btn_base_id, VALUE="Help", UVALUE="help")
cancel_id = widget_button(btn_base_id, VALUE="Cancel", UVALUE="cancel")
if modal then widget_control, save_id, /CANCEL_BUTTON

dummy_base_id = widget_base(par_base_id, /FRAME, /COL)
dummy = widget_label(dummy_base_id, VALUE='Closed loop actuators data:')
act_base_id = widget_base(dummy_base_id, /ROW)
for i=0,n_elements(state.id.act)-1 do begin
	dummy_base_id = widget_base(act_base_id, /FRAME, /COL)
	state.id.act[i] = cw_lslider(dummy_base_id, $
                           TITLE='Act. number', $
                           VALUE=state.par.act[i], $
                           UVALUE='act'+strtrim(i,2), $
                           /DRAG, /EDIT, $
                           MINIMUM=0L, MAXIMUM=adsec.n_actuators-1)
	state.id.pos[i] = cw_lslider(dummy_base_id, $
                           TITLE='Act. pos.', $
                           VALUE=state.par.pos[i], $
                           UVALUE='pos'+strtrim(i,2), $
						   /DRAG, /EDIT, $
						   MINIMUM=fix('8000'X), MAXIMUM=fix('7FFF'X))
endfor

dummy_base_id = widget_base(par_base_id, /ROW)
state.id.curr = cw_lslider(dummy_base_id, $
                       TITLE='Common current', $
                       VALUE=state.par.curr, $
                       UVALUE='curr', $
                       /DRAG, /EDIT, $
                       MINIMUM=-5000L, MAXIMUM=0L)

opt_base_id = widget_base(dummy_base_id, /COL, /FRAME)
state.id.opt_curr = cw_bgroup(opt_base_id, $
                            ['Optimize curr. pattern'], $
                            BUTTON_UVALUE=['opt_curr'], $
                            SET_VALUE = [state.par.opt_curr], $
                            UVALUE='opt_curr', /NONEXCLUSIVE)
state.id.iter_num = cw_lslider(opt_base_id, $
                       TITLE='Number of iteration', $
                       VALUE=state.par.iter_num, $
                       UVALUE='iter_num', $
                       /DRAG, /EDIT, $
                       MINIMUM=1L, MAXIMUM=10L)


; initialize all the sensitive states
xmirror_set_set_sensitive, state
; save the state structure of the GUI in the top base uvalue
widget_control, root_base_id, SET_UVALUE=state
; draw the GUI
widget_control, root_base_id, /realize

xmanager, 'xmirror_set', root_base_id, GROUP_LEADER=group

return, error

end





