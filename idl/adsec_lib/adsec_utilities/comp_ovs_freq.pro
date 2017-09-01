Function comp_ovs_freq, wfs_period_tmp, BOUND=tw

    wfs_period = (float(wfs_period_tmp) > 0.001) < 0.1
    td = 0.001
    if n_elements(tw) eq 0 then tw = 0.0015
    if wfs_period lt 0.002 then new_ovs = 0.0025
    if wfs_period gt 0.002 then begin
        tmp = floor((wfs_period-td)/tw)
        new_ovs = wfs_period / tmp
    endif
    PRINT, "wfs_period ", wfs_period
    print, "new_ovs", new_ovs
    return, new_ovs
end

    
