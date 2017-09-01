; $Id: count2meter_nl.pro,v 1.4 2004/11/02 16:25:10 marco Exp $
;+
; HISTORY
;   02 Nov 2004, MX
;     Adsec.all changed in sc.all.
;-

function count2meter_nl, act_list, pos_count
;to be modified !!!
    @adsec_common 
    if act_list[0] eq sc.all_actuators then act_l=indgen(adsec.n_actuators) else act_l=act_list
    return, 1e-6*adsec.d0(act_l)*(2.0*(float(pos_count(act_l))-adsec.min_pos_val)/(float(adsec.max_pos_val)-adsec.min_pos_val)-adsec.c_ratio(act_l))^(-1)
end
