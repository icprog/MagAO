HEADER
; IDL Visual Widget Builder Resource file. Version 1
; Generated on:	01/17/2003 00:16.49
VERSION 1
END

WID_BASE_0 BASE 5 5 988 700
ONPOSTCREATE "xdds_gui_post_creation"
TLB
CAPTION "XDDS GUI"
XPAD = 3
YPAD = 3
SPACE = 3
BEGIN
  WID_BASE_1 BASE 5 4 107 50
  XPAD = 3
  YPAD = 3
  SPACE = 3
  BEGIN
    REFRESH_TIME_FLD TEXT 3 7 0 0
    NUMITEMS = 1
    ITEM "1.0"
    EDITABLE
    ALLEVENTS
    ONINSERTSTRING "xdds_gui_update_refresh_time"
    WIDTH = 20
    HEIGHT = 1
    END
    WID_LABEL_0 LABEL 12 27 0 0
    VALUE "Refresh time [s]"
    ALIGNLEFT
    END
  END
  POS_BASE BASE 7 60 187 399
  ONPOSTCREATE "xdds_gui_pos_base_post_creation"
  XPAD = 3
  YPAD = 3
  SPACE = 3
  BEGIN
    POS_DRAW DRAW 2 21 180 180
    END
    WID_LABEL_1 LABEL 1 3 0 0
    VALUE "Position"
    ALIGNLEFT
    END
    POS_MAX_FLD TEXT 41 208 0 0
    NUMITEMS = 1
    ITEM "0"
    NONEWLINE
    WIDTH = 20
    HEIGHT = 1
    END
    POS_MIN_FLD TEXT 41 227 0 0
    NUMITEMS = 1
    ITEM "0"
    NONEWLINE
    WIDTH = 20
    HEIGHT = 1
    END
    WID_LABEL_2 LABEL 14 206 0 0
    VALUE "Max:"
    ALIGNLEFT
    END
    WID_LABEL_3 LABEL 14 227 0 0
    VALUE "Min:"
    ALIGNLEFT
    END
    POS_OPT_CUTS_BTN PUSHBUTTON 67 266 0 0
    VALUE "Opt. cuts"
    ALIGNCENTER
    ONACTIVATE "xdds_gui_pos_opt_cuts"
    END
    POS_COLOR_BAR DRAW 21 253 160 10
    END
    POS_MIN_CBAR_FLD TEXT 22 265 44 15
    NUMITEMS = 1
    ITEM "-32768"
    ALLEVENTS
    WIDTH = 20
    HEIGHT = 1
    END
    POS_MAX_CBAR_FLD TEXT 133 264 48 18
    NUMITEMS = 1
    ITEM "32767"
    WIDTH = 20
    HEIGHT = 1
    END
    POS_SET_OFFSET_BASE BASE 21 292 0 0
    COLUMNS = 1
    NONEXCLUSIVE
    BEGIN
      POS_SET_OFFSET_BTN PUSHBUTTON -1 -1 0 0
      VALUE "Offset"
      ALIGNLEFT
      ONACTIVATE "xdds_gui_pos_set_offset"
      END
    END
    POS_NEW_OFFSET_BTN PUSHBUTTON 115 295 0 0
    VALUE "New offset"
    ALIGNCENTER
    ONACTIVATE "pos_new_offset"
    END
    POS_NUM_SET_TO_AVE SLIDER 82 341 0 0
    MINIMUM = 1
    MAXIMUM = 1000
    END
    WID_LABEL_32 LABEL 17 347 0 0
    VALUE "Sets to ave.:"
    ALIGNLEFT
    END
    WID_BASE_8 BASE 18 318 0 0
    COLUMNS = 1
    NONEXCLUSIVE
    BEGIN
      POS_SHOW_AVE_BTN PUSHBUTTON -1 -1 0 0
      VALUE "Show ave. command"
      ALIGNLEFT
      ONACTIVATE "xdds_gui_pos_show_ave"
      END
    END
  END
  CURR_BASE BASE 200 61 186 398
  ONPOSTCREATE "xdds_gui_curr_base_post_creation"
  XPAD = 243
  YPAD = 3
  SPACE = 3
  BEGIN
    CURR_DRAW DRAW 4 20 180 180
    END
    WID_LABEL_7 LABEL 1 3 0 0
    VALUE "Current"
    ALIGNLEFT
    END
    CURR_MAX_FLD TEXT 43 208 0 0
    NUMITEMS = 1
    ITEM "0"
    NONEWLINE
    WIDTH = 20
    HEIGHT = 1
    END
    CURR_MIN_FLD TEXT 43 228 0 0
    NUMITEMS = 1
    ITEM "0"
    NONEWLINE
    WIDTH = 20
    HEIGHT = 1
    END
    WID_LABEL_8 LABEL 16 206 0 0
    VALUE "Max:"
    ALIGNLEFT
    END
    WID_LABEL_9 LABEL 16 228 0 0
    VALUE "Min:"
    ALIGNLEFT
    END
    CURR_OPT_CUTS_BTN PUSHBUTTON 67 265 0 0
    VALUE "Opt. cuts"
    ALIGNCENTER
    ONACTIVATE "xdds_gui_curr_opt_cuts"
    END
    CURR_MAX_CBAR_FLD TEXT 130 264 48 15
    NUMITEMS = 1
    ITEM "32767"
    WIDTH = 20
    HEIGHT = 1
    END
    CURR_COLOR_BAR DRAW 20 252 160 10
    END
    CURR_MIN_CBAR_FLD TEXT 16 261 48 18
    NUMITEMS = 1
    ITEM "-32768"
    WIDTH = 20
    HEIGHT = 1
    END
    CURR_NEW_OFFSET_BTN PUSHBUTTON 112 291 0 0
    VALUE "New offset"
    ALIGNCENTER
    ONACTIVATE "curr_new_offset"
    END
    WID_BASE_9 BASE 18 288 0 0
    COLUMNS = 1
    NONEXCLUSIVE
    BEGIN
      CURR_SET_OFFSET_BTN PUSHBUTTON -1 -1 0 0
      VALUE "Offset"
      ALIGNLEFT
      ONACTIVATE "xdds_gui_curr_set_offset"
      END
    END
    CURR_NUM_SET_TO_AVE SLIDER 81 338 0 0
    MINIMUM = 1
    MAXIMUM = 1000
    END
    WID_LABEL_31 LABEL 18 342 0 0
    VALUE "Sets to ave.:"
    ALIGNLEFT
    END
    WID_BASE_5 BASE 17 312 0 0
    COLUMNS = 1
    NONEXCLUSIVE
    BEGIN
      CURR_SHOW_AVE_BTN PUSHBUTTON -1 -1 0 0
      VALUE "Show ave. command"
      ALIGNLEFT
      ONACTIVATE "xdds_gui_curr_show_ave"
      END
    END
  END
  MINMAX_BASE BASE 575 62 190 398
  ONPOSTCREATE "xdds_gui_minmax_base_post_creation"
  XPAD = 3
  YPAD = 3
  SPACE = 3
  BEGIN
    MINMAX_DRAW DRAW 4 24 180 180
    END
    MINMAX_LABEL LABEL 1 3 0 0
    VALUE "Max-Min"
    ALIGNLEFT
    END
    MINMAX_MAX_FLD TEXT 35 210 0 0
    NUMITEMS = 1
    ITEM "0"
    NONEWLINE
    WIDTH = 20
    HEIGHT = 1
    END
    MINMAX_MIN_FLD TEXT 36 230 0 0
    NUMITEMS = 1
    ITEM "0"
    NONEWLINE
    WIDTH = 20
    HEIGHT = 1
    END
    WID_LABEL_11 LABEL 8 208 0 0
    VALUE "Max:"
    ALIGNLEFT
    END
    WID_LABEL_12 LABEL 9 230 0 0
    VALUE "Min:"
    ALIGNLEFT
    END
    MINMAX_OPT_CUTS_BTN PUSHBUTTON 62 266 0 0
    VALUE "Opt. cuts"
    ALIGNCENTER
    ONACTIVATE "xdds_gui_minmax_opt_cuts"
    END
    MINMAX_MAX_CBAR_FLD TEXT 129 267 46 15
    NUMITEMS = 1
    ITEM "65535"
    WIDTH = 20
    HEIGHT = 1
    END
    MINMAX_MIN_CBAR_FLD TEXT 16 265 43 15
    NUMITEMS = 1
    ITEM "0"
    WIDTH = 20
    HEIGHT = 1
    END
    MINMAX_COLOR_BAR DRAW 16 252 152 10
    END
    MINMAX_NUM_SET_TO_AVE SLIDER 76 336 0 0
    MINIMUM = 1
    MAXIMUM = 1000
    END
    WID_LABEL_33 LABEL 11 342 0 0
    VALUE "Sets to ave.:"
    ALIGNLEFT
    END
    WID_BASE_11 BASE 12 313 0 0
    COLUMNS = 1
    NONEXCLUSIVE
    BEGIN
      MINMAX_SHOW_AVE_BTN PUSHBUTTON -1 -1 0 0
      VALUE "Show ave. command"
      ALIGNLEFT
      ONACTIVATE "xdds_gui_minmax_show_ave"
      END
    END
  END
  SPOT_SLOPE_BASE BASE 768 64 206 354
  XPAD = 3
  YPAD = 3
  SPACE = 3
  BEGIN
    SPOT_SLOPE_DRAW DRAW 3 24 192 192
    END
    WID_LABEL_13 LABEL 1 3 0 0
    VALUE "Spots"
    ALIGNLEFT
    END
    FRAME_MAX_FLD TEXT 69 235 0 0
    NUMITEMS = 1
    ITEM "0"
    NONEWLINE
    WIDTH = 20
    HEIGHT = 1
    END
    FRAME_MIN_FLD TEXT 68 257 0 0
    NUMITEMS = 1
    ITEM "0"
    NONEWLINE
    WIDTH = 20
    HEIGHT = 1
    END
    WID_LABEL_14 LABEL 11 233 0 0
    VALUE "Max:"
    ALIGNLEFT
    END
    WID_LABEL_15 LABEL 11 254 0 0
    VALUE "Min:"
    ALIGNLEFT
    END
    SPOT_OPT_CUTS_BTN PUSHBUTTON 65 295 0 0
    VALUE "Opt. cuts"
    ALIGNCENTER
    ONACTIVATE "xdds_gui_spot_opt_cuts"
    END
    SPOT_COLOR_BAR DRAW 18 280 152 10
    END
    SPOT_MAX_CBAR_FLD TEXT 128 295 49 15
    NUMITEMS = 1
    ITEM "32767"
    WIDTH = 20
    HEIGHT = 1
    END
    SPOT_MIN_CBAR_FLD TEXT 19 293 41 15
    NUMITEMS = 1
    ITEM "0"
    ALLEVENTS
    WIDTH = 20
    HEIGHT = 1
    END
    NUM_FRAME_TO_AVE SLIDER 96 321 0 0
    MINIMUM = 1
    MAXIMUM = 30
    END
    WID_LABEL_25 LABEL 0 332 0 0
    VALUE "Frames to average"
    ALIGNLEFT
    END
    TAKE_BACKGROUND_BTN PUSHBUTTON 104 0 0 0
    VALUE "Take background"
    ALIGNCENTER
    ONACTIVATE "xdds_take_background"
    END
  END
  START_STOP_READ_BTN PUSHBUTTON 129 3 37 21
  VALUE "Start"
  ALIGNCENTER
  TIMEREVENT "xdds_gui_on_timer"
  ONACTIVATE "xdds_gui_start_stop"
  END
  WID_BASE_6 BASE 236 4 349 57
  UNMAP
  XPAD = 3
  YPAD = 3
  SPACE = 3
  BEGIN
    WID_LABEL_16 LABEL 3 8 0 0
    VALUE "Act.mir #"
    ALIGNLEFT
    END
    WID_LABEL_17 LABEL 0 32 0 0
    VALUE "Act.dsp #"
    ALIGNLEFT
    END
    WID_TEXT_0 TEXT 48 7 0 0
    WIDTH = 20
    HEIGHT = 1
    END
    ACT_DSP_NUM_FLD TEXT 49 31 0 0
    WIDTH = 20
    HEIGHT = 1
    END
    WID_LABEL_18 LABEL 116 7 0 0
    VALUE "Position:"
    ALIGNLEFT
    END
    WID_LABEL_19 LABEL 116 29 0 0
    VALUE "Current:"
    ALIGNLEFT
    END
    WID_LABEL_20 LABEL 244 7 0 0
    VALUE "Command:"
    ALIGNLEFT
    END
    WID_LABEL_21 LABEL 228 29 0 0
    VALUE "Max-Min Pos.:"
    ALIGNLEFT
    END
    ACT_POS_FLD TEXT 158 6 0 0
    WIDTH = 20
    HEIGHT = 1
    END
    ACT_CURR_FLD TEXT 159 29 0 0
    WIDTH = 20
    HEIGHT = 1
    END
    ACT_COMM_FLD TEXT 296 8 0 0
    WIDTH = 20
    HEIGHT = 1
    END
    ACT_MINMAX_FLD TEXT 298 30 0 0
    WIDTH = 20
    HEIGHT = 1
    END
  END
  COMM_BASE BASE 389 62 184 400
  ONPOSTCREATE "xdds_gui_comm_base_post_creation"
  XPAD = 3
  YPAD = 3
  SPACE = 3
  BEGIN
    COMM_DRAW DRAW 2 24 180 180
    END
    WID_LABEL_22 LABEL 1 3 0 0
    VALUE "Command"
    ALIGNLEFT
    END
    COMM_MAX_FLD TEXT 37 209 0 0
    NUMITEMS = 1
    ITEM "0"
    NONEWLINE
    WIDTH = 20
    HEIGHT = 1
    END
    COMM_MIN_FLD TEXT 38 230 0 0
    NUMITEMS = 1
    ITEM "0"
    NONEWLINE
    WIDTH = 20
    HEIGHT = 1
    END
    WID_LABEL_23 LABEL 10 207 0 0
    VALUE "Max:"
    ALIGNLEFT
    END
    WID_LABEL_24 LABEL 11 227 22 18
    VALUE "Min:"
    ALIGNLEFT
    END
    COMM_OPT_CUTS_BTN PUSHBUTTON 62 268 0 0
    VALUE "Opt. cuts"
    ALIGNCENTER
    ONACTIVATE "xdds_gui_comm_opt_cuts"
    END
    COMM_MAX_CBAR_FLD TEXT 124 267 52 15
    NUMITEMS = 1
    ITEM "32767"
    WIDTH = 20
    HEIGHT = 1
    END
    COMM_COLOR_BAR DRAW 14 253 152 10
    END
    COMM_MIN_CBAR_FLD TEXT 13 266 46 15
    NUMITEMS = 1
    ITEM "-32768"
    WIDTH = 20
    HEIGHT = 1
    END
    COMM_NEW_OFFSET_BTN PUSHBUTTON 110 293 0 0
    VALUE "New offset"
    ALIGNCENTER
    ONACTIVATE "comm_new_offset"
    END
    WID_BASE_10 BASE 16 290 0 0
    COLUMNS = 1
    NONEXCLUSIVE
    BEGIN
      COMM_SET_OFFSET_BTN PUSHBUTTON -1 -1 0 0
      VALUE "Offset"
      ALIGNLEFT
      ONACTIVATE "xdds_gui_comm_set_offset"
      END
    END
    COMM_NUM_SET_TO_AVE SLIDER 81 339 0 0
    MINIMUM = 1
    MAXIMUM = 1000
    END
    APPLY_AVE_COMMAND_BTN PUSHBUTTON 14 370 0 0
    VALUE "Apply ave. command"
    ALIGNCENTER
    ONACTIVATE "xdds_apply_ave_command"
    END
    WID_BASE_3 BASE 16 316 0 0
    COLUMNS = 1
    NONEXCLUSIVE
    BEGIN
      COMM_SHOW_AVE_BTN PUSHBUTTON -1 -1 0 0
      VALUE "Show ave. command"
      ALIGNLEFT
      ONACTIVATE "xdds_gui_comm_show_ave"
      END
    END
    WID_LABEL_30 LABEL 15 345 0 0
    VALUE "Sets to ave.:"
    ALIGNLEFT
    END
  END
  EXIT_BTN PUSHBUTTON 129 29 0 0
  VALUE "Exit"
  ALIGNCENTER
  ONACTIVATE "xdds_gui_exit"
  END
  WID_BASE_7 BASE 187 5 0 0
  COLUMNS = 1
  NONEXCLUSIVE
  BEGIN
    READ_FROM_DDS_BTN PUSHBUTTON -1 -1 0 0
    VALUE "Read from DDS"
    ALIGNLEFT
    ONACTIVATE "xdds_gui_read_from_dds"
    END
  END
  POINTING_ERROR_BASE BASE 451 2 519 56
  ONPOSTCREATE "xdds_gui_pointing_error_creation"
  XPAD = 3
  YPAD = 3
  SPACE = 3
  BEGIN
    WID_LABEL_4 LABEL 7 19 0 0
    VALUE "Azimuth:"
    ALIGNLEFT
    END
    WID_LABEL_5 LABEL 7 36 0 0
    VALUE "Elevation:"
    ALIGNLEFT
    END
    OFFLOAD_TILT_BTN PUSHBUTTON 167 22 0 0
    VALUE "Offload tilt now"
    ALIGNCENTER
    ONACTIVATE "xdds_gui_offload_tilt"
    END
    DELTA_AZIMUTH_FLD TEXT 60 17 0 0
    WIDTH = 20
    HEIGHT = 1
    END
    DELTA_ELEVATION_FLD TEXT 60 35 0 0
    WIDTH = 20
    HEIGHT = 1
    END
    WID_LABEL_6 LABEL 3 1 0 0
    VALUE "Pointing error [arcsec]:"
    ALIGNLEFT
    END
    WID_LABEL_27 LABEL 265 13 0 0
    VALUE "Hexapode z error [um]:"
    ALIGNLEFT
    END
    DELTA_FOCUS_FLD TEXT 265 30 0 0
    WIDTH = 20
    HEIGHT = 1
    END
    OFFLOAD_AVE_SLIDER SLIDER 402 20 0 0
    MINIMUM = 1
    END
    WID_LABEL_28 LABEL 401 0 0 0
    VALUE "Running mean steps:"
    ALIGNLEFT
    END
  END
  WID_BASE_2 BASE 186 32 0 0
  COLUMNS = 1
  NONEXCLUSIVE
  BEGIN
    REC_DCOMM_BTN PUSHBUTTON -1 -1 0 0
    VALUE "Display rec. dcomm"
    ALIGNLEFT
    ONACTIVATE "rec_dcomm_press_btn"
    END
  END
  MODAL_DECOMP_DRAW DRAW 6 466 754 203
  END
  MODAL_DECOMP_DLIST DROPLIST 783 512 0 0
  NUMITEMS = 5
  ITEM "No plot"
  ITEM "Position"
  ITEM "Current"
  ITEM "Command"
  ITEM "Rec. delta-command"
  ONSELECT "xdds_gui_modal_decomp_select"
  END
  WID_LABEL_10 LABEL 776 493 0 0
  VALUE "Modal decomposition"
  ALIGNLEFT
  END
  WID_LABEL_26 LABEL 780 551 0 0
  VALUE "Range [nm]:"
  ALIGNLEFT
  END
  MODAL_DECOMP_YRANGE_SLIDE SLIDER 785 567 0 0
  VALUE = 500
  MINIMUM = 100
  MAXIMUM = 5000
  END
END