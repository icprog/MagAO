; $Id: communication_test.pro,v 1.8 2007/02/07 16:57:56 marco Exp $

;+
;
; COMMUNICATION_TEST
;
; This function performs the communication test.
;
;     err = communication_test(test_mode, first_dsp, last_dsp, dsp_address, data_len $
;                              [, WORD_WRONG=word_wrong] $
;                              [,SEND_BUFFER=send_buffer] $
;                              [,RECEIVE_BUFFER=read_buffer] $
;                              [,SET_PM=set_pm][,ITER_NUMBER=n_iter][, /VERBOSE] $
;                              [,/RETRY_ON_ERROR][DSP|BOARD|SIGGEN|BCU])
;
;
; INPUTS
;
;   test_mode:       short-int scalar.
;                      0 = writing using WR_SAME and reading  RD_SEQ
;                      1 = writing using WR_SEQ and reading using RD_SEQ
;
;   first_id:        short-int scalar. First DSP/DSP board/SIGGEN board/BCU board ID to test.
;
;   last_id :        short-int scalar. Last DSP/DSP board/SIGGEN board/BCU board ID to test.
;
;   dsp_address:     long-int scalar. DSP/SDRAM/SRAM address from which
;                    the write-read test is performed.
;
;   data_len:        long-int scalar. Number of words per DSP to write and read
;                    for the communication test.
;
; OUTPUTS
;
;   err:             long-int scalar. (see adsec_error in init_gvar.pro)
;
; KEYWORDS
;
;   ITER_NUMBER:     int or long scalar. Number of write-read iterations to perform.
;
;   WORD_WRONG:      long-int scalar. Output. Number of wrong words found comparing the
;                    written and read buffer.
;
;   SEND_BUFFER:     named variable. ulong array[N,M]. output. Vector of random written data used in
;                    the test. N=data_len, M=last_id-first_id+1.
;
;   RECEIVE_BUFFER:  named variable. ulong array[N,M] output. Vector of data read from the DSPs.
;                    N=data_len, M=last_dsp-first_dsp+1. If this array
;                    doesn't match SEND_BUFFER a communication error occurred.
;
;   SET_PM:          byte scalar. Code identifing the type of memory to
;                    write to. Default value is dsp_datasheet.data_mem_id,
;                    it is used to read from DSP data memory. See
;                    dsp_datasheet structurefor allowed values
;
;   RETRY_ON_ERROR:  if set, when an error occurred, the buffer is read again until the read data match
;                    the already written ones or the maximum number of readings (10) is reached. Use this
;                    keyword to understand if the writing or reading process failed.
;
;   VERBOSE:         if set, on error some info is printed out
;
;   BCU:             set it to test a BCU
;
;   SIGGEN:          set it to test a SIGGEN
;
;   SWITCH:          set it to test a SWITCH_BCU
;
;   If not BCU nor SIGGEN nor SWITCH are defined, the test is performed on DSP boards. The target memory for the test
;   is defined by the SET_PM keyword
;
; HISTORY:
;
;       Thu Jun 24 18:07:21 1999, written by Armando Riccardi
;       <riccardi@arcetri.astro.it>
;
;       Jul 13 2000, Brusa & Riccardi
;       The test is now implemented without using the call to the AM2 communication test DLL
;
;       Jan 31 2001, A. Riccardi
;       The test in program memory has been implemented
;
;       Feb 12 2001, A. Riccardi
;       The keywords RETRY_ON_ERROR and VERBOSE have been added.
;       The value passed in the ITER_NUMBER can be a long-int,
;       not only a short-int.
;
;       13 Mar 2002, A. Riccardi
;       n_dsp > 256 is now handled. First_dsp and last_dsp can be short-int
;       scalars.
;
;       20 May 2004, AR
;       Modified to support new LBT electronics
;
;       26 Jan 2007, MX
;       Added BCU, SIGGEN and SWITCH_BCU keywords (TO CHECK)
;-

function communication_test, test_mode, first_dsp, last_dsp, dsp_address, data_len, $
                             ITER_NUMBER=n_iter, $
                             WORD_WRONG=word_wrong, $
                             SEND_BUFFER=write_buf, $
                             RECEIVE_BUFFER=read_buf, $
                             SET_PM=the_set_pm, VERBOSE=verbose, $
                             RETRY_ON_ERROR=retry_on_error, $
                             BCU = is_bcu, SIGGEN=is_siggen, SWITCH_BCU = switch_bcu

@adsec_common

;=================================================
; checks for the input parameters
;

if n_elements(the_set_pm) eq 0 then set_pm=0 else set_pm=the_set_pm
;datasheet = get_datasheet(BCU=is_bcu, SIGGEN=is_siggen)

if test_type(test_mode, /INT, N_EL=n_el) then begin
    message, "test_mode must be an integer", CONT=(sc.debug eq 0B)
    return, adsec_error.input_type
endif
if n_el ne 1 then begin
    message, "test_mode must be a scalar", CONT=(sc.debug eq 0B)
    return, adsec_error.input_type
endif
if total(test_mode eq [0,1]) ne 1 then begin
    message, "test_mode must be 0, 1", CONT=(sc.debug eq 0B)
    return, adsec_error.input_type
endif

if test_type(data_len, /LONG, N_EL=n_el) then begin
    message, "data_len must be a long-int", CONT=(sc.debug eq 0B)
    return, adsec_error.input_type
endif
if n_el ne 1 then begin
    message, "data_len must be a scalar", CONT=(sc.debug eq 0B)
    return, adsec_error.input_type
endif
;if data_len[0] le 0 or $
;  dsp_address[0]+data_len[0] gt get_mem_size(datasheet, set_pm) then begin
;    message, "data_len is out of range", CONT=(sc.debug eq 0B)
;    return, adsec_error.input_type
;endif

if n_elements(n_iter) eq 0 then begin
    n_iter = 1
endif else begin
    if test_type(n_iter, /INT, /LONG, N_EL=n_el) then begin
        message, "The number of iterations must be a short or long integer", CONT=(sc.debug eq 0B)
        return, adsec_error.input_type
    endif
    if n_el ne 1 then begin
        message, "ITER_NUMBER must be a scalar", CONT=(sc.debug eq 0B)
        return, adsec_error.input_type
    endif
    if n_iter[0] le 0 then begin
        message, "ITER_NUMBER cannot be less then or equal to zero", CONT=(sc.debug eq 0B)
        return, adsec_error.input_type
    endif
endelse

;
;**** end of the checks
;==============================================================

n_dsp = last_dsp-first_dsp+1
dsp_list = indgen(n_dsp)+first_dsp

word_wrong = 0L
if keyword_set(retry_on_error) then max_retry_iter=10 else max_retry_iter=0

if sc.host_comm eq "Dummy" then begin
    err = adsec_error.ok
endif else begin
    for i=1L,n_iter do begin
        if test_mode eq 0 then begin
            write_buf = ulong(randomu(seed, data_len, /DOUBLE)*'FFFFFFFF'XUL)
        endif else begin
            write_buf = ulong(randomu(seed, data_len, n_dsp, /DOUBLE)*'FFFFFFFF'XUL)
        endelse
        if is_dsp_mem(set_pm) then begin
            if test_mode eq 0 then begin
                err = write_same_dsp(dsp_list, dsp_address, write_buf, SET_PM=set_pm)
            endif else begin
                err = write_seq_dsp(dsp_list, dsp_address, write_buf, SET_PM=set_pm)
            endelse
        endif else begin
            if test_mode eq 0 then begin
                err = write_same_board(dsp_list, dsp_address, write_buf, SET_PM=set_pm, $
                                       BCU = is_bcu, SIGGEN=is_siggen)
            endif else begin
                err = write_seq_board(dsp_list, dsp_address, write_buf, SET_PM=set_pm, $
                                      BCU = is_bcu, SIGGEN=is_siggen)
            endelse
        endelse
        if err ne adsec_error.ok then return, err
        
        n_retry_iter = 0
        repeat begin
         ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
         ; lines for debugging purposes:
         ;
         ;if test_mode eq 0 then begin
         ;    err = write_same_dsp(dsp_list, dsp_address, write_buf,SET_PM=keyword_set(set_pm))
         ;    if err ne adsec_error.ok then return, err
         ;endif else begin
         ;    err = write_seq_dsp(dsp_list, dsp_address, write_buf, SET_PM=keyword_set(set_pm))
         ;    if err ne adsec_error.ok then return, err
         ;endelse
         ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
       if is_dsp_mem(set_pm) then begin
            err = read_seq_dsp(dsp_list, dsp_address, data_len, read_buf, $
                               SET_PM=set_pm, TYPE=size(write_buf,/TYPE))
        endif else begin
            err = read_seq_board(dsp_list, dsp_address, data_len, read_buf, $
                                 SET_PM=set_pm, TYPE=size(write_buf,/TYPE), $
                                 BCU = is_bcu, SIGGEN=is_siggen)
        endelse
       if err ne adsec_error.ok then return, err

       if test_mode eq 0 then begin
           idx_word_wrong = where(read_buf ne rebin(write_buf,data_len,n_dsp), word_count)
       endif else begin
           idx_word_wrong = where(read_buf ne write_buf, word_count)
       endelse
       word_wrong = word_wrong+word_count

         if word_count ne 0 then begin
          if keyword_set(verbose) then begin
              for j=0,(word_count < 10000)-1 do begin
                 dsp_num = idx_word_wrong[j]/data_len
                 word_num= idx_word_wrong[j] mod data_len
                  if test_mode eq 0 then begin
                      word_written = write_buf[word_num]
                  endif else begin
                      word_written = write_buf[idx_word_wrong[j]]
                  endelse
                   print, " ITER#",strtrim(i,2),"(",strtrim(n_retry_iter,2),") ERROR#",strtrim(j+1,2)," DSP#" $
                        , strtrim(dsp_num+first_dsp,2)," WORD#",strtrim(word_num, 2) $
                        , " Written:"+string(word_written,FORMAT='(Z8.8)') $
                        , " Read:",string(read_buf[idx_word_wrong[j]],FORMAT='(Z8.8)')
              endfor
          endif
          if word_count gt 10 then print, "ERROR: only 10 of "+strtrim(word_count,2)+" wrong word displayed for this iteration."
         endif
         n_retry_iter = n_retry_iter+1
       endrep until ((n_retry_iter ge max_retry_iter) or (word_count eq 0))

    endfor
endelse

if word_wrong then $
    return, adsec_error.comm_test_fail $
else $
    return, adsec_error.ok

end
