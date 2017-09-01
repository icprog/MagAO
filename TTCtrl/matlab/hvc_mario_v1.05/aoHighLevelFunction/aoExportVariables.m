function aoExportVariables(database,outFileName)
% aoExportVariables(database,outFileName)
% This function exports the aoVariables loaded from "database" into a text file according to the 
% LCU interpreter format developed for the DSM-VLT project
%
% Parameters:
% database    = can be either a file containing the variables or a workspace variable
% outFileName = naem of the output file

% Author(s): Mario
%
% Copyright 2004-2008 Microgate s.r.l.
% $Revision 0.1 $ $Date: 2011/11/03
% $Revision 0.2 $ $Date: 2013/08/23 by Roby. Added the possibility of using directly a workspace variable as input

AO_VARIABLE_DEFINE();
try 
    aoVariables=evalin('caller',database);
catch
	try
	   load(database);
	catch
	   error('database filename was not found!!!');
	end
end;

fd=fopen(outFileName,'wt');
max_name_len=0;
for i=1:length(aoVariables)
   max_name_len=max(max_name_len,length(aoVariables(i).name));
end
fprintf(fd,'#\n');
fprintf(fd,'#  Microgate M2DSM software\n');
fprintf(fd,'#  AO Variables\n');
fprintf(fd,'#\n');
fprintf(fd,'#  AO (adaptive optics) variables indicate memory locations\n');
fprintf(fd,'#  that contain data with a physical interpretation\n');
fprintf(fd,'#\n');
fprintf(fd,'#  DO NOT EDIT MANUALLY\n');
fprintf(fd,'#\n');
fprintf(fd,'#  "@(#) $Id$"\n');
fprintf(fd,'#  ''$Rev::             $''\n');
fprintf(fd,'#\n');
fprintf(fd,'\n');
fprintf(fd,'# name                      nrItem memPointer category    type\n');
for i=1:length(aoVariables)
   fprintf(fd,'%s',aoVariables(i).name);
   fprintf(fd,blanks(max_name_len-length(aoVariables(i).name)+2));
   fprintf(fd,'%5d  ',aoVariables(i).nrItem);
   fprintf(fd,'%9d  ',aoVariables(i).memPointer);
   switch aoVariables(i).category
      case CAT_BOARD
         fprintf(fd,'  board  ');
      case CAT_CHANNEL
         fprintf(fd,'channel  ');
   end
   switch aoVariables(i).type
      case TYPE_SINGLE
         fprintf(fd,' float  ');
      case TYPE_UINT32
         fprintf(fd,'uint32  ');
      case TYPE_INT32
         fprintf(fd,' int32  ');
      case TYPE_STRUCT
         fprintf(fd,'struct  ');
   end
   fprintf(fd,'%s\n',aoVariables(i).description);
end

fclose(fd);
