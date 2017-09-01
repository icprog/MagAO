function [resData,resStatus]=m2dsmHKLCommand(command,varargin)
% [resData,resStatus]=m2dsmHKLCommand(command[,connectionNr][,params])
% command:      HKL command
% connectionNr: default is 1
% params:       optional parameters (string format)
% 
% Returns:
% resData:   returned data in uint8 (conversion from base64 is performed within the function
% resStatus: status return, 'OK command' if OK, 'BAD command' if an error occurred
% 
% Author(s): Mario & Chris
%
% Copyright 2011 Microgate s.r.l.
% 
persistent tag;

% limit_size=10485759; % dimensione massima accettabile dalla LCU (mettere un numero dispari...)
limit_size=2097151; % dimensione ottimizzata tra velocita' matlab e LCU (mettere un numero dispari...)

if isempty(tag)
   tag=0;
end

resData='';
params=[];
connectionNr=1;
if nargin>=2
    connectionNr=varargin{1};
end
if nargin>=3
    params=varargin{2};
end

%get communication socket
socket=getSocket(connectionNr);

%write data
strTag=sprintf('A%04d',tag);
cnt=1;
repStr=[];
if     isempty(params)
   str=sprintf('%s %s\n',strTag,upper(command));
   if length(str)>255
      error('Invalid HKL command string length (%d > 255)',length(str));
   end
   pnet(socket,'write',str);
elseif ischar(params)
   str=sprintf('%s %s %s\n',strTag,upper(command),params);
   if length(str)>255
      error('Invalid HKL command string length (%d > 255)',length(str));
   end
   pnet(socket,'write',str);
elseif strcmp(class(params),'uint8')
   params=base64encode(params);
   str=sprintf('%s %s\n',strTag,upper(command));
   if length(str)>255
      error('Invalid HKL command string length (%d > 255)',length(str));
   end
   pnet(socket,'write',str);
   repStr=pnet(socket,'readline');
   if isempty(repStr)
      error('Timeout error from LCU');
   elseif strcmp('+',repStr)
      pnet(socket,'write',params);
      repStr=[];
   end
else
   error('Invalid params class');
end
while 1
   str1=repStr;
   while isempty(str1) || length(str1)==limit_size
      str1=pnet(socket,'readline',limit_size);
      if isempty(str1)
         error('Timeout error from LCU');
      end
      repStr=[repStr str1];
   end
   if length(repStr)>=5 && strcmp(strTag,repStr(1:5))
      break;
   elseif strcmp('+',repStr(1))
      % 64bit decoding (da fare fuori)
      resData=uint8(base64decode(repStr(3:end)));
   elseif strcmp('*',repStr(1))
      resData{cnt}=repStr;
      cnt=cnt+1;
   else
      error('Unexpected string reply: %s',repStr);
   end
   repStr=[];
end

tag=tag+1;
if tag>=10000
    tag=0;
end
    
resStatus = repStr(7:end);

% pnet(socket,'write',uint32(dword(2))); % write header
% pnet(socket,'write',uint32(dword(3)),'intel'); % write header
