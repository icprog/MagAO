function data_out=get_sys_status(act_list,act_map,varargin)
%
% NAME:
%  GET_SYS_STATUS(act_list,act_map,[dmSystem])
%
% PURPOSE:
%	The routine retrives the main channel status for the ff matrix acquisition.
%
% USAGE:
%	data_out = get_sys_status(act_list,act_map)
%
% INPUT:
%  act_list:   list of channels to read
%  act_map:    column of the DSP_CONF_MAT to use
%  [dmSystem]: select the deformable system (0=M4-DP, 1=D45) - default is 0 (M4-DP)
%
% OUTPUT:
%  data_out: structure with the acquired information
%
% KEYWORD:
%
% NOTE:
%
% HISTORY
%  01/02/10 - Modified by Mario to expand for D45 system
%

sys_data=evalin('base','sys_data');
DSP_CONF_MAT=evalin('base','DSP_CONF_MAT');

dmSystem=0;
if nargin>=3
   dmSystem=varargin{1};
end

if dmSystem==0
   step_res = uint32(aoRead('storage_area',act_list,1,act_map));
   num_samples = aoRead('num_samples',0);
   data_out.command=double(int32(step_res(:,3)))/2^sys_data.scale_input;
   data_out.mean_position=aoRead('distance_acc',act_list,1,act_map)/num_samples/2^sys_data.scale_input;
   data_out.mean_tot_curr=(aoRead('current_acc',act_list,1,act_map)/num_samples-sys_data.DAC_offset)/sys_data.A2BIT_gain;
   data_out.bias_current=double(typecast(uint16(aoRead('bias_force',act_list,1,act_map)),'int16'))/sys_data.A2BIT_gain;
   data_out.ext_current=double(typecast(step_res(:,6),'int32'))/sys_data.A2BIT_gain;
   data_out.DAC_value=step_res(:,7);
   data_out.pos_force=double(typecast(bitset(bitand(hex2dec('7FFFFFFF'),step_res(:,4)),32,bitget(step_res(:,4),31)),'int32'))/sys_data.A2BIT_gain;
   data_out.err_force=double(typecast(bitset(bitand(hex2dec('7FFFFFFF'),step_res(:,5)),32,bitget(step_res(:,5),31)),'int32'))/sys_data.A2BIT_gain;
elseif dmSystem==1
%    if sum(aoRead('_dm2_runningAcc',0:sys_data.nDsp-1))
%       error('The accumulators are still running');
%    end
% versione ottimizzata
   a1=mgp_op_rdseq_dsp(0,sys_data.nDsp-1,sys_data.nActBoard/2,aoGetAddress('_dm2_preshapedCmd'),'single');
   a2=mgp_op_rdseq_dsp(0,sys_data.nDsp-1,sys_data.nActBoard/2,aoGetAddress('_dm2_distAverage'),'single');
   a3=mgp_op_rdseq_dsp(0,sys_data.nDsp-1,sys_data.nActBoard/2,aoGetAddress('_dm2_currAverage'),'single');
   a4=mgp_op_rdseq_dsp(0,sys_data.nDsp-1,sys_data.nActBoard/2,aoGetAddress('_dm2_biasCurrent'),'single');
   a5=mgp_op_rdseq_dsp(0,sys_data.nDsp-1,sys_data.nActBoard/2,aoGetAddress('_dm2_preshapedCurr'),'single');
   a6=mgp_op_rdseq_dsp(0,sys_data.nDsp-1,sys_data.nActBoard/2,aoGetAddress('_dm2_floatDACValue'),'single');
   a7=mgp_op_rdseq_dsp(0,sys_data.nDsp-1,sys_data.nActBoard/2,aoGetAddress('_dm2_posCurrent'),'single');
   a8=mgp_op_rdseq_dsp(0,sys_data.nDsp-1,sys_data.nActBoard/2,aoGetAddress('_dm2_errCurrent'),'single');
   aa=sortrows([DSP_CONF_MAT(:,act_map) a1 a2 a3 a4 a5 a6 a7 a8],1);
   data_out.command=aa(act_list+1,2);
   data_out.mean_position=aa(act_list+1,3);
   data_out.mean_tot_curr=aa(act_list+1,4);
   data_out.bias_current=aa(act_list+1,5);
   data_out.ext_current=aa(act_list+1,6);
   data_out.DAC_value=aa(act_list+1,7);
   data_out.pos_force=aa(act_list+1,8);
   data_out.err_force=aa(act_list+1,9);

% % versione lenta
%    data_out.command=aoRead('_dm2_preshapedCmd',act_list,1,act_map);
%    data_out.mean_position=aoRead('_dm2_distAverage',act_list,1,act_map);
%    data_out.mean_tot_curr=aoRead('_dm2_currAverage',act_list,1,act_map);
%    data_out.bias_current=aoRead('_dm2_biasCurrent',act_list,1,act_map);
%    data_out.ext_current=aoRead('_dm2_preshapedCurr',act_list,1,act_map);
%    data_out.DAC_value=aoRead('_dm2_floatDACValue',act_list,1,act_map);
%    data_out.pos_force=aoRead('_dm2_posCurrent',act_list,1,act_map);
%    data_out.err_force=aoRead('_dm2_errCurrent',act_list,1,act_map);
else
   error('Invalid dmSystem option');
end
