Function convert_env, data, BCU=BCU, NIOS=NIOS


;i valor i **** sono noti essere errati.
; DSP diagnostics
;

if not keyword_set(BCU) then begin  
   result = {                                                    $
      stratix_temp  : data.stratix_temp*0.0078125              , $; // temperatura del chip stratix (C deg)
      power_temp    : data.power_temp*0.0078125                , $; // temperatura board nella zona di power (C deg)
      dsps_temp     : data.dsps_temp*0.0078125                 , $; // temperatura board tra i due dsp (C deg)
      driver_temp   : data.driver_temp*0.0078125               , $; // temperatura board tra i driver di corrente (C deg)
      coil_current  : (data.coil_current and '0FFF'xu)*0.611-751 $; // corrente sui coils (mA)
   }
endif else begin
;
; bcu_diagnostics.txt
;
   result = {                                                    $;

      stratix_temp    : data.stratix_temp *0.0078125           , $; // temperatura del chip stratix (C deg)
      power_temp      : data.power_temp *0.0078125             , $; // temperatura board nella zona di power (C deg)
;
      bck_digitalIO   : data.bck_digitalIO                     , $; // canale spi sul backplane (MAX7301 U1) ?????
      voltage_vccl    : data.voltage_vccl*0.8364077            , $; // canale spi sul backplane (AD7927 U30) (mV)
;
      voltage_vcca    : data.voltage_vcca*0.8364077*0          , $; // canale spi sul backplane (mV) ****
      voltage_vssa    : data.voltage_vssa*4.0690104            , $; // canale spi sul backplane(mV)
      voltage_vccp    : data.voltage_vccp*0.8364077*0          , $; // canale spi sul backplane(mV) ****
      voltage_vssp    : data.voltage_vssp*4.0690104            , $; // canale spi sul backplane(mV)
      current_vccl    : data.current_vccl*12.2070313           , $; // canale spi sul backplane (mA)
      current_vcca    : data.current_vcca*12.2070313           , $; // canale spi sul backplane (mA)
      current_vssa    : data.current_vssa*12.2070313*0         , $; // canale spi sul backplane (mA) ****
;
      current_vccp    : data.current_vccp*12.2070313           , $; // canale spi sul backplane (AD7927 U31) (mA)
      current_vssp    : data.current_vssp*12.2070313*0         , $; // canale spi sul backplane (mA) ****
      tot_current_vccp: data.tot_current_vccp*2.6638253        , $; // canale spi sul backplane (mA) (to check...)
      tot_current_vssp: data.tot_current_vssp*2.6638253        , $; // canale spi sul backplane (mA) (to check...)
      tot_current_vp  : (float(data.tot_current_vp) - 2048) *5.3276505                       , $
;                                                                 ; // canale spi sul backplane (mA) (to check...)
      in0_temp        : float(data.in0_temp)/3499 - float(data.in0_temp)*5039/1000 + 10473   , $
;                                                                 ; // canale spi sul backplane (AD7927 U32) (C deg)
      in1_temp        : float(data.in1_temp)/3499 - float(data.in1_temp)*5039/1000 + 10473   , $
;                                                                 ; // canale spi sul backplane (C deg)
      out0_temp       : float(data.out0_temp)/3499 - float(data.out0_temp)*5039/1000 + 10473 , $
;                                                                 ; // canale spi sul backplane (C deg)
      out1_temp       : float(data.out1_temp)/3499 - float(data.out1_temp)*5039/1000 + 10473 , $
;                                                                 ; // canale spi sul backplane (C deg)
      ext_umidity     : float(data.ext_umidity)*3984/10000-274   $; // canale spi sul backplane (%)
   }
endelse

return, result
End
