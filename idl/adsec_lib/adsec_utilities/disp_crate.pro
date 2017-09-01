;+
;
; DISP_CRATE, data_vector [,/NO_CLEAR_DUMMY_ACT]
;
; display the adsec.n_elements vector in data_vector using
; the crate-dsp geometry
;
;
; HISTORY
;   1 Jul 2007: written by A. Riccardi (AR)
;               scale of color bar not yet correctly working
;-

pro disp_crate, data, _EXTRA=extra, NO_CLEAR_DUMMY_ACT=no_clear_dummy, DISP_ALL=disp_all
	@adsec_common
   
	aa=data
	if ~keyword_set(DISP_ALL) then aa[adsec.act_wo_pos] = mean(aa[adsec.act_w_pos])
	if ~keyword_set(no_clear_dummy) then aa[adsec.dummy_act]=min(aa[adsec.true_act])
	image_show,reform(transpose(reform(aa,8,14,6),[0,2,1]),8*6,14), _EXTRA=extra $
	          , YTIT="board number inside the crate", XTIT="crate#" $
	          , XTICKINTERVAL=1, XTICKLEN=1.0, XMINOR=8, YMINOR=1 $
	          , YTICKINTERVAL=1, YTICKLEN=1.0, YTICKLAYOUT=2, XTICKLAYOUT=2, XAX=[0,adsec.n_crates]
end
