;$Id: load_diag_data_bcu.pro,v 1.1 2007/02/13 11:01:37 marco Exp $$
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

Function load_diag_data_bcu, numframes, data_reduced, READFC=readfc, SKIP = skip, NOFASTCHECK=nofastcheck

    @adsec_common
    If keyword_set(READFC) then begin


        ;check se la fast va leggendo due volte di seguito il puntatore

        ;read the frames counter from the SRAM ptr
        err = read_seq_board(sc.all, bcu_sram_map.wr_diagnostic_record_ptr, 1l, wr_ptr, SET_PM=bcu_datasheet.sram_mem_id, /ULONG, /BCU)
        If err ne adsec_error.ok then return , err
        If total(abs(wr_ptr - wr_ptr[0])) ne 0 then begin
            message, 'WARNING: Mismatch on frames counter.', CONT=1
        endIf

        ;wr_ptr now is in WORD
        wr_ptr = wr_ptr[0]/4.
        diag_len = ((rtr.diag_len-12) * adsec.n_dsp_per_board + 8)

        ;HP: If there's a request of more frames, we suppose a wrap-around done.
        flag_wrap = numframes gt (wr_ptr/ diag_len)
        If flag_wrap then begin

            message, "WARNING: the number of frames requested doesn't exist from start of buffer. Automatic wrap-around assumed.", CONT =1
            nf_post_wrap = (wr_ptr/ diag_len)
            nf_before_wrap = numframes - nf_post_wrap
            
            init_addr_post_wrap = 0L
            init_addr_before_wrap = (floor(bcu_datasheet.sdram_mem_size / diag_len ) - nf_before_wrap ) * (wr_ptr/ diag_len)

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
    templ_sdram = gen_fastdiag_template(/BCU)
    data_reduced = gen_fastdiag_template(numframes, /BCU)

    ;ciclo su data e riempio la nuova struttura
    For i=0, adsec.n_bcu -1 do begin


        If flag_wrap then begin

            err = read_seq_board(i, long(init_addr_post_wrap), long(nf_post_wrap) * rtr.diag_len *adsec.n_dsp_per_board, temp_post, $
                                 SET_PM=bcu_datasheet.sdram_mem_id, /ULONG, /BCU)
            If err ne adsec_error.ok then return, err

            err = read_seq_board(i, long(init_addr_before_wrap), long(nf_before_wrap) * rtr.diag_len *adsec.n_dsp_per_board, temp_before, $
                                 SET_PM=bcu_datasheet.sdram_mem_id, /ULONG, /BCU)
            If err ne adsec_error.ok then return, err
            
            temp = [reform(temp_before), reform(temp_post)]

        endIf Else begin

            err = read_seq_board(i, long(init_addr), long(numframes) * rtr.diag_len *adsec.n_dsp_per_board, temp, $
                             SET_PM=bcu_datasheet.sdram_mem_id, /ULONG, /BCU)
            If err ne adsec_error.ok then return, err


        endElse

        data = reform(temporary(temp))

        For j=0UL, numframes-1 do begin

                current = data[j*diag_len*adsec.n_bcu + diag_len*i : $
                               j*diag_len*adsec.n_bcu + diag_len*(i+1)-1]

                fill_fastdiag, current, data_reduced, BCU=i

        endFor

    endFor
    return, err
        
End
