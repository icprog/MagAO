;$Id: get_sram_data.pro,v 1.2 2007/04/16 13:43:23 labot Exp $$
;+
; NAME:
;   GET_SRAM_DATA
;
; PURPOSE:
;   Get DAC calibration matrix fom sram memory.
;
; USAGE:
;   err = get_sram_data(board_list, mat_dac)
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
;   zanotti@arcetri.astro.it
;   Creation and test
;-
function get_sram_data, board_list, mat_dac

    @adsec_common
        addr_sram = '18174'xl ; da inserire nei file di configurazione 
        n_gain = 6
        n_ch_per_board = adsec.n_act_per_dsp*adsec.n_dsp_per_board
        if board_list eq sc.all then n_boards = adsec.n_board_per_bcu*adsec.n_crates $
                else n_boards = n_elements(board_list) 
        nword = n_ch_per_board*n_gain
    if (sc.host_comm eq "Dummy") or (sc.host_comm eq 'OnlyCommunication') then begin
;    if sc.host_comm eq "Dummy" or adsec.n_actuators le 48 then begin ;to debug per P45 
        mat_dac = fltarr(n_ch_per_board, n_gain, n_boards)+1.0 
        mat_dac[*,2,*] = 0.0
        mat_dac[*,4,*] = 0.0

    endif else begin

        err = read_seq_board(board_list,addr_sram,nword, buf, SET_PM = 3)
        if err ne adsec_error.ok then return, err
        mat_dac = buf
        mat_dac = reform(temporary(mat_dac), n_ch_per_board, n_gain, n_boards)     
    endelse     
    
    return,adsec_error.ok

end
