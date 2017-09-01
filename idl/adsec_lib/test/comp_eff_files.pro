;Used to compute real efficiency to be written
;
    ref_mag = 100
    meff_file = './conf/mag_eff.txt'
    ceff_file = './conf/coils_eff.txt'


;read magnets efficiency
    meff = read_ascii_array(meff_file)
    if n_elements(meff) lt adsec.n_actuators then begin
        
        print, FORMAT='(%"WARNING: %3.3I actuators missed. The LAST value will be used to pad the vector.")', adsec.n_actuators - n_elements(meff)

        meff = [meff, replicate(meff(n_elements(meff)-1), adsec.n_atuators-n_elements(meff))]
    endif

;read coils efficiency
    ceff = read_ascii_array(ceff_file)
    if n_elements(ceff) ne adsec.n_actuators then begin
        print, 'actuators number wrong. Nothing done.'
    endif else begin
;compute actuators efficiency
        
        act_eff = ceff * meff / meff[ref_mag]
        write_ascii_array, 'act_eff_tmp.txt', act_eff
    endelse
end
