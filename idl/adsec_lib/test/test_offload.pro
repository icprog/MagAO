;$Id: test_offload.pro,v 1.1 2007/04/12 13:53:23 labot Exp $$
;+
;   NAME:
;       TEST_OFFLOAD
;
;   PURPOSE:
;    ROUTINE TEST FOR THE USE OF OFFLOAD COMMANDS AND OFFLOAD FORCES.
;
;   NOTE:
;
;   USAGE:
;    test_offload, CMD = cmd, FORCE=force, WFS_MODE=wfs_mode
;
;   INPUT:
;
;   OUTPUT:
;       None.
;
;   KEYWORDS:
;       CMD = enable offload commands
;       FORCE = enable offload forces
;       WFS_MODE = enable WFS frames
;
;
;    HISTORY
;
;   03  April 2007, Marco Xompero(MX)
;   marco@arcetri.astro.it 
;-

Pro test_offload, CMD = cmd, FORCE=force, WFS_MODE=wfs_mode


    @adsec_common
    no_th=1
    
    if ~keyword_set(WFS_MODE) then begin 
        if keyword_set(intmodes) then dummy = replicate(10e-6, adsec.n_actuators) else dummy = replicate(1e-6, adsec.n_actuators)
        if keyword_set(NO_TH) then dummy[indgen(672)] = 1e6
        dummy = replicate(1e6, adsec.n_actuators)
        err = set_skip_par(dummy, /MODE, /DELTA)
        if err ne adsec_error.ok then message, "Error encountered: "+strtrim(err,2)
        
        dummy = replicate(10e-6, adsec.n_actuators)
        dummy[adsec.dummy_act] = 1e6
        if keyword_set(NO_TH) then dummy[indgen(672)] = 1e6
        err = set_skip_par(dummy, /MODE, /MAX)
        if err ne adsec_error.ok then message, "Error encountered: "+strtrim(err,2)
        
        dummy = replicate(25e-6, adsec.n_actuators)
        dummy[adsec.dummy_act] = -1e6 
        if keyword_set(NO_TH) then dummy[indgen(672)] = -1e6
        err = set_skip_par(dummy, /CMD, /MIN)
        if err ne adsec_error.ok then message, "Error encountered: "+strtrim(err,2)
        
        dummy = replicate(110e-6, adsec.n_actuators)
        dummy[adsec.dummy_act] = 1e6
        if keyword_set(NO_TH) then dummy[indgen(672)] = 1e6
        err = set_skip_par(dummy, /CMD, /MAX)
        if err ne adsec_error.ok then message, "Error encountered: "+strtrim(err,2)
        
        if keyword_set(intmodes) then dummy = replicate(10e-6, adsec.n_actuators) else dummy = replicate(1e-6, adsec.n_actuators)
        dummy[adsec.dummy_act] = 1e6
        if keyword_set(NO_TH) then dummy[indgen(672)] = 1e6
        err = set_skip_par(dummy, /CMD, /DELTA)
        if err ne adsec_error.ok then message, "Error encountered: "+strtrim(err,2)
        
        dummy = replicate(-0.5, adsec.n_actuators)
        dummy[adsec.dummy_act] = -1e6
        if keyword_set(NO_TH) then dummy[indgen(672)] = -1e6
        err = set_skip_par(dummy, /FORCE, /MIN)
        if err ne adsec_error.ok then message, "Error encountered: "+strtrim(err,2)
        
        dummy = replicate(0.5, adsec.n_actuators)
        dummy[adsec.dummy_act] = 1e6
        if keyword_set(NO_TH) then dummy[indgen(672)] = 1e6
        err = set_skip_par(dummy, /FORCE, /MAX)
        if err ne adsec_error.ok then message, "Error encountered: "+strtrim(err,2)
        
        freq = 200
        period=1./freq
        err = set_acc(50,6)
        pbs =  2UL^6 or 2UL^7   ;(diagnostic SDRAM + FASTLINK ENABLE)
        err = write_same_dsp(0, rtr_map.switch_SCStartRTR, [0L,pbs], /SWITCH, /CHECK)
            if err ne adsec_error.ok then message, "Error encountered: "+strtrim(err,2)
            err = set_diagnostic(/MASTER)
            err = set_diagnostic(over=period)
            
    endif


    is_cmd = keyword_set(CMD)
    prevf = fltarr(adsec.n_actuators)
    prevc = fltarr(adsec.n_actuators)
    strc = "ChFFCommand" 
    strf = "ChCurrAverage"
    name = strf+' '+strc
    err = getdiagnvalue(process_list.fastdiagn.msgd_name, name, -1, -1, buf, TIME=2.)
    if err ne adsec_error.ok then print, "ERROR"
    prevf = exdiagbuf (strf, buf, /STD)
    prevc = exdiagbuf (strc, buf, /STD)
    prevf = fltarr(adsec.n_actuators)
    prevc = fltarr(adsec.n_actuators)




    for i=0, adsec.n_actuators-1 do begin
       
        vect = fltarr(adsec.n_actuators)
        val = 0.1 * randomn(seed, 1)
        vect[i] = val
        if is_cmd then err = set_offload_cmd(vect, /NOCHECK, /START) else err = set_offload_force(vect, /NOCHECK, /START)
        ;err = read_seq_dsp(0, switch_map.offloadselector, 1l, bb, /switch,/ ul)
        ;print, "Current Block #"+strtrim(bb,2)
        wait, 0.4
        err = getdiagnvalue(process_list.fastdiagn.msgd_name, name, -1, -1, buf, TIME=0.5)
        print, i
        
        if err ne 0 then print, error
        curr_buf_f = exdiagbuf (strf, buf,/STD)
        curr_buf_c = exdiagbuf (strc, buf, /STD)
        
        if is_cmd then begin
            err = read_seq_ch(sc.all, dsp_map.pos_command, 1l, curr_buf_c)
            id = where(curr_buf_c ne 0, cc)
            if (cc ne 1) or (id[0] ne i) then begin
                  print, "ACT #"+string(i, format='(I3.3)')
                  print, "Unmatches: ", strtrim(id,2)
            endif
        endif else begin
            id = where(curr_buf_f ne 0, cc)
            if (cc ne 1) or (id[0] ne i) then begin
                  print, "################ ACT #"+string(i, format='(I3.3)')
                  print, "################ Unmatches: ", strtrim(id,2)
            endif

        endelse

    endfor


    if ~keyword_set(WFS_MODE) then begin
        err = set_diagnostic(over=0.0)
        err = set_diagnostic(MASTER=0)
    endif



    return

End

