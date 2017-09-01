;$Id: get_mem_size.pro,v 1.5 2004/11/29 15:10:00 labot Exp $
;+
; HISTORY
;  13 July 2004, A. Riccardi
;    changed fpgabuff_mem_id in fpgabuf_mem_id
;    changed fpgabuff_mem_size in fpgabuf_mem_size
;
;  27 Ago 2004,
;    added fpgasig_mem_size entry in datasheet
;-
function get_mem_size, datasheet, op_code

case op_code of
    datasheet.data_mem_id: return, datasheet.data_mem_size
    datasheet.prog_mem_id: return, datasheet.prog_mem_size
    datasheet.sdram_mem_id: return, datasheet.sdram_mem_size
    datasheet.sram_mem_id: return, datasheet.sram_mem_size
    datasheet.flash_mem_id: return, datasheet.flash_mem_size
    datasheet.fpgabuf_mem_id: return, datasheet.fpgabuf_mem_size
    datasheet.fpgasig_mem_id: return, datasheet.fpgasig_mem_size
    else: message, "Not valid memory type"
endcase
end
