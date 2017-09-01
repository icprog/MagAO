;$Id: get_flash_data.pro,v 1.1 2007/03/28 15:32:56 labot Exp $$
;+
; NAME:
;   GET_FLASH_DATA
;
; PURPOSE:
;   Get DAC calibration matrix fom flash memory.
;
; USAGE:
;   err = get_dac_matrix(dsp_list, mat_dac)
;
; INPUT:
;   board_list:       board_list to receive
;   mat_dac:        matrix cointaining the calibration for the DAC
;
; OUTPUT:
;   Error code
;
; KEYWORD:
;   27 Mar 2007, Daniela Zanotti (DZ)
;   marco@arcetri.astro.it
;   Creation and test
;-
function get_flash_data, board_list, mat_dac

    @adsec_common
        addr_flash = '18174'xl ; da inserire nei file di configurazione 
        n_gain = 6
        n_ch_per_board = adsec.n_act_per_dsp*adsec.n_dsp_per_board
        if board_list eq sc.all then n_boards = adsec.n_board_per_bcu*adsec.n_crates $
                else n_boards = n_elements(board_list) 
        nword = n_ch_per_board*n_gain
;    if sc.host_comm eq "Dummy" then begin
    if sc.host_comm eq "Dummy" or adsec.n_actuators le 48 then begin ;to debug per P45 
        mat_dac = fltarr(n_ch_per_board, n_gain, n_boards)+1.0 
        mat_dac[*,2,*] = 0.0
        mat_dac[*,4,*] = 0.0

    endif else begin

        err = read_seq_board(board_list,addr_flash,nword, buf, SET_PM =4 )
        if err ne adsec_error.ok then return, err
        mat_dac = buf
        mat_dac = reform(temporary(mat_dac), n_ch_per_board, n_gain, n_boards)     
    endelse     
    
    return,adsec_error.ok

end
