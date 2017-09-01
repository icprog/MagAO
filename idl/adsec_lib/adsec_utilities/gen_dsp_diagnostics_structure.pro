; $Id: gen_dsp_diagnostics_structure.pro,v 1.2 2007/02/07 18:06:08 marco Exp $

function gen_dsp_diagnostics_structure

@adsec_common

int16 = 0
uint16 = 0U
nc = adsec.n_act_per_dsp*adsec.n_dsp_per_board

dsp_diagnostics = $
{ $
	stratix_temp   :int16, $; // temperatura del chip stratix
	power_temp     :int16, $; // temperatura board nella zona di power
	dsps_temp      :int16, $; // temperatura board tra i due dsp
	driver_temp    :int16, $; // temperatura board tra i driver di corrente
	coil_current   :replicate(uint16,nc) $; // corrente sui coils
}
return, dsp_diagnostics
end
