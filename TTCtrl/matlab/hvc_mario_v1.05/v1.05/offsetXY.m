
function offsetXY(sys_data, xoffset, yoffset)

aoWrite('_hvcfc_TT1_bias_command', [xoffset yoffset 0 0], 0:sys_data.hvc_nAct-1, sys_data.SCConnectionID);
pause(1);

return;


