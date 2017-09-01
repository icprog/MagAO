for i=0, 100 do begin


    err=init_adsec_sram(ST=sram_data, NOFILL=nofill_sram)
    if err ne adsec_error.ok then message,"The adsec ADC e DAC calibrations fields are not initialized"

    log_print, string(i)
    log_print, "Stop diagnostic..."
    err=set_diagnostic(over=0.0)
    if err ne adsec_error.ok then break

    log_print, "Stop master..."
    err=set_diagnostic(MASTER=0)
    if err ne adsec_error.ok then break

    log_print, "Stop watchdog..."
    err = disable_watchdog(/DSP)
    if err ne adsec_error.ok then break

    log_print, "Sleep for update..."
    err=sleep4update()
    if err ne adsec_error.ok then break

    log_print, "Stop interrupt..."
    err = manage_interrupt(/DISABLE)
    if err ne adsec_error.ok then break
    
    log_print, "Send wave..."
    err = init_adsec_wave()
    if err ne adsec_error.ok then break
;
    log_print, "Fiber test..."
    err = fastlink_alignment(/VER, TEST=10)
    if err ne adsec_error.ok then break

end
end


