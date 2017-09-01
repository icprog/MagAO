Function offload_icc

    @adsec_common

    err =  intersection(adsec.act_w_cl, adsec.act_wo_icc, act_w_cl_wo_icc)
    err =  complement(act_w_cl_wo_icc, adsec.act_w_cl, act_w_icc)
    err = read_seq_ch(sc.all,dsp_map.int_control_current, 1l, icc1)
    icc1[act_w_icc] -= float(mean(double(icc1[act_w_icc])))
    newDcmd = fltarr(adsec.n_actuators)
    newDcmd[act_w_icc] =((icc1)[act_w_icc])/(sys_status.pgain[act_w_icc]*(1e6)*sys_status.icc_dcgain[act_w_icc])
    newDcmd[act_w_cl_wo_icc] = sys_status.ctrl_current[act_w_cl_wo_icc]/(sys_status.pgain[act_w_icc]*(1e6))

    modalcf = transpose(adsec.ff_p_svec) ## transpose(newDcmd)
    modalcf[0:100] = 0
    newDcmd = float(adsec.ff_p_svec ## modalcf)
    newDcmd[adsec.act_w_cl] -= float(mean(double(newDcmd[adsec.act_w_cl])))
   
    
    err = set_offload_cmd(newDcmd[adsec.act_w_pos], /SWITCH, /NOCHECK, /DELTA, /START )
    return, err

End
