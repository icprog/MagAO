;$Id: switch_trigger.pro,v 1.2 2009/04/10 13:25:07 labot Exp $$
;
;+
; NAME:
;   SWITCH_TRIGGER
;
; PURPOSE:
;   Use fiber communication to trigger action on the DSP boards using dsp_map.test_address.
;   !!! NOTE !!! : DO NOT USE IT if the OVERSAMPLING FRAMES or SLOPE COMPUTER are activated.
;
; USAGE:
;   err = switch_trigger()
;
; INPUT:
;   None.
;
; OUTPUT:
;   err: error code.
;
; KEYWORDS:
;   None.
;
; HISTORY:
;   Created by Marco Xompero on Apr 2009.
;
;-

function switch_trigger


    @adsec_common
    err = write_same_dsp(0, rtr_map.switch_scslopevector, lonarr(40)+1,/switch)


    first_crt = 0
    last_crt = adsec.n_crates - 1 
    num_dsps_crt = adsec.n_dsp_per_board*adsec.n_board_per_bcu 
    mode_last_crt = ceil(float(rtr.n_modes2correct) / (adsec.n_dsp_per_board * adsec.n_act_per_dsp*adsec.n_board_per_bcu)) - 1
    num_dsps_crt = adsec.n_dsp_per_board*adsec.n_board_per_bcu 
    first_dsp = 0
    last_dsp = ceil((adsec.n_actuators / float(adsec.n_act_per_dsp))/2)*2  - 1
    mode_last_dsp = ceil((rtr.n_modes2correct/float(adsec.n_act_per_dsp))/2)*2-1
    last_dsp_single_crt = (last_dsp-(last_crt-first_crt)*num_dsps_crt)
    mode_last_dsp_single_crt = mode_last_dsp-(mode_last_crt-first_crt)*num_dsps_crt

    flink = ulonarr(4)
    ;wr_same a tutti i DSP di 40w
    flink[0] = 40*65536ul+8192
    flink[1] = 0ul*16777216+last_crt*1048576ul+last_dsp_single_crt*4096ul+first_crt*256ul+first_dsp
    flink[2] = rtr_map.test_address-39
    flink[3] = 40*65536ul+floor(rtr_map.switch_scslopevector/524288.)*32768ul+(rtr_map.switch_scslopevector-524288)/2

    err = write_same_dsp(0, switch_map.fastlinkhdr, [flink,0,0,0,0], /switch )
    err = write_same_dsp(0, '18035a'xl, not ('04000000'xul), /switch )
    wait, 0.1
    err = write_same_dsp(0, '180359'xl, '04000000'xul , /switch)

    err = write_same_dsp(0, rtr_map.switch_scslopevector, lonarr(40),/switch)

end
