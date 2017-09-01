function aoVariables=aoReadVariables(inFileName)

AO_VARIABLE_DEFINE();

fd=fopen(inFileName,'rt');
if fd==-1
   error('Invalid file name');
end

aoVariables=[];
aoVariables.name='';
aoVariables.nrItem=0;
aoVariables.memPointer=0;
aoVariables.category=0;
aoVariables.description='';
aoVariables.type=0;
aoVariables.operation=0;
idx=1;

while 1
   line=fgetl(fd);
   if line==-1
      break;
   end
   if ~isempty(line) && line(1)=='_'
      aa=textscan(line,'%s%d%d%s%s');
      aoVariables(idx).name=cell2mat(aa{1});
      aoVariables(idx).nrItem=aa{2};
      aoVariables(idx).memPointer=aa{3};
      switch cell2mat(aa{4})
         case 'board'
            aoVariables(idx).category = CAT_BOARD;
         case 'channel'
            aoVariables(idx).category = CAT_CHANNEL;
         otherwise
            error('Invalid variable category');
      end
      switch cell2mat(aa{5})
         case 'float'
            aoVariables(idx).type=TYPE_SINGLE;
         case 'uint32'
            aoVariables(idx).type=TYPE_UINT32;
         case 'int32'
            aoVariables(idx).type=TYPE_INT32;
         case 'struct'
            aoVariables(idx).type=TYPE_STRUCT;
         otherwise
            error('Invalid variable type');
      end
      aoVariables(idx).description='';
      idx=idx+1;
   end
end

fclose(fd);
