function x=readFinByIndex(finArray,connectionNr,len,startAddress,dataType)
% this function can be used by supplying the fin as an array
% unique values are returned

% Author(s): Mario
%
% Copyright 2004-2008 Microgate s.r.l.
% $Revision 0.1 $ $Date: 22/12/2004

finList=unique(finArray);

readFinList=getContinuousIndex(finList);

if isempty(readFinList)
   firstFin=finList(1);
   lastFin=finList(end);
   readData=m4_op_read(firstFin,lastFin,len,startAddress,connectionNr,dataType,2);
else
   readData=[];
	firstFin=finList(1);
   for i=reshape(readFinList,1,length(readFinList))
      lastFin=finList(i);    
      readData=[readData m4_op_read(firstFin,lastFin,len,startAddress,connectionNr,dataType)];
      firstFin=finList(i+1);
   end
   lastFin=finList(end);
   readData=[readData m4_op_read(firstFin,lastFin,len,startAddress,connectionNr,dataType)];
end

x=readData;
