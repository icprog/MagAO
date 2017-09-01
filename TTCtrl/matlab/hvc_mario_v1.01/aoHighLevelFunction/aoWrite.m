function aoWrite(var,data,varargin)
% aoWrite((varname|varStruct),data,[indexArray],[connectionNrArray],[dspConfMatCol])
%
% This function is a more userfriendly routine to write data to ao
% this function uses the mgp* AO communication basic routines
% aoVariables "database" instructs this function how to get data
% see aoGetVar for possible usage of varname
%
% indexArray: defaults to the defaultIndex of variable category
% for *NORMAL TYPE* variables this is simply the DSP number, 
% be aware that this is an array not a firstDSP and lastDSP, thus if you
% want to read from 0 to 5 you must supply [0:5], and not [0,5] this would
% read just 0 and 5 don't be confused by the lower level "mgp library".
% for variables of *TYPE CHANNEL* this is the index 
% of DSP_CONF_MAT see  DSP_CONF_MAT for instructions
%
% connectionNrArray: defaults to 1, is the list of connections/crates. 
% 
% dspConfCol: default is column DSP_MY_INDEX (typically dsp actuator numbering) 
%             7 for mirror actuator ordering (see DSP_CONF_MAT for different column options)

% Author(s): D. Pescoller
%
% Copyright 2004-2008 Microgate s.r.l.
% $Revision 0.1 $ $Date: 2004/09/25
% $Revision 0.2 $ $Date: 09/03/2007 - multiple Crate connection D. Pescoller
% 12/06/09 modified to select different DSP_CONF_MAT column - by mario
% 24/06/09 modified to use in the M4 system - by mario
% 28/09/09 modified the definition of M4 system - by mario

AO_VARIABLE_DEFINE();

%if we pass only the name we search in the database
if(~isstruct(var))
      var=aoGetVar(var);
      if(isempty(var))
          error('could not find var name')
      end;
end;

%process varargin

indexArray=varCatArray(var.category).defaultIndex;
connectionNrArray=1;
dspConfMatCol=evalin('base','DSP_MY_INDEX');
if nargin>=3
   indexArray=varargin{1};
end;
if nargin>=4
   connectionNrArray=varargin{2};
end;
if nargin>=5
   dspConfMatCol=varargin{3};
end;

dataType=varTypeArray(var.type).name;
len=var.nrItem;
startAddress=getNumeric(var.memPointer);
try
   device=var.device;
   if isempty(device)
      device=0;
   end
catch
   device=0;
end

switch var.category
	case CAT_CHANNEL
      writeChannelVar(indexArray,startAddress,data,dataType,dspConfMatCol,len,device);
   case CAT_SIG_GEN
   	x=mgp_op_rd_siggen(indexArray,indexArray,len,startAddress,data,dataType);
   otherwise
      for connectionNr=connectionNrArray(:)'
         if     device==0
            writeDspByIndex(indexArray,connectionNr,len,startAddress,data,dataType);
         elseif device==1
            writeFinByIndex(indexArray,connectionNr,len,startAddress,data,dataType);
         else
            error('Invalid device selected');
         end
      end
end

