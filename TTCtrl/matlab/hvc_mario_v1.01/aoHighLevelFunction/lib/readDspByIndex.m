function x=readDspByIndex(dspArray,connectionNr,len,startAddress,dataType)
% this function can be used by supplying the dsp as an array
% unique values are returned

% Author(s): D. Pescoller
%
% Copyright 2004-2008 Microgate s.r.l.
% $Revision 0.1 $ $Date: 22/12/2004

dspList=unique(dspArray);
%read data from DSP

readData=[];
readDspList=getContinuousIndex(dspList);

% se len è magiore di 7 spezzeto le letture in letture per dsp
if len>7;
    for i=dspList(:)'
        readData=[readData;mgp_op_rdseq_dsp(i,i,len,startAddress,connectionNr,dataType)];
    end;
else
    if isempty(readDspList)
        firstDsp=dspList(1);
        lastDsp=dspList(end);
        [firstDsp,lastDsp];
        readData=mgp_op_rdseq_dsp(firstDsp,lastDsp,len,startAddress,connectionNr,dataType);
    else
        firstDsp=dspList(1);
        for i=reshape(readDspList,1,length(readDspList))
        lastDsp=dspList(i);    
        [firstDsp,lastDsp];
        readData=[readData;mgp_op_rdseq_dsp(firstDsp,lastDsp,len,startAddress,connectionNr,dataType)];
        firstDsp=dspList(i+1);
        end;
        lastDsp=dspList(end);
        [firstDsp,lastDsp];
        readData=[readData;mgp_op_rdseq_dsp(firstDsp,lastDsp,len,startAddress,connectionNr,dataType)];
    end;
end;
x=readData;
