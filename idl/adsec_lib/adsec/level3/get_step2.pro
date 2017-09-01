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
function get_step2, type, number, amp, times, trig, pr_cmd, pr_cur, response $
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
		else: return, adsec_error.input_type
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

;partial data reduction for minimal saving
if type eq 'actuator' then begin
;TBW
endif else begin

    np = n_elements(response[*,0,0])
    scale=adsec.m_per_pcount[number]*1e9
    true = n_elements(adsec.true_act)

    dummy_pos=reform(response[*,0,*]-rebin(response[0,0,*],np,1,adsec.n_actuators))*scale
    mod_pos=transpose(ff_p_svec[number,*])##dummy_pos
    dummy_com=reform(response[*,2,*]-rebin(response[0,2,*],np,1,adsec.n_actuators))*scale
    mod_com=transpose(ff_p_svec[number,*])##dummy_com

    mod_cur=mod_cur/sqrt(true)     ;qui li cambio in metri!!!!!
    mod_comf=mod_comf/sqrt(true)     ;qui li cambio in metri!!!!!

    mod_pos=mod_pos/sqrt(true)     ;qui li cambio in metri!!!!!
    mod_com=mod_com/sqrt(true)     ;qui li cambio in metri!!!!!
    c0 = mod_com[0]
    c1 = mod_com[np-1]

    idst=where(abs(mod_pos) gt abs(c0+1.1*(c1-c0)) or abs(mod_pos) lt abs(c0+0.9*(c1-c0)), n_st)
    if n_st eq 0 then set_time = -1 else set_time = time(idst[n_st-1])-time[trig]
    maxypos= max(mod_pos, id_tmax)
    max_ov_v = maxypos
    max_ov_t = time[id_tmax]-time[trig]
    idrt=where(abs(mod_pos) ge abs(c0+0.9*(c1-c0)), pp)
    if pp eq 0 or  idrt[0] eq 0 then rise_time = -1 else rise_time =time[idrt[0]-1]-time[trig]
    steady_pos=rebin(mod_pos[n_elements(mod_pos)-10:*],1)

    dc = c1-c0
    overshoot = (maxypos-steady_pos)/steady_pos
    static_error = (steady_pos-dc)/dc

    mod_pos=transpose(ff_p_svec[oplot_list[ii],*])##dummy_pos
    mod_pos=mod_pos/sqrt(true)
    mod_com=transpose(ff_p_svec[oplot_list[ii],*])##dummy_com
    mod_com=mod_com/sqrt(true)



endelse



save, file=filename_base+'_single_step.sav', time, mod_com, mod_pos, number, dc, steady_pos, set_time, maxypos, max_ov_t, rise_time, overshoot, static_error
return,adsec_error.ok
end
