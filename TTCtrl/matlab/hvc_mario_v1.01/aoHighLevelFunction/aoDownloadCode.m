function aoDownloadCode(firstDsp,lastDsp,filename,varargin);
% aoDownloadCode(firstDsp,lastDsp,filename,[startCode],[connectionNr])
% downloads DSP code
% startCode defaults to 0 (1=start 0=do not start)
% connectionNr default is 1

% Author(s): D. Pescoller
%
% Copyright 2004-2008 Microgate s.r.l.
% $Revision 0.1 $ $Date: 2004/08/09
% $Revision 0.2 $ $Date: 09/03/2007 - multiple Crate connection D. Pescoller
% moved the linkport register initialization in to the aoDspStopCode function

fid=fopen(filename,'r');
MGP_DEFINE();

connectionNr=1;
if nargin>=5
    connectionNr=varargin{2};
end

aoDspStopCode(firstDsp,lastDsp,connectionNr);

% ora leggo la parte programma
fileData = fscanf(fid,'%x'); 
commandPtr=1;
while commandPtr<length(fileData)
    cmd=fileData(commandPtr);
    commandPtr;
    dec2hex(cmd);
    switch bitshift(cmd,-28)
        case 4
            address=fileData(commandPtr+1);
            len=bitand(cmd,hex2dec('3FFFFF'));
            data=uint32(fileData(commandPtr+2:commandPtr+2+len));
            mgp_op_wrsame_dsp(firstDsp,lastDsp,len,address,data,connectionNr,'uint32');
            commandPtr=commandPtr+2+len;
        case 8
            address=fileData(commandPtr+1);
            len=bitand(cmd,hex2dec('3FFFFF'));
            data=uint32(zeros(len,1));
            mgp_op_wrsame_dsp(firstDsp,lastDsp,len,address,data,connectionNr,'uint32');
            commandPtr=commandPtr+2;
        case 0
            address=0;
            len=256;
            data=uint32(fileData(commandPtr+1:end));
            if(length(data)~=len)
                error('errore in file parsing 255 elements');
            end;
            mgp_op_wrsame_dsp(firstDsp,lastDsp,len,address,data,connectionNr,'uint32');
            commandPtr=commandPtr+2+len;
        otherwise
            error('error in file parsing');
    end;
end;
        

% ultimo pacchetto per far partire il programma
if nargin>=4
    if varargin{1}==1
        aoDspStartCode(firstDsp,lastDsp,connectionNr);
    end
end

fclose(fid);
