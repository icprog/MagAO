; $Id: xcomm_select_eventcb.pro,v 1.2 2004/10/08 17:21:01 riccardi Exp $
;
; IDL Event Callback Procedures
; xcomm_select_eventcb
;
; Generated on:	03/10/2001 22:41.46
;
;-----------------------------------------------------------------
; Notify Realize Callback Procedure.
; Argument:
;   wWidget - ID number of specific widget.
;
;
;   Retrieve the IDs of other widgets in the widget hierarchy using
;       id=widget_info(Event.top, FIND_BY_UNAME=name)

;-----------------------------------------------------------------
pro xcs_on_realize_comm_type, wWidget
  common xcs_block, comm_type, dummy
  widget_control, wWidget, SET_DROPLIST_SELECT=comm_type
end
;-----------------------------------------------------------------
; Activate Button Callback Procedure.
; Argument:
;   Event structure:
;
;   {WIDGET_BUTTON, ID:0L, TOP:0L, HANDLER:0L, SELECT:0}
;
;   ID is the widget ID of the component generating the event. TOP is
;       the widget ID of the top level widget containing ID. HANDLER
;       contains the widget ID of the widget associated with the
;       handler routine.

;   SELECT is set to 1 if the button was set, and 0 if released.
;       Normal buttons do not generate events when released, so
;       SELECT will always be 1. However, toggle buttons (created by
;       parenting a button to an exclusive or non-exclusive base)
;       return separate events for the set and release actions.

;   Retrieve the IDs of other widgets in the widget hierarchy using
;       id=widget_info(Event.top, FIND_BY_UNAME=name)

;-----------------------------------------------------------------
pro xcs_on_press_ok, Event
  widget_control, Event.top, /DESTROY
end
;-----------------------------------------------------------------
; Droplist Select Item Callback Procedure.
; Argument:
;   Event structure:
;
;   {WIDGET_DROPLIST, ID:0L, TOP:0L, HANDLER:0L, INDEX:0L }
;
;   ID is the widget ID of the component generating the event. TOP is
;       the widget ID of the top level widget containing ID. HANDLER
;       contains the widget ID of the widget associated with the
;       handler routine.

;   INDEX returns the index of the selected item. This can be used to
;       index the array of names originally used to set the widget's
;       value.

;   Retrieve the IDs of other widgets in the widget hierarchy using
;       id=widget_info(Event.top, FIND_BY_UNAME=name)

;-----------------------------------------------------------------
pro xcs_on_select_comm_type, Event
  common xcs_block, comm_type, dummy
  comm_type = Event.index
end

;
; Empty stub procedure used for autoloading.
;
pro xcomm_select_eventcb
end

;-----------------------------------------------------------------
; Post Create Widget Procedure.
; Argument:
;   wWidget - ID number of specific widget.
;
;   Any keywords passed into the generated widget creation procedure
;       are passed into this procudure.

;   Retrieve the IDs of other widgets in the widget hierarchy using
;       id=widget_info(Event.top, FIND_BY_UNAME=name)

;-----------------------------------------------------------------
pro xcs_on_creation_root_base, wWidget, DEFAULT_COMM_TYPE=def_comm_type, _EXTRA=_VWBExtra_
  common xcs_block, comm_type, dummy
  if n_elements(def_comm_type) eq 0 then comm_type=1 else comm_type=def_comm_type
end
