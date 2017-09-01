
function pack_master_data, data, file2save

    @adsec_common
    npoints=n_elements(data) ;modificare
    if npoints ne 0 then begin
        slopes  =  data.switch_bcu.slopes
        nact=adsec.n_actuators
        dsp=data.crate_bcu.single_dsp_record
        wpos=adsec.act_w_pos
        wcur=adsec.act_w_curr

        DISTAVERAGE = reform(dsp.distaverage[*],nact, npoints)
        CURRAVERAGE = reform(dsp.curraverage[*],nact, npoints)
        MODES = reform(dsp.modes[*],nact, npoints)
        NEWDELTACOMMAND = reform(dsp.newdeltacommand[*],nact, npoints)
        FFCOMMAND = reform(dsp.ffcommand[*],nact, npoints)
        INTCONTROLCURRENT = reform(dsp.intcontrolcurrent[*],nact, npoints)
        FFPURECURRENT = reform(dsp.ffpurecurrent[*],nact, npoints)
        
        wfscounter= data.switch_bcu.switch_bcu_header.(0)
        mircounter= data.switch_bcu.switch_bcu_header.(2)    

        save, filename=file2save, DISTAVERAGE,CURRAVERAGE,MODES,NEWDELTACOMMAND,FFCOMMAND,INTCONTROLCURRENT,FFPURECURRENT,slopes,wfscounter,mircounter, wpos, wcur
        return, 0
    endif else begin

        print, 'No points in the structure'
        return, -1
    endelse
end
