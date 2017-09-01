Function  test_dist_threshold, resume


    @adsec_common
    resume = fltarr(adsec.n_actuators)
    err = set_dist_threshold(sc.all, 2., 1ul)
    wait, 2.
    time=2.0
    
    for i=0, adsec.n_actuators-1 do begin

        err = set_dist_threshold(i, 20e-6, 1ul)
        wait, time
        if sc.model eq 'P45' then err =  getdiagnvalue(process_list.housekeeper.msgd_name,  'BCUResetStatusBusPowerFault', 0,0,cc)   $
                             else err =  getdiagnvalue(process_list.housekeeper.msgd_name,  'BCUDigitalIOBusSysFault', 0,0,cc)

        ;controllo la scheda che ha generato il fault
        idb = i /  8
        err =  getdiagnvalue(process_list.housekeeper.msgd_name,  'DSPDriverStatusBusPowerFault', idb,idb,df) 
        err =  getdiagnvalue(process_list.housekeeper.msgd_name,  'DSPDriverStatusBusPowerFaultCtrl', idb,idb,dfc)
        if (df.last ne 1) or (dfc.last ne 1) then message, "ERROR: the board" + strtrim(idb, 2) +" not generated bus_sys_fault"

        if cc.last eq 1 then print, "Act #"+string(i, FORMAT='(I3.3)')+ ' OK'  $
                        else print, "Act #"+string(i, FORMAT='(I3.3)')+ ' BAD' 
        resume[i] = cc.last
        err = set_dist_threshold(i, 2.0, 1ul)
        err = adam_disable_coils()
        err = adam_enable_coils()

        wait, time
        if sc.model eq 'P45' then err =  getdiagnvalue(process_list.housekeeper.msgd_name,  'BCUResetStatusBusPowerFault', 0,0,cc)   $
                             else err =  getdiagnvalue(process_list.housekeeper.msgd_name,  'BCUDigitalIOBusSysFault', 0,0,cc)
        err =  getdiagnvalue(process_list.housekeeper.msgd_name,  'BCUDigitalIOBusSysFault', 0,0,cc)   
        if cc.last ne 0 then message, "ERROR: BUS SYS FAULT NOT RESET"

    endfor

    plot, resume, psym=-4
    return, adsec_error.ok
end
