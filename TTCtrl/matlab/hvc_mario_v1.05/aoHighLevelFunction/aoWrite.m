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
% 12/06/2009 modified to select different DSP_CONF_MAT column - by mario
% 24/06/2009 modified to use in the M4 system - by mario
% 28/09/2009 modified the definition of M4 system - by mario
% 12/03/2012 extension to use HKL/DSM_aoREAD (mario)
% $Revision 0.3 $ $Date: 18/02/2014 - extended for LATT; - by D. Pescoller
%     device is now a char DSP default
%     dataType can also be the matlab types, i.e. 'single', 'uint32' and so
%     on not only the number id

AO_VARIABLE_DEFINE();
try
   DSP_NUM_CHANNELS=evalin('base','DSP_NUM_CHANNELS');
catch
   DSP_NUM_CHANNELS=4;
end;

%if we pass only the name we search in the database
if ~isstruct(var)
   var=aoGetVar(var);
   if isempty(var)
      error('could not find var name')
   end
end

%process varargin

indexArray=varCatArray(var.category).defaultIndex;
connectionNrArray=1;
dspConfMatCol=evalin('base','DSP_MY_INDEX');
dspConfMatColGiven = false;

if nargin>=3
   indexArray=varargin{1};
end
if nargin>=4
   connectionNrArray=varargin{2};
end
if nargin>=5
   dspConfMatCol=varargin{3};
   dspConfMatColGiven = true;
end

if isnumeric(var.type)
    dataType=varTypeArray(var.type).name;
else
    dataType=var.type;
end

len=var.nrItem;
startAddress=uint32(getNumeric(var.memPointer));
if isfield(var,'device')
   device=var.device;
else
   device='DSP';
end
if isfield(var,'stepPtr') && ~isempty(var.stepPtr)
   step_ptr=var.stepPtr;
else
   step_ptr=len/DSP_NUM_CHANNELS;
end

% ---------------------------------------------------------------------------------------------------
% begin - extension to use HKL/DSM_aoREAD 
% chris+diti 2011/08/01
% 2014/06/23 modified by mario from transport 'tcp' to 'hkl' according to the new getSocket function
% ---------------------------------------------------------------------------------------------------
if ~isempty(connectionNrArray) && ~any(isnan(connectionNrArray))
   [aa,transport]=getSocket(connectionNrArray); % occhio alla compatibilita' con i vecchi codici matlab!!!
else
   transport=[];
end

if strcmpi(transport, 'hkl')
   % actNr -> comma separated list of ints
   bb=diff(indexArray);bb(end+1)=0;cc=[1 diff(bb)];[idx]=find(cc~=0);
   actNr=[];
   for i=1:length(idx)-1
      if     idx(i+1)==idx(i)+1
         actNr=[actNr int2str(indexArray(idx(i))) ','];
      else
         actNr=[actNr int2str(indexArray(idx(i))) '-'];
      end
   end
   actNr=[actNr int2str(indexArray(end))];
   % actNr -> empty string for board variables and 'DSP' or 'MIRROR' for channel variables 
   switch var.category
      case CAT_BOARD
         if dspConfMatColGiven
         	error('aoWrite() / HKL mode: to write board variables, parameter dspConfMatCol must not be given');
         end
         actNrType = '';
      case CAT_CHANNEL
         if ~dspConfMatColGiven || ~ischar(dspConfMatCol)
            error('aoWrite() / HKL mode: to write channel variables, parameter dspConfMatCol must be ''DSP'' or '' MIRROR'')');
         end
         if strcmpi(dspConfMatCol, 'DSP')
            actNrType = 'DSP';
         elseif strcmpi(dspConfMatCol, 'MIRROR')
            actNrType = 'MIRROR';
         else
            error('aoWrite() / HKL mode: dspConfMatCol must be ''DSP'' or '' MIRROR''');
         end
      otherwise
         error('aoWrite() / HKL mode: only variables of category CAT_BOARD or CAT_CHANNEL are currently supported');
   end
   % ok, execute HKL command
   switch var.type
      case TYPE_SINGLE
         data=typecast(single(data),'uint8');
      case TYPE_INT32
         data=typecast(int32(data),'uint8');
      case TYPE_UINT32
         data=typecast(uint32(data),'uint8');
      case TYPE_STRUCT
         error('Not supported data type in write command');
      otherwise
         error('Invalid variable type');
   end
   [resData,resStatus]=m2dsmHKLCommand(['DSM_AOWRITE ', var.name, ' ', actNr, ' ', actNrType],connectionNrArray,data);
   if ~strcmpi(resStatus, 'OK DSM_AOWRITE 200')
      error(['aoWrite() / HKL mode: LCU returned error: ', resStatus]);
   end; 
   return
end
% --------------------------------------------------------------
% end - extension to use HKL/DSM_aoREAD
% --------------------------------------------------------------
switch var.category
	case CAT_CHANNEL
      writeChannelVar(indexArray,startAddress,data,dataType,dspConfMatCol,len,device,step_ptr);
   case CAT_SIG_GEN
   	x=mgp_op_rd_siggen(indexArray,indexArray,len,startAddress,data,dataType);
   otherwise
      for ii = 1:length(connectionNrArray)
         if     strcmpi(device,'DSP')
            writeDspByIndex(indexArray,connectionNrArray(ii),len,startAddress,data,dataType);
         elseif strcmpi(device,'SRAM') % this case is used to address the SRAM memory of BCU or DSP boards via UDP/IP socket
            writeSramByIndex(indexArray,connectionNrArray(ii),len,startAddress,data,dataType);
         elseif strcmpi(device,'M4')
            writeFinByIndex(indexArray,connectionNrArray(ii),len,startAddress,data,dataType);
         elseif strcmpi(device,'SAB')
            writeSabByIndex(indexArray,len,startAddress,data,dataType,dspConfMatCol);
         elseif strcmpi(device,'FPGA_CNTLOOP') || strcmpi(device,'FPGA_SIGGEN ') || strcmpi(device,'FPGA_DIAGBUF') || strcmpi(device,'FPGA_HIGHSPD') || ...
                strcmpi(device,'SRAM        ') || strcmpi(device,'FPGA_PTPINT ') || strcmpi(device,'FPGA_MCINT  ') || strcmpi(device,'FPGA_COMPINT')
            switch var.type
               case TYPE_SINGLE
                  data=typecast(single(data),'uint32');
               case TYPE_INT32
                  data=typecast(int32(data),'uint32');
               case TYPE_UINT32
                  data=typecast(uint32(data),'uint32');
               case TYPE_STRUCT
                  error('Not supported data type in write command');
               otherwise
                  error('Invalid variable type');
            end
            if     size(data,1) == length(connectionNrArray) && size(data,2) == len
               [tmp,aa]=mgp_op_tcp_command('MGP_MEM_WRITE',device,len,startAddress,data(ii,:),connectionNrArray(ii));
            elseif size(data,1) == len || size(data,2) == len
               [tmp,aa]=mgp_op_tcp_command('MGP_MEM_WRITE',device,len,startAddress,data,connectionNrArray(ii));
            else
               error('Invalid data size');
            end
            if ~strcmpi(aa,'MGP_CMD_SUCCESS')
               error('MGP/TCP command failed, the reply is %s',aa);
            end
         else
            error('Invalid device selected');
         end
      end
end
