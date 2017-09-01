HEADER
; IDL Visual Widget Builder Resource file. Version 1
; Generated on:	02/12/2008 02:22.41
VERSION 1
END

HOUSEKEEPING_BASE BASE 5 5 909 704
REALIZE "housekeeping_on_realize"
TIMEREVENT "housekeeping_on_timer"
ONPOSTCREATE "housekeeping_post_creation"
TLB
CAPTION "LBT672 Housekeeping"
COLUMNS = 1
XPAD = 3
YPAD = 3
SPACE = 3
BEGIN
  WID_TAB_0 TAB -1 -1 895 667
  BEGIN
    GENERAL_SUMMARY_BASE BASE 0 0 887 641
    ROWS = 1
    XPAD = 3
    YPAD = 3
    SPACE = 3
    CAPTION "System Summary"
    BEGIN
      WID_BASE_0 BASE -1 -1 0 0
      ROWS = 1
      XPAD = 3
      YPAD = 3
      SPACE = 3
      CAPTION "IDL"
      BEGIN
        start PUSHBUTTON -1 -1 37 18
        VALUE "Start"
        ALIGNCENTER
        ONACTIVATE "start_btn_on_button_press"
        END
        exit PUSHBUTTON -1 -1 37 18
        VALUE "Exit"
        ALIGNCENTER
        ONACTIVATE "exit_btn_on_button_press"
        END
      END
      WID_BASE_69 BASE -1 -1 0 0
      FRAME = 1
      COLUMNS = 1
      XPAD = 3
      YPAD = 3
      SPACE = 3
      CHILDALIGNRIGHT
      CAPTION "IDL"
      BEGIN
        WID_LABEL_151 LABEL -1 -1 0 0
        VALUE "DiagnApp data"
        ALIGNRIGHT
        END
        WID_BASE_77 BASE -1 -1 0 0
        ROWS = 1
        XPAD = 3
        YPAD = 3
        SPACE = 3
        CAPTION "IDL"
        BEGIN
          WID_LABEL_152 LABEL -1 -1 0 0
          VALUE "DiagnAppFrameTimestamp"
          ALIGNLEFT
          END
          diagnappframetimestamp TEXT -1 -1 0 0
          WIDTH = 20
          HEIGHT = 1
          END
          diagnappframetimestamp_ PUSHBUTTON -1 -1 0 0
          VALUE "D:\users\labot\idl\adopt\adsec_lib\gui\housekeeping\ok.bmp"
          ALIGNCENTER
          BITMAP
          END
        END
        WID_BASE_78 BASE -1 -1 0 0
        ROWS = 1
        XPAD = 3
        YPAD = 3
        SPACE = 3
        CAPTION "IDL"
        BEGIN
          WID_LABEL_153 LABEL -1 -1 0 0
          VALUE "DiagnAppFrameCounter"
          ALIGNLEFT
          END
          diagnappframecounter TEXT -1 -1 0 0
          WIDTH = 20
          HEIGHT = 1
          END
          diagnappframecounter_ PUSHBUTTON -1 -1 0 0
          VALUE "D:\users\labot\idl\adopt\adsec_lib\gui\housekeeping\ok.bmp"
          ALIGNCENTER
          BITMAP
          END
        END
        WID_BASE_107 BASE -1 -1 0 0
        ROWS = 1
        XPAD = 3
        YPAD = 3
        SPACE = 3
        CAPTION "IDL"
        BEGIN
          WID_LABEL_154 LABEL -1 -1 0 0
          VALUE "DiagnAppFastVarsCounter"
          ALIGNLEFT
          END
          diagnappfastvarscounter TEXT -1 -1 0 0
          WIDTH = 20
          HEIGHT = 1
          END
          diagnappfastvarscounter_ PUSHBUTTON -1 -1 0 0
          VALUE "D:\users\labot\idl\adopt\adsec_lib\gui\housekeeping\ok.bmp"
          ALIGNCENTER
          BITMAP
          END
        END
        WID_BASE_108 BASE -1 -1 0 0
        ROWS = 1
        XPAD = 3
        YPAD = 3
        SPACE = 3
        CAPTION "IDL"
        BEGIN
          WID_LABEL_155 LABEL -1 -1 0 0
          VALUE "DiagnAppSlowVarsCounter"
          ALIGNLEFT
          END
          diagnappslowvarscounter TEXT -1 -1 0 0
          WIDTH = 20
          HEIGHT = 1
          END
          diagnappslowvarscounter_ PUSHBUTTON -1 -1 0 0
          VALUE "D:\users\labot\idl\adopt\adsec_lib\gui\housekeeping\ok.bmp"
          ALIGNCENTER
          BITMAP
          END
        END
        WID_BASE_109 BASE -1 -1 0 0
        ROWS = 1
        XPAD = 3
        YPAD = 3
        SPACE = 3
        CAPTION "IDL"
        BEGIN
          WID_LABEL_156 LABEL -1 -1 0 0
          VALUE "DiagnAppFrameRate"
          ALIGNLEFT
          END
          diagnappframerate TEXT -1 -1 0 0
          WIDTH = 20
          HEIGHT = 1
          END
          diagnappframerate_ PUSHBUTTON -1 -1 0 0
          VALUE "D:\users\labot\idl\adopt\adsec_lib\gui\housekeeping\ok.bmp"
          ALIGNCENTER
          BITMAP
          END
        END
        WID_BASE_110 BASE -1 -1 0 0
        ROWS = 1
        XPAD = 3
        YPAD = 3
        SPACE = 3
        CAPTION "IDL"
        BEGIN
          WID_LABEL_157 LABEL -1 -1 0 0
          VALUE "DiagnAppFastVarsRate"
          ALIGNLEFT
          END
          diagnappfastvarsrate TEXT -1 -1 0 0
          WIDTH = 20
          HEIGHT = 1
          END
          diagnappfastvarsrate_ PUSHBUTTON -1 -1 0 0
          VALUE "D:\users\labot\idl\adopt\adsec_lib\gui\housekeeping\ok.bmp"
          ALIGNCENTER
          BITMAP
          END
        END
        WID_BASE_111 BASE -1 -1 0 0
        ROWS = 1
        XPAD = 3
        YPAD = 3
        SPACE = 3
        CAPTION "IDL"
        BEGIN
          WID_LABEL_158 LABEL -1 -1 0 0
          VALUE "DiagnAppSlowVarsRate"
          ALIGNLEFT
          END
          diagnappslowvarsrate TEXT -1 -1 0 0
          WIDTH = 20
          HEIGHT = 1
          END
          diagnappslowvarsrate_ PUSHBUTTON -1 -1 0 0
          VALUE "D:\users\labot\idl\adopt\adsec_lib\gui\housekeeping\ok.bmp"
          ALIGNCENTER
          BITMAP
          END
        END
      END
      WID_BASE_85 BASE -1 -1 0 0
      FRAME = 1
      COLUMNS = 1
      XPAD = 3
      YPAD = 3
      SPACE = 3
      CHILDALIGNRIGHT
      CAPTION "IDL"
      BEGIN
        WID_LABEL_171 LABEL -1 -1 0 0
        VALUE "External temperatures and cooling data"
        ALIGNRIGHT
        END
        WID_BASE_87 BASE -1 -1 0 0
        ROWS = 1
        XPAD = 3
        YPAD = 3
        SPACE = 3
        CAPTION "IDL"
        BEGIN
          WID_LABEL_172 LABEL -1 -1 0 0
          VALUE "ExternalTemperature"
          ALIGNLEFT
          END
          externaltemperature TEXT -1 -1 0 0
          WIDTH = 20
          HEIGHT = 1
          END
          externaltemperature_ PUSHBUTTON -1 -1 0 0
          VALUE "D:\users\labot\idl\adopt\adsec_lib\gui\housekeeping\ok.bmp"
          ALIGNCENTER
          BITMAP
          END
        END
        WID_BASE_88 BASE -1 -1 0 0
        ROWS = 1
        XPAD = 3
        YPAD = 3
        SPACE = 3
        CAPTION "IDL"
        BEGIN
          WID_LABEL_173 LABEL -1 -1 0 0
          VALUE "FluxRateIn"
          ALIGNLEFT
          END
          fluxratein TEXT -1 -1 0 0
          WIDTH = 20
          HEIGHT = 1
          END
          fluxratein_ PUSHBUTTON -1 -1 0 0
          VALUE "D:\users\labot\idl\adopt\adsec_lib\gui\housekeeping\ok.bmp"
          ALIGNCENTER
          BITMAP
          END
        END
        WID_BASE_114 BASE -1 -1 0 0
        ROWS = 1
        XPAD = 3
        YPAD = 3
        SPACE = 3
        CAPTION "IDL"
        BEGIN
          WID_LABEL_174 LABEL -1 -1 0 0
          VALUE "WaterMainInlet"
          ALIGNLEFT
          END
          watermaininlet TEXT -1 -1 0 0
          WIDTH = 20
          HEIGHT = 1
          END
          watermaininlet_ PUSHBUTTON -1 -1 0 0
          VALUE "D:\users\labot\idl\adopt\adsec_lib\gui\housekeeping\ok.bmp"
          ALIGNCENTER
          BITMAP
          END
        END
        WID_BASE_115 BASE -1 -1 0 0
        ROWS = 1
        XPAD = 3
        YPAD = 3
        SPACE = 3
        CAPTION "IDL"
        BEGIN
          WID_LABEL_175 LABEL -1 -1 0 0
          VALUE "WaterMainOutlet"
          ALIGNLEFT
          END
          watermainoutlet TEXT -1 -1 0 0
          WIDTH = 20
          HEIGHT = 1
          END
          watermainoutlet_ PUSHBUTTON -1 -1 0 0
          VALUE "D:\users\labot\idl\adopt\adsec_lib\gui\housekeeping\ok.bmp"
          ALIGNCENTER
          BITMAP
          END
        END
        WID_BASE_116 BASE -1 -1 0 0
        ROWS = 1
        XPAD = 3
        YPAD = 3
        SPACE = 3
        CAPTION "IDL"
        BEGIN
          WID_LABEL_176 LABEL -1 -1 0 0
          VALUE "WaterColdPlateInlet"
          ALIGNLEFT
          END
          watercoldplateinlet TEXT -1 -1 0 0
          WIDTH = 20
          HEIGHT = 1
          END
          watercoldplateinlet_ PUSHBUTTON -1 -1 0 0
          VALUE "D:\users\labot\idl\adopt\adsec_lib\gui\housekeeping\ok.bmp"
          ALIGNCENTER
          BITMAP
          END
        END
        WID_BASE_117 BASE -1 -1 0 0
        ROWS = 1
        XPAD = 3
        YPAD = 3
        SPACE = 3
        CAPTION "IDL"
        BEGIN
          WID_LABEL_177 LABEL -1 -1 0 0
          VALUE "WaterColdPlateOutlet"
          ALIGNLEFT
          END
          watercoldplateoutlet TEXT -1 -1 0 0
          WIDTH = 20
          HEIGHT = 1
          END
          watercoldplateoutlet_ PUSHBUTTON -1 -1 0 0
          VALUE "D:\users\labot\idl\adopt\adsec_lib\gui\housekeeping\ok.bmp"
          ALIGNCENTER
          BITMAP
          END
        END
        WID_BASE_118 BASE -1 -1 0 0
        ROWS = 1
        XPAD = 3
        YPAD = 3
        SPACE = 3
        CAPTION "IDL"
        BEGIN
          WID_LABEL_178 LABEL -1 -1 0 0
          VALUE "ExternalHumidity"
          ALIGNLEFT
          END
          externalhumidity TEXT -1 -1 0 0
          WIDTH = 20
          HEIGHT = 1
          END
          externalhumidity_ PUSHBUTTON -1 -1 0 0
          VALUE "D:\users\labot\idl\adopt\adsec_lib\gui\housekeeping\ok.bmp"
          ALIGNCENTER
          BITMAP
          END
        END
        WID_BASE_119 BASE -1 -1 0 0
        ROWS = 1
        XPAD = 3
        YPAD = 3
        SPACE = 3
        CAPTION "IDL"
        BEGIN
          WID_LABEL_179 LABEL -1 -1 0 0
          VALUE "DewPoint"
          ALIGNLEFT
          END
          dewpoint TEXT -1 -1 0 0
          WIDTH = 20
          HEIGHT = 1
          END
          dewpoint_ PUSHBUTTON -1 -1 0 0
          VALUE "D:\users\labot\idl\adopt\adsec_lib\gui\housekeeping\ok.bmp"
          ALIGNCENTER
          BITMAP
          END
        END
        WID_BASE_120 BASE -1 -1 0 0
        ROWS = 1
        XPAD = 3
        YPAD = 3
        SPACE = 3
        CAPTION "IDL"
        BEGIN
          WID_LABEL_180 LABEL -1 -1 0 0
          VALUE "CheckDewPoint"
          ALIGNLEFT
          END
          checkdewpoint TEXT -1 -1 0 0
          WIDTH = 20
          HEIGHT = 1
          END
          checkdewpoint_ PUSHBUTTON -1 -1 0 0
          VALUE "D:\users\labot\idl\adopt\adsec_lib\gui\housekeeping\ok.bmp"
          ALIGNCENTER
          BITMAP
          END
        END
      END
    END
    CRATE_SUMMARY_BASE BASE 0 0 887 641
    XPAD = 3
    YPAD = 3
    SPACE = 3
    CAPTION "Crate Summary"
    BEGIN
    END
    CRATE_BCU1_BASE BASE 0 0 887 641
    FRAME = 1
    COLUMNS = 1
    XPAD = 3
    YPAD = 3
    SPACE = 3
    CAPTION "Crate BCU 1"
    BEGIN
      WID_BASE_4 BASE -1 -1 0 0
      ROWS = 1
      XPAD = 3
      YPAD = 3
      SPACE = 3
      CAPTION "IDL"
      BEGIN
        WID_BASE_38 BASE -1 -1 0 0
        FRAME = 1
        COLUMNS = 1
        XPAD = 3
        YPAD = 3
        SPACE = 3
        CHILDALIGNRIGHT
        CAPTION "IDL"
        BEGIN
          WID_LABEL_68 LABEL -1 -1 0 0
          VALUE "NIOS Fixed Area"
          ALIGNRIGHT
          END
          WID_BASE_42 BASE -1 -1 0 0
          ROWS = 1
          XPAD = 3
          YPAD = 3
          SPACE = 3
          CAPTION "IDL"
          BEGIN
            WID_LABEL_72 LABEL -1 -1 0 0
            VALUE "CrateID"
            ALIGNLEFT
            END
            bcucrateid TEXT -1 -1 0 0
            WIDTH = 20
            HEIGHT = 1
            END
            bcucrateid_ PUSHBUTTON -1 -1 0 0
            VALUE "D:\users\labot\idl\adopt\adsec_lib\gui\housekeeping\ok.bmp"
            ALIGNCENTER
            BITMAP
            END
          END
          WID_BASE_43 BASE -1 -1 0 0
          ROWS = 1
          XPAD = 3
          YPAD = 3
          SPACE = 3
          CAPTION "IDL"
          BEGIN
            WID_LABEL_73 LABEL -1 -1 0 0
            VALUE "WhoAmI"
            ALIGNLEFT
            END
            bcuwhoami TEXT -1 -1 0 0
            WIDTH = 20
            HEIGHT = 1
            END
            bcuwhoami_ PUSHBUTTON -1 -1 0 0
            VALUE "D:\users\labot\idl\adopt\adsec_lib\gui\housekeeping\ok.bmp"
            ALIGNCENTER
            BITMAP
            END
          END
          WID_BASE_45 BASE -1 -1 0 0
          ROWS = 1
          XPAD = 3
          YPAD = 3
          SPACE = 3
          CAPTION "IDL"
          BEGIN
            WID_LABEL_75 LABEL -1 -1 0 0
            VALUE "SoftwareRelease"
            ALIGNLEFT
            END
            bcusoftwarerelease TEXT -1 -1 0 0
            WIDTH = 20
            HEIGHT = 1
            END
            bcusoftwarerelease_ PUSHBUTTON -1 -1 0 0
            VALUE "D:\users\labot\idl\adopt\adsec_lib\gui\housekeeping\ok.bmp"
            ALIGNCENTER
            BITMAP
            END
          END
          WID_BASE_46 BASE -1 -1 0 0
          ROWS = 1
          XPAD = 3
          YPAD = 3
          SPACE = 3
          CAPTION "IDL"
          BEGIN
            WID_LABEL_76 LABEL -1 -1 0 0
            VALUE "LogicRelease"
            ALIGNLEFT
            END
            bculogicrelease TEXT -1 -1 0 0
            WIDTH = 20
            HEIGHT = 1
            END
            bculogicrelease_ PUSHBUTTON -1 -1 0 0
            VALUE "D:\users\labot\idl\adopt\adsec_lib\gui\housekeeping\ok.bmp"
            ALIGNCENTER
            BITMAP
            END
          END
          WID_BASE_47 BASE -1 -1 0 0
          ROWS = 1
          XPAD = 3
          YPAD = 3
          SPACE = 3
          CAPTION "IDL"
          BEGIN
            WID_LABEL_77 LABEL -1 -1 0 0
            VALUE "IPAddress"
            ALIGNLEFT
            END
            bcuipaddress TEXT -1 -1 0 0
            WIDTH = 20
            HEIGHT = 1
            END
            bcuipaddress_ PUSHBUTTON -1 -1 0 0
            VALUE "D:\users\labot\idl\adopt\adsec_lib\gui\housekeeping\ok.bmp"
            ALIGNCENTER
            BITMAP
            END
          END
          WID_BASE_48 BASE -1 -1 0 0
          ROWS = 1
          XPAD = 3
          YPAD = 3
          SPACE = 3
          CAPTION "IDL"
          BEGIN
            WID_LABEL_78 LABEL -1 -1 0 0
            VALUE "FramesCounter"
            ALIGNLEFT
            END
            bcuframescounter TEXT -1 -1 0 0
            WIDTH = 20
            HEIGHT = 1
            END
            bcuframescounter_ PUSHBUTTON -1 -1 0 0
            VALUE "D:\users\labot\idl\adopt\adsec_lib\gui\housekeeping\ok.bmp"
            ALIGNCENTER
            BITMAP
            END
          END
          WID_BASE_49 BASE -1 -1 0 0
          ROWS = 1
          XPAD = 3
          YPAD = 3
          SPACE = 3
          CAPTION "IDL"
          BEGIN
            WID_LABEL_79 LABEL -1 -1 0 0
            VALUE "SerialNumber"
            ALIGNLEFT
            END
            bcuserialnumber TEXT -1 -1 0 0
            WIDTH = 20
            HEIGHT = 1
            END
            bcuserialnumber_ PUSHBUTTON -1 -1 0 0
            VALUE "D:\users\labot\idl\adopt\adsec_lib\gui\housekeeping\ok.bmp"
            ALIGNCENTER
            BITMAP
            END
          END
          WID_BASE_50 BASE -1 -1 0 0
          ROWS = 1
          XPAD = 3
          YPAD = 3
          SPACE = 3
          CAPTION "IDL"
          BEGIN
            WID_LABEL_80 LABEL -1 -1 0 0
            VALUE "PowerBackplaneSerial"
            ALIGNLEFT
            END
            bcupowerbackplaneserial TEXT -1 -1 0 0
            WIDTH = 20
            HEIGHT = 1
            END
            bcupowerbackplaneserial_ PUSHBUTTON -1 -1 0 0
            VALUE "D:\users\labot\idl\adopt\adsec_lib\gui\housekeeping\ok.bmp"
            ALIGNCENTER
            BITMAP
            END
          END
          WID_BASE_51 BASE -1 -1 0 0
          ROWS = 1
          XPAD = 3
          YPAD = 3
          SPACE = 3
          CAPTION "IDL"
          BEGIN
            WID_LABEL_81 LABEL -1 -1 0 0
            VALUE "EnableMasterDiagnostic"
            ALIGNLEFT
            END
            bcuenablemasterdiagnostic TEXT -1 -1 0 0
            WIDTH = 20
            HEIGHT = 1
            END
            bcuenablemasterdiagnostic_ PUSHBUTTON -1 -1 0 0
            VALUE "D:\users\labot\idl\adopt\adsec_lib\gui\housekeeping\ok.bmp"
            ALIGNCENTER
            BITMAP
            END
          END
          WID_BASE_52 BASE -1 -1 0 0
          ROWS = 1
          XPAD = 3
          YPAD = 3
          SPACE = 3
          CAPTION "IDL"
          BEGIN
            WID_LABEL_82 LABEL -1 -1 0 0
            VALUE "DecimationFactor"
            ALIGNLEFT
            END
            bcudecimationfactor TEXT -1 -1 0 0
            WIDTH = 20
            HEIGHT = 1
            END
            bcudecimationfactor_ PUSHBUTTON -1 -1 0 0
            VALUE "D:\users\labot\idl\adopt\adsec_lib\gui\housekeeping\ok.bmp"
            ALIGNCENTER
            BITMAP
            END
          END
          WID_BASE_53 BASE -1 -1 0 0
          ROWS = 1
          XPAD = 3
          YPAD = 3
          SPACE = 3
          CAPTION "IDL"
          BEGIN
            WID_LABEL_83 LABEL -1 -1 0 0
            VALUE "RemoteIPAddress"
            ALIGNLEFT
            END
            bcuremoteipaddress TEXT -1 -1 0 0
            WIDTH = 20
            HEIGHT = 1
            END
            bcuremoteipaddress_ PUSHBUTTON -1 -1 0 0
            VALUE "D:\users\labot\idl\adopt\adsec_lib\gui\housekeeping\ok.bmp"
            ALIGNCENTER
            BITMAP
            END
          END
        END
        WID_BASE_7 BASE -1 -1 0 0
        COLUMNS = 1
        XPAD = 3
        YPAD = 3
        SPACE = 3
        CHILDALIGNRIGHT
        CAPTION "IDL"
        BEGIN
          WID_BASE_10 BASE -1 -1 0 0
          FRAME = 1
          COLUMNS = 1
          XPAD = 3
          YPAD = 3
          SPACE = 3
          CHILDALIGNRIGHT
          CAPTION "IDL"
          BEGIN
            WID_LABEL_8 LABEL -1 -1 0 0
            VALUE "Temperatures"
            ALIGNRIGHT
            END
            WID_BASE_82 BASE -1 -1 0 0
            ROWS = 1
            XPAD = 3
            YPAD = 3
            SPACE = 3
            CAPTION "IDL"
            BEGIN
              WID_LABEL_206 LABEL -1 -1 0 0
              VALUE "StratixTemp"
              ALIGNLEFT
              END
              bcustratixtemp TEXT -1 -1 0 0
              WIDTH = 20
              HEIGHT = 1
              END
              bcustratixtemp_ PUSHBUTTON -1 -1 0 0
              VALUE "D:\users\labot\idl\adopt\adsec_lib\gui\housekeeping\ok.bmp"
              ALIGNCENTER
              BITMAP
              END
            END
            WID_BASE_86 BASE -1 -1 0 0
            ROWS = 1
            XPAD = 3
            YPAD = 3
            SPACE = 3
            CAPTION "IDL"
            BEGIN
              WID_LABEL_207 LABEL -1 -1 0 0
              VALUE "PowerTemp"
              ALIGNLEFT
              END
              bcupowertemp TEXT -1 -1 0 0
              WIDTH = 20
              HEIGHT = 1
              END
              bcupowertemp_ PUSHBUTTON -1 -1 0 0
              VALUE "D:\users\labot\idl\adopt\adsec_lib\gui\housekeeping\ok.bmp"
              ALIGNCENTER
              BITMAP
              END
            END
          END
          WID_BASE_11 BASE -1 -1 0 0
          FRAME = 1
          COLUMNS = 1
          XPAD = 3
          YPAD = 3
          SPACE = 3
          CHILDALIGNRIGHT
          CAPTION "IDL"
          BEGIN
            WID_LABEL_9 LABEL -1 -1 0 0
            VALUE "Environment"
            ALIGNRIGHT
            END
            WID_BASE_96 BASE -1 -1 0 0
            ROWS = 1
            XPAD = 3
            YPAD = 3
            SPACE = 3
            CAPTION "IDL"
            BEGIN
              WID_LABEL_208 LABEL -1 -1 0 0
              VALUE "Cooler in0 temp."
              ALIGNLEFT
              END
              bcucoolerin0temp TEXT -1 -1 0 0
              WIDTH = 20
              HEIGHT = 1
              END
              bcucoolerin0temp_ PUSHBUTTON -1 -1 0 0
              VALUE "D:\users\labot\idl\adopt\adsec_lib\gui\housekeeping\ok.bmp"
              ALIGNCENTER
              BITMAP
              END
            END
            WID_BASE_124 BASE -1 -1 0 0
            ROWS = 1
            XPAD = 3
            YPAD = 3
            SPACE = 3
            CAPTION "IDL"
            BEGIN
              WID_LABEL_209 LABEL -1 -1 0 0
              VALUE "Cooler in1 temp."
              ALIGNLEFT
              END
              bcucoolerin1temp TEXT -1 -1 0 0
              WIDTH = 20
              HEIGHT = 1
              END
              bcucoolerin1temp_ PUSHBUTTON -1 -1 0 0
              VALUE "D:\users\labot\idl\adopt\adsec_lib\gui\housekeeping\ok.bmp"
              ALIGNCENTER
              BITMAP
              END
            END
            WID_BASE_125 BASE -1 -1 0 0
            ROWS = 1
            XPAD = 3
            YPAD = 3
            SPACE = 3
            CAPTION "IDL"
            BEGIN
              WID_LABEL_210 LABEL -1 -1 0 0
              VALUE "Cooler out0 temp."
              ALIGNLEFT
              END
              bcucoolerout0temp TEXT -1 -1 0 0
              WIDTH = 20
              HEIGHT = 1
              END
              bcucoolerout0temp_ PUSHBUTTON -1 -1 0 0
              VALUE "D:\users\labot\idl\adopt\adsec_lib\gui\housekeeping\ok.bmp"
              ALIGNCENTER
              BITMAP
              END
            END
            WID_BASE_126 BASE -1 -1 0 0
            ROWS = 1
            XPAD = 3
            YPAD = 3
            SPACE = 3
            CAPTION "IDL"
            BEGIN
              WID_LABEL_211 LABEL -1 -1 0 0
              VALUE "Cooler out1 temp"
              ALIGNLEFT
              END
              bcucoolerout1temp TEXT -1 -1 0 0
              WIDTH = 20
              HEIGHT = 1
              END
              bcucoolerout1temp_ PUSHBUTTON -1 -1 0 0
              VALUE "D:\users\labot\idl\adopt\adsec_lib\gui\housekeeping\ok.bmp"
              ALIGNCENTER
              BITMAP
              END
            END
            WID_BASE_127 BASE -1 -1 0 0
            ROWS = 1
            XPAD = 3
            YPAD = 3
            SPACE = 3
            CAPTION "IDL"
            BEGIN
              WID_LABEL_212 LABEL -1 -1 0 0
              VALUE "Cooler pressure"
              ALIGNLEFT
              END
              bcucoolerpressure TEXT -1 -1 0 0
              WIDTH = 20
              HEIGHT = 1
              END
              bcucoolerpressure_ PUSHBUTTON -1 -1 0 0
              VALUE "D:\users\labot\idl\adopt\adsec_lib\gui\housekeeping\ok.bmp"
              ALIGNCENTER
              BITMAP
              END
            END
            WID_BASE_128 BASE -1 -1 0 0
            ROWS = 1
            XPAD = 3
            YPAD = 3
            SPACE = 3
            CAPTION "IDL"
            BEGIN
              WID_LABEL_213 LABEL -1 -1 0 0
              VALUE "Humidity"
              ALIGNLEFT
              END
              bcuhumidity TEXT -1 -1 0 0
              WIDTH = 20
              HEIGHT = 1
              END
              bcuhumidity_ PUSHBUTTON -1 -1 0 0
              VALUE "D:\users\labot\idl\adopt\adsec_lib\gui\housekeeping\ok.bmp"
              ALIGNCENTER
              BITMAP
              END
            END
          END
        END
        WID_BASE_12 BASE -1 -1 0 0
        FRAME = 1
        COLUMNS = 1
        XPAD = 3
        YPAD = 3
        SPACE = 3
        CHILDALIGNRIGHT
        CAPTION "IDL"
        BEGIN
          WID_LABEL_10 LABEL -1 -1 0 0
          VALUE "Voltage and current levels"
          ALIGNRIGHT
          END
          WID_BASE_97 BASE -1 -1 0 0
          ROWS = 1
          XPAD = 3
          YPAD = 3
          SPACE = 3
          CAPTION "IDL"
          BEGIN
            WID_LABEL_214 LABEL -1 -1 0 0
            VALUE "VoltageVCCL"
            ALIGNLEFT
            END
            bcuvoltagevccl TEXT -1 -1 0 0
            WIDTH = 20
            HEIGHT = 1
            END
            bcuvoltagevccl_ PUSHBUTTON -1 -1 0 0
            VALUE "D:\users\labot\idl\adopt\adsec_lib\gui\housekeeping\ok.bmp"
            ALIGNCENTER
            BITMAP
            END
          END
          WID_BASE_129 BASE -1 -1 0 0
          ROWS = 1
          XPAD = 3
          YPAD = 3
          SPACE = 3
          CAPTION "IDL"
          BEGIN
            WID_LABEL_215 LABEL -1 -1 0 0
            VALUE "VoltageVCCA"
            ALIGNLEFT
            END
            bcuvoltagevcca TEXT -1 -1 0 0
            WIDTH = 20
            HEIGHT = 1
            END
            bcuvoltagevcca_ PUSHBUTTON -1 -1 0 0
            VALUE "D:\users\labot\idl\adopt\adsec_lib\gui\housekeeping\ok.bmp"
            ALIGNCENTER
            BITMAP
            END
          END
          WID_BASE_130 BASE -1 -1 0 0
          ROWS = 1
          XPAD = 3
          YPAD = 3
          SPACE = 3
          CAPTION "IDL"
          BEGIN
            WID_LABEL_216 LABEL -1 -1 0 0
            VALUE "VoltageVSSA"
            ALIGNLEFT
            END
            bcuvoltagevssa TEXT -1 -1 0 0
            WIDTH = 20
            HEIGHT = 1
            END
            bcuvoltagevssa_ PUSHBUTTON -1 -1 0 0
            VALUE "D:\users\labot\idl\adopt\adsec_lib\gui\housekeeping\ok.bmp"
            ALIGNCENTER
            BITMAP
            END
          END
          WID_BASE_131 BASE -1 -1 0 0
          ROWS = 1
          XPAD = 3
          YPAD = 3
          SPACE = 3
          CAPTION "IDL"
          BEGIN
            WID_LABEL_217 LABEL -1 -1 0 0
            VALUE "VoltageVCCP"
            ALIGNLEFT
            END
            bcuvoltagevccp TEXT -1 -1 0 0
            WIDTH = 20
            HEIGHT = 1
            END
            bcuvoltagevccp_ PUSHBUTTON -1 -1 0 0
            VALUE "D:\users\labot\idl\adopt\adsec_lib\gui\housekeeping\ok.bmp"
            ALIGNCENTER
            BITMAP
            END
          END
          WID_BASE_132 BASE -1 -1 0 0
          ROWS = 1
          XPAD = 3
          YPAD = 3
          SPACE = 3
          CAPTION "IDL"
          BEGIN
            WID_LABEL_218 LABEL -1 -1 0 0
            VALUE "VoltageVSSP"
            ALIGNLEFT
            END
            bcuvoltagevssp TEXT -1 -1 0 0
            WIDTH = 20
            HEIGHT = 1
            END
            bcuvoltagevssp_ PUSHBUTTON -1 -1 0 0
            VALUE "D:\users\labot\idl\adopt\adsec_lib\gui\housekeeping\ok.bmp"
            ALIGNCENTER
            BITMAP
            END
          END
          WID_BASE_133 BASE -1 -1 0 0
          ROWS = 1
          XPAD = 3
          YPAD = 3
          SPACE = 3
          CAPTION "IDL"
          BEGIN
            WID_LABEL_219 LABEL -1 -1 0 0
            VALUE "CurrentVCCL"
            ALIGNLEFT
            END
            bcucurrentvccl TEXT -1 -1 0 0
            WIDTH = 20
            HEIGHT = 1
            END
            bcucurrentvccl_ PUSHBUTTON -1 -1 0 0
            VALUE "D:\users\labot\idl\adopt\adsec_lib\gui\housekeeping\ok.bmp"
            ALIGNCENTER
            BITMAP
            END
          END
          WID_BASE_134 BASE -1 -1 0 0
          ROWS = 1
          XPAD = 3
          YPAD = 3
          SPACE = 3
          CAPTION "IDL"
          BEGIN
            WID_LABEL_220 LABEL -1 -1 0 0
            VALUE "CurrentVCCA"
            ALIGNLEFT
            END
            bcucurrentvcca TEXT -1 -1 0 0
            WIDTH = 20
            HEIGHT = 1
            END
            bcucurrentvcca_ PUSHBUTTON -1 -1 0 0
            VALUE "D:\users\labot\idl\adopt\adsec_lib\gui\housekeeping\ok.bmp"
            ALIGNCENTER
            BITMAP
            END
          END
          WID_BASE_135 BASE -1 -1 0 0
          ROWS = 1
          XPAD = 3
          YPAD = 3
          SPACE = 3
          CAPTION "IDL"
          BEGIN
            WID_LABEL_221 LABEL -1 -1 0 0
            VALUE "CurrentVSSA"
            ALIGNLEFT
            END
            bcucurrentvssa TEXT -1 -1 0 0
            WIDTH = 20
            HEIGHT = 1
            END
            bcucurrentvssa_ PUSHBUTTON -1 -1 0 0
            VALUE "D:\users\labot\idl\adopt\adsec_lib\gui\housekeeping\ok.bmp"
            ALIGNCENTER
            BITMAP
            END
          END
          WID_BASE_136 BASE -1 -1 0 0
          ROWS = 1
          XPAD = 3
          YPAD = 3
          SPACE = 3
          CAPTION "IDL"
          BEGIN
            WID_LABEL_222 LABEL -1 -1 0 0
            VALUE "CurrentVCCP"
            ALIGNLEFT
            END
            bcucurrentvccp TEXT -1 -1 0 0
            WIDTH = 20
            HEIGHT = 1
            END
            bcucurrentvccp_ PUSHBUTTON -1 -1 0 0
            VALUE "D:\users\labot\idl\adopt\adsec_lib\gui\housekeeping\ok.bmp"
            ALIGNCENTER
            BITMAP
            END
          END
          WID_BASE_137 BASE -1 -1 0 0
          ROWS = 1
          XPAD = 3
          YPAD = 3
          SPACE = 3
          CAPTION "IDL"
          BEGIN
            WID_LABEL_223 LABEL -1 -1 0 0
            VALUE "CurrentVSSP"
            ALIGNLEFT
            END
            bcucurrentvssp TEXT -1 -1 0 0
            WIDTH = 20
            HEIGHT = 1
            END
            bcucurrentvssp_ PUSHBUTTON -1 -1 0 0
            VALUE "D:\users\labot\idl\adopt\adsec_lib\gui\housekeeping\ok.bmp"
            ALIGNCENTER
            BITMAP
            END
          END
        END
      END
      WID_BASE_6 BASE -1 -1 0 0
      ROWS = 1
      XPAD = 3
      YPAD = 3
      SPACE = 3
      CAPTION "IDL"
      BEGIN
        bcu_crate_num1 SLIDER -1 -1 0 0
        CAPTION "Crate (0-5)"
        MAXIMUM = 5
        END
      END
    END
    CRATE_BCU2_BASE BASE 0 0 887 641
    FRAME = 1
    COLUMNS = 1
    XPAD = 3
    YPAD = 3
    SPACE = 3
    CAPTION "Crate BCU 2"
    BEGIN
      WID_BASE_1 BASE -1 -1 0 0
      ROWS = 1
      XPAD = 3
      YPAD = 3
      SPACE = 3
      CAPTION "IDL"
      BEGIN
        WID_BASE_89 BASE -1 -1 0 0
        FRAME = 1
        COLUMNS = 1
        XPAD = 3
        YPAD = 3
        SPACE = 3
        CAPTION "IDL"
        BEGIN
          WID_LABEL_34 LABEL -1 -1 0 0
          VALUE "Reset status"
          ALIGNLEFT
          END
          WID_BASE_91 BASE -1 -1 0 0
          COLUMNS = 1
          NONEXCLUSIVE
          CAPTION "IDL"
          BEGIN
            bcuresetstatusfpgareset PUSHBUTTON -1 -1 0 0
            VALUE "FPGA Reset"
            ALIGNLEFT
            NORELEASE
            END
          END
          WID_BASE_285 BASE -1 -1 0 0
          COLUMNS = 1
          NONEXCLUSIVE
          CAPTION "IDL"
          BEGIN
            bcuresetstatusbusreset PUSHBUTTON -1 -1 0 0
            VALUE "BUS Reset"
            ALIGNLEFT
            NORELEASE
            END
          END
          WID_BASE_286 BASE -1 -1 0 0
          COLUMNS = 1
          NONEXCLUSIVE
          CAPTION "IDL"
          BEGIN
            bcuresetstatusdspreset PUSHBUTTON -1 -1 0 0
            VALUE "DSP Reset"
            ALIGNLEFT
            NORELEASE
            END
          END
          WID_BASE_287 BASE -1 -1 0 0
          COLUMNS = 1
          NONEXCLUSIVE
          CAPTION "IDL"
          BEGIN
            bcuresetstatusflashreset PUSHBUTTON -1 -1 0 0
            VALUE "Flash Reset"
            ALIGNLEFT
            NORELEASE
            END
          END
          WID_BASE_288 BASE -1 -1 0 0
          COLUMNS = 1
          NONEXCLUSIVE
          CAPTION "IDL"
          BEGIN
            bcuresetstatuspcireset PUSHBUTTON -1 -1 0 0
            VALUE "PCI Reset"
            ALIGNLEFT
            NORELEASE
            END
          END
          WID_BASE_289 BASE -1 -1 0 0
          COLUMNS = 1
          NONEXCLUSIVE
          CAPTION "IDL"
          BEGIN
            bcuresetstatusbusdriverenable PUSHBUTTON -1 -1 0 0
            VALUE "BUS Driver Enable"
            ALIGNLEFT
            NORELEASE
            END
          END
          WID_BASE_290 BASE -1 -1 0 0
          COLUMNS = 1
          NONEXCLUSIVE
          CAPTION "IDL"
          BEGIN
            bcuresetstatusbusdriverenablestatus PUSHBUTTON -1 -1 0 0
            VALUE "BUS Driver Enable Status"
            ALIGNLEFT
            NORELEASE
            END
          END
          WID_BASE_291 BASE -1 -1 0 0
          COLUMNS = 1
          NONEXCLUSIVE
          CAPTION "IDL"
          BEGIN
            bcuresetstatusbuspowerfaultctrl PUSHBUTTON -1 -1 0 0
            VALUE "BUS Power Fault Ctrl"
            ALIGNLEFT
            NORELEASE
            END
          END
          WID_BASE_292 BASE -1 -1 0 0
          COLUMNS = 1
          NONEXCLUSIVE
          CAPTION "IDL"
          BEGIN
            bcuresetstatusbuspowerfault PUSHBUTTON -1 -1 0 0
            VALUE "BUS Power Fault"
            ALIGNLEFT
            NORELEASE
            END
          END
          WID_BASE_293 BASE -1 -1 0 0
          COLUMNS = 1
          NONEXCLUSIVE
          CAPTION "IDL"
          BEGIN
            bcuresetstatussystemwatchdog PUSHBUTTON -1 -1 0 0
            VALUE "System watchdog"
            ALIGNLEFT
            NORELEASE
            END
          END
        END
        WID_BASE_92 BASE -1 -1 0 0
        FRAME = 1
        COLUMNS = 1
        XPAD = 3
        YPAD = 3
        SPACE = 3
        CAPTION "IDL"
        BEGIN
          WID_LABEL_35 LABEL -1 -1 0 0
          VALUE "Digital I/O"
          ALIGNLEFT
          END
          WID_BASE_93 BASE -1 -1 0 0
          COLUMNS = 1
          NONEXCLUSIVE
          CAPTION "IDL"
          BEGIN
            bcudigitaliodriverenabled PUSHBUTTON -1 -1 96 22
            VALUE "Driver Enabled"
            ALIGNLEFT
            NORELEASE
            END
          END
          WID_BASE_294 BASE -1 -1 0 0
          COLUMNS = 1
          NONEXCLUSIVE
          CAPTION "IDL"
          BEGIN
            bcudigitaliocrateismaster PUSHBUTTON -1 -1 0 0
            VALUE "Crate is master"
            ALIGNLEFT
            NORELEASE
            END
          END
          WID_BASE_295 BASE -1 -1 0 0
          COLUMNS = 1
          NONEXCLUSIVE
          CAPTION "IDL"
          BEGIN
            bcudigitaliobussysfault PUSHBUTTON -1 -1 0 0
            VALUE "BUS system fault"
            ALIGNLEFT
            NORELEASE
            END
          END
          WID_BASE_296 BASE -1 -1 0 0
          COLUMNS = 1
          NONEXCLUSIVE
          CAPTION "IDL"
          BEGIN
            bcudigitaliovccfault PUSHBUTTON -1 -1 0 0
            VALUE "VCC fault"
            ALIGNLEFT
            NORELEASE
            END
          END
          WID_BASE_297 BASE -1 -1 0 0
          COLUMNS = 1
          NONEXCLUSIVE
          CAPTION "IDL"
          BEGIN
            bcudigitalioacpowerfault0 PUSHBUTTON -1 -1 0 0
            VALUE "AC power fault 0"
            ALIGNLEFT
            NORELEASE
            END
          END
          WID_BASE_298 BASE -1 -1 0 0
          COLUMNS = 1
          NONEXCLUSIVE
          CAPTION "IDL"
          BEGIN
            bcudigitalioacpowerfault1 PUSHBUTTON -1 -1 0 0
            VALUE "AC power fault 1"
            ALIGNLEFT
            NORELEASE
            END
          END
          WID_BASE_299 BASE -1 -1 0 0
          COLUMNS = 1
          NONEXCLUSIVE
          CAPTION "IDL"
          BEGIN
            bcudigitalioacpowerfault2 PUSHBUTTON -1 -1 0 0
            VALUE "AC power fault 2"
            ALIGNLEFT
            NORELEASE
            END
          END
          WID_BASE_300 BASE -1 -1 0 0
          COLUMNS = 1
          NONEXCLUSIVE
          CAPTION "IDL"
          BEGIN
            bcudigitalioicdisconnected PUSHBUTTON -1 -1 0 0
            VALUE "IC disconnected"
            ALIGNLEFT
            NORELEASE
            END
          END
          WID_BASE_301 BASE -1 -1 0 0
          COLUMNS = 1
          NONEXCLUSIVE
          CAPTION "IDL"
          BEGIN
            bcudigitalioovercurrent PUSHBUTTON -1 -1 0 0
            VALUE "Overcurrent"
            ALIGNLEFT
            NORELEASE
            END
          END
          WID_BASE_302 BASE -1 -1 0 0
          COLUMNS = 1
          NONEXCLUSIVE
          CAPTION "IDL"
          BEGIN
            bcudigitaliocrateid PUSHBUTTON -1 -1 0 0
            VALUE "Crate ID"
            ALIGNLEFT
            NORELEASE
            END
          END
        END
        WID_BASE_14 BASE -1 -1 0 0
        FRAME = 1
        COLUMNS = 1
        XPAD = 3
        YPAD = 3
        SPACE = 3
        CHILDALIGNRIGHT
        CAPTION "IDL"
        BEGIN
          WID_LABEL_13 LABEL -1 -1 0 0
          VALUE "Master crate global current levels"
          ALIGNRIGHT
          END
          WID_BASE_152 BASE -1 -1 0 0
          ROWS = 1
          XPAD = 3
          YPAD = 3
          SPACE = 3
          CAPTION "IDL"
          BEGIN
            WID_LABEL_245 LABEL -1 -1 0 0
            VALUE "CurrentVCCP"
            ALIGNLEFT
            END
            totalcurrentvccp TEXT -1 -1 0 0
            WIDTH = 20
            HEIGHT = 1
            END
            totalcurrentvccp_ PUSHBUTTON -1 -1 0 0
            VALUE "D:\users\labot\idl\adopt\adsec_lib\gui\housekeeping\ok.bmp"
            ALIGNCENTER
            BITMAP
            END
          END
          WID_BASE_153 BASE -1 -1 0 0
          ROWS = 1
          XPAD = 3
          YPAD = 3
          SPACE = 3
          CAPTION "IDL"
          BEGIN
            WID_LABEL_246 LABEL -1 -1 0 0
            VALUE "CurrentVSSP"
            ALIGNLEFT
            END
            totalcurrentvssp TEXT -1 -1 0 0
            WIDTH = 20
            HEIGHT = 1
            END
            totalcurrentvssp_ PUSHBUTTON -1 -1 0 0
            VALUE "D:\users\labot\idl\adopt\adsec_lib\gui\housekeeping\ok.bmp"
            ALIGNCENTER
            BITMAP
            END
          END
          WID_BASE_154 BASE -1 -1 0 0
          ROWS = 1
          XPAD = 3
          YPAD = 3
          SPACE = 3
          CAPTION "IDL"
          BEGIN
            WID_LABEL_247 LABEL -1 -1 0 0
            VALUE "CurrentVP"
            ALIGNLEFT
            END
            totalcurrentvp TEXT -1 -1 0 0
            WIDTH = 20
            HEIGHT = 1
            END
            totalcurrentvp_ PUSHBUTTON -1 -1 0 0
            VALUE "D:\users\labot\idl\adopt\adsec_lib\gui\housekeeping\ok.bmp"
            ALIGNCENTER
            BITMAP
            END
          END
        END
        WID_BASE_70 BASE -1 -1 0 0
        FRAME = 1
        COLUMNS = 1
        XPAD = 3
        YPAD = 3
        SPACE = 3
        CHILDALIGNRIGHT
        CAPTION "IDL"
        BEGIN
          WID_LABEL_163 LABEL -1 -1 0 0
          VALUE "DiagnApp data"
          ALIGNRIGHT
          END
          WID_BASE_79 BASE -1 -1 0 0
          ROWS = 1
          XPAD = 3
          YPAD = 3
          SPACE = 3
          CAPTION "IDL"
          BEGIN
            WID_LABEL_164 LABEL -1 -1 0 0
            VALUE "LocalCurrentThreshold"
            ALIGNLEFT
            END
            bculocalcurrentthreshold TEXT -1 -1 0 0
            WIDTH = 20
            HEIGHT = 1
            END
            bculocalcurrentthreshold_ PUSHBUTTON -1 -1 0 0
            VALUE "D:\users\labot\idl\adopt\adsec_lib\gui\housekeeping\ok.bmp"
            ALIGNCENTER
            BITMAP
            END
          END
          WID_BASE_84 BASE -1 -1 0 0
          ROWS = 1
          XPAD = 3
          YPAD = 3
          SPACE = 3
          CAPTION "IDL"
          BEGIN
            WID_LABEL_165 LABEL -1 -1 0 0
            VALUE "VPSet"
            ALIGNLEFT
            END
            bcuvpset TEXT -1 -1 0 0
            WIDTH = 20
            HEIGHT = 1
            END
            bcuvpset_ PUSHBUTTON -1 -1 0 0
            VALUE "D:\users\labot\idl\adopt\adsec_lib\gui\housekeeping\ok.bmp"
            ALIGNCENTER
            BITMAP
            END
          END
          WID_BASE_112 BASE -1 -1 0 0
          ROWS = 1
          XPAD = 3
          YPAD = 3
          SPACE = 3
          CAPTION "IDL"
          BEGIN
            WID_LABEL_166 LABEL -1 -1 0 0
            VALUE "TotalCurrentThresholdPos"
            ALIGNLEFT
            END
            bcutotalcurrentthresholdpos TEXT -1 -1 0 0
            WIDTH = 20
            HEIGHT = 1
            END
            bcutotalcurrentthresholdpos_ PUSHBUTTON -1 -1 0 0
            VALUE "D:\users\labot\idl\adopt\adsec_lib\gui\housekeeping\ok.bmp"
            ALIGNCENTER
            BITMAP
            END
          END
          WID_BASE_113 BASE -1 -1 0 0
          ROWS = 1
          XPAD = 3
          YPAD = 3
          SPACE = 3
          CAPTION "IDL"
          BEGIN
            WID_LABEL_167 LABEL -1 -1 0 0
            VALUE "TotalCurrentThresholdNeg"
            ALIGNLEFT
            END
            bcutotalcurrentthresholdneg TEXT -1 -1 0 0
            WIDTH = 20
            HEIGHT = 1
            END
            bcutotalcurrentthresholdneg_ PUSHBUTTON -1 -1 0 0
            VALUE "D:\users\labot\idl\adopt\adsec_lib\gui\housekeeping\ok.bmp"
            ALIGNCENTER
            BITMAP
            END
          END
        END
      END
      WID_BASE_8 BASE -1 -1 0 0
      ROWS = 1
      XPAD = 3
      YPAD = 3
      SPACE = 3
      CAPTION "IDL"
      BEGIN
        bcu_crate_num2 SLIDER -1 -1 0 0
        CAPTION "Crate (0-5)"
        MAXIMUM = 5
        END
      END
    END
    CRATE_DSP_BASE BASE 0 0 887 641
    FRAME = 1
    COLUMNS = 1
    XPAD = 3
    YPAD = 3
    CAPTION "Crate DSP"
    BEGIN
      WID_BASE_3 BASE -1 -1 0 0
      ROWS = 1
      XPAD = 3
      YPAD = 3
      SPACE = 3
      CAPTION "IDL"
      BEGIN
        WID_BASE_147 BASE -1 -1 0 0
        FRAME = 1
        COLUMNS = 1
        XPAD = 3
        YPAD = 3
        CHILDALIGNRIGHT
        CAPTION "IDL"
        BEGIN
          WID_LABEL_104 LABEL -1 -1 0 0
          VALUE "NIOS Fixed Area"
          ALIGNRIGHT
          END
          WID_BASE_148 BASE -1 -1 0 0
          ROWS = 1
          XPAD = 3
          YPAD = 3
          SPACE = 3
          CAPTION "IDL"
          BEGIN
            WID_LABEL_259 LABEL -1 -1 0 0
            VALUE "WhoAmI"
            ALIGNLEFT
            END
            dspwhoami TEXT -1 -1 0 0
            WIDTH = 20
            HEIGHT = 1
            END
            dspwhoami_ PUSHBUTTON -1 -1 0 0
            VALUE "D:\users\labot\idl\adopt\adsec_lib\gui\housekeeping\ok.bmp"
            ALIGNCENTER
            BITMAP
            END
          END
          WID_BASE_149 BASE -1 -1 0 0
          ROWS = 1
          XPAD = 3
          YPAD = 3
          SPACE = 3
          CAPTION "IDL"
          BEGIN
            WID_LABEL_260 LABEL -1 -1 0 0
            VALUE "SoftwareRelease"
            ALIGNLEFT
            END
            dspsoftwarerelease TEXT -1 -1 0 0
            WIDTH = 20
            HEIGHT = 1
            END
            dspsoftwarerelease_ PUSHBUTTON -1 -1 0 0
            VALUE "D:\users\labot\idl\adopt\adsec_lib\gui\housekeeping\ok.bmp"
            ALIGNCENTER
            BITMAP
            END
          END
          WID_BASE_150 BASE -1 -1 0 0
          ROWS = 1
          XPAD = 3
          YPAD = 3
          SPACE = 3
          CAPTION "IDL"
          BEGIN
            WID_LABEL_261 LABEL -1 -1 0 0
            VALUE "LogicRelease"
            ALIGNLEFT
            END
            dsplogicrelease TEXT -1 -1 0 0
            WIDTH = 20
            HEIGHT = 1
            END
            dsplogicrelease_ PUSHBUTTON -1 -1 0 0
            VALUE "D:\users\labot\idl\adopt\adsec_lib\gui\housekeeping\ok.bmp"
            ALIGNCENTER
            BITMAP
            END
          END
          WID_BASE_201 BASE -1 -1 0 0
          ROWS = 1
          XPAD = 3
          YPAD = 3
          SPACE = 3
          CAPTION "IDL"
          BEGIN
            WID_LABEL_262 LABEL -1 -1 0 0
            VALUE "SerialNumber"
            ALIGNLEFT
            END
            dspserialnumber TEXT -1 -1 0 0
            WIDTH = 20
            HEIGHT = 1
            END
            dspserialnumber_ PUSHBUTTON -1 -1 0 0
            VALUE "D:\users\labot\idl\adopt\adsec_lib\gui\housekeeping\ok.bmp"
            ALIGNCENTER
            BITMAP
            END
          END
          WID_BASE_202 BASE -1 -1 0 0
          ROWS = 1
          XPAD = 3
          YPAD = 3
          SPACE = 3
          CAPTION "IDL"
          BEGIN
            WID_LABEL_263 LABEL -1 -1 0 0
            VALUE "DiagnosticRecordPtr"
            ALIGNLEFT
            END
            dspdiagnosticrecordptr TEXT -1 -1 0 0
            WIDTH = 20
            HEIGHT = 1
            END
            dspdiagnosticrecordptr_ PUSHBUTTON -1 -1 0 0
            VALUE "D:\users\labot\idl\adopt\adsec_lib\gui\housekeeping\ok.bmp"
            ALIGNCENTER
            BITMAP
            END
          END
          WID_BASE_203 BASE -1 -1 0 0
          ROWS = 1
          XPAD = 3
          YPAD = 3
          SPACE = 3
          CAPTION "IDL"
          BEGIN
            WID_LABEL_264 LABEL -1 -1 0 0
            VALUE "DiagnosticRecordLen"
            ALIGNLEFT
            END
            dspdiagnosticrecordlen TEXT -1 -1 0 0
            WIDTH = 20
            HEIGHT = 1
            END
            dspdiagnosticrecordlen_ PUSHBUTTON -1 -1 0 0
            VALUE "D:\users\labot\idl\adopt\adsec_lib\gui\housekeeping\ok.bmp"
            ALIGNCENTER
            BITMAP
            END
          END
          WID_BASE_204 BASE -1 -1 0 0
          ROWS = 1
          XPAD = 3
          YPAD = 3
          SPACE = 3
          CAPTION "IDL"
          BEGIN
            WID_LABEL_265 LABEL -1 -1 0 0
            VALUE "RdDiagnosticRecordPtr"
            ALIGNLEFT
            END
            dsprddiagnosticrecordptr TEXT -1 -1 0 0
            WIDTH = 20
            HEIGHT = 1
            END
            dsprddiagnosticrecordptr_ PUSHBUTTON -1 -1 0 0
            VALUE "D:\users\labot\idl\adopt\adsec_lib\gui\housekeeping\ok.bmp"
            ALIGNCENTER
            BITMAP
            END
          END
          WID_BASE_205 BASE -1 -1 0 0
          ROWS = 1
          XPAD = 3
          YPAD = 3
          SPACE = 3
          CAPTION "IDL"
          BEGIN
            WID_LABEL_266 LABEL -1 -1 0 0
            VALUE "WrDiagnosticRecordPtr"
            ALIGNLEFT
            END
            dspwrdiagnosticrecordptr TEXT -1 -1 0 0
            WIDTH = 20
            HEIGHT = 1
            END
            dspwrdiagnosticrecordptr_ PUSHBUTTON -1 -1 0 0
            VALUE "D:\users\labot\idl\adopt\adsec_lib\gui\housekeeping\ok.bmp"
            ALIGNCENTER
            BITMAP
            END
          END
          WID_BASE_64 BASE -1 -1 0 0
          FRAME = 1
          COLUMNS = 1
          XPAD = 3
          YPAD = 3
          CHILDALIGNRIGHT
          CAPTION "IDL"
          BEGIN
            WID_BASE_206 BASE -1 -1 0 0
            ROWS = 1
            XPAD = 3
            YPAD = 3
            CAPTION "IDL"
            BEGIN
              WID_LABEL_267 LABEL -1 -1 0 0
              VALUE "ADCspiOffset"
              ALIGNLEFT
              END
              dspadcspioffset TEXT -1 -1 0 0
              WIDTH = 20
              HEIGHT = 1
              END
              dspadcspioffset_ PUSHBUTTON -1 -1 0 0
              VALUE "D:\users\labot\idl\adopt\adsec_lib\gui\housekeeping\ok.bmp"
              ALIGNCENTER
              BITMAP
              END
            END
            WID_BASE_207 BASE -1 -1 0 0
            ROWS = 1
            XPAD = 3
            YPAD = 3
            CAPTION "IDL"
            BEGIN
              WID_LABEL_268 LABEL -1 -1 0 0
              VALUE "ADCspiGain"
              ALIGNLEFT
              END
              dspadcspigain TEXT -1 -1 0 0
              WIDTH = 20
              HEIGHT = 1
              END
              dspadcspigain_ PUSHBUTTON -1 -1 0 0
              VALUE "D:\users\labot\idl\adopt\adsec_lib\gui\housekeeping\ok.bmp"
              ALIGNCENTER
              BITMAP
              END
            END
            WID_BASE_208 BASE -1 -1 0 0
            ROWS = 1
            XPAD = 3
            YPAD = 3
            CAPTION "IDL"
            BEGIN
              WID_LABEL_269 LABEL -1 -1 0 0
              VALUE "ADCOffset"
              ALIGNLEFT
              END
              dspadcoffset TEXT -1 -1 0 0
              WIDTH = 20
              HEIGHT = 1
              END
              dspadcoffset_ PUSHBUTTON -1 -1 0 0
              VALUE "D:\users\labot\idl\adopt\adsec_lib\gui\housekeeping\ok.bmp"
              ALIGNCENTER
              BITMAP
              END
            END
            WID_BASE_209 BASE -1 -1 0 0
            ROWS = 1
            XPAD = 3
            YPAD = 3
            CAPTION "IDL"
            BEGIN
              WID_LABEL_270 LABEL -1 -1 0 0
              VALUE "ADCGain"
              ALIGNLEFT
              END
              dspadcgain TEXT -1 -1 0 0
              WIDTH = 20
              HEIGHT = 1
              END
              dspadcgain_ PUSHBUTTON -1 -1 0 0
              VALUE "D:\users\labot\idl\adopt\adsec_lib\gui\housekeeping\ok.bmp"
              ALIGNCENTER
              BITMAP
              END
            END
            WID_BASE_210 BASE -1 -1 0 0
            ROWS = 1
            XPAD = 3
            YPAD = 3
            CAPTION "IDL"
            BEGIN
              WID_LABEL_271 LABEL -1 -1 0 0
              VALUE "DACOffset"
              ALIGNLEFT
              END
              dspdacoffset TEXT -1 -1 0 0
              WIDTH = 20
              HEIGHT = 1
              END
              dspdacoffset_ PUSHBUTTON -1 -1 0 0
              VALUE "D:\users\labot\idl\adopt\adsec_lib\gui\housekeeping\ok.bmp"
              ALIGNCENTER
              BITMAP
              END
            END
            WID_BASE_211 BASE -1 -1 0 0
            ROWS = 1
            XPAD = 3
            YPAD = 3
            CAPTION "IDL"
            BEGIN
              WID_LABEL_272 LABEL -1 -1 0 0
              VALUE "DACGain"
              ALIGNLEFT
              END
              dspdacgain TEXT -1 -1 0 0
              WIDTH = 20
              HEIGHT = 1
              END
              dspdacgain_ PUSHBUTTON -1 -1 0 0
              VALUE "D:\users\labot\idl\adopt\adsec_lib\gui\housekeeping\ok.bmp"
              ALIGNCENTER
              BITMAP
              END
            END
            WID_BASE_73 BASE -1 -1 0 0
            ROWS = 1
            XPAD = 3
            YPAD = 3
            CAPTION "IDL"
            BEGIN
              WID_LABEL_23 LABEL -1 -1 0 0
              VALUE "Board channel (0-7)"
              ALIGNCENTER
              END
              dsp_board_channel SLIDER -1 -1 0 0
              MAXIMUM = 7
              END
            END
          END
        END
        WID_BASE_74 BASE -1 -1 0 0
        COLUMNS = 1
        XPAD = 3
        YPAD = 3
        CHILDALIGNRIGHT
        CAPTION "IDL"
        BEGIN
          WID_BASE_75 BASE -1 -1 0 0
          FRAME = 1
          COLUMNS = 1
          XPAD = 3
          YPAD = 3
          CHILDALIGNRIGHT
          CAPTION "IDL"
          BEGIN
            WID_LABEL_24 LABEL -1 -1 0 0
            VALUE "Temperatures"
            ALIGNRIGHT
            END
            WID_BASE_212 BASE -1 -1 0 0
            ROWS = 1
            XPAD = 3
            YPAD = 3
            SPACE = 3
            CAPTION "IDL"
            BEGIN
              WID_LABEL_273 LABEL -1 -1 0 0
              VALUE "Stratix"
              ALIGNLEFT
              END
              dspstratixtemp TEXT -1 -1 0 0
              WIDTH = 20
              HEIGHT = 1
              END
              dspstratixtemp_ PUSHBUTTON -1 -1 0 0
              VALUE "D:\users\labot\idl\adopt\adsec_lib\gui\housekeeping\ok.bmp"
              ALIGNCENTER
              BITMAP
              END
            END
            WID_BASE_213 BASE -1 -1 0 0
            ROWS = 1
            XPAD = 3
            YPAD = 3
            SPACE = 3
            CAPTION "IDL"
            BEGIN
              WID_LABEL_274 LABEL -1 -1 0 0
              VALUE "Power"
              ALIGNLEFT
              END
              dsppowertemp TEXT -1 -1 0 0
              WIDTH = 20
              HEIGHT = 1
              END
              dsppowertemp_ PUSHBUTTON -1 -1 0 0
              VALUE "D:\users\labot\idl\adopt\adsec_lib\gui\housekeeping\ok.bmp"
              ALIGNCENTER
              BITMAP
              END
            END
            WID_BASE_214 BASE -1 -1 0 0
            ROWS = 1
            XPAD = 3
            YPAD = 3
            SPACE = 3
            CAPTION "IDL"
            BEGIN
              WID_LABEL_275 LABEL -1 -1 0 0
              VALUE "DSPs"
              ALIGNLEFT
              END
              dspdspstemp TEXT -1 -1 0 0
              WIDTH = 20
              HEIGHT = 1
              END
              dspdspstemp_ PUSHBUTTON -1 -1 0 0
              VALUE "D:\users\labot\idl\adopt\adsec_lib\gui\housekeeping\ok.bmp"
              ALIGNCENTER
              BITMAP
              END
            END
            WID_BASE_215 BASE -1 -1 0 0
            ROWS = 1
            XPAD = 3
            YPAD = 3
            SPACE = 3
            CAPTION "IDL"
            BEGIN
              WID_LABEL_276 LABEL -1 -1 0 0
              VALUE "Drivers"
              ALIGNLEFT
              END
              dspdrivertemp TEXT -1 -1 0 0
              WIDTH = 20
              HEIGHT = 1
              END
              dspdrivertemp_ PUSHBUTTON -1 -1 0 0
              VALUE "D:\users\labot\idl\adopt\adsec_lib\gui\housekeeping\ok.bmp"
              ALIGNCENTER
              BITMAP
              END
            END
          END
          WID_BASE_80 BASE -1 -1 0 0
          FRAME = 1
          COLUMNS = 1
          XPAD = 3
          YPAD = 3
          CAPTION "IDL"
          BEGIN
            WID_LABEL_25 LABEL -1 -1 0 0
            VALUE "Coil currents (SPI)"
            ALIGNRIGHT
            END
            WID_BASE_216 BASE -1 -1 0 0
            ROWS = 1
            XPAD = 3
            YPAD = 3
            SPACE = 3
            CAPTION "IDL"
            BEGIN
              WID_LABEL_277 LABEL -1 -1 0 0
              VALUE "ch0"
              ALIGNLEFT
              END
              dspcoilcurrent0 TEXT -1 -1 0 0
              WIDTH = 20
              HEIGHT = 1
              END
              dspcoilcurrent0_ PUSHBUTTON -1 -1 0 0
              VALUE "D:\users\labot\idl\adopt\adsec_lib\gui\housekeeping\ok.bmp"
              ALIGNCENTER
              BITMAP
              END
            END
            WID_BASE_217 BASE -1 -1 0 0
            ROWS = 1
            XPAD = 3
            YPAD = 3
            SPACE = 3
            CAPTION "IDL"
            BEGIN
              WID_LABEL_278 LABEL -1 -1 0 0
              VALUE "ch1"
              ALIGNLEFT
              END
              dspcoilcurrent1 TEXT -1 -1 0 0
              WIDTH = 20
              HEIGHT = 1
              END
              dspcoilcurrent1_ PUSHBUTTON -1 -1 0 0
              VALUE "D:\users\labot\idl\adopt\adsec_lib\gui\housekeeping\ok.bmp"
              ALIGNCENTER
              BITMAP
              END
            END
            WID_BASE_218 BASE -1 -1 0 0
            ROWS = 1
            XPAD = 3
            YPAD = 3
            SPACE = 3
            CAPTION "IDL"
            BEGIN
              WID_LABEL_279 LABEL -1 -1 0 0
              VALUE "ch2"
              ALIGNLEFT
              END
              dspcoilcurrent2 TEXT -1 -1 0 0
              WIDTH = 20
              HEIGHT = 1
              END
              dspcoilcurrent2_ PUSHBUTTON -1 -1 0 0
              VALUE "D:\users\labot\idl\adopt\adsec_lib\gui\housekeeping\ok.bmp"
              ALIGNCENTER
              BITMAP
              END
            END
            WID_BASE_219 BASE -1 -1 0 0
            ROWS = 1
            XPAD = 3
            YPAD = 3
            SPACE = 3
            CAPTION "IDL"
            BEGIN
              WID_LABEL_280 LABEL -1 -1 0 0
              VALUE "ch3"
              ALIGNLEFT
              END
              dspcoilcurrent3 TEXT -1 -1 0 0
              WIDTH = 20
              HEIGHT = 1
              END
              dspcoilcurrent3_ PUSHBUTTON -1 -1 0 0
              VALUE "D:\users\labot\idl\adopt\adsec_lib\gui\housekeeping\ok.bmp"
              ALIGNCENTER
              BITMAP
              END
            END
            WID_BASE_220 BASE -1 -1 0 0
            ROWS = 1
            XPAD = 3
            YPAD = 3
            SPACE = 3
            CAPTION "IDL"
            BEGIN
              WID_LABEL_281 LABEL -1 -1 0 0
              VALUE "ch4"
              ALIGNLEFT
              END
              dspcoilcurrent4 TEXT -1 -1 0 0
              WIDTH = 20
              HEIGHT = 1
              END
              dspcoilcurrent4_ PUSHBUTTON -1 -1 0 0
              VALUE "D:\users\labot\idl\adopt\adsec_lib\gui\housekeeping\ok.bmp"
              ALIGNCENTER
              BITMAP
              END
            END
            WID_BASE_221 BASE -1 -1 0 0
            ROWS = 1
            XPAD = 3
            YPAD = 3
            SPACE = 3
            CAPTION "IDL"
            BEGIN
              WID_LABEL_282 LABEL -1 -1 0 0
              VALUE "ch5"
              ALIGNLEFT
              END
              dspcoilcurrent5 TEXT -1 -1 0 0
              WIDTH = 20
              HEIGHT = 1
              END
              dspcoilcurrent5_ PUSHBUTTON -1 -1 0 0
              VALUE "D:\users\labot\idl\adopt\adsec_lib\gui\housekeeping\ok.bmp"
              ALIGNCENTER
              BITMAP
              END
            END
            WID_BASE_222 BASE -1 -1 0 0
            ROWS = 1
            XPAD = 3
            YPAD = 3
            SPACE = 3
            CAPTION "IDL"
            BEGIN
              WID_LABEL_283 LABEL -1 -1 0 0
              VALUE "ch6"
              ALIGNLEFT
              END
              dspcoilcurrent6 TEXT -1 -1 0 0
              WIDTH = 20
              HEIGHT = 1
              END
              dspcoilcurrent6_ PUSHBUTTON -1 -1 0 0
              VALUE "D:\users\labot\idl\adopt\adsec_lib\gui\housekeeping\ok.bmp"
              ALIGNCENTER
              BITMAP
              END
            END
            WID_BASE_223 BASE -1 -1 0 0
            ROWS = 1
            XPAD = 3
            YPAD = 3
            SPACE = 3
            CAPTION "IDL"
            BEGIN
              WID_LABEL_284 LABEL -1 -1 0 0
              VALUE "ch7"
              ALIGNLEFT
              END
              dspcoilcurrent7 TEXT -1 -1 0 0
              WIDTH = 20
              HEIGHT = 1
              END
              dspcoilcurrent7_ PUSHBUTTON -1 -1 0 0
              VALUE "D:\users\labot\idl\adopt\adsec_lib\gui\housekeeping\ok.bmp"
              ALIGNCENTER
              BITMAP
              END
            END
          END
        END
        WID_BASE_81 BASE -1 -1 0 0
        FRAME = 1
        COLUMNS = 1
        XPAD = 3
        YPAD = 3
        CAPTION "IDL"
        BEGIN
          WID_LABEL_26 LABEL -1 -1 0 0
          VALUE "Driver Status"
          ALIGNLEFT
          END
          WID_BASE_83 BASE -1 -1 0 0
          COLUMNS = 1
          NONEXCLUSIVE
          CAPTION "IDL"
          BEGIN
            dspdriverstatusfpgareset PUSHBUTTON -1 -1 0 0
            VALUE "FPGA Reset"
            ALIGNLEFT
            NORELEASE
            END
          END
          WID_BASE_224 BASE -1 -1 0 0
          COLUMNS = 1
          NONEXCLUSIVE
          CAPTION "IDL"
          BEGIN
            dspdriverstatusdsp0reset PUSHBUTTON -1 -1 0 0
            VALUE "DSP0 Reset"
            ALIGNLEFT
            NORELEASE
            END
          END
          WID_BASE_225 BASE -1 -1 0 0
          COLUMNS = 1
          NONEXCLUSIVE
          CAPTION "IDL"
          BEGIN
            dspdriverstatusdsp1reset PUSHBUTTON -1 -1 0 0
            VALUE "DSP1 Reset"
            ALIGNLEFT
            NORELEASE
            END
          END
          WID_BASE_226 BASE -1 -1 0 0
          COLUMNS = 1
          NONEXCLUSIVE
          CAPTION "IDL"
          BEGIN
            dspdriverstatusflashreset PUSHBUTTON -1 -1 0 0
            VALUE "Flash Reset"
            ALIGNLEFT
            NORELEASE
            END
          END
          WID_BASE_227 BASE -1 -1 0 0
          COLUMNS = 1
          NONEXCLUSIVE
          CAPTION "IDL"
          BEGIN
            dspdriverstatusbuspowerfaultctrl PUSHBUTTON -1 -1 0 0
            VALUE "Bus Power Fault Ctrl"
            ALIGNLEFT
            NORELEASE
            END
          END
          WID_BASE_228 BASE -1 -1 0 0
          COLUMNS = 1
          NONEXCLUSIVE
          CAPTION "IDL"
          BEGIN
            dspdriverstatusbuspowerfault PUSHBUTTON -1 -1 0 0
            VALUE "BUS Power Fault"
            ALIGNLEFT
            NORELEASE
            END
          END
          WID_BASE_229 BASE -1 -1 0 0
          COLUMNS = 1
          NONEXCLUSIVE
          CAPTION "IDL"
          BEGIN
            dspdriverstatusbusdriverenable PUSHBUTTON -1 -1 0 0
            VALUE "BUS Driver Enable"
            ALIGNLEFT
            NORELEASE
            END
          END
          WID_BASE_230 BASE -1 -1 0 0
          COLUMNS = 1
          NONEXCLUSIVE
          CAPTION "IDL"
          BEGIN
            dspdriverstatusenabledspwatchdog PUSHBUTTON -1 -1 0 0
            VALUE "Enable DSP Watchdog"
            ALIGNLEFT
            NORELEASE
            END
          END
          WID_BASE_231 BASE -1 -1 0 0
          COLUMNS = 1
          NONEXCLUSIVE
          CAPTION "IDL"
          BEGIN
            dspdriverstatusdsp0watchdogexpired PUSHBUTTON -1 -1 0 0
            VALUE "DSP0 watchdog expired"
            ALIGNLEFT
            NORELEASE
            END
          END
          WID_BASE_232 BASE -1 -1 0 0
          COLUMNS = 1
          NONEXCLUSIVE
          CAPTION "IDL"
          BEGIN
            dspdriverstatusdsp1watchdogexpired PUSHBUTTON -1 -1 0 0
            VALUE "DSP1 watchdog expired"
            ALIGNLEFT
            NORELEASE
            END
          END
          WID_BASE_233 BASE -1 -1 0 0
          FRAME = 1
          COLUMNS = 1
          XPAD = 3
          YPAD = 3
          CAPTION "IDL"
          BEGIN
            WID_LABEL_27 LABEL -1 -1 0 0
            VALUE "Driver Enabled"
            ALIGNLEFT
            END
            WID_BASE_234 BASE -1 -1 0 0
            ROWS = 1
            XPAD = 3
            YPAD = 3
            CAPTION "IDL"
            BEGIN
              WID_BASE_235 BASE -1 -1 0 0
              COLUMNS = 1
              XPAD = 3
              YPAD = 3
              CAPTION "IDL"
              BEGIN
                WID_BASE_236 BASE -1 -1 0 0
                COLUMNS = 1
                NONEXCLUSIVE
                CAPTION "IDL"
                BEGIN
                  dspdriverstatusdriverenable0 PUSHBUTTON -1 -1 0 0
                  VALUE "ch0"
                  ALIGNLEFT
                  END
                END
                WID_BASE_237 BASE -1 -1 0 0
                COLUMNS = 1
                NONEXCLUSIVE
                CAPTION "IDL"
                BEGIN
                  dspdriverstatusdriverenable2 PUSHBUTTON -1 -1 0 0
                  VALUE "ch2"
                  ALIGNLEFT
                  END
                END
                WID_BASE_238 BASE -1 -1 0 0
                COLUMNS = 1
                NONEXCLUSIVE
                CAPTION "IDL"
                BEGIN
                  dspdriverstatusdriverenable4 PUSHBUTTON -1 -1 0 0
                  VALUE "ch4"
                  ALIGNLEFT
                  END
                END
                WID_BASE_239 BASE -1 -1 0 0
                COLUMNS = 1
                NONEXCLUSIVE
                CAPTION "IDL"
                BEGIN
                  dspdriverstatusdriverenable6 PUSHBUTTON -1 -1 0 0
                  VALUE "ch6"
                  ALIGNLEFT
                  END
                END
              END
              WID_BASE_240 BASE -1 -1 0 0
              COLUMNS = 1
              XPAD = 3
              YPAD = 3
              CAPTION "IDL"
              BEGIN
                WID_BASE_241 BASE -1 -1 0 0
                COLUMNS = 1
                NONEXCLUSIVE
                CAPTION "IDL"
                BEGIN
                  dspdriverstatusdriverenable1 PUSHBUTTON -1 -1 0 0
                  VALUE "ch1"
                  ALIGNLEFT
                  END
                END
                WID_BASE_242 BASE -1 -1 0 0
                COLUMNS = 1
                NONEXCLUSIVE
                CAPTION "IDL"
                BEGIN
                  dspdriverstatusdriverenable3 PUSHBUTTON -1 -1 0 0
                  VALUE "ch3"
                  ALIGNLEFT
                  END
                END
                WID_BASE_243 BASE -1 -1 0 0
                COLUMNS = 1
                NONEXCLUSIVE
                CAPTION "IDL"
                BEGIN
                  dspdriverstatusdriverenable5 PUSHBUTTON -1 -1 0 0
                  VALUE "ch5"
                  ALIGNLEFT
                  END
                END
                WID_BASE_244 BASE -1 -1 0 0
                COLUMNS = 1
                NONEXCLUSIVE
                CAPTION "IDL"
                BEGIN
                  dspdriverstatusdriverenable7 PUSHBUTTON -1 -1 0 0
                  VALUE "ch7"
                  ALIGNLEFT
                  END
                END
              END
            END
          END
        END
      END
      WID_BASE_5 BASE -1 -1 0 0
      ROWS = 1
      XPAD = 3
      YPAD = 3
      CAPTION "IDL"
      BEGIN
        dsp_board_num SLIDER -1 -1 0 0
        CAPTION "DSP Bosrd (0-83)"
        MAXIMUM = 83
        END
      END
    END
    CRATE_SIGGEN_BASE BASE 0 0 887 641
    COLUMNS = 1
    XPAD = 3
    YPAD = 3
    SPACE = 3
    CAPTION "Crate SIGGEN"
    BEGIN
      WID_BASE_54 BASE -1 -1 0 0
      FRAME = 1
      COLUMNS = 1
      XPAD = 3
      YPAD = 3
      SPACE = 3
      CHILDALIGNRIGHT
      CAPTION "IDL"
      BEGIN
        WID_LABEL_84 LABEL -1 -1 0 0
        VALUE "NIOS Fixed Area"
        ALIGNRIGHT
        END
        WID_BASE_55 BASE -1 -1 0 0
        ROWS = 1
        XPAD = 3
        YPAD = 3
        SPACE = 3
        CAPTION "IDL"
        BEGIN
          WID_LABEL_85 LABEL -1 -1 0 0
          VALUE "WhoAmI"
          ALIGNLEFT
          END
          sgnwhoami TEXT -1 -1 0 0
          WIDTH = 20
          HEIGHT = 1
          END
          sgnwhoami_ PUSHBUTTON -1 -1 0 0
          VALUE "D:\users\labot\idl\adopt\adsec_lib\gui\housekeeping\ok.bmp"
          ALIGNCENTER
          BITMAP
          END
        END
        WID_BASE_56 BASE -1 -1 0 0
        ROWS = 1
        XPAD = 3
        YPAD = 3
        SPACE = 3
        CAPTION "IDL"
        BEGIN
          WID_LABEL_86 LABEL -1 -1 0 0
          VALUE "SoftwareRelease"
          ALIGNLEFT
          END
          sgnsoftwarerelease TEXT -1 -1 0 0
          WIDTH = 20
          HEIGHT = 1
          END
          sgnsoftwarerelease_ PUSHBUTTON -1 -1 0 0
          VALUE "D:\users\labot\idl\adopt\adsec_lib\gui\housekeeping\ok.bmp"
          ALIGNCENTER
          BITMAP
          END
        END
        WID_BASE_65 BASE -1 -1 0 0
        ROWS = 1
        XPAD = 3
        YPAD = 3
        SPACE = 3
        CAPTION "IDL"
        BEGIN
          WID_LABEL_87 LABEL -1 -1 0 0
          VALUE "LogicRelease"
          ALIGNLEFT
          END
          sgnlogicrelease TEXT -1 -1 0 0
          WIDTH = 20
          HEIGHT = 1
          END
          sgnlogicrelease_ PUSHBUTTON -1 -1 0 0
          VALUE "D:\users\labot\idl\adopt\adsec_lib\gui\housekeeping\ok.bmp"
          ALIGNCENTER
          BITMAP
          END
        END
        WID_BASE_67 BASE -1 -1 0 0
        ROWS = 1
        XPAD = 3
        YPAD = 3
        SPACE = 3
        CAPTION "IDL"
        BEGIN
          WID_LABEL_88 LABEL -1 -1 0 0
          VALUE "SerialNumber"
          ALIGNLEFT
          END
          sgnserialnumber TEXT -1 -1 0 0
          WIDTH = 20
          HEIGHT = 1
          END
          sgnserialnumber_ PUSHBUTTON -1 -1 0 0
          VALUE "D:\users\labot\idl\adopt\adsec_lib\gui\housekeeping\ok.bmp"
          ALIGNCENTER
          BITMAP
          END
        END
      END
      WID_BASE_68 BASE -1 -1 0 0
      FRAME = 1
      COLUMNS = 1
      XPAD = 3
      YPAD = 3
      SPACE = 3
      CAPTION "IDL"
      BEGIN
        WID_LABEL_21 LABEL -1 -1 0 0
        VALUE "Temperatures"
        ALIGNRIGHT
        END
        WID_BASE_200 BASE -1 -1 0 0
        ROWS = 1
        XPAD = 3
        YPAD = 3
        SPACE = 3
        CAPTION "IDL"
        BEGIN
          WID_LABEL_258 LABEL -1 -1 0 0
          VALUE "Stratix"
          ALIGNLEFT
          END
          sgnstratixtemp TEXT -1 -1 0 0
          WIDTH = 20
          HEIGHT = 1
          END
          sgnstratixtemp_ PUSHBUTTON -1 -1 0 0
          VALUE "D:\users\labot\idl\adopt\adsec_lib\gui\housekeeping\ok.bmp"
          ALIGNCENTER
          BITMAP
          END
        END
        WID_BASE_247 BASE -1 -1 0 0
        ROWS = 1
        XPAD = 3
        YPAD = 3
        SPACE = 3
        CAPTION "IDL"
        BEGIN
          WID_LABEL_287 LABEL -1 -1 0 0
          VALUE "Power"
          ALIGNLEFT
          END
          sgnpowertemp TEXT -1 -1 0 0
          WIDTH = 20
          HEIGHT = 1
          END
          sgnpowertemp_ PUSHBUTTON -1 -1 0 0
          VALUE "D:\users\labot\idl\adopt\adsec_lib\gui\housekeeping\ok.bmp"
          ALIGNCENTER
          BITMAP
          END
        END
        WID_BASE_249 BASE -1 -1 0 0
        ROWS = 1
        XPAD = 3
        YPAD = 3
        SPACE = 3
        CAPTION "IDL"
        BEGIN
          WID_LABEL_289 LABEL -1 -1 0 0
          VALUE "DSPs"
          ALIGNLEFT
          END
          sgndspstemp TEXT -1 -1 0 0
          WIDTH = 20
          HEIGHT = 1
          END
          sgndspstemp_ PUSHBUTTON -1 -1 0 0
          VALUE "D:\users\labot\idl\adopt\adsec_lib\gui\housekeeping\ok.bmp"
          ALIGNCENTER
          BITMAP
          END
        END
      END
      WID_BASE_9 BASE -1 -1 0 0
      ROWS = 1
      XPAD = 3
      YPAD = 3
      SPACE = 3
      CAPTION "IDL"
      BEGIN
        sgn_crate_num SLIDER -1 -1 0 0
        CAPTION "Crate (0-5)"
        MAXIMUM = 5
        END
      END
    END
    SWITCH_BCU_BASE BASE 0 0 887 641
    ROWS = 1
    XPAD = 3
    YPAD = 3
    SPACE = 3
    CAPTION "Switch BCU"
    BEGIN
      WID_BASE_66 BASE -1 -1 0 0
      FRAME = 1
      COLUMNS = 1
      XPAD = 3
      YPAD = 3
      SPACE = 3
      CHILDALIGNRIGHT
      CAPTION "IDL"
      BEGIN
        WID_LABEL_139 LABEL -1 -1 0 0
        VALUE "NIOS Fixed Area"
        ALIGNRIGHT
        END
        WID_BASE_71 BASE -1 -1 0 0
        ROWS = 1
        XPAD = 3
        YPAD = 3
        SPACE = 3
        CAPTION "IDL"
        BEGIN
          WID_LABEL_140 LABEL -1 -1 0 0
          VALUE "CrateID"
          ALIGNLEFT
          END
          switchcrateid TEXT -1 -1 0 0
          WIDTH = 20
          HEIGHT = 1
          END
          switchcrateid_ PUSHBUTTON -1 -1 0 0
          VALUE "D:\users\labot\idl\adopt\adsec_lib\gui\housekeeping\ok.bmp"
          ALIGNCENTER
          BITMAP
          END
        END
        WID_BASE_72 BASE -1 -1 0 0
        ROWS = 1
        XPAD = 3
        YPAD = 3
        SPACE = 3
        CAPTION "IDL"
        BEGIN
          WID_LABEL_141 LABEL -1 -1 0 0
          VALUE "WhoAmI"
          ALIGNLEFT
          END
          switchwhoami TEXT -1 -1 0 0
          WIDTH = 20
          HEIGHT = 1
          END
          switchwhoami_ PUSHBUTTON -1 -1 0 0
          VALUE "D:\users\labot\idl\adopt\adsec_lib\gui\housekeeping\ok.bmp"
          ALIGNCENTER
          BITMAP
          END
        END
        WID_BASE_98 BASE -1 -1 0 0
        ROWS = 1
        XPAD = 3
        YPAD = 3
        SPACE = 3
        CAPTION "IDL"
        BEGIN
          WID_LABEL_142 LABEL -1 -1 0 0
          VALUE "SoftwareRelease"
          ALIGNLEFT
          END
          switchsoftwarerelease TEXT -1 -1 0 0
          WIDTH = 20
          HEIGHT = 1
          END
          switchsoftwarerelease_ PUSHBUTTON -1 -1 0 0
          VALUE "D:\users\labot\idl\adopt\adsec_lib\gui\housekeeping\ok.bmp"
          ALIGNCENTER
          BITMAP
          END
        END
        WID_BASE_99 BASE -1 -1 0 0
        ROWS = 1
        XPAD = 3
        YPAD = 3
        SPACE = 3
        CAPTION "IDL"
        BEGIN
          WID_LABEL_143 LABEL -1 -1 0 0
          VALUE "LogicRelease"
          ALIGNLEFT
          END
          switchlogicrelease TEXT -1 -1 0 0
          WIDTH = 20
          HEIGHT = 1
          END
          switchlogicrelease_ PUSHBUTTON -1 -1 0 0
          VALUE "D:\users\labot\idl\adopt\adsec_lib\gui\housekeeping\ok.bmp"
          ALIGNCENTER
          BITMAP
          END
        END
        WID_BASE_100 BASE -1 -1 0 0
        ROWS = 1
        XPAD = 3
        YPAD = 3
        SPACE = 3
        CAPTION "IDL"
        BEGIN
          WID_LABEL_144 LABEL -1 -1 0 0
          VALUE "IPAddress"
          ALIGNLEFT
          END
          switchipaddress TEXT -1 -1 0 0
          WIDTH = 20
          HEIGHT = 1
          END
          switchipaddress_ PUSHBUTTON -1 -1 0 0
          VALUE "D:\users\labot\idl\adopt\adsec_lib\gui\housekeeping\ok.bmp"
          ALIGNCENTER
          BITMAP
          END
        END
        WID_BASE_101 BASE -1 -1 0 0
        ROWS = 1
        XPAD = 3
        YPAD = 3
        SPACE = 3
        CAPTION "IDL"
        BEGIN
          WID_LABEL_145 LABEL -1 -1 0 0
          VALUE "FramesCounter"
          ALIGNLEFT
          END
          switchframescounter TEXT -1 -1 0 0
          WIDTH = 20
          HEIGHT = 1
          END
          switchframescounter_ PUSHBUTTON -1 -1 0 0
          VALUE "D:\users\labot\idl\adopt\adsec_lib\gui\housekeeping\ok.bmp"
          ALIGNCENTER
          BITMAP
          END
        END
        WID_BASE_102 BASE -1 -1 0 0
        ROWS = 1
        XPAD = 3
        YPAD = 3
        SPACE = 3
        CAPTION "IDL"
        BEGIN
          WID_LABEL_146 LABEL -1 -1 0 0
          VALUE "SerialNumber"
          ALIGNLEFT
          END
          switchserialnumber TEXT -1 -1 0 0
          WIDTH = 20
          HEIGHT = 1
          END
          switchserialnumber_ PUSHBUTTON -1 -1 0 0
          VALUE "D:\users\labot\idl\adopt\adsec_lib\gui\housekeeping\ok.bmp"
          ALIGNCENTER
          BITMAP
          END
        END
        WID_BASE_103 BASE -1 -1 0 0
        ROWS = 1
        XPAD = 3
        YPAD = 3
        SPACE = 3
        CAPTION "IDL"
        BEGIN
          WID_LABEL_147 LABEL -1 -1 0 0
          VALUE "PowerBackplaneSerial"
          ALIGNLEFT
          END
          switchpowerbackplaneserial TEXT -1 -1 0 0
          WIDTH = 20
          HEIGHT = 1
          END
          switchpowerbackplaneserial_ PUSHBUTTON -1 -1 0 0
          VALUE "D:\users\labot\idl\adopt\adsec_lib\gui\housekeeping\ok.bmp"
          ALIGNCENTER
          BITMAP
          END
        END
        WID_BASE_104 BASE -1 -1 0 0
        ROWS = 1
        XPAD = 3
        YPAD = 3
        SPACE = 3
        CAPTION "IDL"
        BEGIN
          WID_LABEL_148 LABEL -1 -1 0 0
          VALUE "EnableMasterDiagnostic"
          ALIGNLEFT
          END
          switchenablemasterdiagnostic TEXT -1 -1 0 0
          WIDTH = 20
          HEIGHT = 1
          END
          switchenablemasterdiagnostic_ PUSHBUTTON -1 -1 0 0
          VALUE "D:\users\labot\idl\adopt\adsec_lib\gui\housekeeping\ok.bmp"
          ALIGNCENTER
          BITMAP
          END
        END
        WID_BASE_105 BASE -1 -1 0 0
        ROWS = 1
        XPAD = 3
        YPAD = 3
        SPACE = 3
        CAPTION "IDL"
        BEGIN
          WID_LABEL_149 LABEL -1 -1 0 0
          VALUE "DecimationFactor"
          ALIGNLEFT
          END
          switchdecimationfactor TEXT -1 -1 0 0
          WIDTH = 20
          HEIGHT = 1
          END
          switchdecimationfactor_ PUSHBUTTON -1 -1 0 0
          VALUE "D:\users\labot\idl\adopt\adsec_lib\gui\housekeeping\ok.bmp"
          ALIGNCENTER
          BITMAP
          END
        END
        WID_BASE_106 BASE -1 -1 0 0
        ROWS = 1
        XPAD = 3
        YPAD = 3
        SPACE = 3
        CAPTION "IDL"
        BEGIN
          WID_LABEL_150 LABEL -1 -1 0 0
          VALUE "RemoteIPAddress"
          ALIGNLEFT
          END
          switchremoteipaddress TEXT -1 -1 0 0
          WIDTH = 20
          HEIGHT = 1
          END
          switchremoteipaddress_ PUSHBUTTON -1 -1 0 0
          VALUE "D:\users\labot\idl\adopt\adsec_lib\gui\housekeeping\ok.bmp"
          ALIGNCENTER
          BITMAP
          END
        END
      END
      WID_BASE_76 BASE -1 -1 0 0
      FRAME = 1
      COLUMNS = 1
      XPAD = 3
      YPAD = 3
      SPACE = 3
      CAPTION "IDL"
      BEGIN
        WID_LABEL_29 LABEL -1 -1 0 0
        VALUE "Temperatures"
        ALIGNRIGHT
        END
        WID_BASE_245 BASE -1 -1 0 0
        ROWS = 1
        XPAD = 3
        YPAD = 3
        SPACE = 3
        CAPTION "IDL"
        BEGIN
          WID_LABEL_285 LABEL -1 -1 0 0
          VALUE "Stratix"
          ALIGNLEFT
          END
          switchstratixtemp TEXT -1 -1 0 0
          WIDTH = 20
          HEIGHT = 1
          END
          switchstratixtemp_ PUSHBUTTON -1 -1 0 0
          VALUE "D:\users\labot\idl\adopt\adsec_lib\gui\housekeeping\ok.bmp"
          ALIGNCENTER
          BITMAP
          END
        END
        WID_BASE_246 BASE -1 -1 0 0
        ROWS = 1
        XPAD = 3
        YPAD = 3
        SPACE = 3
        CAPTION "IDL"
        BEGIN
          WID_LABEL_286 LABEL -1 -1 0 0
          VALUE "Power"
          ALIGNLEFT
          END
          switchpowertemp TEXT -1 -1 0 0
          WIDTH = 20
          HEIGHT = 1
          END
          switchpowertemp_ PUSHBUTTON -1 -1 0 0
          VALUE "D:\users\labot\idl\adopt\adsec_lib\gui\housekeeping\ok.bmp"
          ALIGNCENTER
          BITMAP
          END
        END
      END
    END
  END
END
