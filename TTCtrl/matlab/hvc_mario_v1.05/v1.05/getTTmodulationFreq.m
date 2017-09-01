
function freq=getTTmodulationFreq( sys_data)

step = aoRead('_hvcfb_TT0_CurrHistStep', sys_data.hvc_board, sys_data.SCConnectionID);

freq = step*sys_data.minModFreq;

return;

