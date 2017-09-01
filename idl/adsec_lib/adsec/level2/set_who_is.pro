; $Id: set_who_is.pro,v 1.6 2007/02/07 16:57:57 marco Exp $
;+
; SET_WHO_IS
;
; err = set_who_is()
;
; HISTORY
; 27 May 2004, A. Riccardi
;   long type data written for LBT compatibility
;   only one who_is_ch variable for all the DSP channels.
;   It suppose that who_is_ch1=board_number+1, ecc.
;   It differs from MMT case in which all the chx had to
;   be specified.
;   07 Feb 2007 MX
;       LUT of variable deleted and name refurbished.
;-

function set_who_is

@adsec_common

dsp_list = indgen(adsec.n_dsp)
ch0_list = ulong(reform(dsp_list,1,adsec.n_dsp)*adsec.n_act_per_dsp)
err = write_seq_dsp(dsp_list, dsp_map.board_number, ch0_list)

return, err

end
