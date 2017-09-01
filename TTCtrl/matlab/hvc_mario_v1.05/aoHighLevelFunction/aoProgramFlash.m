function aoProgramFlash(flashBlock,varargin)
% aoProgramFlash(flashBlock,[filename],[firstDsp,[lastDsp]],[connectionNr])
% flashes a specified flashBlock
%
% flashBlock can be 1 to 7
% 1 -> default logic
% 2 -> default program
% 3 -> user program
% 4 -> user logic
% 5 -> unused area 0
% 6 -> configuration area
% 7 -> unused area 1
% filename is the location of file to download if not specified an open
% file GUI will appear
% firstDsp/lastDsp are the targe address (see mgp_op_xxxx protocol) 
% default is 0

% Author(s): D. Pescoller
%
% Copyright 2004-2008 Microgate s.r.l.
% $Revision 0.1 $ $Date: 14/01/2005
% 13/04/2010 modified by mario: inserted a sub_len size to break the big file (avoiding eth packets lost)
% 17/10/2011 modified by mario: updated to accept the conncectionNr optional argument

%include for MGP_FLASH_TABLE
MGP_DEFINE();

firstDsp=0;
lastDsp=0;
connectionNr=1;
sub_len=20000;

%processing parameters
if nargin>1
	if isstr(varargin{1})
      fileName=varargin{1};
      if nargin > 2
         firstDsp=varargin{2};
      end;
      if nargin > 3
         lastDsp=varargin{3};
      else
         lastDsp=firstDsp;
      end;
      if nargin > 4
         connectionNr=varargin{4};
      end;
	else
      firstDsp=varargin{1};
      if nargin > 2
         lastDsp=varargin{2};
      else
         lastDsp=firstDsp;
      end;
      if nargin > 3
         connectionNr=varargin{3};
      end;
	end;
end;

%get file
if ~exist('fileName')
    [filename, pathname] = uigetfile('*.*', 'selecting flash file');   
    fileName=[pathname,filename];
end;

fid=fopen(fileName);
fileContent=fread(fid,inf,'uint32');

%flashing
startAddress=MGP_FLASH_TABLE(flashBlock).startAddress;
fileLen=length(fileContent);
fprintf('flashing %s\n',MGP_FLASH_TABLE(flashBlock).name);
% mgp_op_write_flash(firstDsp,lastDsp,fileLen,startAddress,fileContent);
i=0;
while i<fileLen
   if i+sub_len<fileLen
      mgp_op_write_flash(firstDsp,lastDsp,sub_len,startAddress+i,fileContent(i+1:i+sub_len),connectionNr);
   else
      mgp_op_write_flash(firstDsp,lastDsp,fileLen-i,startAddress+i,fileContent(i+1:fileLen),connectionNr);
   end
   i=i+sub_len;
end
%compare
flashContent=mgp_op_rdseq_flash(firstDsp,firstDsp,fileLen,startAddress,connectionNr);
if(all(flashContent==fileContent))
	disp('flashing succeded');
else
	error('flashing failed');
end;
fclose(fid);
