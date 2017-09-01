; $Id: adsec_common.pro,v 1.23 2009/11/27 15:49:08 lbusoni Exp $
;+
; HISTORY
;   written by A. Riccardi (AR)
;   10 Mar 2004:
;       patch_block common added.
;   04 Jun 2004:
;       dsp_sdram_map_block, fpgabuf_map_block common added
;   02 Nov 2004: by M.Xompero(MX) & D.Zanotti(DZ)
;       adsec_shell_block common added
;   20 May 2005:
;       Added rtr and rtr_map structures. (MX)
;   23 Jan 2006:
;       Added adsec_state structure. (DZ)
;   March 2006:
;       Added  cbuf_clin, cbuf_clout, cbuf_template structure.(MX)
;   07 Feb 2007 MX
;       Common Blocks added to match switch and crate BCUs
;   20 Feb 2007 MX
;       Fastlink common blocks name changed
;   02 July 2007 AR
;       added sys_buffer and sys_buffer_idx in the system_block common block
;       (see update_status)
;   19 Sep 2007 MX
;       added adam_block and shm_block
;-
common graph_block, gr
common adsec_block, adsec
common adsec_shell_block, adsec_shell
common rtr_block, rtr, rtr_map, crate2switch_collect_conf, dsp2crate_collect_conf
common system_block, sys_status, sys_buffer, sys_buffer_idx
common wyko_block, wyko
common wfs_block, wfs
common dsp_const_block, dsp_const
common dsp_map_block, dsp_map, dsp_map_pm, sdram_map, sram_map, fpgabuf_map, fpgasig_map
common switch_map_block, switch_map, switch_sdram_map, switch_sram_map, switch_fpgabuf_map, switch_fpgasig_map
common bcu_map_block, bcu_map, bcu_sdram_map, bcu_sram_map, bcu_fpgabuf_map, bcu_fpgasig_map
common acc_map_block, acc_map, acc_sram_map
common dsp_datasheet_block, dsp_datasheet
common bcu_datasheet_block, bcu_datasheet
common siggen_datasheet_block, siggen_datasheet
common accel_datasheet_block, accel_datasheet
common switch_datasheet_block, switch_datasheet
common generic_block, sc, error_code, selected_acts, prog_vers, idl_error_status
common adsec_error_block, adsec_error
common adsec_path_block, adsec_path
common patch_block, patch
common env_status_block, dsp_diagnostics, bcu_diagnostics, dsp_nios, bcu_nios
common adsec_state_block,  adsec_state
common cloop_info_block, cbuf_clin, cbuf_clout, cbuf_template
common adam_block, adam_in, adam_out
common shm_block, shm_sys_status, shm_master
common telescope_block, telescope_side, is_side_master
common pm_offload_block, pm_offload
common process_list_block, process_list
