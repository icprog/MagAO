function var=aoReadMapFile(filename,varargin)
% reads file filename and returns an array of var 
%(name,address and nrItem category)
%var=aoReadMapFile(filename)
AO_VARIABLE_DEFINE()
category=0;
if nargin==2
    if isnumeric(varargin{1})
        category=varargin{1};
    else
        category=find(strcmp({varCatArray.prefix},varargin{1}));
    end;
end;
if isempty(category)
    error('category not found');
end;

xDoc=xmlread(filename);
allListItems=xDoc.getElementsByTagName('SYMBOL');
i=1;
for k=0:allListItems.getLength-1
    item=allListItems.item(k);
    currentVar.name=char(item.getAttribute('name'));
    currentVar.nrItem=getNumeric(char(item.getAttribute('size')));
    currentVar.memPointer=getNumeric(char(item.getAttribute('address')));
    if currentVar.nrItem==0
        continue
    end;
    switch currentVar.nrItem
        case 4
             if(category==0)
                        currentVar.category=CAT_CHANNEL;
            else
                currentVar.category=category;
                currentVar.name=[varCatArray(category).prefix,currentVar.name];
            end;
            var(i)=currentVar;
            i=i+1;
%         case 8
%             if(category==0)
%                         currentVar.category=CAT_CHANNEL;
%             else
%                 currentVar.category=category;
%                 currentVar.name=[varCatArray(category).prefix,currentVar.name];
%             end;
%             tmpName=currentVar.name;
%             currentVar.nrItem=4;
%             currentVar.name=[tmpName,'_A'];
%             var(i)=currentVar;
%             i=i+1;
%             currentVar.name=[tmpName,'_B'];
%             currentVar.memPointer=currentVar.memPointer+4;
%             var(i)=currentVar;
%             i=i+1;
        otherwise
            if(category==0)
                        currentVar.category=CAT_BOARD;
            else
                currentVar.category=category;
                currentVar.name=[varCatArray(category).prefix,currentVar.name];
            end;
            var(i)=currentVar;
            i=i+1;
      end;
end;

