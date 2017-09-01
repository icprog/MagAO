Function compare_flash, list, filename, buf_byte, fullbuf_byte                            $
                       , DEFAULT_LOGIC=default_logic_kw, USER_LOGIC=user_logic_kw         $
                       , DEFAULT_PROGRAM=default_program_kw, USER_PROGRAM=user_program_kw $
                       , BCU=bcu, SWITCHB=switchb, SIGGEN=siggen, SAVEF=savefile, SHOWIDX=showidx, BACKUP=backup

    on_error, 2
    undefine, buf_byte, fullbuf_byte
    @adsec_common
    default_logic = {                  $
        start_address: 0L,             $
        len: '180000'xl/4              $
    }
    default_program = {                                                          $
        start_address: default_logic.start_address+default_logic.len,            $
        len: '40000'xl/4                                                         $
    }
    user_program = {                                                             $
        start_address: default_program.start_address+default_program.len ,       $
        len: '40000'xl/4                                                         $
    }
    user_logic = {                                                               $
        start_address: user_program.start_address+user_program.len ,             $
        len: '180000'xl/4                                                        $
    }
    unused_area0 = {                                                             $
        start_address: user_logic.start_address+user_logic.len ,                 $
        len: '70000'xl/4                                                         $
    }
    conf_area = {                                                                $
        start_address: unused_area0.start_address+unused_area0.len ,             $
        len: 'A000'xl/4                                                          $
    }
    unused_area1 = {                                                             $
        start_address: conf_area.start_address+conf_area.len ,                   $
        len: '6000'xl/4                                                          $
    }

    if keyword_set(backup) then begin
	if n_elements(filename) eq 0 then filename = 'logicbackup.bin'
	filename = meas_path('logics')+filename
        if keyword_set(USER_PROGRAM_kw) then begin
            addr = user_program.start_address
            len = user_program.len
        endif
        if keyword_set(USER_LOGIC_kw) then begin
            addr = user_logic.start_address
            len = user_logic.len
        endif
        
        if list[0] eq sc.all then list=0
        err = read_seq_board(list[0], addr, long(len), fullbuf, BCU=BCU, SWITCHB=switchb, SIGGEN=siggen, SET_PM = bcu_datasheet.flash_mem_id, /ULONG)
        openw, unit, filename+'.downloaded', /GET_LUN
        fullbuf_byte = byte(fullbuf, 0, n_elements(fullbuf)*4)
        writeu, unit, fullbuf_byte
        free_lun, unit
        print, filename+'.downloaded file written'
        print, "saved"
	return, 0
    endif


    if n_elements(filename) eq 0 then filename=dialog_pickfile()
    master_buf = read_binary(filename)
    nel = n_elements(master_buf)
    len = ceil(nel/4d)
    print, list[0]
    

    if keyword_set(DEFAULT_LOGIC_kw) then begin

        err = read_seq_board(list, default_logic.start_address, long(len), buf, BCU=BCU, SWITCHB=switchb, SIGGEN=siggen, SET_PM = bcu_datasheet.flash_mem_id, /ULONG)
        if err ne adsec_error.ok then return, err

    endif 

    if keyword_set(USER_LOGIC_kw) then begin

        err = read_seq_board(list, user_logic.start_address, long(len), buf, BCU=BCU, SWITCHB=switchb,  SIGGEN=siggen,SET_PM = bcu_datasheet.flash_mem_id, /ULONG)
        if err ne adsec_error.ok then return, err

    endif



    if keyword_set(DEFAULT_PROGRAM_kw) then begin

        err = read_seq_board(list, default_program.start_address, long(len), buf, BCU=BCU, SWITCHB=switchb,  SIGGEN=siggen,SET_PM = bcu_datasheet.flash_mem_id, /ULONG)
        if err ne adsec_error.ok then return, err

    endif

    if keyword_set(USER_PROGRAM_kw) then begin

        err = read_seq_board(list, user_program.start_address, long(len), buf, BCU=BCU, SWITCHB=switchb, SIGGEN=siggen, SET_PM = bcu_datasheet.flash_mem_id, /ULONG)
        if err ne adsec_error.ok then return, err

    endif


    sbuf = size(buf, /DIM)
    if n_elements(sbuf) eq 1 then begin

        buf_byte = byte(buf, 0, sbuf[0]*4)
        master_buf_reb = master_buf

    endif else begin

        buf_byte = byte(buf, 0, sbuf[0]*4, sbuf[1])
        master_buf_reb = rebin(master_buf, nel,sbuf[1], /SAM)
        buf_byte = buf_byte[0:nel-1, *]

    endelse


   ; buf_byte = buf_byte[0:nel-1, *]
    if total(minmax(float(buf_byte)-float(master_buf_reb)) eq [0,0]) eq 2 then begin
        print, "Logic/program matching verified."
      ;  return, adsec_error.ok
    endif else begin
        idx = where(float(buf_byte)-float(master_buf_reb) ne 0, count)
        print, "Logic/program matching failed in # "+ strtrim(count, 2)
        if keyword_set(SHOWIDX) then print, idx
        
    endelse

    if keyword_set(savefile) then begin
        if keyword_set(USER_PROGRAM_kw) then begin
            addr = user_program.start_address
            len = user_program.len
        endif
        if keyword_set(USER_LOGIC_kw) then begin
            addr = user_logic.start_address
            len = user_logic.len
        endif
        
        if list[0] eq sc.all then list=0
        err = read_seq_board(list[0], addr, long(len), fullbuf, BCU=BCU, SWITCHB=switchb, SIGGEN=siggen, SET_PM = bcu_datasheet.flash_mem_id, /ULONG)
        openw, unit, filename+'.downloaded', /GET_LUN
        fullbuf_byte = byte(fullbuf, 0, n_elements(fullbuf)*4)
        writeu, unit, fullbuf_byte
        free_lun, unit
        print, "saved"
    endif

    return, adsec_error.ok
    
End

;la logica e' paddata con FF mentre il bin con 00
