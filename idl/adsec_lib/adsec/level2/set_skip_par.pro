;$Id: set_skip_par.pro,v 1.1 2007/02/07 17:00:25 marco Exp $$
;+
;   NAME:
;    SET_SKIP_PAR
;   PURPOSE:
;    Setting up the threshold conditions for the optical loop skip frame.
;   USAGE:
;    err = set_skip_par(par[,/CMD, /MODE, /FORCE, /MAX, /MIN, /DELTA, /CLEAR])
;   INPUT:
;    par:   float array to use as threshold. Not necessary if you use CLEAR keyword.
;   OUTPUT:
;    err:   error code.
;   KEYWORDS:
;    CMD:  command threshold setup. [m]
;    MODE: mode threshold setup.    [depending on RecMat]
;    FORCE:force threshold setup.   [N]
;    MAX:  upper bound setup.
;    MIN:  lower bound setup.
;    DELTA:max stroke setup.
;    CLEAR:clear threshold skip frame memory locations.
;   HISTORY:
;    26 Jan 2007
;    Created by Marco Xompero (MX)
;    marco@arcetri.astro.it
;-

Function set_skip_par, par, CMD=cmd, MODE=mode, FORCE=force, MAX=max, MIN=min, DELTA=delta, CLEAR=clear

    @adsec_common
    if n_elements(par) eq 1 then par = replicate(par, adsec.n_actuators)
    if n_elements(par) ne adsec.n_actuators and ~keyword_set(CLEAR) then begin
        message, 'The parameters inserted have wrong dimensions. Returning without setting up the thresholds.', CONT = (sc.debug eq 0)
        return, adsec_error.input_type
    endif
   
    id = fix(keyword_set(CMD)) or 2*keyword_set(MODE) or 4*keyword_set(FORCE) or $
         8*keyword_set(MAX) or 16*keyword_set(MIN) or 32*keyword_set(DELTA) or 64*keyword_set(CLEAR)

    case id of

        9: begin
            err = write_seq_ch(sc.all, dsp_map.max_cmd, par, /CHECK)
        end

        17: begin
            err = write_seq_ch(sc.all, dsp_map.min_cmd, par, /CHECK)
        end

        33:begin
            err = write_seq_ch(sc.all, dsp_map.dmax_cmd, par, /CHECK)
        end


        10:begin
            err = write_seq_ch(sc.all, dsp_map.max_mode, par, /CHECK)
        end

        34:begin
            err = write_seq_ch(sc.all, dsp_map.dmax_mode, par, /CHECK)
        end

        12:begin
            err = write_seq_ch(sc.all, dsp_map.max_curr, par, /CHECK)
        end

        20:begin
            err = write_seq_ch(sc.all, dsp_map.min_curr, par, /CHECK)
        end

        64:begin
            err = write_seq_ch(sc.all, dsp_map.max_mode, fltarr(7, adsec.n_actuators), /CHECK)
        end

        else: begin
            message, 'Wrong combination of keywords. No action done.', CONT = (sc.debug eq 0)
            return, adsec_error.input_type
        end
    endcase
    
    if err ne adsec_error.ok  then return, err

End
    
