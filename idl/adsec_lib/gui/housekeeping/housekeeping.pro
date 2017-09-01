; 
; IDL Widget Interface Procedures. This Code is automatically 
;     generated and should not be modified.

; 
; Generated on:	02/12/2008 02:22.48
; 
pro HOUSEKEEPING_BASE_event, Event

  wTarget = (widget_info(Event.id,/NAME) eq 'TREE' ?  $
      widget_info(Event.id, /tree_root) : event.id)


  wWidget =  Event.top

  case wTarget of

    Widget_Info(wWidget, FIND_BY_UNAME='HOUSEKEEPING_BASE'): begin
      if( Tag_Names(Event, /STRUCTURE_NAME) eq 'WIDGET_TIMER' )then $
        housekeeping_on_timer, Event
    end
    Widget_Info(wWidget, FIND_BY_UNAME='start'): begin
      if( Tag_Names(Event, /STRUCTURE_NAME) eq 'WIDGET_BUTTON' )then $
        start_btn_on_button_press, Event
    end
    Widget_Info(wWidget, FIND_BY_UNAME='exit'): begin
      if( Tag_Names(Event, /STRUCTURE_NAME) eq 'WIDGET_BUTTON' )then $
        exit_btn_on_button_press, Event
    end
    else:
  endcase

end

pro HOUSEKEEPING_BASE, GROUP_LEADER=wGroup, _EXTRA=_VWBExtra_

  Resolve_Routine, 'housekeeping_eventcb',/COMPILE_FULL_FILE  ; Load event callback routines
  
  HOUSEKEEPING_BASE = Widget_Base( GROUP_LEADER=wGroup,  $
      UNAME='HOUSEKEEPING_BASE' ,XOFFSET=5 ,YOFFSET=5 ,SCR_XSIZE=909  $
      ,SCR_YSIZE=704 ,NOTIFY_REALIZE='housekeeping_on_realize'  $
      ,TITLE='LBT672 Housekeeping' ,SPACE=3 ,XPAD=3 ,YPAD=3  $
      ,COLUMN=1)

  
  housekeeping_post_creation, HOUSEKEEPING_BASE, _EXTRA=_VWBExtra_

  
  WID_TAB_0 = Widget_Tab(HOUSEKEEPING_BASE, UNAME='WID_TAB_0'  $
      ,XOFFSET=3 ,YOFFSET=3 ,SCR_XSIZE=895 ,SCR_YSIZE=667)

  
  GENERAL_SUMMARY_BASE = Widget_Base(WID_TAB_0,  $
      UNAME='GENERAL_SUMMARY_BASE' ,SCR_XSIZE=887 ,SCR_YSIZE=641  $
      ,TITLE='System Summary' ,SPACE=3 ,XPAD=3 ,YPAD=3 ,ROW=1)

  
  WID_BASE_0 = Widget_Base(GENERAL_SUMMARY_BASE, UNAME='WID_BASE_0'  $
      ,XOFFSET=3 ,YOFFSET=3 ,TITLE='IDL' ,SPACE=3 ,XPAD=3 ,YPAD=3  $
      ,ROW=1)

  
  start = Widget_Button(WID_BASE_0, UNAME='start' ,XOFFSET=3  $
      ,YOFFSET=3 ,SCR_XSIZE=37 ,SCR_YSIZE=18 ,/ALIGN_CENTER  $
      ,VALUE='Start')

  
  exit = Widget_Button(WID_BASE_0, UNAME='exit' ,XOFFSET=43  $
      ,YOFFSET=3 ,SCR_XSIZE=37 ,SCR_YSIZE=18 ,/ALIGN_CENTER  $
      ,VALUE='Exit')

  
  WID_BASE_69 = Widget_Base(GENERAL_SUMMARY_BASE, UNAME='WID_BASE_69'  $
      ,FRAME=1 ,XOFFSET=89 ,YOFFSET=3 ,/BASE_ALIGN_RIGHT ,TITLE='IDL'  $
      ,SPACE=3 ,XPAD=3 ,YPAD=3 ,COLUMN=1)

  
  WID_LABEL_151 = Widget_Label(WID_BASE_69, UNAME='WID_LABEL_151'  $
      ,XOFFSET=218 ,YOFFSET=3 ,/ALIGN_RIGHT ,VALUE='DiagnApp data')

  
  WID_BASE_77 = Widget_Base(WID_BASE_69, UNAME='WID_BASE_77'  $
      ,XOFFSET=4 ,YOFFSET=21 ,TITLE='IDL' ,SPACE=3 ,XPAD=3 ,YPAD=3  $
      ,ROW=1)

  
  WID_LABEL_152 = Widget_Label(WID_BASE_77, UNAME='WID_LABEL_152'  $
      ,XOFFSET=3 ,YOFFSET=3 ,/ALIGN_LEFT  $
      ,VALUE='DiagnAppFrameTimestamp')

  
  diagnappframetimestamp = Widget_Text(WID_BASE_77,  $
      UNAME='diagnappframetimestamp' ,XOFFSET=135 ,YOFFSET=3  $
      ,XSIZE=20 ,YSIZE=1)

  
  diagnappframetimestamp_ = Widget_Button(WID_BASE_77,  $
      UNAME='diagnappframetimestamp_' ,XOFFSET=262 ,YOFFSET=3  $
      ,/ALIGN_CENTER ,VALUE='ok.bmp' ,/BITMAP)

  
  WID_BASE_78 = Widget_Base(WID_BASE_69, UNAME='WID_BASE_78'  $
      ,XOFFSET=18 ,YOFFSET=51 ,TITLE='IDL' ,SPACE=3 ,XPAD=3 ,YPAD=3  $
      ,ROW=1)

  
  WID_LABEL_153 = Widget_Label(WID_BASE_78, UNAME='WID_LABEL_153'  $
      ,XOFFSET=3 ,YOFFSET=3 ,/ALIGN_LEFT  $
      ,VALUE='DiagnAppFrameCounter')

  
  diagnappframecounter = Widget_Text(WID_BASE_78,  $
      UNAME='diagnappframecounter' ,XOFFSET=121 ,YOFFSET=3 ,XSIZE=20  $
      ,YSIZE=1)

  
  diagnappframecounter_ = Widget_Button(WID_BASE_78,  $
      UNAME='diagnappframecounter_' ,XOFFSET=248 ,YOFFSET=3  $
      ,/ALIGN_CENTER ,VALUE='ok.bmp' ,/BITMAP)

  
  WID_BASE_107 = Widget_Base(WID_BASE_69, UNAME='WID_BASE_107'  $
      ,XOFFSET=6 ,YOFFSET=81 ,TITLE='IDL' ,SPACE=3 ,XPAD=3 ,YPAD=3  $
      ,ROW=1)

  
  WID_LABEL_154 = Widget_Label(WID_BASE_107, UNAME='WID_LABEL_154'  $
      ,XOFFSET=3 ,YOFFSET=3 ,/ALIGN_LEFT  $
      ,VALUE='DiagnAppFastVarsCounter')

  
  diagnappfastvarscounter = Widget_Text(WID_BASE_107,  $
      UNAME='diagnappfastvarscounter' ,XOFFSET=133 ,YOFFSET=3  $
      ,XSIZE=20 ,YSIZE=1)

  
  diagnappfastvarscounter_ = Widget_Button(WID_BASE_107,  $
      UNAME='diagnappfastvarscounter_' ,XOFFSET=260 ,YOFFSET=3  $
      ,/ALIGN_CENTER ,VALUE='ok.bmp' ,/BITMAP)

  
  WID_BASE_108 = Widget_Base(WID_BASE_69, UNAME='WID_BASE_108'  $
      ,XOFFSET=3 ,YOFFSET=111 ,TITLE='IDL' ,SPACE=3 ,XPAD=3 ,YPAD=3  $
      ,ROW=1)

  
  WID_LABEL_155 = Widget_Label(WID_BASE_108, UNAME='WID_LABEL_155'  $
      ,XOFFSET=3 ,YOFFSET=3 ,/ALIGN_LEFT  $
      ,VALUE='DiagnAppSlowVarsCounter')

  
  diagnappslowvarscounter = Widget_Text(WID_BASE_108,  $
      UNAME='diagnappslowvarscounter' ,XOFFSET=136 ,YOFFSET=3  $
      ,XSIZE=20 ,YSIZE=1)

  
  diagnappslowvarscounter_ = Widget_Button(WID_BASE_108,  $
      UNAME='diagnappslowvarscounter_' ,XOFFSET=263 ,YOFFSET=3  $
      ,/ALIGN_CENTER ,VALUE='ok.bmp' ,/BITMAP)

  
  WID_BASE_109 = Widget_Base(WID_BASE_69, UNAME='WID_BASE_109'  $
      ,XOFFSET=32 ,YOFFSET=141 ,TITLE='IDL' ,SPACE=3 ,XPAD=3 ,YPAD=3  $
      ,ROW=1)

  
  WID_LABEL_156 = Widget_Label(WID_BASE_109, UNAME='WID_LABEL_156'  $
      ,XOFFSET=3 ,YOFFSET=3 ,/ALIGN_LEFT ,VALUE='DiagnAppFrameRate')

  
  diagnappframerate = Widget_Text(WID_BASE_109,  $
      UNAME='diagnappframerate' ,XOFFSET=107 ,YOFFSET=3 ,XSIZE=20  $
      ,YSIZE=1)

  
  diagnappframerate_ = Widget_Button(WID_BASE_109,  $
      UNAME='diagnappframerate_' ,XOFFSET=234 ,YOFFSET=3  $
      ,/ALIGN_CENTER ,VALUE='ok.bmp' ,/BITMAP)

  
  WID_BASE_110 = Widget_Base(WID_BASE_69, UNAME='WID_BASE_110'  $
      ,XOFFSET=20 ,YOFFSET=171 ,TITLE='IDL' ,SPACE=3 ,XPAD=3 ,YPAD=3  $
      ,ROW=1)

  
  WID_LABEL_157 = Widget_Label(WID_BASE_110, UNAME='WID_LABEL_157'  $
      ,XOFFSET=3 ,YOFFSET=3 ,/ALIGN_LEFT  $
      ,VALUE='DiagnAppFastVarsRate')

  
  diagnappfastvarsrate = Widget_Text(WID_BASE_110,  $
      UNAME='diagnappfastvarsrate' ,XOFFSET=119 ,YOFFSET=3 ,XSIZE=20  $
      ,YSIZE=1)

  
  diagnappfastvarsrate_ = Widget_Button(WID_BASE_110,  $
      UNAME='diagnappfastvarsrate_' ,XOFFSET=246 ,YOFFSET=3  $
      ,/ALIGN_CENTER ,VALUE='ok.bmp' ,/BITMAP)

  
  WID_BASE_111 = Widget_Base(WID_BASE_69, UNAME='WID_BASE_111'  $
      ,XOFFSET=17 ,YOFFSET=201 ,TITLE='IDL' ,SPACE=3 ,XPAD=3 ,YPAD=3  $
      ,ROW=1)

  
  WID_LABEL_158 = Widget_Label(WID_BASE_111, UNAME='WID_LABEL_158'  $
      ,XOFFSET=3 ,YOFFSET=3 ,/ALIGN_LEFT  $
      ,VALUE='DiagnAppSlowVarsRate')

  
  diagnappslowvarsrate = Widget_Text(WID_BASE_111,  $
      UNAME='diagnappslowvarsrate' ,XOFFSET=122 ,YOFFSET=3 ,XSIZE=20  $
      ,YSIZE=1)

  
  diagnappslowvarsrate_ = Widget_Button(WID_BASE_111,  $
      UNAME='diagnappslowvarsrate_' ,XOFFSET=249 ,YOFFSET=3  $
      ,/ALIGN_CENTER ,VALUE='ok.bmp' ,/BITMAP)

  
  WID_BASE_85 = Widget_Base(GENERAL_SUMMARY_BASE, UNAME='WID_BASE_85'  $
      ,FRAME=1 ,XOFFSET=386 ,YOFFSET=3 ,/BASE_ALIGN_RIGHT  $
      ,TITLE='IDL' ,SPACE=3 ,XPAD=3 ,YPAD=3 ,COLUMN=1)

  
  WID_LABEL_171 = Widget_Label(WID_BASE_85, UNAME='WID_LABEL_171'  $
      ,XOFFSET=79 ,YOFFSET=3 ,/ALIGN_RIGHT ,VALUE='External'+ $
      ' temperatures and cooling data')

  
  WID_BASE_87 = Widget_Base(WID_BASE_85, UNAME='WID_BASE_87'  $
      ,XOFFSET=7 ,YOFFSET=21 ,TITLE='IDL' ,SPACE=3 ,XPAD=3 ,YPAD=3  $
      ,ROW=1)

  
  WID_LABEL_172 = Widget_Label(WID_BASE_87, UNAME='WID_LABEL_172'  $
      ,XOFFSET=3 ,YOFFSET=3 ,/ALIGN_LEFT  $
      ,VALUE='ExternalTemperature')

  
  externaltemperature = Widget_Text(WID_BASE_87,  $
      UNAME='externaltemperature' ,XOFFSET=106 ,YOFFSET=3 ,XSIZE=20  $
      ,YSIZE=1)

  
  externaltemperature_ = Widget_Button(WID_BASE_87,  $
      UNAME='externaltemperature_' ,XOFFSET=233 ,YOFFSET=3  $
      ,/ALIGN_CENTER ,VALUE='ok.bmp' ,/BITMAP)

  
  WID_BASE_88 = Widget_Base(WID_BASE_85, UNAME='WID_BASE_88'  $
      ,XOFFSET=54 ,YOFFSET=51 ,TITLE='IDL' ,SPACE=3 ,XPAD=3 ,YPAD=3  $
      ,ROW=1)

  
  WID_LABEL_173 = Widget_Label(WID_BASE_88, UNAME='WID_LABEL_173'  $
      ,XOFFSET=3 ,YOFFSET=3 ,/ALIGN_LEFT ,VALUE='FluxRateIn')

  
  fluxratein = Widget_Text(WID_BASE_88, UNAME='fluxratein'  $
      ,XOFFSET=59 ,YOFFSET=3 ,XSIZE=20 ,YSIZE=1)

  
  fluxratein_ = Widget_Button(WID_BASE_88, UNAME='fluxratein_'  $
      ,XOFFSET=186 ,YOFFSET=3 ,/ALIGN_CENTER ,VALUE='ok.bmp'  $
      ,/BITMAP)

  
  WID_BASE_114 = Widget_Base(WID_BASE_85, UNAME='WID_BASE_114'  $
      ,XOFFSET=33 ,YOFFSET=81 ,TITLE='IDL' ,SPACE=3 ,XPAD=3 ,YPAD=3  $
      ,ROW=1)

  
  WID_LABEL_174 = Widget_Label(WID_BASE_114, UNAME='WID_LABEL_174'  $
      ,XOFFSET=3 ,YOFFSET=3 ,/ALIGN_LEFT ,VALUE='WaterMainInlet')

  
  watermaininlet = Widget_Text(WID_BASE_114, UNAME='watermaininlet'  $
      ,XOFFSET=80 ,YOFFSET=3 ,XSIZE=20 ,YSIZE=1)

  
  watermaininlet_ = Widget_Button(WID_BASE_114,  $
      UNAME='watermaininlet_' ,XOFFSET=207 ,YOFFSET=3 ,/ALIGN_CENTER  $
      ,VALUE='ok.bmp' ,/BITMAP)

  
  WID_BASE_115 = Widget_Base(WID_BASE_85, UNAME='WID_BASE_115'  $
      ,XOFFSET=25 ,YOFFSET=111 ,TITLE='IDL' ,SPACE=3 ,XPAD=3 ,YPAD=3  $
      ,ROW=1)

  
  WID_LABEL_175 = Widget_Label(WID_BASE_115, UNAME='WID_LABEL_175'  $
      ,XOFFSET=3 ,YOFFSET=3 ,/ALIGN_LEFT ,VALUE='WaterMainOutlet')

  
  watermainoutlet = Widget_Text(WID_BASE_115, UNAME='watermainoutlet'  $
      ,XOFFSET=88 ,YOFFSET=3 ,XSIZE=20 ,YSIZE=1)

  
  watermainoutlet_ = Widget_Button(WID_BASE_115,  $
      UNAME='watermainoutlet_' ,XOFFSET=215 ,YOFFSET=3 ,/ALIGN_CENTER  $
      ,VALUE='ok.bmp' ,/BITMAP)

  
  WID_BASE_116 = Widget_Base(WID_BASE_85, UNAME='WID_BASE_116'  $
      ,XOFFSET=11 ,YOFFSET=141 ,TITLE='IDL' ,SPACE=3 ,XPAD=3 ,YPAD=3  $
      ,ROW=1)

  
  WID_LABEL_176 = Widget_Label(WID_BASE_116, UNAME='WID_LABEL_176'  $
      ,XOFFSET=3 ,YOFFSET=3 ,/ALIGN_LEFT  $
      ,VALUE='WaterColdPlateInlet')

  
  watercoldplateinlet = Widget_Text(WID_BASE_116,  $
      UNAME='watercoldplateinlet' ,XOFFSET=102 ,YOFFSET=3 ,XSIZE=20  $
      ,YSIZE=1)

  
  watercoldplateinlet_ = Widget_Button(WID_BASE_116,  $
      UNAME='watercoldplateinlet_' ,XOFFSET=229 ,YOFFSET=3  $
      ,/ALIGN_CENTER ,VALUE='ok.bmp' ,/BITMAP)

  
  WID_BASE_117 = Widget_Base(WID_BASE_85, UNAME='WID_BASE_117'  $
      ,XOFFSET=3 ,YOFFSET=171 ,TITLE='IDL' ,SPACE=3 ,XPAD=3 ,YPAD=3  $
      ,ROW=1)

  
  WID_LABEL_177 = Widget_Label(WID_BASE_117, UNAME='WID_LABEL_177'  $
      ,XOFFSET=3 ,YOFFSET=3 ,/ALIGN_LEFT  $
      ,VALUE='WaterColdPlateOutlet')

  
  watercoldplateoutlet = Widget_Text(WID_BASE_117,  $
      UNAME='watercoldplateoutlet' ,XOFFSET=110 ,YOFFSET=3 ,XSIZE=20  $
      ,YSIZE=1)

  
  watercoldplateoutlet_ = Widget_Button(WID_BASE_117,  $
      UNAME='watercoldplateoutlet_' ,XOFFSET=237 ,YOFFSET=3  $
      ,/ALIGN_CENTER ,VALUE='ok.bmp' ,/BITMAP)

  
  WID_BASE_118 = Widget_Base(WID_BASE_85, UNAME='WID_BASE_118'  $
      ,XOFFSET=27 ,YOFFSET=201 ,TITLE='IDL' ,SPACE=3 ,XPAD=3 ,YPAD=3  $
      ,ROW=1)

  
  WID_LABEL_178 = Widget_Label(WID_BASE_118, UNAME='WID_LABEL_178'  $
      ,XOFFSET=3 ,YOFFSET=3 ,/ALIGN_LEFT ,VALUE='ExternalHumidity')

  
  externalhumidity = Widget_Text(WID_BASE_118,  $
      UNAME='externalhumidity' ,XOFFSET=86 ,YOFFSET=3 ,XSIZE=20  $
      ,YSIZE=1)

  
  externalhumidity_ = Widget_Button(WID_BASE_118,  $
      UNAME='externalhumidity_' ,XOFFSET=213 ,YOFFSET=3  $
      ,/ALIGN_CENTER ,VALUE='ok.bmp' ,/BITMAP)

  
  WID_BASE_119 = Widget_Base(WID_BASE_85, UNAME='WID_BASE_119'  $
      ,XOFFSET=59 ,YOFFSET=231 ,TITLE='IDL' ,SPACE=3 ,XPAD=3 ,YPAD=3  $
      ,ROW=1)

  
  WID_LABEL_179 = Widget_Label(WID_BASE_119, UNAME='WID_LABEL_179'  $
      ,XOFFSET=3 ,YOFFSET=3 ,/ALIGN_LEFT ,VALUE='DewPoint')

  
  dewpoint = Widget_Text(WID_BASE_119, UNAME='dewpoint' ,XOFFSET=54  $
      ,YOFFSET=3 ,XSIZE=20 ,YSIZE=1)

  
  dewpoint_ = Widget_Button(WID_BASE_119, UNAME='dewpoint_'  $
      ,XOFFSET=181 ,YOFFSET=3 ,/ALIGN_CENTER ,VALUE='ok.bmp'  $
      ,/BITMAP)

  
  WID_BASE_120 = Widget_Base(WID_BASE_85, UNAME='WID_BASE_120'  $
      ,XOFFSET=28 ,YOFFSET=261 ,TITLE='IDL' ,SPACE=3 ,XPAD=3 ,YPAD=3  $
      ,ROW=1)

  
  WID_LABEL_180 = Widget_Label(WID_BASE_120, UNAME='WID_LABEL_180'  $
      ,XOFFSET=3 ,YOFFSET=3 ,/ALIGN_LEFT ,VALUE='CheckDewPoint')

  
  checkdewpoint = Widget_Text(WID_BASE_120, UNAME='checkdewpoint'  $
      ,XOFFSET=85 ,YOFFSET=3 ,XSIZE=20 ,YSIZE=1)

  
  checkdewpoint_ = Widget_Button(WID_BASE_120, UNAME='checkdewpoint_'  $
      ,XOFFSET=212 ,YOFFSET=3 ,/ALIGN_CENTER ,VALUE='ok.bmp'  $
      ,/BITMAP)

  
  CRATE_SUMMARY_BASE = Widget_Base(WID_TAB_0,  $
      UNAME='CRATE_SUMMARY_BASE' ,SCR_XSIZE=887 ,SCR_YSIZE=641  $
      ,TITLE='Crate Summary' ,SPACE=3 ,XPAD=3 ,YPAD=3)

  
  CRATE_BCU1_BASE = Widget_Base(WID_TAB_0, UNAME='CRATE_BCU1_BASE'  $
      ,FRAME=1 ,SCR_XSIZE=887 ,SCR_YSIZE=641 ,TITLE='Crate BCU 1'  $
      ,SPACE=3 ,XPAD=3 ,YPAD=3 ,COLUMN=1)

  
  WID_BASE_4 = Widget_Base(CRATE_BCU1_BASE, UNAME='WID_BASE_4'  $
      ,XOFFSET=3 ,YOFFSET=3 ,TITLE='IDL' ,SPACE=3 ,XPAD=3 ,YPAD=3  $
      ,ROW=1)

  
  WID_BASE_38 = Widget_Base(WID_BASE_4, UNAME='WID_BASE_38' ,FRAME=1  $
      ,XOFFSET=3 ,YOFFSET=3 ,/BASE_ALIGN_RIGHT ,TITLE='IDL' ,SPACE=3  $
      ,XPAD=3 ,YPAD=3 ,COLUMN=1)

  
  WID_LABEL_68 = Widget_Label(WID_BASE_38, UNAME='WID_LABEL_68'  $
      ,XOFFSET=197 ,YOFFSET=3 ,/ALIGN_RIGHT ,VALUE='NIOS Fixed Area')

  
  WID_BASE_42 = Widget_Base(WID_BASE_38, UNAME='WID_BASE_42'  $
      ,XOFFSET=82 ,YOFFSET=21 ,TITLE='IDL' ,SPACE=3 ,XPAD=3 ,YPAD=3  $
      ,ROW=1)

  
  WID_LABEL_72 = Widget_Label(WID_BASE_42, UNAME='WID_LABEL_72'  $
      ,XOFFSET=3 ,YOFFSET=3 ,/ALIGN_LEFT ,VALUE='CrateID')

  
  bcucrateid = Widget_Text(WID_BASE_42, UNAME='bcucrateid'  $
      ,XOFFSET=44 ,YOFFSET=3 ,XSIZE=20 ,YSIZE=1)

  
  bcucrateid_ = Widget_Button(WID_BASE_42, UNAME='bcucrateid_'  $
      ,XOFFSET=171 ,YOFFSET=3 ,/ALIGN_CENTER ,VALUE='ok.bmp'  $
      ,/BITMAP)

  
  WID_BASE_43 = Widget_Base(WID_BASE_38, UNAME='WID_BASE_43'  $
      ,XOFFSET=77 ,YOFFSET=51 ,TITLE='IDL' ,SPACE=3 ,XPAD=3 ,YPAD=3  $
      ,ROW=1)

  
  WID_LABEL_73 = Widget_Label(WID_BASE_43, UNAME='WID_LABEL_73'  $
      ,XOFFSET=3 ,YOFFSET=3 ,/ALIGN_LEFT ,VALUE='WhoAmI')

  
  bcuwhoami = Widget_Text(WID_BASE_43, UNAME='bcuwhoami' ,XOFFSET=49  $
      ,YOFFSET=3 ,XSIZE=20 ,YSIZE=1)

  
  bcuwhoami_ = Widget_Button(WID_BASE_43, UNAME='bcuwhoami_'  $
      ,XOFFSET=176 ,YOFFSET=3 ,/ALIGN_CENTER ,VALUE='ok.bmp'  $
      ,/BITMAP)

  
  WID_BASE_45 = Widget_Base(WID_BASE_38, UNAME='WID_BASE_45'  $
      ,XOFFSET=37 ,YOFFSET=81 ,TITLE='IDL' ,SPACE=3 ,XPAD=3 ,YPAD=3  $
      ,ROW=1)

  
  WID_LABEL_75 = Widget_Label(WID_BASE_45, UNAME='WID_LABEL_75'  $
      ,XOFFSET=3 ,YOFFSET=3 ,/ALIGN_LEFT ,VALUE='SoftwareRelease')

  
  bcusoftwarerelease = Widget_Text(WID_BASE_45,  $
      UNAME='bcusoftwarerelease' ,XOFFSET=89 ,YOFFSET=3 ,XSIZE=20  $
      ,YSIZE=1)

  
  bcusoftwarerelease_ = Widget_Button(WID_BASE_45,  $
      UNAME='bcusoftwarerelease_' ,XOFFSET=216 ,YOFFSET=3  $
      ,/ALIGN_CENTER ,VALUE='ok.bmp' ,/BITMAP)

  
  WID_BASE_46 = Widget_Base(WID_BASE_38, UNAME='WID_BASE_46'  $
      ,XOFFSET=53 ,YOFFSET=111 ,TITLE='IDL' ,SPACE=3 ,XPAD=3 ,YPAD=3  $
      ,ROW=1)

  
  WID_LABEL_76 = Widget_Label(WID_BASE_46, UNAME='WID_LABEL_76'  $
      ,XOFFSET=3 ,YOFFSET=3 ,/ALIGN_LEFT ,VALUE='LogicRelease')

  
  bculogicrelease = Widget_Text(WID_BASE_46, UNAME='bculogicrelease'  $
      ,XOFFSET=73 ,YOFFSET=3 ,XSIZE=20 ,YSIZE=1)

  
  bculogicrelease_ = Widget_Button(WID_BASE_46,  $
      UNAME='bculogicrelease_' ,XOFFSET=200 ,YOFFSET=3 ,/ALIGN_CENTER  $
      ,VALUE='ok.bmp' ,/BITMAP)

  
  WID_BASE_47 = Widget_Base(WID_BASE_38, UNAME='WID_BASE_47'  $
      ,XOFFSET=70 ,YOFFSET=141 ,TITLE='IDL' ,SPACE=3 ,XPAD=3 ,YPAD=3  $
      ,ROW=1)

  
  WID_LABEL_77 = Widget_Label(WID_BASE_47, UNAME='WID_LABEL_77'  $
      ,XOFFSET=3 ,YOFFSET=3 ,/ALIGN_LEFT ,VALUE='IPAddress')

  
  bcuipaddress = Widget_Text(WID_BASE_47, UNAME='bcuipaddress'  $
      ,XOFFSET=56 ,YOFFSET=3 ,XSIZE=20 ,YSIZE=1)

  
  bcuipaddress_ = Widget_Button(WID_BASE_47, UNAME='bcuipaddress_'  $
      ,XOFFSET=183 ,YOFFSET=3 ,/ALIGN_CENTER ,VALUE='ok.bmp'  $
      ,/BITMAP)

  
  WID_BASE_48 = Widget_Base(WID_BASE_38, UNAME='WID_BASE_48'  $
      ,XOFFSET=47 ,YOFFSET=171 ,TITLE='IDL' ,SPACE=3 ,XPAD=3 ,YPAD=3  $
      ,ROW=1)

  
  WID_LABEL_78 = Widget_Label(WID_BASE_48, UNAME='WID_LABEL_78'  $
      ,XOFFSET=3 ,YOFFSET=3 ,/ALIGN_LEFT ,VALUE='FramesCounter')

  
  bcuframescounter = Widget_Text(WID_BASE_48,  $
      UNAME='bcuframescounter' ,XOFFSET=79 ,YOFFSET=3 ,XSIZE=20  $
      ,YSIZE=1)

  
  bcuframescounter_ = Widget_Button(WID_BASE_48,  $
      UNAME='bcuframescounter_' ,XOFFSET=206 ,YOFFSET=3  $
      ,/ALIGN_CENTER ,VALUE='ok.bmp' ,/BITMAP)

  
  WID_BASE_49 = Widget_Base(WID_BASE_38, UNAME='WID_BASE_49'  $
      ,XOFFSET=55 ,YOFFSET=201 ,TITLE='IDL' ,SPACE=3 ,XPAD=3 ,YPAD=3  $
      ,ROW=1)

  
  WID_LABEL_79 = Widget_Label(WID_BASE_49, UNAME='WID_LABEL_79'  $
      ,XOFFSET=3 ,YOFFSET=3 ,/ALIGN_LEFT ,VALUE='SerialNumber')

  
  bcuserialnumber = Widget_Text(WID_BASE_49, UNAME='bcuserialnumber'  $
      ,XOFFSET=71 ,YOFFSET=3 ,XSIZE=20 ,YSIZE=1)

  
  bcuserialnumber_ = Widget_Button(WID_BASE_49,  $
      UNAME='bcuserialnumber_' ,XOFFSET=198 ,YOFFSET=3 ,/ALIGN_CENTER  $
      ,VALUE='ok.bmp' ,/BITMAP)

  
  WID_BASE_50 = Widget_Base(WID_BASE_38, UNAME='WID_BASE_50'  $
      ,XOFFSET=11 ,YOFFSET=231 ,TITLE='IDL' ,SPACE=3 ,XPAD=3 ,YPAD=3  $
      ,ROW=1)

  
  WID_LABEL_80 = Widget_Label(WID_BASE_50, UNAME='WID_LABEL_80'  $
      ,XOFFSET=3 ,YOFFSET=3 ,/ALIGN_LEFT  $
      ,VALUE='PowerBackplaneSerial')

  
  bcupowerbackplaneserial = Widget_Text(WID_BASE_50,  $
      UNAME='bcupowerbackplaneserial' ,XOFFSET=115 ,YOFFSET=3  $
      ,XSIZE=20 ,YSIZE=1)

  
  bcupowerbackplaneserial_ = Widget_Button(WID_BASE_50,  $
      UNAME='bcupowerbackplaneserial_' ,XOFFSET=242 ,YOFFSET=3  $
      ,/ALIGN_CENTER ,VALUE='ok.bmp' ,/BITMAP)

  
  WID_BASE_51 = Widget_Base(WID_BASE_38, UNAME='WID_BASE_51'  $
      ,XOFFSET=3 ,YOFFSET=261 ,TITLE='IDL' ,SPACE=3 ,XPAD=3 ,YPAD=3  $
      ,ROW=1)

  
  WID_LABEL_81 = Widget_Label(WID_BASE_51, UNAME='WID_LABEL_81'  $
      ,XOFFSET=3 ,YOFFSET=3 ,/ALIGN_LEFT  $
      ,VALUE='EnableMasterDiagnostic')

  
  bcuenablemasterdiagnostic = Widget_Text(WID_BASE_51,  $
      UNAME='bcuenablemasterdiagnostic' ,XOFFSET=123 ,YOFFSET=3  $
      ,XSIZE=20 ,YSIZE=1)

  
  bcuenablemasterdiagnostic_ = Widget_Button(WID_BASE_51,  $
      UNAME='bcuenablemasterdiagnostic_' ,XOFFSET=250 ,YOFFSET=3  $
      ,/ALIGN_CENTER ,VALUE='ok.bmp' ,/BITMAP)

  
  WID_BASE_52 = Widget_Base(WID_BASE_38, UNAME='WID_BASE_52'  $
      ,XOFFSET=35 ,YOFFSET=291 ,TITLE='IDL' ,SPACE=3 ,XPAD=3 ,YPAD=3  $
      ,ROW=1)

  
  WID_LABEL_82 = Widget_Label(WID_BASE_52, UNAME='WID_LABEL_82'  $
      ,XOFFSET=3 ,YOFFSET=3 ,/ALIGN_LEFT ,VALUE='DecimationFactor')

  
  bcudecimationfactor = Widget_Text(WID_BASE_52,  $
      UNAME='bcudecimationfactor' ,XOFFSET=91 ,YOFFSET=3 ,XSIZE=20  $
      ,YSIZE=1)

  
  bcudecimationfactor_ = Widget_Button(WID_BASE_52,  $
      UNAME='bcudecimationfactor_' ,XOFFSET=218 ,YOFFSET=3  $
      ,/ALIGN_CENTER ,VALUE='ok.bmp' ,/BITMAP)

  
  WID_BASE_53 = Widget_Base(WID_BASE_38, UNAME='WID_BASE_53'  $
      ,XOFFSET=33 ,YOFFSET=321 ,TITLE='IDL' ,SPACE=3 ,XPAD=3 ,YPAD=3  $
      ,ROW=1)

  
  WID_LABEL_83 = Widget_Label(WID_BASE_53, UNAME='WID_LABEL_83'  $
      ,XOFFSET=3 ,YOFFSET=3 ,/ALIGN_LEFT ,VALUE='RemoteIPAddress')

  
  bcuremoteipaddress = Widget_Text(WID_BASE_53,  $
      UNAME='bcuremoteipaddress' ,XOFFSET=93 ,YOFFSET=3 ,XSIZE=20  $
      ,YSIZE=1)

  
  bcuremoteipaddress_ = Widget_Button(WID_BASE_53,  $
      UNAME='bcuremoteipaddress_' ,XOFFSET=220 ,YOFFSET=3  $
      ,/ALIGN_CENTER ,VALUE='ok.bmp' ,/BITMAP)

  
  WID_BASE_7 = Widget_Base(WID_BASE_4, UNAME='WID_BASE_7'  $
      ,XOFFSET=287 ,YOFFSET=3 ,/BASE_ALIGN_RIGHT ,TITLE='IDL'  $
      ,SPACE=3 ,XPAD=3 ,YPAD=3 ,COLUMN=1)

  
  WID_BASE_10 = Widget_Base(WID_BASE_7, UNAME='WID_BASE_10' ,FRAME=1  $
      ,XOFFSET=29 ,YOFFSET=3 ,/BASE_ALIGN_RIGHT ,TITLE='IDL' ,SPACE=3  $
      ,XPAD=3 ,YPAD=3 ,COLUMN=1)

  
  WID_LABEL_8 = Widget_Label(WID_BASE_10, UNAME='WID_LABEL_8'  $
      ,XOFFSET=153 ,YOFFSET=3 ,/ALIGN_RIGHT ,VALUE='Temperatures')

  
  WID_BASE_82 = Widget_Base(WID_BASE_10, UNAME='WID_BASE_82'  $
      ,XOFFSET=4 ,YOFFSET=21 ,TITLE='IDL' ,SPACE=3 ,XPAD=3 ,YPAD=3  $
      ,ROW=1)

  
  WID_LABEL_206 = Widget_Label(WID_BASE_82, UNAME='WID_LABEL_206'  $
      ,XOFFSET=3 ,YOFFSET=3 ,/ALIGN_LEFT ,VALUE='StratixTemp')

  
  bcustratixtemp = Widget_Text(WID_BASE_82, UNAME='bcustratixtemp'  $
      ,XOFFSET=64 ,YOFFSET=3 ,XSIZE=20 ,YSIZE=1)

  
  bcustratixtemp_ = Widget_Button(WID_BASE_82,  $
      UNAME='bcustratixtemp_' ,XOFFSET=191 ,YOFFSET=3 ,/ALIGN_CENTER  $
      ,VALUE='ok.bmp' ,/BITMAP)

  
  WID_BASE_86 = Widget_Base(WID_BASE_10, UNAME='WID_BASE_86'  $
      ,XOFFSET=3 ,YOFFSET=51 ,TITLE='IDL' ,SPACE=3 ,XPAD=3 ,YPAD=3  $
      ,ROW=1)

  
  WID_LABEL_207 = Widget_Label(WID_BASE_86, UNAME='WID_LABEL_207'  $
      ,XOFFSET=3 ,YOFFSET=3 ,/ALIGN_LEFT ,VALUE='PowerTemp')

  
  bcupowertemp = Widget_Text(WID_BASE_86, UNAME='bcupowertemp'  $
      ,XOFFSET=65 ,YOFFSET=3 ,XSIZE=20 ,YSIZE=1)

  
  bcupowertemp_ = Widget_Button(WID_BASE_86, UNAME='bcupowertemp_'  $
      ,XOFFSET=192 ,YOFFSET=3 ,/ALIGN_CENTER ,VALUE='ok.bmp'  $
      ,/BITMAP)

  
  WID_BASE_11 = Widget_Base(WID_BASE_7, UNAME='WID_BASE_11' ,FRAME=1  $
      ,XOFFSET=3 ,YOFFSET=87 ,/BASE_ALIGN_RIGHT ,TITLE='IDL' ,SPACE=3  $
      ,XPAD=3 ,YPAD=3 ,COLUMN=1)

  
  WID_LABEL_9 = Widget_Label(WID_BASE_11, UNAME='WID_LABEL_9'  $
      ,XOFFSET=185 ,YOFFSET=3 ,/ALIGN_RIGHT ,VALUE='Environment')

  
  WID_BASE_96 = Widget_Base(WID_BASE_11, UNAME='WID_BASE_96'  $
      ,XOFFSET=10 ,YOFFSET=21 ,TITLE='IDL' ,SPACE=3 ,XPAD=3 ,YPAD=3  $
      ,ROW=1)

  
  WID_LABEL_208 = Widget_Label(WID_BASE_96, UNAME='WID_LABEL_208'  $
      ,XOFFSET=3 ,YOFFSET=3 ,/ALIGN_LEFT ,VALUE='Cooler in0 temp.')

  
  bcucoolerin0temp = Widget_Text(WID_BASE_96,  $
      UNAME='bcucoolerin0temp' ,XOFFSET=84 ,YOFFSET=3 ,XSIZE=20  $
      ,YSIZE=1)

  
  bcucoolerin0temp_ = Widget_Button(WID_BASE_96,  $
      UNAME='bcucoolerin0temp_' ,XOFFSET=211 ,YOFFSET=3  $
      ,/ALIGN_CENTER ,VALUE='ok.bmp' ,/BITMAP)

  
  WID_BASE_124 = Widget_Base(WID_BASE_11, UNAME='WID_BASE_124'  $
      ,XOFFSET=10 ,YOFFSET=51 ,TITLE='IDL' ,SPACE=3 ,XPAD=3 ,YPAD=3  $
      ,ROW=1)

  
  WID_LABEL_209 = Widget_Label(WID_BASE_124, UNAME='WID_LABEL_209'  $
      ,XOFFSET=3 ,YOFFSET=3 ,/ALIGN_LEFT ,VALUE='Cooler in1 temp.')

  
  bcucoolerin1temp = Widget_Text(WID_BASE_124,  $
      UNAME='bcucoolerin1temp' ,XOFFSET=84 ,YOFFSET=3 ,XSIZE=20  $
      ,YSIZE=1)

  
  bcucoolerin1temp_ = Widget_Button(WID_BASE_124,  $
      UNAME='bcucoolerin1temp_' ,XOFFSET=211 ,YOFFSET=3  $
      ,/ALIGN_CENTER ,VALUE='ok.bmp' ,/BITMAP)

  
  WID_BASE_125 = Widget_Base(WID_BASE_11, UNAME='WID_BASE_125'  $
      ,XOFFSET=3 ,YOFFSET=81 ,TITLE='IDL' ,SPACE=3 ,XPAD=3 ,YPAD=3  $
      ,ROW=1)

  
  WID_LABEL_210 = Widget_Label(WID_BASE_125, UNAME='WID_LABEL_210'  $
      ,XOFFSET=3 ,YOFFSET=3 ,/ALIGN_LEFT ,VALUE='Cooler out0 temp.')

  
  bcucoolerout0temp = Widget_Text(WID_BASE_125,  $
      UNAME='bcucoolerout0temp' ,XOFFSET=91 ,YOFFSET=3 ,XSIZE=20  $
      ,YSIZE=1)

  
  bcucoolerout0temp_ = Widget_Button(WID_BASE_125,  $
      UNAME='bcucoolerout0temp_' ,XOFFSET=218 ,YOFFSET=3  $
      ,/ALIGN_CENTER ,VALUE='ok.bmp' ,/BITMAP)

  
  WID_BASE_126 = Widget_Base(WID_BASE_11, UNAME='WID_BASE_126'  $
      ,XOFFSET=6 ,YOFFSET=111 ,TITLE='IDL' ,SPACE=3 ,XPAD=3 ,YPAD=3  $
      ,ROW=1)

  
  WID_LABEL_211 = Widget_Label(WID_BASE_126, UNAME='WID_LABEL_211'  $
      ,XOFFSET=3 ,YOFFSET=3 ,/ALIGN_LEFT ,VALUE='Cooler out1 temp')

  
  bcucoolerout1temp = Widget_Text(WID_BASE_126,  $
      UNAME='bcucoolerout1temp' ,XOFFSET=88 ,YOFFSET=3 ,XSIZE=20  $
      ,YSIZE=1)

  
  bcucoolerout1temp_ = Widget_Button(WID_BASE_126,  $
      UNAME='bcucoolerout1temp_' ,XOFFSET=215 ,YOFFSET=3  $
      ,/ALIGN_CENTER ,VALUE='ok.bmp' ,/BITMAP)

  
  WID_BASE_127 = Widget_Base(WID_BASE_11, UNAME='WID_BASE_127'  $
      ,XOFFSET=13 ,YOFFSET=141 ,TITLE='IDL' ,SPACE=3 ,XPAD=3 ,YPAD=3  $
      ,ROW=1)

  
  WID_LABEL_212 = Widget_Label(WID_BASE_127, UNAME='WID_LABEL_212'  $
      ,XOFFSET=3 ,YOFFSET=3 ,/ALIGN_LEFT ,VALUE='Cooler pressure')

  
  bcucoolerpressure = Widget_Text(WID_BASE_127,  $
      UNAME='bcucoolerpressure' ,XOFFSET=81 ,YOFFSET=3 ,XSIZE=20  $
      ,YSIZE=1)

  
  bcucoolerpressure_ = Widget_Button(WID_BASE_127,  $
      UNAME='bcucoolerpressure_' ,XOFFSET=208 ,YOFFSET=3  $
      ,/ALIGN_CENTER ,VALUE='ok.bmp' ,/BITMAP)

  
  WID_BASE_128 = Widget_Base(WID_BASE_11, UNAME='WID_BASE_128'  $
      ,XOFFSET=46 ,YOFFSET=171 ,TITLE='IDL' ,SPACE=3 ,XPAD=3 ,YPAD=3  $
      ,ROW=1)

  
  WID_LABEL_213 = Widget_Label(WID_BASE_128, UNAME='WID_LABEL_213'  $
      ,XOFFSET=3 ,YOFFSET=3 ,/ALIGN_LEFT ,VALUE='Humidity')

  
  bcuhumidity = Widget_Text(WID_BASE_128, UNAME='bcuhumidity'  $
      ,XOFFSET=48 ,YOFFSET=3 ,XSIZE=20 ,YSIZE=1)

  
  bcuhumidity_ = Widget_Button(WID_BASE_128, UNAME='bcuhumidity_'  $
      ,XOFFSET=175 ,YOFFSET=3 ,/ALIGN_CENTER ,VALUE='ok.bmp'  $
      ,/BITMAP)

  
  WID_BASE_12 = Widget_Base(WID_BASE_4, UNAME='WID_BASE_12' ,FRAME=1  $
      ,XOFFSET=545 ,YOFFSET=3 ,/BASE_ALIGN_RIGHT ,TITLE='IDL'  $
      ,SPACE=3 ,XPAD=3 ,YPAD=3 ,COLUMN=1)

  
  WID_LABEL_10 = Widget_Label(WID_BASE_12, UNAME='WID_LABEL_10'  $
      ,XOFFSET=102 ,YOFFSET=3 ,/ALIGN_RIGHT ,VALUE='Voltage and'+ $
      ' current levels')

  
  WID_BASE_97 = Widget_Base(WID_BASE_12, UNAME='WID_BASE_97'  $
      ,XOFFSET=4 ,YOFFSET=21 ,TITLE='IDL' ,SPACE=3 ,XPAD=3 ,YPAD=3  $
      ,ROW=1)

  
  WID_LABEL_214 = Widget_Label(WID_BASE_97, UNAME='WID_LABEL_214'  $
      ,XOFFSET=3 ,YOFFSET=3 ,/ALIGN_LEFT ,VALUE='VoltageVCCL')

  
  bcuvoltagevccl = Widget_Text(WID_BASE_97, UNAME='bcuvoltagevccl'  $
      ,XOFFSET=71 ,YOFFSET=3 ,XSIZE=20 ,YSIZE=1)

  
  bcuvoltagevccl_ = Widget_Button(WID_BASE_97,  $
      UNAME='bcuvoltagevccl_' ,XOFFSET=198 ,YOFFSET=3 ,/ALIGN_CENTER  $
      ,VALUE='ok.bmp' ,/BITMAP)

  
  WID_BASE_129 = Widget_Base(WID_BASE_12, UNAME='WID_BASE_129'  $
      ,XOFFSET=3 ,YOFFSET=51 ,TITLE='IDL' ,SPACE=3 ,XPAD=3 ,YPAD=3  $
      ,ROW=1)

  
  WID_LABEL_215 = Widget_Label(WID_BASE_129, UNAME='WID_LABEL_215'  $
      ,XOFFSET=3 ,YOFFSET=3 ,/ALIGN_LEFT ,VALUE='VoltageVCCA')

  
  bcuvoltagevcca = Widget_Text(WID_BASE_129, UNAME='bcuvoltagevcca'  $
      ,XOFFSET=72 ,YOFFSET=3 ,XSIZE=20 ,YSIZE=1)

  
  bcuvoltagevcca_ = Widget_Button(WID_BASE_129,  $
      UNAME='bcuvoltagevcca_' ,XOFFSET=199 ,YOFFSET=3 ,/ALIGN_CENTER  $
      ,VALUE='ok.bmp' ,/BITMAP)

  
  WID_BASE_130 = Widget_Base(WID_BASE_12, UNAME='WID_BASE_130'  $
      ,XOFFSET=3 ,YOFFSET=81 ,TITLE='IDL' ,SPACE=3 ,XPAD=3 ,YPAD=3  $
      ,ROW=1)

  
  WID_LABEL_216 = Widget_Label(WID_BASE_130, UNAME='WID_LABEL_216'  $
      ,XOFFSET=3 ,YOFFSET=3 ,/ALIGN_LEFT ,VALUE='VoltageVSSA')

  
  bcuvoltagevssa = Widget_Text(WID_BASE_130, UNAME='bcuvoltagevssa'  $
      ,XOFFSET=72 ,YOFFSET=3 ,XSIZE=20 ,YSIZE=1)

  
  bcuvoltagevssa_ = Widget_Button(WID_BASE_130,  $
      UNAME='bcuvoltagevssa_' ,XOFFSET=199 ,YOFFSET=3 ,/ALIGN_CENTER  $
      ,VALUE='ok.bmp' ,/BITMAP)

  
  WID_BASE_131 = Widget_Base(WID_BASE_12, UNAME='WID_BASE_131'  $
      ,XOFFSET=3 ,YOFFSET=111 ,TITLE='IDL' ,SPACE=3 ,XPAD=3 ,YPAD=3  $
      ,ROW=1)

  
  WID_LABEL_217 = Widget_Label(WID_BASE_131, UNAME='WID_LABEL_217'  $
      ,XOFFSET=3 ,YOFFSET=3 ,/ALIGN_LEFT ,VALUE='VoltageVCCP')

  
  bcuvoltagevccp = Widget_Text(WID_BASE_131, UNAME='bcuvoltagevccp'  $
      ,XOFFSET=72 ,YOFFSET=3 ,XSIZE=20 ,YSIZE=1)

  
  bcuvoltagevccp_ = Widget_Button(WID_BASE_131,  $
      UNAME='bcuvoltagevccp_' ,XOFFSET=199 ,YOFFSET=3 ,/ALIGN_CENTER  $
      ,VALUE='ok.bmp' ,/BITMAP)

  
  WID_BASE_132 = Widget_Base(WID_BASE_12, UNAME='WID_BASE_132'  $
      ,XOFFSET=3 ,YOFFSET=141 ,TITLE='IDL' ,SPACE=3 ,XPAD=3 ,YPAD=3  $
      ,ROW=1)

  
  WID_LABEL_218 = Widget_Label(WID_BASE_132, UNAME='WID_LABEL_218'  $
      ,XOFFSET=3 ,YOFFSET=3 ,/ALIGN_LEFT ,VALUE='VoltageVSSP')

  
  bcuvoltagevssp = Widget_Text(WID_BASE_132, UNAME='bcuvoltagevssp'  $
      ,XOFFSET=72 ,YOFFSET=3 ,XSIZE=20 ,YSIZE=1)

  
  bcuvoltagevssp_ = Widget_Button(WID_BASE_132,  $
      UNAME='bcuvoltagevssp_' ,XOFFSET=199 ,YOFFSET=3 ,/ALIGN_CENTER  $
      ,VALUE='ok.bmp' ,/BITMAP)

  
  WID_BASE_133 = Widget_Base(WID_BASE_12, UNAME='WID_BASE_133'  $
      ,XOFFSET=6 ,YOFFSET=171 ,TITLE='IDL' ,SPACE=3 ,XPAD=3 ,YPAD=3  $
      ,ROW=1)

  
  WID_LABEL_219 = Widget_Label(WID_BASE_133, UNAME='WID_LABEL_219'  $
      ,XOFFSET=3 ,YOFFSET=3 ,/ALIGN_LEFT ,VALUE='CurrentVCCL')

  
  bcucurrentvccl = Widget_Text(WID_BASE_133, UNAME='bcucurrentvccl'  $
      ,XOFFSET=69 ,YOFFSET=3 ,XSIZE=20 ,YSIZE=1)

  
  bcucurrentvccl_ = Widget_Button(WID_BASE_133,  $
      UNAME='bcucurrentvccl_' ,XOFFSET=196 ,YOFFSET=3 ,/ALIGN_CENTER  $
      ,VALUE='ok.bmp' ,/BITMAP)

  
  WID_BASE_134 = Widget_Base(WID_BASE_12, UNAME='WID_BASE_134'  $
      ,XOFFSET=5 ,YOFFSET=201 ,TITLE='IDL' ,SPACE=3 ,XPAD=3 ,YPAD=3  $
      ,ROW=1)

  
  WID_LABEL_220 = Widget_Label(WID_BASE_134, UNAME='WID_LABEL_220'  $
      ,XOFFSET=3 ,YOFFSET=3 ,/ALIGN_LEFT ,VALUE='CurrentVCCA')

  
  bcucurrentvcca = Widget_Text(WID_BASE_134, UNAME='bcucurrentvcca'  $
      ,XOFFSET=70 ,YOFFSET=3 ,XSIZE=20 ,YSIZE=1)

  
  bcucurrentvcca_ = Widget_Button(WID_BASE_134,  $
      UNAME='bcucurrentvcca_' ,XOFFSET=197 ,YOFFSET=3 ,/ALIGN_CENTER  $
      ,VALUE='ok.bmp' ,/BITMAP)

  
  WID_BASE_135 = Widget_Base(WID_BASE_12, UNAME='WID_BASE_135'  $
      ,XOFFSET=5 ,YOFFSET=231 ,TITLE='IDL' ,SPACE=3 ,XPAD=3 ,YPAD=3  $
      ,ROW=1)

  
  WID_LABEL_221 = Widget_Label(WID_BASE_135, UNAME='WID_LABEL_221'  $
      ,XOFFSET=3 ,YOFFSET=3 ,/ALIGN_LEFT ,VALUE='CurrentVSSA')

  
  bcucurrentvssa = Widget_Text(WID_BASE_135, UNAME='bcucurrentvssa'  $
      ,XOFFSET=70 ,YOFFSET=3 ,XSIZE=20 ,YSIZE=1)

  
  bcucurrentvssa_ = Widget_Button(WID_BASE_135,  $
      UNAME='bcucurrentvssa_' ,XOFFSET=197 ,YOFFSET=3 ,/ALIGN_CENTER  $
      ,VALUE='ok.bmp' ,/BITMAP)

  
  WID_BASE_136 = Widget_Base(WID_BASE_12, UNAME='WID_BASE_136'  $
      ,XOFFSET=5 ,YOFFSET=261 ,TITLE='IDL' ,SPACE=3 ,XPAD=3 ,YPAD=3  $
      ,ROW=1)

  
  WID_LABEL_222 = Widget_Label(WID_BASE_136, UNAME='WID_LABEL_222'  $
      ,XOFFSET=3 ,YOFFSET=3 ,/ALIGN_LEFT ,VALUE='CurrentVCCP')

  
  bcucurrentvccp = Widget_Text(WID_BASE_136, UNAME='bcucurrentvccp'  $
      ,XOFFSET=70 ,YOFFSET=3 ,XSIZE=20 ,YSIZE=1)

  
  bcucurrentvccp_ = Widget_Button(WID_BASE_136,  $
      UNAME='bcucurrentvccp_' ,XOFFSET=197 ,YOFFSET=3 ,/ALIGN_CENTER  $
      ,VALUE='ok.bmp' ,/BITMAP)

  
  WID_BASE_137 = Widget_Base(WID_BASE_12, UNAME='WID_BASE_137'  $
      ,XOFFSET=5 ,YOFFSET=291 ,TITLE='IDL' ,SPACE=3 ,XPAD=3 ,YPAD=3  $
      ,ROW=1)

  
  WID_LABEL_223 = Widget_Label(WID_BASE_137, UNAME='WID_LABEL_223'  $
      ,XOFFSET=3 ,YOFFSET=3 ,/ALIGN_LEFT ,VALUE='CurrentVSSP')

  
  bcucurrentvssp = Widget_Text(WID_BASE_137, UNAME='bcucurrentvssp'  $
      ,XOFFSET=70 ,YOFFSET=3 ,XSIZE=20 ,YSIZE=1)

  
  bcucurrentvssp_ = Widget_Button(WID_BASE_137,  $
      UNAME='bcucurrentvssp_' ,XOFFSET=197 ,YOFFSET=3 ,/ALIGN_CENTER  $
      ,VALUE='ok.bmp' ,/BITMAP)

  
  WID_BASE_6 = Widget_Base(CRATE_BCU1_BASE, UNAME='WID_BASE_6'  $
      ,XOFFSET=3 ,YOFFSET=363 ,TITLE='IDL' ,SPACE=3 ,XPAD=3 ,YPAD=3  $
      ,ROW=1)

  
  bcu_crate_num1 = Widget_Slider(WID_BASE_6, UNAME='bcu_crate_num1'  $
      ,XOFFSET=3 ,YOFFSET=3 ,TITLE='Crate (0-5)' ,MAXIMUM=5)

  
  CRATE_BCU2_BASE = Widget_Base(WID_TAB_0, UNAME='CRATE_BCU2_BASE'  $
      ,FRAME=1 ,SCR_XSIZE=887 ,SCR_YSIZE=641 ,TITLE='Crate BCU 2'  $
      ,SPACE=3 ,XPAD=3 ,YPAD=3 ,COLUMN=1)

  
  WID_BASE_1 = Widget_Base(CRATE_BCU2_BASE, UNAME='WID_BASE_1'  $
      ,XOFFSET=3 ,YOFFSET=3 ,TITLE='IDL' ,SPACE=3 ,XPAD=3 ,YPAD=3  $
      ,ROW=1)

  
  WID_BASE_89 = Widget_Base(WID_BASE_1, UNAME='WID_BASE_89' ,FRAME=1  $
      ,XOFFSET=3 ,YOFFSET=3 ,TITLE='IDL' ,SPACE=3 ,XPAD=3 ,YPAD=3  $
      ,COLUMN=1)

  
  WID_LABEL_34 = Widget_Label(WID_BASE_89, UNAME='WID_LABEL_34'  $
      ,XOFFSET=3 ,YOFFSET=3 ,/ALIGN_LEFT ,VALUE='Reset status')

  
  WID_BASE_91 = Widget_Base(WID_BASE_89, UNAME='WID_BASE_91'  $
      ,XOFFSET=3 ,YOFFSET=21 ,TITLE='IDL' ,COLUMN=1 ,/NONEXCLUSIVE)

  
  bcuresetstatusfpgareset = Widget_Button(WID_BASE_91,  $
      UNAME='bcuresetstatusfpgareset' ,/ALIGN_LEFT ,/NO_RELEASE  $
      ,VALUE='FPGA Reset')

  
  WID_BASE_285 = Widget_Base(WID_BASE_89, UNAME='WID_BASE_285'  $
      ,XOFFSET=3 ,YOFFSET=46 ,TITLE='IDL' ,COLUMN=1 ,/NONEXCLUSIVE)

  
  bcuresetstatusbusreset = Widget_Button(WID_BASE_285,  $
      UNAME='bcuresetstatusbusreset' ,/ALIGN_LEFT ,/NO_RELEASE  $
      ,VALUE='BUS Reset')

  
  WID_BASE_286 = Widget_Base(WID_BASE_89, UNAME='WID_BASE_286'  $
      ,XOFFSET=3 ,YOFFSET=71 ,TITLE='IDL' ,COLUMN=1 ,/NONEXCLUSIVE)

  
  bcuresetstatusdspreset = Widget_Button(WID_BASE_286,  $
      UNAME='bcuresetstatusdspreset' ,/ALIGN_LEFT ,/NO_RELEASE  $
      ,VALUE='DSP Reset')

  
  WID_BASE_287 = Widget_Base(WID_BASE_89, UNAME='WID_BASE_287'  $
      ,XOFFSET=3 ,YOFFSET=96 ,TITLE='IDL' ,COLUMN=1 ,/NONEXCLUSIVE)

  
  bcuresetstatusflashreset = Widget_Button(WID_BASE_287,  $
      UNAME='bcuresetstatusflashreset' ,/ALIGN_LEFT ,/NO_RELEASE  $
      ,VALUE='Flash Reset')

  
  WID_BASE_288 = Widget_Base(WID_BASE_89, UNAME='WID_BASE_288'  $
      ,XOFFSET=3 ,YOFFSET=121 ,TITLE='IDL' ,COLUMN=1 ,/NONEXCLUSIVE)

  
  bcuresetstatuspcireset = Widget_Button(WID_BASE_288,  $
      UNAME='bcuresetstatuspcireset' ,/ALIGN_LEFT ,/NO_RELEASE  $
      ,VALUE='PCI Reset')

  
  WID_BASE_289 = Widget_Base(WID_BASE_89, UNAME='WID_BASE_289'  $
      ,XOFFSET=3 ,YOFFSET=146 ,TITLE='IDL' ,COLUMN=1 ,/NONEXCLUSIVE)

  
  bcuresetstatusbusdriverenable = Widget_Button(WID_BASE_289,  $
      UNAME='bcuresetstatusbusdriverenable' ,/ALIGN_LEFT ,/NO_RELEASE  $
      ,VALUE='BUS Driver Enable')

  
  WID_BASE_290 = Widget_Base(WID_BASE_89, UNAME='WID_BASE_290'  $
      ,XOFFSET=3 ,YOFFSET=171 ,TITLE='IDL' ,COLUMN=1 ,/NONEXCLUSIVE)

  
  bcuresetstatusbusdriverenablestatus = Widget_Button(WID_BASE_290,  $
      UNAME='bcuresetstatusbusdriverenablestatus' ,/ALIGN_LEFT  $
      ,/NO_RELEASE ,VALUE='BUS Driver Enable Status')

  
  WID_BASE_291 = Widget_Base(WID_BASE_89, UNAME='WID_BASE_291'  $
      ,XOFFSET=3 ,YOFFSET=196 ,TITLE='IDL' ,COLUMN=1 ,/NONEXCLUSIVE)

  
  bcuresetstatusbuspowerfaultctrl = Widget_Button(WID_BASE_291,  $
      UNAME='bcuresetstatusbuspowerfaultctrl' ,/ALIGN_LEFT  $
      ,/NO_RELEASE ,VALUE='BUS Power Fault Ctrl')

  
  WID_BASE_292 = Widget_Base(WID_BASE_89, UNAME='WID_BASE_292'  $
      ,XOFFSET=3 ,YOFFSET=221 ,TITLE='IDL' ,COLUMN=1 ,/NONEXCLUSIVE)

  
  bcuresetstatusbuspowerfault = Widget_Button(WID_BASE_292,  $
      UNAME='bcuresetstatusbuspowerfault' ,/ALIGN_LEFT ,/NO_RELEASE  $
      ,VALUE='BUS Power Fault')

  
  WID_BASE_293 = Widget_Base(WID_BASE_89, UNAME='WID_BASE_293'  $
      ,XOFFSET=3 ,YOFFSET=246 ,TITLE='IDL' ,COLUMN=1 ,/NONEXCLUSIVE)

  
  bcuresetstatussystemwatchdog = Widget_Button(WID_BASE_293,  $
      UNAME='bcuresetstatussystemwatchdog' ,/ALIGN_LEFT ,/NO_RELEASE  $
      ,VALUE='System watchdog')

  
  WID_BASE_92 = Widget_Base(WID_BASE_1, UNAME='WID_BASE_92' ,FRAME=1  $
      ,XOFFSET=162 ,YOFFSET=3 ,TITLE='IDL' ,SPACE=3 ,XPAD=3 ,YPAD=3  $
      ,COLUMN=1)

  
  WID_LABEL_35 = Widget_Label(WID_BASE_92, UNAME='WID_LABEL_35'  $
      ,XOFFSET=3 ,YOFFSET=3 ,/ALIGN_LEFT ,VALUE='Digital I/O')

  
  WID_BASE_93 = Widget_Base(WID_BASE_92, UNAME='WID_BASE_93'  $
      ,XOFFSET=3 ,YOFFSET=21 ,TITLE='IDL' ,COLUMN=1 ,/NONEXCLUSIVE)

  
  bcudigitaliodriverenabled = Widget_Button(WID_BASE_93,  $
      UNAME='bcudigitaliodriverenabled' ,SCR_XSIZE=96 ,SCR_YSIZE=22  $
      ,/ALIGN_LEFT ,/NO_RELEASE ,VALUE='Driver Enabled')

  
  WID_BASE_294 = Widget_Base(WID_BASE_92, UNAME='WID_BASE_294'  $
      ,XOFFSET=3 ,YOFFSET=46 ,TITLE='IDL' ,COLUMN=1 ,/NONEXCLUSIVE)

  
  bcudigitaliocrateismaster = Widget_Button(WID_BASE_294,  $
      UNAME='bcudigitaliocrateismaster' ,/ALIGN_LEFT ,/NO_RELEASE  $
      ,VALUE='Crate is master')

  
  WID_BASE_295 = Widget_Base(WID_BASE_92, UNAME='WID_BASE_295'  $
      ,XOFFSET=3 ,YOFFSET=71 ,TITLE='IDL' ,COLUMN=1 ,/NONEXCLUSIVE)

  
  bcudigitaliobussysfault = Widget_Button(WID_BASE_295,  $
      UNAME='bcudigitaliobussysfault' ,/ALIGN_LEFT ,/NO_RELEASE  $
      ,VALUE='BUS system fault')

  
  WID_BASE_296 = Widget_Base(WID_BASE_92, UNAME='WID_BASE_296'  $
      ,XOFFSET=3 ,YOFFSET=96 ,TITLE='IDL' ,COLUMN=1 ,/NONEXCLUSIVE)

  
  bcudigitaliovccfault = Widget_Button(WID_BASE_296,  $
      UNAME='bcudigitaliovccfault' ,/ALIGN_LEFT ,/NO_RELEASE  $
      ,VALUE='VCC fault')

  
  WID_BASE_297 = Widget_Base(WID_BASE_92, UNAME='WID_BASE_297'  $
      ,XOFFSET=3 ,YOFFSET=121 ,TITLE='IDL' ,COLUMN=1 ,/NONEXCLUSIVE)

  
  bcudigitalioacpowerfault0 = Widget_Button(WID_BASE_297,  $
      UNAME='bcudigitalioacpowerfault0' ,/ALIGN_LEFT ,/NO_RELEASE  $
      ,VALUE='AC power fault 0')

  
  WID_BASE_298 = Widget_Base(WID_BASE_92, UNAME='WID_BASE_298'  $
      ,XOFFSET=3 ,YOFFSET=146 ,TITLE='IDL' ,COLUMN=1 ,/NONEXCLUSIVE)

  
  bcudigitalioacpowerfault1 = Widget_Button(WID_BASE_298,  $
      UNAME='bcudigitalioacpowerfault1' ,/ALIGN_LEFT ,/NO_RELEASE  $
      ,VALUE='AC power fault 1')

  
  WID_BASE_299 = Widget_Base(WID_BASE_92, UNAME='WID_BASE_299'  $
      ,XOFFSET=3 ,YOFFSET=171 ,TITLE='IDL' ,COLUMN=1 ,/NONEXCLUSIVE)

  
  bcudigitalioacpowerfault2 = Widget_Button(WID_BASE_299,  $
      UNAME='bcudigitalioacpowerfault2' ,/ALIGN_LEFT ,/NO_RELEASE  $
      ,VALUE='AC power fault 2')

  
  WID_BASE_300 = Widget_Base(WID_BASE_92, UNAME='WID_BASE_300'  $
      ,XOFFSET=3 ,YOFFSET=196 ,TITLE='IDL' ,COLUMN=1 ,/NONEXCLUSIVE)

  
  bcudigitalioicdisconnected = Widget_Button(WID_BASE_300,  $
      UNAME='bcudigitalioicdisconnected' ,/ALIGN_LEFT ,/NO_RELEASE  $
      ,VALUE='IC disconnected')

  
  WID_BASE_301 = Widget_Base(WID_BASE_92, UNAME='WID_BASE_301'  $
      ,XOFFSET=3 ,YOFFSET=221 ,TITLE='IDL' ,COLUMN=1 ,/NONEXCLUSIVE)

  
  bcudigitalioovercurrent = Widget_Button(WID_BASE_301,  $
      UNAME='bcudigitalioovercurrent' ,/ALIGN_LEFT ,/NO_RELEASE  $
      ,VALUE='Overcurrent')

  
  WID_BASE_302 = Widget_Base(WID_BASE_92, UNAME='WID_BASE_302'  $
      ,XOFFSET=3 ,YOFFSET=246 ,TITLE='IDL' ,COLUMN=1 ,/NONEXCLUSIVE)

  
  bcudigitaliocrateid = Widget_Button(WID_BASE_302,  $
      UNAME='bcudigitaliocrateid' ,/ALIGN_LEFT ,/NO_RELEASE  $
      ,VALUE='Crate ID')

  
  WID_BASE_14 = Widget_Base(WID_BASE_1, UNAME='WID_BASE_14' ,FRAME=1  $
      ,XOFFSET=279 ,YOFFSET=3 ,/BASE_ALIGN_RIGHT ,TITLE='IDL'  $
      ,SPACE=3 ,XPAD=3 ,YPAD=3 ,COLUMN=1)

  
  WID_LABEL_13 = Widget_Label(WID_BASE_14, UNAME='WID_LABEL_13'  $
      ,XOFFSET=67 ,YOFFSET=3 ,/ALIGN_RIGHT ,VALUE='Master crate'+ $
      ' global current levels')

  
  WID_BASE_152 = Widget_Base(WID_BASE_14, UNAME='WID_BASE_152'  $
      ,XOFFSET=3 ,YOFFSET=21 ,TITLE='IDL' ,SPACE=3 ,XPAD=3 ,YPAD=3  $
      ,ROW=1)

  
  WID_LABEL_245 = Widget_Label(WID_BASE_152, UNAME='WID_LABEL_245'  $
      ,XOFFSET=3 ,YOFFSET=3 ,/ALIGN_LEFT ,VALUE='CurrentVCCP')

  
  totalcurrentvccp = Widget_Text(WID_BASE_152,  $
      UNAME='totalcurrentvccp' ,XOFFSET=70 ,YOFFSET=3 ,XSIZE=20  $
      ,YSIZE=1)

  
  totalcurrentvccp_ = Widget_Button(WID_BASE_152,  $
      UNAME='totalcurrentvccp_' ,XOFFSET=197 ,YOFFSET=3  $
      ,/ALIGN_CENTER ,VALUE='ok.bmp' ,/BITMAP)

  
  WID_BASE_153 = Widget_Base(WID_BASE_14, UNAME='WID_BASE_153'  $
      ,XOFFSET=3 ,YOFFSET=51 ,TITLE='IDL' ,SPACE=3 ,XPAD=3 ,YPAD=3  $
      ,ROW=1)

  
  WID_LABEL_246 = Widget_Label(WID_BASE_153, UNAME='WID_LABEL_246'  $
      ,XOFFSET=3 ,YOFFSET=3 ,/ALIGN_LEFT ,VALUE='CurrentVSSP')

  
  totalcurrentvssp = Widget_Text(WID_BASE_153,  $
      UNAME='totalcurrentvssp' ,XOFFSET=70 ,YOFFSET=3 ,XSIZE=20  $
      ,YSIZE=1)

  
  totalcurrentvssp_ = Widget_Button(WID_BASE_153,  $
      UNAME='totalcurrentvssp_' ,XOFFSET=197 ,YOFFSET=3  $
      ,/ALIGN_CENTER ,VALUE='ok.bmp' ,/BITMAP)

  
  WID_BASE_154 = Widget_Base(WID_BASE_14, UNAME='WID_BASE_154'  $
      ,XOFFSET=17 ,YOFFSET=81 ,TITLE='IDL' ,SPACE=3 ,XPAD=3 ,YPAD=3  $
      ,ROW=1)

  
  WID_LABEL_247 = Widget_Label(WID_BASE_154, UNAME='WID_LABEL_247'  $
      ,XOFFSET=3 ,YOFFSET=3 ,/ALIGN_LEFT ,VALUE='CurrentVP')

  
  totalcurrentvp = Widget_Text(WID_BASE_154, UNAME='totalcurrentvp'  $
      ,XOFFSET=56 ,YOFFSET=3 ,XSIZE=20 ,YSIZE=1)

  
  totalcurrentvp_ = Widget_Button(WID_BASE_154,  $
      UNAME='totalcurrentvp_' ,XOFFSET=183 ,YOFFSET=3 ,/ALIGN_CENTER  $
      ,VALUE='ok.bmp' ,/BITMAP)

  
  WID_BASE_70 = Widget_Base(WID_BASE_1, UNAME='WID_BASE_70' ,FRAME=1  $
      ,XOFFSET=510 ,YOFFSET=3 ,/BASE_ALIGN_RIGHT ,TITLE='IDL'  $
      ,SPACE=3 ,XPAD=3 ,YPAD=3 ,COLUMN=1)

  
  WID_LABEL_163 = Widget_Label(WID_BASE_70, UNAME='WID_LABEL_163'  $
      ,XOFFSET=215 ,YOFFSET=3 ,/ALIGN_RIGHT ,VALUE='DiagnApp data')

  
  WID_BASE_79 = Widget_Base(WID_BASE_70, UNAME='WID_BASE_79'  $
      ,XOFFSET=21 ,YOFFSET=21 ,TITLE='IDL' ,SPACE=3 ,XPAD=3 ,YPAD=3  $
      ,ROW=1)

  
  WID_LABEL_164 = Widget_Label(WID_BASE_79, UNAME='WID_LABEL_164'  $
      ,XOFFSET=3 ,YOFFSET=3 ,/ALIGN_LEFT  $
      ,VALUE='LocalCurrentThreshold')

  
  bculocalcurrentthreshold = Widget_Text(WID_BASE_79,  $
      UNAME='bculocalcurrentthreshold' ,XOFFSET=115 ,YOFFSET=3  $
      ,XSIZE=20 ,YSIZE=1)

  
  bculocalcurrentthreshold_ = Widget_Button(WID_BASE_79,  $
      UNAME='bculocalcurrentthreshold_' ,XOFFSET=242 ,YOFFSET=3  $
      ,/ALIGN_CENTER ,VALUE='ok.bmp' ,/BITMAP)

  
  WID_BASE_84 = Widget_Base(WID_BASE_70, UNAME='WID_BASE_84'  $
      ,XOFFSET=98 ,YOFFSET=51 ,TITLE='IDL' ,SPACE=3 ,XPAD=3 ,YPAD=3  $
      ,ROW=1)

  
  WID_LABEL_165 = Widget_Label(WID_BASE_84, UNAME='WID_LABEL_165'  $
      ,XOFFSET=3 ,YOFFSET=3 ,/ALIGN_LEFT ,VALUE='VPSet')

  
  bcuvpset = Widget_Text(WID_BASE_84, UNAME='bcuvpset' ,XOFFSET=38  $
      ,YOFFSET=3 ,XSIZE=20 ,YSIZE=1)

  
  bcuvpset_ = Widget_Button(WID_BASE_84, UNAME='bcuvpset_'  $
      ,XOFFSET=165 ,YOFFSET=3 ,/ALIGN_CENTER ,VALUE='ok.bmp'  $
      ,/BITMAP)

  
  WID_BASE_112 = Widget_Base(WID_BASE_70, UNAME='WID_BASE_112'  $
      ,XOFFSET=5 ,YOFFSET=81 ,TITLE='IDL' ,SPACE=3 ,XPAD=3 ,YPAD=3  $
      ,ROW=1)

  
  WID_LABEL_166 = Widget_Label(WID_BASE_112, UNAME='WID_LABEL_166'  $
      ,XOFFSET=3 ,YOFFSET=3 ,/ALIGN_LEFT  $
      ,VALUE='TotalCurrentThresholdPos')

  
  bcutotalcurrentthresholdpos = Widget_Text(WID_BASE_112,  $
      UNAME='bcutotalcurrentthresholdpos' ,XOFFSET=131 ,YOFFSET=3  $
      ,XSIZE=20 ,YSIZE=1)

  
  bcutotalcurrentthresholdpos_ = Widget_Button(WID_BASE_112,  $
      UNAME='bcutotalcurrentthresholdpos_' ,XOFFSET=258 ,YOFFSET=3  $
      ,/ALIGN_CENTER ,VALUE='ok.bmp' ,/BITMAP)

  
  WID_BASE_113 = Widget_Base(WID_BASE_70, UNAME='WID_BASE_113'  $
      ,XOFFSET=3 ,YOFFSET=111 ,TITLE='IDL' ,SPACE=3 ,XPAD=3 ,YPAD=3  $
      ,ROW=1)

  
  WID_LABEL_167 = Widget_Label(WID_BASE_113, UNAME='WID_LABEL_167'  $
      ,XOFFSET=3 ,YOFFSET=3 ,/ALIGN_LEFT  $
      ,VALUE='TotalCurrentThresholdNeg')

  
  bcutotalcurrentthresholdneg = Widget_Text(WID_BASE_113,  $
      UNAME='bcutotalcurrentthresholdneg' ,XOFFSET=133 ,YOFFSET=3  $
      ,XSIZE=20 ,YSIZE=1)

  
  bcutotalcurrentthresholdneg_ = Widget_Button(WID_BASE_113,  $
      UNAME='bcutotalcurrentthresholdneg_' ,XOFFSET=260 ,YOFFSET=3  $
      ,/ALIGN_CENTER ,VALUE='ok.bmp' ,/BITMAP)

  
  WID_BASE_8 = Widget_Base(CRATE_BCU2_BASE, UNAME='WID_BASE_8'  $
      ,XOFFSET=3 ,YOFFSET=283 ,TITLE='IDL' ,SPACE=3 ,XPAD=3 ,YPAD=3  $
      ,ROW=1)

  
  bcu_crate_num2 = Widget_Slider(WID_BASE_8, UNAME='bcu_crate_num2'  $
      ,XOFFSET=3 ,YOFFSET=3 ,TITLE='Crate (0-5)' ,MAXIMUM=5)

  
  CRATE_DSP_BASE = Widget_Base(WID_TAB_0, UNAME='CRATE_DSP_BASE'  $
      ,FRAME=1 ,SCR_XSIZE=887 ,SCR_YSIZE=641 ,TITLE='Crate DSP'  $
      ,XPAD=3 ,YPAD=3 ,COLUMN=1)

  
  WID_BASE_3 = Widget_Base(CRATE_DSP_BASE, UNAME='WID_BASE_3'  $
      ,XOFFSET=3 ,YOFFSET=3 ,TITLE='IDL' ,SPACE=3 ,XPAD=3 ,YPAD=3  $
      ,ROW=1)

  
  WID_BASE_147 = Widget_Base(WID_BASE_3, UNAME='WID_BASE_147'  $
      ,FRAME=1 ,XOFFSET=3 ,YOFFSET=3 ,/BASE_ALIGN_RIGHT ,TITLE='IDL'  $
      ,XPAD=3 ,YPAD=3 ,COLUMN=1)

  
  WID_LABEL_104 = Widget_Label(WID_BASE_147, UNAME='WID_LABEL_104'  $
      ,XOFFSET=194 ,YOFFSET=3 ,/ALIGN_RIGHT ,VALUE='NIOS Fixed Area')

  
  WID_BASE_148 = Widget_Base(WID_BASE_147, UNAME='WID_BASE_148'  $
      ,XOFFSET=74 ,YOFFSET=18 ,TITLE='IDL' ,SPACE=3 ,XPAD=3 ,YPAD=3  $
      ,ROW=1)

  
  WID_LABEL_259 = Widget_Label(WID_BASE_148, UNAME='WID_LABEL_259'  $
      ,XOFFSET=3 ,YOFFSET=3 ,/ALIGN_LEFT ,VALUE='WhoAmI')

  
  dspwhoami = Widget_Text(WID_BASE_148, UNAME='dspwhoami' ,XOFFSET=49  $
      ,YOFFSET=3 ,XSIZE=20 ,YSIZE=1)

  
  dspwhoami_ = Widget_Button(WID_BASE_148, UNAME='dspwhoami_'  $
      ,XOFFSET=176 ,YOFFSET=3 ,/ALIGN_CENTER ,VALUE='ok.bmp'  $
      ,/BITMAP)

  
  WID_BASE_149 = Widget_Base(WID_BASE_147, UNAME='WID_BASE_149'  $
      ,XOFFSET=34 ,YOFFSET=45 ,TITLE='IDL' ,SPACE=3 ,XPAD=3 ,YPAD=3  $
      ,ROW=1)

  
  WID_LABEL_260 = Widget_Label(WID_BASE_149, UNAME='WID_LABEL_260'  $
      ,XOFFSET=3 ,YOFFSET=3 ,/ALIGN_LEFT ,VALUE='SoftwareRelease')

  
  dspsoftwarerelease = Widget_Text(WID_BASE_149,  $
      UNAME='dspsoftwarerelease' ,XOFFSET=89 ,YOFFSET=3 ,XSIZE=20  $
      ,YSIZE=1)

  
  dspsoftwarerelease_ = Widget_Button(WID_BASE_149,  $
      UNAME='dspsoftwarerelease_' ,XOFFSET=216 ,YOFFSET=3  $
      ,/ALIGN_CENTER ,VALUE='ok.bmp' ,/BITMAP)

  
  WID_BASE_150 = Widget_Base(WID_BASE_147, UNAME='WID_BASE_150'  $
      ,XOFFSET=50 ,YOFFSET=72 ,TITLE='IDL' ,SPACE=3 ,XPAD=3 ,YPAD=3  $
      ,ROW=1)

  
  WID_LABEL_261 = Widget_Label(WID_BASE_150, UNAME='WID_LABEL_261'  $
      ,XOFFSET=3 ,YOFFSET=3 ,/ALIGN_LEFT ,VALUE='LogicRelease')

  
  dsplogicrelease = Widget_Text(WID_BASE_150, UNAME='dsplogicrelease'  $
      ,XOFFSET=73 ,YOFFSET=3 ,XSIZE=20 ,YSIZE=1)

  
  dsplogicrelease_ = Widget_Button(WID_BASE_150,  $
      UNAME='dsplogicrelease_' ,XOFFSET=200 ,YOFFSET=3 ,/ALIGN_CENTER  $
      ,VALUE='ok.bmp' ,/BITMAP)

  
  WID_BASE_201 = Widget_Base(WID_BASE_147, UNAME='WID_BASE_201'  $
      ,XOFFSET=52 ,YOFFSET=99 ,TITLE='IDL' ,SPACE=3 ,XPAD=3 ,YPAD=3  $
      ,ROW=1)

  
  WID_LABEL_262 = Widget_Label(WID_BASE_201, UNAME='WID_LABEL_262'  $
      ,XOFFSET=3 ,YOFFSET=3 ,/ALIGN_LEFT ,VALUE='SerialNumber')

  
  dspserialnumber = Widget_Text(WID_BASE_201, UNAME='dspserialnumber'  $
      ,XOFFSET=71 ,YOFFSET=3 ,XSIZE=20 ,YSIZE=1)

  
  dspserialnumber_ = Widget_Button(WID_BASE_201,  $
      UNAME='dspserialnumber_' ,XOFFSET=198 ,YOFFSET=3 ,/ALIGN_CENTER  $
      ,VALUE='ok.bmp' ,/BITMAP)

  
  WID_BASE_202 = Widget_Base(WID_BASE_147, UNAME='WID_BASE_202'  $
      ,XOFFSET=17 ,YOFFSET=126 ,TITLE='IDL' ,SPACE=3 ,XPAD=3 ,YPAD=3  $
      ,ROW=1)

  
  WID_LABEL_263 = Widget_Label(WID_BASE_202, UNAME='WID_LABEL_263'  $
      ,XOFFSET=3 ,YOFFSET=3 ,/ALIGN_LEFT  $
      ,VALUE='DiagnosticRecordPtr')

  
  dspdiagnosticrecordptr = Widget_Text(WID_BASE_202,  $
      UNAME='dspdiagnosticrecordptr' ,XOFFSET=106 ,YOFFSET=3  $
      ,XSIZE=20 ,YSIZE=1)

  
  dspdiagnosticrecordptr_ = Widget_Button(WID_BASE_202,  $
      UNAME='dspdiagnosticrecordptr_' ,XOFFSET=233 ,YOFFSET=3  $
      ,/ALIGN_CENTER ,VALUE='ok.bmp' ,/BITMAP)

  
  WID_BASE_203 = Widget_Base(WID_BASE_147, UNAME='WID_BASE_203'  $
      ,XOFFSET=12 ,YOFFSET=153 ,TITLE='IDL' ,SPACE=3 ,XPAD=3 ,YPAD=3  $
      ,ROW=1)

  
  WID_LABEL_264 = Widget_Label(WID_BASE_203, UNAME='WID_LABEL_264'  $
      ,XOFFSET=3 ,YOFFSET=3 ,/ALIGN_LEFT  $
      ,VALUE='DiagnosticRecordLen')

  
  dspdiagnosticrecordlen = Widget_Text(WID_BASE_203,  $
      UNAME='dspdiagnosticrecordlen' ,XOFFSET=111 ,YOFFSET=3  $
      ,XSIZE=20 ,YSIZE=1)

  
  dspdiagnosticrecordlen_ = Widget_Button(WID_BASE_203,  $
      UNAME='dspdiagnosticrecordlen_' ,XOFFSET=238 ,YOFFSET=3  $
      ,/ALIGN_CENTER ,VALUE='ok.bmp' ,/BITMAP)

  
  WID_BASE_204 = Widget_Base(WID_BASE_147, UNAME='WID_BASE_204'  $
      ,XOFFSET=3 ,YOFFSET=180 ,TITLE='IDL' ,SPACE=3 ,XPAD=3 ,YPAD=3  $
      ,ROW=1)

  
  WID_LABEL_265 = Widget_Label(WID_BASE_204, UNAME='WID_LABEL_265'  $
      ,XOFFSET=3 ,YOFFSET=3 ,/ALIGN_LEFT  $
      ,VALUE='RdDiagnosticRecordPtr')

  
  dsprddiagnosticrecordptr = Widget_Text(WID_BASE_204,  $
      UNAME='dsprddiagnosticrecordptr' ,XOFFSET=120 ,YOFFSET=3  $
      ,XSIZE=20 ,YSIZE=1)

  
  dsprddiagnosticrecordptr_ = Widget_Button(WID_BASE_204,  $
      UNAME='dsprddiagnosticrecordptr_' ,XOFFSET=247 ,YOFFSET=3  $
      ,/ALIGN_CENTER ,VALUE='ok.bmp' ,/BITMAP)

  
  WID_BASE_205 = Widget_Base(WID_BASE_147, UNAME='WID_BASE_205'  $
      ,XOFFSET=3 ,YOFFSET=207 ,TITLE='IDL' ,SPACE=3 ,XPAD=3 ,YPAD=3  $
      ,ROW=1)

  
  WID_LABEL_266 = Widget_Label(WID_BASE_205, UNAME='WID_LABEL_266'  $
      ,XOFFSET=3 ,YOFFSET=3 ,/ALIGN_LEFT  $
      ,VALUE='WrDiagnosticRecordPtr')

  
  dspwrdiagnosticrecordptr = Widget_Text(WID_BASE_205,  $
      UNAME='dspwrdiagnosticrecordptr' ,XOFFSET=120 ,YOFFSET=3  $
      ,XSIZE=20 ,YSIZE=1)

  
  dspwrdiagnosticrecordptr_ = Widget_Button(WID_BASE_205,  $
      UNAME='dspwrdiagnosticrecordptr_' ,XOFFSET=247 ,YOFFSET=3  $
      ,/ALIGN_CENTER ,VALUE='ok.bmp' ,/BITMAP)

  
  WID_BASE_64 = Widget_Base(WID_BASE_147, UNAME='WID_BASE_64'  $
      ,FRAME=1 ,XOFFSET=52 ,YOFFSET=234 ,/BASE_ALIGN_RIGHT  $
      ,TITLE='IDL' ,XPAD=3 ,YPAD=3 ,COLUMN=1)

  
  WID_BASE_206 = Widget_Base(WID_BASE_64, UNAME='WID_BASE_206'  $
      ,XOFFSET=3 ,YOFFSET=3 ,TITLE='IDL' ,XPAD=3 ,YPAD=3 ,ROW=1)

  
  WID_LABEL_267 = Widget_Label(WID_BASE_206, UNAME='WID_LABEL_267'  $
      ,XOFFSET=3 ,YOFFSET=3 ,/ALIGN_LEFT ,VALUE='ADCspiOffset')

  
  dspadcspioffset = Widget_Text(WID_BASE_206, UNAME='dspadcspioffset'  $
      ,XOFFSET=68 ,YOFFSET=3 ,XSIZE=20 ,YSIZE=1)

  
  dspadcspioffset_ = Widget_Button(WID_BASE_206,  $
      UNAME='dspadcspioffset_' ,XOFFSET=192 ,YOFFSET=3 ,/ALIGN_CENTER  $
      ,VALUE='ok.bmp' ,/BITMAP)

  
  WID_BASE_207 = Widget_Base(WID_BASE_64, UNAME='WID_BASE_207'  $
      ,XOFFSET=9 ,YOFFSET=30 ,TITLE='IDL' ,XPAD=3 ,YPAD=3 ,ROW=1)

  
  WID_LABEL_268 = Widget_Label(WID_BASE_207, UNAME='WID_LABEL_268'  $
      ,XOFFSET=3 ,YOFFSET=3 ,/ALIGN_LEFT ,VALUE='ADCspiGain')

  
  dspadcspigain = Widget_Text(WID_BASE_207, UNAME='dspadcspigain'  $
      ,XOFFSET=62 ,YOFFSET=3 ,XSIZE=20 ,YSIZE=1)

  
  dspadcspigain_ = Widget_Button(WID_BASE_207, UNAME='dspadcspigain_'  $
      ,XOFFSET=186 ,YOFFSET=3 ,/ALIGN_CENTER ,VALUE='ok.bmp'  $
      ,/BITMAP)

  
  WID_BASE_208 = Widget_Base(WID_BASE_64, UNAME='WID_BASE_208'  $
      ,XOFFSET=16 ,YOFFSET=57 ,TITLE='IDL' ,XPAD=3 ,YPAD=3 ,ROW=1)

  
  WID_LABEL_269 = Widget_Label(WID_BASE_208, UNAME='WID_LABEL_269'  $
      ,XOFFSET=3 ,YOFFSET=3 ,/ALIGN_LEFT ,VALUE='ADCOffset')

  
  dspadcoffset = Widget_Text(WID_BASE_208, UNAME='dspadcoffset'  $
      ,XOFFSET=55 ,YOFFSET=3 ,XSIZE=20 ,YSIZE=1)

  
  dspadcoffset_ = Widget_Button(WID_BASE_208, UNAME='dspadcoffset_'  $
      ,XOFFSET=179 ,YOFFSET=3 ,/ALIGN_CENTER ,VALUE='ok.bmp'  $
      ,/BITMAP)

  
  WID_BASE_209 = Widget_Base(WID_BASE_64, UNAME='WID_BASE_209'  $
      ,XOFFSET=22 ,YOFFSET=84 ,TITLE='IDL' ,XPAD=3 ,YPAD=3 ,ROW=1)

  
  WID_LABEL_270 = Widget_Label(WID_BASE_209, UNAME='WID_LABEL_270'  $
      ,XOFFSET=3 ,YOFFSET=3 ,/ALIGN_LEFT ,VALUE='ADCGain')

  
  dspadcgain = Widget_Text(WID_BASE_209, UNAME='dspadcgain'  $
      ,XOFFSET=49 ,YOFFSET=3 ,XSIZE=20 ,YSIZE=1)

  
  dspadcgain_ = Widget_Button(WID_BASE_209, UNAME='dspadcgain_'  $
      ,XOFFSET=173 ,YOFFSET=3 ,/ALIGN_CENTER ,VALUE='ok.bmp'  $
      ,/BITMAP)

  
  WID_BASE_210 = Widget_Base(WID_BASE_64, UNAME='WID_BASE_210'  $
      ,XOFFSET=16 ,YOFFSET=111 ,TITLE='IDL' ,XPAD=3 ,YPAD=3 ,ROW=1)

  
  WID_LABEL_271 = Widget_Label(WID_BASE_210, UNAME='WID_LABEL_271'  $
      ,XOFFSET=3 ,YOFFSET=3 ,/ALIGN_LEFT ,VALUE='DACOffset')

  
  dspdacoffset = Widget_Text(WID_BASE_210, UNAME='dspdacoffset'  $
      ,XOFFSET=55 ,YOFFSET=3 ,XSIZE=20 ,YSIZE=1)

  
  dspdacoffset_ = Widget_Button(WID_BASE_210, UNAME='dspdacoffset_'  $
      ,XOFFSET=179 ,YOFFSET=3 ,/ALIGN_CENTER ,VALUE='ok.bmp'  $
      ,/BITMAP)

  
  WID_BASE_211 = Widget_Base(WID_BASE_64, UNAME='WID_BASE_211'  $
      ,XOFFSET=22 ,YOFFSET=138 ,TITLE='IDL' ,XPAD=3 ,YPAD=3 ,ROW=1)

  
  WID_LABEL_272 = Widget_Label(WID_BASE_211, UNAME='WID_LABEL_272'  $
      ,XOFFSET=3 ,YOFFSET=3 ,/ALIGN_LEFT ,VALUE='DACGain')

  
  dspdacgain = Widget_Text(WID_BASE_211, UNAME='dspdacgain'  $
      ,XOFFSET=49 ,YOFFSET=3 ,XSIZE=20 ,YSIZE=1)

  
  dspdacgain_ = Widget_Button(WID_BASE_211, UNAME='dspdacgain_'  $
      ,XOFFSET=173 ,YOFFSET=3 ,/ALIGN_CENTER ,VALUE='ok.bmp'  $
      ,/BITMAP)

  
  WID_BASE_73 = Widget_Base(WID_BASE_64, UNAME='WID_BASE_73'  $
      ,XOFFSET=19 ,YOFFSET=165 ,TITLE='IDL' ,XPAD=3 ,YPAD=3 ,ROW=1)

  
  WID_LABEL_23 = Widget_Label(WID_BASE_73, UNAME='WID_LABEL_23'  $
      ,XOFFSET=3 ,YOFFSET=11 ,/ALIGN_CENTER ,VALUE='Board channel'+ $
      ' (0-7)')

  
  dsp_board_channel = Widget_Slider(WID_BASE_73,  $
      UNAME='dsp_board_channel' ,XOFFSET=98 ,YOFFSET=3 ,MAXIMUM=7)

  
  WID_BASE_74 = Widget_Base(WID_BASE_3, UNAME='WID_BASE_74'  $
      ,XOFFSET=284 ,YOFFSET=3 ,/BASE_ALIGN_RIGHT ,TITLE='IDL' ,XPAD=3  $
      ,YPAD=3 ,COLUMN=1)

  
  WID_BASE_75 = Widget_Base(WID_BASE_74, UNAME='WID_BASE_75' ,FRAME=1  $
      ,XOFFSET=3 ,YOFFSET=3 ,/BASE_ALIGN_RIGHT ,TITLE='IDL' ,XPAD=3  $
      ,YPAD=3 ,COLUMN=1)

  
  WID_LABEL_24 = Widget_Label(WID_BASE_75, UNAME='WID_LABEL_24'  $
      ,XOFFSET=129 ,YOFFSET=3 ,/ALIGN_RIGHT ,VALUE='Temperatures')

  
  WID_BASE_212 = Widget_Base(WID_BASE_75, UNAME='WID_BASE_212'  $
      ,XOFFSET=7 ,YOFFSET=18 ,TITLE='IDL' ,SPACE=3 ,XPAD=3 ,YPAD=3  $
      ,ROW=1)

  
  WID_LABEL_273 = Widget_Label(WID_BASE_212, UNAME='WID_LABEL_273'  $
      ,XOFFSET=3 ,YOFFSET=3 ,/ALIGN_LEFT ,VALUE='Stratix')

  
  dspstratixtemp = Widget_Text(WID_BASE_212, UNAME='dspstratixtemp'  $
      ,XOFFSET=37 ,YOFFSET=3 ,XSIZE=20 ,YSIZE=1)

  
  dspstratixtemp_ = Widget_Button(WID_BASE_212,  $
      UNAME='dspstratixtemp_' ,XOFFSET=164 ,YOFFSET=3 ,/ALIGN_CENTER  $
      ,VALUE='ok.bmp' ,/BITMAP)

  
  WID_BASE_213 = Widget_Base(WID_BASE_75, UNAME='WID_BASE_213'  $
      ,XOFFSET=6 ,YOFFSET=45 ,TITLE='IDL' ,SPACE=3 ,XPAD=3 ,YPAD=3  $
      ,ROW=1)

  
  WID_LABEL_274 = Widget_Label(WID_BASE_213, UNAME='WID_LABEL_274'  $
      ,XOFFSET=3 ,YOFFSET=3 ,/ALIGN_LEFT ,VALUE='Power')

  
  dsppowertemp = Widget_Text(WID_BASE_213, UNAME='dsppowertemp'  $
      ,XOFFSET=38 ,YOFFSET=3 ,XSIZE=20 ,YSIZE=1)

  
  dsppowertemp_ = Widget_Button(WID_BASE_213, UNAME='dsppowertemp_'  $
      ,XOFFSET=165 ,YOFFSET=3 ,/ALIGN_CENTER ,VALUE='ok.bmp'  $
      ,/BITMAP)

  
  WID_BASE_214 = Widget_Base(WID_BASE_75, UNAME='WID_BASE_214'  $
      ,XOFFSET=9 ,YOFFSET=72 ,TITLE='IDL' ,SPACE=3 ,XPAD=3 ,YPAD=3  $
      ,ROW=1)

  
  WID_LABEL_275 = Widget_Label(WID_BASE_214, UNAME='WID_LABEL_275'  $
      ,XOFFSET=3 ,YOFFSET=3 ,/ALIGN_LEFT ,VALUE='DSPs')

  
  dspdspstemp = Widget_Text(WID_BASE_214, UNAME='dspdspstemp'  $
      ,XOFFSET=35 ,YOFFSET=3 ,XSIZE=20 ,YSIZE=1)

  
  dspdspstemp_ = Widget_Button(WID_BASE_214, UNAME='dspdspstemp_'  $
      ,XOFFSET=162 ,YOFFSET=3 ,/ALIGN_CENTER ,VALUE='ok.bmp'  $
      ,/BITMAP)

  
  WID_BASE_215 = Widget_Base(WID_BASE_75, UNAME='WID_BASE_215'  $
      ,XOFFSET=3 ,YOFFSET=99 ,TITLE='IDL' ,SPACE=3 ,XPAD=3 ,YPAD=3  $
      ,ROW=1)

  
  WID_LABEL_276 = Widget_Label(WID_BASE_215, UNAME='WID_LABEL_276'  $
      ,XOFFSET=3 ,YOFFSET=3 ,/ALIGN_LEFT ,VALUE='Drivers')

  
  dspdrivertemp = Widget_Text(WID_BASE_215, UNAME='dspdrivertemp'  $
      ,XOFFSET=41 ,YOFFSET=3 ,XSIZE=20 ,YSIZE=1)

  
  dspdrivertemp_ = Widget_Button(WID_BASE_215, UNAME='dspdrivertemp_'  $
      ,XOFFSET=168 ,YOFFSET=3 ,/ALIGN_CENTER ,VALUE='ok.bmp'  $
      ,/BITMAP)

  
  WID_BASE_80 = Widget_Base(WID_BASE_74, UNAME='WID_BASE_80' ,FRAME=1  $
      ,XOFFSET=18 ,YOFFSET=132 ,TITLE='IDL' ,XPAD=3 ,YPAD=3  $
      ,COLUMN=1)

  
  WID_LABEL_25 = Widget_Label(WID_BASE_80, UNAME='WID_LABEL_25'  $
      ,XOFFSET=95 ,YOFFSET=3 ,/ALIGN_RIGHT ,VALUE='Coil currents'+ $
      ' (SPI)')

  
  WID_BASE_216 = Widget_Base(WID_BASE_80, UNAME='WID_BASE_216'  $
      ,XOFFSET=3 ,YOFFSET=18 ,TITLE='IDL' ,SPACE=3 ,XPAD=3 ,YPAD=3  $
      ,ROW=1)

  
  WID_LABEL_277 = Widget_Label(WID_BASE_216, UNAME='WID_LABEL_277'  $
      ,XOFFSET=3 ,YOFFSET=3 ,/ALIGN_LEFT ,VALUE='ch0')

  
  dspcoilcurrent0 = Widget_Text(WID_BASE_216, UNAME='dspcoilcurrent0'  $
      ,XOFFSET=26 ,YOFFSET=3 ,XSIZE=20 ,YSIZE=1)

  
  dspcoilcurrent0_ = Widget_Button(WID_BASE_216,  $
      UNAME='dspcoilcurrent0_' ,XOFFSET=153 ,YOFFSET=3 ,/ALIGN_CENTER  $
      ,VALUE='ok.bmp' ,/BITMAP)

  
  WID_BASE_217 = Widget_Base(WID_BASE_80, UNAME='WID_BASE_217'  $
      ,XOFFSET=3 ,YOFFSET=45 ,TITLE='IDL' ,SPACE=3 ,XPAD=3 ,YPAD=3  $
      ,ROW=1)

  
  WID_LABEL_278 = Widget_Label(WID_BASE_217, UNAME='WID_LABEL_278'  $
      ,XOFFSET=3 ,YOFFSET=3 ,/ALIGN_LEFT ,VALUE='ch1')

  
  dspcoilcurrent1 = Widget_Text(WID_BASE_217, UNAME='dspcoilcurrent1'  $
      ,XOFFSET=26 ,YOFFSET=3 ,XSIZE=20 ,YSIZE=1)

  
  dspcoilcurrent1_ = Widget_Button(WID_BASE_217,  $
      UNAME='dspcoilcurrent1_' ,XOFFSET=153 ,YOFFSET=3 ,/ALIGN_CENTER  $
      ,VALUE='ok.bmp' ,/BITMAP)

  
  WID_BASE_218 = Widget_Base(WID_BASE_80, UNAME='WID_BASE_218'  $
      ,XOFFSET=3 ,YOFFSET=72 ,TITLE='IDL' ,SPACE=3 ,XPAD=3 ,YPAD=3  $
      ,ROW=1)

  
  WID_LABEL_279 = Widget_Label(WID_BASE_218, UNAME='WID_LABEL_279'  $
      ,XOFFSET=3 ,YOFFSET=3 ,/ALIGN_LEFT ,VALUE='ch2')

  
  dspcoilcurrent2 = Widget_Text(WID_BASE_218, UNAME='dspcoilcurrent2'  $
      ,XOFFSET=26 ,YOFFSET=3 ,XSIZE=20 ,YSIZE=1)

  
  dspcoilcurrent2_ = Widget_Button(WID_BASE_218,  $
      UNAME='dspcoilcurrent2_' ,XOFFSET=153 ,YOFFSET=3 ,/ALIGN_CENTER  $
      ,VALUE='ok.bmp' ,/BITMAP)

  
  WID_BASE_219 = Widget_Base(WID_BASE_80, UNAME='WID_BASE_219'  $
      ,XOFFSET=3 ,YOFFSET=99 ,TITLE='IDL' ,SPACE=3 ,XPAD=3 ,YPAD=3  $
      ,ROW=1)

  
  WID_LABEL_280 = Widget_Label(WID_BASE_219, UNAME='WID_LABEL_280'  $
      ,XOFFSET=3 ,YOFFSET=3 ,/ALIGN_LEFT ,VALUE='ch3')

  
  dspcoilcurrent3 = Widget_Text(WID_BASE_219, UNAME='dspcoilcurrent3'  $
      ,XOFFSET=26 ,YOFFSET=3 ,XSIZE=20 ,YSIZE=1)

  
  dspcoilcurrent3_ = Widget_Button(WID_BASE_219,  $
      UNAME='dspcoilcurrent3_' ,XOFFSET=153 ,YOFFSET=3 ,/ALIGN_CENTER  $
      ,VALUE='ok.bmp' ,/BITMAP)

  
  WID_BASE_220 = Widget_Base(WID_BASE_80, UNAME='WID_BASE_220'  $
      ,XOFFSET=3 ,YOFFSET=126 ,TITLE='IDL' ,SPACE=3 ,XPAD=3 ,YPAD=3  $
      ,ROW=1)

  
  WID_LABEL_281 = Widget_Label(WID_BASE_220, UNAME='WID_LABEL_281'  $
      ,XOFFSET=3 ,YOFFSET=3 ,/ALIGN_LEFT ,VALUE='ch4')

  
  dspcoilcurrent4 = Widget_Text(WID_BASE_220, UNAME='dspcoilcurrent4'  $
      ,XOFFSET=26 ,YOFFSET=3 ,XSIZE=20 ,YSIZE=1)

  
  dspcoilcurrent4_ = Widget_Button(WID_BASE_220,  $
      UNAME='dspcoilcurrent4_' ,XOFFSET=153 ,YOFFSET=3 ,/ALIGN_CENTER  $
      ,VALUE='ok.bmp' ,/BITMAP)

  
  WID_BASE_221 = Widget_Base(WID_BASE_80, UNAME='WID_BASE_221'  $
      ,XOFFSET=3 ,YOFFSET=153 ,TITLE='IDL' ,SPACE=3 ,XPAD=3 ,YPAD=3  $
      ,ROW=1)

  
  WID_LABEL_282 = Widget_Label(WID_BASE_221, UNAME='WID_LABEL_282'  $
      ,XOFFSET=3 ,YOFFSET=3 ,/ALIGN_LEFT ,VALUE='ch5')

  
  dspcoilcurrent5 = Widget_Text(WID_BASE_221, UNAME='dspcoilcurrent5'  $
      ,XOFFSET=26 ,YOFFSET=3 ,XSIZE=20 ,YSIZE=1)

  
  dspcoilcurrent5_ = Widget_Button(WID_BASE_221,  $
      UNAME='dspcoilcurrent5_' ,XOFFSET=153 ,YOFFSET=3 ,/ALIGN_CENTER  $
      ,VALUE='ok.bmp' ,/BITMAP)

  
  WID_BASE_222 = Widget_Base(WID_BASE_80, UNAME='WID_BASE_222'  $
      ,XOFFSET=3 ,YOFFSET=180 ,TITLE='IDL' ,SPACE=3 ,XPAD=3 ,YPAD=3  $
      ,ROW=1)

  
  WID_LABEL_283 = Widget_Label(WID_BASE_222, UNAME='WID_LABEL_283'  $
      ,XOFFSET=3 ,YOFFSET=3 ,/ALIGN_LEFT ,VALUE='ch6')

  
  dspcoilcurrent6 = Widget_Text(WID_BASE_222, UNAME='dspcoilcurrent6'  $
      ,XOFFSET=26 ,YOFFSET=3 ,XSIZE=20 ,YSIZE=1)

  
  dspcoilcurrent6_ = Widget_Button(WID_BASE_222,  $
      UNAME='dspcoilcurrent6_' ,XOFFSET=153 ,YOFFSET=3 ,/ALIGN_CENTER  $
      ,VALUE='ok.bmp' ,/BITMAP)

  
  WID_BASE_223 = Widget_Base(WID_BASE_80, UNAME='WID_BASE_223'  $
      ,XOFFSET=3 ,YOFFSET=207 ,TITLE='IDL' ,SPACE=3 ,XPAD=3 ,YPAD=3  $
      ,ROW=1)

  
  WID_LABEL_284 = Widget_Label(WID_BASE_223, UNAME='WID_LABEL_284'  $
      ,XOFFSET=3 ,YOFFSET=3 ,/ALIGN_LEFT ,VALUE='ch7')

  
  dspcoilcurrent7 = Widget_Text(WID_BASE_223, UNAME='dspcoilcurrent7'  $
      ,XOFFSET=26 ,YOFFSET=3 ,XSIZE=20 ,YSIZE=1)

  
  dspcoilcurrent7_ = Widget_Button(WID_BASE_223,  $
      UNAME='dspcoilcurrent7_' ,XOFFSET=153 ,YOFFSET=3 ,/ALIGN_CENTER  $
      ,VALUE='ok.bmp' ,/BITMAP)

  
  WID_BASE_81 = Widget_Base(WID_BASE_3, UNAME='WID_BASE_81' ,FRAME=1  $
      ,XOFFSET=492 ,YOFFSET=3 ,TITLE='IDL' ,XPAD=3 ,YPAD=3 ,COLUMN=1)

  
  WID_LABEL_26 = Widget_Label(WID_BASE_81, UNAME='WID_LABEL_26'  $
      ,XOFFSET=3 ,YOFFSET=3 ,/ALIGN_LEFT ,VALUE='Driver Status')

  
  WID_BASE_83 = Widget_Base(WID_BASE_81, UNAME='WID_BASE_83'  $
      ,XOFFSET=3 ,YOFFSET=18 ,TITLE='IDL' ,COLUMN=1 ,/NONEXCLUSIVE)

  
  dspdriverstatusfpgareset = Widget_Button(WID_BASE_83,  $
      UNAME='dspdriverstatusfpgareset' ,/ALIGN_LEFT ,/NO_RELEASE  $
      ,VALUE='FPGA Reset')

  
  WID_BASE_224 = Widget_Base(WID_BASE_81, UNAME='WID_BASE_224'  $
      ,XOFFSET=3 ,YOFFSET=40 ,TITLE='IDL' ,COLUMN=1 ,/NONEXCLUSIVE)

  
  dspdriverstatusdsp0reset = Widget_Button(WID_BASE_224,  $
      UNAME='dspdriverstatusdsp0reset' ,/ALIGN_LEFT ,/NO_RELEASE  $
      ,VALUE='DSP0 Reset')

  
  WID_BASE_225 = Widget_Base(WID_BASE_81, UNAME='WID_BASE_225'  $
      ,XOFFSET=3 ,YOFFSET=62 ,TITLE='IDL' ,COLUMN=1 ,/NONEXCLUSIVE)

  
  dspdriverstatusdsp1reset = Widget_Button(WID_BASE_225,  $
      UNAME='dspdriverstatusdsp1reset' ,/ALIGN_LEFT ,/NO_RELEASE  $
      ,VALUE='DSP1 Reset')

  
  WID_BASE_226 = Widget_Base(WID_BASE_81, UNAME='WID_BASE_226'  $
      ,XOFFSET=3 ,YOFFSET=84 ,TITLE='IDL' ,COLUMN=1 ,/NONEXCLUSIVE)

  
  dspdriverstatusflashreset = Widget_Button(WID_BASE_226,  $
      UNAME='dspdriverstatusflashreset' ,/ALIGN_LEFT ,/NO_RELEASE  $
      ,VALUE='Flash Reset')

  
  WID_BASE_227 = Widget_Base(WID_BASE_81, UNAME='WID_BASE_227'  $
      ,XOFFSET=3 ,YOFFSET=106 ,TITLE='IDL' ,COLUMN=1 ,/NONEXCLUSIVE)

  
  dspdriverstatusbuspowerfaultctrl = Widget_Button(WID_BASE_227,  $
      UNAME='dspdriverstatusbuspowerfaultctrl' ,/ALIGN_LEFT  $
      ,/NO_RELEASE ,VALUE='Bus Power Fault Ctrl')

  
  WID_BASE_228 = Widget_Base(WID_BASE_81, UNAME='WID_BASE_228'  $
      ,XOFFSET=3 ,YOFFSET=128 ,TITLE='IDL' ,COLUMN=1 ,/NONEXCLUSIVE)

  
  dspdriverstatusbuspowerfault = Widget_Button(WID_BASE_228,  $
      UNAME='dspdriverstatusbuspowerfault' ,/ALIGN_LEFT ,/NO_RELEASE  $
      ,VALUE='BUS Power Fault')

  
  WID_BASE_229 = Widget_Base(WID_BASE_81, UNAME='WID_BASE_229'  $
      ,XOFFSET=3 ,YOFFSET=150 ,TITLE='IDL' ,COLUMN=1 ,/NONEXCLUSIVE)

  
  dspdriverstatusbusdriverenable = Widget_Button(WID_BASE_229,  $
      UNAME='dspdriverstatusbusdriverenable' ,/ALIGN_LEFT  $
      ,/NO_RELEASE ,VALUE='BUS Driver Enable')

  
  WID_BASE_230 = Widget_Base(WID_BASE_81, UNAME='WID_BASE_230'  $
      ,XOFFSET=3 ,YOFFSET=172 ,TITLE='IDL' ,COLUMN=1 ,/NONEXCLUSIVE)

  
  dspdriverstatusenabledspwatchdog = Widget_Button(WID_BASE_230,  $
      UNAME='dspdriverstatusenabledspwatchdog' ,/ALIGN_LEFT  $
      ,/NO_RELEASE ,VALUE='Enable DSP Watchdog')

  
  WID_BASE_231 = Widget_Base(WID_BASE_81, UNAME='WID_BASE_231'  $
      ,XOFFSET=3 ,YOFFSET=194 ,TITLE='IDL' ,COLUMN=1 ,/NONEXCLUSIVE)

  
  dspdriverstatusdsp0watchdogexpired = Widget_Button(WID_BASE_231,  $
      UNAME='dspdriverstatusdsp0watchdogexpired' ,/ALIGN_LEFT  $
      ,/NO_RELEASE ,VALUE='DSP0 watchdog expired')

  
  WID_BASE_232 = Widget_Base(WID_BASE_81, UNAME='WID_BASE_232'  $
      ,XOFFSET=3 ,YOFFSET=216 ,TITLE='IDL' ,COLUMN=1 ,/NONEXCLUSIVE)

  
  dspdriverstatusdsp1watchdogexpired = Widget_Button(WID_BASE_232,  $
      UNAME='dspdriverstatusdsp1watchdogexpired' ,/ALIGN_LEFT  $
      ,/NO_RELEASE ,VALUE='DSP1 watchdog expired')

  
  WID_BASE_233 = Widget_Base(WID_BASE_81, UNAME='WID_BASE_233'  $
      ,FRAME=1 ,XOFFSET=3 ,YOFFSET=238 ,TITLE='IDL' ,XPAD=3 ,YPAD=3  $
      ,COLUMN=1)

  
  WID_LABEL_27 = Widget_Label(WID_BASE_233, UNAME='WID_LABEL_27'  $
      ,XOFFSET=3 ,YOFFSET=3 ,/ALIGN_LEFT ,VALUE='Driver Enabled')

  
  WID_BASE_234 = Widget_Base(WID_BASE_233, UNAME='WID_BASE_234'  $
      ,XOFFSET=3 ,YOFFSET=18 ,TITLE='IDL' ,XPAD=3 ,YPAD=3 ,ROW=1)

  
  WID_BASE_235 = Widget_Base(WID_BASE_234, UNAME='WID_BASE_235'  $
      ,XOFFSET=3 ,YOFFSET=3 ,TITLE='IDL' ,XPAD=3 ,YPAD=3 ,COLUMN=1)

  
  WID_BASE_236 = Widget_Base(WID_BASE_235, UNAME='WID_BASE_236'  $
      ,XOFFSET=3 ,YOFFSET=3 ,TITLE='IDL' ,COLUMN=1 ,/NONEXCLUSIVE)

  
  dspdriverstatusdriverenable0 = Widget_Button(WID_BASE_236,  $
      UNAME='dspdriverstatusdriverenable0' ,/ALIGN_LEFT ,VALUE='ch0')

  
  WID_BASE_237 = Widget_Base(WID_BASE_235, UNAME='WID_BASE_237'  $
      ,XOFFSET=3 ,YOFFSET=25 ,TITLE='IDL' ,COLUMN=1 ,/NONEXCLUSIVE)

  
  dspdriverstatusdriverenable2 = Widget_Button(WID_BASE_237,  $
      UNAME='dspdriverstatusdriverenable2' ,/ALIGN_LEFT ,VALUE='ch2')

  
  WID_BASE_238 = Widget_Base(WID_BASE_235, UNAME='WID_BASE_238'  $
      ,XOFFSET=3 ,YOFFSET=47 ,TITLE='IDL' ,COLUMN=1 ,/NONEXCLUSIVE)

  
  dspdriverstatusdriverenable4 = Widget_Button(WID_BASE_238,  $
      UNAME='dspdriverstatusdriverenable4' ,/ALIGN_LEFT ,VALUE='ch4')

  
  WID_BASE_239 = Widget_Base(WID_BASE_235, UNAME='WID_BASE_239'  $
      ,XOFFSET=3 ,YOFFSET=69 ,TITLE='IDL' ,COLUMN=1 ,/NONEXCLUSIVE)

  
  dspdriverstatusdriverenable6 = Widget_Button(WID_BASE_239,  $
      UNAME='dspdriverstatusdriverenable6' ,/ALIGN_LEFT ,VALUE='ch6')

  
  WID_BASE_240 = Widget_Base(WID_BASE_234, UNAME='WID_BASE_240'  $
      ,XOFFSET=55 ,YOFFSET=3 ,TITLE='IDL' ,XPAD=3 ,YPAD=3 ,COLUMN=1)

  
  WID_BASE_241 = Widget_Base(WID_BASE_240, UNAME='WID_BASE_241'  $
      ,XOFFSET=3 ,YOFFSET=3 ,TITLE='IDL' ,COLUMN=1 ,/NONEXCLUSIVE)

  
  dspdriverstatusdriverenable1 = Widget_Button(WID_BASE_241,  $
      UNAME='dspdriverstatusdriverenable1' ,/ALIGN_LEFT ,VALUE='ch1')

  
  WID_BASE_242 = Widget_Base(WID_BASE_240, UNAME='WID_BASE_242'  $
      ,XOFFSET=3 ,YOFFSET=25 ,TITLE='IDL' ,COLUMN=1 ,/NONEXCLUSIVE)

  
  dspdriverstatusdriverenable3 = Widget_Button(WID_BASE_242,  $
      UNAME='dspdriverstatusdriverenable3' ,/ALIGN_LEFT ,VALUE='ch3')

  
  WID_BASE_243 = Widget_Base(WID_BASE_240, UNAME='WID_BASE_243'  $
      ,XOFFSET=3 ,YOFFSET=47 ,TITLE='IDL' ,COLUMN=1 ,/NONEXCLUSIVE)

  
  dspdriverstatusdriverenable5 = Widget_Button(WID_BASE_243,  $
      UNAME='dspdriverstatusdriverenable5' ,/ALIGN_LEFT ,VALUE='ch5')

  
  WID_BASE_244 = Widget_Base(WID_BASE_240, UNAME='WID_BASE_244'  $
      ,XOFFSET=3 ,YOFFSET=69 ,TITLE='IDL' ,COLUMN=1 ,/NONEXCLUSIVE)

  
  dspdriverstatusdriverenable7 = Widget_Button(WID_BASE_244,  $
      UNAME='dspdriverstatusdriverenable7' ,/ALIGN_LEFT ,VALUE='ch7')

  
  WID_BASE_5 = Widget_Base(CRATE_DSP_BASE, UNAME='WID_BASE_5'  $
      ,XOFFSET=3 ,YOFFSET=451 ,TITLE='IDL' ,XPAD=3 ,YPAD=3 ,ROW=1)

  
  dsp_board_num = Widget_Slider(WID_BASE_5, UNAME='dsp_board_num'  $
      ,XOFFSET=3 ,YOFFSET=3 ,TITLE='DSP Bosrd (0-83)' ,MAXIMUM=83)

  
  CRATE_SIGGEN_BASE = Widget_Base(WID_TAB_0,  $
      UNAME='CRATE_SIGGEN_BASE' ,SCR_XSIZE=887 ,SCR_YSIZE=641  $
      ,TITLE='Crate SIGGEN' ,SPACE=3 ,XPAD=3 ,YPAD=3 ,COLUMN=1)

  
  WID_BASE_54 = Widget_Base(CRATE_SIGGEN_BASE, UNAME='WID_BASE_54'  $
      ,FRAME=1 ,XOFFSET=3 ,YOFFSET=3 ,/BASE_ALIGN_RIGHT ,TITLE='IDL'  $
      ,SPACE=3 ,XPAD=3 ,YPAD=3 ,COLUMN=1)

  
  WID_LABEL_84 = Widget_Label(WID_BASE_54, UNAME='WID_LABEL_84'  $
      ,XOFFSET=163 ,YOFFSET=3 ,/ALIGN_RIGHT ,VALUE='NIOS Fixed Area')

  
  WID_BASE_55 = Widget_Base(WID_BASE_54, UNAME='WID_BASE_55'  $
      ,XOFFSET=43 ,YOFFSET=21 ,TITLE='IDL' ,SPACE=3 ,XPAD=3 ,YPAD=3  $
      ,ROW=1)

  
  WID_LABEL_85 = Widget_Label(WID_BASE_55, UNAME='WID_LABEL_85'  $
      ,XOFFSET=3 ,YOFFSET=3 ,/ALIGN_LEFT ,VALUE='WhoAmI')

  
  sgnwhoami = Widget_Text(WID_BASE_55, UNAME='sgnwhoami' ,XOFFSET=49  $
      ,YOFFSET=3 ,XSIZE=20 ,YSIZE=1)

  
  sgnwhoami_ = Widget_Button(WID_BASE_55, UNAME='sgnwhoami_'  $
      ,XOFFSET=176 ,YOFFSET=3 ,/ALIGN_CENTER ,VALUE='ok.bmp'  $
      ,/BITMAP)

  
  WID_BASE_56 = Widget_Base(WID_BASE_54, UNAME='WID_BASE_56'  $
      ,XOFFSET=3 ,YOFFSET=51 ,TITLE='IDL' ,SPACE=3 ,XPAD=3 ,YPAD=3  $
      ,ROW=1)

  
  WID_LABEL_86 = Widget_Label(WID_BASE_56, UNAME='WID_LABEL_86'  $
      ,XOFFSET=3 ,YOFFSET=3 ,/ALIGN_LEFT ,VALUE='SoftwareRelease')

  
  sgnsoftwarerelease = Widget_Text(WID_BASE_56,  $
      UNAME='sgnsoftwarerelease' ,XOFFSET=89 ,YOFFSET=3 ,XSIZE=20  $
      ,YSIZE=1)

  
  sgnsoftwarerelease_ = Widget_Button(WID_BASE_56,  $
      UNAME='sgnsoftwarerelease_' ,XOFFSET=216 ,YOFFSET=3  $
      ,/ALIGN_CENTER ,VALUE='ok.bmp' ,/BITMAP)

  
  WID_BASE_65 = Widget_Base(WID_BASE_54, UNAME='WID_BASE_65'  $
      ,XOFFSET=19 ,YOFFSET=81 ,TITLE='IDL' ,SPACE=3 ,XPAD=3 ,YPAD=3  $
      ,ROW=1)

  
  WID_LABEL_87 = Widget_Label(WID_BASE_65, UNAME='WID_LABEL_87'  $
      ,XOFFSET=3 ,YOFFSET=3 ,/ALIGN_LEFT ,VALUE='LogicRelease')

  
  sgnlogicrelease = Widget_Text(WID_BASE_65, UNAME='sgnlogicrelease'  $
      ,XOFFSET=73 ,YOFFSET=3 ,XSIZE=20 ,YSIZE=1)

  
  sgnlogicrelease_ = Widget_Button(WID_BASE_65,  $
      UNAME='sgnlogicrelease_' ,XOFFSET=200 ,YOFFSET=3 ,/ALIGN_CENTER  $
      ,VALUE='ok.bmp' ,/BITMAP)

  
  WID_BASE_67 = Widget_Base(WID_BASE_54, UNAME='WID_BASE_67'  $
      ,XOFFSET=21 ,YOFFSET=111 ,TITLE='IDL' ,SPACE=3 ,XPAD=3 ,YPAD=3  $
      ,ROW=1)

  
  WID_LABEL_88 = Widget_Label(WID_BASE_67, UNAME='WID_LABEL_88'  $
      ,XOFFSET=3 ,YOFFSET=3 ,/ALIGN_LEFT ,VALUE='SerialNumber')

  
  sgnserialnumber = Widget_Text(WID_BASE_67, UNAME='sgnserialnumber'  $
      ,XOFFSET=71 ,YOFFSET=3 ,XSIZE=20 ,YSIZE=1)

  
  sgnserialnumber_ = Widget_Button(WID_BASE_67,  $
      UNAME='sgnserialnumber_' ,XOFFSET=198 ,YOFFSET=3 ,/ALIGN_CENTER  $
      ,VALUE='ok.bmp' ,/BITMAP)

  
  WID_BASE_68 = Widget_Base(CRATE_SIGGEN_BASE, UNAME='WID_BASE_68'  $
      ,FRAME=1 ,XOFFSET=3 ,YOFFSET=147 ,TITLE='IDL' ,SPACE=3 ,XPAD=3  $
      ,YPAD=3 ,COLUMN=1)

  
  WID_LABEL_21 = Widget_Label(WID_BASE_68, UNAME='WID_LABEL_21'  $
      ,XOFFSET=126 ,YOFFSET=3 ,/ALIGN_RIGHT ,VALUE='Temperatures')

  
  WID_BASE_200 = Widget_Base(WID_BASE_68, UNAME='WID_BASE_200'  $
      ,XOFFSET=3 ,YOFFSET=21 ,TITLE='IDL' ,SPACE=3 ,XPAD=3 ,YPAD=3  $
      ,ROW=1)

  
  WID_LABEL_258 = Widget_Label(WID_BASE_200, UNAME='WID_LABEL_258'  $
      ,XOFFSET=3 ,YOFFSET=3 ,/ALIGN_LEFT ,VALUE='Stratix')

  
  sgnstratixtemp = Widget_Text(WID_BASE_200, UNAME='sgnstratixtemp'  $
      ,XOFFSET=37 ,YOFFSET=3 ,XSIZE=20 ,YSIZE=1)

  
  sgnstratixtemp_ = Widget_Button(WID_BASE_200,  $
      UNAME='sgnstratixtemp_' ,XOFFSET=164 ,YOFFSET=3 ,/ALIGN_CENTER  $
      ,VALUE='ok.bmp' ,/BITMAP)

  
  WID_BASE_247 = Widget_Base(WID_BASE_68, UNAME='WID_BASE_247'  $
      ,XOFFSET=3 ,YOFFSET=51 ,TITLE='IDL' ,SPACE=3 ,XPAD=3 ,YPAD=3  $
      ,ROW=1)

  
  WID_LABEL_287 = Widget_Label(WID_BASE_247, UNAME='WID_LABEL_287'  $
      ,XOFFSET=3 ,YOFFSET=3 ,/ALIGN_LEFT ,VALUE='Power')

  
  sgnpowertemp = Widget_Text(WID_BASE_247, UNAME='sgnpowertemp'  $
      ,XOFFSET=38 ,YOFFSET=3 ,XSIZE=20 ,YSIZE=1)

  
  sgnpowertemp_ = Widget_Button(WID_BASE_247, UNAME='sgnpowertemp_'  $
      ,XOFFSET=165 ,YOFFSET=3 ,/ALIGN_CENTER ,VALUE='ok.bmp'  $
      ,/BITMAP)

  
  WID_BASE_249 = Widget_Base(WID_BASE_68, UNAME='WID_BASE_249'  $
      ,XOFFSET=3 ,YOFFSET=81 ,TITLE='IDL' ,SPACE=3 ,XPAD=3 ,YPAD=3  $
      ,ROW=1)

  
  WID_LABEL_289 = Widget_Label(WID_BASE_249, UNAME='WID_LABEL_289'  $
      ,XOFFSET=3 ,YOFFSET=3 ,/ALIGN_LEFT ,VALUE='DSPs')

  
  sgndspstemp = Widget_Text(WID_BASE_249, UNAME='sgndspstemp'  $
      ,XOFFSET=35 ,YOFFSET=3 ,XSIZE=20 ,YSIZE=1)

  
  sgndspstemp_ = Widget_Button(WID_BASE_249, UNAME='sgndspstemp_'  $
      ,XOFFSET=162 ,YOFFSET=3 ,/ALIGN_CENTER ,VALUE='ok.bmp'  $
      ,/BITMAP)

  
  WID_BASE_9 = Widget_Base(CRATE_SIGGEN_BASE, UNAME='WID_BASE_9'  $
      ,XOFFSET=3 ,YOFFSET=261 ,TITLE='IDL' ,SPACE=3 ,XPAD=3 ,YPAD=3  $
      ,ROW=1)

  
  sgn_crate_num = Widget_Slider(WID_BASE_9, UNAME='sgn_crate_num'  $
      ,XOFFSET=3 ,YOFFSET=3 ,TITLE='Crate (0-5)' ,MAXIMUM=5)

  
  SWITCH_BCU_BASE = Widget_Base(WID_TAB_0, UNAME='SWITCH_BCU_BASE'  $
      ,SCR_XSIZE=887 ,SCR_YSIZE=641 ,TITLE='Switch BCU' ,SPACE=3  $
      ,XPAD=3 ,YPAD=3 ,ROW=1)

  
  WID_BASE_66 = Widget_Base(SWITCH_BCU_BASE, UNAME='WID_BASE_66'  $
      ,FRAME=1 ,XOFFSET=3 ,YOFFSET=3 ,/BASE_ALIGN_RIGHT ,TITLE='IDL'  $
      ,SPACE=3 ,XPAD=3 ,YPAD=3 ,COLUMN=1)

  
  WID_LABEL_139 = Widget_Label(WID_BASE_66, UNAME='WID_LABEL_139'  $
      ,XOFFSET=197 ,YOFFSET=3 ,/ALIGN_RIGHT ,VALUE='NIOS Fixed Area')

  
  WID_BASE_71 = Widget_Base(WID_BASE_66, UNAME='WID_BASE_71'  $
      ,XOFFSET=82 ,YOFFSET=21 ,TITLE='IDL' ,SPACE=3 ,XPAD=3 ,YPAD=3  $
      ,ROW=1)

  
  WID_LABEL_140 = Widget_Label(WID_BASE_71, UNAME='WID_LABEL_140'  $
      ,XOFFSET=3 ,YOFFSET=3 ,/ALIGN_LEFT ,VALUE='CrateID')

  
  switchcrateid = Widget_Text(WID_BASE_71, UNAME='switchcrateid'  $
      ,XOFFSET=44 ,YOFFSET=3 ,XSIZE=20 ,YSIZE=1)

  
  switchcrateid_ = Widget_Button(WID_BASE_71, UNAME='switchcrateid_'  $
      ,XOFFSET=171 ,YOFFSET=3 ,/ALIGN_CENTER ,VALUE='ok.bmp'  $
      ,/BITMAP)

  
  WID_BASE_72 = Widget_Base(WID_BASE_66, UNAME='WID_BASE_72'  $
      ,XOFFSET=77 ,YOFFSET=51 ,TITLE='IDL' ,SPACE=3 ,XPAD=3 ,YPAD=3  $
      ,ROW=1)

  
  WID_LABEL_141 = Widget_Label(WID_BASE_72, UNAME='WID_LABEL_141'  $
      ,XOFFSET=3 ,YOFFSET=3 ,/ALIGN_LEFT ,VALUE='WhoAmI')

  
  switchwhoami = Widget_Text(WID_BASE_72, UNAME='switchwhoami'  $
      ,XOFFSET=49 ,YOFFSET=3 ,XSIZE=20 ,YSIZE=1)

  
  switchwhoami_ = Widget_Button(WID_BASE_72, UNAME='switchwhoami_'  $
      ,XOFFSET=176 ,YOFFSET=3 ,/ALIGN_CENTER ,VALUE='ok.bmp'  $
      ,/BITMAP)

  
  WID_BASE_98 = Widget_Base(WID_BASE_66, UNAME='WID_BASE_98'  $
      ,XOFFSET=37 ,YOFFSET=81 ,TITLE='IDL' ,SPACE=3 ,XPAD=3 ,YPAD=3  $
      ,ROW=1)

  
  WID_LABEL_142 = Widget_Label(WID_BASE_98, UNAME='WID_LABEL_142'  $
      ,XOFFSET=3 ,YOFFSET=3 ,/ALIGN_LEFT ,VALUE='SoftwareRelease')

  
  switchsoftwarerelease = Widget_Text(WID_BASE_98,  $
      UNAME='switchsoftwarerelease' ,XOFFSET=89 ,YOFFSET=3 ,XSIZE=20  $
      ,YSIZE=1)

  
  switchsoftwarerelease_ = Widget_Button(WID_BASE_98,  $
      UNAME='switchsoftwarerelease_' ,XOFFSET=216 ,YOFFSET=3  $
      ,/ALIGN_CENTER ,VALUE='ok.bmp' ,/BITMAP)

  
  WID_BASE_99 = Widget_Base(WID_BASE_66, UNAME='WID_BASE_99'  $
      ,XOFFSET=53 ,YOFFSET=111 ,TITLE='IDL' ,SPACE=3 ,XPAD=3 ,YPAD=3  $
      ,ROW=1)

  
  WID_LABEL_143 = Widget_Label(WID_BASE_99, UNAME='WID_LABEL_143'  $
      ,XOFFSET=3 ,YOFFSET=3 ,/ALIGN_LEFT ,VALUE='LogicRelease')

  
  switchlogicrelease = Widget_Text(WID_BASE_99,  $
      UNAME='switchlogicrelease' ,XOFFSET=73 ,YOFFSET=3 ,XSIZE=20  $
      ,YSIZE=1)

  
  switchlogicrelease_ = Widget_Button(WID_BASE_99,  $
      UNAME='switchlogicrelease_' ,XOFFSET=200 ,YOFFSET=3  $
      ,/ALIGN_CENTER ,VALUE='ok.bmp' ,/BITMAP)

  
  WID_BASE_100 = Widget_Base(WID_BASE_66, UNAME='WID_BASE_100'  $
      ,XOFFSET=70 ,YOFFSET=141 ,TITLE='IDL' ,SPACE=3 ,XPAD=3 ,YPAD=3  $
      ,ROW=1)

  
  WID_LABEL_144 = Widget_Label(WID_BASE_100, UNAME='WID_LABEL_144'  $
      ,XOFFSET=3 ,YOFFSET=3 ,/ALIGN_LEFT ,VALUE='IPAddress')

  
  switchipaddress = Widget_Text(WID_BASE_100, UNAME='switchipaddress'  $
      ,XOFFSET=56 ,YOFFSET=3 ,XSIZE=20 ,YSIZE=1)

  
  switchipaddress_ = Widget_Button(WID_BASE_100,  $
      UNAME='switchipaddress_' ,XOFFSET=183 ,YOFFSET=3 ,/ALIGN_CENTER  $
      ,VALUE='ok.bmp' ,/BITMAP)

  
  WID_BASE_101 = Widget_Base(WID_BASE_66, UNAME='WID_BASE_101'  $
      ,XOFFSET=47 ,YOFFSET=171 ,TITLE='IDL' ,SPACE=3 ,XPAD=3 ,YPAD=3  $
      ,ROW=1)

  
  WID_LABEL_145 = Widget_Label(WID_BASE_101, UNAME='WID_LABEL_145'  $
      ,XOFFSET=3 ,YOFFSET=3 ,/ALIGN_LEFT ,VALUE='FramesCounter')

  
  switchframescounter = Widget_Text(WID_BASE_101,  $
      UNAME='switchframescounter' ,XOFFSET=79 ,YOFFSET=3 ,XSIZE=20  $
      ,YSIZE=1)

  
  switchframescounter_ = Widget_Button(WID_BASE_101,  $
      UNAME='switchframescounter_' ,XOFFSET=206 ,YOFFSET=3  $
      ,/ALIGN_CENTER ,VALUE='ok.bmp' ,/BITMAP)

  
  WID_BASE_102 = Widget_Base(WID_BASE_66, UNAME='WID_BASE_102'  $
      ,XOFFSET=55 ,YOFFSET=201 ,TITLE='IDL' ,SPACE=3 ,XPAD=3 ,YPAD=3  $
      ,ROW=1)

  
  WID_LABEL_146 = Widget_Label(WID_BASE_102, UNAME='WID_LABEL_146'  $
      ,XOFFSET=3 ,YOFFSET=3 ,/ALIGN_LEFT ,VALUE='SerialNumber')

  
  switchserialnumber = Widget_Text(WID_BASE_102,  $
      UNAME='switchserialnumber' ,XOFFSET=71 ,YOFFSET=3 ,XSIZE=20  $
      ,YSIZE=1)

  
  switchserialnumber_ = Widget_Button(WID_BASE_102,  $
      UNAME='switchserialnumber_' ,XOFFSET=198 ,YOFFSET=3  $
      ,/ALIGN_CENTER ,VALUE='ok.bmp' ,/BITMAP)

  
  WID_BASE_103 = Widget_Base(WID_BASE_66, UNAME='WID_BASE_103'  $
      ,XOFFSET=11 ,YOFFSET=231 ,TITLE='IDL' ,SPACE=3 ,XPAD=3 ,YPAD=3  $
      ,ROW=1)

  
  WID_LABEL_147 = Widget_Label(WID_BASE_103, UNAME='WID_LABEL_147'  $
      ,XOFFSET=3 ,YOFFSET=3 ,/ALIGN_LEFT  $
      ,VALUE='PowerBackplaneSerial')

  
  switchpowerbackplaneserial = Widget_Text(WID_BASE_103,  $
      UNAME='switchpowerbackplaneserial' ,XOFFSET=115 ,YOFFSET=3  $
      ,XSIZE=20 ,YSIZE=1)

  
  switchpowerbackplaneserial_ = Widget_Button(WID_BASE_103,  $
      UNAME='switchpowerbackplaneserial_' ,XOFFSET=242 ,YOFFSET=3  $
      ,/ALIGN_CENTER ,VALUE='ok.bmp' ,/BITMAP)

  
  WID_BASE_104 = Widget_Base(WID_BASE_66, UNAME='WID_BASE_104'  $
      ,XOFFSET=3 ,YOFFSET=261 ,TITLE='IDL' ,SPACE=3 ,XPAD=3 ,YPAD=3  $
      ,ROW=1)

  
  WID_LABEL_148 = Widget_Label(WID_BASE_104, UNAME='WID_LABEL_148'  $
      ,XOFFSET=3 ,YOFFSET=3 ,/ALIGN_LEFT  $
      ,VALUE='EnableMasterDiagnostic')

  
  switchenablemasterdiagnostic = Widget_Text(WID_BASE_104,  $
      UNAME='switchenablemasterdiagnostic' ,XOFFSET=123 ,YOFFSET=3  $
      ,XSIZE=20 ,YSIZE=1)

  
  switchenablemasterdiagnostic_ = Widget_Button(WID_BASE_104,  $
      UNAME='switchenablemasterdiagnostic_' ,XOFFSET=250 ,YOFFSET=3  $
      ,/ALIGN_CENTER ,VALUE='ok.bmp' ,/BITMAP)

  
  WID_BASE_105 = Widget_Base(WID_BASE_66, UNAME='WID_BASE_105'  $
      ,XOFFSET=35 ,YOFFSET=291 ,TITLE='IDL' ,SPACE=3 ,XPAD=3 ,YPAD=3  $
      ,ROW=1)

  
  WID_LABEL_149 = Widget_Label(WID_BASE_105, UNAME='WID_LABEL_149'  $
      ,XOFFSET=3 ,YOFFSET=3 ,/ALIGN_LEFT ,VALUE='DecimationFactor')

  
  switchdecimationfactor = Widget_Text(WID_BASE_105,  $
      UNAME='switchdecimationfactor' ,XOFFSET=91 ,YOFFSET=3 ,XSIZE=20  $
      ,YSIZE=1)

  
  switchdecimationfactor_ = Widget_Button(WID_BASE_105,  $
      UNAME='switchdecimationfactor_' ,XOFFSET=218 ,YOFFSET=3  $
      ,/ALIGN_CENTER ,VALUE='ok.bmp' ,/BITMAP)

  
  WID_BASE_106 = Widget_Base(WID_BASE_66, UNAME='WID_BASE_106'  $
      ,XOFFSET=33 ,YOFFSET=321 ,TITLE='IDL' ,SPACE=3 ,XPAD=3 ,YPAD=3  $
      ,ROW=1)

  
  WID_LABEL_150 = Widget_Label(WID_BASE_106, UNAME='WID_LABEL_150'  $
      ,XOFFSET=3 ,YOFFSET=3 ,/ALIGN_LEFT ,VALUE='RemoteIPAddress')

  
  switchremoteipaddress = Widget_Text(WID_BASE_106,  $
      UNAME='switchremoteipaddress' ,XOFFSET=93 ,YOFFSET=3 ,XSIZE=20  $
      ,YSIZE=1)

  
  switchremoteipaddress_ = Widget_Button(WID_BASE_106,  $
      UNAME='switchremoteipaddress_' ,XOFFSET=220 ,YOFFSET=3  $
      ,/ALIGN_CENTER ,VALUE='ok.bmp' ,/BITMAP)

  
  WID_BASE_76 = Widget_Base(SWITCH_BCU_BASE, UNAME='WID_BASE_76'  $
      ,FRAME=1 ,XOFFSET=287 ,YOFFSET=3 ,TITLE='IDL' ,SPACE=3 ,XPAD=3  $
      ,YPAD=3 ,COLUMN=1)

  
  WID_LABEL_29 = Widget_Label(WID_BASE_76, UNAME='WID_LABEL_29'  $
      ,XOFFSET=126 ,YOFFSET=3 ,/ALIGN_RIGHT ,VALUE='Temperatures')

  
  WID_BASE_245 = Widget_Base(WID_BASE_76, UNAME='WID_BASE_245'  $
      ,XOFFSET=3 ,YOFFSET=21 ,TITLE='IDL' ,SPACE=3 ,XPAD=3 ,YPAD=3  $
      ,ROW=1)

  
  WID_LABEL_285 = Widget_Label(WID_BASE_245, UNAME='WID_LABEL_285'  $
      ,XOFFSET=3 ,YOFFSET=3 ,/ALIGN_LEFT ,VALUE='Stratix')

  
  switchstratixtemp = Widget_Text(WID_BASE_245,  $
      UNAME='switchstratixtemp' ,XOFFSET=37 ,YOFFSET=3 ,XSIZE=20  $
      ,YSIZE=1)

  
  switchstratixtemp_ = Widget_Button(WID_BASE_245,  $
      UNAME='switchstratixtemp_' ,XOFFSET=164 ,YOFFSET=3  $
      ,/ALIGN_CENTER ,VALUE='ok.bmp' ,/BITMAP)

  
  WID_BASE_246 = Widget_Base(WID_BASE_76, UNAME='WID_BASE_246'  $
      ,XOFFSET=3 ,YOFFSET=51 ,TITLE='IDL' ,SPACE=3 ,XPAD=3 ,YPAD=3  $
      ,ROW=1)

  
  WID_LABEL_286 = Widget_Label(WID_BASE_246, UNAME='WID_LABEL_286'  $
      ,XOFFSET=3 ,YOFFSET=3 ,/ALIGN_LEFT ,VALUE='Power')

  
  switchpowertemp = Widget_Text(WID_BASE_246, UNAME='switchpowertemp'  $
      ,XOFFSET=38 ,YOFFSET=3 ,XSIZE=20 ,YSIZE=1)

  
  switchpowertemp_ = Widget_Button(WID_BASE_246,  $
      UNAME='switchpowertemp_' ,XOFFSET=165 ,YOFFSET=3 ,/ALIGN_CENTER  $
      ,VALUE='ok.bmp' ,/BITMAP)

  Widget_Control, /REALIZE, HOUSEKEEPING_BASE

  XManager, 'HOUSEKEEPING_BASE', HOUSEKEEPING_BASE, /NO_BLOCK  

end
; 
; Empty stub procedure used for autoloading.
; 
pro housekeeping, GROUP_LEADER=wGroup, _EXTRA=_VWBExtra_
  HOUSEKEEPING_BASE, GROUP_LEADER=wGroup, _EXTRA=_VWBExtra_
end
