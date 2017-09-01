; $Id: step_ff.pro,v 1.13 2008/02/05 10:43:39 labot Exp $

;+
;   NAME:
;    STEP_FF
;
;   PURPOSE:
;    Step response utility for the adaptive secondary mirror.
;
;   NOTE:
;    The mirror has to be set and the local closed loop enabled.
;
;   USAGE:
;    err=step_ff(type, number, amp, times, trig, pr_cmd, pr_cur, response $
;        ,sys_status0=sys_status0,sys_status1=sys_status1, ADDR=addr2save)
;
;   INPUT:
;    type:              type of step response ('actuator', 'modal' or 'zern').
;    number:            list of actuators/modes to step.
;    amp:               mode/actuators amplitude.
;    times:             buffer length for the acquisition.
;    trig:              triggering value.
;
;   OUTPUT:
;    pr_cmd:            current preshaper set.
;    pr_cur:            command preshaper set.
;    response:          step response given.
;    sys_status0:       sys_status structure at the start of the step response.
;    sys_status1:       sys_status structure at the end of the step response.
;
;   KEYWORDS:
;    ADDR       : dsp memory map address to monitor. Default distance, float_dac_value, preshaped_cmd.
;
;   HISTORY:
;
;   13 Mar 2002, A. Riccardi (AR)
;       Bug fixed in the CONT keyword setting of the MESSAGE call.
;
;   10 Mar 2004, AR
;       Keyword CHECK_PM_16BIT in read_seq_dsp call no longer used
;
;   12 Aug 2004, AR
;       Modifications for matching LBT specifications
;       More checks on input parameters
;       return error correctly managed
;   02 Nov 2004, MX
;       Adsec.all changed in sc.all.
;       08 Feb 2005, MX
;    Data types bug fixed.
;       Help written.
;   25 May 2005, MX
;       Dsp_map updated to rtr_map.
;   26 Nov 2006, MX
;       Delay_acc len fixed (from 0 to 700). The accumulation began before steady state.
;   07 Feb 2007 MX
;       LUT of variable deleted and name refurbished.
;   26 Feb 2007 MX and AR
;       Update ff application and code cleaning removing old useless settings.
;-
;
; function per la risposta temporale
; suppone che lo specchio sia settato e con ctrl enabled

function step_ff, type, number, amp, times, trig, pr_cmd, pr_cur, response $
        ,sys_status0=sys_status0,sys_status1=sys_status1, ADDR=addr2save

@adsec_common

;stop
if n_elements(addr2save) eq 0 then begin
    addr2save=[dsp_map.distance,dsp_map.float_dac_value,dsp_map.preshaped_cmd]
endif else begin
    if test_type(addr2save, /LONG, /ULONG, N_EL=n_el) then begin
        message, "The address to read data from must be integer" $
                 , CONT= (sc.debug eq 0B)
        return, adsec_error.input_type
    endif
    if n_el gt 3 then begin
        message, "No more than 3 addresses per actuator can be requested" $
                 , CONT= (sc.debug eq 0B)
        return, adsec_error.input_type
    endif

endelse

offset_trig=1000       ;extra time to wait for the comunication crosstalk to go to zero

err=update_status(1024)
sys_status0=sys_status

if number lt 0 or number gt adsec.n_actuators then begin
    message, "Unexpected mode/act number.", CONT=sc.debug eq 0B
    return, adsec_error.input_type
endif

if times lt 1 or times gt dsp_const.max_data_IO_len then begin
    message, "Unexpected number of samples for the step responce.", CONT=sc.debug eq 0B
    return, adsec_error.input_type
endif

if trig lt 0 or trig gt times then begin
    message, "Unexpected value for the step trigger.", CONT=sc.debug eq 0B
    return, adsec_error.input_type
endif

;parameters
case type of
"actuator": begin
                mode=replicate(0.0,adsec.n_actuators)
                mode(number)=1.0
            end
"modal": begin
            mode=float(adsec.ff_p_svec[number,*])
         end
"zern": begin
            mode=float(get_zern_pattern(number))
         end
else: return,-1
endcase

;settaggio accumul. nuova versione ff
num_samples=1024
delay_acc=700 

;*****start correction old things....
;err = set_acc(num_samples, delay_acc, old_samples, old_delay)
;if err ne adsec_error.ok then return, err

;;err = write_same_dsp(sc.all_dsp,dsp_map.acc_divisor,fix(32767.0/num_samples)+1)
;;if err ne adsec_error.ok then message, "ERROR!"

old_ovs = rtr.oversampling_time

err = set_diagnostic(over=0.0, MASTER=0)
if err ne adsec_error.ok then print, "Message ERROR!!!"
wait, 0.5

err = get_commands(sc.all_actuators, command)
if err ne adsec_error.ok then return, err

;err = set_position_vector(command)
;if err ne adsec_error.ok then return, err

;err = update_command()
;if err ne adsec_error.ok then return, err

;wait,(adsec.sampling_time*num_samples)>0.01


err = set_delta_position_vector(amp*mode)
if err ne adsec_error.ok then return, err

n_addr2save = n_elements(addr2save)
nbuff= indgen(n_addr2save)

;err = set_IO_buffer(sc.all_dsp, nbuff, addr2save, times, $
;                    rtr_map.start_ff, trig, /TRIG, RECORD=adsec.n_act_per_dsp)

err = set_IO_buffer(sc.all_dsp, nbuff, addr2save, times, $
                    rtr_map.update_ff, trig, /TRIG, RECORD=adsec.n_act_per_dsp)
if err ne adsec_error.ok then return, err

err = get_preshaper(sc.all, pr_cmd_old)
if err ne adsec_error.ok then return, err
err = get_preshaper(sc.all, pr_cur_old, /FF)
if err ne adsec_error.ok then return, err

err = set_preshaper(sc.all, pr_cmd)
if err ne adsec_error.ok then return, err
err = set_preshaper(sc.all, pr_cur, /FF)
if err ne adsec_error.ok then return, err

;stop

err = enable_IO_buffer(sc.all_dsp, nbuff)
if err ne adsec_error.ok then message, "ERROR!"

;wait, 0.01
;stop

err=start_ff(trig+offset_trig)
if err ne adsec_error.ok then message, "ERROR!"

wait, (1.1*(trig+offset_trig+times)*adsec.sampling_time) > 0.01

err=update_status(1024)
sys_status1=sys_status

;verifica del fischio
;wait, 1

;err = clear_ff()
;if err ne adsec_error.ok then message, "ERROR!"

;err = set_position_vector(command)
;if err ne adsec_error.ok then return, err

;err =  update_command()
;if err ne adsec_error.ok then message, "ERROR!"

err = set_delta_position_vector(-amp*mode)
if err ne adsec_error.ok then return, err

err = start_ff()
if err ne adsec_error.ok then message, "ERROR!"
wait, 0.05

err = set_preshaper(sc.all, pr_cmd_old)
if err ne adsec_error.ok then return, err
err = set_preshaper(sc.all, pr_cur_old, /FF)
if err ne adsec_error.ok then return, err

err=read_IO_buffer_ch(sc.all_dsp, nbuff, times, response)
if err ne adsec_error.ok then return, err

err = set_diagnostic(over=old_ovs, /MASTER)
if err ne adsec_error.ok then print, "Message ERROR!!!"
wait, 0.5


return, adsec_error.ok
end
