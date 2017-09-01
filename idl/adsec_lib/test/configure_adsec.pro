;@File: prepare.py
;
; AO Supervisor adaptive secondary configuration utility
; 
; M.Xompero, Sep 2011.
;
; This procedure will prompt from you a few pieces of information and
; set up the configuration of the current AdaptiveSecondary.
;
; History:
; 1.0 First release.
;@


Function expand_keys, fields

    @adsec_common
    first = fix(fields[1])
    last  = fix(fields[2])
    if last-first eq 0 then begin
        return, fields[0]+string(last, format='(I3.3)')
    endif else begin
        id = indgen(last-first+1)+first
        return, fields[0]+string(id, format='(I3.3)')
    endelse

End

Function expand_vals, fields

    @adsec_common
    first = fix(fields[1])
    last  = fix(fields[2])
    struct = {amin:fields[3], wmin:fields[4], wmax:fields[5], amax:fields[6], rm:fields[7], caf:fields[8], ena:fields[9], loop:fields[10] }
    if last-first eq 0 then begin
        return, struct
    endif else begin
        id = indgen(last-first+1)+first
        return, replicate(struct, last-first+1)
    endelse

End

Function create_diagnapp_dict, txt, DICT_KEYS=dict_keys_out, DICT_VALS =dict_vals_out
    
    @adsec_common
    print, "Loading FastDiagnostic configuration (it will take a while...)"            
    t_val = {amin:0, wmin:0, wmax:0, amax:0, rm:0, caf:0, ena:0, loop:'' }
    t_key = "" ;family+index

    for i=0, n_elements(txt)-1 do begin
        fields = strsplit(txt[i], ' ', /ex) 
        if n_elements(dict_keys) eq 0 then begin
            dict_keys = expand_keys(fields)
            dict_vals = expand_vals(fields)
        endif else begin
            dict_key1 = expand_keys(fields)
            dict_val1 = expand_vals(fields)
            for k=0, n_elements(dict_key1)-1 do begin
                id = where(dict_keys eq dict_key1[k])
                if id[0] eq -1 then begin
                    dict_keys=[dict_keys, dict_key1[k]]
                    dict_vals=[dict_vals, dict_val1[k]]
                endif else begin
                    dict_keys[id]=dict_key1[k]
                    dict_vals[id]=dict_val1[k]
                endelse
            endfor
        endelse
    endfor
    dict_keys_out=dict_keys
    dict_vals_out=dict_vals
    return, 0

End


Function read_param_file,  DICT_KEYS=dict_keys, DICT_VALS =dict_vals, COMMENTS=comments

    @adsec_common
    fastfile = adsec_path.conf+'processConf/fastdiagn/fastdiagn.param'
    txt = read_text_file(fastfile)

    ;remove comments
    id = where(stregex(txt, '^#'), COMPL=cc)
    if id[0] ge 0 then begin
        comments = txt[cc[1:*]]
        txt = txt[id]
    endif
        
    id = where(strlen(txt) ne 0)
    if id[0] ge 0 then txt = txt[id]

    ;create dictionary
    return, create_diagnapp_dict(txt,  DICT_KEYS=dict_keys, DICT_VALS =dict_vals)
End

Function vals2str,  struct

    str=""
    for i=0, n_tags(struct)-1 do str += '    '+struct.(i)
    return, str

End

Function dump_param_file, dict_keys, dict_vals, TN=tn, ELEC=elec, COMMENTS=comments

    @adsec_common
    ;dump diagnapp dict to file
    if n_elements(tn) eq 0 then tn = tracknum()
    if n_elements(elec) eq 0 then elec=adsec
    fastfile = adsec_path.conf+'processConf/fastdiagn/fastdiagn.param'
    openw, unit, fastfile, /get
    
    txt=['CHCURRAVERAGE             0     671   -0.79       -0.7        0.75        0.79        0       0       ena   fast' $
        ,'CHCURRRMS                 0     671   -1          -3e-14       0.5         0.5         0       0       ena   fast' $
        ,'CHDISTAVERAGE             0     671    2e-05       2e-05      0.000125    0.000135    0       0       ena   fast' $
        ,'CHDISTRMS                 0     671   -1          -3e-14       3e-08       5e-07       0       0       ena   fast' $
        ,'CHFFCOMMAND               0     671   -0.69       -0.3        0.3         0.69        0       0       ena   fast' $
        ,'CHFFPURECURRENT           0     671   -0.69       -0.3        0.3         0.69        0       0       ena   fast' $
        ,'CHINTCONTROLCURRENT       0     671   -0.69       -0.3        0.3         0.69        0       0       ena   fast' $
        ,'CHNEWDELTACOMMAND         0     671   -0.69       -0.3        0.3         0.69        0       0       ena   fast' $
        ,'COMMANDHISTORYPTR         0     0     0           0           inf         inf         0       0       ena   fast' $
        ,'DIAGNAPPFASTVARSCOUNTER   0     0     0           0           inf         inf         0       0       ena   fast' $
        ,'DIAGNAPPFASTVARSRATE      0     0     0           0           inf         inf         0       0       ena   fast' $
        ,'DIAGNAPPFRAMECOUNTER      0     0     0           0           inf         inf         0       0       ena   fast' $
        ,'DIAGNAPPFRAMERATE         0     0     0           0           inf         inf         0       0       ena   fast' $
        ,'DIAGNAPPFRAMETIMESTAMP    0     0     0           0           inf         inf         0       0       ena   fast' $
        ,'DIAGNAPPINITTIMESTAMP     0     0     0           0           inf         inf         0       0       ena   fast' $
        ,'DIAGNAPPSLOWVARSCOUNTER   0     0     0           0           inf         inf         0       0       ena   fast' $
        ,'DIAGNAPPSLOWVARSRATE      0     0     0           0           inf         inf         0       0       ena   fast' $
        ,'MIRRORFRAMECOUNTER        0     0     0           0           inf         inf         0       0       ena   fast' $
        ,'MODES                     0     671   -0.001      -0.001      0.001       0.001       0       0       ena   fast' $
        ,'OFFLOADMODES              0     21    -inf        -inf        inf         inf         1       0       ena   fast' $
        ,'PARAMBLOCKACCELEROMETERS  0     0     -1          -1          2           2           0       1       ena   fast' $
        ,'PARAMBLOCKBLOCKSELECTION  0     0     -1          -1          2           2           0       1       ena   fast' $
        ,'PARAMBLOCKDELTACOMMAND    0     0     -1          -1          2           2           0       1       ena   fast' $
        ,'PARAMBLOCKDIAGNSTORAGE    0     0     -1          -1          2           2           0       1       ena   fast' $
        ,'PARAMBLOCKDISTURBENABLED  0     0     -1          -1          2           2           0       1       ena   fast' $
        ,'PARAMBLOCKFASTLINK        0     0     -1          -1          2           2           0       1       ena   fast' $
        ,'PARAMBLOCKOFFLOADBLOCK    0     0     -1          -1          2           2           0       1       ena   fast' $
        ,'PARAMBLOCKSLOPELINEARIZE  0     0     -1          -1          2           2           0       1       ena   fast' $
        ,'PARAMBLOCKSLOPELINEARIZEMETHOD 0     0     -1          -1          8           8           0       1       ena   fast' $
        ,'SKIPFRAMERATE             0     0     -inf        -inf        inf         inf         1       0       ena   fast' $
        ,'SLOPES                    0     1599  -inf        -inf        inf         inf         0.1     0       ena   fast' $
        ,'SWITCHACCELEROMETERCOEFF  0     2     -inf        -inf        inf         inf         0.1     0       ena   fast' $
        ,'SWITCHNUMFLCRCERR         0     0     0           0           inf         inf         0       0       ena   fast' $
        ,'SWITCHNUMFLTIMEOUT        0     0     0           0           inf         inf         0       0       ena   fast' $
        ,'SWITCHPENDINGSKIPCOUNTER  0     0     0           0           inf         inf         0       0       ena   fast' $
        ,'SWITCHSAFESKIPCOUNTER     0     0     0           0           inf         inf         0       0       ena   fast' $
        ,'SWITCHTIMESTAMP           0     0     0           0           inf         inf         0       0       ena   fast' $
        ,'SWITCHWFSGLOBALTIMEOUT    0     0     0           0           inf         inf         0       0       ena   fast' $
        ,'WFSFRAMECOUNTER           0     0     -2          -2          inf         inf         0       0       ena   fast' $
        ,'POSPISTON                 0     0     -inf        -inf        inf         inf         0.5     0       ena   fast' $
        ,'CURRPISTON                0     0     -inf        -inf        inf         inf         0.5     0       ena   fast' ]

                
    print, "Creating base dictionary (it will take a while...)"            
    printf, unit, '#;;+& '+tn+' &-;;'
    if n_elements(comments) gt 0 then printf, unit, comments
    err = create_diagnapp_dict(txt,  DICT_KEYS=base_keys, DICT_VALS =base_vals)
    printf, unit, txt
 
    ;MODIFY adsec.act_wo_pos
    keys = 'CHDISTAVERAGE'+string(elec.act_wo_pos, format='(I3.3)')
    for i=0, n_elements(keys)-1 do begin
        id = where(dict_keys eq keys[i])
        dict_vals[id].ena=  'dis'
    endfor
    keys = 'CHDISTRMS'+string(elec.act_wo_pos,format='(I3.3)' )
    for i=0, n_elements(keys)-1 do begin
        id = where(dict_keys eq keys[i])
        dict_vals[id].ena=  'dis'
    endfor


    ;MODIFY adsec.act_wo_curr
    keys = 'CHCURRAVERAGE'+string(elec.act_wo_curr, format='(I3.3)')
    for i=0, n_elements(keys)-1 do begin
        id = where(dict_keys eq keys[i])
        dict_vals[id].ena=  'dis'
    endfor

    keys = 'CHCURRRMS'+string(elec.act_wo_curr, format='(I3.3)')
    for i=0, n_elements(keys)-1 do begin
        id = where(dict_keys eq keys[i])
        dict_vals[id].ena=  'dis'
    endfor


    ;MODIFY adsec.act_wo_icc
    keys = 'CHDISTRMS'+string(elec.act_wo_icc, format='(I3.3)')
    for i=0, n_elements(keys)-1 do begin
        id = where(dict_keys eq keys[i])
        dict_vals[id].AMAX=  'Inf'
    endfor

    bool_match = fltarr(n_elements(dict_keys))
    for i=0, n_elements(dict_keys)-1 do bool_match[i] = total((compare_struct(dict_vals[i], base_vals[i])).ndiff) gt 0
    match = where(bool_match, nm)
    id = intarr(nm)
    for i=0, nm-1 do id[i]=fix(strmid(dict_keys[match[i]],strlen(dict_keys[match[i]])-3))
    sid = reverse(sort(id))
    
    for i=0, n_elements(match)-1 do begin
;        if (i gt 0) then if (id[sid[i]] ne id[sid[i-1]]) then print, ""
        keylen = strlen(dict_keys[match[sid[i]]])
        key = strmid(dict_keys[match[sid[i]]], 0, keylen-3)
        from = strmid(dict_keys[match[sid[i]]], keylen-3)
        to = from
        params = vals2str(dict_vals[match[sid[i]]])
        printf, unit, key+" "+from+" "+to+" "+params
    endfor

    free_lun, unit
    return,0 
   

End




Pro printSubMenuFF
    @adsec_common
    print, '1) Change traknum' 
    print, '2) Change to "None"'
    print, '3) Return'
End


Pro printSubMenu
    @adsec_common
    print, '1) Add act' 
    print, '2) Remove act'
    print, '3) Return'
End

Pro printMenu
    @adsec_common
    print, 'Parameters you can configure:'
    print, ''
    slist = strjoin(string(adsec.act_wo_pos, format='(I4.3)'), ' ')
    print, '1) Not working actuators (bad capacitive sensors): '
    print, slist
    slist = strjoin(string(adsec.act_wo_curr, format='(I4.3)'), ' ')
    print, '2) Not working actuators (bad driver current / coils): '
    print, slist
    slist = strjoin(string(adsec.act_wo_icc, format='(I4.3)'), ' ')
    print, '3) Not stable actuators:                 '
    print, slist
    print, '4) Feed forward matrix:                  TN '+(*!AO_STATUS.configuration).ff_matrix
    print, '5) Zernike projection matrix(PMZ):       TN '+(*!AO_STATUS.configuration).pmz
    print, '6) Force projection matrix:              TN '+(*!AO_STATUS.configuration).curr2modes
    print, '7) High order offload projection matrix: TN '+(*!AO_STATUS.configuration).pmhofor
    print, '8) Position projection matrix:           TN '+(*!AO_STATUS.configuration).pos2modes
    print, "9) Instrument flat selection"
    print, '10) Save changes'
    print, '11) Make all changes as permanents'
    print, '12) Restore all configurations from $ADOPT_SOURCE'
    print, '13) Quit without saving'
End

Function getAct
    @adsec_common
    ans = ''
    ex = 0
    ii = 0
    while ~ex do begin
        read, ans, prompt = 'Actuator DSP number> '
        reads, ans, ii, format='(I)'
        if ii lt 0 or ii gt adsec.n_actuators then return, -1 else return, fix(ans)
    endwhile
End

Function getFF, tns, type
    @adsec_common
    ans = ''
    ex = 0
    ii = 0
    while ~ex do begin
        read, ans, prompt = 'FF matrix tracking number> '
        id = where(strmatch(tns, ans))
        if id[0] eq -1 then begin
            print, "Wrong choice. Nothing done."
            return, -1
        endif else return, tns[id]
    endwhile
End

Function manage_ff, type
    
    @adsec_common
    case type of
        "pmz":      begin
                        name   = 'pmz.fits'
                        ctn = (*!AO_STATUS.CONFIGURATION).pmz
                        if strcmp(ctn, "") then tn="None"
                        extdir = 'optical-projection'
                    end
        "pmhofor":  begin
                        name   = 'pmhofor.fits'
                        ctn = (*!AO_STATUS.CONFIGURATION).pmhofor
                        if strcmp(ctn, "") then tn="None"
                        extdir = 'electric-projection'
                    end
        "pmfor":    begin
                        ctn = (*!AO_STATUS.CONFIGURATION).pmfor
                        if strcmp(ctn, "") then tn="None"
                        name   = 'curr2modes.fits'
                        extdir = 'optical-projection'
                    end
        "pmpos":    begin
                        ctn = (*!AO_STATUS.CONFIGURATION).pmpos
                        if strcmp(ctn, "") then tn="None"
                        name   = 'pos2modes.fits'
                        extdir = 'optical-projection'
                    end
         else:      begin
                        name   = 'pos2modes.fits'
                        extdir = 'ff_matrix'
                        ctn = (*!AO_STATUS.CONFIGURATION).ff_matrix 
                        if strcmp(ctn, "") then tn="None"
                    end
    endcase
    printSubMenuFF
    ans = ''
    ex = 0
    fftn = adsec_path.meas+extdir+'/*'
    tns = file_basename(file_search(fftn))
    update=0
    while ~ex do begin
        slist = strjoin(tns, ' ')
        print, 'Current Tracknum: '+ctn
        print, 'Tracknum available: '+slist
        read, ans, prompt = 'Command> '
        case ans of 
            '1': begin
                   tn = getFF(tns) 
                   if tn[0] ne -1 then begin
                        ctn=tn
                        update = 1
                   endif
                 end
            '2': begin
                   ctn = "None"
                   update=1
                 end
            '3': begin
                 ex = 1
                 end
            else: begin
                    print, "Wrong option. Nothing done." 
                    printSubMenuFF
                  end
        endcase
        if update then begin
            case type of
                "pmz":    (*!AO_STATUS.CONFIGURATION).pmz = ctn
                "pmfor":  (*!AO_STATUS.CONFIGURATION).curr2modes = ctn
                "pmpos":  (*!AO_STATUS.CONFIGURATION).pos2modes = ctn
                "pmhofor":(*!AO_STATUS.CONFIGURATION).pmhofor = ctn
                else:     (*!AO_STATUS.CONFIGURATION).ff_matrix = ctn
            endcase
        endif
    endwhile
    return, ctn

End

Function manage_list, list, list_dep
    
    @adsec_common
    newlist = list
    ;slist = string(newlist, format='(I3.3," ")')
    ;print, 'Current list: '+slist
    printSubMenu
    ans = ''
    ex = 0
    while ~ex do begin
        slist = strjoin(string(newlist, format='(I4.3)'), ' ')
        print, 'Current list: '+slist
        read, ans, prompt = 'Command> '
        case ans of 
            '1': begin
                    act = getAct()
                    if act ne -1 then begin
                        newlist = [list,act]
                        newlist = newlist[uniq(newlist, sort(newlist))]
                    endif
                 end
            '2': begin
                    act = getAct()
                    if act ne -1 then begin
                        err = complement(act, list, newlist)
                        if err eq 0 then newlist = newlist[uniq(newlist, sort(newlist))] $
                        else print, "Wrong actuator number. Nothing done."
                    endif
                 end
            '3': begin
                    ex=1
                 end
            else: begin
                    print, "Wrong option. Nothing done." 
                    printSubMenu
                  end

        endcase
    endwhile
    return, newlist

End

Function manage_flat
    return, 0
End

Function displayMain, elec, conf, act_wo_pos, act_wo_curr, act_wo_icc, _pmffTN, _pmzTN,  _pmhoforTN, _pmforTN, _pmposTN

    @adsec_common
    printMenu
    ans = ''
    ex = 0
    if n_elements(act_wo_pos) eq 0 then act_wo_pos  = adsec.act_wo_pos
    if n_elements(act_wo_curr) eq 0 then act_wo_curr = adsec.act_wo_curr
    if n_elements(act_wo_icc) eq 0 then act_wo_icc  = adsec.act_wo_icc
    while ~ex do begin
        read, ans, prompt = 'Command> '
        case ans of
            '1': begin
                list = elec.act_wo_pos
                list_dep = elec.act_wo_curr
                act_wo_pos = manage_list(list, list_dep) 
                printMenu
               end
            '2': begin
                list = elec.act_wo_curr
                list_dep = elec.act_wo_pos
                act_wo_curr = manage_list(list, list_dep)
                printMenu
               end
            '3': begin
                list = elec.act_wo_icc
                list_dep = elec.act_wo_icc
                act_wo_icc = manage_list(list, list_dep)
                printMenu
               end
            '4': begin
                   fftn= manage_ff('ff')
                    printMenu
               end
            '5': begin
                   pmztn= manage_ff('pmz')
                    printMenu
               end
            '6': begin
                   fortn= manage_ff('pmfor')
                    printMenu
               end
            '7': begin
                   hofortn= manage_ff('pmhofor')
                    printMenu
               end
            '8': begin
                   postn= manage_ff('pmpos')
                    printMenu
               end
            '9': begin
                   print, "Not implemeted yet"
               end
            '10': begin
                    ;elec.txt
                    newelec = create_struct('act_wo_pos',act_wo_pos, 'act_wo_curr',act_wo_curr, 'act_wo_icc',act_wo_icc)
                    tags = tag_names(elec)
                    for i=0, n_elements(tags)-1 do begin
                        if (tags[i] eq "ACT_WO_POS") or (tags[i] eq "ACT_WO_CURR") or (tags[i] eq "ACT_WO_ICC") then begin
                            ;donothing
                        endif else  newelec = create_struct(tags[i], elec.(i), newelec)
                    endfor
                    elecfile = adsec_path.conf+!AO_CONST.shell+'/elec.txt'
                    txt = read_text_file(elecfile)
                    tn = tracknum()
                    txt[0] = ';;+& '+tn+' &-;;'
                    id = where(stregex(txt, '^;') eq 0)
                    write_ascii_structure, newelec, FILE=elecfile
                    newtxt = transpose([txt[id], read_text_file(elecfile)])
                    openw, unit, elecfile, /get
                    printf, unit, newtxt
                    free_lun, unit
    
                    conf = read_ascii_structure(adsec_path.conf+'configuration.txt')
                    base_conf = conf

                    ;dat matrices
                    if n_elements(fftn) gt 0 then begin
                        if (fftn ne _pmffTN)  and (fftn ne "None") then begin
                            print, adsec_path.meas+'ff_matrix/'+fftn+'/*.fits' 
                            print, adsec_path.data
                            file_copy, adsec_path.meas+'ff_matrix/'+fftn+'/*.fits' , adsec_path.data, /OVER
                        endif
                        if strcmp(fftn,"None") then conf.ff_matrix = ""
                    endif else begin
                        if strcmp(_pmffTN,"None") then conf.ff_matrix = ""
                    endelse
                    if n_elements(pmztn) gt 0 then begin
                        if (pmztn ne _pmzTN) and (pmztn ne "None")  then begin
                            print, adsec_path.meas+'optical-projection/'+pmztn+'/pmz.fits' 
                            print, adsec_path.data
                            file_copy, adsec_path.meas+'optical-projection/'+pmztn+'/pmz.fits' , adsec_path.data, /OVER
                        endif
                        if strcmp(pmztn,"None") then conf.pmz =""
                    endif else begin
                        if strcmp(_pmzTN,"None") then conf.pmz =""
                    endelse
                    if n_elements(hofortn) gt 0 then begin
                        if (hofortn ne _pmhoforTN) and (hofortn ne "None") then begin
                            print, adsec_path.meas+'electric-projection/'+hofortn+'/*.fits' 
                            print, adsec_path.data
                            file_copy, adsec_path.meas+'electric-projection/'+hofortn+'/pmhofor.fits' , adsec_path.data, /OVER
                        endif
                        if strcmp(hofortn,"None") then conf.pmhofor=""
                    endif else begin
                        if strcmp(_pmhoforTN,"None") then conf.pmhofor=""
                    endelse
                    if n_elements(fortn) gt 0 then begin
                        if (fortn ne _pmforTN) and (fortn ne "None") then begin
                            print, adsec_path.meas+'electric-projection/'+fortn+'/curr2modes.fits' 
                            print, adsec_path.data
                            file_copy, adsec_path.meas+'electric-projection/'+fortn+'/curr2modes.fits' , adsec_path.data+'pmfor.fits', /OVER
                        endif
                        if strcmp(fortn,"None") then conf.pmfor = ""
                    endif else begin
                        if strcmp(_pmforTN,"None") then conf.pmfor = ""
                    endelse
                    if n_elements(postn) gt 0 then begin
                        if (postn ne _pmposTN) and (postn ne "None")then begin
                            print, adsec_path.meas+'optical-projection/'+postn+'/*.fits' 
                            print, adsec_path.data
                            file_copy, adsec_path.meas+'optical-projection/'+postn+'/pos2modes.fits' , adsec_path.data+'pmpos.fits', /OVER
                        endif
                        if strcmp(postn,"None") then conf.pmpos = ""
                    endif else begin
                        if strcmp(_pmposTN,"None") then conf.pmpos = ""
                    endelse
                    if total((compare_struct(conf, base_conf)).ndiff) gt 0 then $
                        write_ascii_structure, conf, file=adsec_path.conf+'configuration.txt' 

                    ;FastDiagnostic
                    err = read_param_file(DICT_KEYS=dict_keys, DICT_VALS =dict_vals, COMMENTS=comments)
                    err = dump_param_file(dict_keys, dict_vals, TN=tn, ELEC=newelecm, COMMENTS=comments)

               end
           '11': begin
                ;io userei rsync per i file che si decidono....
                ex=1
                elecfile = adsec_path.conf+!AO_CONST.shell+'/elec.txt'
                fastfile = adsec_path.conf+'processConf/fastdiagn/fastdiagn.param'
                file_copy, elecfile, '$ADOPT_SOURCE/conf/adsec/'+!AO_CONST.unit+'/'+!AO_CONST.shell+'/elec.txt', /OVER
                file_copy, fastfile, '$ADOPT_SOURCE/conf/adsec/'+!AO_CONST.unit+'/processConf/fastdiagn/fastdiagn.param', /OVER
                spawn, "rsync -avC  $ADOPT_ROOT/calib/adsec/"+!AO_CONST.unit+" $ADOPT_SOURCE/calib/adsec"
                spawn, "rsync -avC  $ADOPT_ROOT/conf/adsec/"+!AO_CONST.unit+"/"+!AO_CONST.shell+" $ADOPT_SOURCE/conf/adsec/"+!AO_CONST.unit
                print, "All changes reported in $ADOPT_SOURCE"
               end
           '12': begin
                cd, current=cc
                cd, getenv('$ADOPT_SOURCE')
                spawn, "make install-conf"
                spawn, "make install-calib"
                print, "All changes discarded. $ADOPT_ROOT synchronized back with $ADOPT_SOURCE"
                cd, cc
               end
           '13': begin
                ex=1
                print, "Quitting..."
               end
           else: begin
                    print, "Bad input. Ignored."
                    printMenu
                 end
        endcase
    endwhile


    return, 0

End


Pro configure_adsec

    @adsec_common
    _aoroot =getenv('ADOPT_ROOT')
    _unit = !AO_CONST.unit
    _side = !AO_CONST.telescope_side
    _calib = adsec_path.data
    _conf = adsec_path.conf
    _shell = !AO_CONST.shell
    conf = read_ascii_structure(adsec_path.conf+'configuration.txt')

    tmpfile = _calib+conf.pmhofor
    if file_test(tmpfile, /REGULAR) gt 0 then begin
        tmp = readfits(tmpfile, h, /SIL)
        id = where(transpose(strmid(h, 0, 8)) eq 'TRACKNUM')
        _pmhoforTN = stregex(h[id], '[0-9]+_[0-9]+',/EX)
    endif else _pmhoforTN = 'None'
    (*!AO_STATUS.CONFIGURATION).pmhofor = _pmhoforTN

    tmpfile = _calib+conf.pmfor
    if file_test(tmpfile, /REGULAR) gt 0 then begin
        tmp = readfits(tmpfile, h, /SIL)
        id = where(transpose(strmid(h, 0, 8)) eq 'TRACKNUM')
        _pmforTN = stregex(h[id], '[0-9]+_[0-9]+',/EX)
    endif else    _pmforTN = 'None'
    (*!AO_STATUS.CONFIGURATION).curr2modes = _pmforTN

    tmpfile = _calib+conf.pmz
    if file_test(tmpfile, /REGULAR) gt 0 then begin
        tmp = readfits(tmpfile, h, /SIL)
        id = where(transpose(strmid(h, 0, 8)) eq 'TRACKNUM')
        _pmzTN = stregex(h[id], '[0-9]+_[0-9]+',/EX)
    endif else    _pmzTN = 'None'
    (*!AO_STATUS.CONFIGURATION).pmz = _pmzTN

    tmpfile = _calib+conf.pmpos
    if file_test(tmpfile, /REGULAR) gt 0 then begin
        tmp = readfits(tmpfile, h, /SIL)
        id = where(transpose(strmid(h, 0, 8)) eq 'TRACKNUM')
        _pmposTN = stregex(h[id], '[0-9]+_[0-9]+',/EX)
    endif else    _pmposTN = 'None'
    (*!AO_STATUS.CONFIGURATION).pos2modes = _pmposTN

    tmpfile = _calib+conf.ff_matrix
    if (file_test(tmpfile, /REGULAR) gt 0) and (total(adsec.ff_matrix) gt 0) then begin
        tmp = readfits(tmpfile, h, /SIL)
        id = where(transpose(strmid(h, 0, 8)) eq 'TRACKNUM')
        _pmffTN = stregex(h[id], '[0-9]+_[0-9]+',/EX)
    endif else _pmffTN = 'None'
    (*!AO_STATUS.CONFIGURATION).ff_matrix = _pmffTN

    elec = read_ascii_structure(_conf+_shell+'/elec.txt')
    
    Version = '1.00' 

    print, 'AO Supervisor - Adaptive secondary configuration utility - Version: ' + version
    print, 'Unit: '+_unit
    print, 'Shell: '+_shell
    print, 'Unit: '+_side
    print, '--------------------------------------------------------------------------'
    ans = displayMain(elec, conf, act_wo_pos, act_wo_curr, act_wo_icc, _pmffTN, _pmzTN,  _pmhoforTN, _pmforTN, _pmposTN)
    
    
    return

end
