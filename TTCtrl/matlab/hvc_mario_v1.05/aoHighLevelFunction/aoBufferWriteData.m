function aoBufferWriteData(bufferStruct,data,varargin)
% aoBufferWriteData(bufferStruct,data,[connectionNr]) writes buffer data
% connectonNr default = 1


% Author(s): D. Pescoller
%
% Copyright 2004-2008 Microgate s.r.l.
% $Revision 0.1 $ $Date: 2004/08/09
% $Revision 0.2 $ $Date: 09/03/2007 - multiple Crate connection D. Pescoller


AO_VARIABLE_DEFINE();
aoVariables=evalin('base','aoVariables');

connectionNr=1;
if nargin>=3
    connectionNr=varargin{1};
end

firstDsp=bufferStruct.firstDsp;
lastDsp=bufferStruct.lastDsp;
len=getNumeric(bufferStruct.len)*getNumeric(bufferStruct.nrItem);
pointer=bufferStruct.sdramPointerValue;
type=varTypeArray(bufferStruct.dataType).name;
if prod(size(data)) ~= len*(floor((lastDsp-firstDsp)/2)+1),
    error('Wrong number of elements in parameter data');
end;

data=reshape(data,len,prod(size(data))/len);

[socket,trasport]=getSocket(connectionNr);

if strcmp(trasport,'tcp')
    switch type
        case 'single'
            hkl_var='_sdrfb_diagbuff';
            hkl_type=1;
            hkl_str=[hkl_var, ' ', num2str(len), ' ', num2str(pointer), ' board float'];
        case 'int32'
            hkl_var='_sdrib_diagbuff';
            hkl_type=2;
            hkl_str=[hkl_var, ' ', num2str(len), ' ', num2str(pointer), ' board int32'];
        case 'uint32'
            hkl_var='_sdrub_diagbuff';
            hkl_type=3;
            hkl_str=[hkl_var, ' ', num2str(len), ' ', num2str(pointer), ' board uint32'];
    end
    [resData,resStatus]=m2dsmHKLCommand('DSM_ADDVAR',connectionNr,hkl_str);
    if ~strcmpi(resStatus, 'OK DSM_ADDVAR')
        error(['aoWrite() / HKL mode: LCU returned error: ', resStatus]);
    end
    new_var.name=hkl_var;
    new_var.nrItem=len;
    new_var.memPointer=pointer;
    new_var.category=1;
    new_var.description='';
    new_var.type=hkl_type;
    new_var.operation=[];
    try
        [varStruct,varIndex]=aoGetVar(hkl_var);
        if aoVariables(varIndex).nrItem~=new_var.nrItem || ...
                aoVariables(varIndex).memPointer~=new_var.memPointer || ...
                aoVariables(varIndex).type~=new_var.type
            aoVariables(varIndex)=new_var;
            assignin('base','aoVariables',aoVariables);
            clear aoGetVar;
        end
    catch
        aoVariables(end+1)=new_var;
        assignin('base','aoVariables',aoVariables);
        clear aoGetVar;
    end
    
    % scrittura a piu' cicli
    firstDsp=(floor(firstDsp/2))*2;
    lastDsp=(floor(lastDsp/2))*2+1;
    cnt=1;
    for ii=firstDsp:2:lastDsp
        aoWrite(hkl_var,data(:,cnt),ii:ii,connectionNr);
        cnt=cnt+1;
    end
    
else
    i2=1;
    for ii=firstDsp:2:lastDsp,
        mgp_op_wrsame_sdram(ii,ii,len,pointer,data(:,i2),connectionNr,type);
        i2=i2+1;
    end;
end

