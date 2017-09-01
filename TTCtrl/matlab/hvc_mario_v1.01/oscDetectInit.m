% this script initializes the MGAOS autooscillation/followingerror
% detection algorithm
% just initialization is done the detection must be enabled by configuring
% the _wfp_XTTServoOscOnOff variable
% Example: aoWrite('_wfp_DTTServoOscOnOff',hex2dec('11'),HVC_DSP);                 

% Author(s): D. Pescoller
%
% Copyright 2006-2010 Microgate s.r.l.
% $Revision 0.1 $ $Date: 15/12/2006


WINDOW_BASE_POINTER     = hex2dec('89000');
WINDOW_MAX_POINTER      = hex2dec('8F000');
WINDOW_LENGTH_BITS      = 7;
TRIGGER_LEVEL           = 500;
TRIGGER_TIMEOUT         = 2000;
% FOLLOW_ERROR            = 3e-6;
% FOLLOW_ERROR_REDUCED    = 3e-7;
FOLLOW_ERROR            = 4.5e-3;
FOLLOW_ERROR_REDUCED    = 4.5e-3;
WINDOW_LENGTH           = 2^WINDOW_LENGTH_BITS;
stopDataPtr             = WINDOW_BASE_POINTER+4*WINDOW_LENGTH;
if(stopDataPtr>WINDOW_MAX_POINTER)
    error('stopDataPtr > WINDOW_MAX_POINTER');
end

% servo oscillator init pointers
aoWrite('hvc_TT1ServoOscOnOff',0,sys_data.hvc_board); 
aoWrite('hvc_TT1ServoOscStartDataPtr',WINDOW_BASE_POINTER,sys_data.hvc_board);
aoWrite('hvc_TT1ServoOscCurrDataPtr',WINDOW_BASE_POINTER,sys_data.hvc_board);
aoWrite('hvc_TT1ServoOscStopDataPtr',stopDataPtr,sys_data.hvc_board);
aoWrite('hvc_TT1ServoOscBitShift',[-2*WINDOW_LENGTH_BITS (32-2*WINDOW_LENGTH_BITS)*256+2*WINDOW_LENGTH_BITS -WINDOW_LENGTH_BITS (32-WINDOW_LENGTH_BITS)*256+WINDOW_LENGTH_BITS],sys_data.hvc_board);
aoWrite('hvc_TT1ServoOscSum02',zeros(4,1),sys_data.hvc_board);
aoWrite('hvc_TT1ServoOscSum13',zeros(4,1),sys_data.hvc_board);
aoWrite('hvc_TT1ServoOscSumSqr02',zeros(4,1),sys_data.hvc_board);
aoWrite('hvc_TT1ServoOscSumSqr13',zeros(4,1),sys_data.hvc_board);
aoWrite('hvc_TT1ServoOscTrigLevel',ones(4,1)*TRIGGER_LEVEL,sys_data.hvc_board);
aoWrite('hvc_TT1CLErrorTrigLevel',ones(4,1)*FOLLOW_ERROR,sys_data.hvc_board);
aoWrite('hvc_TT1ServoOscTrigTimeout',ones(4,1)*TRIGGER_TIMEOUT,sys_data.hvc_board);
aoWrite('hvc_TT1ServoOscTrigCounter',zeros(4,1),sys_data.hvc_board);
mgp_op_wrsame_dsp(sys_data.hvc_board,sys_data.hvc_board,WINDOW_LENGTH*4,WINDOW_BASE_POINTER,uint32(zeros(4*WINDOW_LENGTH,1)));

disp('oscillator detect init done!!!');
