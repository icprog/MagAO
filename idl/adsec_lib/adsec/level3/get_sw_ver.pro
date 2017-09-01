;$Id: get_sw_ver.pro,v 1.7 2009/11/27 09:20:35 marco Exp $$
Function convert_dsp_ver, locv

    tmp = [ishft(locv,-8) AND 255 ,ishft(locv,0) AND 255]
    str = string(tmp, format='(z4.4)')
    reads, str, format = '(I)', a, b
    return, strtrim(fix(a),2)+'.'+string(b, format='(I2.2)')

End

Function convert_nios_ver, locv

    x=ulong64(locv) 
    tmp = [ishft(x,-24) AND 255, ishft(x,-16) AND 255, ishft(x,0) AND 65535]
    str = string(tmp, format='(z4.4)')
    reads, str, format = '(I)', a, b, c
    return, strtrim(fix(a), 2)+'.' +string(b, format='(I2.2)') +'.'+string(c, format='(I4.4)')

End

Function convert_logic_ver, locv

    x=ulong64(locv)     
    tmp = [ishft(x,-8) AND 255, ishft(x,0) AND 255]
    str = string(tmp, format='(z4.4)')
    reads, str, format = '(I)', a, b
    return, strtrim(fix(a), 2)+'.'+string(b, format='(I2.2)')

End






;+
;   NAME:
;    GET_SW_VER
;
;   PURPOSE:
;    Get the current release software version and eventually check them.
;
;   USAGE:
;    err = get_sw_ver(path, ver, /CHECK)
;
;   INPUT:
;    None.
;   
;   OUTPUT:
;    err:       Error code.
;    path:      DSP programs path.
;    ver:       Structure of all sw/fw versions.
;
;   KEYWORD:
;    CHECK:     Get data from AdSec electonics and compare them with the ones stored in common block.
;
;   HISTORY:
;    Created by Marco Xompero (MX) on 16 Apr 2007
;   17 May 2007 MX
;    Fixed some minor bug, fixed switch bcu manage. To do Siggen and accelerometers. 
;   19 Mar 2008, MX
;    Added variable "is_acc" in the SIGGEN field to discover ACCELEROMETERS board
;
;-

Function get_sw_ver, path, ver, CHECK=check, FWCHECK=fwcheck, DISPLAY=displ, PVR = prog_vers_read

    @adsec_common

    slow_diag_name = process_list.housekeeper.msgd_name

    sw_templ = {                                $
        dsp         :   "Undefined version",    $
        logic       :   "Undefined version",    $
        nios        :   "Undefined version"     $
    }
        
    sig_templ = {                               $
        dsp         :   "Undefined version",    $
        logic       :   "Undefined version",    $
        nios        :   "Undefined version",    $
        is_acc      :   0B                      $
    }

    crate_templ = {                                                 $
        crate_id    :   -1,                                         $
        bcu         :   sw_templ,                                   $
        siggen      :   sig_templ,                                  $
        dspboard    :   replicate(sw_templ, adsec.n_board_per_bcu)  $
    } 

    if ~keyword_set(CHECK) and ~keyword_set(FWCHECK) then begin

        prog_vers = {                                                    $
            Crate       :  replicate(crate_templ, adsec.n_crates),      $
            SwitchBcu   :  sw_templ                                     $
        }
    
        ;reading current release file
        readcol,adsec_path.conf+'last_release', cid, name, bid, pname, ver, format='I,A,I,A,A'
        ;fill prog_vers common block
 
        ;software version field number
        sw_fields = 3

        ;index of bcu, siggen and dsp location
        index_bcu = indgen(adsec.n_crates) * sw_fields * (adsec.n_board_per_bcu +2)
        index_siggen = index_bcu + sw_fields
        index_dsp = index_siggen + sw_fields
        index_switch = adsec.n_crates * sw_fields * (adsec.n_board_per_bcu +2)

        for i=0, adsec.n_crates-1 do begin

            prog_vers.crate[i].crate_id     = i
            prog_vers.crate[i].bcu.dsp      = ver[index_bcu[i]]
            prog_vers.crate[i].bcu.logic    = ver[index_bcu[i]+1]
            prog_vers.crate[i].bcu.nios     = ver[index_bcu[i]+2]

            prog_vers.crate[i].siggen.dsp   = ver[index_siggen[i]]
            prog_vers.crate[i].siggen.logic = ver[index_siggen[i]+1]
            prog_vers.crate[i].siggen.nios  = ver[index_siggen[i]+2]
            if name[index_siggen[i]] eq "ACCBOARD" then prog_vers.crate[i].siggen.is_acc  = 1B

            for j=0, adsec.n_board_per_bcu-1 do begin

                prog_vers.crate[i].dspboard[j].dsp   = ver[index_dsp[i]+j*sw_fields]
                prog_vers.crate[i].dspboard[j].logic = ver[index_dsp[i]+j*sw_fields+1]
                prog_vers.crate[i].dspboard[j].nios  = ver[index_dsp[i]+j*sw_fields+2]
                
            endfor

        endfor
        
        prog_vers.switchbcu.dsp      = ver[index_switch]
        prog_vers.switchbcu.logic    = ver[index_switch+1]
        prog_vers.switchbcu.nios     = ver[index_switch+2]

        return, adsec_error.ok

    endif else begin
        
        if keyword_set(DISPL) then begin
             dbg=sc.debug
             sc.debug=0
        endif

        if sc.name_comm eq "Dummy"  or (sc.host_comm eq 'OnlyCommunication') then return, adsec_error.ok
        ;Get data from HOUSEKEEPER and compare with the prog_vers structure

        prog_vers_read = {                                              $
            Crate       :  replicate(crate_templ, adsec.n_crates),      $
            SwitchBcu   :  sw_templ                                     $
        }
        
        err = getdiagnvalue(slow_diag_name, "BCULogicRelease", -1, -1, bcu_lr, TIM=5)
        if err ne adsec_error.ok then return, err

        err = getdiagnvalue(slow_diag_name, "BCUSoftwareRelease", -1, -1,  bcu_sr, TIM=5)
        if err ne adsec_error.ok then return, err
        
        err = getdiagnvalue(slow_diag_name, "BCUCrateId", -1, -1,  bcu_ci, TIM=5)
        if err ne adsec_error.ok then return, err
        
        ;TO RESTORE AFTER LORENZO MODIFICATION on HOUSEKEEPER - - - > DONE
        err = getdiagnvalue(slow_diag_name, "SwitchLogicRelease", -1, -1,  switch_lr, TIM=5)
        if err ne adsec_error.ok then return, err

        err = getdiagnvalue(slow_diag_name, "SwitchSoftwareRelease", -1, -1,  switch_sr, TIM=5)
        if err ne adsec_error.ok then return, err
        
        err = getdiagnvalue(slow_diag_name, "DSPLogicRelease", -1, -1,  dsp_lr, TIM=5)
        if err ne adsec_error.ok then return, err

        err = getdiagnvalue(slow_diag_name, "DSPSoftwareRelease", -1, -1,  dsp_sr, TIM=5)
        if err ne adsec_error.ok then return, err

        err = getdiagnvalue(slow_diag_name, "SGNLogicRelease", -1, -1,  sig_lr, TIM=5)   
        if err ne adsec_error.ok then return, err

        err = getdiagnvalue(slow_diag_name, "SGNSoftwareRelease", -1, -1,  sig_sr, TIM=5)
        if err ne adsec_error.ok then return, err

        err = read_seq_dsp(sc.all, bcu_map.SoftwareRelease, 1L, bcu_dp, /ULONG, /BCU)
        if err ne adsec_error.ok then return, err

        err = read_seq_dsp(sc.all, switch_map.SoftwareRelease, 1L, switch_dp, /ULONG, /SWITCH)
        if err ne adsec_error.ok then return, err

        err = read_seq_dsp(sc.all, dsp_map.Software_Release, 1L, dsp_dp, /ULONG)
        if err ne adsec_error.ok then return, err

        ;switch_lr = bcu_lr[0]
        ;bcu_lr = bcu_lr[1:*]
        ;switch_sr = bcu_sr[0]
        ;bcu_sr = bcu_sr[1:*]


        ;Fill logic versions of all BCU's from Housekeeper
        for i=0, adsec.n_crates-1 do begin

            prog_vers_read.crate[i].crate_id     = fix(bcu_ci[i].last)
            prog_vers_read.crate[i].bcu.dsp      = convert_dsp_ver(bcu_dp[i])
            prog_vers_read.crate[i].bcu.logic    = convert_logic_ver(bcu_lr[i].last)
            prog_vers_read.crate[i].bcu.nios     = convert_nios_ver(bcu_sr[i].last)

            if prog_vers.crate[i].siggen.is_acc then begin
                err = read_seq_dsp(i, dsp_map.Software_Release, 1L, sig_dp, /ULONG, /SIGGEN)
                if err ne adsec_error.ok then return, err
                prog_vers_read.crate[i].siggen.dsp   = convert_dsp_ver(sig_dp)
                prog_vers_read.crate[i].siggen.is_acc= 1
            endif
            prog_vers_read.crate[i].siggen.logic = convert_logic_ver(sig_lr[i].last)
            prog_vers_read.crate[i].siggen.nios  = convert_nios_ver(sig_sr[i].last)
        
        endfor

        ;Fill logic versions of SWITCH BCU
        prog_vers_read.switchbcu.dsp      = convert_dsp_ver(switch_dp[0])
        prog_vers_read.switchbcu.logic    = convert_logic_ver(switch_lr[0].last)
        prog_vers_read.switchbcu.nios     = convert_nios_ver(switch_sr[0].last)

        ;Fill logic versions of all DSP BOARDS from Housekeeper
        for i=0, adsec.n_crates*adsec.n_board_per_bcu-1 do begin
            
            cid = i / fix(adsec.n_board_per_bcu)
            bid = i mod adsec.n_board_per_bcu
            prog_vers_read.crate[cid].dspboard[bid].dsp   = convert_dsp_ver(dsp_dp[i])
            prog_vers_read.crate[cid].dspboard[bid].logic = convert_logic_ver(dsp_lr[i].last)
            prog_vers_read.crate[cid].dspboard[bid].nios  = convert_nios_ver(dsp_sr[i].last)

        endfor

        ;Version test
        count = 0
        for i=0, adsec.n_crates-1 do begin

            dummy = strtrim(prog_vers.crate[i].crate_id,2)
            if dummy ne "None" then begin
                if dummy ne  strtrim(prog_vers_read.crate[i].crate_id,2) then begin
                    log_print, 'Error checking Crate ID for Crate#' + string(prog_vers.crate[i].crate_id, format='(I2.2)') $
                       , log_lev=!AO_CONST.LOG_LEV_ERROR
                    if ~keyword_set(DISPL) then return, adsec_error.IDL_FW_MISMATCH
                endif
            endif

            if keyword_set(FWCHECK) then dummy='None' else dummy = prog_vers.crate[i].bcu.dsp 
            
            if dummy ne "None" then begin
                if dummy ne  prog_vers_read.crate[i].bcu.dsp then begin
                log_print, 'Error checking BCU for Crate#' + string(prog_vers.crate[i].crate_id, format='(I2.2)') $
                        +' in DSP Program version.' , log_lev=!AO_CONST.LOG_LEV_ERROR
                if ~keyword_set(DISPL) then return, adsec_error.IDL_FW_MISMATCH
                endif
            endif

            dummy = prog_vers.crate[i].bcu.logic
            if dummy ne "None" then begin
                if dummy ne  prog_vers_read.crate[i].bcu.logic then begin
                log_print, 'Error checking BCU for Crate#' + string(prog_vers.crate[i].crate_id, format='(I2.2)') $
                        +' in Logic version.', log_lev=!AO_CONST.LOG_LEV_ERROR
                if ~keyword_set(DISPL) then return, adsec_error.IDL_FW_MISMATCH
                endif
            endif

            dummy = prog_vers.crate[i].bcu.nios  
            if dummy ne "None" then begin
                if dummy ne  prog_vers_read.crate[i].bcu.nios then begin
                log_print, 'Error checking BCU for Crate#' + string(prog_vers.crate[i].crate_id, format='(I2.2)') $
                        +' in Nios version.', log_lev=!AO_CONST.LOG_LEV_ERROR
                if ~keyword_set(DISPL) then return, adsec_error.IDL_FW_MISMATCH
                endif
            endif

            if keyword_set(FWCHECK) then dummy='None' else dummy = prog_vers.crate[i].siggen.dsp 
            if dummy ne "None" then begin
                if dummy ne  prog_vers_read.crate[i].siggen.dsp then begin
                if prog_vers_read.crate[i].siggen.is_acc then lname = "ACCELEROMETER" else lname = "SIGGEN"
                log_print, 'Error checking '+lname+' for Crate#' + string(prog_vers.crate[i].crate_id, format='(I2.2)') $
                        +' in DSP Program version.' , log_lev=!AO_CONST.LOG_LEV_ERROR
                if ~keyword_set(DISPL) then return, adsec_error.IDL_FW_MISMATCH
                endif
            endif

            dummy = prog_vers.crate[i].siggen.logic
            if dummy ne "None" then begin
                if dummy ne  prog_vers_read.crate[i].siggen.logic then begin
                if prog_vers_read.crate[i].siggen.is_acc then lname = "ACCELEROMETER" else lname = "SIGGEN"
                log_print, 'Error checking '+lname+' for Crate#' + string(prog_vers.crate[i].crate_id, format='(I2.2)') $
                        +' in Logic version.' , log_lev=!AO_CONST.LOG_LEV_ERROR
                if ~keyword_set(DISPL) then return, adsec_error.IDL_FW_MISMATCH
                endif
            endif
            dummy = prog_vers.crate[i].siggen.nios  
            if dummy ne "None" then begin
                if dummy ne  prog_vers_read.crate[i].siggen.nios then begin
                if prog_vers_read.crate[i].siggen.is_acc then lname = "ACCELEROMETER" else lname = "SIGGEN"
                log_print, 'Error checking '+lname+' for Crate#' + string(prog_vers.crate[i].crate_id, format='(I2.2)') $
                        +' in Nios version.' , log_lev=!AO_CONST.LOG_LEV_ERROR
                if ~keyword_set(DISPL) then return, adsec_error.IDL_FW_MISMATCH
                endif
            endif

            for j=0, adsec.n_board_per_bcu-1 do begin

                if keyword_set(FWCHECK) then dummy='None' else dummy = prog_vers.crate[i].dspboard[j].dsp  
                if dummy ne "None" then begin
                    if dummy ne  prog_vers_read.crate[i].dspboard[j].dsp then begin
                    log_print, 'Error checking DSpBoard#'+ string(j, format='(I2.2)')              $
                            +' for Crate#' + string(prog_vers.crate[i].crate_id, format='(I2.2)') $
                            +' in DSP Program version.' , log_lev=!AO_CONST.LOG_LEV_ERROR
                    if ~keyword_set(DISPL) then return, adsec_error.IDL_FW_MISMATCH
                    endif
                endif

                dummy = prog_vers.crate[i].dspboard[j].logic 
                if dummy ne "None" then begin
                    if dummy ne  prog_vers_read.crate[i].dspboard[j].logic then begin
                    log_print, 'Error checking DSpBoard#'+ string(j, format='(I2.2)')              $
                            +' for Crate#' + string(prog_vers.crate[i].crate_id, format='(I2.2)') $
                            +' in Logic version.' , log_lev=!AO_CONST.LOG_LEV_ERROR
                    if ~keyword_set(DISPL) then return, adsec_error.IDL_FW_MISMATCH
                    endif
                endif

                dummy = prog_vers.crate[i].dspboard[j].nios 
                if dummy ne "None" then begin
                    if dummy ne  prog_vers_read.crate[i].dspboard[j].nios then begin
                    log_print, 'Error checking DSpBoard#'+ string(j, format='(I2.2)')              $
                            +' for Crate#' + string(prog_vers.crate[i].crate_id, format='(I2.2)') $
                            +' in Nios version.' , log_lev=!AO_CONST.LOG_LEV_ERROR
                    if ~keyword_set(DISPL) then return, adsec_error.IDL_FW_MISMATCH
                    endif
                endif
                
            endfor

        endfor


        if keyword_set(FWCHECK) then dummy='None' else dummy = prog_vers.switchbcu.dsp
        if dummy ne "None" then begin
            if dummy ne  prog_vers_read.switchbcu.dsp then begin
                log_print, 'Error checking SWITCHBCU in DSP Program version.' , log_lev=!AO_CONST.LOG_LEV_ERROR
                 if ~keyword_set(DISPL) then return, adsec_error.IDL_FW_MISMATCH
            endif
        endif
        dummy = prog_vers.switchbcu.logic
        if dummy ne "None" then begin
            if dummy ne  prog_vers_read.switchbcu.logic then begin
                log_print, 'Error checking SWITCHBCU in Logic version.' , log_lev=!AO_CONST.LOG_LEV_ERROR
                 if ~keyword_set(DISPL) then return, adsec_error.IDL_FW_MISMATCH
            endif
        endif
        dummy = prog_vers.switchbcu.nios
        if dummy ne "None" then begin
            if dummy ne  prog_vers_read.switchbcu.nios then begin
                log_print, 'Error checking SWITCHBCU in Nios version.' , log_lev=!AO_CONST.LOG_LEV_ERROR
                if ~keyword_set(DISPL) then return, adsec_error.IDL_FW_MISMATCH
            endif
        endif

       
    endelse

    if keyword_set(DISPL) then sc.debug=dbg

    return, adsec_error.ok
    

End
