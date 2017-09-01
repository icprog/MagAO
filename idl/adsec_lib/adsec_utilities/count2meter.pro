; $Id: count2meter.pro,v 1.3 2003/06/10 18:32:23 riccardi Exp $

function count2meter, pos_count, ACT_LIST=act_list

    @adsec_common 
    if n_elements(act_list) eq 0 then act_list=indgen(adsec.n_actuators)
    if n_elements(pos_count) ne n_elements(act_list) then message,'Size mismatch between actuator list and position values'

    return, ((float(pos_count)-adsec.min_pos_val)*adsec.m_per_pcount[act_list]) $
            +adsec.min_lin_dist[act_list]*1e-6
end
