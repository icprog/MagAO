; $Id: curr2color.pro,v 1.6 2005/08/29 14:10:17 labot Exp $
;+
; HYSTORY:
;   written by Armando Riccardi (AR) on 1912
;   16 Jul 2004
;     fixed floating illegal operand error (overflow long).
;-
function curr2color, current, NOSATURATE=nosaturate, CUTS2USE=curr_cuts

    common graph_block, gr

    if n_elements(curr_cuts) eq 0 then curr_cuts=gr.curr_cuts
    dcuts = curr_cuts[1]-curr_cuts[0]
    if dcuts ne 0.0 then begin
        color = long(((double(current-curr_cuts[0])*(gr.n_act_color-1L)/dcuts) > '80000000'XL) < '7FFFFFFF'XL)
    endif else begin
        color = long(0.0*current)
    endelse

    if keyword_set(nosaturate) then begin
        return, color
    endif else begin
        return, (color < gr.n_act_color-1L) > 0L
    endelse
end
