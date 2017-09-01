; 
; IDL Widget Interface Procedures. This Code is automatically 
;     generated and should not be modified.

; 
; Generated on:	03/06/2007 14:59.19
; 
pro WID_BASE_0_event, Event

  wTarget = (widget_info(Event.id,/NAME) eq 'TREE' ?  $
      widget_info(Event.id, /tree_root) : event.id)


  wWidget =  Event.top

  case wTarget of

    Widget_Info(wWidget, FIND_BY_UNAME='REFRESH_TIME_FLD'): begin
      if( Tag_Names(Event, /STRUCTURE_NAME) eq 'WIDGET_TEXT_STR' )then $
        xol_gui_update_refresh_time, Event
    end
    Widget_Info(wWidget, FIND_BY_UNAME='START_STOP_READ_BTN'): begin
      if( Tag_Names(Event, /STRUCTURE_NAME) eq 'WIDGET_TIMER' )then $
        xol_gui_on_timer, Event
      if( Tag_Names(Event, /STRUCTURE_NAME) eq 'WIDGET_BUTTON' )then $
        xol_gui_start_stop, Event
    end
    Widget_Info(wWidget, FIND_BY_UNAME='EXIT_BTN'): begin
      if( Tag_Names(Event, /STRUCTURE_NAME) eq 'WIDGET_BUTTON' )then $
        xol_gui_exit, Event
    end
    Widget_Info(wWidget, FIND_BY_UNAME='READ_FROM_DDS_BTN'): begin
      if( Tag_Names(Event, /STRUCTURE_NAME) eq 'WIDGET_BUTTON' )then $
        xol_gui_read_from_dds, Event
    end
    Widget_Info(wWidget, FIND_BY_UNAME='REC_DCOMM_BTN'): begin
      if( Tag_Names(Event, /STRUCTURE_NAME) eq 'WIDGET_BUTTON' )then $
        rec_dcomm_press_btn, Event
    end
    Widget_Info(wWidget, FIND_BY_UNAME='OFFLOAD_TILT_BTN'): begin
      if( Tag_Names(Event, /STRUCTURE_NAME) eq 'WIDGET_BUTTON' )then $
        xol_gui_offload_tilt, Event
    end
    Widget_Info(wWidget, FIND_BY_UNAME='POS_OPT_CUTS_BTN'): begin
      if( Tag_Names(Event, /STRUCTURE_NAME) eq 'WIDGET_BUTTON' )then $
        xol_gui_pos_opt_cuts, Event
    end
    Widget_Info(wWidget, FIND_BY_UNAME='POS_SET_OFFSET_BTN'): begin
      if( Tag_Names(Event, /STRUCTURE_NAME) eq 'WIDGET_BUTTON' )then $
        xol_gui_pos_set_offset, Event
    end
    Widget_Info(wWidget, FIND_BY_UNAME='POS_NEW_OFFSET_BTN'): begin
      if( Tag_Names(Event, /STRUCTURE_NAME) eq 'WIDGET_BUTTON' )then $
        pos_new_offset, Event
    end
    Widget_Info(wWidget, FIND_BY_UNAME='POS_SHOW_AVE_BTN'): begin
      if( Tag_Names(Event, /STRUCTURE_NAME) eq 'WIDGET_BUTTON' )then $
        xol_gui_pos_show_ave, Event
    end
    Widget_Info(wWidget, FIND_BY_UNAME='CURR_OPT_CUTS_BTN'): begin
      if( Tag_Names(Event, /STRUCTURE_NAME) eq 'WIDGET_BUTTON' )then $
        xol_gui_curr_opt_cuts, Event
    end
    Widget_Info(wWidget, FIND_BY_UNAME='CURR_SET_OFFSET_BTN'): begin
      if( Tag_Names(Event, /STRUCTURE_NAME) eq 'WIDGET_BUTTON' )then $
        xol_gui_curr_set_offset, Event
    end
    Widget_Info(wWidget, FIND_BY_UNAME='CURR_NEW_OFFSET_BTN'): begin
      if( Tag_Names(Event, /STRUCTURE_NAME) eq 'WIDGET_BUTTON' )then $
        curr_new_offset, Event
    end
    Widget_Info(wWidget, FIND_BY_UNAME='CURR_SHOW_AVE_BTN'): begin
      if( Tag_Names(Event, /STRUCTURE_NAME) eq 'WIDGET_BUTTON' )then $
        xol_gui_curr_show_ave, Event
    end
    Widget_Info(wWidget, FIND_BY_UNAME='COMM_OPT_CUTS_BTN'): begin
      if( Tag_Names(Event, /STRUCTURE_NAME) eq 'WIDGET_BUTTON' )then $
        xol_gui_comm_opt_cuts, Event
    end
    Widget_Info(wWidget, FIND_BY_UNAME='COMM_SET_OFFSET_BTN'): begin
      if( Tag_Names(Event, /STRUCTURE_NAME) eq 'WIDGET_BUTTON' )then $
        xol_gui_comm_set_offset, Event
    end
    Widget_Info(wWidget, FIND_BY_UNAME='COMM_NEW_OFFSET_BTN'): begin
      if( Tag_Names(Event, /STRUCTURE_NAME) eq 'WIDGET_BUTTON' )then $
        comm_new_offset, Event
    end
    Widget_Info(wWidget, FIND_BY_UNAME='COMM_SHOW_AVE_BTN'): begin
      if( Tag_Names(Event, /STRUCTURE_NAME) eq 'WIDGET_BUTTON' )then $
        xol_gui_comm_show_ave, Event
    end
    widget_Info(wWidget, FIND_BY_UNAME='APPLY_AVE_COMMAND_BTN'): begin
      if( Tag_Names(Event, /STRUCTURE_NAME) eq 'WIDGET_BUTTON' )then $
        xol_apply_ave_command, Event
    end
    Widget_Info(wWidget, FIND_BY_UNAME='RMS_OPT_CUTS_BTN'): begin
      if( Tag_Names(Event, /STRUCTURE_NAME) eq 'WIDGET_BUTTON' )then $
        xol_gui_rms_opt_cuts, Event
    end
    Widget_Info(wWidget, FIND_BY_UNAME='RMS_SET_OFFSET_BTN'): begin
      if( Tag_Names(Event, /STRUCTURE_NAME) eq 'WIDGET_BUTTON' )then $
        xol_gui_rms_set_offset, Event
    end
    Widget_Info(wWidget, FIND_BY_UNAME='RMS_NEW_OFFSET_BTN'): begin
      if( Tag_Names(Event, /STRUCTURE_NAME) eq 'WIDGET_BUTTON' )then $
        rms_new_offset, Event
    end
    Widget_Info(wWidget, FIND_BY_UNAME='RMS_SHOW_AVE_BTN'): begin
      if( Tag_Names(Event, /STRUCTURE_NAME) eq 'WIDGET_BUTTON' )then $
        xol_gui_rms_show_ave, Event
    end
    Widget_Info(wWidget, FIND_BY_UNAME='MODAL_DECOMP_DLIST'): begin
      if( Tag_Names(Event, /STRUCTURE_NAME) eq 'WIDGET_COMBOBOX' )then $
        xol_gui_modal_decomp_select, Event
    end
    else:
  endcase

end

pro WID_BASE_0, GROUP_LEADER=wGroup, _EXTRA=_VWBExtra_

  Resolve_Routine, 'xol_gui_eventcb',/COMPILE_FULL_FILE  ; Load event callback routines
  
  WID_BASE_0 = Widget_Base( GROUP_LEADER=wGroup, UNAME='WID_BASE_0'  $
      ,XOFFSET=5 ,YOFFSET=5 ,/ALIGN_CENTER ,/BASE_ALIGN_CENTER  $
      ,TITLE='GUI-V2.0' ,SPACE=3 ,XPAD=3 ,YPAD=3 ,COLUMN=1)

  xol_gui_post_creation, WID_BASE_0, _EXTRA=_VWBExtra_
  
  WID_BASE_15 = Widget_Base(WID_BASE_0, UNAME='WID_BASE_15'  $
      ,XOFFSET=131 ,YOFFSET=3 ,TITLE='IDL' ,SPACE=3 ,XPAD=3 ,YPAD=3  $
      ,ROW=1)

  WID_BASE_7 = Widget_Base(WID_BASE_15, UNAME='WID_BASE_7' ,XOFFSET=3  $
      ,YOFFSET=3 ,TITLE='IDL' ,SPACE=3 ,XPAD=3 ,YPAD=3 ,ROW=1)

  
  WID_BASE_16 = Widget_Base(WID_BASE_7, UNAME='WID_BASE_16'  $
      ,XOFFSET=3 ,YOFFSET=3 ,SPACE=3 ,XPAD=3 ,YPAD=3 ,COLUMN=1)

  
  REFRESH_TIME_FLD = Widget_Text(WID_BASE_16,  $
      UNAME='REFRESH_TIME_FLD' ,XOFFSET=3 ,YOFFSET=3 ,/EDITABLE  $
      ,/ALL_EVENTS ,VALUE=[ '1.0' ] ,XSIZE=20 ,YSIZE=1)

  
  WID_LABEL_38 = Widget_Label(WID_BASE_16, UNAME='WID_LABEL_38'  $
      ,XOFFSET=27 ,YOFFSET=27 ,/ALIGN_CENTER ,VALUE='Refresh time'+ $
      ' [s]')

  
  WID_BASE_17 = Widget_Base(WID_BASE_7, UNAME='WID_BASE_17'  $
      ,XOFFSET=136 ,YOFFSET=3 ,TITLE='IDL' ,SPACE=3 ,XPAD=3 ,YPAD=3  $
      ,COLUMN=1)

  
  START_STOP_READ_BTN = Widget_Button(WID_BASE_17,  $
      UNAME='START_STOP_READ_BTN' ,XOFFSET=3 ,YOFFSET=3  $
      ,/ALIGN_CENTER ,VALUE='Start')

  
  EXIT_BTN = Widget_Button(WID_BASE_17, UNAME='EXIT_BTN' ,XOFFSET=5  $
      ,YOFFSET=28 ,/ALIGN_CENTER ,VALUE='Exit')

  
  WID_BASE_18 = Widget_Base(WID_BASE_7, UNAME='WID_BASE_18'  $
      ,XOFFSET=182 ,YOFFSET=3 ,TITLE='IDL' ,COLUMN=1 ,/NONEXCLUSIVE)

  
  READ_FROM_DDS_BTN = Widget_Button(WID_BASE_18,  $
      UNAME='READ_FROM_DDS_BTN' ,/ALIGN_LEFT ,VALUE='Read from DDS')

  
  REC_DCOMM_BTN = Widget_Button(WID_BASE_18, UNAME='REC_DCOMM_BTN'  $
      ,YOFFSET=22 ,/ALIGN_LEFT ,VALUE='Display rec. dcomm')

  
  POINTING_ERROR_BASE = Widget_Base(WID_BASE_15,  $
      UNAME='POINTING_ERROR_BASE' ,XOFFSET=311 ,YOFFSET=3 ,SPACE=3  $
      ,XPAD=3 ,YPAD=3 ,COLUMN=1)

  
  xol_gui_pointing_error_creation, POINTING_ERROR_BASE, _EXTRA=_VWBExtra_

  
  WID_LABEL_39 = Widget_Label(POINTING_ERROR_BASE,  $
      UNAME='WID_LABEL_39' ,XOFFSET=3 ,YOFFSET=3 ,/ALIGN_LEFT  $
      ,VALUE='Hexapode z error [um]:')

  
  DELTA_FOCUS_FLD = Widget_Text(POINTING_ERROR_BASE,  $
      UNAME='DELTA_FOCUS_FLD' ,XOFFSET=3 ,YOFFSET=21 ,XSIZE=20  $
      ,YSIZE=1)

  
  WID_LABEL_40 = Widget_Label(POINTING_ERROR_BASE,  $
      UNAME='WID_LABEL_40' ,XOFFSET=14 ,YOFFSET=45 ,/ALIGN_CENTER  $
      ,VALUE='Running mean steps:')

  
  OFFLOAD_AVE_SLIDER = Widget_Slider(POINTING_ERROR_BASE,  $
      UNAME='OFFLOAD_AVE_SLIDER' ,XOFFSET=3 ,YOFFSET=63 ,MINIMUM=1)

  
  WID_BASE_4 = Widget_Base(WID_BASE_15, UNAME='WID_BASE_4'  $
      ,XOFFSET=444 ,YOFFSET=3 ,TITLE='IDL' ,SPACE=3 ,XPAD=3 ,YPAD=3  $
      ,COLUMN=1)

  
  WID_LABEL_41 = Widget_Label(WID_BASE_4, UNAME='WID_LABEL_41'  $
      ,XOFFSET=40 ,YOFFSET=3 ,/ALIGN_CENTER ,VALUE='Pointing error'+ $
      ' [arcsec]:')

  
  WID_BASE_19 = Widget_Base(WID_BASE_4, UNAME='WID_BASE_19'  $
      ,XOFFSET=10 ,YOFFSET=21 ,/ALIGN_RIGHT ,TITLE='IDL' ,SPACE=3  $
      ,XPAD=3 ,YPAD=3 ,ROW=1)

  
  WID_LABEL_42 = Widget_Label(WID_BASE_19, UNAME='WID_LABEL_42'  $
      ,XOFFSET=3 ,YOFFSET=3 ,/ALIGN_LEFT ,VALUE='Azimuth:')

  
  DELTA_AZIMUTH_FLD = Widget_Text(WID_BASE_19,  $
      UNAME='DELTA_AZIMUTH_FLD' ,XOFFSET=48 ,YOFFSET=3 ,XSIZE=20  $
      ,YSIZE=1)

  
  WID_BASE_20 = Widget_Base(WID_BASE_4, UNAME='WID_BASE_20'  $
      ,XOFFSET=3 ,YOFFSET=51 ,/ALIGN_RIGHT ,TITLE='IDL' ,SPACE=3  $
      ,XPAD=3 ,YPAD=3 ,ROW=1)

  
  WID_LABEL_43 = Widget_Label(WID_BASE_20, UNAME='WID_LABEL_43'  $
      ,XOFFSET=3 ,YOFFSET=3 ,/ALIGN_LEFT ,VALUE='Elevation:')

  
  DELTA_ELEVATION_FLD = Widget_Text(WID_BASE_20,  $
      UNAME='DELTA_ELEVATION_FLD' ,XOFFSET=55 ,YOFFSET=3 ,XSIZE=20  $
      ,YSIZE=1)

  
  OFFLOAD_TILT_BTN = Widget_Button(WID_BASE_4,  $
      UNAME='OFFLOAD_TILT_BTN' ,XOFFSET=100 ,YOFFSET=81 ,/ALIGN_RIGHT  $
      ,VALUE='Offload tilt now')

  
  WID_BASE_1 = Widget_Base(WID_BASE_0, UNAME='WID_BASE_1' ,XOFFSET=69  $
      ,YOFFSET=118 ,TITLE='IDL' ,SPACE=3 ,XPAD=3 ,YPAD=3 ,ROW=1)

  
  POS_BASE = Widget_Base(WID_BASE_1, UNAME='POS_BASE' ,XOFFSET=3  $
      ,YOFFSET=3 ,SPACE=3 ,XPAD=3 ,YPAD=3 ,COLUMN=1)

  
  xol_gui_pos_base_post_creation, POS_BASE, _EXTRA=_VWBExtra_

  
  WID_LABEL_28 = Widget_Label(POS_BASE, UNAME='WID_LABEL_28'  $
      ,XOFFSET=73 ,YOFFSET=3 ,/ALIGN_LEFT ,VALUE='Position')

  
  POS_DRAW = Widget_Draw(POS_BASE, UNAME='POS_DRAW' ,XOFFSET=3  $
      ,YOFFSET=21 ,SCR_XSIZE=180 ,SCR_YSIZE=180)

  
  WID_BASE_28 = Widget_Base(POS_BASE, UNAME='WID_BASE_28' ,XOFFSET=14  $
      ,YOFFSET=204 ,/ALIGN_CENTER ,TITLE='IDL' ,SPACE=3 ,XPAD=3  $
      ,YPAD=3 ,ROW=1)

  
  WID_LABEL_30 = Widget_Label(WID_BASE_28, UNAME='WID_LABEL_30'  $
      ,XOFFSET=3 ,YOFFSET=3 ,/ALIGN_LEFT ,VALUE='Max:')

  
  POS_MAX_FLD = Widget_Text(WID_BASE_28, UNAME='POS_MAX_FLD'  $
      ,XOFFSET=31 ,YOFFSET=3 ,/NO_NEWLINE ,VALUE=[ '0' ] ,XSIZE=20  $
      ,YSIZE=1)

  
  WID_BASE_29 = Widget_Base(POS_BASE, UNAME='WID_BASE_29' ,XOFFSET=15  $
      ,YOFFSET=234 ,/ALIGN_CENTER ,TITLE='IDL' ,SPACE=3 ,XPAD=3  $
      ,YPAD=3 ,ROW=1)

  
  WID_LABEL_58 = Widget_Label(WID_BASE_29, UNAME='WID_LABEL_58'  $
      ,XOFFSET=3 ,YOFFSET=3 ,/ALIGN_LEFT ,VALUE='Min:')

  
  POS_MIN_FLD = Widget_Text(WID_BASE_29, UNAME='POS_MIN_FLD'  $
      ,XOFFSET=28 ,YOFFSET=3 ,/NO_NEWLINE ,VALUE=[ '0' ] ,XSIZE=20  $
      ,YSIZE=1)

  
  WID_BASE_30 = Widget_Base(POS_BASE, UNAME='WID_BASE_30' ,XOFFSET=13  $
      ,YOFFSET=264 ,/ALIGN_CENTER ,TITLE='IDL' ,SPACE=3 ,XPAD=3  $
      ,YPAD=3 ,COLUMN=1)

  
  WID_BASE_5 = Widget_Base(WID_BASE_30, UNAME='WID_BASE_5' ,XOFFSET=3  $
      ,YOFFSET=3 ,/ALIGN_RIGHT ,TITLE='IDL' ,SPACE=3 ,XPAD=3 ,YPAD=3  $
      ,ROW=1)

  
  WID_LABEL_59 = Widget_Label(WID_BASE_5, UNAME='WID_LABEL_59'  $
      ,XOFFSET=3 ,YOFFSET=6 ,/ALIGN_CENTER ,VALUE='UB:')

  
  POS_MAX_CBAR_FLD = Widget_Text(WID_BASE_5, UNAME='POS_MAX_CBAR_FLD'  $
      ,XOFFSET=26 ,YOFFSET=3 ,VALUE=[ '32767' ] ,XSIZE=20 ,YSIZE=1)

  
  WID_BASE_6 = Widget_Base(WID_BASE_30, UNAME='WID_BASE_6' ,XOFFSET=5  $
      ,YOFFSET=33 ,/ALIGN_RIGHT ,TITLE='IDL' ,SPACE=3 ,XPAD=3 ,YPAD=3  $
      ,ROW=1)

  
  WID_LABEL_60 = Widget_Label(WID_BASE_6, UNAME='WID_LABEL_60'  $
      ,XOFFSET=3 ,YOFFSET=6 ,/ALIGN_CENTER ,VALUE='LB:')

  
  POS_MIN_CBAR_FLD = Widget_Text(WID_BASE_6, UNAME='POS_MIN_CBAR_FLD'  $
      ,XOFFSET=24 ,YOFFSET=3 ,/ALL_EVENTS ,VALUE=[ '-32768' ]  $
      ,XSIZE=20 ,YSIZE=1)

  
  POS_OPT_CUTS_BTN = Widget_Button(WID_BASE_30,  $
      UNAME='POS_OPT_CUTS_BTN' ,XOFFSET=50 ,YOFFSET=63 ,/ALIGN_CENTER  $
      ,VALUE='Opt. cuts')

  
  WID_BASE_31 = Widget_Base(POS_BASE, UNAME='WID_BASE_31' ,XOFFSET=10  $
      ,YOFFSET=355 ,/ALIGN_CENTER ,TITLE='IDL' ,SPACE=3 ,XPAD=3  $
      ,YPAD=3 ,ROW=1)

  
  POS_COLOR_BAR = Widget_Draw(WID_BASE_31, UNAME='POS_COLOR_BAR'  $
      ,XOFFSET=3 ,YOFFSET=3 ,SCR_XSIZE=160 ,SCR_YSIZE=10)

  
  WID_BASE_32 = Widget_Base(POS_BASE, UNAME='WID_BASE_32' ,XOFFSET=27  $
      ,YOFFSET=374 ,/ALIGN_CENTER ,TITLE='IDL' ,SPACE=3 ,XPAD=3  $
      ,YPAD=3 ,ROW=1)

  
  WID_BASE_46 = Widget_Base(WID_BASE_32, UNAME='WID_BASE_46'  $
      ,XOFFSET=3 ,YOFFSET=3 ,COLUMN=1 ,/NONEXCLUSIVE)

  
  POS_SET_OFFSET_BTN = Widget_Button(WID_BASE_46,  $
      UNAME='POS_SET_OFFSET_BTN' ,/ALIGN_LEFT ,VALUE='Offset')

  
  POS_NEW_OFFSET_BTN = Widget_Button(WID_BASE_32,  $
      UNAME='POS_NEW_OFFSET_BTN' ,XOFFSET=62 ,YOFFSET=3  $
      ,/ALIGN_CENTER ,VALUE='New offset')

  
  WID_BASE_34 = Widget_Base(POS_BASE, UNAME='WID_BASE_34' ,XOFFSET=29  $
      ,YOFFSET=405 ,/ALIGN_CENTER ,COLUMN=1 ,/NONEXCLUSIVE)

  
  POS_SHOW_AVE_BTN = Widget_Button(WID_BASE_34,  $
      UNAME='POS_SHOW_AVE_BTN' ,/ALIGN_LEFT ,VALUE='Show ave.'+ $
      ' command')

  
  WID_BASE_35 = Widget_Base(POS_BASE, UNAME='WID_BASE_35' ,XOFFSET=3  $
      ,YOFFSET=430 ,TITLE='IDL' ,SPACE=3 ,XPAD=3 ,YPAD=3 ,COLUMN=1)

  
  WID_BASE_47 = Widget_Base(WID_BASE_35, UNAME='WID_BASE_47'  $
      ,XOFFSET=3 ,YOFFSET=3 ,TITLE='IDL' ,SPACE=3 ,XPAD=3 ,YPAD=3  $
      ,ROW=1)

  
  WID_LABEL_49 = Widget_Label(WID_BASE_47, UNAME='WID_LABEL_49'  $
      ,XOFFSET=3 ,YOFFSET=3 ,/ALIGN_LEFT ,VALUE='Sets to ave.:')

  
  POS_NUM_SET_TO_AVE = Widget_Slider(WID_BASE_47,  $
      UNAME='POS_NUM_SET_TO_AVE' ,XOFFSET=68 ,YOFFSET=3 ,MINIMUM=1  $
      ,MAXIMUM=1000)

  
  CURR_BASE = Widget_Base(WID_BASE_1, UNAME='CURR_BASE' ,XOFFSET=192  $
      ,YOFFSET=3 ,SPACE=3 ,XPAD=3 ,YPAD=3 ,COLUMN=1)

  
  xol_gui_curr_base_post_creation, CURR_BASE, _EXTRA=_VWBExtra_

  
  WID_LABEL_45 = Widget_Label(CURR_BASE, UNAME='WID_LABEL_45'  $
      ,XOFFSET=75 ,YOFFSET=3 ,/ALIGN_LEFT ,VALUE='Current')

  
  CURR_DRAW = Widget_Draw(CURR_BASE, UNAME='CURR_DRAW' ,XOFFSET=3  $
      ,YOFFSET=21 ,SCR_XSIZE=180 ,SCR_YSIZE=180)

  
  WID_BASE_37 = Widget_Base(CURR_BASE, UNAME='WID_BASE_37'  $
      ,XOFFSET=14 ,YOFFSET=204 ,/ALIGN_CENTER ,TITLE='IDL' ,SPACE=3  $
      ,XPAD=3 ,YPAD=3 ,ROW=1)

  
  WID_LABEL_50 = Widget_Label(WID_BASE_37, UNAME='WID_LABEL_50'  $
      ,XOFFSET=3 ,YOFFSET=3 ,/ALIGN_LEFT ,VALUE='Max:')

  
  CURR_MAX_FLD = Widget_Text(WID_BASE_37, UNAME='CURR_MAX_FLD'  $
      ,XOFFSET=31 ,YOFFSET=3 ,/NO_NEWLINE ,VALUE=[ '0' ] ,XSIZE=20  $
      ,YSIZE=1)

  
  WID_BASE_38 = Widget_Base(CURR_BASE, UNAME='WID_BASE_38'  $
      ,XOFFSET=15 ,YOFFSET=234 ,/ALIGN_CENTER ,TITLE='IDL' ,SPACE=3  $
      ,XPAD=3 ,YPAD=3 ,ROW=1)

  
  WID_LABEL_51 = Widget_Label(WID_BASE_38, UNAME='WID_LABEL_51'  $
      ,XOFFSET=3 ,YOFFSET=3 ,/ALIGN_LEFT ,VALUE='Min:')

  
  CURR_MIN_FLD = Widget_Text(WID_BASE_38, UNAME='CURR_MIN_FLD'  $
      ,XOFFSET=28 ,YOFFSET=3 ,/NO_NEWLINE ,VALUE=[ '0' ] ,XSIZE=20  $
      ,YSIZE=1)

  
  WID_BASE_39 = Widget_Base(CURR_BASE, UNAME='WID_BASE_39'  $
      ,XOFFSET=13 ,YOFFSET=264 ,/ALIGN_CENTER ,TITLE='IDL' ,SPACE=3  $
      ,XPAD=3 ,YPAD=3 ,COLUMN=1)

  
  WID_BASE_8 = Widget_Base(WID_BASE_39, UNAME='WID_BASE_8' ,XOFFSET=3  $
      ,YOFFSET=3 ,/ALIGN_RIGHT ,TITLE='IDL' ,SPACE=3 ,XPAD=3 ,YPAD=3  $
      ,ROW=1)

  
  WID_LABEL_65 = Widget_Label(WID_BASE_8, UNAME='WID_LABEL_65'  $
      ,XOFFSET=3 ,YOFFSET=6 ,/ALIGN_CENTER ,VALUE='UB:')

  
  CURR_MAX_CBAR_FLD = Widget_Text(WID_BASE_8,  $
      UNAME='CURR_MAX_CBAR_FLD' ,XOFFSET=26 ,YOFFSET=3 ,VALUE=[  $
      '32767' ] ,XSIZE=20 ,YSIZE=1)

  
  WID_BASE_9 = Widget_Base(WID_BASE_39, UNAME='WID_BASE_9' ,XOFFSET=5  $
      ,YOFFSET=33 ,/ALIGN_RIGHT ,TITLE='IDL' ,SPACE=3 ,XPAD=3 ,YPAD=3  $
      ,ROW=1)

  
  WID_LABEL_66 = Widget_Label(WID_BASE_9, UNAME='WID_LABEL_66'  $
      ,XOFFSET=3 ,YOFFSET=6 ,/ALIGN_CENTER ,VALUE='LB:')

  
  CURR_MIN_CBAR_FLD = Widget_Text(WID_BASE_9,  $
      UNAME='CURR_MIN_CBAR_FLD' ,XOFFSET=24 ,YOFFSET=3 ,/ALL_EVENTS  $
      ,VALUE=[ '-32768' ] ,XSIZE=20 ,YSIZE=1)

  
  CURR_OPT_CUTS_BTN = Widget_Button(WID_BASE_39,  $
      UNAME='CURR_OPT_CUTS_BTN' ,XOFFSET=50 ,YOFFSET=63  $
      ,/ALIGN_CENTER ,VALUE='Opt. cuts')

  
  WID_BASE_40 = Widget_Base(CURR_BASE, UNAME='WID_BASE_40'  $
      ,XOFFSET=10 ,YOFFSET=355 ,/ALIGN_CENTER ,TITLE='IDL' ,SPACE=3  $
      ,XPAD=3 ,YPAD=3 ,ROW=1)

  
  CURR_COLOR_BAR = Widget_Draw(WID_BASE_40, UNAME='CURR_COLOR_BAR'  $
      ,XOFFSET=3 ,YOFFSET=3 ,SCR_XSIZE=160 ,SCR_YSIZE=10)

  
  WID_BASE_41 = Widget_Base(CURR_BASE, UNAME='WID_BASE_41'  $
      ,XOFFSET=27 ,YOFFSET=374 ,/ALIGN_CENTER ,TITLE='IDL' ,SPACE=3  $
      ,XPAD=3 ,YPAD=3 ,ROW=1)

  
  WID_BASE_48 = Widget_Base(WID_BASE_41, UNAME='WID_BASE_48'  $
      ,XOFFSET=3 ,YOFFSET=3 ,COLUMN=1 ,/NONEXCLUSIVE)

  
  CURR_SET_OFFSET_BTN = Widget_Button(WID_BASE_48,  $
      UNAME='CURR_SET_OFFSET_BTN' ,/ALIGN_LEFT ,VALUE='Offset')

  
  CURR_NEW_OFFSET_BTN = Widget_Button(WID_BASE_41,  $
      UNAME='CURR_NEW_OFFSET_BTN' ,XOFFSET=62 ,YOFFSET=3  $
      ,/ALIGN_CENTER ,VALUE='New offset')

  
  WID_BASE_43 = Widget_Base(CURR_BASE, UNAME='WID_BASE_43'  $
      ,XOFFSET=29 ,YOFFSET=405 ,/ALIGN_CENTER ,COLUMN=1  $
      ,/NONEXCLUSIVE)

  
  CURR_SHOW_AVE_BTN = Widget_Button(WID_BASE_43,  $
      UNAME='CURR_SHOW_AVE_BTN' ,/ALIGN_LEFT ,VALUE='Show ave.'+ $
      ' command')

  
  WID_BASE_44 = Widget_Base(CURR_BASE, UNAME='WID_BASE_44' ,XOFFSET=3  $
      ,YOFFSET=430 ,TITLE='IDL' ,SPACE=3 ,XPAD=3 ,YPAD=3 ,COLUMN=1)

  
  WID_BASE_49 = Widget_Base(WID_BASE_44, UNAME='WID_BASE_49'  $
      ,XOFFSET=3 ,YOFFSET=3 ,TITLE='IDL' ,SPACE=3 ,XPAD=3 ,YPAD=3  $
      ,ROW=1)

  
  WID_LABEL_53 = Widget_Label(WID_BASE_49, UNAME='WID_LABEL_53'  $
      ,XOFFSET=3 ,YOFFSET=3 ,/ALIGN_LEFT ,VALUE='Sets to ave.:')

  
  CURR_NUM_SET_TO_AVE = Widget_Slider(WID_BASE_49,  $
      UNAME='CURR_NUM_SET_TO_AVE' ,XOFFSET=68 ,YOFFSET=3 ,MINIMUM=1  $
      ,MAXIMUM=1000)

  
  COMM_BASE = Widget_Base(WID_BASE_1, UNAME='COMM_BASE' ,XOFFSET=381  $
      ,YOFFSET=3 ,SPACE=3 ,XPAD=3 ,YPAD=3 ,COLUMN=1)

  
  xol_gui_comm_base_post_creation, COMM_BASE, _EXTRA=_VWBExtra_

  
  COMM_LABEL = Widget_Label(COMM_BASE, UNAME='COMM_LABEL' ,XOFFSET=68  $
      ,YOFFSET=3 ,/ALIGN_LEFT ,VALUE='Command')

  
  COMM_DRAW = Widget_Draw(COMM_BASE, UNAME='COMM_DRAW' ,XOFFSET=3  $
      ,YOFFSET=21 ,SCR_XSIZE=180 ,SCR_YSIZE=180)

  
  WID_BASE_55 = Widget_Base(COMM_BASE, UNAME='WID_BASE_55'  $
      ,XOFFSET=14 ,YOFFSET=204 ,/ALIGN_CENTER ,TITLE='IDL' ,SPACE=3  $
      ,XPAD=3 ,YPAD=3 ,ROW=1)

  
  WID_LABEL_54 = Widget_Label(WID_BASE_55, UNAME='WID_LABEL_54'  $
      ,XOFFSET=3 ,YOFFSET=3 ,/ALIGN_LEFT ,VALUE='Max:')

  
  COMM_MAX_FLD = Widget_Text(WID_BASE_55, UNAME='COMM_MAX_FLD'  $
      ,XOFFSET=31 ,YOFFSET=3 ,/NO_NEWLINE ,VALUE=[ '0' ] ,XSIZE=20  $
      ,YSIZE=1)

  
  WID_BASE_56 = Widget_Base(COMM_BASE, UNAME='WID_BASE_56'  $
      ,XOFFSET=15 ,YOFFSET=234 ,/ALIGN_CENTER ,TITLE='IDL' ,SPACE=3  $
      ,XPAD=3 ,YPAD=3 ,ROW=1)

  
  WID_LABEL_55 = Widget_Label(WID_BASE_56, UNAME='WID_LABEL_55'  $
      ,XOFFSET=3 ,YOFFSET=3 ,/ALIGN_LEFT ,VALUE='Min:')

  
  COMM_MIN_FLD = Widget_Text(WID_BASE_56, UNAME='COMM_MIN_FLD'  $
      ,XOFFSET=28 ,YOFFSET=3 ,/NO_NEWLINE ,VALUE=[ '0' ] ,XSIZE=20  $
      ,YSIZE=1)

  
  WID_BASE_57 = Widget_Base(COMM_BASE, UNAME='WID_BASE_57'  $
      ,XOFFSET=13 ,YOFFSET=264 ,/ALIGN_CENTER ,TITLE='IDL' ,SPACE=3  $
      ,XPAD=3 ,YPAD=3 ,COLUMN=1)

  
  WID_BASE_2 = Widget_Base(WID_BASE_57, UNAME='WID_BASE_2' ,XOFFSET=3  $
      ,YOFFSET=3 ,/ALIGN_RIGHT ,TITLE='IDL' ,SPACE=3 ,XPAD=3 ,YPAD=3  $
      ,ROW=1)

  
  WID_LABEL_69 = Widget_Label(WID_BASE_2, UNAME='WID_LABEL_69'  $
      ,XOFFSET=3 ,YOFFSET=6 ,/ALIGN_CENTER ,VALUE='UB:')

  
  COMM_MAX_CBAR_FLD = Widget_Text(WID_BASE_2,  $
      UNAME='COMM_MAX_CBAR_FLD' ,XOFFSET=26 ,YOFFSET=3 ,VALUE=[  $
      '32767' ] ,XSIZE=20 ,YSIZE=1)

  
  WID_BASE_3 = Widget_Base(WID_BASE_57, UNAME='WID_BASE_3' ,XOFFSET=5  $
      ,YOFFSET=33 ,/ALIGN_RIGHT ,TITLE='IDL' ,SPACE=3 ,XPAD=3 ,YPAD=3  $
      ,ROW=1)

  
  WID_LABEL_70 = Widget_Label(WID_BASE_3, UNAME='WID_LABEL_70'  $
      ,XOFFSET=3 ,YOFFSET=6 ,/ALIGN_CENTER ,VALUE='LB:')

  
  COMM_MIN_CBAR_FLD = Widget_Text(WID_BASE_3,  $
      UNAME='COMM_MIN_CBAR_FLD' ,XOFFSET=24 ,YOFFSET=3 ,/ALL_EVENTS  $
      ,VALUE=[ '-32768' ] ,XSIZE=20 ,YSIZE=1)

  
  COMM_OPT_CUTS_BTN = Widget_Button(WID_BASE_57,  $
      UNAME='COMM_OPT_CUTS_BTN' ,XOFFSET=50 ,YOFFSET=63  $
      ,/ALIGN_CENTER ,VALUE='Opt. cuts')

  
  WID_BASE_58 = Widget_Base(COMM_BASE, UNAME='WID_BASE_58'  $
      ,XOFFSET=10 ,YOFFSET=355 ,/ALIGN_CENTER ,TITLE='IDL' ,SPACE=3  $
      ,XPAD=3 ,YPAD=3 ,ROW=1)

  
  COMM_COLOR_BAR = Widget_Draw(WID_BASE_58, UNAME='COMM_COLOR_BAR'  $
      ,XOFFSET=3 ,YOFFSET=3 ,SCR_XSIZE=160 ,SCR_YSIZE=10)

  
  WID_BASE_59 = Widget_Base(COMM_BASE, UNAME='WID_BASE_59'  $
      ,XOFFSET=27 ,YOFFSET=374 ,/ALIGN_CENTER ,TITLE='IDL' ,SPACE=3  $
      ,XPAD=3 ,YPAD=3 ,ROW=1)

  
  WID_BASE_60 = Widget_Base(WID_BASE_59, UNAME='WID_BASE_60'  $
      ,XOFFSET=3 ,YOFFSET=3 ,COLUMN=1 ,/NONEXCLUSIVE)

  
  COMM_SET_OFFSET_BTN = Widget_Button(WID_BASE_60,  $
      UNAME='COMM_SET_OFFSET_BTN' ,/ALIGN_LEFT ,VALUE='Offset')

  
  COMM_NEW_OFFSET_BTN = Widget_Button(WID_BASE_59,  $
      UNAME='COMM_NEW_OFFSET_BTN' ,XOFFSET=62 ,YOFFSET=3  $
      ,/ALIGN_CENTER ,VALUE='New offset')

  
  WID_BASE_61 = Widget_Base(COMM_BASE, UNAME='WID_BASE_61'  $
      ,XOFFSET=29 ,YOFFSET=405 ,/ALIGN_CENTER ,COLUMN=1  $
      ,/NONEXCLUSIVE)

  
  COMM_SHOW_AVE_BTN = Widget_Button(WID_BASE_61,  $
      UNAME='COMM_SHOW_AVE_BTN' ,/ALIGN_LEFT ,VALUE='Show ave.'+ $
      ' command')

  
  WID_BASE_62 = Widget_Base(COMM_BASE, UNAME='WID_BASE_62' ,XOFFSET=3  $
      ,YOFFSET=430 ,TITLE='IDL' ,SPACE=3 ,XPAD=3 ,YPAD=3 ,COLUMN=1)

  
  WID_BASE_63 = Widget_Base(WID_BASE_62, UNAME='WID_BASE_63'  $
      ,XOFFSET=3 ,YOFFSET=3 ,TITLE='IDL' ,SPACE=3 ,XPAD=3 ,YPAD=3  $
      ,ROW=1)

  
  WID_LABEL_56 = Widget_Label(WID_BASE_63, UNAME='WID_LABEL_56'  $
      ,XOFFSET=3 ,YOFFSET=3 ,/ALIGN_LEFT ,VALUE='Sets to ave.:')

  
  COMM_NUM_SET_TO_AVE = Widget_Slider(WID_BASE_63,  $
      UNAME='COMM_NUM_SET_TO_AVE' ,XOFFSET=68 ,YOFFSET=3 ,MINIMUM=1  $
      ,MAXIMUM=1000)
  
  APPLY_AVE_COMMAND_BTN = Widget_Button(COMM_BASE,  $
      UNAME='APPLY_AVE_COMMAND_BTN' ,XOFFSET=3 ,YOFFSET=3  $
      ,/ALIGN_CENTER ,VALUE='Apply ave. command')

  
  RMS_BASE = Widget_Base(WID_BASE_1, UNAME='RMS_BASE'  $
      ,XOFFSET=570 ,YOFFSET=3 ,SPACE=3 ,XPAD=3 ,YPAD=3 ,COLUMN=1)

  
  xol_gui_comm_base_post_creation, RMS_BASE, _EXTRA=_VWBExtra_

  
  RMS_LABEL = Widget_Label(RMS_BASE, UNAME='RMS_LABEL'  $
      ,XOFFSET=72 ,YOFFSET=3 ,/ALIGN_LEFT ,VALUE='PositionRMS           ')

  
  RMS_DRAW = Widget_Draw(RMS_BASE, UNAME='RMS_DRAW'  $
      ,XOFFSET=3 ,YOFFSET=21 ,SCR_XSIZE=180 ,SCR_YSIZE=180)

  
  WID_BASE_64 = Widget_Base(RMS_BASE, UNAME='WID_BASE_64'  $
      ,XOFFSET=14 ,YOFFSET=204 ,/ALIGN_CENTER ,TITLE='IDL' ,SPACE=3  $
      ,XPAD=3 ,YPAD=3 ,ROW=1)

  
  WID_LABEL_57 = Widget_Label(WID_BASE_64, UNAME='WID_LABEL_57'  $
      ,XOFFSET=3 ,YOFFSET=3 ,/ALIGN_LEFT ,VALUE='Max:')

  
  RMS_MAX_FLD = Widget_Text(WID_BASE_64, UNAME='RMS_MAX_FLD'  $
      ,XOFFSET=31 ,YOFFSET=3 ,/NO_NEWLINE ,VALUE=[ '0' ] ,XSIZE=20  $
      ,YSIZE=1)

  
  WID_BASE_65 = Widget_Base(RMS_BASE, UNAME='WID_BASE_65'  $
      ,XOFFSET=15 ,YOFFSET=234 ,/ALIGN_CENTER ,TITLE='IDL' ,SPACE=3  $
      ,XPAD=3 ,YPAD=3 ,ROW=1)

  
  WID_LABEL_61 = Widget_Label(WID_BASE_65, UNAME='WID_LABEL_61'  $
      ,XOFFSET=3 ,YOFFSET=3 ,/ALIGN_LEFT ,VALUE='Min:')

  
  RMS_MIN_FLD = Widget_Text(WID_BASE_65, UNAME='RMS_MIN_FLD'  $
      ,XOFFSET=28 ,YOFFSET=3 ,/NO_NEWLINE ,VALUE=[ '0' ] ,XSIZE=20  $
      ,YSIZE=1)

  
  WID_BASE_66 = Widget_Base(RMS_BASE, UNAME='WID_BASE_66'  $
      ,XOFFSET=13 ,YOFFSET=264 ,/ALIGN_CENTER ,TITLE='IDL' ,SPACE=3  $
      ,XPAD=3 ,YPAD=3 ,COLUMN=1)

  
  WID_BASE_10 = Widget_Base(WID_BASE_66, UNAME='WID_BASE_10'  $
      ,XOFFSET=3 ,YOFFSET=3 ,/ALIGN_RIGHT ,TITLE='IDL' ,SPACE=3  $
      ,XPAD=3 ,YPAD=3 ,ROW=1)

  
  WID_LABEL_71 = Widget_Label(WID_BASE_10, UNAME='WID_LABEL_71'  $
      ,XOFFSET=3 ,YOFFSET=6 ,/ALIGN_CENTER ,VALUE='UB:')

  
  RMS_MAX_CBAR_FLD = Widget_Text(WID_BASE_10,  $
      UNAME='RMS_MAX_CBAR_FLD' ,XOFFSET=26 ,YOFFSET=3 ,VALUE=[  $
      '32767' ] ,XSIZE=20 ,YSIZE=1)

  
  WID_BASE_11 = Widget_Base(WID_BASE_66, UNAME='WID_BASE_11'  $
      ,XOFFSET=5 ,YOFFSET=33 ,/ALIGN_RIGHT ,TITLE='IDL' ,SPACE=3  $
      ,XPAD=3 ,YPAD=3 ,ROW=1)

  
  WID_LABEL_72 = Widget_Label(WID_BASE_11, UNAME='WID_LABEL_72'  $
      ,XOFFSET=3 ,YOFFSET=6 ,/ALIGN_CENTER ,VALUE='LB:')

  
  RMS_MIN_CBAR_FLD = Widget_Text(WID_BASE_11,  $
      UNAME='RMS_MIN_CBAR_FLD' ,XOFFSET=24 ,YOFFSET=3 ,/ALL_EVENTS  $
      ,VALUE=[ '-32768' ] ,XSIZE=20 ,YSIZE=1)

  
  RMS_OPT_CUTS_BTN = Widget_Button(WID_BASE_66,  $
      UNAME='RMS_OPT_CUTS_BTN' ,XOFFSET=50 ,YOFFSET=63  $
      ,/ALIGN_CENTER ,VALUE='Opt. cuts')

  
  WID_BASE_67 = Widget_Base(RMS_BASE, UNAME='WID_BASE_67'  $
      ,XOFFSET=10 ,YOFFSET=355 ,/ALIGN_CENTER ,TITLE='IDL' ,SPACE=3  $
      ,XPAD=3 ,YPAD=3 ,ROW=1)

  
  RMS_COLOR_BAR = Widget_Draw(WID_BASE_67,  $
      UNAME='RMS_COLOR_BAR' ,XOFFSET=3 ,YOFFSET=3 ,SCR_XSIZE=160  $
      ,SCR_YSIZE=10)

  
  WID_BASE_68 = Widget_Base(RMS_BASE, UNAME='WID_BASE_68'  $
      ,XOFFSET=27 ,YOFFSET=374 ,/ALIGN_CENTER ,TITLE='IDL' ,SPACE=3  $
      ,XPAD=3 ,YPAD=3 ,ROW=1)

  
  WID_BASE_69 = Widget_Base(WID_BASE_68, UNAME='WID_BASE_69'  $
      ,XOFFSET=3 ,YOFFSET=3 ,COLUMN=1 ,/NONEXCLUSIVE)

  
  RMS_SET_OFFSET_BTN = Widget_Button(WID_BASE_69,  $
      UNAME='RMS_SET_OFFSET_BTN' ,/ALIGN_LEFT ,VALUE='Offset')

  
  RMS_NEW_OFFSET_BTN = Widget_Button(WID_BASE_68,  $
      UNAME='RMS_NEW_OFFSET_BTN' ,XOFFSET=62 ,YOFFSET=3  $
      ,/ALIGN_CENTER ,VALUE='New offset')

  
  WID_BASE_70 = Widget_Base(RMS_BASE, UNAME='WID_BASE_70'  $
      ,XOFFSET=29 ,YOFFSET=405 ,/ALIGN_CENTER ,COLUMN=1  $
      ,/NONEXCLUSIVE)

  
  RMS_SHOW_AVE_BTN = Widget_Button(WID_BASE_70,  $
      UNAME='RMS_SHOW_AVE_BTN' ,/ALIGN_LEFT ,VALUE='Show ave.'+ $
      ' command')

  
  WID_BASE_71 = Widget_Base(RMS_BASE, UNAME='WID_BASE_71'  $
      ,XOFFSET=3 ,YOFFSET=430 ,TITLE='IDL' ,SPACE=3 ,XPAD=3 ,YPAD=3  $
      ,COLUMN=1)

  
  WID_BASE_72 = Widget_Base(WID_BASE_71, UNAME='WID_BASE_72'  $
      ,XOFFSET=3 ,YOFFSET=3 ,TITLE='IDL' ,SPACE=3 ,XPAD=3 ,YPAD=3  $
      ,ROW=1)

  
  WID_LABEL_62 = Widget_Label(WID_BASE_72, UNAME='WID_LABEL_62'  $
      ,XOFFSET=3 ,YOFFSET=3 ,/ALIGN_LEFT ,VALUE='Sets to ave.:')

  
  RMS_NUM_SET_TO_AVE = Widget_Slider(WID_BASE_72,  $
      UNAME='RMS_NUM_SET_TO_AVE' ,XOFFSET=68 ,YOFFSET=3 ,MINIMUM=1  $
      ,MAXIMUM=1000)

  
  WID_BASE_12 = Widget_Base(WID_BASE_0, UNAME='WID_BASE_12'  $
      ,XOFFSET=3 ,YOFFSET=603 ,TITLE='IDL' ,SPACE=3 ,XPAD=3 ,YPAD=3  $
      ,ROW=1)

  
  MODAL_DECOMP_DRAW = Widget_Draw(WID_BASE_12,  $
      UNAME='MODAL_DECOMP_DRAW' ,XOFFSET=3 ,YOFFSET=3 ,SCR_XSIZE=754  $
      ,SCR_YSIZE=203)

  
  WID_BASE_13 = Widget_Base(WID_BASE_12, UNAME='WID_BASE_13'  $
      ,XOFFSET=760 ,YOFFSET=3 ,TITLE='IDL' ,SPACE=3 ,XPAD=3 ,YPAD=3  $
      ,COLUMN=1)

  
  WID_LABEL_12 = Widget_Label(WID_BASE_13, UNAME='WID_LABEL_12'  $
      ,XOFFSET=3 ,YOFFSET=3 ,/ALIGN_LEFT ,VALUE='Modal'+ $
      ' decomposition')

  
  MODAL_DECOMP_DLIST = Widget_Combobox(WID_BASE_13,  $
      UNAME='MODAL_DECOMP_DLIST' ,XOFFSET=3 ,YOFFSET=21 ,VALUE=[  $
      'No plot', 'Position', 'Current', 'Command', 'Rec.'+ $
      ' delta-command' ])

  
  WID_LABEL_27 = Widget_Label(WID_BASE_13, UNAME='WID_LABEL_27'  $
      ,XOFFSET=34 ,YOFFSET=46 ,/ALIGN_CENTER ,VALUE='Range [nm]:')

  
  MODAL_DECOMP_YRANGE_SLIDE = Widget_Slider(WID_BASE_13,  $
      UNAME='MODAL_DECOMP_YRANGE_SLIDE' ,XOFFSET=3 ,YOFFSET=64  $
      ,MINIMUM=100 ,MAXIMUM=5000 ,VALUE=500)

  Widget_Control, /REALIZE, WID_BASE_0

  XManager, 'WID_BASE_0', WID_BASE_0, /NO_BLOCK  

end
; 
; Empty stub procedure used for autoloading.
; 
pro xol_gui, GROUP_LEADER=wGroup, _EXTRA=_VWBExtra_
    WID_BASE_0, GROUP_LEADER=wGroup, _EXTRA=_VWBExtra_
end
