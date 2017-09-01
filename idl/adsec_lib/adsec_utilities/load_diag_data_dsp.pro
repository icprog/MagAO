;$Id: load_diag_data_dsp.pro,v 1.4 2008/08/01 08:01:32 labot Exp $$
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

Function load_diag_data_dsp, numframes, data_reduced, READFC=readfc, SKIP = skip, NOFASTCHECK=nofastcheck, XADSEC=xadsec

    @adsec_common
    If keyword_set(READFC) then begin


        ;check se la fast va leggendo due volte di seguito il puntatore

        ;read the frames counter from the SRAM ptr
        err = read_seq_board(sc.all, sram_map.wr_diagnostic_record_ptr, 1l, wr_ptr, SET_PM=dsp_datasheet.sram_mem_id, /ULONG)
        If err ne adsec_error.ok then return , err
        If total(abs(wr_ptr - wr_ptr[0])) ne 0 then begin
            message, 'WARNING: Mismatch on frames counter.', CONT=1
        endIf

        If ~keyword_set(nofastcheck) then begin

            ;read the frames counter from the SRAM for cheack If the FAST DIAGNOSTIC is operating
            err = read_seq_board(sc.all, sram_map.wr_diagnostic_record_ptr, 1l, wr_ptr2, SET_PM=dsp_datasheet.sram_mem_id, /ULONG)
            If err ne adsec_error.ok then return , err

            If (wr_ptr2[0] - wr_ptr[0]) eq 0 then begin
                message, 'The FAST DIAGNOSTIC is not running. Gains no raised up.', CONT= (sc.debug eq 0)
                return, adsec_error.IDL_OVS_NOT_RUNNING
            endIf

        endIf

        ;wr_ptr now is in WORD
        wr_ptr = wr_ptr[0]/4.

        ;HP: If there's a request of more frames, we suppose a wrap-around done.
        flag_wrap = numframes gt (wr_ptr/ (rtr.diag_len * adsec.n_dsp_per_board))
        If flag_wrap then begin

            message, "WARNING: the number of frames requested doesn't exist from start of buffer. Automatic wrap-around assumed.", CONT =1
            nf_post_wrap = (wr_ptr/ (rtr.diag_len * adsec.n_dsp_per_board))
            nf_before_wrap = numframes - nf_post_wrap
            
            init_addr_post_wrap = 0L
            init_addr_before_wrap = (floor(dsp_datasheet.sdram_mem_size / (rtr.diag_len * adsec.n_dsp_per_board) ) - nf_before_wrap ) * (rtr.diag_len * adsec.n_dsp_per_board)

        endIf Else begin

            ;set the initial address
            init_addr = ((wr_ptr / (rtr.diag_len * adsec.n_dsp_per_board) ) - numframes) * (rtr.diag_len * adsec.n_dsp_per_board)

        endElse

    endIf Else begin
        If n_elements(skip) eq 0 then skip =0
        init_addr = long(skip) * rtr.diag_len * adsec.n_dsp_per_board
        flag_wrap=0
    endElse

    ;data diventa un lunghissimo vettore con tutti i dati
    templ_sdram = gen_fastdiag_template()
    data_reduced = gen_fastdiag_template(numframes)

    ;ciclo su data e riempio la nuova struttura
    For i=0, adsec.n_board_per_bcu*adsec.n_bcu -1 do begin
        
        log_print, "BOARD #"+string(i)


        If flag_wrap then begin

            err = read_seq_board(i, long(init_addr_post_wrap), long(nf_post_wrap) * rtr.diag_len *adsec.n_dsp_per_board, temp_post, $
                                 SET_PM=dsp_datasheet.sdram_mem_id, /ULONG)
            If err ne adsec_error.ok then return, err

            err = read_seq_board(i, long(init_addr_before_wrap), long(nf_before_wrap) * rtr.diag_len *adsec.n_dsp_per_board, temp_before, $
                                 SET_PM=dsp_datasheet.sdram_mem_id, /ULONG)
            If err ne adsec_error.ok then return, err
            
            temp = [reform(temp_before), reform(temp_post)]

        endIf Else begin

            err = read_seq_board(i, long(init_addr), long(numframes) * rtr.diag_len *adsec.n_dsp_per_board, temp, $
                             SET_PM=dsp_datasheet.sdram_mem_id, /ULONG)
            If err ne adsec_error.ok then return, err


        endElse

        data = reform(temporary(temp))
        id = indgen(adsec.n_act_per_dsp*adsec.n_dsp_per_board) + i*adsec.n_act_per_dsp*adsec.n_dsp_per_board

        For j=0UL, numframes-1 do begin
            
            if keyword_set(XADSEC) then begin 
                   err = update_status()
                   if err ne adsec_error.ok then return, err      
                   update_panels,/no_read, /OPT
            endif

            For k=0UL, adsec.n_dsp_per_board-1 do begin

                current = data[j*rtr.diag_len*adsec.n_dsp_per_board + rtr.diag_len*k : $
                               j*rtr.diag_len*adsec.n_dsp_per_board + rtr.diag_len*(k+1)-1]
                id_act = id[k*adsec.n_act_per_dsp : (k+1)*adsec.n_act_per_dsp -1]

                fill_fastdiag, current, data_reduced, j, id_act

            endFor

        endFor

    endFor
    return, err
        
End
