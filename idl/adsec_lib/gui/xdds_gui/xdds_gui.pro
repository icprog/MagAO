; 
; IDL Widget Interface Procedures. This Code is automatically 
;     generated and should not be modified.

; 
; Generated on:	01/17/2003 00:16.55
; 
pro WID_BASE_0_event, Event

  wWidget =  Event.top

  case Event.id of

    Widget_Info(wWidget, FIND_BY_UNAME='REFRESH_TIME_FLD'): begin
      if( Tag_Names(Event, /STRUCTURE_NAME) eq 'WIDGET_TEXT_STR' )then $
        xdds_gui_update_refresh_time, Event
    end
    Widget_Info(wWidget, FIND_BY_UNAME='POS_OPT_CUTS_BTN'): begin
      if( Tag_Names(Event, /STRUCTURE_NAME) eq 'WIDGET_BUTTON' )then $
        xdds_gui_pos_opt_cuts, Event
    end
    Widget_Info(wWidget, FIND_BY_UNAME='POS_SET_OFFSET_BTN'): begin
      if( Tag_Names(Event, /STRUCTURE_NAME) eq 'WIDGET_BUTTON' )then $
        xdds_gui_pos_set_offset, Event
    end
    Widget_Info(wWidget, FIND_BY_UNAME='POS_NEW_OFFSET_BTN'): begin
      if( Tag_Names(Event, /STRUCTURE_NAME) eq 'WIDGET_BUTTON' )then $
        pos_new_offset, Event
    end
    Widget_Info(wWidget, FIND_BY_UNAME='POS_SHOW_AVE_BTN'): begin
      if( Tag_Names(Event, /STRUCTURE_NAME) eq 'WIDGET_BUTTON' )then $
        xdds_gui_pos_show_ave, Event
    end
    Widget_Info(wWidget, FIND_BY_UNAME='CURR_OPT_CUTS_BTN'): begin
      if( Tag_Names(Event, /STRUCTURE_NAME) eq 'WIDGET_BUTTON' )then $
        xdds_gui_curr_opt_cuts, Event
    end
    Widget_Info(wWidget, FIND_BY_UNAME='CURR_NEW_OFFSET_BTN'): begin
      if( Tag_Names(Event, /STRUCTURE_NAME) eq 'WIDGET_BUTTON' )then $
        curr_new_offset, Event
    end
    Widget_Info(wWidget, FIND_BY_UNAME='CURR_SET_OFFSET_BTN'): begin
      if( Tag_Names(Event, /STRUCTURE_NAME) eq 'WIDGET_BUTTON' )then $
        xdds_gui_curr_set_offset, Event
    end
    Widget_Info(wWidget, FIND_BY_UNAME='CURR_SHOW_AVE_BTN'): begin
      if( Tag_Names(Event, /STRUCTURE_NAME) eq 'WIDGET_BUTTON' )then $
        xdds_gui_curr_show_ave, Event
    end
    Widget_Info(wWidget, FIND_BY_UNAME='MINMAX_OPT_CUTS_BTN'): begin
      if( Tag_Names(Event, /STRUCTURE_NAME) eq 'WIDGET_BUTTON' )then $
        xdds_gui_minmax_opt_cuts, Event
    end
    Widget_Info(wWidget, FIND_BY_UNAME='MINMAX_SHOW_AVE_BTN'): begin
      if( Tag_Names(Event, /STRUCTURE_NAME) eq 'WIDGET_BUTTON' )then $
        xdds_gui_minmax_show_ave, Event
    end
    Widget_Info(wWidget, FIND_BY_UNAME='SPOT_OPT_CUTS_BTN'): begin
      if( Tag_Names(Event, /STRUCTURE_NAME) eq 'WIDGET_BUTTON' )then $
        xdds_gui_spot_opt_cuts, Event
    end
    Widget_Info(wWidget, FIND_BY_UNAME='TAKE_BACKGROUND_BTN'): begin
      if( Tag_Names(Event, /STRUCTURE_NAME) eq 'WIDGET_BUTTON' )then $
        xdds_take_background, Event
    end
    Widget_Info(wWidget, FIND_BY_UNAME='START_STOP_READ_BTN'): begin
      if( Tag_Names(Event, /STRUCTURE_NAME) eq 'WIDGET_TIMER' )then $
        xdds_gui_on_timer, Event
      if( Tag_Names(Event, /STRUCTURE_NAME) eq 'WIDGET_BUTTON' )then $
        xdds_gui_start_stop, Event
    end
    Widget_Info(wWidget, FIND_BY_UNAME='COMM_OPT_CUTS_BTN'): begin
      if( Tag_Names(Event, /STRUCTURE_NAME) eq 'WIDGET_BUTTON' )then $
        xdds_gui_comm_opt_cuts, Event
    end
    Widget_Info(wWidget, FIND_BY_UNAME='COMM_NEW_OFFSET_BTN'): begin
      if( Tag_Names(Event, /STRUCTURE_NAME) eq 'WIDGET_BUTTON' )then $
        comm_new_offset, Event
    end
    Widget_Info(wWidget, FIND_BY_UNAME='COMM_SET_OFFSET_BTN'): begin
      if( Tag_Names(Event, /STRUCTURE_NAME) eq 'WIDGET_BUTTON' )then $
        xdds_gui_comm_set_offset, Event
    end
    Widget_Info(wWidget, FIND_BY_UNAME='APPLY_AVE_COMMAND_BTN'): begin
      if( Tag_Names(Event, /STRUCTURE_NAME) eq 'WIDGET_BUTTON' )then $
        xdds_apply_ave_command, Event
    end
    Widget_Info(wWidget, FIND_BY_UNAME='COMM_SHOW_AVE_BTN'): begin
      if( Tag_Names(Event, /STRUCTURE_NAME) eq 'WIDGET_BUTTON' )then $
        xdds_gui_comm_show_ave, Event
    end
    Widget_Info(wWidget, FIND_BY_UNAME='EXIT_BTN'): begin
      if( Tag_Names(Event, /STRUCTURE_NAME) eq 'WIDGET_BUTTON' )then $
        xdds_gui_exit, Event
    end
    Widget_Info(wWidget, FIND_BY_UNAME='READ_FROM_DDS_BTN'): begin
      if( Tag_Names(Event, /STRUCTURE_NAME) eq 'WIDGET_BUTTON' )then $
        xdds_gui_read_from_dds, Event
    end
    Widget_Info(wWidget, FIND_BY_UNAME='OFFLOAD_TILT_BTN'): begin
      if( Tag_Names(Event, /STRUCTURE_NAME) eq 'WIDGET_BUTTON' )then $
        xdds_gui_offload_tilt, Event
    end
    Widget_Info(wWidget, FIND_BY_UNAME='REC_DCOMM_BTN'): begin
      if( Tag_Names(Event, /STRUCTURE_NAME) eq 'WIDGET_BUTTON' )then $
        rec_dcomm_press_btn, Event
    end
    Widget_Info(wWidget, FIND_BY_UNAME='MODAL_DECOMP_DLIST'): begin
      if( Tag_Names(Event, /STRUCTURE_NAME) eq 'WIDGET_DROPLIST' )then $
        xdds_gui_modal_decomp_select, Event
    end
    else:
  endcase

end

pro WID_BASE_0, GROUP_LEADER=wGroup, _EXTRA=_VWBExtra_

  Resolve_Routine, 'xdds_gui_eventcb',/COMPILE_FULL_FILE  ; Load event callback routines
  
  WID_BASE_0 = Widget_Base( GROUP_LEADER=wGroup, UNAME='WID_BASE_0'  $
      ,XOFFSET=5 ,YOFFSET=5 ,SCR_XSIZE=988 ,SCR_YSIZE=700  $
      ,TITLE='XDDS GUI' ,SPACE=3 ,XPAD=3 ,YPAD=3)

  
  xdds_gui_post_creation, WID_BASE_0, _EXTRA=_VWBExtra_

  
  WID_BASE_1 = Widget_Base(WID_BASE_0, UNAME='WID_BASE_1' ,XOFFSET=5  $
      ,YOFFSET=4 ,SCR_XSIZE=107 ,SCR_YSIZE=50 ,SPACE=3 ,XPAD=3  $
      ,YPAD=3)

  
  REFRESH_TIME_FLD = Widget_Text(WID_BASE_1, UNAME='REFRESH_TIME_FLD'  $
      ,XOFFSET=3 ,YOFFSET=7 ,/EDITABLE ,/ALL_EVENTS ,VALUE=[ '1.0' ]  $
      ,XSIZE=20 ,YSIZE=1)

  
  WID_LABEL_0 = Widget_Label(WID_BASE_1, UNAME='WID_LABEL_0'  $
      ,XOFFSET=12 ,YOFFSET=27 ,/ALIGN_LEFT ,VALUE='Refresh time [s]')

  
  POS_BASE = Widget_Base(WID_BASE_0, UNAME='POS_BASE' ,XOFFSET=7  $
      ,YOFFSET=60 ,SCR_XSIZE=187 ,SCR_YSIZE=399 ,SPACE=3 ,XPAD=3  $
      ,YPAD=3)

  
  xdds_gui_pos_base_post_creation, POS_BASE, _EXTRA=_VWBExtra_

  
  POS_DRAW = Widget_Draw(POS_BASE, UNAME='POS_DRAW' ,XOFFSET=2  $
      ,YOFFSET=21 ,SCR_XSIZE=180 ,SCR_YSIZE=180)

  
  WID_LABEL_1 = Widget_Label(POS_BASE, UNAME='WID_LABEL_1' ,XOFFSET=1  $
      ,YOFFSET=3 ,/ALIGN_LEFT ,VALUE='Position')

  
  POS_MAX_FLD = Widget_Text(POS_BASE, UNAME='POS_MAX_FLD' ,XOFFSET=41  $
      ,YOFFSET=208 ,/NO_NEWLINE ,VALUE=[ '0' ] ,XSIZE=20 ,YSIZE=1)

  
  POS_MIN_FLD = Widget_Text(POS_BASE, UNAME='POS_MIN_FLD' ,XOFFSET=41  $
      ,YOFFSET=227 ,/NO_NEWLINE ,VALUE=[ '0' ] ,XSIZE=20 ,YSIZE=1)

  
  WID_LABEL_2 = Widget_Label(POS_BASE, UNAME='WID_LABEL_2'  $
      ,XOFFSET=14 ,YOFFSET=206 ,/ALIGN_LEFT ,VALUE='Max:')

  
  WID_LABEL_3 = Widget_Label(POS_BASE, UNAME='WID_LABEL_3'  $
      ,XOFFSET=14 ,YOFFSET=227 ,/ALIGN_LEFT ,VALUE='Min:')

  
  POS_OPT_CUTS_BTN = Widget_Button(POS_BASE, UNAME='POS_OPT_CUTS_BTN'  $
      ,XOFFSET=67 ,YOFFSET=266 ,/ALIGN_CENTER ,VALUE='Opt. cuts')

  
  POS_COLOR_BAR = Widget_Draw(POS_BASE, UNAME='POS_COLOR_BAR'  $
      ,XOFFSET=21 ,YOFFSET=253 ,SCR_XSIZE=160 ,SCR_YSIZE=10)

  
  POS_MIN_CBAR_FLD = Widget_Text(POS_BASE, UNAME='POS_MIN_CBAR_FLD'  $
      ,XOFFSET=22 ,YOFFSET=265 ,SCR_XSIZE=44 ,SCR_YSIZE=15  $
      ,/ALL_EVENTS ,VALUE=[ '-32768' ] ,XSIZE=20 ,YSIZE=1)

  
  POS_MAX_CBAR_FLD = Widget_Text(POS_BASE, UNAME='POS_MAX_CBAR_FLD'  $
      ,XOFFSET=133 ,YOFFSET=264 ,SCR_XSIZE=48 ,SCR_YSIZE=18 ,VALUE=[  $
      '32767' ] ,XSIZE=20 ,YSIZE=1)

  
  POS_SET_OFFSET_BASE = Widget_Base(POS_BASE,  $
      UNAME='POS_SET_OFFSET_BASE' ,XOFFSET=21 ,YOFFSET=292 ,COLUMN=1  $
      ,/NONEXCLUSIVE)

  
  POS_SET_OFFSET_BTN = Widget_Button(POS_SET_OFFSET_BASE,  $
      UNAME='POS_SET_OFFSET_BTN' ,/ALIGN_LEFT ,VALUE='Offset')

  
  POS_NEW_OFFSET_BTN = Widget_Button(POS_BASE,  $
      UNAME='POS_NEW_OFFSET_BTN' ,XOFFSET=115 ,YOFFSET=295  $
      ,/ALIGN_CENTER ,VALUE='New offset')

  
  POS_NUM_SET_TO_AVE = Widget_Slider(POS_BASE,  $
      UNAME='POS_NUM_SET_TO_AVE' ,XOFFSET=82 ,YOFFSET=341 ,MINIMUM=1  $
      ,MAXIMUM=1000)

  
  WID_LABEL_32 = Widget_Label(POS_BASE, UNAME='WID_LABEL_32'  $
      ,XOFFSET=17 ,YOFFSET=347 ,/ALIGN_LEFT ,VALUE='Sets to ave.:')

  
  WID_BASE_8 = Widget_Base(POS_BASE, UNAME='WID_BASE_8' ,XOFFSET=18  $
      ,YOFFSET=318 ,TITLE='IDL' ,COLUMN=1 ,/NONEXCLUSIVE)

  
  POS_SHOW_AVE_BTN = Widget_Button(WID_BASE_8,  $
      UNAME='POS_SHOW_AVE_BTN' ,/ALIGN_LEFT ,VALUE='Show ave.'+ $
      ' command')

  
  CURR_BASE = Widget_Base(WID_BASE_0, UNAME='CURR_BASE' ,XOFFSET=200  $
      ,YOFFSET=61 ,SCR_XSIZE=186 ,SCR_YSIZE=398 ,SPACE=3 ,XPAD=243  $
      ,YPAD=3)

  
  xdds_gui_curr_base_post_creation, CURR_BASE, _EXTRA=_VWBExtra_

  
  CURR_DRAW = Widget_Draw(CURR_BASE, UNAME='CURR_DRAW' ,XOFFSET=4  $
      ,YOFFSET=20 ,SCR_XSIZE=180 ,SCR_YSIZE=180)

  
  WID_LABEL_7 = Widget_Label(CURR_BASE, UNAME='WID_LABEL_7'  $
      ,XOFFSET=1 ,YOFFSET=3 ,/ALIGN_LEFT ,VALUE='Current')

  
  CURR_MAX_FLD = Widget_Text(CURR_BASE, UNAME='CURR_MAX_FLD'  $
      ,XOFFSET=43 ,YOFFSET=208 ,/NO_NEWLINE ,VALUE=[ '0' ] ,XSIZE=20  $
      ,YSIZE=1)

  
  CURR_MIN_FLD = Widget_Text(CURR_BASE, UNAME='CURR_MIN_FLD'  $
      ,XOFFSET=43 ,YOFFSET=228 ,/NO_NEWLINE ,VALUE=[ '0' ] ,XSIZE=20  $
      ,YSIZE=1)

  
  WID_LABEL_8 = Widget_Label(CURR_BASE, UNAME='WID_LABEL_8'  $
      ,XOFFSET=16 ,YOFFSET=206 ,/ALIGN_LEFT ,VALUE='Max:')

  
  WID_LABEL_9 = Widget_Label(CURR_BASE, UNAME='WID_LABEL_9'  $
      ,XOFFSET=16 ,YOFFSET=228 ,/ALIGN_LEFT ,VALUE='Min:')

  
  CURR_OPT_CUTS_BTN = Widget_Button(CURR_BASE,  $
      UNAME='CURR_OPT_CUTS_BTN' ,XOFFSET=67 ,YOFFSET=265  $
      ,/ALIGN_CENTER ,VALUE='Opt. cuts')

  
  CURR_MAX_CBAR_FLD = Widget_Text(CURR_BASE,  $
      UNAME='CURR_MAX_CBAR_FLD' ,XOFFSET=130 ,YOFFSET=264  $
      ,SCR_XSIZE=48 ,SCR_YSIZE=15 ,VALUE=[ '32767' ] ,XSIZE=20  $
      ,YSIZE=1)

  
  CURR_COLOR_BAR = Widget_Draw(CURR_BASE, UNAME='CURR_COLOR_BAR'  $
      ,XOFFSET=20 ,YOFFSET=252 ,SCR_XSIZE=160 ,SCR_YSIZE=10)

  
  CURR_MIN_CBAR_FLD = Widget_Text(CURR_BASE,  $
      UNAME='CURR_MIN_CBAR_FLD' ,XOFFSET=16 ,YOFFSET=261  $
      ,SCR_XSIZE=48 ,SCR_YSIZE=18 ,VALUE=[ '-32768' ] ,XSIZE=20  $
      ,YSIZE=1)

  
  CURR_NEW_OFFSET_BTN = Widget_Button(CURR_BASE,  $
      UNAME='CURR_NEW_OFFSET_BTN' ,XOFFSET=112 ,YOFFSET=291  $
      ,/ALIGN_CENTER ,VALUE='New offset')

  
  WID_BASE_9 = Widget_Base(CURR_BASE, UNAME='WID_BASE_9' ,XOFFSET=18  $
      ,YOFFSET=288 ,COLUMN=1 ,/NONEXCLUSIVE)

  
  CURR_SET_OFFSET_BTN = Widget_Button(WID_BASE_9,  $
      UNAME='CURR_SET_OFFSET_BTN' ,/ALIGN_LEFT ,VALUE='Offset')

  
  CURR_NUM_SET_TO_AVE = Widget_Slider(CURR_BASE,  $
      UNAME='CURR_NUM_SET_TO_AVE' ,XOFFSET=81 ,YOFFSET=338 ,MINIMUM=1  $
      ,MAXIMUM=1000)

  
  WID_LABEL_31 = Widget_Label(CURR_BASE, UNAME='WID_LABEL_31'  $
      ,XOFFSET=18 ,YOFFSET=342 ,/ALIGN_LEFT ,VALUE='Sets to ave.:')

  
  WID_BASE_5 = Widget_Base(CURR_BASE, UNAME='WID_BASE_5' ,XOFFSET=17  $
      ,YOFFSET=312 ,TITLE='IDL' ,COLUMN=1 ,/NONEXCLUSIVE)

  
  CURR_SHOW_AVE_BTN = Widget_Button(WID_BASE_5,  $
      UNAME='CURR_SHOW_AVE_BTN' ,/ALIGN_LEFT ,VALUE='Show ave.'+ $
      ' command')

  
  MINMAX_BASE = Widget_Base(WID_BASE_0, UNAME='MINMAX_BASE'  $
      ,XOFFSET=575 ,YOFFSET=62 ,SCR_XSIZE=190 ,SCR_YSIZE=398 ,SPACE=3  $
      ,XPAD=3 ,YPAD=3)

  
  xdds_gui_minmax_base_post_creation, MINMAX_BASE, _EXTRA=_VWBExtra_

  
  MINMAX_DRAW = Widget_Draw(MINMAX_BASE, UNAME='MINMAX_DRAW'  $
      ,XOFFSET=4 ,YOFFSET=24 ,SCR_XSIZE=180 ,SCR_YSIZE=180)

  
  MINMAX_LABEL = Widget_Label(MINMAX_BASE, UNAME='MINMAX_LABEL'  $
      ,XOFFSET=1 ,YOFFSET=3 ,/ALIGN_LEFT ,VALUE='Max-Min')

  
  MINMAX_MAX_FLD = Widget_Text(MINMAX_BASE, UNAME='MINMAX_MAX_FLD'  $
      ,XOFFSET=35 ,YOFFSET=210 ,/NO_NEWLINE ,VALUE=[ '0' ] ,XSIZE=20  $
      ,YSIZE=1)

  
  MINMAX_MIN_FLD = Widget_Text(MINMAX_BASE, UNAME='MINMAX_MIN_FLD'  $
      ,XOFFSET=36 ,YOFFSET=230 ,/NO_NEWLINE ,VALUE=[ '0' ] ,XSIZE=20  $
      ,YSIZE=1)

  
  WID_LABEL_11 = Widget_Label(MINMAX_BASE, UNAME='WID_LABEL_11'  $
      ,XOFFSET=8 ,YOFFSET=208 ,/ALIGN_LEFT ,VALUE='Max:')

  
  WID_LABEL_12 = Widget_Label(MINMAX_BASE, UNAME='WID_LABEL_12'  $
      ,XOFFSET=9 ,YOFFSET=230 ,/ALIGN_LEFT ,VALUE='Min:')

  
  MINMAX_OPT_CUTS_BTN = Widget_Button(MINMAX_BASE,  $
      UNAME='MINMAX_OPT_CUTS_BTN' ,XOFFSET=62 ,YOFFSET=266  $
      ,/ALIGN_CENTER ,VALUE='Opt. cuts')

  
  MINMAX_MAX_CBAR_FLD = Widget_Text(MINMAX_BASE,  $
      UNAME='MINMAX_MAX_CBAR_FLD' ,XOFFSET=129 ,YOFFSET=267  $
      ,SCR_XSIZE=46 ,SCR_YSIZE=15 ,VALUE=[ '65535' ] ,XSIZE=20  $
      ,YSIZE=1)

  
  MINMAX_MIN_CBAR_FLD = Widget_Text(MINMAX_BASE,  $
      UNAME='MINMAX_MIN_CBAR_FLD' ,XOFFSET=16 ,YOFFSET=265  $
      ,SCR_XSIZE=43 ,SCR_YSIZE=15 ,VALUE=[ '0' ] ,XSIZE=20 ,YSIZE=1)

  
  MINMAX_COLOR_BAR = Widget_Draw(MINMAX_BASE,  $
      UNAME='MINMAX_COLOR_BAR' ,XOFFSET=16 ,YOFFSET=252  $
      ,SCR_XSIZE=152 ,SCR_YSIZE=10)

  
  MINMAX_NUM_SET_TO_AVE = Widget_Slider(MINMAX_BASE,  $
      UNAME='MINMAX_NUM_SET_TO_AVE' ,XOFFSET=76 ,YOFFSET=336  $
      ,MINIMUM=1 ,MAXIMUM=1000)

  
  WID_LABEL_33 = Widget_Label(MINMAX_BASE, UNAME='WID_LABEL_33'  $
      ,XOFFSET=11 ,YOFFSET=342 ,/ALIGN_LEFT ,VALUE='Sets to ave.:')

  
  WID_BASE_11 = Widget_Base(MINMAX_BASE, UNAME='WID_BASE_11'  $
      ,XOFFSET=12 ,YOFFSET=313 ,TITLE='IDL' ,COLUMN=1 ,/NONEXCLUSIVE)

  
  MINMAX_SHOW_AVE_BTN = Widget_Button(WID_BASE_11,  $
      UNAME='MINMAX_SHOW_AVE_BTN' ,/ALIGN_LEFT ,VALUE='Show ave.'+ $
      ' command')

  
  SPOT_SLOPE_BASE = Widget_Base(WID_BASE_0, UNAME='SPOT_SLOPE_BASE'  $
      ,XOFFSET=768 ,YOFFSET=64 ,SCR_XSIZE=206 ,SCR_YSIZE=354 ,SPACE=3  $
      ,XPAD=3 ,YPAD=3)

  
  SPOT_SLOPE_DRAW = Widget_Draw(SPOT_SLOPE_BASE,  $
      UNAME='SPOT_SLOPE_DRAW' ,XOFFSET=3 ,YOFFSET=24 ,SCR_XSIZE=192  $
      ,SCR_YSIZE=192)

  
  WID_LABEL_13 = Widget_Label(SPOT_SLOPE_BASE, UNAME='WID_LABEL_13'  $
      ,XOFFSET=1 ,YOFFSET=3 ,/ALIGN_LEFT ,VALUE='Spots')

  
  FRAME_MAX_FLD = Widget_Text(SPOT_SLOPE_BASE, UNAME='FRAME_MAX_FLD'  $
      ,XOFFSET=69 ,YOFFSET=235 ,/NO_NEWLINE ,VALUE=[ '0' ] ,XSIZE=20  $
      ,YSIZE=1)

  
  FRAME_MIN_FLD = Widget_Text(SPOT_SLOPE_BASE, UNAME='FRAME_MIN_FLD'  $
      ,XOFFSET=68 ,YOFFSET=257 ,/NO_NEWLINE ,VALUE=[ '0' ] ,XSIZE=20  $
      ,YSIZE=1)

  
  WID_LABEL_14 = Widget_Label(SPOT_SLOPE_BASE, UNAME='WID_LABEL_14'  $
      ,XOFFSET=11 ,YOFFSET=233 ,/ALIGN_LEFT ,VALUE='Max:')

  
  WID_LABEL_15 = Widget_Label(SPOT_SLOPE_BASE, UNAME='WID_LABEL_15'  $
      ,XOFFSET=11 ,YOFFSET=254 ,/ALIGN_LEFT ,VALUE='Min:')

  
  SPOT_OPT_CUTS_BTN = Widget_Button(SPOT_SLOPE_BASE,  $
      UNAME='SPOT_OPT_CUTS_BTN' ,XOFFSET=65 ,YOFFSET=295  $
      ,/ALIGN_CENTER ,VALUE='Opt. cuts')

  
  SPOT_COLOR_BAR = Widget_Draw(SPOT_SLOPE_BASE,  $
      UNAME='SPOT_COLOR_BAR' ,XOFFSET=18 ,YOFFSET=280 ,SCR_XSIZE=152  $
      ,SCR_YSIZE=10)

  
  SPOT_MAX_CBAR_FLD = Widget_Text(SPOT_SLOPE_BASE,  $
      UNAME='SPOT_MAX_CBAR_FLD' ,XOFFSET=128 ,YOFFSET=295  $
      ,SCR_XSIZE=49 ,SCR_YSIZE=15 ,VALUE=[ '32767' ] ,XSIZE=20  $
      ,YSIZE=1)

  
  SPOT_MIN_CBAR_FLD = Widget_Text(SPOT_SLOPE_BASE,  $
      UNAME='SPOT_MIN_CBAR_FLD' ,XOFFSET=19 ,YOFFSET=293  $
      ,SCR_XSIZE=41 ,SCR_YSIZE=15 ,/ALL_EVENTS ,VALUE=[ '0' ]  $
      ,XSIZE=20 ,YSIZE=1)

  
  NUM_FRAME_TO_AVE = Widget_Slider(SPOT_SLOPE_BASE,  $
      UNAME='NUM_FRAME_TO_AVE' ,XOFFSET=96 ,YOFFSET=321 ,MINIMUM=1  $
      ,MAXIMUM=30)

  
  WID_LABEL_25 = Widget_Label(SPOT_SLOPE_BASE, UNAME='WID_LABEL_25'  $
      ,YOFFSET=332 ,/ALIGN_LEFT ,VALUE='Frames to average')

  
  TAKE_BACKGROUND_BTN = Widget_Button(SPOT_SLOPE_BASE,  $
      UNAME='TAKE_BACKGROUND_BTN' ,XOFFSET=104 ,/ALIGN_CENTER  $
      ,VALUE='Take background')

  
  START_STOP_READ_BTN = Widget_Button(WID_BASE_0,  $
      UNAME='START_STOP_READ_BTN' ,XOFFSET=129 ,YOFFSET=3  $
      ,SCR_XSIZE=37 ,SCR_YSIZE=21 ,/ALIGN_CENTER ,VALUE='Start')

  
  WID_BASE_6 = Widget_Base(WID_BASE_0, UNAME='WID_BASE_6'  $
      ,XOFFSET=236 ,YOFFSET=4 ,SCR_XSIZE=349 ,SCR_YSIZE=57 ,MAP=0  $
      ,SPACE=3 ,XPAD=3 ,YPAD=3)

  
  WID_LABEL_16 = Widget_Label(WID_BASE_6, UNAME='WID_LABEL_16'  $
      ,XOFFSET=3 ,YOFFSET=8 ,/ALIGN_LEFT ,VALUE='Act.mir #')

  
  WID_LABEL_17 = Widget_Label(WID_BASE_6, UNAME='WID_LABEL_17'  $
      ,YOFFSET=32 ,/ALIGN_LEFT ,VALUE='Act.dsp #')

  
  WID_TEXT_0 = Widget_Text(WID_BASE_6, UNAME='WID_TEXT_0' ,XOFFSET=48  $
      ,YOFFSET=7 ,XSIZE=20 ,YSIZE=1)

  
  ACT_DSP_NUM_FLD = Widget_Text(WID_BASE_6, UNAME='ACT_DSP_NUM_FLD'  $
      ,XOFFSET=49 ,YOFFSET=31 ,XSIZE=20 ,YSIZE=1)

  
  WID_LABEL_18 = Widget_Label(WID_BASE_6, UNAME='WID_LABEL_18'  $
      ,XOFFSET=116 ,YOFFSET=7 ,/ALIGN_LEFT ,VALUE='Position:')

  
  WID_LABEL_19 = Widget_Label(WID_BASE_6, UNAME='WID_LABEL_19'  $
      ,XOFFSET=116 ,YOFFSET=29 ,/ALIGN_LEFT ,VALUE='Current:')

  
  WID_LABEL_20 = Widget_Label(WID_BASE_6, UNAME='WID_LABEL_20'  $
      ,XOFFSET=244 ,YOFFSET=7 ,/ALIGN_LEFT ,VALUE='Command:')

  
  WID_LABEL_21 = Widget_Label(WID_BASE_6, UNAME='WID_LABEL_21'  $
      ,XOFFSET=228 ,YOFFSET=29 ,/ALIGN_LEFT ,VALUE='Max-Min Pos.:')

  
  ACT_POS_FLD = Widget_Text(WID_BASE_6, UNAME='ACT_POS_FLD'  $
      ,XOFFSET=158 ,YOFFSET=6 ,XSIZE=20 ,YSIZE=1)

  
  ACT_CURR_FLD = Widget_Text(WID_BASE_6, UNAME='ACT_CURR_FLD'  $
      ,XOFFSET=159 ,YOFFSET=29 ,XSIZE=20 ,YSIZE=1)

  
  ACT_COMM_FLD = Widget_Text(WID_BASE_6, UNAME='ACT_COMM_FLD'  $
      ,XOFFSET=296 ,YOFFSET=8 ,XSIZE=20 ,YSIZE=1)

  
  ACT_MINMAX_FLD = Widget_Text(WID_BASE_6, UNAME='ACT_MINMAX_FLD'  $
      ,XOFFSET=298 ,YOFFSET=30 ,XSIZE=20 ,YSIZE=1)

  
  COMM_BASE = Widget_Base(WID_BASE_0, UNAME='COMM_BASE' ,XOFFSET=389  $
      ,YOFFSET=62 ,SCR_XSIZE=184 ,SCR_YSIZE=400 ,SPACE=3 ,XPAD=3  $
      ,YPAD=3)

  
  xdds_gui_comm_base_post_creation, COMM_BASE, _EXTRA=_VWBExtra_

  
  COMM_DRAW = Widget_Draw(COMM_BASE, UNAME='COMM_DRAW' ,XOFFSET=2  $
      ,YOFFSET=24 ,SCR_XSIZE=180 ,SCR_YSIZE=180)

  
  WID_LABEL_22 = Widget_Label(COMM_BASE, UNAME='WID_LABEL_22'  $
      ,XOFFSET=1 ,YOFFSET=3 ,/ALIGN_LEFT ,VALUE='Command')

  
  COMM_MAX_FLD = Widget_Text(COMM_BASE, UNAME='COMM_MAX_FLD'  $
      ,XOFFSET=37 ,YOFFSET=209 ,/NO_NEWLINE ,VALUE=[ '0' ] ,XSIZE=20  $
      ,YSIZE=1)

  
  COMM_MIN_FLD = Widget_Text(COMM_BASE, UNAME='COMM_MIN_FLD'  $
      ,XOFFSET=38 ,YOFFSET=230 ,/NO_NEWLINE ,VALUE=[ '0' ] ,XSIZE=20  $
      ,YSIZE=1)

  
  WID_LABEL_23 = Widget_Label(COMM_BASE, UNAME='WID_LABEL_23'  $
      ,XOFFSET=10 ,YOFFSET=207 ,/ALIGN_LEFT ,VALUE='Max:')

  
  WID_LABEL_24 = Widget_Label(COMM_BASE, UNAME='WID_LABEL_24'  $
      ,XOFFSET=11 ,YOFFSET=227 ,SCR_XSIZE=22 ,SCR_YSIZE=18  $
      ,/ALIGN_LEFT ,VALUE='Min:')

  
  COMM_OPT_CUTS_BTN = Widget_Button(COMM_BASE,  $
      UNAME='COMM_OPT_CUTS_BTN' ,XOFFSET=62 ,YOFFSET=268  $
      ,/ALIGN_CENTER ,VALUE='Opt. cuts')

  
  COMM_MAX_CBAR_FLD = Widget_Text(COMM_BASE,  $
      UNAME='COMM_MAX_CBAR_FLD' ,XOFFSET=124 ,YOFFSET=267  $
      ,SCR_XSIZE=52 ,SCR_YSIZE=15 ,VALUE=[ '32767' ] ,XSIZE=20  $
      ,YSIZE=1)

  
  COMM_COLOR_BAR = Widget_Draw(COMM_BASE, UNAME='COMM_COLOR_BAR'  $
      ,XOFFSET=14 ,YOFFSET=253 ,SCR_XSIZE=152 ,SCR_YSIZE=10)

  
  COMM_MIN_CBAR_FLD = Widget_Text(COMM_BASE,  $
      UNAME='COMM_MIN_CBAR_FLD' ,XOFFSET=13 ,YOFFSET=266  $
      ,SCR_XSIZE=46 ,SCR_YSIZE=15 ,VALUE=[ '-32768' ] ,XSIZE=20  $
      ,YSIZE=1)

  
  COMM_NEW_OFFSET_BTN = Widget_Button(COMM_BASE,  $
      UNAME='COMM_NEW_OFFSET_BTN' ,XOFFSET=110 ,YOFFSET=293  $
      ,/ALIGN_CENTER ,VALUE='New offset')

  
  WID_BASE_10 = Widget_Base(COMM_BASE, UNAME='WID_BASE_10'  $
      ,XOFFSET=16 ,YOFFSET=290 ,COLUMN=1 ,/NONEXCLUSIVE)

  
  COMM_SET_OFFSET_BTN = Widget_Button(WID_BASE_10,  $
      UNAME='COMM_SET_OFFSET_BTN' ,/ALIGN_LEFT ,VALUE='Offset')

  
  COMM_NUM_SET_TO_AVE = Widget_Slider(COMM_BASE,  $
      UNAME='COMM_NUM_SET_TO_AVE' ,XOFFSET=81 ,YOFFSET=339 ,MINIMUM=1  $
      ,MAXIMUM=1000)

  
  APPLY_AVE_COMMAND_BTN = Widget_Button(COMM_BASE,  $
      UNAME='APPLY_AVE_COMMAND_BTN' ,XOFFSET=14 ,YOFFSET=370  $
      ,/ALIGN_CENTER ,VALUE='Apply ave. command')

  
  WID_BASE_3 = Widget_Base(COMM_BASE, UNAME='WID_BASE_3' ,XOFFSET=16  $
      ,YOFFSET=316 ,TITLE='IDL' ,COLUMN=1 ,/NONEXCLUSIVE)

  
  COMM_SHOW_AVE_BTN = Widget_Button(WID_BASE_3,  $
      UNAME='COMM_SHOW_AVE_BTN' ,/ALIGN_LEFT ,VALUE='Show ave.'+ $
      ' command')

  
  WID_LABEL_30 = Widget_Label(COMM_BASE, UNAME='WID_LABEL_30'  $
      ,XOFFSET=15 ,YOFFSET=345 ,/ALIGN_LEFT ,VALUE='Sets to ave.:')

  
  EXIT_BTN = Widget_Button(WID_BASE_0, UNAME='EXIT_BTN' ,XOFFSET=129  $
      ,YOFFSET=29 ,/ALIGN_CENTER ,VALUE='Exit')

  
  WID_BASE_7 = Widget_Base(WID_BASE_0, UNAME='WID_BASE_7'  $
      ,XOFFSET=187 ,YOFFSET=5 ,COLUMN=1 ,/NONEXCLUSIVE)

  
  READ_FROM_DDS_BTN = Widget_Button(WID_BASE_7,  $
      UNAME='READ_FROM_DDS_BTN' ,/ALIGN_LEFT ,VALUE='Read from DDS')

  
  POINTING_ERROR_BASE = Widget_Base(WID_BASE_0,  $
      UNAME='POINTING_ERROR_BASE' ,XOFFSET=451 ,YOFFSET=2  $
      ,SCR_XSIZE=519 ,SCR_YSIZE=56 ,SPACE=3 ,XPAD=3 ,YPAD=3)

  
  xdds_gui_pointing_error_creation, POINTING_ERROR_BASE, _EXTRA=_VWBExtra_

  
  WID_LABEL_4 = Widget_Label(POINTING_ERROR_BASE, UNAME='WID_LABEL_4'  $
      ,XOFFSET=7 ,YOFFSET=19 ,/ALIGN_LEFT ,VALUE='Azimuth:')

  
  WID_LABEL_5 = Widget_Label(POINTING_ERROR_BASE, UNAME='WID_LABEL_5'  $
      ,XOFFSET=7 ,YOFFSET=36 ,/ALIGN_LEFT ,VALUE='Elevation:')

  
  OFFLOAD_TILT_BTN = Widget_Button(POINTING_ERROR_BASE,  $
      UNAME='OFFLOAD_TILT_BTN' ,XOFFSET=167 ,YOFFSET=22  $
      ,/ALIGN_CENTER ,VALUE='Offload tilt now')

  
  DELTA_AZIMUTH_FLD = Widget_Text(POINTING_ERROR_BASE,  $
      UNAME='DELTA_AZIMUTH_FLD' ,XOFFSET=60 ,YOFFSET=17 ,XSIZE=20  $
      ,YSIZE=1)

  
  DELTA_ELEVATION_FLD = Widget_Text(POINTING_ERROR_BASE,  $
      UNAME='DELTA_ELEVATION_FLD' ,XOFFSET=60 ,YOFFSET=35 ,XSIZE=20  $
      ,YSIZE=1)

  
  WID_LABEL_6 = Widget_Label(POINTING_ERROR_BASE, UNAME='WID_LABEL_6'  $
      ,XOFFSET=3 ,YOFFSET=1 ,/ALIGN_LEFT ,VALUE='Pointing error'+ $
      ' [arcsec]:')

  
  WID_LABEL_27 = Widget_Label(POINTING_ERROR_BASE,  $
      UNAME='WID_LABEL_27' ,XOFFSET=265 ,YOFFSET=13 ,/ALIGN_LEFT  $
      ,VALUE='Hexapode z error [um]:')

  
  DELTA_FOCUS_FLD = Widget_Text(POINTING_ERROR_BASE,  $
      UNAME='DELTA_FOCUS_FLD' ,XOFFSET=265 ,YOFFSET=30 ,XSIZE=20  $
      ,YSIZE=1)

  
  OFFLOAD_AVE_SLIDER = Widget_Slider(POINTING_ERROR_BASE,  $
      UNAME='OFFLOAD_AVE_SLIDER' ,XOFFSET=402 ,YOFFSET=20 ,MINIMUM=1)

  
  WID_LABEL_28 = Widget_Label(POINTING_ERROR_BASE,  $
      UNAME='WID_LABEL_28' ,XOFFSET=401 ,/ALIGN_LEFT ,VALUE='Running'+ $
      ' mean steps:')

  
  WID_BASE_2 = Widget_Base(WID_BASE_0, UNAME='WID_BASE_2'  $
      ,XOFFSET=186 ,YOFFSET=32 ,COLUMN=1 ,/NONEXCLUSIVE)

  
  REC_DCOMM_BTN = Widget_Button(WID_BASE_2, UNAME='REC_DCOMM_BTN'  $
      ,/ALIGN_LEFT ,VALUE='Display rec. dcomm')

  
  MODAL_DECOMP_DRAW = Widget_Draw(WID_BASE_0,  $
      UNAME='MODAL_DECOMP_DRAW' ,XOFFSET=6 ,YOFFSET=466  $
      ,SCR_XSIZE=754 ,SCR_YSIZE=203)

  
  MODAL_DECOMP_DLIST = Widget_Droplist(WID_BASE_0,  $
      UNAME='MODAL_DECOMP_DLIST' ,XOFFSET=783 ,YOFFSET=512 ,VALUE=[  $
      'No plot', 'Position', 'Current', 'Command', 'Rec.'+ $
      ' delta-command' ])

  
  WID_LABEL_10 = Widget_Label(WID_BASE_0, UNAME='WID_LABEL_10'  $
      ,XOFFSET=776 ,YOFFSET=493 ,/ALIGN_LEFT ,VALUE='Modal'+ $
      ' decomposition')

  
  WID_LABEL_26 = Widget_Label(WID_BASE_0, UNAME='WID_LABEL_26'  $
      ,XOFFSET=780 ,YOFFSET=551 ,/ALIGN_LEFT ,VALUE='Range [nm]:')

  
  MODAL_DECOMP_YRANGE_SLIDE = Widget_Slider(WID_BASE_0,  $
      UNAME='MODAL_DECOMP_YRANGE_SLIDE' ,XOFFSET=785 ,YOFFSET=567  $
      ,MINIMUM=100 ,MAXIMUM=5000 ,VALUE=500)

  Widget_Control, /REALIZE, WID_BASE_0

  XManager, 'WID_BASE_0', WID_BASE_0, /NO_BLOCK  

end
; 
; Empty stub procedure used for autoloading.
; 
pro xdds_gui, GROUP_LEADER=wGroup, _EXTRA=_VWBExtra_
  WID_BASE_0, GROUP_LEADER=wGroup, _EXTRA=_VWBExtra_
end
