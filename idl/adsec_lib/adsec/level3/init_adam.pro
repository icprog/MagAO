;$Id: init_adam.pro,v 1.10 2009/11/24 16:57:08 marco Exp $$
;+
;   NAME:
;    INIT_ADAM
;
;   PURPOSE:
;    ADAM ethernet digital IO controller initialization
;
;   USAGE:
;    err = INIT_ADAM(status_in, status_out)
;
;   INPUT:
;    None.
;
;   OUTPUT:
;    status_in:     actual status of digital IO input.
;    status_out:    actual status of digital IO output.
;    err:           error code.
;
;   KEYWORDS:
;    None.
;
;   HISTORY:
;    10 July 2007
;    Created by Marco Xompero (MX)
;    marco@arcetri.astro.it
;-
Function adam_update

    @adsec_common
    if sc.name_comm eq "Dummy" or sc.model eq "P45" then begin

        return, adsec_error.ok    

    endif else begin

        out_dec = adam_modbus_ro_wrap()
        if (out_dec lt 0) then return, out_dec
        in_dec  = adam_modbus_ri_wrap()
        if (in_dec lt 0) then return, in_dec
        out_bin = conv2bin(uint(out_dec))
        in_bin = conv2bin(uint(in_dec))
        for i=1, 8 do adam_out.(i-1) = byte(strmid(out_bin, strlen(out_bin)-i, 1)) - 48
        for i=1, 8 do adam_in.(i-1) = byte(strmid(in_bin, strlen(in_bin)-i, 1)) - 48
        return, update_rtdb()

    endelse

end


Function adam_send, structin

    @adsec_common
    if sc.name_comm eq 'Dummy' or sc.model eq "P45" then begin
        err = adsec_error.ok
    endif else begin
        err = adam_modbus_ws_wrap(structin)
        if err ne  adsec_error.ok then return, err 
        err = adam_update()
        if err ne  adsec_error.ok then return, err 
    endelse

    return, adsec_error.ok

End

Function adam_close

    @adsec_common
    if sc.name_comm eq "Dummy" or sc.model eq "P45" then return, adsec_error.ok    $
                                                    else return, adam_modbus_disconnect_wrap(dummy)  

End

Function adam_enable_coils
    
    @adsec_common
    err = adam_update()
    if err ne adsec_error.ok then return, err
    out_str = adam_out
    out_str.tcs_driver_enable = 1
    err = adam_send(out_str)
    if err ne adsec_error.ok then return, err
    return, update_rtdb()

End


Function adam_disable_coils
    
    @adsec_common
    err = adam_update()
    if err ne adsec_error.ok then return, err
    out_str = adam_out
    out_str.tcs_driver_enable = 0
    err = adam_send(out_str)
    if err ne adsec_error.ok then return, err
    return, update_rtdb()
End

Function adam_enable_tss
    
    @adsec_common
    err = adam_update()
    if err ne adsec_error.ok then return, err
    out_str = adam_out
    out_str.tss_disable = 0
    err = adam_send(out_str)
    if err ne adsec_error.ok then return, err
    return, update_rtdb()

End

Function adam_disable_tss
    
    @adsec_common
    err = adam_update()
    if err ne adsec_error.ok then return, err
    out_str = adam_out
    out_str.tss_disable = 1
    err = adam_send(out_str)
    if err ne adsec_error.ok then return, err
    return, update_rtdb()

End

Function adam_enable_coils_dis_tss
    
    @adsec_common
    err = adam_update()
    if err ne adsec_error.ok then return, err
    out_str = adam_out
    out_str.tcs_driver_enable = 1
    out_str.tss_disable = 1
    err = adam_send(out_str)
    if err ne adsec_error.ok then return, err
    return, update_rtdb()

End


Function adam_power_on
    
    @adsec_common
    err = adam_update()
    if err ne adsec_error.ok then return, err
    out_str = adam_out
    out_str.main_power = 1
    err = adam_send(out_str)
    if err ne adsec_error.ok then return, err
    return, update_rtdb()

End

Function adam_power_off
    
    @adsec_common
    err = adam_disable_coils()
    if err ne adsec_error.ok then return, err

    out_str = adam_out
    out_str.main_power = 0
    err = adam_send(out_str)
    if err ne adsec_error.ok then return, err
    return, update_rtdb()

End

Function adam_sys_reset

    @adsec_common
    err = adam_update()
    if err ne adsec_error.ok then return, err
    out_str = adam_out
    out_str.tcs_sys_rst_n = 0
    err = adam_send(out_str)
    if err ne adsec_error.ok then return, err
    wait, 2
    out_str.tcs_sys_rst_n = 1
    err = adam_send(out_str)
    if err ne adsec_error.ok then return, err
    return, update_rtdb()

End

Function adam_sys_startup, HOLD=hold, KEEP_TSS=keep_tss

    @adsec_common
    err = adam_update()
    if err ne adsec_error.ok then return, err

    out_str = adam_out
    if ~keyword_set(HOLD) then begin
        
        out_str = adam_out
        out_str.tcs_sys_rst_n      = 0
        out_str.tcs_boot_select_n  = 1   ;;;;;; 0 DEFAULT, 1 USERS
        out_str.tcs_fpga_clr_n     = 1
        out_str.tcs_driver_enable  = 0
        ; Force tss_disable to 1 to have the same command sequence as src2
        if keyword_set(KEEP_TSS) then out_str.tss_disable=adam_out.tss_disable else out_str.tss_disable = 1
        ;out_str.tss_disable        = 1
        out_str.out5               = adam_out.out5
        out_str.out6               = 0
        out_str.main_power         = 0


    endif
    

    err = adam_send(out_str)
    if err ne adsec_error.ok then return, err
    wait, 1
    err = adam_power_on()    ;da qui il sistema si accende ma esce in reset
    if err ne adsec_error.ok then return, err
 
    ;wait, 1
    err = adam_sys_reset()   ;questo serve per rilasciare il reset
    if err ne adsec_error.ok then return, err
  
    wait, 5
    ;inizio patch(uncomment)
    err = adam_sys_reset()   ;patch per permettere l'inizializzazione con il nuovo sync
    if err ne adsec_error.ok then return, err
    wait, 5
    ;fine patch

    ;patch per permettere l'inizializzazione con il nuovo sync (comment)
    ;err = reset_hw()         
    ;if err ne adsec_error.ok then return, err
    ;fine patch

    ;;wait, 1
    ;err = reset_hw(/SWITCH)
    ;if err ne adsec_error.ok then return, err    
    ;return, adsec_error.ok
                                ;
    return, update_rtdb()
    
End

Function init_adam, stin, stout, STARTUP=startup, KEEP_TSS=keep_tss

    @adsec_common
    stout = {		       $
        tcs_sys_rst_n      :0, $
        tcs_boot_select_n  :1, $
        tcs_fpga_clr_n     :1, $
        tcs_driver_enable  :0, $
        tss_disable        :0, $
        out5               :1, $
        out6               :0, $
        main_power         :1  $
    }

    baseout = {		           $
        tcs_sys_rst_n      :-1, $
        tcs_boot_select_n  :-1, $
        tcs_fpga_clr_n     :-1, $
        tcs_driver_enable  :-1, $
        tss_disable        :-1, $
        out5               :-1, $
        out6               :-1, $
        main_power         :-1  $
    }
    ;L'ADAM LI RIBALTA TUTTI PER UN SUO MODO DI FUNZIONAMENTO
    ;QUESTI SONO I SEGNALI ELETTRICI REALI
    ; TO DO!!!!!
    ;LA KEYWORD "DRY" TIENE CONTO DI QUESTO FUNZIONAMENTO DELL'ADAM

    basein = {		               $
        tss_ac_power_fault_n0   :-1,$
        tss_ac_power_fault_n1   :-1,$
        in2                     :-1,$
        tss_fault_n             :-1,$
        tcs_sys_fault           :-1,$
        tcs_ac_power_fault_n2   :-1,$
        tcs_ac_power_fault_n1   :-1,$
        tcs_ac_power_fault_n0   :-1 $
    }

    stout = {		       $
        tcs_sys_rst_n      :0, $
        tcs_boot_select_n  :1, $
        tcs_fpga_clr_n     :1, $
        tcs_driver_enable  :0, $
        tss_disable        :0, $
        out5               :1, $
        out6               :0, $
        main_power         :1  $
    }
    adam_out = baseout
    adam_in = basein
    if sc.name_comm ne 'Dummy' and sc.model ne "P45" then begin
        if sys_status.adam_connected eq 0 then begin
            err = adam_modbus_init_wrap()
            if err ne adsec_error.ok then return, err
            sys_status.adam_connected = 1B
        endif
    endif

    if keyword_set(STARTUP) and !AO_CONST.side_master eq 1 then begin
        err = adam_update()
        if err ne adsec_error.ok then return, err
    ;    adam_in  = stin
    ;    adam_out = stout
        err = adam_sys_startup(KEEP_TSS=keep_tss)
        if err ne adsec_error.ok then return, err
    endif ;else begin
    ;    err = adam_update()
    ;    if err ne adsec_error.ok then return, err
    ;endelse

    return, adsec_error.ok

End


