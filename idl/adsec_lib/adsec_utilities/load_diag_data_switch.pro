;$Id: load_diag_data_switch.pro,v 1.3 2008/02/05 10:52:47 labot Exp $$
;+
; NAME:
;   LOAD_DIAG_DATA
;
; PURPOSE:
;   Read the fast diagnostic data directly from the SDRAM of the AdSec.
;
; CATEGORY:
;   General utility.
;
; CALLING SEQUENCE:
;   err = LOAD_DIAG_DATA(numframes, final, SKIP=skip)
;
; INPUTS:
;   numframes :  number of frames to read.
;
; KEYWORD PARAMETERS:
;   SKIP=skip : allow to skip "skip" frames from the start of the SDRAM memory.
;   
; OUTPUTS:
;   err    : error code.
;   final  : output buffer with fast diagnostic data.
;
; RESTRICTIONS:
;   None.
;
; MODIfICATION HISTORY:
;   14 Apr 2005
;    Created by Marco Xompero (MX)
;    marco@arcetri.astro.it
;   
;-

Function load_diag_data_switch, numframes, data_reduced, READFC=readfc, SKIP = skip, NOFASTCHECK=nofastcheck, XADSEC=xadsec

    @adsec_common
    If keyword_set(READFC) then begin


        ;check se la fast va leggendo due volte di seguito il puntatore

        ;read the frames counter from the SRAM ptr
        err = read_seq_board(sc.all, switch_sram_map.wr_diagnostic_record_ptr, 1l, wr_ptr, SET_PM=switch_datasheet.sram_mem_id, /ULONG, /SWITCHB)
        If err ne adsec_error.ok then return , err
        If total(abs(wr_ptr - wr_ptr[0])) ne 0 then begin
            message, 'WARNING: Mismatch on frames counter.', CONT=1
        endIf

        ;wr_ptr now is in WORD
        wr_ptr = wr_ptr[0]/4.
        diag_len =  rtr.n_slope + 20

        ;HP: If there's a request of more frames, we suppose a wrap-around done.
        flag_wrap = numframes gt (wr_ptr/ diag_len)
        If flag_wrap then begin

            message, "WARNING: the number of frames requested doesn't exist from start of buffer. Automatic wrap-around assumed.", CONT =1
            nf_post_wrap = (wr_ptr/ diag_len)
            nf_before_wrap = numframes - nf_post_wrap
            
            init_addr_post_wrap = 0L
            init_addr_before_wrap = (floor(switch_datasheet.sdram_mem_size / diag_len ) - nf_before_wrap ) * (diag_len)

        endIf Else begin

            ;set the initial address
            init_addr = ((wr_ptr / diag_len ) - numframes) * (diag_len)

        endElse

    endIf Else begin
        If n_elements(skip) eq 0 then skip =0
        init_addr = long(skip) * diag_len
        flag_wrap=0
    endElse

    ;data diventa un lunghissimo vettore con tutti i dati
    templ_sdram = gen_fastdiag_template(/SWITCHB)
    data_reduced = gen_fastdiag_template(numframes, /SWITCHB)

    ;ciclo su data e riempio la nuova struttura

    If flag_wrap then begin

        err = read_seq_board(0, long(init_addr_post_wrap), long(nf_post_wrap) * diag_len, temp_post, $
                             SET_PM=switch_datasheet.sdram_mem_id, /ULONG, /SWITCHB)
        If err ne adsec_error.ok then return, err

        err = read_seq_board(0, long(init_addr_before_wrap), long(nf_before_wrap) * diag_len, temp_before, $
                                 SET_PM=switch_datasheet.sdram_mem_id, /ULONG, /SWITCHB)
        If err ne adsec_error.ok then return, err
            
        temp = [reform(temp_before), reform(temp_post)]

    endIf Else begin

        err = read_seq_board(0, long(init_addr), long(numframes) * diag_len, temp, $
                             SET_PM=switch_datasheet.sdram_mem_id, /ULONG, /SWITCHB)
        If err ne adsec_error.ok then return, err


    endElse

    data = reform(temporary(temp))

    For j=0UL, numframes-1 do begin

            if keyword_set(XADSEC) then begin 
                err = update_status()
                if err ne adsec_error.ok then return, err
                update_panels,/no_read, /OPT
            endif

            current = data[j*diag_len : j*diag_len + diag_len-1]

            fill_fastdiag, current, data_reduced,  j,  /SWITCHB

    endFor

    return, adsec_error.ok
        
End
