
;+
;
; KEYWORDS
; 
; list        return the list of variables name that can be queried (strarr)
; index       if varianble is an array (e.g. ChDistAverage) , select index
; from        select on timestamp. (string, format='20091026_124256')
; to          select on timestamp. (string, format='20091026_124256')
; noplot      self-explaining
;-
function telemetry, variable_name, index=index, from=from, to=to,  jtime=time $
                  , plot_do=plot_do, list=list, filename=filename, NOW=now, JFROM=jfrom, JTO=jto

    plist_file = getenv("ADOPT_ROOT")+'/conf/adsec/current/processConf/processList.conf'
    err = get_process_list(plist_file, OUT=process_list)
    
    ;LUT FOR MANAGING TELEMETRY DATA BEFORE 2011-04-27
    varstruct_old = { $
                 SwitchMirrFrames: process_list.housekeeper.msgd_name , $
                 ParamBlockFastLi: process_list.fastdiagn.msgd_name , $
                 ParamBlockDistur: process_list.fastdiagn.msgd_name , $
                 SwitchStratixTem: process_list.housekeeper.msgd_name , $
                 SwitchSafeSkipFr: process_list.housekeeper.msgd_name , $
                 SwitchPendingSki: process_list.housekeeper.msgd_name , $
                 SwitchNumFLTimeo: process_list.housekeeper.msgd_name , $
                 ExternalTemperat: process_list.housekeeper.msgd_name , $
                 WaterColdplateIn: process_list.housekeeper.msgd_name , $
                 WaterColdplateOu: process_list.housekeeper.msgd_name  $
    }

    varstruct = {Offloadmodes: process_list.fastdiagn.msgd_name ,$
                 HighOrderOffload: process_list.fastdiagn.msgd_name , $
                 ParamBlockFastLink: process_list.fastdiagn.msgd_name , $
                 ParamBlockDisturbEnabled: process_list.fastdiagn.msgd_name , $
                 ChCurrAverage: process_list.fastdiagn.msgd_name , $
                 ChDistAverage: process_list.fastdiagn.msgd_name , $
                 ChCurrRMS: process_list.fastdiagn.msgd_name , $
                 ChDistRMS: process_list.fastdiagn.msgd_name , $
                 PosPiston: process_list.fastdiagn.msgd_name , $
                 CurrPiston: process_list.fastdiagn.msgd_name , $
                 BCUCurrentVCCA: process_list.housekeeper.msgd_name , $
                 BCUCurrentVCCL: process_list.housekeeper.msgd_name , $
                 BCUCurrentVCCP: process_list.housekeeper.msgd_name , $
                 BCUCurrentVSSA: process_list.housekeeper.msgd_name , $
                 BCUCurrentVSSP: process_list.housekeeper.msgd_name , $
                 BCUVoltageVCCA: process_list.housekeeper.msgd_name , $
                 BCUVoltageVCCL: process_list.housekeeper.msgd_name , $
                 BCUVoltageVCCP: process_list.housekeeper.msgd_name , $
                 BCUVoltageVSSA: process_list.housekeeper.msgd_name , $
                 BCUVoltageVSSP: process_list.housekeeper.msgd_name , $
                 SGNStratixTemp: process_list.housekeeper.msgd_name , $
                 DSPStratixTemp: process_list.housekeeper.msgd_name , $
                 SwitchStratixTemp: process_list.housekeeper.msgd_name , $
                 SwitchPowerTemp: process_list.housekeeper.msgd_name , $
                 BCUStratixTemp: process_list.housekeeper.msgd_name , $
                 BCUPowerTemp: process_list.housekeeper.msgd_name , $
                 SwitchMirrFramesCounter: process_list.housekeeper.msgd_name , $
                 SwitchSafeSkipFrameCnt: process_list.housekeeper.msgd_name , $
                 SwitchPendingSkipFrameCnt: process_list.housekeeper.msgd_name , $
                 SwitchNumFLTimeoout: process_list.housekeeper.msgd_name , $
                 SwitchNumCrcErr: process_list.housekeeper.msgd_name , $
                 ExternalTemperature: process_list.housekeeper.msgd_name , $
                 ExternalHumidity: process_list.housekeeper.msgd_name , $
                 DewPoint: process_list.housekeeper.msgd_name , $
                 CheckDewPoint: process_list.housekeeper.msgd_name , $
                 WaterColdplateInlet: process_list.housekeeper.msgd_name , $
                 WaterColdplateOutlet: process_list.housekeeper.msgd_name , $
                 WaterMainInlet:  process_list.housekeeper.msgd_name , $
                 WaterMainOutlet:  process_list.housekeeper.msgd_name , $
                 FluxRateIn: process_list.housekeeper.msgd_name , $
                 HubTemp: process_list.housekeeper.msgd_name , $
                 PBPlaneTemp: process_list.housekeeper.msgd_name , $
                 CPTemp: process_list.housekeeper.msgd_name , $
                 RBodyTemp: process_list.housekeeper.msgd_name , $
                 InnerStrTemp: process_list.housekeeper.msgd_name , $
                 MeanDSPCoilCurrent: process_list.housekeeper.msgd_name , $
                 ELEVATION:process_list.varsmonitor.msgd_name , $
                 WINDSPEED:process_list.varsmonitor.msgd_name ,  $ 
                 EXTWINDSPEED:process_list.varsmonitor.msgd_name ,  $ 
                 EXTWINDDIR:process_list.varsmonitor.msgd_name ,  $ 
                 DIMMSEEING:process_list.varsmonitor.msgd_name,   $ 
                 AZIMUTH:process_list.varsmonitor.msgd_name,   $ 
                 AdamBootSelectN:process_list.adamhousekeeper.msgd_name, $
                 AdamDriverEnable:process_list.adamhousekeeper.msgd_name, $
                 AdamFPGAClearN:process_list.adamhousekeeper.msgd_name, $
                 AdamFrameCounter:process_list.adamhousekeeper.msgd_name, $
                 AdamMainPower:process_list.adamhousekeeper.msgd_name, $
                 AdamSysResetN:process_list.adamhousekeeper.msgd_name, $
                 AdamTCSPowerFaultN0:process_list.adamhousekeeper.msgd_name, $
                 AdamTCSPowerFaultN1:process_list.adamhousekeeper.msgd_name, $
                 AdamTCSPowerFaultN2:process_list.adamhousekeeper.msgd_name, $
                 AdamTCSSysFault:process_list.adamhousekeeper.msgd_name, $
                 AdamTSSDisable:process_list.adamhousekeeper.msgd_name, $
                 AdamTSSFaultN:process_list.adamhousekeeper.msgd_name, $
                 AdamTSSPowerFaultN0:process_list.adamhousekeeper.msgd_name, $
                 AdamTSSPowerFaultN1:process_list.adamhousekeeper.msgd_name, $
                 AdamWatchdogExp:process_list.adamhousekeeper.msgd_name $
                }
     
    variables = tag_names(varstruct)
    variables_old = tag_names(varstruct_old)
    diagnapp = strarr(n_tags(varstruct))
    diagnapp_old = strarr(n_tags(varstruct_old))
    for i=0, n_tags(varstruct)-1 do diagnapp[i] = varstruct.(i)
    for i=0, n_tags(varstruct_old)-1 do diagnapp_old[i] = varstruct_old.(i)
    if n_elements(from) eq 0 and n_elements(to) eq 0 and ~keyword_set(NOW) and ~keyword_set(list) and n_elements(jfrom) eq 0 and n_elements(jto) then begin
        print, "telemetry.pro USAGE:  data = telemetry('varname', [FROM='YYYYMMDD_HHMMSS', TO='YYYYMMDD_HHMMSS' | /NOW])"
        print, "if FROM is missing, the start time for data collecting will be set about 2h before the TO request"
        print, "if TO   is missing, the stop  time for data collecting will be set at the telemetry request"
        print, "if NOW is set as keyword, FROM and TO parameters will be ignored and the last data will be provided."
        print, "HELP TO BE UPDATED"
;        return, -1
    endif

;    diagnapp  =    [replicate(process_list.fastdiagn.msgd_name, 9), replicate(process_list.housekeeper.msgd_name,34), replicate('varsmonitor.L',2)]

    if n_elements(list) ne 0 then begin
        return, variables
    endif

    idxold = where(strmatch(variables_old, variable_name, /FOLD_CASE) eq 1, cnt)
    if cnt ne 0 then begin
        print, "You selected and old-style variable....: is valid until 2011-04-27 on B unit, 2011-06-01 on A unit"
        variables = variables_old
        diagnapp = diagnapp_old
    endif

    idx = where(strmatch(variables, variable_name, /FOLD_CASE) eq 1, cnt)
    if cnt eq 0 then begin
        message, 'no matching variables. Try print, telemetry(/list) for a list of variables', /info
        return, -1
    endif

    print, "Variable selected: " + variable_name
    if strlowcase(variable_name) eq "dewpoint" then modif = '-w ' else modif = ''
    if strlowcase(variable_name) eq "windspeed" then modif = '-w ' else modif = ''
    ;TELEMETRY FILE SELECTION
    if ~keyword_set(now) then begin 
        fast_file = [file_search( "$ADOPT_LOG/"+diagnapp[idx]+"_TELEMETRY.*.tel*" )]
        last_file = file_search("$ADOPT_LOG/"+diagnapp[idx]+"_TELEMETRY.tel*")
        print, diagnapp[idx]
        if (diagnapp[idx] eq '(process_list.adsecarb.msgd_name') then begin
            fast_file = [file_search( "$ADOPT_LOG/"+diagnapp[idx]+".*.tel" )]
            last_file = file_search("$ADOPT_LOG/"+diagnapp[idx]+".tel")   
        endif
        fast_file_info = file_info(fast_file)
        nf = n_elements(fast_file)
        ;zipflag = intarr(nf)
        ;for i=0, nf-1 do zipflag[i] = strmid(fast_file[i], strlen(fast_file[i])-2) eq "gz"
        ;currzipflag = zipflag
        ;lastzipflag = fix(strmid(last_file[0], strlen(last_file[0])-2) eq "gz")
        ;currlastzipflag = lastzipflag
        fast_file_systime = ulong64( strmid(fast_file,17,14,/rev) )
        fast_file_jultime = dblarr(nf)
        for i=0, nf-1 do begin
            tmp = bin_date(systime(0, fast_file_systime[i]))
            fast_file_jultime[i] = julday(tmp[1], tmp[2], tmp[0], tmp[3], tmp[4], tmp[5])
        endfor

        if last_file[0] ne "" then begin
            fast_file = [fast_file, last_file]
            last_file_info = file_info(last_file)
            fast_file_info = [fast_file_info,last_file_info]
            spawn, 'date --date "`tail -q -n 1 '+last_file+' | cut -b 36-61`" +%s', last_tmp
            tmp = bin_date(systime(0, last_tmp))
            nf +=1
            fast_file_jultime = [fast_file_jultime,  julday(tmp[1], tmp[2], tmp[0], tmp[3], tmp[4], tmp[5])]
        endif
    
    ;    if keyword_set(NOW) then begin
    ;        julian_to = systime(/julian)
    ;        julian_from = julian_to - 7200
    ;    endif else begin
            
        if n_elements(to) ne 0 then begin
            yy_t= long(strmid(to, 0,4))
            mm_t = long(strmid(to, 4,2))
            dd_t = long(strmid(to, 6,2))
            h_t  = long(strmid(to, 9,2))
            m_t  = long(strmid(to, 11,2))
            s_t  = long(strmid(to, 13,2))
            julian_to = julday(mm_t, dd_t, yy_t, h_t, m_t, s_t) 
        endif else begin
            julian_to = systime(/julian)
            if n_elements(jto) eq 0 then julian_to = systime(/julian) else julian_to=jto
        endelse

        if n_elements(from) ne 0 then begin
            yy_t= long(strmid(from, 0,4))
            mm_t = long(strmid(from, 4,2))
            dd_t = long(strmid(from, 6,2))
            h_t  = long(strmid(from, 9,2))
            m_t  = long(strmid(from, 11,2))
            s_t  = long(strmid(from, 13,2))
            julian_from = julday(mm_t, dd_t, yy_t, h_t, m_t, s_t) 
        endif else begin
            if n_elements(jfrom) eq 0 then julian_from = julian_to - 2/24. else julian_from=jfrom
        endelse
        
;    endelse

        if julian_from gt julian_to then begin
            print, "Error in FROM - TO request, FROM timestamp greater than TO timestamp "
            return, -1
        endif
 
        ;TEST if it is in the last record
        idx1= (where(fast_file_jultime ge julian_from))[0]
        idx2= (where(fast_file_jultime gt julian_to))[0]

        if idx1[0] eq -1 then begin
            spawn, 'zcat -f '+fast_file[0]+'| head -q -n 1 | cut -b 36-61', first_time
            print, 'WARNING: no records in the specified date, using first available on '+first_time
            idx1=0
        endif

        if idx2[0] eq -1 then begin
            spawn, 'zcat -f '+last_file[0]+' | tail -q -n 1  | cut -b 36-61', last_time
            print, 'WARNING: no records in the specified date, using last available on '+last_time
            idx2=nf-1
        endif 

        if idx1 gt idx2 then begin
            print, "Bad request: please specify better FROM and TO parameters."
        endif

        if idx1[0] gt 0 then idx1[0] -=1
        if idx2[0] lt nf-1 then idx2[0] +=1

        file2analyze = fast_file[idx1:idx2]
        if n_elements(file2analize) eq 1 then file2analize = file2analize[0]

    endif else begin
        file2analyze = file_search("$ADOPT_LOG/"+diagnapp[idx]+"_TELEMETRY.tel")
    endelse

    print, "File selected: "
    for k=0, n_elements(file2analyze)-1 do print, file2analyze[k]
    
    str  = " zgrep -ih "+modif+variables[idx]+" "+strjoin(file2analyze, ' ')+"  | cut -b 36-61 > $ADOPT_LOG/times.dat"
;    if (variable_name eq 'elevation') then str  = " zgrep -ihw "++" "+strjoin(file2analyze, ' ')+"  | cut -b 36-61 > $ADOPT_LOG/times.dat" 
    print, "Sending grep command for time: "+str
    spawn, str, res, errres, exit_status=exit_status  
    if ((res ne 0) )  then begin
        message, 'error parsing telemetry files' 
        return, -1
    endif

    ; template to parse date/time
    templ = { $
        version : 1.0, $
        datastart : 0L, $
        delimiter : 32B, $
        missingvalue: 0.0, $
        commentsymbol: '', $
        fieldcount: 2L, $
        fieldtypes: [7L, 7L], $
        fieldnames: ['date', 'time'], $
        fieldlocations: [0L, 11L], $
        fieldgroups: [0L, 1] $
    }
    print, "Getting times.dat file..."
    tmp_t = read_ascii(getenv('ADOPT_LOG')+'/times.dat', templ=templ) 
;    spawn, 'rm -f $ADOPT_LOG/tempi.dat'
    datetime_str = tmp_t(*).date+' '+tmp_t(*).time
    datetime_str = tmp_t(*).date+' '+tmp_t(*).time
    yy = long(strmid(tmp_t(*).date, 0,4))
    mm = long(strmid(tmp_t(*).date, 5,2))
    dd = long(strmid(tmp_t(*).date, 8,2))
    h  = long(strmid(tmp_t(*).time, 0,2))
    m  = long(strmid(tmp_t(*).time, 3,2))
    s  = float(strmid(tmp_t(*).time, 6,9))
    julian = julday(mm, dd, yy, h, m, s)

    if keyword_set(NOW) then begin
        line_stop = n_elements(julian)-1
        line_start = n_elements(julian)-1
        time = julian[n_elements(julian)-1]
    endif else begin

        line_start = (where(julian ge julian_from))[0]
        line_stop  = (where(julian ge julian_to))[0]

        if line_start[0] eq -1 then line_start=0
        if line_stop[0] eq -1 then line_stop=n_elements(julian)-1
        time = julian[line_start:line_stop]

    endelse


    str1  = " zgrep -ih "+modif+variables[idx]+" "+strjoin(file2analyze, ' ')+"  | cut -b 63- | sed 's/^.*> //' |  sed -n -e '"+strtrim(line_start+1,2)+","+strtrim(line_stop+1,2)+"p' > $ADOPT_LOG/values.dat" 
   
    if (diagnapp[idx] eq 'ADSECARB00') then str1  = " zgrep -ih "+modif+variables[idx]+" "+strjoin(file2analyze, ' ')+" | cut -b 96-100|  sed -n -e '"+strtrim(line_start+1,2)+","+strtrim(line_stop+1,2)+"p' > $ADOPT_LOG/values.dat"

 
    print, "Sending grep command for time: "+str1
    spawn, str1, res1, errres1, exit_status=exit_status1  
    if (res1 ne 0)  then begin
        message, 'error parsing telemetry files' 
        return, -1
    endif   
    spawn, 'rm -f $ADOPT_LOG/times.dat'
    ntimes = n_elements(julian)
    tmp_v = read_ascii(getenv('ADOPT_LOG')+'/values.dat') 
    spawn, 'rm -f $ADOPT_LOG/values.dat'

    values = temporary(tmp_v.(0))
    ntimes = n_elements(julian)
    
    if keyword_set(plot_do) then begin
        date_label = LABEL_DATE(DATE_FORMAT =  ['%H:%I', '%D %M, %Y'])
        
        plot, julian, minmax(values), /nodata, /XSTYLE, $ 
            ; displaying titles. 
            TITLE = variables[idx], $ 
            XTITLE = 'Time', $ 
            YTITLE = 'Values', $ 
            ; applying date/time formats to X-axis labels. 
            POSITION = [0.2, 0.25, 0.9, 0.9], $ 
            XTICKFORMAT = ['LABEL_DATE', 'LABEL_DATE'], $ 
            XTICKUNITS = ['Time', 'Day'], $ 
            XTICKLAYOUT = 2  , $
            XTICKINTERVAL = 2 

        ;plot, julian, minmax(values), /nodata
        for i=0, nvars-1 do begin
            oplot, julian, values[i,*]
        endfor 
    endif
 
    return, values
 
end
