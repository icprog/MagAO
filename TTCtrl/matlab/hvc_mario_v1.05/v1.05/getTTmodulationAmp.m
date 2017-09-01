
function ampPtV=getTTmodulationAmp( sys_data)

ampPtV = aoRead('_hvcfb_CurrHistAmplitude', sys_data.hvc_board, sys_data.SCConnectionID);

return;

