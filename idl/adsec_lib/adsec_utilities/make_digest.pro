Function stime2jul, tfile, TEMPLATE=templ
    if n_elements(templ) eq 0 then begin
    templ = { $
        version : 1.0, $
        datastart : 0L, $
        delimiter : [32B], $
        missingvalue: 0.0, $
        commentsymbol: '', $
        fieldcount: 2L, $
        fieldtypes: [7L, 7L], $
        fieldnames: ['date', 'time'], $
        fieldlocations: [0L, 11L], $
        fieldgroups: [0L, 1] $
    }
    endif
    tmp_t = read_ascii(tfile, templ=templ)
    datetime_str = tmp_t(*).date+' '+tmp_t(*).time
    datetime_str = tmp_t(*).date+' '+tmp_t(*).time
    yy = long(strmid(tmp_t(*).date, 0,4))
    mm = long(strmid(tmp_t(*).date, 5,2))
    dd = long(strmid(tmp_t(*).date, 8,2))
    h  = long(strmid(tmp_t(*).time, 0,2))
    m  = long(strmid(tmp_t(*).time, 3,2))
    s  = float(strmid(tmp_t(*).time, 6,9))
    jtimeline = julday(mm, dd, yy, h, m, s)
  
    return, jtimeline
End

;Function cut_digest, digest, julian_from, julian_to, TIMELINE=timeline
;
;    if digest eq "" then return, digest
;    if !d.name eq "WIN" then term= string([13b, 10b]) else term = string([10B])
;    tstr = strmid(strsplit(digest, term, /EXT),35, 26)
;    time = fltarr(n_elements(tstr))
;    for i=0, n_elements(tstr)-1 do begin
;        spawn, 'date --date "'+tstr[i]+'" +%s', tmptime
;        tmp = bin_date(systime(0, tmptime))
;        time[i] = julday(tmp[1], tmp[2], tmp[0], tmp[3], tmp[4], tmp[5])
;    endfor
;    id1 = where(time gt julian_from)
;    id2 = where(time lt julian_to)
;    if id1[0] eq -1 then return, ""
;    if id2[0] eq -1 then return, ""
;    timeline = time(tstr[id1[0]:(reverse(id2))[0]])
;    return, strjoin(tstr[id1[0]:(reverse(id2))[0]], term)
;
;End

Function md_multiline_grep, file, pattern0, pattern1, TIMELINE=timeline, JULIAN_FROM=julian_from, JULIAN_TO=julian_to, TXT=txt


    undefine, timeline
    undefine, txt
    if !d.name eq "WIN" then term= string([13b, 10b]) else term = string([10B])
    tfile0 = getenv('ADOPT_LOG')+'/md_times0.dat'
    tfile1 = getenv('ADOPT_LOG')+'/md_times1.dat'
    lfile0 = getenv('ADOPT_LOG')+'/md_lines0.dat'
    lfile1 = getenv('ADOPT_LOG')+'/md_lines1.dat'
    if pattern0 eq pattern1 then begin
        spawn, ' zgrep -hw "'+pattern0+'" '+file,txt
        spawn, ' zgrep -hw "'+pattern0+'" '+file +'| cut -b 36-61 > '+tfile0
        if txt[0] eq "" then begin
            undefine,timeline 
            tmptext= ""
        endif else begin
            timeline = stime2jul(tfile0)
            select = where(timeline gt julian_from AND timeline lt julian_to, ns)
            if select[0] eq -1 then begin
                undefine, timeline
                return,  ""
            endif
            txt = txt[select]
            timeline = timeline[select]
            if txt[0] eq "" then begin
                undefine, timeline
                return, ""
            endif else tmptext = strjoin(txt, term)+term
        endelse
    spawn, "rm -f "+ tfile0
    spawn, "rm -f "+ tfile1
    spawn, "rm -f "+ lfile0
    spawn, "rm -f "+ lfile1
    endif else begin
        spawn, ' zgrep -hwn "'+pattern0+'" '+file+'> $ADOPT_LOG/md_tmptext0.dat'
        spawn, ' cat $ADOPT_LOG/md_tmptext0.dat | grep -o "|.*|" | cut -b 17-42 > '+tfile0
        spawn, ' cat $ADOPT_LOG/md_tmptext0.dat | grep -o "^[0-9]*" > '+lfile0
        spawn, ' zgrep -hwn "'+pattern1+'" '+file+'> $ADOPT_LOG/md_tmptext1.dat'
        spawn, ' cat $ADOPT_LOG/md_tmptext1.dat | grep -o "|.*|" | cut -b 17-42 > '+tfile1
        spawn, ' cat $ADOPT_LOG/md_tmptext1.dat | grep -o "^[0-9]*" > '+lfile1

        if file_test(tfile0) and ~file_test(tfile0, /ZERO) then begin
            tline0 = stime2jul(tfile0)
            spawn, 'rm -f '+tfile0
            if file_test(tfile1) and ~file_test(tfile1, /ZERO) then begin
                tline1 = stime2jul(tfile1)
                spawn, 'rm -f '+tfile1
                timeline = [tline0, tline1]
            endif else begin
                undefine, timeline
                return, ""
            endelse
        
        endif else begin
            undefine, timeline
            return, ""
        endelse

        bflag = [tline0*0, tline1*0+1]
        itime = [indgen(n_elements(tline0)), indgen(n_elements(tline1))]
        ltime0 = fix((read_ascii(lfile0)).field1)
        ltime1 = fix((read_ascii(lfile1)).field1)
        ltime = [ltime0, ltime1]
        atime = [tline0, tline1] 
        idx = sort(atime)
        ord_bflag = bflag[idx]
        ord_ltime = ltime[idx]
        ord_tline = timeline[idx]

        if n_elements(julian_from) gt 0 and n_elements(julian_to) gt 0 then begin
            idb = where(ord_tline lt julian_from, compl=c_idb)
            ida = where(ord_tline gt julian_to, compl=c_ida)
            if ~(idb[0] eq -1 and ida[0] eq -1) then begin
                if idb[0] eq -1 then begin
                    ord_bflag = ord_bflag[c_idb]
                    ord_ltime = ord_ltime[c_idb]
                    ord_tline = ord_tline[c_idb]
                endif
                if ida[0] eq -1 then begin
                    ord_bflag = ord_bflag[c_ida]
                    ord_ltime = ord_ltime[c_ida]
                    ord_tline = ord_tline[c_ida]
                endif
                if (idb[0] ne -1 and ida[0] ne -1) then begin
                    err = complement(idb, ida, idc)
                    if err eq 1 then begin
                        text=""
                        undefine, timeline
                        return, text
                    endif else begin
                        ord_bflag = ord_bflag[idc]
                        ord_ltime = ord_ltime[idc]
                        ord_tline = ord_tline[idc]
                    endelse
                endif
            endif
        endif

        for i=0, n_elements(ord_bflag)-2 do begin
            if ord_bflag[i] eq 0 then break
        endfor
        if i eq n_elements(ord_bflag)-2 then begin
            text = ""
            undefine, timeline
            return, text
        endif
        cut0 = i

        for j=n_elements(ord_bflag)-1, -1, 1 do begin
            if ord_bflag[j] eq 1 then break
        endfor
        if j eq 1  then begin
            text = ""
            undefine, timeline
            return, text
        endif
        cut1 = j
        ord_ltime = ord_ltime[cut0:cut1]
        ord_bflag = ord_bflag[cut0:cut1]
        ord_tline = ord_tline[cut0:cut1]

        id = 0
        for i=1, n_elements(ord_bflag)-1 do  if ord_bflag[i-1] ne ord_bflag[i] then id = [id, i]
        ord_bflag = ord_bflag[id]
        ord_ltime = ord_ltime[id]
        ord_tline = ord_tline[id]
        timeline = ord_tline

        select = where(ord_tline gt julian_from AND ord_tline lt julian_to)
        if select[0] eq -1 then return, ""
        timeline = timeline[select]
        ord_bflag = ord_bflag[select]
        ord_ltime = ord_ltime[select]
        tmptext = ""
        for i=0, n_elements(ord_bflag)-1., 2 do begin
            spawn, "sed -n '"+string(ord_ltime[i], format='(I)')+","+string(ord_ltime[i+1], format='(I)')+"p' "+file, txt
            tmptext += strjoin(txt, term)+term
        endfor

    endelse
    return, tmptext

End

Function md_select_file, name, TELEMETRY=telemetry, JULIAN_FROM=julian_from, JULIAN_TO=julian_to

    ;File selections
    if keyword_set(TELEMETRY) then ext='tel*' else ext='log*'
    spawn, "ls -rt "+getenv('ADOPT_LOG')+"/"+name+".*."+ext, flist
    last_file = file_search("$ADOPT_LOG/"+name+"."+ext)
    flist_info = file_info(flist)
    nf = n_elements(flist)
    flist_jultime = dblarr(nf)
    flist_systime = ulong64( strmid(flist,17,14,/rev) )
    for i=0, nf-1 do begin
        tmp = bin_date(systime(0, flist_systime[i]))
        flist_jultime[i] = julday(tmp[1], tmp[2], tmp[0], tmp[3], tmp[4], tmp[5])
    endfor

    if last_file[0] ne "" then begin
        flist = [flist, last_file]
        last_file_info = file_info(last_file)
        flist_info = [flist_info,last_file_info]
        spawn, 'date --date "`tail -q -n 1 '+last_file+' | cut -b 36-61`" +%s', last_tmp
        tmp = bin_date(systime(0, last_tmp))
        nf +=1
        flist_jultime = [flist_jultime,  julday(tmp[1], tmp[2], tmp[0], tmp[3], tmp[4], tmp[5])]
    endif

    ;    if keyword_set(NOW) then begin
    ;        julian_to = systime(/julian)
    ;        julian_from = julian_to - 7200
    ;    endif else begin
    if julian_from gt julian_to then begin
        print, "Error in FROM - TO request, FROM timestamp greater than TO timestamp "
        return, -1
    endif

    ;TEST if it is in the last record
    idx1= (where(flist_jultime ge julian_from))[0]
    idx2= (where(flist_jultime gt julian_to))[0]

    if idx1[0] eq -1 then begin
        if flist[0] ne "" then begin
            spawn, 'zcat -f '+flist[0]+' | head -q -n 1  | cut -b 36-61', first_time
            print, 'WARNING: no records in the specified date, using first available on '+first_time
        endif
        idx1=0
    endif

    if idx2[0] eq -1 then begin
        if last_file[0] ne "" then begin
            spawn, 'zcat -f '+last_file[0]+'| tail -q -n 1  | cut -b 36-61', last_time
            print, 'WARNING: no records in the specified date, using last available on '+last_time
        endif
        idx2=nf-1
    endif
    if idx1 gt idx2 then begin
        print, "Bad request: please specify better FROM and TO parameters."
    endif

    if idx1[0] gt 0 then idx1[0] -=1
    if idx2[0] lt nf-1 then idx2[0] +=1

    file2analyze = flist[idx1:idx2]
    if n_elements(file2analyze) eq 1 then file2analyze = file2analyze[0]
    return, file2analyze

End

Pro make_digest


    set_plot, 'z'
    device, SET_RESOLUTION=[1024, 600], SET_PIXEL_DEPTH=24
    erase
;    set_plot, 'x'
    !P.color=0
    !P.background='ffffff'xl
    !P.charthick=1.0
    !P.charsize=0.9
    !p.thick = 3
    ;Log files selection
    ;process list identification
    julian_from = systime(/julian)-1.
    julian_to = systime(/julian)

    plist_file = filepath(Root=getenv('ADOPT_ROOT'), SUB=['conf','adsec', 'current', 'processConf'], "processList.conf")
    ll = read_ascii_structure(plist_file)
    pl_names =[ strsplit(ll.sys_processes, ' ', /ex), strsplit(ll.adsec_processes,' ', /ex), strsplit(ll.interfaces, ' ', /ex)]
    msgd_names = pl_names+'.'+getenv('ADOPT_SIDE')
    if !d.name eq "WIN" then term= string([13b, 10b]) else term = string([10B])

    ;;idlctrl.L;;
    name = "adsec"+'.'+getenv('ADOPT_SIDE')
    file2analyze = md_select_file(name, JULIAN_FROM=julian_from, JULIAN_TO=julian_to)

    pattern0 = "PROCESS DUMP START"
    pattern1 = "PROCESS DUMP END"
    print, "File processing: "
    digest = ""
    digest_adsec = ""
    tmpdigest = ""
    for k=0, n_elements(file2analyze)-1 do begin 
    ;    digest_adsec +=file2analyze[k] + term
        print, file2analyze[k]
        tmpdigest += md_multiline_grep(file2analyze[k], pattern0, pattern1, JULIAN_FROM=julian_from, JULIAN_TO=julian_to, TIMELINE=ttmp)
        if n_elements(ttmp) gt 0 and n_elements(tadsec) gt 0 then tadsec = [tadsec, ttmp]
        if n_elements(ttmp) gt 0 and n_elements(tadsec) eq 0 then tadsec = ttmp
    endfor
    undefine, ttmp
    digest_adsec = tmpdigest

    name = "fastdiagn"+'.'+getenv('ADOPT_SIDE')
    tmpdigest = ""
    file2analyze = md_select_file(name, JULIAN_FROM=julian_from, JULIAN_TO=julian_to)
    pattern0 = "FUNCTEMERGENCYST"
    pattern1 = pattern0
    for k=0, n_elements(file2analyze)-1 do begin 
        print, file2analyze[k]
        tmpdigest += md_multiline_grep(file2analyze[k], pattern0, pattern1, TIMELINE=ttmp, JULIAN_FROM=julian_from, JULIAN_TO=julian_to, TXT=txt1)
        if n_elements(ttmp) gt 0 and n_elements(tfast1) gt 0 then begin
            tfast1 = [tfast1, ttmp]
            tmp_txt1 = [tmp_txt1, txt1]
        endif
        if n_elements(ttmp) gt 0 and n_elements(tfast1) eq 0 then begin
            tfast1 = ttmp
            tmp_txt1 = txt1
        endif
    endfor
    undefine, ttmp
    digest_fast = tmpdigest

    name = "fastdiagn"+'.'+getenv('ADOPT_SIDE')
    tmpdigest = ""
;    file2analyze = md_select_file(name, JULIAN_FROM=julian_from, JULIAN_TO=julian_to)
    pattern0 = "FUNCTALARM"
    pattern1 = pattern0
    for k=0, n_elements(file2analyze)-1 do begin 
        print, file2analyze[k]
        tmpdigest += md_multiline_grep(file2analyze[k], pattern0, pattern1, TIMELINE=ttmp, JULIAN_FROM=julian_from, JULIAN_TO=julian_to, TXT=txt2)
        if n_elements(ttmp) gt 0 and n_elements(tfast2) gt 0 then begin
            tfast2 = [tfast2, ttmp]
            tmp_txt2 = [tmp_txt2, txt2]
        endif
        if n_elements(ttmp) gt 0 and n_elements(tfast2) eq 0 then begin
            tfast2 = ttmp
            tmp_txt2 = txt2
        endif
    endfor
    undefine, ttmp
    digest_fast2 = tmpdigest

    name = "fastdiagn"+'.'+getenv('ADOPT_SIDE')
    tmpdigest = ""
;    file2analyze = md_select_file(name, JULIAN_FROM=julian_from, JULIAN_TO=julian_to)
    pattern0 = "FUNCTWARNING"
    pattern1 = pattern0
    for k=0, n_elements(file2analyze)-1 do begin 
        print, file2analyze[k]
        tmpdigest += md_multiline_grep(file2analyze[k], pattern0, pattern1, TIMELINE=ttmp, JULIAN_FROM=julian_from, JULIAN_TO=julian_to, TXT=txt3)
        if n_elements(ttmp) gt 0 and n_elements(tfast3) gt 0 then begin
            tfast3 = [tfast3, ttmp]
            tmp_txt3 = [tmp_txt3, txt3]
        endif
        if n_elements(ttmp) gt 0 and n_elements(tfast3) eq 0 then begin
            tfast3 = ttmp
            tmp_txt3 = txt3
        endif
    endfor
    undefine, ttmp
    digest_fast3 = tmpdigest
;    if n_elements(tfast1) gt 0 then tmp_tfast = tfast1
;    if n_elements(tfast2) gt 0 and n_elements(tmp_tfast) gt 0 then tmp_tfast = [tmp_tfast,tfast2] else tmp_tfast = tfast2
;    if n_elements(tfast3) gt 0 and n_elements(tmp_tfast) gt 0 then tmp_tfast = [tmp_tfast,tfast3] else tmp_tfast = tfast3
    tmp_txt=""
    if n_elements(tmp_txt1) gt 0 then begin
        tmp_txt = tmp_txt1
        tmp_tfast = tfast1
    endif
    if n_elements(tmp_txt2) gt 0 then begin
        if n_elements(tmp_txt) gt 0 then begin
            tmp_txt = [tmp_txt,tmp_txt2]
            tmp_tfast = [tmp_tfast,tfast2]
        endif else begin
            tmp_txt = tmp_txt2
            tmp_tfast = tfast2
        endelse
    endif
    if n_elements(tmp_txt3) gt 0 then begin
        if n_elements(tmp_txt) gt 0 then begin
            tmp_txt = [tmp_txt,tmp_txt3]
            tmp_tfast = [tmp_tfast,tfast3]
        endif else begin
            tmp_txt = tmp_txt3
            tmp_tfast = tfast3
        endelse
    endif

    if tmp_txt[0] ne "" then begin
        ids = sort(tmp_tfast)
        tfast = tmp_tfast[ids]
        digest_fast = strjoin(tmp_txt[ids], term)
    endif else digest_fast = ""
    name = "housekeeper"+'.'+getenv('ADOPT_SIDE')
    tmpdigest = ""
    file2analyze = md_select_file(name, JULIAN_FROM=julian_from, JULIAN_TO=julian_to)
    pattern0 = "FUNCTEMERGENCYST"
    pattern1 = pattern0
    for k=0, n_elements(file2analyze)-1 do begin 
        tmpdigest += md_multiline_grep(file2analyze[k], pattern0, pattern1, TIMELINE=ttmp, JULIAN_FROM=julian_from, JULIAN_TO=julian_to, TXT=txt1)
        if n_elements(ttmp) gt 0 and n_elements(thkpr) gt 0 then thkpr = [thkpr, ttmp]
        if n_elements(ttmp) gt 0 and n_elements(thkpr) eq 0 then thkpr = ttmp
    endfor
    undefine, ttmp
    digest_hkpr = tmpdigest


    ;PLOT TEMPO VS ELEVAZIONE/AZIMUTH/WINDSPEED/TSSENABLE/TSSACTIVATED + FAULT
    date_label = LABEL_DATE(DATE_FORMAT =  ['%H:%I', '%D %M, %Y UT'])

;    window,0, xs=1024, ys=600
    erase
    col = comp_colors(3)
    var0 = telemetry('elevation', jfrom=julian_from, jto=julian_to, jtime=jtime0)
    id = where(finite(var0), compl=nid)
    plot, [julian_from, julian_to], [0,0], YSTYLE=9, YR=[0, 90] , YTITLE='Elevation [deg]' $
        , POSITION = [0.05, 0.12, 0.95, 0.95], XTICKFORMAT = ['LABEL_DATE', 'LABEL_DATE'] $
        , XTICKUNITS = ['Hour','Day'], XTICKLAYOUT = 2  , XTICKINTERVAL = 1, psym=3, /NODATA 
    if id[0] ne -1 then oplot, jtime0[id], var0[id]/3600., COL=col[0], psym=4, symsize = 0.2

    r0 = !Y.CRANGE
    r1 = !Y.CRANGE/max(!Y.CRANGE)*360
    axis, /YAXIS, YR=r1, YTITLE='Azimuth [deg]', YS=17, YTICKFORMAT='(I2)'
    fact = (max(r0)-min(r0))/(max(r1)-min(r1))
    offs = 0

    var1 = telemetry('azimuth', jfrom=julian_from, jto=julian_to, jtime=jtime1)
    id1 = where(finite(var1), compl=nid)
    if id1[0] ne -1 then begin
        var1[id1]= ((var1[id1]/3600. + 36000 ) mod 360)/4.
        oplot, jtime1[id1], (var1[id1])*fact+offs, COL=col[1], psym=4, symsize = 0.2
    endif
    var3b = telemetry('extwinddir', jfrom=julian_from, jto=julian_to, jtime=jtime3b)
    id3b = where(finite(var3b), compl=nid)
    if id3b[0] ne -1 then begin
        var3b[id3b]= ((var3b[id3b] + 36000 ) mod 360)/4.
        oplot, jtime3b[id3b], (var3b[id3b])*fact+offs, COL=col[2], psym=4, symsize = 0.2
    endif
    legend, col=col, ['Elevation', 'Azimuth', 'External Wind Direction'], /right, textcol = col
    write_jpeg, '/tmp/digest_elev.jpg', tvrd(/true), quality=100, /TRUE


;    window,1, xs=1024, ys=600
    erase
    col = comp_colors(4)
    var2 = telemetry('windspeed', jfrom=julian_from, jto=julian_to, jtime=jtime2)
    id2 = where(finite(var2), compl=nid2)
    if id2[0] ne -1 then yr2=minmax(var2[id2]) else yr2 = [0, 25]

    var3 = telemetry('extwindspeed', jfrom=julian_from, jto=julian_to, jtime=jtime3)
    id3 = where(finite(var3), compl=nid3)
    if id3[0] ne -1 then yr3=minmax(var3[id3]) else yr3 = [0, 25]

    yr = minmax([yr2, yr3])
    yr[0] = yr[0] < 0
    yr[1] = yr[1] > 25
    plot, [julian_from, julian_to], [0,0], YSTYLE=9, YR=yr , YTITLE='Speed [m/s]' $
        , POSITION = [0.05, 0.12, 0.95, 0.95], XTICKFORMAT = ['LABEL_DATE', 'LABEL_DATE'] $
        , XTICKUNITS = ['Hour','Day'], XTICKLAYOUT = 2  , XTICKINTERVAL = 1, psym=3, /NODATA
    r0 = !Y.CRANGE
    r1 = !Y.CRANGE/max(!Y.CRANGE)*360
    axis, /YAXIS, YR=r1, YTITLE='Azimuth and Wind Direction [deg]', YS=17, YTICKFORMAT='(I3)'
    fact = (max(r0)-min(r0))/(max(r1)-min(r1))
    offs = 0

    if id3b[0] ne -1 then oplot, jtime3b[id3b], (var3b[id3b])*fact+offs, COL=col[2], psym=4, symsize = 0.2
    if id1[0] ne -1 then oplot, jtime1[id1], (var1[id1])*fact+offs, COL=col[3], psym=4, symsize = 0.2
    if id2[0] ne -1 then oplot, jtime2[id2], var2[id2], COL=col[0], psym=4, symsize = 0.2
    if id3[0] ne -1 then oplot, jtime3[id3], var3[id3], COL=col[1], psym=4, symsize = 0.2
    legend, col=col, ['Windspeed', 'External Windspeed', 'External Wind Direction', 'Azimuth'], /right, textcol = col
    write_jpeg, '/tmp/digest_wind.jpg', tvrd(/true), quality=100, /TRUE

;    window,2, xs=1024, ys=600
    erase
    col = comp_colors(9)
;    col = col([2,4,6,8,10,1,3,5,7,9,0])
    plot, [julian_from, julian_to], [0,0], YSTYLE=1, YR=[-2, 3] , YTITLE='[au]' $
        , POSITION = [0.05, 0.12, 0.95, 0.95], XTICKFORMAT = ['LABEL_DATE', 'LABEL_DATE'] $
        , XTICKUNITS = ['Hour','Day'], XTICKLAYOUT = 2  , XTICKINTERVAL = 1, psym=3, /NODATA, TITLE='POWER STATUS'
    var4 = telemetry('AdamTSSDisable', jfrom=julian_from, jto=julian_to, jtime=jtime4)
    id = where(finite(var4), compl=nid)
    fact = 1;(max(r0)-min(r0))/(max(r1)-min(r1))
    offs = 0
    if id[0] ne -1 then oplot, jtime4[id], (var4[id])*fact+offs, COL=col[0], psym=6, symsize = 0.2

    var5 = telemetry('AdamDriverEnable', jfrom=julian_from, jto=julian_to, jtime=jtime5)
    id = where(finite(var5), compl=nid)
    if id[0] ne -1 then oplot, jtime5[id], var5[id]*fact+offs, COL=col[1], psym=6, symsize = 0.2

    var6 = telemetry('AdamMainPower', jfrom=julian_from, jto=julian_to, jtime=jtime6)
    id = where(finite(var6), compl=nid)
    if id[0] ne -1 then oplot, jtime6[id], var6[id]*fact+offs, COL=col[2], psym=6, symsize = 0.2

    ;questa e' un po' maiala perche' suppongo i tempi perfettamente sovrapposti ma mi sfava interpolare....
    var7 = ~var5 and ~var4
    id = where(finite(var7), compl=nid)
    jtime7 = jtime5
    if id[0] ne -1 then oplot, jtime7[id], var7[id]*fact+offs, COL=col[3], psym=6, symsize = 0.2

    if n_elements(tfast3) gt 0 then  oplot, tfast3, tfast3*0-0.5, psym=3, col=col[4]
    if n_elements(tfast2) gt 0 then  oplot, tfast2, tfast2*0-0.5, psym=3, col=col[5]
    if n_elements(tfast1) gt 0 then  oplot, tfast1, tfast1*0-0.5, psym=4, col=col[6]
    if n_elements(thkpr) gt 0 then  oplot, thkpr, thkpr*0-1, psym=4, col=col[7]
   legend, col=col, ['TSSDisable', 'DriverEnable', 'MainPower', 'TSS-Active', 'Warnig from Fast' , 'Alarm from Fast','EmergencyStop from Fast','EmergencyStop from Housekeeper'], /right, textcol = col
    write_jpeg, '/tmp/digest_status.jpg', tvrd(/true), quality=100, /TRUE
    erase
    ;PLOT TEMPO VS TEMPERATURE CP, RB, EXT, STRATIX, COOLING + FAULT
    h0d = telemetry('BCUPowerTemp', jfrom=julian_from, jto=julian_to, jtime=jtime0)
    h0 = rebin(temporary(h0d), 1, n_elements(jtime0))
    h1d = telemetry('BCUStratixTemp', jfrom=julian_from, jto=julian_to, jtime=jtime1)
    h1 = rebin(temporary(h1d), 1, n_elements(jtime1))
    h2d = telemetry('DSPStratixTemp', jfrom=julian_from, jto=julian_to, jtime=jtime2)
    h2 = rebin(temporary(h2d), 1, n_elements(jtime2))
    h3 = telemetry('SwitchStratixTemp', jfrom=julian_from, jto=julian_to, jtime=jtime3)
    h4 = telemetry('DewPoint', jfrom=julian_from, jto=julian_to, jtime=jtime4)
    id4 = where(h4 lt 30, compl=nid4)
    h5 = telemetry('CPTemp', jfrom=julian_from, jto=julian_to, jtime=jtime5)
    h6 = telemetry('RBodyTemp', jfrom=julian_from, jto=julian_to, jtime=jtime6)
    ;h7 = telemetry('ExternalHumidity', jfrom=julian_from, jto=julian_to, jtime=jtime7)
    h8 = telemetry('ExternalTemperature', jfrom=julian_from, jto=julian_to, jtime=jtime8)
    h9 = telemetry('HubTemp', jfrom=julian_from, jto=julian_to, jtime=jtime9)
    ha = telemetry('WaterMainInlet', jfrom=julian_from, jto=julian_to, jtime=jtimea)
    hb = telemetry('WaterMainOutlet', jfrom=julian_from, jto=julian_to, jtime=jtimeb)
    hc = telemetry('WaterColdPlateInlet', jfrom=julian_from, jto=julian_to, jtime=jtimec)
    hd = telemetry('WaterColdPlateOutlet', jfrom=julian_from, jto=julian_to, jtime=jtimed)
    he = telemetry('FluxRateIn', jfrom=julian_from, jto=julian_to, jtime=jtimee)
    if id4[0] ne -1 then h4r = minmax(h4[id4]) else h4r=[0,0]
    yra = minmax([minmax(h0),minmax(h1), minmax(h2), minmax(h3), h4r, minmax(h5) $
                , minmax(h6), minmax(h8), minmax(h9), minmax(ha), minmax(hb) $
                , minmax(hc), minmax(hd)])+[-1, 1]
    yrf = [-15, 110]
    yr = [[yra], [yrf]]
    col = [comp_colors(13),0]
    !P.THICK=1
    for i=0, 1 do begin
     
        ;window,/FREE, xs=1024, ys=600
        plot, [julian_from, julian_to], [0,0], /XSTYLE, YSTYLE=1, YR=yr[*,i] , YTITLE='[C deg]'$
            , POSITION = [0.05, 0.12, 0.95, 0.95], XTICKFORMAT = ['LABEL_DATE', 'LABEL_DATE'] $
            , XTICKUNITS = ['Hour','Day'], XTICKLAYOUT = 2  , XTICKINTERVAL = 1, psym=10, /NODATA, TITLE='Temperature Digest'
        r0 = !Y.CRANGE
        r1 = !Y.CRANGE/max(!Y.CRANGE)*15
        fact = (max(r0)-min(r0))/(max(r1)-min(r1))
        offs = 0
        axis, /YAXIS, YR=r1, YTITLE='[l/min]', YS=17
        oplot, jtime0, h0, COL=col[0], psym=3
        oplot, jtime1, h1, COL=col[1], psym=3
        oplot, jtime2, h2, COL=col[2], psym=3
        oplot, jtime3, h3, COL=col[3], psym=3
        if id4[0] ne -1 then oplot, jtime4[id4], h4[id4], COL=col[4], psym=3
        if nid4[0] ne -1 then begin
            h4[nid4] = 0
            oplot, jtime4[nid4], h4[nid4], COL=0, psym=3
        endif
        oplot, jtime5, h5, COL=col[5], psym=3
        oplot, jtime6, h6, COL=col[6], psym=3
        ;oplot, jtime7, h7, COL=col[7], psym=3
        oplot, jtime8, h8, COL=col[7], psym=3
        oplot, jtime9, h9, COL=col[8], psym=3
        oplot, jtimea, ha, COL=col[9], psym=3
        oplot, jtimeb, hb, COL=col[10], psym=3
        oplot, jtimec, hc, COL=col[11], psym=3
        oplot, jtimed, hd, COL=col[12], psym=3
        oplot, jtimee, he*fact+offs, COL=col[13], psym=3
        legend, col=col, ['BCUPowerTemp','BCUStratixTemp','DSPStratixTemp','SwitchStratixTemp','DewPoint','CPTemp','RBodyTemp','ExternalTemperature','HubTemp','WaterMainInlet','WaterMainOutlet','WaterColdPlateInlet','WaterColdPlateOutlet', 'FluxRateIn'], /right,   textcol = col

        if i eq 0 then write_jpeg, '/tmp/digest_temp_auto.jpg', tvrd(/true), true=1, quality=100 $
        else  write_jpeg, '/tmp/digest_temp_fixed.jpg', tvrd(/true), true=1, quality=100
        erase
    endfor
    
    openw, unit, '/tmp/digest_adsec.txt', /GET
    printf, unit, digest_adsec
    free_lun, unit
    
    openw, unit, '/tmp/digest_fast.txt', /GET
    printf, unit, digest_fast
    free_lun, unit
    
    openw, unit, '/tmp/digest_hkpr.txt', /GET
    printf, unit, digest_hkpr
    free_lun, unit
    
    

return
end
