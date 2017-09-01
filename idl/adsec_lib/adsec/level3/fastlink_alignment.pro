;$Id: fastlink_alignment.pro,v 1.11 2009/11/24 16:57:08 marco Exp $
;+
; NAME:
;   FASTLINK_ALIGNEMENT
;
; PURPOSE:
;   Align FastLink connection between SwitchBCU and all CrateBcu.
; 
; USAGE:
;   err = fastlink_alignement(/VERBOSE)
;
; KEYWORDS:
;   VERBOSE:    more info printed.
;   TEST_TIME:  test time in seconds
;
; HISTORY:
;   Created by Marco Xompero (MX) on Mar 2007
;   marco@arcetri.astro.it
;
; NB single crate reading implemented BUT NOT WORKING!!!!
;-

Function FastLink_Alignment, VERBOSE=verbose, TEST_TIME=test_time, STOPONERR=stoponerr, CRATE_ID=crate_id, CEND=cid_end

    @adsec_common
    if (sc.name_comm eq "Dummy") or (sc.name_comm eq "OnlyComm") then return, adsec_error.ok
    log_print, "Fastlink alignment procedure initialized."
    ;NON DEVONO ESSERCI PROGRAMMI DSP SULLA CRATEBCU!!!!!
    
    if keyword_set(stoponerr) then $
         FL_WRRD_Test_Program  = adsec_path.prog+'switchBcu/dsp/'+prog_vers.switchbcu.dsp+'/BCU_dsp_FastLinkTest_1.02_stop_on_err.ldr'   $
    else FL_WRRD_Test_Program  = adsec_path.prog+'switchBcu/dsp/'+prog_vers.switchbcu.dsp+'/BCU_dsp_FastLinkTest.ldr';

    BANK_1             = '00080000'xl
    BANK_2             = '00100000'xl

    p_flinterface        = '0x1000'xl;
    p_write_buffer_len   = '0x80000'xl;
    p_first_dsp          = '0x80001'xl;
    p_first_crt          = '0x80002'xl;
    p_last_dsp           = '0x80003'xl;
    p_last_crt           = '0x80004'xl;
    p_tot_num_dsp        = '0x80005'xl;
    p_remote_addr        = '0x80006'xl;
    p_counter            = '0x80080'xl;
    p_counter1           = '0x80087'xl;
    p_memcmp_good_cnt    = '0x80082'xl;
    p_timeout_wr_err_cnt = '0x80084'xl;
    p_timeout_rd_err_cnt = '0x80086'xl;
    p_crc_wr_err_cnt     = '0x80088'xl;
    p_crc_rd_err_cnt     = '0x8008a'xl;
    p_memcmp_err_cnt     = '0x8008c'xl;
    p_enabled            = '0x8008e'xl;
    p_irq_signal	 = '0x8008f'xl;

    p_write_buffer       = '0x080800'xl;
    p_read_buffer        = '0x100000'xl;

    ;BCU_DSP            = 255;    % indirizzo DSP scheda BCU
    FIRST_DSP          = 0;      % primo DSP del ricostruttore
    LAST_DSP           = long(adsec.n_board_per_bcu * adsec.n_dsp_per_board-1);     % ultimo DSP del ricostruttore
    FIRST_CRATE        = 0;      % primo crate del ricostruttore
    LAST_CRATE         = adsec.n_crates;      % ultimo crate del ricostruttore
    WRITE_BUFFER_SIZE  = 'B000'xl;
    READ_BUFFER_SIZE   = '10000'xl;

    ;% path dei files con il codice DSP
    ;DspMainProgram        = '.\firmware\DspMainProgram.ldr';

    if n_elements(crate_id) gt 0 then begin
        crate_id = long(crate_id)
        if crate_id gt adsec.n_crates or crate_id lt 0 then return, adsec_error.IDL_INPUT_TYPE
        
    endif

    ;% WR comand
    ;write_buffer_len = 20L;
    ;remote_addr = '8000'xl;
    write_buffer_len = 20L; multiple of 4
    remote_addr = '8000'xl;
    if sc.model eq 'P45' then begin
        first_crt = 1L
        last_crt = 1L
    endif else begin

        if n_elements(crate_id) gt 0 then begin
            first_crt=crate_id
            last_crt=crate_id
            if n_elements(cid_end) gt 0 then last_crt = long(cid_end) else last_crt = cid
            ncrt = last_crt-first_crt+1
        endif else begin
            first_crt = 0L          ;
            last_crt = long(adsec.n_crates-1) ;
            ncrt = adsec.n_crates
        endelse
    endelse
    print, first_crt
    print, last_crt
    first_dsp=0L
    last_dsp = long(adsec.n_dsp_per_board*adsec.n_board_per_bcu -1)
    tot_num_dsp = long(adsec.n_dsp_per_board*adsec.n_board_per_bcu * ncrt)
    if n_elements(TEST_TIME) eq 0 then test_time=15; % in secondi
    test_length=test_time*2L; % in tick di 500ms
    init_test_length = test_length

    ;% verifica dei dati di input
    if ((write_buffer_len + (remote_addr and 65535) ) gt 65536 ) or $
        (write_buffer_len gt WRITE_BUFFER_SIZE) or                  $
        (tot_num_dsp*write_buffer_len) gt READ_BUFFER_SIZE          $
        then return, adsec_error.input_type;
    

    ;DOWNLOADING TEST PROGRAM
    err = download_program(0,0,FL_WRRD_Test_Program, VERB=verbose, /SWITCH)
    if err ne adsec_error.ok then return, err

    ;SET PARAMETERS INTO SWITCH BCU MEMORY
    err = write_same_dsp(0,p_write_buffer_len,write_buffer_len, /SWITCH);
    if err ne adsec_error.ok then return, err
    err = write_same_dsp(0,p_first_dsp,first_dsp, /SWITCH);
    if err ne adsec_error.ok then return, err
    err = write_same_dsp(0,p_first_crt,first_crt, /SWITCH);
    if err ne adsec_error.ok then return, err
    err = write_same_dsp(0,p_last_dsp,last_dsp, /SWITCH);
    if err ne adsec_error.ok then return, err
    err = write_same_dsp(0,p_last_crt,last_crt, /SWITCH);
    if err ne adsec_error.ok then return, err
    err = write_same_dsp(0,p_tot_num_dsp,tot_num_dsp, /SWITCH);
    if err ne adsec_error.ok then return, err
    err = write_same_dsp(0,p_remote_addr,remote_addr, /SWITCH);
    if err ne adsec_error.ok then return, err
    err = write_same_dsp(0,p_enabled,1L, /SWITCH);
    if err ne adsec_error.ok then return, err


    ;% verifica dei contatori di status
    time0 = systime(/sec)
    err_comp = [0,0,0,0,0]
    while (test_length gt 0) do begin
        err=read_seq_dsp(0,p_counter, 14L,res0, /SWITCH, /UL);
        if err ne adsec_error.ok then begin
            err1 = write_same_dsp(0,p_enabled,0L, /SWITCH);
            if err1 ne adsec_error.ok then return, err1
            return, err
        endif

        res = long64(res0,0,7)
        ber=1./((tot_num_dsp+1)*write_buffer_len*32*double(res[1])) ;
        ;   if (res[3] ne 0 || res[4] ne 0 || res[5] ne 0 || res[6] ne 0 || res[7] ne 0)
        ;% sono: Numero comandi fatti, Numero
        ;interazioni avvenute con successo (write+read), Numero Timeout WR,
        ;Numero Timeout RD, Num CRC write, Num CRC Read, Num Memory compare
        ;errors (sono tutti a 64 bit)
        if keyword_set(verbose) then begin
;            print, format='(%"TOT:%10d GOOD:%10d BER:%10.3e")' , res[0], res[1], ber
  ;          print, format='(%"TIM_WR:%10d - TIM_RD:%10d - CRC_WR:%10d - CRC_RD:%10d - MEM_CMP:%10d")',res[2],res[3],res[4],res[5],res[6] ;
            fo = "(I8)"
            log_print, "TIM_WR:"+string(res[2], FORMAT=fo)+"; TIM_RD:"+string(res[3], format=fo)+   $
                       "; CRC_WR:"+string(res[4], format=fo)+"; CRC_RD:"+string(res[5], format=fo)+ $
                       "; MEM_CMP:"+string(res[6], format=fo)
   ;         log_print, strjoin(string(res[2:6] ))
        endif
        if test_length eq round(init_test_length/2.) then err_comp = res[2:6]
        wait,0.5             ;
        test_length=test_length-1 ;
        time1 = systime(/SEC)
        ;if total(res[2:6]) ne 0 or res[6] gt 0 then break
   
    endwhile

    wait, 0.2
    uu = 0
    dbg = sc.debug
    sc.debug=0
    while uu lt 5 do begin
        if uu gt 0 then log_print, "WRITE FAILS "+strtrim(uu,2)+ " TIMES..."
        err = write_seq_dsp(0,p_enabled,0L, /SWITCH) ;
        if err eq adsec_error.ok then break else uu+=1
        wait, 0.1
    endwhile
    sc.debug=dbg
    
    if err ne adsec_error.ok then return, err
    
    if total(res[2:6]-err_comp) ne 0 or res[6] gt 0 then begin
    ;if total(res[2:6]) ne 0 or res[6] gt 0 then begin
;        if keyword_set(verbose) then begin
;            print, format='(%"TIM_WR:%10d - TIM_RD:%10d - CRC_WR:%10d - CRC_RD:%10d - MEM_CMP:%10d")',err_comp[0],err_comp[1],err_comp[2], err_comp[3], err_comp[4] 
;            print, format='(%"TIM_WR:%10d - TIM_RD:%10d - CRC_WR:%10d - CRC_RD:%10d - MEM_CMP:%10d")',res[1],res[3],res[4], res[5], res[6] 
;        endif
        log_message, "Error on Switch-Crate FastLink Alignment.", cont = (sc.debug eq 0)
        return, adsec_error.IDL_FAST_LINK_ERROR

    endif else begin

        return, adsec_error.ok

    endelse


End



;aa = read_seq_dsp(sc.all, '8000'xl, 20L, bb, /UL)
;cc = bb - rebin(bb[*,0], 20, 168, /sam)
;id = where(cc ne 0)
;print, id / 20 /2
;print, id / 20 /2 /14, id / 20 /2 mod 14
