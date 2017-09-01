;$Id: set_curr2bias.pro,v 1.2 2007/11/09 14:46:46 marco Exp $$
;+
;   NAME:
;    SET_CURR2BIAS
;
;   PUPRPOSE:
;    It trasfers the current in the bias current before close the optical loop.
;
;   USAGE:
;     err = set_curr2bias()
;
;   KEYWORDS:
;    CONTROL:   transfers control current to bias
;    FULLBIAS:  transfers full_bias current to bias with 1e-3 [N] steps
;    XADSEC:    update engineering GUI
;
;   HISTORY:
;    Created by Marco Xompero on Nov 2007
;    marco@arcetri.astro.it
;-
function set_curr2bias, CONTROL=control, FULLBIAS=fullbias, XADSEC=xadsec

     @adsec_common
     old_st = rtr.oversampling_time
     ;err = set_diagnostic(OVER=0.0)
     ;if err ne adsec_error.ok then return, err
     ;wait, old_st > 0.01

     time_ps = time_preshaper()
     wait, time_ps
     err = update_status()
     if err ne adsec_error.ok then return, err

     err = set_diagnostic(OVER=0.0)
     if err ne adsec_error.ok then return, err
     wait, old_st > 0.01

     if keyword_set(FULLBIAS) then begin
         
         bias = sys_status.bias_current
         null_slopes = fltarr(rtr.n_slope)
         
         err = write_same_ch(sc.all, dsp_map.fullbias_curr, 0l)
         if err ne adsec_error.ok then return, err
         
         err = get_commands(adsec.act_w_curr, cmd_off, /CURR)
         if err ne adsec_error.ok then return, err
         
         nstep = round(max(abs(cmd_off))/1e-3)
         
         for i=0, nstep-1 do begin 
             
             curr2w = bias + i* (cmd_off)/(nstep-1)
             cmd_off2w = cmd_off - cmd_off/(nstep-1) * i
             err = set_offload_force(cmd_off2w, /SWITCHB, /START)
             err = write_bias_curr(adsec.act_w_curr, curr2w)
             if err ne adsec_error.ok then return, err
             err = wfs_switch_step(null_slopes)
             if err ne adsec_error.ok then return, err
             wait, time_ps
             err = update_status(1)
             if err ne adsec_error.ok then return, err
             if keyword_set(XADSEC) then update_panels, /NO_READ, /OPT
             
         endfor
         
         err = set_offload_force(/RESET, /NOCHECK)
         if err ne adsec_error.ok then return, err
                
     endif

     if keyword_set(CONTROL) then begin
         
         curr = sys_status.bias_current[adsec.act_w_curr]+sys_status.ctrl_current[adsec.act_w_curr]
         err = set_cmd2bias(/POSITION)
         if err ne adsec_error.ok then return, err

         err = write_bias_curr(adsec.act_w_curr, curr)
         if err ne adsec_error.ok then return, err
         err = wfs_switch_step(null_slopes)
         if err ne adsec_error.ok then return, err
         wait, time_ps
         err = update_status(1)
         if err ne adsec_error.ok then return, err
         if keyword_set(XADSEC) then update_panels, /NO_READ, /OPT

      endif

         err = set_diagnostic(OVER=old_st)
         if err ne adsec_error.ok then return, err
         wait, old_st > 0.01
         return, err
end
