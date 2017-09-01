;@File: format_side.pro
;
; Formats a side string from L or R to a proper name. Defaults to "left" if an undefined value is passed.
;
;@

function format_side, side

s = ""

if not keyword_set(side) then s = "left" else begin
    if side eq "L" then s = "left"
    if side eq "R" then s = "right"
endelse

if s eq "" then s = side

return, s
end

