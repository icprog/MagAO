; $Id: download_program.pro,v 1.11 2008/04/08 09:28:05 labot Exp $
;+
;
; DOWNLOAD_PROGRAM
;
; This function resets the DSPs and downloads the program to the DSP memory
; of the DSP boards.
;
; err = download_program(first_dsp, last_dsp, filename)
;
; INPUTS
;
;   first_dsp:       byte scalar. First DSP to which the program is sent.
;
;   last_dsp :       byte scalar. Last DSP to which the program is sent.
;
;   filename:        string scalar. Name of the file of the DSP program.
;
;
; OUTPUTS
;
;   err:             long-int scalar. (see adsec_error in init_gvar.pro)
;
; KEYWORDS
;
;
;   VERBOSE:         if set some informations are printed
;
;   NORESET:         if set the DSP are not reset (forced in idle state)
;                    before downloading the program.
;
;   NOSTART:         if set the DSP program is not started after the
;                    uploading.
;
;   BCU:             if set the program address the Crate BCU
;
;   SWITCHB:         if set the program address the SWITCH BCU
;
;
; HISTORY:
;
;       Thu Jun 24 18:07:21 1999, Armando Riccardi (AR)
;       Osservatorio Astrofisico di Arcetri, ITALY
;       <riccardi@riccardi.arcetri.astro.it>
;
;       13 Mar 2002, AR
;       The "dummy" communication is now handled.
;       SET_RESET, SET_IRQ and WPHL_RESULT keywords have been removed
;
;       11 May 2004, AR
;       Code totaly rewritten for LBT compatibility (AD Tiger-shark DSP)
;
;       13 July 2004, AR & M. Xompero
;       VERBOSE, NORESET and NOSTART keywords added.
;       code reorganized
;
;       07 Feb 2007 MX
;       LUT of variable deleted and name refurbished.
;       BCU and SWTICHB keywords added.
;-

pro download_program_comp_addr, the_databuf_startaddr, the_databuf_len, addr0, addr0len, addr1, addr1len, ACCEL=accel

@adsec_common

databuf_startaddr = long(the_databuf_startaddr)
databuf_len = long(the_databuf_len)
if keyword_set(accel) then begin
    map = acc_map 
    toadd = 2
endif else begin
    map = dsp_map
    toadd = 0
endelse

offset = long(map.adc_value)-databuf_startaddr


case 1B of
    (offset le -2): begin
        addr0=databuf_startaddr+toadd
        addr0len=databuf_len-toadd
        addr1len=0L
    end
    
    (offset eq -1): begin
        addr0=databuf_startaddr+1+toadd
        addr0len=databuf_len-1-toadd
        addr1len=0L
    end
    
    (offset eq  0): begin
        addr0=databuf_startaddr+2+toadd
        addr0len=databuf_len-2-toadd
        addr1len=0L
    end
    
    (offset ge  databuf_len): begin
        addr0=databuf_startaddr
        addr0len=databuf_len
        addr1len=0L
    end
    
    (offset eq  databuf_len-1): begin
        addr0=databuf_startaddr
        addr0len=databuf_len-1
        addr1len=0L
    end
    
    (offset eq  databuf_len-2): begin
        addr0=databuf_startaddr
        addr0len=databuf_len-2
        addr1len=0L
    end
    
    else: begin
        addr0=databuf_startaddr
        addr0len=offset
        addr1=dsp_map.adc_value+2
        addr1len=databuf_startaddr+databuf_len-addr1
    end
endcase
end

function download_program, first_dsp, last_dsp, filename                     $
                         , VERBOSE=verbose, NORESET=noreset, NOSTART=nostart $
                         , BCU=bcu, SWITCHB=switchb, SIGGEN=siggen, ACCEL=accel

    @adsec_common
    
    ;=================================================
    ; checks for the input parameters
    ;

    if test_type(first_dsp, /NOFLOAT, N_EL=n_el) then begin
        message, "first_dsp must be a short integer", CONT=(sc.debug eq 0B)
        return, adsec_error.input_type
    endif
    if n_el ne 1 then begin
        message, "first_dsp must be a scalar", CONT=(sc.debug eq 0B)
        return, adsec_error.input_type
    endif
    if first_dsp[0] gt adsec.last_dsp then begin
        message, "first_dsp must be a valid dsp number", CONT=(sc.debug eq 0B)
        return, adsec_error.input_type
    endif
    
    
    if test_type(last_dsp, /NOFLOAT, N_EL=n_el) then begin
        message, "last_dsp must be a short integer", CONT=(sc.debug eq 0B)
        return, adsec_error.input_type
    endif
    if n_el ne 1 then begin
        message, "last_dsp must be a scalar", CONT=(sc.debug eq 0B)
        return, adsec_error.input_type
    endif
    if first_dsp[0] gt adsec.last_dsp then begin
        message, "first_dsp must be a valid dsp number", CONT=(sc.debug eq 0B)
        return, adsec_error.input_type
    endif
    if last_dsp lt first_dsp then begin
        message, "last_dsp cannot be less then the first_one", $
                 CONT=(sc.debug eq 0B)
        return, adsec_error.input_type
    endif
    
    n_dsp = fix(last_dsp)-fix(first_dsp)+1
    dsp_list = indgen(n_dsp)+first_dsp
    
    if test_type(filename, /STRING, N_EL=n_el) then begin
        message, "filename must be a string", CONT=(sc.debug eq 0B)
        return, adsec_error.input_type
    endif
    if n_el ne 1 then begin
        message, "filename must be a scalar", CONT=(sc.debug eq 0B)
        return, adsec_error.input_type
    endif
    
    dummy = findfile(filename[0], COUNT=count)
    if count ne 1 then begin
        message, "filename must be an existing file", CONT=(sc.debug eq 0B)
        return, adsec_error.input_type
    endif
    
    ;
    ;**** end of the checks
    ;==============================================================

    if ~ keyword_set(noreset) then begin
        ;
        ; stop the program and set in idle the DSPs
        ;
        err = force_dsp_idle(dsp_list, SWITCHB=switchb, BCU=bcu, SIGGEN=siggen)
        if err ne adsec_error.ok then return, err

        f_info = file_info(filename[0])
        if not f_info.read or f_info.directory then begin
           message, "File not found.", CONT = sc.debug eq 0B
           return, adsec_error.IDL_INPUT_TYPE
        endif
    endif

    max_n_lines = file_lines(filename[0])
    n_lines = 0UL
    str =""
    ul = 0UL
    data = ulonarr(max_n_lines)

    openr, unit, filename[0], /GET_LUN
    for i=1UL,max_n_lines do begin
        readf, unit, str
        str = strtrim(str,2)
        if strcmp(str, '0x', 2) then begin
            reads, str, ul, FORMAT="('0x',Z)"
            data[n_lines]=ul
            n_lines += 1
        endif
    endfor
    free_lun, unit
    
    ; skip last unfilled words.
    data = data[0:n_lines-1]

    i = 0
    while (n_lines-i) gt 0 do begin
        
        case 1B of
            ; check for databuffer
            ((data[i] and 'F0000000'XUL) eq '40000000'XUL): begin
                databuf_len = (data[i] and '0FFFFFFF'XUL)
                i += 1
                                ; get databuffer start address
                databuf_startaddr = long(data[i])
                i += 1
                if keyword_set(verbose) then $
                  print, databuf_startaddr, databuf_len, FORMAT='("Data buffer addr:0x",Z8.8," len:",I)'
                ; send the databuffer
                ; do not perform the check if ADC
                ; locations are included
                download_program_comp_addr, databuf_startaddr, databuf_len, addr0, addr0len, addr1, addr1len, ACCEL=accel
                if addr0len gt 0 then begin
                    di = addr0-long(databuf_startaddr)
                    err = write_same_dsp(dsp_list, addr0, data[i+di:i+di+addr0len-1], /CHECK, SWITCHB=switchb, BCU=bcu, SIGGEN=siggen)
                    if err ne adsec_error.ok then return, err
                endif
                if addr1len gt 0 then begin
                    di = addr1-long(databuf_startaddr)
                    err = write_same_dsp(dsp_list, addr1, data[i+di:i+di+addr1len-1], /CHECK, SWITCHB=switchb, BCU=bcu, SIGGEN=siggen)
                    if err ne adsec_error.ok then return, err
                endif
                i += databuf_len
            end
            
                                ;check for a buffer to zero out
            ((data[i] and 'F0000000'XUL) eq '80000000'XUL): begin
                databuf_len = (data[i] and '0FFFFFFF'XUL)
                i += 1
                                ;get databuffer start address
                databuf_startaddr = long(data[i])
                i += 1
                
                if keyword_set(verbose) then $
                  print, databuf_startaddr, databuf_len, FORMAT='("Zero data buffer addr:",Z8.8," len:",I)'
                ; send a zero databuffer
                ; do not perform the check if ADC locations are included
                download_program_comp_addr, databuf_startaddr, databuf_len, addr0, addr0len, addr1, addr1len, ACCEL=accel
                if addr0len gt 0 then begin
                    di = addr0-long(databuf_startaddr)
                    err = write_same_dsp(dsp_list, addr0, ulonarr(addr0len), /CHECK, SWITCHB=switchb, BCU=bcu, SIGGEN=siggen)
                    if err ne adsec_error.ok then return, err
                endif
                if addr1len gt 0 then begin
                    di = addr1-long(databuf_startaddr)
                    err = write_same_dsp(dsp_list, addr1, ulonarr(addr1len), /CHECK, SWITCHB=switchb, BCU=bcu, SIGGEN=siggen)
                    if err ne adsec_error.ok then return, err
                endif
            end
            
                                ; check for a DSP code buffer (it's the last one)
            (data[i] eq '00000000'XUL): begin
                i += 1
                databuf_startaddr = 0L
                databuf_len = n_lines-i
                if keyword_set(verbose) then $
                  print, databuf_startaddr, databuf_len, FORMAT='("Last data buffer addr:",Z8.8," len:",I)'
                ; send last databuffer
                ; do not perform the check if ADC locations are included
                download_program_comp_addr, databuf_startaddr, databuf_len, addr0, addr0len, addr1, addr1len, ACCEL=accel
                if addr0len gt 0 then begin
                    di = addr0-long(databuf_startaddr)
                    err = write_same_dsp(dsp_list, addr0, data[i+di:i+di+addr0len-1], /CHECK, SWITCHB=switchb, BCU=bcu, SIGGEN=siggen)
                    if err ne adsec_error.ok then return, err
                endif
                if addr1len gt 0 then begin
                    di = addr1-long(databuf_startaddr)
                    err = write_same_dsp(dsp_list, addr1, data[i+di:i+di+addr1len-1], /CHECK, SWITCHB=switchb, BCU=bcu, SIGGEN=siggen)
                    if err ne adsec_error.ok then return, err
                endif
                i = n_lines     ;stops the loop
            end
            
            else: begin
                message, "Wrong DSP program format.", CONT=sc.debug eq 0B
                return, adsec_error.IDL_INPUT_TYPE
            end
        endcase
    endwhile
    
    if ~ keyword_set(nostart) then begin
        ; start the DSP program
        err = start_program(dsp_list, SWITCHB=switchb, BCU=bcu, SIGGEN=siggen)
        if err ne adsec_error.ok then return, err
    endif
    
    return, adsec_error.ok
end
