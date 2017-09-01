;$Id: set_msgd_var.pro,v 1.2 2008/02/05 10:07:26 labot Exp $
;wrapper fot set_var to check the correct variable setting

Function set_msgd_var, name, send_data_buffer

     @adsec_common
     if (sc.host_comm eq "Dummy") then begin
         return, adsec_error.ok
     endif

     if test_type(send_data_buffer, /ULONG, /LONG, /INT, /UINT) then begin
        err = set_sint_var_wrap(name, send_data_buffer)
    endif else begin
        return, adsec_error.VAR_TYPE_ERROR
    endelse


End
