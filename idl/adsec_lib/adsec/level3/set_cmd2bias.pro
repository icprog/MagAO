;$Id: set_cmd2bias.pro,v 1.6 2009/05/22 16:32:55 labot Exp $
;+
;   NAME:
;    SET_CMD2BIAS
;
;   PUPRPOSE:
;    To trasfer the command current in the bias current before close the optical loop.
;
;   USAGE:
;     err = set_cmd2bias()
;
;   INPUT:
;    None.
;
;   OUTPUT:
;    err:   error code.
;
;   KEYWORDS:
;    POSITION: final command given by current position
;
;   HISTORY:
;    Created by D.Zanotti & M.Xompero on 2005
;    07 Feb 2007 MX
;       LUT of variable deleted and name refurbished.
;    04 Nov 2007 MX
;       POSITION keyword added. Fast diagnostic managed.
;-
function set_cmd2bias, POSITION=position
     @adsec_common

     old_st = rtr.oversampling_time
     err = set_diagnostic(OVER=0.0)
     wait, old_st > 0.01

     if keyword_set(POSITION) then begin

         final_cmd = sys_status.position

     endif else begin

         err=get_commands(sc.all,cmd_now)
         if err ne adsec_error.ok then return, err
         ;err = get_commands(sc.all, cmd_off, /offloadcmd)
         ;if err ne adsec_error.ok then return, err
         final_cmd = cmd_now   ;+cmd_off

     endelse

     err = write_bias_cmd(transpose(final_cmd), CHECK=check)
     if err ne adsec_error.ok then return, err

     err = set_offload_cmd(/RESET, /NOCHECK)
     if err ne adsec_error.ok then return, err
     ;initialize to zero new_delta_command = c1
     err = write_seq_ch(sc.all,dsp_map.new_delta_command, fltarr(1,adsec.n_actuators), /check)
     if err ne adsec_error.ok then return, err
     ;!NOTE!:the new_delta command in the  previous step must be zero.
     err = update_status(2)
     if err ne adsec_error.ok then return, err

     err = set_diagnostic(OVER=old_st)
     if err ne adsec_error.ok then return, err
     wait, old_st > 0.01
     

return, err
end
