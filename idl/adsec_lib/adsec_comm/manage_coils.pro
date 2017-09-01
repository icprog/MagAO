;$Id: manage_coils.pro,v 1.10 2009/04/10 13:52:15 labot Exp $$
;+
;	MANAGE_COILS
; 
;   This function manages all actuators coils. On default it disables alla coils
;
;   err = disable_coils (board, fist_ch,last_ch, /ENABLE, /ALL)
;
; INPUTS
;	 
;    board:                     number of board to disable(if set ALL, is the number of BCU to disable)
;	 first_ch:					first channel to disable.
;	 last_ch:					last channel to disable.
;
; OUTPUTS
;   err:             long-int scalar. (see adsec_error in init_gvar.pro)
;
; KEYWORDS
;   enable:     enable instead of disable the specified coils
;   all:        refer to a whole single crate
;
; HISTORY:
;
;   Created by  Marco Xompero on 27 Jul 2005
;   <marco@arcetri.astro.it>
;   Jan 2006 MX, Added two help.
;   07 Feb 2007, MX
;       New mirror controller id now correctly managed.
;   3 Mar 2007 MX   
;       Help written.
;   19 april 2007 DZ & Fabio Tosetti(FT)
;       Substituted the call_external with the call to the C function.    
;   23 April 2007 DZ 
;       To manage all coils added  the call to  reset_devices_multi_wrap
;   19 Sep 2007 MX
;       Bug on single board calculation fixed.
;-

Function manage_coils, board, first_ch_tmp, last_ch_tmp $
                     , ENABLE=enable, ALL=all

    @adsec_common
     if ~!AO_CONST.side_master then return, adsec_error.IDL_NOT_SIDE_MASTER


    if keyword_set(all) then begin
        bcu_num=board
        
        if board eq sc.all then begin 
            bcu_num = 0
            bcu_num_last = adsec.n_bcu -1
        endif 
        
        if bcu_num lt 0 or bcu_num gt adsec.n_bcu then begin
            message, 'Wrong bcu number!', cont=(sc.debug  eq 1)
            return , adsec_error.input_type
        endif

    endif else begin
        If board lt 0 or board gt adsec.n_board_per_bcu*adsec.n_bcu then begin
            message, 'Wrong board number!', cont=(sc.debug  eq 1)
            return , adsec_error.input_type
        endif
    endelse

    If n_elements(first_ch_tmp) eq 0 and n_elements(last_ch_tmp) eq 0 then begin

        first_ch =0
        last_ch = adsec.n_dsp_per_board *adsec.n_act_per_dsp -1
        new_board_0 = 0
        new_board_1 = adsec.n_board_per_bcu * adsec.n_dsp_per_board - adsec.n_dsp_per_board

    endIf else begin

        first_ch = first_ch_tmp
        last_ch = last_ch_tmp
        bcu_num = board / (adsec.n_board_per_BCU)
        new_board_0 = (board mod (adsec.n_board_per_BCU)) * adsec.n_dsp_per_board
        new_board_1 = new_board_0

    endelse

    mask = 0uL;

    ;set the template to enable/disable the coils
    If keyword_set(enable) then template = 2UL else template = 1UL

    For i = first_ch, last_ch do mask = mask + ishft(template, 2*i)


    ;==============================================================
    ;debug printing variables
    dspAddress = 0L
    datalength = 2L
    data = [0ul, mask]
    if sc.ll_debug then begin
        print, 'Debugging data'
        ;help, /traceback
        help, bcu_num
        help, board
        print, "MirrCtrl_id ", sc.mirrctrl_id[bcu_num]
        help, new_board_0
        help, new_board_1
        help, datalength
        help, dspAddress
        print, data
        print, 'sc.debug ', sc.debug
        print, 'Mask ', mask

    endif

    ;Dummy case
    if sc.host_comm eq "Dummy" then begin
        err = adsec_error.ok
    endif else begin
        if board eq sc.all then begin
            err = reset_devices_multi_wrap(sc.mirrctrl_id[bcu_num], sc.mirrctrl_id[bcu_num_last],$
                                             new_board_0, new_board_1, dspAddress, datalength, data)

        endif else begin
        ;Standard case
            err = reset_devices_wrap(sc.mirrctrl_id[bcu_num], new_board_0, new_board_1, dspAddress, $
                                    datalength, data)
        endelse
    endelse
    
	if err ne adsec_error.ok then begin
	    message, "READING ERROR BCU", CONT=(sc.debug eq 0B)
	    return, err
	endif

return, err

end

