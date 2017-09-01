; $Id: xcomm_select.pro,v 1.2 2004/10/08 17:12:32 riccardi Exp $
;
; IDL Widget Interface Procedures. This Code is automatically
;     generated and should not be modified.

;
; Generated on:	03/10/2001 23:11.27
;
pro WID_BASE_0_event, Event

  wWidget =  Event.top

  case Event.id of

    Widget_Info(wWidget, FIND_BY_UNAME='COMM_TYPE_DL'): begin
      if( Tag_Names(Event, /STRUCTURE_NAME) eq 'WIDGET_DROPLIST' )then $
        xcs_on_select_comm_type, Event
    end
    Widget_Info(wWidget, FIND_BY_UNAME='XCS_OK_BTN'): begin
      if( Tag_Names(Event, /STRUCTURE_NAME) eq 'WIDGET_BUTTON' )then $
        xcs_on_press_ok, Event
    end
    else:
  endcase

end

pro WID_BASE_0, GROUP_LEADER=wGroup, _EXTRA=_VWBExtra_

  common xcs_block, comm_type, host_comm_list

  Resolve_Routine, 'xcomm_select_eventcb',/COMPILE_FULL_FILE  ; Load event callback routines

  dims = fix(get_screen_size()/[2,3]-[268,141]/2)

  WID_BASE_0 = Widget_Base( GROUP_LEADER=wGroup, UNAME='WID_BASE_0'  $
      ,XOFFSET=dims[0] ,YOFFSET=dims[1] ,SCR_XSIZE=268 ,SCR_YSIZE=142  $
      ,TITLE='Communication type' ,SPACE=3 ,XPAD=3 ,YPAD=3)


  xcs_on_creation_root_base, WID_BASE_0, _EXTRA=_VWBExtra_


  COMM_TYPE_DL = Widget_Droplist(WID_BASE_0, UNAME='COMM_TYPE_DL'  $
      ,XOFFSET=13 ,YOFFSET=21 ,SCR_XSIZE=147 ,SCR_YSIZE=34  $
      ,NOTIFY_REALIZE='xcs_on_realize_comm_type' ,VALUE=host_comm_list)


  WID_LABEL_0 = Widget_Label(WID_BASE_0, UNAME='WID_LABEL_0'  $
      ,XOFFSET=13 ,YOFFSET=3 ,SCR_XSIZE=211 ,SCR_YSIZE=24  $
      ,/ALIGN_LEFT ,VALUE='Select the type of communication:')


  XCS_OK_BTN = Widget_Button(WID_BASE_0, UNAME='XCS_OK_BTN'  $
      ,XOFFSET=173 ,YOFFSET=64 ,SCR_XSIZE=73 ,SCR_YSIZE=34  $
      ,/ALIGN_CENTER ,VALUE='Ok')

  Widget_Control, /REALIZE, WID_BASE_0

  XManager, 'WID_BASE_0', WID_BASE_0

end
;
; Empty stub procedure used for autoloading.
;
function xcomm_select, host_comm_list_inp, GROUP_LEADER=wGroup, _EXTRA=_VWBExtra_
  common xcs_block, comm_type, host_comm_list
  host_comm_list = host_comm_list_inp
  WID_BASE_0, GROUP_LEADER=wGroup, _EXTRA=_VWBExtra_
  return, comm_type
end
