; $Id: write_des_pos.pro,v 1.6 2007/02/07 16:57:58 marco Exp $
;+
; HISTORY:
;   02 Nov 2004, MX
;   Adsec.all changed in sc.all.
;   07 Feb 2007 MX
;       LUT of variable deleted and name refurbished.
;       Polling on nendof_ff removed.
;-
function write_des_pos, act_list, comm_vector

@adsec_common

err = write_seq_ch(act_list, dsp_map.pos_command, comm_vector)
;err = read_seq_ch(sc.all_actuators, dsp_map.pos_command, 1, cv)
;if act_list[0] eq sc.all_actuators then cv[*]=comm_vector else cv[act_list]=comm_vector
;err = write_seq_ch(sc.all_actuators, dsp_map.pos_command, cv)

return, err

end
