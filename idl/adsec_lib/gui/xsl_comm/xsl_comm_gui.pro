; 
; IDL Widget Interface Procedures. This Code is automatically 
;     generated and should not be modified.

; 
; Generated on:	08/10/2005 18:46.40
; 
pro XSL_COMM_GUI_BASE_event, Event

  wTarget = (widget_info(Event.id,/NAME) eq 'TREE' ?  $
      widget_info(Event.id, /tree_root) : event.id)


  wWidget =  Event.top

  case wTarget of

    Widget_Info(wWidget, FIND_BY_UNAME='XSL_COMM_GUI_BASE'): begin
      if( Tag_Names(Event, /STRUCTURE_NAME) eq 'WIDGET_TIMER' )then $
        xls_comm_on_timer, Event
    end
    Widget_Info(wWidget, FIND_BY_UNAME='START_STOP_BTN'): begin
      if( Tag_Names(Event, /STRUCTURE_NAME) eq 'WIDGET_BUTTON' )then $
        xsc_comm_start_btn_on_press, Event
    end
    else:
  endcase

end

pro XSL_COMM_GUI_BASE, GROUP_LEADER=wGroup, _EXTRA=_VWBExtra_

  Resolve_Routine, 'xsl_comm_gui_eventcb',/COMPILE_FULL_FILE  ; Load event callback routines
  
  XSL_COMM_GUI_BASE = Widget_Base( GROUP_LEADER=wGroup,  $
      UNAME='XSL_COMM_GUI_BASE' ,XOFFSET=5 ,YOFFSET=5 ,SCR_XSIZE=745  $
      ,SCR_YSIZE=193 ,TITLE='Slope Computer Communication' ,SPACE=3  $
      ,XPAD=3 ,YPAD=3)

  
  START_STOP_BTN = Widget_Button(XSL_COMM_GUI_BASE,  $
      UNAME='START_STOP_BTN' ,XOFFSET=4 ,YOFFSET=4 ,SCR_XSIZE=75  $
      ,SCR_YSIZE=27 ,/ALIGN_CENTER ,VALUE='Start')

  
  REFRESH_TIME_FLD = Widget_Text(XSL_COMM_GUI_BASE,  $
      UNAME='REFRESH_TIME_FLD' ,XOFFSET=86 ,YOFFSET=36 ,/EDITABLE  $
      ,/NO_NEWLINE ,VALUE=[ '0.1' ] ,XSIZE=20 ,YSIZE=1)

  
  WID_LABEL_0 = Widget_Label(XSL_COMM_GUI_BASE, UNAME='WID_LABEL_0'  $
      ,XOFFSET=5 ,YOFFSET=41 ,SCR_XSIZE=78 ,SCR_YSIZE=18 ,/ALIGN_LEFT  $
      ,VALUE='Polling time [s]:')

  
  WID_LABEL_1 = Widget_Label(XSL_COMM_GUI_BASE, UNAME='WID_LABEL_1'  $
      ,XOFFSET=7 ,YOFFSET=75 ,/ALIGN_LEFT ,VALUE='Messages:')

  
  MESSAGE_FLD = Widget_Text(XSL_COMM_GUI_BASE, UNAME='MESSAGE_FLD'  $
      ,XOFFSET=64 ,YOFFSET=78 ,SCR_XSIZE=669 ,SCR_YSIZE=78 ,XSIZE=20  $
      ,YSIZE=1)

  Widget_Control, /REALIZE, XSL_COMM_GUI_BASE

  XManager, 'XSL_COMM_GUI_BASE', XSL_COMM_GUI_BASE, /NO_BLOCK  

end
; 
; Empty stub procedure used for autoloading.
; 
pro xsl_comm_gui, GROUP_LEADER=wGroup, _EXTRA=_VWBExtra_
  XSL_COMM_GUI_BASE, GROUP_LEADER=wGroup, _EXTRA=_VWBExtra_
end
