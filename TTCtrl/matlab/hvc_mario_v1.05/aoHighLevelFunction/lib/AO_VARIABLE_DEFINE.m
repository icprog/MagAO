function AO_VARIABLE_DEFINE()
% declares various variable properties
% this is a C-INCLUDE like file

% Author(s): D. Pescoller
%
% Copyright 2004-2008 Microgate s.r.l.
% $Revision 0.1 $ $Date: 2004/08/09

%variable types
assignin('caller','TYPE_SINGLE',1);
assignin('caller','TYPE_INT32',2);
assignin('caller','TYPE_UINT32',3);
assignin('caller','TYPE_STRUCT',5);

varTypeArray(1).name='single';
varTypeArray(2).name='int32';
varTypeArray(3).name='uint32';
varTypeArray(4).name='uint16';
varTypeArray(5).name='uint32';
assignin('caller','varTypeArray',varTypeArray);
%variable category
assignin('caller','CAT_BOARD',1);
varCatArray(1).name='board';
varCatArray(1).prefix='';
varCatArray(1).defaultIndex=0;

assignin('caller','CAT_CHANNEL',2);
varCatArray(2).name='channel';
varCatArray(2).prefix='';
varCatArray(2).defaultIndex=0;

assignin('caller','CAT_SLOPE_COMPUTER',3);
varCatArray(3).name='slope';
varCatArray(3).prefix='sc_';
varCatArray(3).defaultIndex=0;

assignin('caller','CAT_AO_COMPUTER',4);
varCatArray(4).name='ao';
varCatArray(4).prefix='bc_';
varCatArray(4).defaultIndex=255;

assignin('caller','CAT_SIG_GEN',5);
varCatArray(5).name='sig';
varCatArray(5).prefix='sg_';
varCatArray(5).defaultIndex=252;

assignin('caller','CAT_SRAM_FIXED',6);
varCatArray(6).name='sram fixed';
varCatArray(6).prefix='srf_';
varCatArray(6).defaultIndex=0;

assignin('caller','varCatArray',varCatArray);

%Default variable file name
assignin('caller','AO_VAR_DATABASE','aoVariables.mat');
%assignin('caller','AO_VAR_DATABASE','aoVariablesKeck.mat');
