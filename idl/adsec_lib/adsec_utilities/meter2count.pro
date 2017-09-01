; $Id: meter2count.pro,v 1.3 2003/06/10 18:32:24 riccardi Exp $

function meter2count, pos_meter, ACT_LIST=act_list

    @adsec_common 
    if n_elements(act_list) eq 0 then act_list=indgen(adsec.n_actuators)
    if n_elements(pos_meter) ne n_elements(act_list) then message,'Size mismatch between actuator list and position values'
    dummy = round((pos_meter-adsec.min_lin_dist[act_list]*1e-6)/adsec.m_per_pcount[act_list]+adsec.min_pos_val)
    dummy = dummy > adsec.min_pos_val < adsec.max_lin_output[act_list]

    return, fix(dummy)
end
