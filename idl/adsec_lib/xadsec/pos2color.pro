; $Id: pos2color.pro,v 1.5 2004/07/21 14:11:46 riccardi Exp $
;+
; HYSTORY:
;   written by Armando Riccardi (AR) on 1912
;   16 Jul 2004
;     fixed floating illegal operand error (overflow long).
;-

function pos2color, position, NOSATURATE=nosaturate, CUTS2USE=pos_cuts

    common graph_block, gr

    if n_elements(pos_cuts) eq 0 then pos_cuts=gr.pos_cuts
    dcuts = pos_cuts[1]-pos_cuts[0]
    if dcuts ne 0.0 then begin
        color = long((double(position-pos_cuts[0])*(gr.n_act_color-1L)/dcuts) > '80000000'XL < '7FFFFFFF'XL)
    endif else begin
        color = long(0.0*position)
    endelse

    if keyword_set(nosaturate) then begin
        return, color
    endif else begin
        return, (color < gr.n_act_color-1L) > 0L
    endelse
end
