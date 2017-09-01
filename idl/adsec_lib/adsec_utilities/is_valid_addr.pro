;$Id: is_valid_addr.pro,v 1.1 2004/07/13 16:01:12 riccardi Exp $
;+
; HISTORY
;   13 July 2004 written by A. Riccardi
;-
function is_valid_addr, addr, datasheet, set_pm

case set_pm of

    datasheet.data_mem_id: begin
        mem_block = addr/datasheet.data_block_offset
        if (total(mem_block lt 0L) ne 0) or (total(mem_block gt 2L) ne 0) then return, 0L
        loc_in_block = addr - mem_block*datasheet.data_block_offset
        if total(loc_in_block ge datasheet.data_mem_size) ne 0 then return, 0L
    end

    datasheet.prog_mem_id: if addr lt 0L or addr ge datasheet.prog_mem_size then return, 0B
    datasheet.sdram_mem_id: if addr lt 0L or addr ge datasheet.sdram_mem_size then return, 0B
    datasheet.sram_mem_id: if addr lt 0L or addr ge datasheet.sram_mem_size then return, 0B
    datasheet.flash_mem_id: if addr lt 0L or addr ge datasheet.flash_mem_size then return, 0B
    datasheet.fpgabuf_mem_id: if addr lt 0L or addr ge datasheet.fpgabuf_mem_size then return, 0B
    else: message, "Unexpected set_pm value."
endcase

return, 1B
end
