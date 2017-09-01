;$Id: curr2bias.pro,v 1.3 2008/03/12 09:48:11 zanotti Exp $$
;+
;pro CURR2BIAS
;   the procedure writes the bias configuration file from the bias force file given by ADS.
;   HISTORY
;   April 2007
;   written by Daniela Zanotti(DZ)
;-
pro curr2bias, OUTPUT_FILE=output_file
    @adsec_common
    ; se non esiste il file gia' ordinato allora lo ricalcolo
    if (file_search(adsec_path.conf+'bias_force.txt')) eq 0 then $
        gen_config, bias_force,INPUT = adsec_path.conf+'temp/magnetBias.txt',/only else $
        bias_force=read_ascii_array(adsec_path.conf+'bias_force.txt')
    n_true_act = n_elements(adsec.true_act)
    mass_force = (adsec_shell.mass*1e-3*9.81/n_true_act)
    ;TODO le forze di bias sono state misurate ad una distanza dal coil di rif di 0.2mm, bisogna riscalarle alns gap (delta)+gap_nominale
    curr4bias = bias_force/mass_force

    if keyword_set(output_file) then file_path = adsec_path.conf+output_file else $
        file_path = dialog_pickfile(title='Bias mag file ')

    write_array_ascii, curr4bias , FILENAME=file_path
end
