% AO low level comunication library
%
% Files
%   AOConnect             - AOConnect('hostname',[port])
%   AODisConnect          - AODisConnect disconnects all AO's connection and clear the AO struct 
%   mgp_op_rd_diagbuf     - mgp_op_rd_diagbuf(firstDsp,lastDsp,len,startAddress,[connectionNr],[dataType])
%   mgp_op_rd_sdram       - MGP_OP_RDSEQ_DSP(firstDsp,lastDsp,len,startAddress,[connectionNr],[dataType])
%   mgp_op_rdseq_dsp      - (firstDsp,lastDsp,len,startAddress,[connectionNr],[dataType])
%   mgp_op_wrsame_diagbuf - mgp_op_wrsame_diagbuf(firstDsp,lastDsp,len,startAddress,data,[connectionNr],[dataType])
%   mgp_op_wrsame_dsp     - mgp_op_wrseq_dsp(firstDsp,lastDsp,len,startAddress,data,[connectionNr],[dataType],[FLAGS])
%   mgp_op_wrsame_sdram   - mgp_op_wrsame_sdram(firstDsp,lastDsp,len,startAddress,data,[connectionNr],[dataType])
%   mgp_op_clear_flash    - mgp_op_clear_flash(firstDsp,lastDsp,len,startAddress,data,[connectionNr],[dataType])
%   mgp_op_hl_rdseq_dsp   - data=mgp_op_hl_rdseq_dsp(firstDsp,lastDsp,len,startAddress,[connectionNr],[dataType],[FLAGS])
%   mgp_op_hl_wrsame_dsp  - mgp_op_hl_wrsame_dsp(firstDsp,lastDsp,len,startAddress,data,[connectionNr],[dataType],[FLAGS])
%   mgp_op_hl_wrseq_dsp   - mgp_op_hl_wrseq_dsp(firstDsp,lastDsp,len,startAddress,data,[connectionNr],[dataType],[FLAGS])
%   mgp_op_rd_sram        - mgp_op_rd_sram(firstDsp,lastDsp,len,startAddress,[connectionNr],[dataType])
%   mgp_op_rdseq_flash    - mgp_op_rdseq_flash(firstDsp,lastDsp,len,startAddress,[connectionNr],[dataType])
%   mgp_op_reset_devices  - mgp_op_reset_devices(first,last,data,[connectionNr],[dataType])
%   mgp_op_wr_screle      - reply=mgp_op_wr_screle(first,last,len,startAddress,data,[connectionNr],[dataType])
%   mgp_op_wr_shmram      - mgp_op_wrshmram(first,last,len,startAddress,data,[connectionNr],[dataType])
%   mgp_op_wr_siggen      - mgp_op_wrsiggen(first,last,len,startAddress,data,[connectionNr],[dataType])
%   mgp_op_write_flash    - mgp_op_write_flash(firstDsp,lastDsp,len,startAddress,data,[connectionNr],[dataType])
%   mgp_op_wrsame_sram    - mgp_op_wrsame_sram(firstDsp,lastDsp,len,startAddress,data,[connectionNr],[dataType])
