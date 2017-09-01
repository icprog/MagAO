; $Id: get_commands.pro,v 1.6 2007/11/05 10:16:12 marco Exp $
;
;+
;   NAME:
;    GET_COMMANDS
;
;   PURPOSE:
;    Retrieve the position command from the adaptive secondary mirror electronics, 
;    either final position commands as position or force offload commands.
;
;   USAGE:
;    err = get_commands(act_list, commands, [/OFFLOADCMD, /CURRCMD])
;   
;   INPUT:
;    act_list: list of actuators to get data.
;
;   OUTPUT:
;    err:       error code.
;    commands:  commands given.
;
;   KEYWORDS:
;    OFFLOADMD: get position offload command instead of absolute position command
;    CURRCMD:   get force offload command instead of absolute position command 
;
;   HISTORY
;    27 May 2004, A. Riccardi
;       long data type (1L) used in read_seq_ch for LBT compatibility
;    07 Feb 2007 Marco Xompero (MX)
;       LUT of variable deleted and name refurbished.
;    04 Nov 2007 MX
;       Added keywords.
;-
function get_commands, act_list, commands, OFFLOADCMD=offloadcmd, CURRCMD=forcecmd

@adsec_common

    if keyword_set(offloadcmd) then begin

        err = read_seq_dsp(0,switch_map.OffloadSelector, 1L, block_of, /SWITCH)
        if err ne adsec_error.ok then return, err
        if block_of then err = read_seq_ch(act_list, dsp_map.cmol_b, 1L, commands) $
        else err = read_seq_ch(act_list, dsp_map.cmol_a, 1L, commands) 

    endif else begin

        if keyword_set(forcecmd) then begin

            err = read_seq_dsp(0,switch_map.OffloadSelector, 1l, block_of, /SWITCH)
            if err ne adsec_error.ok then return, err
            if block_of then err = read_seq_ch(act_list, dsp_map.fmol_b, 1L, commands) $
            else err = read_seq_ch(act_list, dsp_map.fmol_a, 1L, commands) 

        endif else begin

            err = read_seq_ch(act_list, dsp_map.pos_command, 1L, commands)
            if err ne adsec_error.ok then return, err

        endelse

    endelse

    return, err

end
