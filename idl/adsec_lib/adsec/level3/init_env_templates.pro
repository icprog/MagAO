Pro init_env_templates
    
   @adsec_common

   dsp_diagnostics = gen_dsp_diagnostics_structure()
   dsp_diagnostics = replicate(convert_env(dsp_diagnostics), adsec.n_board_per_bcu)
   bcu_diagnostics = gen_bcu_diagnostics_structure()
   bcu_diagnostics = convert_env(bcu_diagnostics, /BCU)

   dsp_nios = gen_dsp_nios_structure()
   dsp_nios = replicate(dsp_nios, adsec.n_board_per_bcu+1)
   bcu_nios = gen_bcu_nios_structure()
   dsp_nios = bcu_nios 
   
End

