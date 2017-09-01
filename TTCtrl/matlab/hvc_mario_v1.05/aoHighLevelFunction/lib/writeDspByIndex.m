function x=writeDspByIndex(dspArray,connectionNr,len,startAddress,data,dataType)
% this function can be used by supplying the dsp as an array


% Author(s): D. Pescoller
%
% Copyright 2004-2008 Microgate s.r.l.
% $Revision 0.1 $ $Date: 22/12/2004

%test if dspArray has unique values
if length(unique(dspArray))~=length(dspArray)
   error('dspArray must have unique values')
end;

if length(dspArray)*len~=length(data) && len~=length(data)
   error('dspArrayLen*len must be equal to dataLen')
end;

dspArray=reshape(dspArray,length(dspArray),1);

if len==length(data)
   dataMat=[dspArray,repmat(data(:)',length(dspArray),1)];
else
   dataMat=[dspArray,reshape(data,len,length(dspArray))'];
end

% we write the same data to all dps's
if size(unique(dataMat(:,2:end),'rows'),1)==1
    dspList=dataMat(:,1);
    writeDspList=getContinuousIndex(dspList);
    if isempty(writeDspList)
        firstDsp=dspList(1);
        lastDsp=dspList(end);
%         [firstDsp,lastDsp];
        mgp_op_wrsame_dsp(firstDsp,lastDsp,len,startAddress,dataMat(1,2:end),connectionNr,dataType);
    else
        firstDsp=dspList(1);
        for i=reshape(writeDspList,1,length(writeDspList))
            lastDsp=dspList(i);    
%             [firstDsp,lastDsp];
            mgp_op_wrsame_dsp(firstDsp,lastDsp,len,startAddress,dataMat(1,2:end),connectionNr,dataType);
            firstDsp=dspList(i+1);
        end;
        lastDsp=dspList(end);
%         [firstDsp,lastDsp];
        mgp_op_wrsame_dsp(firstDsp,lastDsp,len,startAddress,dataMat(1,2:end),connectionNr,dataType);
    end;
    return;
end

%sort dsp list
[nr,nc]=size(dataMat);
for i=1:nr
    mgp_op_wrsame_dsp(dataMat(i,1),dataMat(i,1),len,startAddress,dataMat(i,2:end),connectionNr,dataType);
end;
