; NAME:
;   GET_STATUS
;
; PURPOSE:
;       Get sys_status, adsec, and the control gain .
;
; CATEGORY:
;   General utility.
;
; CALLING SEQUENCE:
;   GET_STATUS, status_save
;
; INPUTS:
;
; KEYWORD PARAMETERS:
;   None.
;
; OUTPUTS:
;   status_save. 
;
; RESTRICTIONS:
;   None.
;
; MODIFICATION HISTORY:
;
;-

Function get_status, status_save
    
    @adsec_common
    err = update_status()
    if err ne adsec_error.ok then return, err
    
    err = get_gain(sc.all, pgain)
    if err ne adsec_error.ok then return, err

    err = get_gain(sc.all, dgain, /SPEED)
    if err ne adsec_error.ok then return, err

    err = get_preshaper(sc.all, presh)
    if err ne adsec_error.ok then return, err

    err = get_preshaper(sc.all, cpresh, /FF)
    if err ne adsec_error.ok then return, err

    caldat, systime(/JULIAN), month, day, year
    date_str = STRING(year, FORMAT="(I4.4)")+"_"+ $
               STRING(month,FORMAT="(I2.2)")+"_"+ $
               STRING(day,  FORMAT="(I2.2)")

    status_save = {                        $
        sys_status:         sys_status,    $
        adsec:              adsec,         $
        adsec_shell:        adsec_shell,   $
        pgain:              pgain,         $
        dgain:              dgain,         $
        pos_preshaper:      presh,         $
        curr_preshaper:     cpresh,        $
        date:               date_str,      $
        ver:                '1.0'          $
    }
        
    return, adsec_error.ok

End
    
