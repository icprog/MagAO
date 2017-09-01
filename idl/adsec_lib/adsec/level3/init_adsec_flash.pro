;$Id: init_adsec_flash.pro,v 1.2 2007/04/12 12:13:00 labot Exp $$
;
;+
;  NAME:
;   INIT_ADSEC_FLASH
;
;  PURPOSE:
;   Load DAC and ADC parameter in the adsec structure.
;
;  USAGE:
;   err = INIT_ADSEC_FLASH(STRUCT=struct, NOFILL=nofill)
;
;  INPUT:
;   None.
;
;  OUTPUT:
;   error               : error code.
;
;  KEYWORDS:
;   STRUCT              : structure with parameters loaded and calculated.
;   NOFILL              : do not fill the adsec structure.
;
;  COMMON BLOCKS:
;   ADSEC               : secondary adaptive base common block. A part of it will be filled.
;
;  PACKAGE:
;   LBT Adsec Libraries.
;
;  HISTORY
;   Created on 27 march 2007 by Daniela Zanotti (DZ).
;   zanotti@arcetri.astro.it
;  
Function init_adsec_flash, STRUCT=struct, NOFILL=nofill


   @adsec_common


   n_acts = adsec.n_actuators
   n_gain = 6
   n_ch_per_board = adsec.n_act_per_dsp*adsec.n_dsp_per_board
 
;===========================================================================
; Read the data gain data
    err = get_flash_data(sc.all, mat_cal)
    if err ne adsec_error.ok then return, err   
    
    ADC_spi_offset = float(reform(mat_cal[*,0,*], n_acts)) 
    ADC_spi_gain = float(reform(mat_cal[*,1,*], n_acts))
    ADC_gain = float(reform(mat_cal[*,2,*], n_acts))
    ADC_offset = float(reform(mat_cal[*,3,*], n_acts))
    DAC_offset_cal = float(reform(mat_cal[*,4,*], n_acts))
    DAC_gain = float(reform(mat_cal[*,5,*], n_acts))
;==========================================================================

; Filling the adsec structure.
   struct = {                                           $ 
      ADC_spi_offset : ADC_spi_offset,                  $
      ADC_spi_gain : ADC_spi_gain,                      $
      ADC_gain : ADC_gain,                              $
      ADC_offset : ADC_offset,                          $
      DAC_GAIN : DAC_GAIN,                              $
      DAC_offset_cal : DAC_offset_cal                              $
   
}

   if not keyword_set(NOFILL) then begin
      adsec.ADC_spi_offset= ADC_spi_offset 
      adsec.ADC_spi_gain  = ADC_spi_gain  
      adsec.ADC_gain      = ADC_gain 
      adsec.ADC_offset    = ADC_offset 
      adsec.DAC_GAIN      = DAC_GAIN  
      adsec.DAC_offset_cal      = DAC_offset_cal  
   endif

   return, adsec_error.ok

End
