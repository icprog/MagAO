;$Id: make_read_array.pro,v 1.1 2004/07/13 16:01:12 riccardi Exp $
;+
; HISTORY
;   13 July 2004, written by A. Riccardi
;-
function make_read_array, d1, d2, d3, d4, d5, d6, d7, d8, ULONG=set_ulong, LONG=set_long, TYPE=data_type

if n_elements(data_type) eq 0 then begin
    if keyword_set(set_long)+keyword_set(set_ulong) gt 1 then begin
        message, "ULONG and LONG keyword are exclusive"
    endif
    case 1B of
        keyword_set(set_ulong): the_data_type=13
        keyword_set(set_long): the_data_type=3
        else: the_data_type=4
    endcase
endif else begin
    case data_type of
       13: ;ulong
       3:  ;long
       4:  ;float
       else: begin
         message, "Requested data type not valid (TYPE="+strtrim(data_type,2)+")."
       end
    endcase
    the_data_type=data_type
endelse

case n_params() of
    0: return,(make_array(1,TYPE=the_data_type))[0]
    1: return, make_array(d1, TYPE=the_data_type)
    2: return, make_array(d1, d2, TYPE=the_data_type)
    3: return, make_array(d1, d2, d3, TYPE=the_data_type)
    4: return, make_array(d1, d2, d3, d4, TYPE=the_data_type)
    5: return, make_array(d1, d2, d3, d4, d5, TYPE=the_data_type)
    6: return, make_array(d1, d2, d3, d4, d5, d6, TYPE=the_data_type)
    7: return, make_array(d1, d2, d3, d4, d5, d6, d7, TYPE=the_data_type)
    8: return, make_array(d1, d2, d3, d4, d5, d6, d7, d8, TYPE=the_data_type)
endcase

end
