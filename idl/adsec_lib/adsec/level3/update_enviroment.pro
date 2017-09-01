; $Id: update_enviroment.pro,v 1.2 2004/12/20 16:32:37 labot Exp $$
;+
; NAME:
; UPDATE_ENVIROMENT	
;
; PURPOSE:
;	This function updates the env_status structure for the monitoring of the enviroment.
;
; CATEGORY:
;	Data retrieve function.
;
; CALLING SEQUENCE:
;	err = update_enviroment()
;
; INPUTS:
;
; OUTPUT:
;
; OUTPUT KEYWORDS:
;
; PROCEDURE:
;	Test if there is an open connection and get the data .
;	Refill the structure "env_status"
;
; MODIFICATION HISTORY:
;	08 Apr 2004 	Initial creation (M.Xompero)
;
;-
Function update_enviroment

@adsec_common 

len_dsp_diag = 6
err = read_seq_board(sc.all, sram_map.dsp_diagnostics_area, len_dsp_diag, buf, set_pm=dsp_datasheet.sram_mem_id)
if err ne adsec_error.ok then return, err
template = replicate( gen_dsp_diagnostics_structure(),adsec.n_board_per_bcu)

buf = reform(buf, n_elements(buf))
fill_structure, buf, template
dsp_diagnostics = convert_env(template)

len_bcu_diag = 11
err = read_seq_board(sc.all, sram_map.bcu_diagnostics_area, len_bcu_diag, buf, set_pm=dsp_datasheet.sram_mem_id,  /BCU)

;si deve decidere cosa fare se fallisce....
if err ne adsec_error.ok then return, err

template = gen_bcu_diagnostics_structure()
fill_structure, buf, template
bcu_diagnostics = convert_env(template, /bcu)

return, adsec_error.ok
end
