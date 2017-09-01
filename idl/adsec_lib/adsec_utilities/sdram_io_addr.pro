; $Id: sdram_io_addr.pro,v 1.2 2007/02/07 18:06:08 marco Exp $
;+
; SDRAM_IO_ADDR
;
; addr = sdram_IO_addr(dsp, buf_num_list)
;
; sdram_IO_addr function returns the starting SDRAM address related
; to buf_num buffer and associated to specified dsp. In case of error
; addr is 'FFFFFFFF'XUL. Only buf_num is checked.
;
; dsp:     scalar integer. Valid DSP number
; buf_num: scalar integer.Valid buffer number
;
; ERROR_FLAG: optional keyword. Input: named variabe. Output: 1B if
;             an error occurred, 0B otherwise.
;
; HISTORY
;
;   05 Aug 2004, written by A. Riccardi
;   07 Feb 2007 MX
;       LUT of variable deleted and name refurbished.
;-
function sdram_IO_addr, dsp, buf_num

    @adsec_common

    if check_dsp(dsp,/NOALL) then begin
        message, "Unexpected dsp value.", CONT= (sc.debug eq 0B)
        return, 'FFFFFFFF'XUL
    endif
    if n_elements(dsp) ne 1 then begin
        message, "dsp must be a scalar.", CONT= (sc.debug eq 0B)
        return, 'FFFFFFFF'XUL
    endif


    if test_type(buf_num, /NOFLOAT, N_EL=n_buf_num) then begin
        message, "buf_num must be integer.", CONT= (sc.debug eq 0B)
        return, 'FFFFFFFF'XUL
    endif
    if n_buf_num ne 1 then begin
        message, "buf_num must be a scalar.", CONT= (sc.debug eq 0B)
        return, 'FFFFFFFF'XUL
    endif
    if (total(buf_num lt 0) ne 0) or (total(buf_num ge dsp_const.n_dataIO/adsec.n_dsp_per_board) ne 0) then begin
        message, "buf_num contents buffer numbers out of range. 0<=buf_num<=" $
                 +strtrim(dsp_const.n_dataIO/adsec.n_dsp_per_board-1,2), CONT=(sc.debug eq 0B)
        return, 'FFFFFFFF'XUL
    endif
    the_buf_num = fix(buf_num[0])+(dsp[0] mod adsec.n_dsp_per_board)*(dsp_const.n_dataIO/adsec.n_dsp_per_board)

    return, sdram_map.dataIO_0 + the_buf_num*dsp_const.max_data_IO_len
end
