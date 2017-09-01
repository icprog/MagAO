
; NAME:
;   FILL_FASTDIAGN
;
; PURPOSE:
;   Convert and fill a fast_diagnostic template with data passes.
;
; CATEGORY:
;   General utility.
;
; CALLING SEQUENCE:
;   FILL_FASTDIAG, data, buf, num, id
;
; INPUTS:
;   num  :  frame number
;   id   :  id actuators to fill
;   data :  data to convert.
;   buf  :  buffer to fill.
;
; KEYWORD PARAMETERS:
;   None.
;   
; OUTPUTS:
;   None. But the buf in input is affected.
;
; RESTRICTIONS:
;   None.
;
; MODIFICATION HISTORY:
;   30 nov 2005
;    Created by Marco Xompero (MX)
;    marco@arcetri.astro.it
;   March 2006, MX
;    Error in many array dimension.
;   07 Feb 2007 MX
;       LUT of variable deleted and name refurbished.
;   10 Sept 2007 MX, Daniela Zanotti (DZ)
;       Update the diagnostic table order.       
;
;-

Pro fill_fastdiag, data, buf, num, id,  BCU_ID=bcu_id, NOHEAD=nohead, SWITCHB=switchb

    @adsec_common

    if ~keyword_set(SWITCHB) then begin
        if keyword_set(NOHEAD) then nohead=4 else nohead=0
        if keyword_set(BCU_ID) then bcu_id = bcu else bcu_id=0
        dist_acc = dblarr(adsec.n_act_per_dsp)
        curr_acc = dblarr(adsec.n_act_per_dsp)
        dist_acc2 = dblarr(adsec.n_act_per_dsp)
        curr_acc2 = dblarr(adsec.n_act_per_dsp)
        ord = [0,2,1,3]
        startd =  bcu_id*(rtr.diag_len) + 12 - nohead
        startc =  bcu_id*(rtr.diag_len) + 20 - nohead
        startd2 = bcu_id*(rtr.diag_len) + 28 - nohead
        plusd2 =  bcu_id*(rtr.diag_len) + 36 - nohead
        startc2 = bcu_id*(rtr.diag_len) + 40 - nohead
        plusc2 =  bcu_id*(rtr.diag_len) + 48 - nohead

        For i=0, adsec.n_act_per_dsp-1 do begin
            dist_acc[i] = double(long64(data[startd+i*2:startd+i*2+1],0)) * dsp_const.pos_fixed_point
            curr_acc[i] = double(long64(data[startc+i*2:startc+i*2+1],0)) * dsp_const.curr_fixed_point
            dist_acc2[i] = long80double(ulong(data[startd2+i*2]), ulong(data[startd2+i*2+1]), ulong(data[plusd2+ord[i]/2]), dsp_const.pos_fixed_point^2, MSB = ord[i] mod 2)
            curr_acc2[i] = long80double(ulong(data[startc2+i*2]), ulong(data[startc2+i*2+1]), ulong(data[plusc2+ord[i]/2]), dsp_const.curr_fixed_point^2, MSB = ord[i] mod 2)

        endFor

        reord = ord

        offset = bcu_id*(rtr.diag_len) - nohead 
        if ~keyword_set(NOHEAD) then begin
            buf.wfs_frames_counter[num] = data[0]
            buf.param_selector[num] = data[1]
            buf.mirror_frames_counter[num] = data[2]
        endif
        buf.dist_average[id, num] = float(data[offset+4:offset+7], 0, adsec.n_act_per_dsp)
        buf.curr_average[id, num] = float(data[offset+8:offset+11], 0, adsec.n_act_per_dsp)
        buf.dist_accumulator[id, num] = dist_acc[reord]
        buf.curr_accumulator[id, num] = curr_acc[reord]
        buf.dist_accumulator2[id, num] = dist_acc2[reord]
        buf.curr_accumulator2[id, num] = curr_acc2[reord]
        buf.modes[id, num] =  float(data[offset+52:offset+55], 0, adsec.n_act_per_dsp)
        buf.new_delta_command[id, num] = float(data[offset+56:offset+59], 0, adsec.n_act_per_dsp)
        buf.ff_command[id, num] = float(data[offset+60:offset+63], 0, adsec.n_act_per_dsp)
        buf.int_ctrl_current[id, num] = float(data[offset+64:offset+67], 0, adsec.n_act_per_dsp)
        buf.ff_pure_current[id, num]  = float(data[offset+68:offset+71], 0, adsec.n_act_per_dsp)

        If ~keyword_set(NOHEAD) then begin
            buf.accelerometers_coeffs[*,num] = float(data[72:75], 0, 4)
            buf.wfs_frames_counter_check[num] = data[76]
            buf.param_selector_check[num] = data[77]
            buf.mirror_frames_counter[num] = data[78]
        endif 

    Endif else begin

        offset = rtr.n_slope+4
        buf.wfs_frames_counter[num]          = data[0]
        buf.param_selector[num]              = data[1]
        buf.mirror_frames_counter[num]       = data[2]
        buf.slopes[*,num]                    = float(data[4:offset-1],0, rtr.n_slope)
        buf.safe_skip_cnt[num]               = data[offset]
        buf.pend_skip_cnt[num]               = data[offset+1]
        buf.wfs_global_timeout_cnt[num]      = data[offset+2]
        buf.acc_coeffs[*,num]                = float(data[offset+8:offset+10], 0, 3)
        buf.timestamp[num]                   = data[offset+11] * 16.87d-6
        buf.wfs_frames_counter_check[num]    = data[offset+12]
        buf.param_selector_check[num]        = data[offset+13]
        buf.mirror_frames_counter_check[num] = data[offset+14]

    endelse

end
