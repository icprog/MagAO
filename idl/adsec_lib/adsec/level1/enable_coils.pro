;$Id: enable_coils.pro,v 1.4 2007/09/19 09:23:36 labot Exp $$
;+
;   NAME:
;    ENABLE_COILS
;   
;   PURPOSE:
;    This routine enables the coils in the Adaptive Secondary. It returns NO_ERROR if all coils are enabled.
;   
;   USAGE:
;    err = enable_coils()
;    
;   INPUT:
;    None.
;
;   KEYWORDS:
;    None.
;
;   OUTPUT:
;    err: error code.
;
; HISTORY
;   14 Oct 2005
;       Written by Marco Xompero (MX)
;       Osservatorio Astrofisico di Arcetri, ITALY
;       marco@arcetri.astro.it
;   3 Mar 2007
;       Help, written.
;   23 April 2007
;       Only one call to manage_coils to enable all coils.    
;   19 Sep 2007
;       Actuator list now managed.
;-

Function enable_coils, act_list

     @adsec_common
    ;mancano i checks!!!
    ;da implementare varie combinazioni di enable/disable
    ;setting up the all disabling
;    if act_list eq sc.all then act_list = indgen(adsec.n_actuators)
;    ranges = list2ranges(act_list, count, BREAK = adsec.n_dsp_per_board * adsec.n_act_per_dsp) 
;    board_list = fix(ranges[0,*]) /  fix(adsec.n_act_per_dsp * adsec.n_dsp_per_board)
;    ranges = ranges mod (adsec.n_act_per_dsp * adsec.n_dsp_per_board)

     if n_elements(act_list) gt 0 then begin

         for i=0, n_elements(act_list)-1 do begin
             board = fix(act_list[i])/adsec.n_act_per_dsp/adsec.n_dsp_per_board
             ch = act_list[i] mod (adsec.n_act_per_dsp*adsec.n_dsp_per_board)
             ;print, board
             ;print, ch
             err = manage_coils(board, ch, ch, /enable)
             if err ne adsec_error.ok then return, err
         endfor

     endif else begin

        err = manage_coils(sc.all, /enable, /all)
        if err ne adsec_error.ok then message, 'Coils not enabled!!!!', cont = (sc.debug eq 1)
        
    endelse


    return, err

End
