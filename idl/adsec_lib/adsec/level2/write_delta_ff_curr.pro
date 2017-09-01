; $Id: write_delta_ff_curr.pro,v 1.4 2007/02/07 16:57:58 marco Exp $
;+
;   07 Feb 2007 MX
;       LUT of variable deleted and name refurbished.
;       Polling on nendof_ff removed.
;-

function write_delta_ff_curr, act_list, curr_vector

@adsec_common 

err = write_seq_ch(act_list, dsp_map.ff_current, curr_vector)

return, err

end
