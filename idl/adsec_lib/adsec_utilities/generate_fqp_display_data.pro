Function generate_fqp_display_data, tn
    
    @adsec_common
    tn = tracknum()
    filename = meas_path('displaydata', dat=tn)+'adsec_struct.sav'
    save, file=filename, adsec, adsec_shell, gr, sc
    log_print, "Data saved in: "+filename
    return, adsec_error.ok

End
