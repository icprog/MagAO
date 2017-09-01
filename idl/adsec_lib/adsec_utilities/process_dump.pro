Function test_threshold, family_params, buf, strout = strout, frame0=frame0

    on_error, 2
    @adsec_common
    fm = family_params[0].family
    sbuf = size(buf, /dim)
    not_ena = where(family_params.enabled eq 0)
    amin_val = (family_params.alarm_min)
    if not_ena[0] ne -1 then amin_val[not_ena] = -1e10
    amax_val = (family_params.alarm_max)
    if not_ena[0] ne -1 then amax_val[not_ena] =  1e10
    amin = rebin(amin_val, sbuf, /sam)
    amax = rebin(amax_val, sbuf, /sam)
    idmi = where(buf lt amin, a1) 
    idma = where(buf gt amax, a2)
    list = [-1]
    if a1 gt 0 then list = [list,idmi]
    if a2 gt 0 then list = [list,idma]
    if n_elements(list) eq 1 then begin
        strout = "No variables in Alarm for family: "+fm[0]
    endif else begin

        list = list[1:*]
        frame_c0 = list[0] / adsec.n_actuators
        act_c = -1
        frame_c = frame_c0
        for i=0L, n_elements(list)-1 do begin
            if (list[i] / adsec.n_actuators) ne frame_c0 then break
            act_c = [act_c, list[i] mod adsec.n_actuators]
        endfor
        err = intersection(act_c, adsec.act_w_cl, cl_act_clist)
        if n_elements(cl_act_clist) eq 0 then begin
            err = intersection(act_c, adsec.act_wo_cl, ol_act_clist)
            if err ne 0 then  strout = "Fam: "+fm+" in Alarm but unable to find the reason"                                  $
                        else  strout = "Fam: "+fm+" in Alarm over indexes: "+strjoin(string(ol_act_clist, FORMAT='(I3.3)'), " ") $
                              + " (OL act: adjust FastDiagnostic thresholds)"
        endif else begin
            strout = "Fam: "+fm+" in Alarm over indexes: "+strjoin(string(cl_act_clist, FORMAT='(I3.3)'), " ")
        endelse

    endelse
    return, adsec_error.ok 

End



Function process_dump, filename, NOSAVE=nosave, NOUPDATE=noupdate, TEXT=text, DATA=data, TRACKNUM=tracknum

    on_error, 2
    @adsec_common
    t0 = systime(/sec)
    tempo = systime()
    text = ""
    if !d.name eq "WIN" then term= string([13b, 10b])else term = string([10B])
    tmptxt = "-- PROCESS DUMP START --"
    log_print, tmptxt
    text+=tmptxt+term
    tmptxt = tempo
    log_print, tmptxt
    text+=tmptxt+term
    if n_elements(filename) eq 0 then filename = ""
    if n_elements(tracknum) gt 0 then begin
        ;file_search(getenv('ADOPT_LOG')+'/ADAPTIVE-SECONDARY_*20100606_184737*')
        NOSAVE=1
    endif
    dump_av = 1;
    hkpr_av = 1;
    adam_av = 1;
    fast_av = 1

    ;GET BASIC DATA FROM DIAGNOSTICs
    tmptxt= "Get info from "+process_list.HOUSEKEEPER.MSGD_NAME+" variables..."
    err = getdiagnvalue(process_list.HOUSEKEEPER.MSGD_NAME , '*', -1,-1, hkpr_vars0)
    if err ne adsec_error.ok then begin
        hkpr_vars0 = 'Undefined'
        tmptxt+= "unable to get variable values: skipped"
    endif else begin
        tmptxt+= "done"
    endelse
    log_print, tmptxt
    text += tmptxt+term
    

    tmptxt= "Get info from "+process_list.MASTERDIAGNOSTIC.MSGD_NAME+" dump file: "+file_basename(filename)+'...'
    err = read_master_buffer_dump(filename, buffer)
    if err ne adsec_error.ok then begin
        dump_av = 0
        tmptxt+= "dump not availlable."
    endif else begin
        tmptxt+= "dump loaded."
    endelse
    log_print, tmptxt
    text += tmptxt+term

    tmptxt= "Get info from "+process_list.FASTDIAGN.MSGD_NAME+" variables..."
    err = getdiagnparam(process_list.FASTDIAGN.MSGD_NAME , '*', -1,-1, fast_params)
    if err ne adsec_error.ok then begin
        fast_av = 0
        fast_params = 'Undefined'
        tmptxt+= "unable to get variable values: skipped"
    endif else begin
        tmptxt+= "done"
    endelse
    log_print, tmptxt
    text += tmptxt+term

    tmptxt= "Get info from "+process_list.FASTDIAGN.MSGD_NAME+" variables..."
    err = getdiagnvalue(process_list.FASTDIAGN.MSGD_NAME , '*', -1,-1, fast_vars)
    if err ne adsec_error.ok then begin
        fast_av = 0
        fast_vars = 'Undefined'
        tmptxt+= "unable to get variable values: skipped"
    endif else begin
        tmptxt+= "done"
    endelse
    log_print, tmptxt
    text += tmptxt+term


    ;Self Diagnostic booleans
    adam_wd = 0;
    eth_wd = 0;
    dsp_wd = 0;
    pos_check = 0;
    curr_drift =0;
    pos_drift =0;
    temp_oor = 0;
    curr_oor =0;
    overcurrent =0;
    ;
    if dump_av and fast_av then begin

        nel = n_elements(buffer)
        pos = reform(buffer.crate_bcu.single_dsp_record.distaverage[*], adsec.n_actuators, nel)
        tt = where(finite(pos), comp=idnan)
        if idnan[0] ne -1 then begin
            actnan = idnan mod adsec.n_actuators
            tmptxt = 'WARNING: NaN value found on position dump on act '+strjoin(string(actnan, FORMAT='(I3.3)'), " ") + '. They will be forced to 0.'
            log_print, tmptxt
            text += tmptxt+term
            pos[idnan] = 0
        endif
        
        pos_red = pos[adsec.act_w_pos, *]

        pacc2raw_64 = reform((buffer.crate_bcu.single_dsp_record.distaccumulator2[*])[0:7, *, *, *], 8*28*6, nel)
        pacc2raw_16 = reform((buffer.crate_bcu.single_dsp_record.distaccumulator2[*])[8:11, *, *, *], 4*28*6, nel)
        paccraw_64 = reform(buffer.crate_bcu.single_dsp_record.distaccumulator[*], 8* 28*6, nel)
        acc_pos_b = double(long64(reform(paccraw_64, 2, adsec.n_actuators, nel),0, adsec.n_actuators, nel))*dsp_const.pos_fixed_point
        ;patched: not using bad values of buf16 (reason of that not known)
        acc2_pos_b = double(long64(reform(pacc2raw_64, 2, adsec.n_actuators, nel),0, adsec.n_actuators, nel))*(dsp_const.pos_fixed_point)^2
        tmpa = acc_pos_b[adsec.act_w_cl,*]
        tmpb = pos[adsec.act_w_cl, *]
        idss = where(tmpb gt 0)
        samples = round(median(tmpa[idss]/tmpb[idss]))
        ;samples = round(median(double(acc_pos_b[adsec.act_w_cl,*])/pos[adsec.act_w_cl, *]))
        ave_pos = acc_pos_b/double(samples)
        rms_pos = sqrt((acc2_pos_b/double(samples)-ave_pos^2) > 0)
 
        curr = reform(buffer.crate_bcu.single_dsp_record.curraverage[*], adsec.n_actuators, nel)
        tt = where(finite(curr), comp=idnan)
        if idnan[0] ne -1 then begin
            actnan = idnan mod adsec.n_actuators
            tmptxt= 'WARNING: NaN value found on force dump on act '+strjoin(string(actnan, FORMAT='(I3.3)'), " ") + '. They will be forced to 0.'
            log_print, tmptxt
            text += tmptxt+term
            curr[idnan] = 0
        endif

        curr_red = curr[adsec.act_w_curr, *]
        pacc2raw_64 = reform((buffer.crate_bcu.single_dsp_record.curraccumulator2[*])[0:7, *, *, *], 8*28*6, nel)
        pacc2raw_16 = reform((buffer.crate_bcu.single_dsp_record.curraccumulator2[*])[8:11, *, *, *], 4*28*6, nel)
        paccraw_64 = reform(buffer.crate_bcu.single_dsp_record.curraccumulator[*], 8* 28*6, nel)
        acc_cur_b = double(long64(reform(paccraw_64, 2, adsec.n_actuators, nel),0, adsec.n_actuators, nel))*dsp_const.curr_fixed_point
        ;patched: not using bad values of buf16 (reason of that not known)
        acc2_cur_b = double(long64(reform(pacc2raw_64, 2, adsec.n_actuators, nel),0, adsec.n_actuators, nel))*(dsp_const.curr_fixed_point)^2
        ave_curr = acc_cur_b/double(samples)
        rms_curr = sqrt((acc2_cur_b/double(samples)-ave_curr^2) > 0)


        slopes  =  buffer.switch_bcu.slopes
        nact    =  adsec.n_actuators
        dsp     =  buffer.crate_bcu.single_dsp_record

        MODES = reform(dsp.modes[*],nact, nel)
        NEWDELTACOMMAND = reform(dsp.newdeltacommand[*],nact, nel)
        FFCOMMAND = reform(dsp.ffcommand[*],nact, nel)
        FFPURECURRENT = reform(dsp.ffpurecurrent[*],nact, nel)
        DISTAVERAGE = reform(dsp.distaverage[*],nact, nel)
        ICC = reform(dsp.intcontrolcurrent[*], adsec.n_actuators, nel)

        wfscounter= buffer.switch_bcu.switch_bcu_header.(0)
        mircounter= buffer.switch_bcu.switch_bcu_header.(2)
        
        ;good frames selection: mircounter gt 0
        goodframes = where(mircounter gt 0)
        if goodframes[0] ne -1 then begin 
            family2test = ['CHCURRAVERAGE','CHDISTAVERAGE','CHINTCONTROLCURRENT', 'CHDISTRMS', 'CHCURRRMS', 'MODES', 'CHNEWDELTACOMMAND', 'CHFFCOMMAND', 'CHFFPURECURRENT']
            for i=0, n_elements(family2test)-1 do begin
                if family2test[i] eq 'CHCURRAVERAGE' then buf=curr[*, goodframes]
                if family2test[i] eq 'CHDISTAVERAGE' then buf=pos[*, goodframes]
                if family2test[i] eq 'CHINTCONTROLCURRENT' then buf=icc[*, goodframes]
                if family2test[i] eq 'CHDISTRMS' then buf=rms_pos[*, goodframes]
                if family2test[i] eq 'CHCURRRMS' then buf=rms_curr[*, goodframes]
                if family2test[i] eq 'MODES' then buf=modes[*, goodframes]
                if family2test[i] eq 'CHNEWDELTACOMMAND' then buf=newdeltacommand[*, goodframes]
                if family2test[i] eq 'CHFFCOMMAND' then buf=ffcommand[*, goodframes]
                if family2test[i] eq 'CHFFPURECURRENT' then buf=ffpurecurrent[*, goodframes]
                id  = where(strmatch(fast_params.family, family2test[i], /FOLD))
                err = test_threshold(fast_params[id], buf, strout = strout, frame0=tmp)
                if n_elements(tmp) gt 0 then frame0 = tmp
                if n_elements(frame0) gt 0 then frame0 = min([frame0, tmp])
                log_print, strout
                text += strout+term
            endfor
        endif else begin

            text += "No good frames on dump. Analysis skipped."+term            

        endelse

        
    endif else begin

        if ~fast_av then begin
            tmptxt += "Fastdiagnostic data not available for processing the dump. Skipped."
            log_print, tmptxt
            text+=tmptxt+term
        endif
    endelse

    wait, 2
    tmptxt= "Get info from "+process_list.HOUSEKEEPER.MSGD_NAME+" variables configuration..."
    err = getdiagnparam(process_list.HOUSEKEEPER.MSGD_NAME , '*', -1,-1, hkpr_params)
    if err ne adsec_error.ok then begin
        hkpr_av = 0
        hkpr_vars = 'Undefined'
        tmptxt+= "unable to get variable values: skipped"
    endif else begin
        tmptxt+= "done"
    endelse
    log_print, tmptxt
    text += tmptxt+term

    tmptxt = "Get info from "+process_list.ADAMHOUSEKEEPER.MSGD_NAME+" variables..."
    err = getdiagnparam(process_list.ADAMHOUSEKEEPER.MSGD_NAME , '*', -1,-1, adam_params)
    if err ne adsec_error.ok then begin
        adam_av = 0
        adam_params = 'Undefined'
        tmptxt += "unable to get variable values: skipped"
    endif else begin
        tmptxt += "done"
    endelse
    log_print, tmptxt
    text += tmptxt+term


    t1 = systime(/sec)
    if (t1-t0) lt 3. then begin
        log_print, "Waiting for diagnostic updates.."
        wait, 3-(t1-t0)
    endif

    tmptxt= "Get info from "+process_list.HOUSEKEEPER.MSGD_NAME+" variables..."
    err = getdiagnvalue(process_list.HOUSEKEEPER.MSGD_NAME , '*', -1,-1, hkpr_vars)
    if err ne adsec_error.ok then begin
        hkpr_av = 0
        hkpr_vars = 'Undefined'
        tmptxt+= "unable to get variable values: skipped"
    endif else begin
        tmptxt+= "done"
    endelse
    log_print, tmptxt
    text += tmptxt+term

    tmptxt= "Get info from "+process_list.ADAMHOUSEKEEPER.MSGD_NAME+" variables..."
    err = getdiagnvalue(process_list.ADAMHOUSEKEEPER.MSGD_NAME , '*', -1,-1, adam_vars)
    if err ne adsec_error.ok then begin
        adam_av = 0
        adam_vars = 'Undefined'
        tmptxt+= "unable to get variable values: skipped"
    endif else begin
        tmptxt+= "done"
    endelse
    log_print, tmptxt
    text += tmptxt+term


    if dump_av then outfile = file_dirname(filename)+'/ADAPTIVE-SECONDARY-DIAGNVARS'+stregex(file_basename(filename), '_.*\.', /EX)+'sav' else $
                   outfile =  add_timestamp((getenv('ADOPT_LOG')+'/ADAPTIVE-SECONDARY-DIAGNVARS_'+string(systime(/sec), format='(I14.14)')+'.sav'), /app)
    if ~keyword_set(NOSAVE) then save, file=outfile, hkpr_vars0, hkpr_vars, adam_vars, hkpr_params, fast_params, adam_params, fast_vars


    ;ADAM WATCHDOG EXPIRED
    if adam_av then begin
        idb = where(strmatch(adam_vars.family, 'ADAMWATCHDOGEXP', /FOLD))
        if (adam_vars.last)[idb] eq 1 then begin
            tmptxt = "Adam watchdog expired: check ethernet connection."
        endif else begin
            tmptxt = "Adam watchdog not expired."
        endelse
    endif
    log_print, tmptxt
    text += tmptxt+term

    if hkpr_av then begin
    ;WATCHDOG on SWITCHBCU
    ;ETHERNET WATCHDOG (da testare unita' b)
    ;the watchdog is expired if both SWITCHWFSGLOBALTIMEOUTCNT and SWITCHMIRRFRAMESCOUNTER are stucked.
        idsw_gt  = where(strmatch(hkpr_vars.family, 'SWITCHWFSGLOBALTIMEOUTCNT', /FOLD))
        idsw_mfc = where(strmatch(hkpr_vars.family, 'SWITCHMIRRFRAMESCOUNTER', /FOLD))
        if (idsw_gt[0] eq -1) or (idsw_mfc[0] eq -1) then begin
            tmptxt = "Ethernet connection watchdog on Switch BCU skipped: no data available."
        endif else begin
            dgt  = (hkpr_vars.last[idsw_gt])-(hkpr_vars0.last)[idsw_gt]
            dmfc = (hkpr_vars.last[idsw_mfc])-(hkpr_vars0.last)[idsw_mfc]
            if dgt eq 0 and dmfc eq 0 then begin
                tmptxt= "Ethernet watchdog expired on Switch BCU: check ethernet connection."
            endif else begin
                tmptxt= "Ethernet connection watchdog on Switch BCU not expired."
            endelse
        endelse
        log_print, tmptxt
        text += tmptxt+term
        
    ;CRATEBCU
    ;delta of DSPGLOBALCOUNTER has to be zero. The one with minimum value is the one in which the watchdog expired.
    ;confrontare su hkpr_vars hkpr_vars0 i dsp global counter. Se sono uguali, un crate watchdog e' scattato. Da verificare che il crate che l'ha fatto scattare ha un "delta" piu'grande (si e' fermato leggermente prima <70 counts) DA VERIFICARE UNITA" B
        id_gc = where(strmatch(hkpr_vars.family, 'DSPGLOBALCOUNTER', /FOLD))
        gc  = (hkpr_vars.last)[id_gc]
        gc0 = (hkpr_vars0.last)[id_gc]
        wexp = where((gc-gc0) eq 0)/(adsec.n_board_per_bcu*adsec.n_dsp_per_board)
        wexp = wexp[uniq(wexp)]
        if total(gc-gc0) eq 0 then begin
            
            tmptxt = "Ethernet watchdog expired on Crate BCU #:"+string(wexp, FORMAT='(I2.2)')+" check ethernet connection."
        endif else begin
            tmptxt = "Ethernet connection watchdog on Crate BCU not expired."
        endelse
        log_print, strjoin(tmptxt, term)
        text += tmptxt+term


	;Get global counters twice, to check running interrupts on DSPs
    tmptext=''
    err_gc0= get_global_counter(sc.all, gc0)
    err_gc1=get_global_counter(sc.all, gc1)
    if (err_gc0 ne adsec_error.ok or err_gc1 ne adsec_error.ok ) then begin
        tmptext += 'Error reading global counters, possibly DSP stucked'
    endif else begin
        gcdiff=gc1-gc0
        dspstopped=where(gcdiff eq 0, gccount)
        if gccount gt 0 then begin
            if gccount eq adsec.n_actuators then begin
                tmptext+='Global counters are not progressing on ALL DSPs'
            endif else begin
                tmptext+='Global counters are not progressing over DSP indexes: '+$
                         strjoin(string(dspstopped, FORMAT='(I3.3)'), " ")
            endelse
        endif else begin
            tmptext+='All global counters are progressing'
        endelse
    endelse
    log_print, tmptext
    text += tmptext+term
            

        ;DSP WATCHDOG
        idb = where(strmatch(hkpr_vars.family, '*Expired*', /FOLD))
        if max((hkpr_vars.last)[idb]) gt 0 then begin
            val = (hkpr_vars.last)[idb]
            nval = [transpose(val[0:(adsec.n_board_per_bcu*adsec.n_crates*adsec.n_dsp_per_board)/2-1]), $
                    transpose(val[(adsec.n_board_per_bcu*adsec.n_crates*adsec.n_dsp_per_board)/2:*])]
            nval = reform(nval, adsec.n_board_per_bcu*adsec.n_crates*adsec.n_dsp_per_board)
            wd_fault = where(nval gt 0, cc)
            dlist = strjoin(string(wd_fault, FORMAT='(I3.3)'), " ")
            ccs = string(cc, format='(I3.3)')
            if cc eq 168 then begin
                dlist = 'All'
                ccs = 'All'
                ext = term
            endif else begin
                ext = '- Consider the dsp boards replacement'
            endelse
            
            tmptxt = "Watchdog expired for DSP ID: "+dlist+'(#'+ccs+')'+ext
        endif else begin
            tmptxt = "DSP board watchdog not expired."
        endelse
        log_print, tmptxt
        text += tmptxt+term
    
        ;POS SENSORs JUMP
        idb = where(strmatch(hkpr_vars.family, 'dspposcheckcnt', /FOLD))
        if idb[0] eq -1 then begin
                tmptxt = "Distance theshold check skipped because no data are available."
        endif else begin
            if max((hkpr_vars.last)[idb]) gt 0 then begin
                val = (hkpr_vars.last)[idb]
                pos_fault = where(val gt 0, cc)
                dlist = strjoin(string(pos_fault, FORMAT='(I3.3)'), " ")
                occ_dlist = strjoin(strtrim(ulong(val[pos_fault]),2))
                tmptxt = "Distance threshold check analysis result:"
                log_print, tmptxt
                text += tmptxt+term
                for i=0, n_elements(dlist)-1 do begin
                    tmptxt = "Not stable capacitive sensor on CH ID: "+dlist[i]+" (times: "+occ_dlist[i]+")"
                    log_print, tmptxt
                    text += tmptxt+term
                endfor
                
                tmptxt = "Consider to remove the actuators with highest fault frequency from the close loop list."
                log_print, tmptxt
                text += tmptxt+term
                tmptxt = "Consider to remove the other detected actuators from the integrator control loop list."
            endif else begin
                tmptxt = "Distance theshold check passed with no issues found."
            endelse
        endelse
        log_print, tmptxt
        text += tmptxt+term

        ;TEMPERATUREs faults
        idb = where(strmatch(hkpr_vars.family, '*temp*', /FOLD))
        if max((hkpr_vars.status)[idb]) gt 0 then begin
            fam = (hkpr_vars.family)[idb]
            idx = (hkpr_vars.index)[idb]
            val = (hkpr_vars.last)[idb]
            alarm = where((hkpr_vars.status)[idb] eq 2)
            warn = where((hkpr_vars.status)[idb] eq 1)
            tmptxt= "Temperature analysis result:"
            log_print, tmptxt
            text += tmptxt+term
            
            if alarm[0] ne -1 then $
                for i=0, n_elements(alarm)-1 do begin
                    tmptxt = "Alarm temperature value of: "+fam[alarm[i]]+" - index: "+strtrim(idx[alarm[i]],2)+" - value [C deg]="+strtrim(val[alarm[i]],2)
                    log_print, tmptxt
                    text += tmptxt+term
                endfor
            if warn[0] ne -1 then $
            for i=0, n_elements(warn)-1 do begin
                tmptxt= "Warn temperature value of: "+fam[warn[i]]+" - index: "+strtrim(idx[warn[i]],2)+" - value [C deg]="+strtrim(val[warn[i]],2)
                    log_print, tmptxt
                    text += tmptxt+term
                endfor
            tmptxt= "Consider to fix the temperature ranges in houskeeper process or substitute the PT sensors."
        endif else begin
            tmptxt = "Temperature check passed with no issues found."
        endelse
        log_print, tmptxt
        text += tmptxt+term

    endif
    ;DUMP ANALYSIS

    if dump_av then begin
        ;OVERCURRENT probability -> (see log twiki in mountain for formula)
        na = adsec.n_actuators
        nel = n_elements(buffer)
        crate_curr = fltarr(nel,6)
        tot_curr = reform(total(sqrt(curr^2/(rebin(adsec.coil_res, na, nel, /sam)*rebin(adsec.act_efficiency, na, nel, /sam)^2)), 1) )
        ncact = adsec.n_actuators/adsec.n_crates
        for i=0, adsec.n_crates-1 do begin 
            ncl = indgen(ncact)+i*ncact
            crate_curr[*,i] = reform(total(sqrt(curr[ncl,*]^2/(rebin(adsec.coil_res[ncl], ncact, nel,/SA)*rebin(adsec.act_efficiency[ncl], ncact, nel, /SA)^2)), 1) )
        endfor
        if hkpr_av then begin
            ;pos_limit -> BCUTOTALCURRENTTHRESHOLDPOS
            idpos = where(strmatch(hkpr_vars.family, 'BCUTOTALCURRENTTHRESHOLDPOS', /FOLD))
            pos_limit = hkpr_vars[idpos].last
            pos_limit = pos_limit[0]
            idneg = where(strmatch(hkpr_vars.family, 'BCUTOTALCURRENTTHRESHOLDNEG', /FOLD))
            neg_limit = hkpr_vars[idneg].last
            neg_limit = neg_limit[0]
            idcr  = where(strmatch(hkpr_vars.family, 'BCULOCALCURRENTTHRESHOLD', /FOLD))
            cr_limit = hkpr_vars[idcr].last
            push = float(rebin(curr[adsec.act_w_cl, *], 1, nel) lt 0)
            pull = float(~push)
        
            tmp =  double(rebin(pos[adsec.act_w_cl, *], 1, nel)) - mean(adsec.d0/2)
            mpos = tmp / max(tmp)
            
            idd = where(mpos lt 0, cc)
            if cc gt 0 then mpos[idd] = 0

            p_tc_push = (tot_curr/pos_limit)*mpos * push *100 
            p_tc_pull = tot_curr/neg_limit *mpos * pull *100
            p_c = crate_curr/rebin(transpose(cr_limit), nel, adsec.n_crates, /SAM)*100 * rebin(transpose(mpos), nel, adsec.n_crates,/SAM)
            idt1= where(p_tc_push gt 100, cc)
            idt2= where(p_tc_pull gt 100, cc)
            idt3= where(p_c gt 100, cc)
            idt = [idt1,idt2, idt3]
            idt = idt[uniq(idt)]
            tmp = where(idt eq -1, cc, COMP=compl)
            if compl[0] ne -1 then idt = idt[compl] else idt = -1
            if n_elements(frame0) eq 0 then frame0 = idt[0]
            if frame0 ne -1 then begin
                p_tc_push[frame0:*] = 0
                p_tc_pull[frame0:*] = 0
                p_c[frame0:*] = 0
            endif
            
            
            tmptxt= "OVERCURRENT probability for TOTAL PUSH CURRENT: "+string(max(p_tc_push), format='(F5.1)')+'%'
            log_print, tmptxt
            text += tmptxt+term
            tmptxt= "OVERCURRENT probability for TOTAL PULL CURRENT: "+string(max(p_tc_pull), format='(F5.1)')+'%'
            log_print, tmptxt
            text += tmptxt+term
            tmptxt= "OVERCURRENT probability for SINGLE CRATE TOTAL CURRENT: "+string(max(p_c), format='(F5.1)')+'%'

        endif else begin
            tmptxt= "Unable to test OVERCURRENT: housekeeper variables not availlable."
        endelse
        log_print, tmptxt
        text += tmptxt+term
    endif 
    ;compare with values read by housekeeper -> occhio: quando disabilito le correnti teoriche vanno al massimo....
    ;give a percentage
    
    tmptxt= "-- PROCESS DUMP END --"
    log_print, tmptxt
    text += tmptxt+term
;    log_print, text
    if dump_av then begin
        data = {                        $
                 pos:pos,               $
                 rms_pos:rms_pos,       $
                 curr:curr,             $
                 rms_curr:rms_curr,     $
                 modes:modes,           $
                 ndc:newdeltacommand,   $
                 ffcmd:ffcommand,       $
                 ffpure:ffpurecurrent,  $ 
                 icc:icc,               $
                 wfsc: wfscounter,      $
                 mirc: mircounter,      $
                 goodframes:goodframes  $ 
        }
    endif
    ;write RTDB var --> display in a GUI
    if !AO_CONST.side_master and sc.name_comm ne "Dummy" and ~keyword_set(NOUPDATE) then begin
        st = strupcase(strmid(!AO_CONST.telescope_side,0,1))
        ;err = write_var('LAST_FAULT_ANALYSIS', text)
        if err ne adsec_error.ok then return, err
    endif

    
    return, adsec_error.ok



End

