; $Id: get_step.pro,v 1.10 2008/02/05 10:20:28 labot Exp $
;+
;   NAME:
;    GET_STEP
;
;   PURPOSE:
;    Step response utility for the adaptive secondary mirror.
;
;   NOTE:
;    The mirror has to be set and the local closed loop enabled.
;
;   USAGE:
;    err=get_step(type, number, amp, times, trig, pr_cmd, pr_cur, response $
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
;
;   KEYWORDS:
;    EXT:               extra filename for data saving.
;    DIR:               folder location for data saving.
;    NO_SAVE:           save no data.
;    CURRENT2SAVE:      integer, kind of current to save selection:
;                       0: (default) total current (out_dac_chx) is saved in response[*,1,*]
;                       1: error position current (err_control_chx) is saved in response[*,1,*]
;                       2: speed position current (speed_control_chx) is saved in response[*,1,*]
;                       3: preshaped bias+FF current (preshaped_curr_chx) is saved in response[*,1,*]
;    MAX_T:             max_t keyword passed to plot_ff.
;    OVERPLOT_LIST:     overplot_list keyword passed to plot_ff.
;    POS_PSYM:          pos_psym keyword passed to plot_ff.
;    SMOOTH:            smooth keyword passed to plot_ff.
;    NO_PLOT:           the procedure doesn't plot data.
;    SAME_DISPLAY_CUTS: same color cuts in the display
;   
;
; HISTORY
;   Written by A. Riccardi (AR) and G. Brusa
;
;   12 Aug 2004, AR
;    POS_PSYM keyword added.
;   08 Feb 2005 ??
;    SMOOTH keyword added. Help rewritten.
;   14 Nov 2006, MX
;    Added no_plot and same_display_cuts keywords.
;   07 Feb 2007 MX
;       LUT of variable deleted and name refurbished.
;   26 Feb 2007 MX
;       SET_TIME keyword added to plot_step
;-
function get_step, type, number, amp, times, trig, pr_cmd, pr_cur, response $
                 , EXT=ext, DIR=dir, NO_SAVE=no_save, CURRENT2SAVE=curr2save $
                 , MAX_T=max_t_range, OVERPLOT_LIST=oplot_list, POS_PSYM=pos_psym $
                 , SMOOTH=smooth, NO_PLOT=no_plot, SAME_DISPLAY_CUTS=same_cuts

@adsec_common
if n_elements(ext) eq 0 then ext=''
if n_elements(dir) eq 0 then dir=''

if n_elements(curr2save) eq 0 then begin
	addr_curr = dsp_map.float_dac_value
endif else begin
	case curr2save of
		0: addr_curr = dsp_map.float_dac_value
		1: addr_curr = dsp_map.pos_current
		2: addr_curr = dsp_map.speed_current
		3: addr_curr = dsp_map.preshaped_curr
		else: message, "CURRENT2SAVE keyword containes no valid value"
	endcase
endelse

addr2save = [dsp_map.distance, addr_curr, dsp_map.preshaped_cmd]
;addr2save = [dsp_map.distance, dsp_map.pos_current, dsp_map.preshaped_cmd]

err = step_ff(type, number, amp, times, trig, pr_cmd, pr_cur, response, ADDR=addr2save)
if err ne adsec_error.ok then return,err


if ~ keyword_set(no_plot) then begin

filename_base =  filepath(ROOT=dir,strmid(type,0,3)+'_'+string(number,form='(i3.3)') $
	           +'_'+ext)
plot_step, type, number, amp, times, trig, pr_cmd, pr_cur, response $
         , MAX_T=max_t_range, OVERPLOT_LIST=oplot_list, POS_PSYM=pos_psym, SMOOTH=smooth $
         , SAME_DISPLAY_CUTS=same_cuts, SET_TIME=set_time, FILE=filename_base
endif


if (~ keyword_set(no_save)) then begin
	filename = filepath(ROOT=dir,strmid(type,0,3)+'_'+string(number,form='(i3.3)') $
	           +'_'+ext+'.sav')
	save,file= filename, type, number, amp, times, trig, pr_cmd, pr_cur, response, set_time
endif

return,adsec_error.ok
end
