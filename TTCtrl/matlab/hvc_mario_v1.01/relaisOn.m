function relaisOn

sys_data=evalin('base','sys_data');
out=double(aoRead('hvc_DAC_value',sys_data.hvc_board))-repmat(hex2dec('80008000'),1,4);
out=out(1:3);
if (any(out))
    error('DAC values be zero');
end;

enableMask='1010101010101010';
mgp_op_reset_devices(sys_data.hvc_board,sys_data.hvc_board,[0,bin2dec(enableMask)]);
