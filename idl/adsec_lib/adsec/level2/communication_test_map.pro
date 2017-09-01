; $Id: communication_test_map.pro,v 1.1 2008/02/05 10:17:53 labot Exp $
; This function performs the communication test.
;
;     err = communication_test_map(dsp_address, data_len, $
;                             WORD_WRONG=word_wrong, $
;                             SET_PM=the_set_pm, VERBOSE=verbose, $
;                             BCU = is_bcu, SIGGEN=is_siggen, SWITCH_BCU = is_switch, $
;                             MAP=map, FILENAME=filename
; INPUTS
;
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

function communication_test_map, dsp_address, data_len, $
                             WORD_WRONG=word_wrong, $
                             SET_PM=the_set_pm, VERBOSE=verbose, $
                             BCU = is_bcu, SIGGEN=is_siggen, SWITCH_BCU = is_switch, $
                             MAP=map, FILENAME=filename

    @adsec_common

    if n_elements(the_set_pm) eq 0 then set_pm=0 else set_pm=the_set_pm
    old_debug = sc.debug
    sc.debug=0
    word_wrong = 0L

    if keyword_set(is_switch) then begin
        n_dsp = 1
        dsp_list = 0
        map = 0
    endif else begin
        if keyword_set(is_bcu) or keyword_set(is_siggen) then begin
            n_dsp = adsec.n_bcu
            dsp_list = indgen(n_dsp)
            map = intarr(adsec.n_bcu,2)
            dsp_board = 0
        endif else begin
            if is_dsp_mem(set_pm) then begin
                n_dsp = adsec.n_board_per_bcu * adsec.n_dsp_per_board * adsec.n_bcu
                dsp_list = indgen(n_dsp)
                map = intarr(n_dsp,2) 
            endif else begin
                n_dsp = adsec.n_board_per_bcu * adsec.n_bcu
                dsp_list = indgen(n_dsp)
                map = intarr(n_dsp/2.,2)
            endelse
            dsp_board = 1
        endelse
    endelse

    ;help, dsp_list
    write_same_buf = ulong(randomu(seed, data_len, /DOUBLE)*'FFFFFFFF'XUL)
    write_buf = ulong(randomu(seed, data_len, /DOUBLE)*'FFFFFFFF'XUL)
    
    for i=0, n_dsp-1 do begin
;print, "DSP:",i
        if is_dsp_mem(set_pm) then begin
            err = write_seq_dsp(dsp_list[i], dsp_address, write_buf, SET_PM=set_pm, BCU=is_bcu, SIGGEN=is_siggen, SWI=is_switch)
        endif else begin
            err = write_seq_board(dsp_list[i], dsp_address, write_buf, SET_PM=set_pm, $
                                  BCU = is_bcu, SIGGEN=is_siggen, SWI=is_switch)
        endelse
if err ne adsec_error.ok then print, "DSP:",i, err, "(1)"
        if err ne adsec_error.ok then begin
            if dsp_board then map[i/2,0]=err else map[i,0] = err
        endif

        if is_dsp_mem(set_pm) then begin
            err = read_seq_dsp(dsp_list[i], dsp_address, data_len, read_buf, BCU=is_bcu, SIGGEN=is_siggen, SWI=is_switch, $
                           SET_PM=set_pm, TYPE=size(write_buf,/TYPE))
        endif else begin
            err = read_seq_board(dsp_list[i], dsp_address, data_len, read_buf, $
                                     SET_PM=set_pm, TYPE=size(write_buf,/TYPE), $
                                     BCU = is_bcu, SIGGEN=is_siggen, SWI=is_switch)
        endelse
if err ne adsec_error.ok then print,"DSP:",i, err, "(2)"
        if err ne adsec_error.ok then begin
            map[i,1] = err
            if dsp_board then map[i/2,0]=err else map[i,0] = err
        endif else begin
            idx_word_wrong = where(read_buf ne write_buf, word_count)
            word_wrong = word_wrong+word_count
if word_count gt 0 then print, "DSP:",i," write-read failed: dsp ",i, "(",word_count,")"
        endelse

    endfor


    if total(abs(map)) eq 0 then begin
        if is_dsp_mem(set_pm) then begin
            err = write_same_dsp(dsp_list, dsp_address, write_buf, SET_PM=set_pm, $
                                   BCU = is_bcu, SIGGEN=is_siggen, SWI=is_switch, /CHECK)
        endif else begin
            err = write_same_board(dsp_list, dsp_address, write_buf, SET_PM=set_pm, $
                                   BCU = is_bcu, SIGGEN=is_siggen, SWI=is_switch, /CHECK)
        endelse
    endif else err=0
if err ne adsec_error.ok then print, err, "(3)"
    sc.debug=old_debug

    if (word_wrong gt 0) or total(abs(map)) gt 0 or (err ne adsec_error.ok) then $
        return, adsec_error.comm_test_fail $
    else $
        return, adsec_error.ok

end
