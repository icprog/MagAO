;$Id: load_diag_data_acc.pro,v 1.1 2008/04/08 09:46:33 labot Exp $$
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

Function load_diag_data_acc, numframes, data_reduced, data, SKIP = skip, READFC=readfc

    @adsec_common
    crate_id=5
    diag_len =  16
    If keyword_set(READFC) then begin


        ;check se la fast va leggendo due volte di seguito il puntatore

        ;read the frames counter from the SRAM ptr
        err = read_seq_board(crate_id, acc_sram_map.wr_diagnostic_record_ptr, 1l, wr_ptr, SET_PM=accel_datasheet.sram_mem_id, /ULONG, /SIGGEN)
        If err ne adsec_error.ok then return , err
        If total(abs(wr_ptr - wr_ptr[0])) ne 0 then begin
            message, 'WARNING: Mismatch on frames counter.', CONT=1
        endIf

        ;wr_ptr now is in WORD
        wr_ptr = wr_ptr[0]/4.
        

        ;HP: If there's a request of more frames, we suppose a wrap-around done.
        flag_wrap = numframes gt (wr_ptr/ diag_len)
        If flag_wrap then begin

            message, "WARNING: the number of frames requested doesn't exist from start of buffer. Automatic wrap-around assumed.", CONT =1
            nf_post_wrap = (wr_ptr/ diag_len)
            nf_before_wrap = numframes - nf_post_wrap
            
            init_addr_post_wrap = 0L
            init_addr_before_wrap = (floor(accel_datasheet.sdram_mem_size / diag_len ) - nf_before_wrap ) * (diag_len)

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
;    templ_sdram = gen_fastdiag_template(/SWITCHB)
;    data_reduced = gen_fastdiag_template(numframes, /SWITCHB)
    data_reduced = {                                                                       $
        a              : ulonarr(numframes)                        ,$
        b              : ulonarr(numframes)                        ,$
        c              : ulonarr(numframes)                        ,$
        d              : ulonarr(numframes)                        ,$
        acc_0          : fltarr(numframes)            ,$
        acc_1          : fltarr(numframes)     ,$
        acc_2          : fltarr(numframes)     ,$
        acc_3          : fltarr(numframes)     ,$
        acc_4          : fltarr(numframes)                      ,$
        acc_5          : fltarr(numframes)     ,$
        acc_6          : fltarr(numframes)                        ,$
        acc_7          : fltarr(numframes)                        ,$
        a_c            : ulonarr(numframes)                        ,$
        b_c            : ulonarr(numframes)                        ,$
        c_c            : ulonarr(numframes)                        ,$
        d_c            : ulonarr(numframes)                        $
    }

    ;ciclo su data e riempio la nuova struttura

    If flag_wrap then begin

        err = read_seq_board(crate_id, long(init_addr_post_wrap), long(nf_post_wrap) * diag_len, temp_post, $
                             SET_PM=accel_datasheet.sdram_mem_id, /ULONG, /SIGGEN)
        If err ne adsec_error.ok then return, err

        err = read_seq_board(crate_id, long(init_addr_before_wrap), long(nf_before_wrap) * diag_len, temp_before, $
                                 SET_PM=accel_datasheet.sdram_mem_id, /ULONG, /SIGGEN)
        If err ne adsec_error.ok then return, err
            
        temp = [reform(temp_before), reform(temp_post)]

    endIf Else begin

        err = read_seq_board(crate_id, long(init_addr), long(numframes) * diag_len, temp, $
                             SET_PM=accel_datasheet.sdram_mem_id, /ULONG, /SIGGEN)
        If err ne adsec_error.ok then return, err


    endElse

    data = reform(temporary(temp))

    For j=0UL, numframes-1 do begin

            current = data[j*diag_len : j*diag_len + diag_len-1]
            
            ;id = indgen(numframes)*diag_len
            data_reduced.a[j] = current[0]
            data_reduced.b[j] = current[1]
            data_reduced.c[j] = current[2]
            data_reduced.d[j] = current[3]
            data_reduced.acc_0[j] = float(current[4],0,1)
            data_reduced.acc_1[j] = float(current[5],0,1)
            data_reduced.acc_2[j] = float(current[6],0,1)
            data_reduced.acc_3[j] = float(current[7],0,1)
            data_reduced.acc_4[j] = float(current[8],0,1)
            data_reduced.acc_5[j] = float(current[9],0,1)
            data_reduced.acc_6[j] = float(current[10],0,1)
            data_reduced.acc_7[j] = float(current[11],0,1)
            data_reduced.a_c[j] = current[12]
            data_reduced.b_c[j] = current[13]
            data_reduced.c_c[j] = current[14]
            data_reduced.d_c[j] = current[15]

    endFor

    return, adsec_error.ok
        
End
