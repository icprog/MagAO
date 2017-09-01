; $Id: gen_bcu_diagnostics_structure.pro,v 1.3 2007/02/07 18:06:08 marco Exp $

function gen_bcu_diagnostics_structure

uint16 = 0U
uint32 = 0UL

bcu_diagnostics = $
{ $
	stratix_temp    :uint16, $;     // temperatura del chip stratix
	power_temp      :uint16, $;       // temperatura board nella zona di power

	bck_digitalIO   :uint32, $;    // canale spi sul backplane (MAX7301 U1)
	voltage_vccl    :uint16, $; // canale spi sul backplane (AD7927 U30)

	voltage_vcca    :uint16, $; // canale spi sul backplane
	voltage_vssa    :uint16, $; // canale spi sul backplane
	voltage_vccp    :uint16, $; // canale spi sul backplane
	voltage_vssp    :uint16, $; // canale spi sul backplane
	current_vccl    :uint16, $; // canale spi sul backplane
	current_vcca    :uint16, $; // canale spi sul backplane
	current_vssa    :uint16, $; // canale spi sul backplane

	current_vccp    :uint16, $; // canale spi sul backplane (AD7927 U31)
	current_vssp    :uint16, $; // canale spi sul backplane
	tot_current_vccp:uint16, $; // canale spi sul backplane
	tot_current_vssp:uint16, $; // canale spi sul backplane
	tot_current_vp  :uint16, $; // canale spi sul backplane

	in0_temp        :uint16, $; // canale spi sul backplane (AD7927 U32)
	in1_temp        :uint16, $; // canale spi sul backplane
	out0_temp       :uint16, $; // canale spi sul backplane
	out1_temp       :uint16, $; // canale spi sul backplane
	ext_umidity     :uint16  $; // canale spi sul backplane
}
return, bcu_diagnostics
end
