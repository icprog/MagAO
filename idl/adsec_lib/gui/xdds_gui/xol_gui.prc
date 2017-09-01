HEADER
; IDL Visual Widget Builder Resource file. Version 1
; Generated on:	03/06/2007 14:58.58
VERSION 1
END

WID_BASE_0 BASE 5 5 0 0
ALIGNCENTER
TLB
CAPTION "GUI-V2.0"
COLUMNS = 1
XPAD = 3
YPAD = 3
SPACE = 3
CHILDALIGNCENTER
BEGIN
  WID_BASE_15 BASE -1 -1 0 0
  ROWS = 1
  XPAD = 3
  YPAD = 3
  SPACE = 3
  CAPTION "IDL"
  BEGIN
    WID_BASE_7 BASE -1 -1 0 0
    ROWS = 1
    XPAD = 3
    YPAD = 3
    SPACE = 3
    CAPTION "IDL"
    BEGIN
      WID_BASE_16 BASE -1 -1 0 0
      COLUMNS = 1
      XPAD = 3
      YPAD = 3
      SPACE = 3
      BEGIN
        REFRESH_TIME_FLD TEXT -1 -1 0 0
        NUMITEMS = 1
        ITEM "1.0"
        EDITABLE
        ALLEVENTS
        ONINSERTSTRING "xdds_gui_update_refresh_time"
        WIDTH = 20
        HEIGHT = 1
        END
        WID_LABEL_38 LABEL -1 -1 0 0
        VALUE "Refresh time [s]"
        ALIGNCENTER
        END
      END
      WID_BASE_17 BASE -1 -1 0 0
      COLUMNS = 1
      XPAD = 3
      YPAD = 3
      SPACE = 3
      CAPTION "IDL"
      BEGIN
        START_STOP_READ_BTN PUSHBUTTON -1 -1 0 0
        VALUE "Start"
        ALIGNCENTER
        TIMEREVENT "xdds_gui_on_timer"
        ONACTIVATE "xdds_gui_start_stop"
        END
        EXIT_BTN PUSHBUTTON -1 -1 0 0
        VALUE "Exit"
        ALIGNCENTER
        ONACTIVATE "xdds_gui_exit"
        END
      END
      WID_BASE_18 BASE -1 -1 0 0
      COLUMNS = 1
      NONEXCLUSIVE
      CAPTION "IDL"
      BEGIN
        READ_FROM_DDS_BTN PUSHBUTTON -1 -1 0 0
        VALUE "Read from DDS"
        ALIGNLEFT
        ONACTIVATE "xdds_gui_read_from_dds"
        END
        REC_DCOMM_BTN PUSHBUTTON -1 -1 0 0
        VALUE "Display rec. dcomm"
        ALIGNLEFT
        ONACTIVATE "rec_dcomm_press_btn"
        END
      END
    END
    POINTING_ERROR_BASE BASE -1 -1 0 0
    ONPOSTCREATE "xdds_gui_pointing_error_creation"
    COLUMNS = 1
    XPAD = 3
    YPAD = 3
    SPACE = 3
    BEGIN
      WID_LABEL_39 LABEL -1 -1 0 0
      VALUE "Hexapode z error [um]:"
      ALIGNLEFT
      END
      DELTA_FOCUS_FLD TEXT -1 -1 0 0
      WIDTH = 20
      HEIGHT = 1
      END
      WID_LABEL_40 LABEL -1 -1 0 0
      VALUE "Running mean steps:"
      ALIGNCENTER
      END
      OFFLOAD_AVE_SLIDER SLIDER -1 -1 0 0
      MINIMUM = 1
      END
    END
    WID_BASE_4 BASE -1 -1 0 0
    COLUMNS = 1
    XPAD = 3
    YPAD = 3
    SPACE = 3
    CAPTION "IDL"
    BEGIN
      WID_LABEL_41 LABEL -1 -1 0 0
      VALUE "Pointing error [arcsec]:"
      ALIGNCENTER
      END
      WID_BASE_19 BASE -1 -1 0 0
      ALIGNRIGHT
      ROWS = 1
      XPAD = 3
      YPAD = 3
      SPACE = 3
      CAPTION "IDL"
      BEGIN
        WID_LABEL_42 LABEL -1 -1 0 0
        VALUE "Azimuth:"
        ALIGNLEFT
        END
        DELTA_AZIMUTH_FLD TEXT -1 -1 0 0
        WIDTH = 20
        HEIGHT = 1
        END
      END
      WID_BASE_20 BASE -1 -1 0 0
      ALIGNRIGHT
      ROWS = 1
      XPAD = 3
      YPAD = 3
      SPACE = 3
      CAPTION "IDL"
      BEGIN
        WID_LABEL_43 LABEL -1 -1 0 0
        VALUE "Elevation:"
        ALIGNLEFT
        END
        DELTA_ELEVATION_FLD TEXT -1 -1 0 0
        WIDTH = 20
        HEIGHT = 1
        END
      END
      OFFLOAD_TILT_BTN PUSHBUTTON -1 -1 0 0
      VALUE "Offload tilt now"
      ALIGNRIGHT
      ONACTIVATE "xdds_gui_offload_tilt"
      END
    END
  END
  WID_BASE_1 BASE -1 -1 0 0
  ROWS = 1
  XPAD = 3
  YPAD = 3
  SPACE = 3
  CAPTION "IDL"
  BEGIN
    POS_BASE BASE -1 -1 0 0
    ONPOSTCREATE "xdds_gui_pos_base_post_creation"
    COLUMNS = 1
    XPAD = 3
    YPAD = 3
    SPACE = 3
    BEGIN
      WID_LABEL_28 LABEL -1 -1 0 0
      VALUE "Position"
      ALIGNCENTER
      END
      POS_DRAW DRAW -1 -1 180 180
      END
      WID_BASE_28 BASE -1 -1 0 0
      ALIGNCENTER
      ROWS = 1
      XPAD = 3
      YPAD = 3
      SPACE = 3
      CAPTION "IDL"
      BEGIN
        WID_LABEL_30 LABEL -1 -1 0 0
        VALUE "Max:"
        ALIGNLEFT
        END
        POS_MAX_FLD TEXT -1 -1 0 0
        NUMITEMS = 1
        ITEM "0"
        NONEWLINE
        WIDTH = 20
        HEIGHT = 1
        END
      END
      WID_BASE_29 BASE -1 -1 0 0
      ALIGNCENTER
      ROWS = 1
      XPAD = 3
      YPAD = 3
      SPACE = 3
      CAPTION "IDL"
      BEGIN
        WID_LABEL_58 LABEL -1 -1 0 0
        VALUE "Min:"
        ALIGNLEFT
        END
        POS_MIN_FLD TEXT -1 -1 0 0
        NUMITEMS = 1
        ITEM "0"
        NONEWLINE
        WIDTH = 20
        HEIGHT = 1
        END
      END
      WID_BASE_30 BASE -1 -1 0 0
      ALIGNCENTER
      COLUMNS = 1
      XPAD = 3
      YPAD = 3
      SPACE = 3
      CAPTION "IDL"
      BEGIN
        WID_BASE_5 BASE -1 -1 0 0
        ALIGNRIGHT
        ROWS = 1
        XPAD = 3
        YPAD = 3
        SPACE = 3
        CAPTION "IDL"
        BEGIN
          WID_LABEL_59 LABEL -1 -1 0 0
          VALUE "UB:"
          ALIGNCENTER
          END
          POS_MAX_CBAR_FLD TEXT -1 -1 0 0
          NUMITEMS = 1
          ITEM "32767"
          WIDTH = 20
          HEIGHT = 1
          END
        END
        WID_BASE_6 BASE -1 -1 0 0
        ALIGNRIGHT
        ROWS = 1
        XPAD = 3
        YPAD = 3
        SPACE = 3
        CAPTION "IDL"
        BEGIN
          WID_LABEL_60 LABEL -1 -1 0 0
          VALUE "LB:"
          ALIGNCENTER
          END
          POS_MIN_CBAR_FLD TEXT -1 -1 0 0
          NUMITEMS = 1
          ITEM "-32768"
          ALLEVENTS
          WIDTH = 20
          HEIGHT = 1
          END
        END
        POS_OPT_CUTS_BTN PUSHBUTTON -1 -1 0 0
        VALUE "Opt. cuts"
        ALIGNCENTER
        ONACTIVATE "xdds_gui_pos_opt_cuts"
        END
      END
      WID_BASE_31 BASE -1 -1 0 0
      ALIGNCENTER
      ROWS = 1
      XPAD = 3
      YPAD = 3
      SPACE = 3
      CAPTION "IDL"
      BEGIN
        POS_COLOR_BAR DRAW -1 -1 160 10
        END
      END
      WID_BASE_32 BASE -1 -1 0 0
      ALIGNCENTER
      ROWS = 1
      XPAD = 3
      YPAD = 3
      SPACE = 3
      CAPTION "IDL"
      BEGIN
        WID_BASE_46 BASE -1 -1 0 0
        COLUMNS = 1
        NONEXCLUSIVE
        BEGIN
          POS_SET_OFFSET_BTN PUSHBUTTON -1 -1 0 0
          VALUE "Offset"
          ALIGNLEFT
          ONACTIVATE "xdds_gui_pos_set_offset"
          END
        END
        POS_NEW_OFFSET_BTN PUSHBUTTON -1 -1 0 0
        VALUE "New offset"
        ALIGNCENTER
        ONACTIVATE "pos_new_offset"
        END
      END
      WID_BASE_34 BASE -1 -1 0 0
      ALIGNCENTER
      COLUMNS = 1
      NONEXCLUSIVE
      BEGIN
        POS_SHOW_AVE_BTN PUSHBUTTON -1 -1 0 0
        VALUE "Show ave. command"
        ALIGNLEFT
        ONACTIVATE "xdds_gui_pos_show_ave"
        END
      END
      WID_BASE_35 BASE -1 -1 0 0
      COLUMNS = 1
      XPAD = 3
      YPAD = 3
      SPACE = 3
      CAPTION "IDL"
      BEGIN
        WID_BASE_47 BASE -1 -1 0 0
        ROWS = 1
        XPAD = 3
        YPAD = 3
        SPACE = 3
        CAPTION "IDL"
        BEGIN
          WID_LABEL_49 LABEL -1 -1 0 0
          VALUE "Sets to ave.:"
          ALIGNLEFT
          END
          POS_NUM_SET_TO_AVE SLIDER -1 -1 0 0
          MINIMUM = 1
          MAXIMUM = 1000
          END
        END
      END
    END
    CURR_BASE BASE -1 -1 0 0
    ONPOSTCREATE "xdds_gui_curr_base_post_creation"
    COLUMNS = 1
    XPAD = 3
    YPAD = 3
    SPACE = 3
    BEGIN
      WID_LABEL_45 LABEL -1 -1 0 0
      VALUE "Current"
      ALIGNCENTER
      END
      CURR_DRAW DRAW -1 -1 180 180
      END
      WID_BASE_37 BASE -1 -1 0 0
      ALIGNCENTER
      ROWS = 1
      XPAD = 3
      YPAD = 3
      SPACE = 3
      CAPTION "IDL"
      BEGIN
        WID_LABEL_50 LABEL -1 -1 0 0
        VALUE "Max:"
        ALIGNLEFT
        END
        CURR_MAX_FLD TEXT -1 -1 0 0
        NUMITEMS = 1
        ITEM "0"
        NONEWLINE
        WIDTH = 20
        HEIGHT = 1
        END
      END
      WID_BASE_38 BASE -1 -1 0 0
      ALIGNCENTER
      ROWS = 1
      XPAD = 3
      YPAD = 3
      SPACE = 3
      CAPTION "IDL"
      BEGIN
        WID_LABEL_51 LABEL -1 -1 0 0
        VALUE "Min:"
        ALIGNLEFT
        END
        CURR_MIN_FLD TEXT -1 -1 0 0
        NUMITEMS = 1
        ITEM "0"
        NONEWLINE
        WIDTH = 20
        HEIGHT = 1
        END
      END
      WID_BASE_39 BASE -1 -1 0 0
      ALIGNCENTER
      COLUMNS = 1
      XPAD = 3
      YPAD = 3
      SPACE = 3
      CAPTION "IDL"
      BEGIN
        WID_BASE_8 BASE -1 -1 0 0
        ALIGNRIGHT
        ROWS = 1
        XPAD = 3
        YPAD = 3
        SPACE = 3
        CAPTION "IDL"
        BEGIN
          WID_LABEL_65 LABEL -1 -1 0 0
          VALUE "UB:"
          ALIGNCENTER
          END
          CURR_MAX_CBAR_FLD TEXT -1 -1 0 0
          NUMITEMS = 1
          ITEM "32767"
          WIDTH = 20
          HEIGHT = 1
          END
        END
        WID_BASE_9 BASE -1 -1 0 0
        ALIGNRIGHT
        ROWS = 1
        XPAD = 3
        YPAD = 3
        SPACE = 3
        CAPTION "IDL"
        BEGIN
          WID_LABEL_66 LABEL -1 -1 0 0
          VALUE "LB:"
          ALIGNCENTER
          END
          CURR_MIN_CBAR_FLD TEXT -1 -1 0 0
          NUMITEMS = 1
          ITEM "-32768"
          ALLEVENTS
          WIDTH = 20
          HEIGHT = 1
          END
        END
        CURR_OPT_CUTS_BTN PUSHBUTTON -1 -1 0 0
        VALUE "Opt. cuts"
        ALIGNCENTER
        ONACTIVATE "xdds_gui_curr_opt_cuts"
        END
      END
      WID_BASE_40 BASE -1 -1 0 0
      ALIGNCENTER
      ROWS = 1
      XPAD = 3
      YPAD = 3
      SPACE = 3
      CAPTION "IDL"
      BEGIN
        CURR_COLOR_BAR DRAW -1 -1 160 10
        END
      END
      WID_BASE_41 BASE -1 -1 0 0
      ALIGNCENTER
      ROWS = 1
      XPAD = 3
      YPAD = 3
      SPACE = 3
      CAPTION "IDL"
      BEGIN
        WID_BASE_48 BASE -1 -1 0 0
        COLUMNS = 1
        NONEXCLUSIVE
        BEGIN
          CURR_SET_OFFSET_BTN PUSHBUTTON -1 -1 0 0
          VALUE "Offset"
          ALIGNLEFT
          ONACTIVATE "xdds_gui_curr_set_offset"
          END
        END
        CURR_NEW_OFFSET_BTN PUSHBUTTON -1 -1 0 0
        VALUE "New offset"
        ALIGNCENTER
        ONACTIVATE "curr_new_offset"
        END
      END
      WID_BASE_43 BASE -1 -1 0 0
      ALIGNCENTER
      COLUMNS = 1
      NONEXCLUSIVE
      BEGIN
        CURR_SHOW_AVE_BTN PUSHBUTTON -1 -1 0 0
        VALUE "Show ave. command"
        ALIGNLEFT
        ONACTIVATE "xdds_gui_curr_show_ave"
        END
      END
      WID_BASE_44 BASE -1 -1 0 0
      COLUMNS = 1
      XPAD = 3
      YPAD = 3
      SPACE = 3
      CAPTION "IDL"
      BEGIN
        WID_BASE_49 BASE -1 -1 0 0
        ROWS = 1
        XPAD = 3
        YPAD = 3
        SPACE = 3
        CAPTION "IDL"
        BEGIN
          WID_LABEL_53 LABEL -1 -1 0 0
          VALUE "Sets to ave.:"
          ALIGNLEFT
          END
          CURR_NUM_SET_TO_AVE SLIDER -1 -1 0 0
          MINIMUM = 1
          MAXIMUM = 1000
          END
        END
      END
    END
    COMM_BASE BASE -1 -1 0 0
    ONPOSTCREATE "xdds_gui_comm_base_post_creation"
    COLUMNS = 1
    XPAD = 3
    YPAD = 3
    SPACE = 3
    BEGIN
      COMM_LABEL LABEL -1 -1 0 0
      VALUE "Command"
      ALIGNCENTER
      END
      COMM_DRAW DRAW -1 -1 180 180
      END
      WID_BASE_55 BASE -1 -1 0 0
      ALIGNCENTER
      ROWS = 1
      XPAD = 3
      YPAD = 3
      SPACE = 3
      CAPTION "IDL"
      BEGIN
        WID_LABEL_54 LABEL -1 -1 0 0
        VALUE "Max:"
        ALIGNLEFT
        END
        COMM_MAX_FLD TEXT -1 -1 0 0
        NUMITEMS = 1
        ITEM "0"
        NONEWLINE
        WIDTH = 20
        HEIGHT = 1
        END
      END
      WID_BASE_56 BASE -1 -1 0 0
      ALIGNCENTER
      ROWS = 1
      XPAD = 3
      YPAD = 3
      SPACE = 3
      CAPTION "IDL"
      BEGIN
        WID_LABEL_55 LABEL -1 -1 0 0
        VALUE "Min:"
        ALIGNLEFT
        END
        COMM_MIN_FLD TEXT -1 -1 0 0
        NUMITEMS = 1
        ITEM "0"
        NONEWLINE
        WIDTH = 20
        HEIGHT = 1
        END
      END
      WID_BASE_57 BASE -1 -1 0 0
      ALIGNCENTER
      COLUMNS = 1
      XPAD = 3
      YPAD = 3
      SPACE = 3
      CAPTION "IDL"
      BEGIN
        WID_BASE_2 BASE -1 -1 0 0
        ALIGNRIGHT
        ROWS = 1
        XPAD = 3
        YPAD = 3
        SPACE = 3
        CAPTION "IDL"
        BEGIN
          WID_LABEL_69 LABEL -1 -1 0 0
          VALUE "UB:"
          ALIGNCENTER
          END
          COMM_MAX_CBAR_FLD TEXT -1 -1 0 0
          NUMITEMS = 1
          ITEM "32767"
          WIDTH = 20
          HEIGHT = 1
          END
        END
        WID_BASE_3 BASE -1 -1 0 0
        ALIGNRIGHT
        ROWS = 1
        XPAD = 3
        YPAD = 3
        SPACE = 3
        CAPTION "IDL"
        BEGIN
          WID_LABEL_70 LABEL -1 -1 0 0
          VALUE "LB:"
          ALIGNCENTER
          END
          COMM_MIN_CBAR_FLD TEXT -1 -1 0 0
          NUMITEMS = 1
          ITEM "-32768"
          ALLEVENTS
          WIDTH = 20
          HEIGHT = 1
          END
        END
        COMM_OPT_CUTS_BTN PUSHBUTTON -1 -1 0 0
        VALUE "Opt. cuts"
        ALIGNCENTER
        ONACTIVATE "xdds_gui_comm_opt_cuts"
        END
      END
      WID_BASE_58 BASE -1 -1 0 0
      ALIGNCENTER
      ROWS = 1
      XPAD = 3
      YPAD = 3
      SPACE = 3
      CAPTION "IDL"
      BEGIN
        COMM_COLOR_BAR DRAW -1 -1 160 10
        END
      END
      WID_BASE_59 BASE -1 -1 0 0
      ALIGNCENTER
      ROWS = 1
      XPAD = 3
      YPAD = 3
      SPACE = 3
      CAPTION "IDL"
      BEGIN
        WID_BASE_60 BASE -1 -1 0 0
        COLUMNS = 1
        NONEXCLUSIVE
        BEGIN
          COMM_SET_OFFSET_BTN PUSHBUTTON -1 -1 0 0
          VALUE "Offset"
          ALIGNLEFT
          ONACTIVATE "xdds_gui_comm_set_offset"
          END
        END
        COMM_NEW_OFFSET_BTN PUSHBUTTON -1 -1 0 0
        VALUE "New offset"
        ALIGNCENTER
        ONACTIVATE "comm_new_offset"
        END
      END
      WID_BASE_61 BASE -1 -1 0 0
      ALIGNCENTER
      COLUMNS = 1
      NONEXCLUSIVE
      BEGIN
        COMM_SHOW_AVE_BTN PUSHBUTTON -1 -1 0 0
        VALUE "Show ave. command"
        ALIGNLEFT
        ONACTIVATE "xdds_gui_comm_show_ave"
        END
      END
      WID_BASE_62 BASE -1 -1 0 0
      COLUMNS = 1
      XPAD = 3
      YPAD = 3
      SPACE = 3
      CAPTION "IDL"
      BEGIN
        WID_BASE_63 BASE -1 -1 0 0
        ROWS = 1
        XPAD = 3
        YPAD = 3
        SPACE = 3
        CAPTION "IDL"
        BEGIN
          WID_LABEL_56 LABEL -1 -1 0 0
          VALUE "Sets to ave.:"
          ALIGNLEFT
          END
          COMM_NUM_SET_TO_AVE SLIDER -1 -1 0 0
          MINIMUM = 1
          MAXIMUM = 1000
          END
        END
      END
    END
    COMM_BASE_0 BASE -1 -1 0 0
    ONPOSTCREATE "xdds_gui_comm_base_post_creation"
    COLUMNS = 1
    XPAD = 3
    YPAD = 3
    SPACE = 3
    BEGIN
      MINMAX_LABEL LABEL -1 -1 0 0
      VALUE "Min-Max"
      ALIGNCENTER
      END
      MINMAX_DRAW DRAW -1 -1 180 180
      END
      WID_BASE_64 BASE -1 -1 0 0
      ALIGNCENTER
      ROWS = 1
      XPAD = 3
      YPAD = 3
      SPACE = 3
      CAPTION "IDL"
      BEGIN
        WID_LABEL_57 LABEL -1 -1 0 0
        VALUE "Max:"
        ALIGNLEFT
        END
        MINMAX_MAX_FLD TEXT -1 -1 0 0
        NUMITEMS = 1
        ITEM "0"
        NONEWLINE
        WIDTH = 20
        HEIGHT = 1
        END
      END
      WID_BASE_65 BASE -1 -1 0 0
      ALIGNCENTER
      ROWS = 1
      XPAD = 3
      YPAD = 3
      SPACE = 3
      CAPTION "IDL"
      BEGIN
        WID_LABEL_61 LABEL -1 -1 0 0
        VALUE "Min:"
        ALIGNLEFT
        END
        MINMAX_MIN_FLD TEXT -1 -1 0 0
        NUMITEMS = 1
        ITEM "0"
        NONEWLINE
        WIDTH = 20
        HEIGHT = 1
        END
      END
      WID_BASE_66 BASE -1 -1 0 0
      ALIGNCENTER
      COLUMNS = 1
      XPAD = 3
      YPAD = 3
      SPACE = 3
      CAPTION "IDL"
      BEGIN
        WID_BASE_10 BASE -1 -1 0 0
        ALIGNRIGHT
        ROWS = 1
        XPAD = 3
        YPAD = 3
        SPACE = 3
        CAPTION "IDL"
        BEGIN
          WID_LABEL_71 LABEL -1 -1 0 0
          VALUE "UB:"
          ALIGNCENTER
          END
          MINMAX_MAX_CBAR_FLD TEXT -1 -1 0 0
          NUMITEMS = 1
          ITEM "32767"
          WIDTH = 20
          HEIGHT = 1
          END
        END
        WID_BASE_11 BASE -1 -1 0 0
        ALIGNRIGHT
        ROWS = 1
        XPAD = 3
        YPAD = 3
        SPACE = 3
        CAPTION "IDL"
        BEGIN
          WID_LABEL_72 LABEL -1 -1 0 0
          VALUE "LB:"
          ALIGNCENTER
          END
          MINMAX_MIN_CBAR_FLD TEXT -1 -1 0 0
          NUMITEMS = 1
          ITEM "-32768"
          ALLEVENTS
          WIDTH = 20
          HEIGHT = 1
          END
        END
        MINMAX_OPT_CUTS_BTN PUSHBUTTON -1 -1 0 0
        VALUE "Opt. cuts"
        ALIGNCENTER
        ONACTIVATE "xdds_gui_minmax_opt_cuts"
        END
      END
      WID_BASE_67 BASE -1 -1 0 0
      ALIGNCENTER
      ROWS = 1
      XPAD = 3
      YPAD = 3
      SPACE = 3
      CAPTION "IDL"
      BEGIN
        MINMAX_COLOR_BAR DRAW -1 -1 160 10
        END
      END
      WID_BASE_68 BASE -1 -1 0 0
      ALIGNCENTER
      ROWS = 1
      XPAD = 3
      YPAD = 3
      SPACE = 3
      CAPTION "IDL"
      BEGIN
        WID_BASE_69 BASE -1 -1 0 0
        COLUMNS = 1
        NONEXCLUSIVE
        BEGIN
          MINMAX_SET_OFFSET_BTN PUSHBUTTON -1 -1 0 0
          VALUE "Offset"
          ALIGNLEFT
          ONACTIVATE "xdds_gui_minmax_set_offset"
          END
        END
        MINMAX_NEW_OFFSET_BTN PUSHBUTTON -1 -1 0 0
        VALUE "New offset"
        ALIGNCENTER
        ONACTIVATE "minmax_new_offset"
        END
      END
      WID_BASE_70 BASE -1 -1 0 0
      ALIGNCENTER
      COLUMNS = 1
      NONEXCLUSIVE
      BEGIN
        MINMAX_SHOW_AVE_BTN PUSHBUTTON -1 -1 0 0
        VALUE "Show ave. command"
        ALIGNLEFT
        ONACTIVATE "xdds_gui_minmax_show_ave"
        END
      END
      WID_BASE_71 BASE -1 -1 0 0
      COLUMNS = 1
      XPAD = 3
      YPAD = 3
      SPACE = 3
      CAPTION "IDL"
      BEGIN
        WID_BASE_72 BASE -1 -1 0 0
        ROWS = 1
        XPAD = 3
        YPAD = 3
        SPACE = 3
        CAPTION "IDL"
        BEGIN
          WID_LABEL_62 LABEL -1 -1 0 0
          VALUE "Sets to ave.:"
          ALIGNLEFT
          END
          MINMAX_NUM_SET_TO_AVE SLIDER -1 -1 0 0
          MINIMUM = 1
          MAXIMUM = 1000
          END
        END
      END
    END
  END
  WID_BASE_12 BASE -1 -1 0 0
  ROWS = 1
  XPAD = 3
  YPAD = 3
  SPACE = 3
  CAPTION "IDL"
  BEGIN
    MODAL_DECOMP_DRAW DRAW -1 -1 754 203
    END
    WID_BASE_13 BASE -1 -1 0 0
    COLUMNS = 1
    XPAD = 3
    YPAD = 3
    SPACE = 3
    CAPTION "IDL"
    BEGIN
      WID_LABEL_12 LABEL -1 -1 0 0
      VALUE "Modal decomposition"
      ALIGNLEFT
      END
      MODAL_DECOMP_DLIST_1 DROPLIST -1 -1 0 0
      NUMITEMS = 5
      ITEM "No plot"
      ITEM "Position"
      ITEM "Current"
      ITEM "Command"
      ITEM "Rec. delta-command"
      ONSELECT "xdds_gui_modal_decomp_select"
      END
      WID_LABEL_27 LABEL -1 -1 0 0
      VALUE "Range [nm]:"
      ALIGNCENTER
      END
      MODAL_DECOMP_YRANGE_SLIDE_1 SLIDER -1 -1 0 0
      VALUE = 500
      MINIMUM = 100
      MAXIMUM = 5000
      END
    END
  END
END
