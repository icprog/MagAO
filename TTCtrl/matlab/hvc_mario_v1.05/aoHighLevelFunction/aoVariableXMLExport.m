fd=fopen('/tmp/aoVariables.xml','w+');

fprintf(fd,'<?xml version="1.0" encoding="utf-8"?>\n');
fprintf(fd,'<AOVAR version="1.0">\n');

for ii=1:length(aoVariables)
    var=aoVariables(ii);
    if var.category==1
        category='board"  ';
    elseif var.category==2
        category='channel"';
    else
        error('unkown category %i',var.category);
    end        
    if var.type==3
        tp='uint32';
    elseif var.type==2
        tp='int32';
    else
        error('unkown type %i',var.type);
    end        
    fprintf(fd,'   <var');
    fprintf(fd,' category="%s',category);
    fprintf(fd,' description="%s"',var.description);
    fprintf(fd,' device="%s"',var.device);
    fprintf(fd,' memPointer="%05d"',var.memPointer);
    nr=fprintf(fd,' name="%s"',var.name);
    for s=1:35-nr, fprintf(fd,' '); end
    fprintf(fd,' nrItem="%04d"',var.nrItem);
    fprintf(fd,' operation="%s"',var.operation);
    fprintf(fd,' stepPtr="%04d"',var.stepPtr);
    nr=fprintf(fd,' type="%s"',tp);
    for s=1:14-nr, fprintf(fd,' '); end
    fprintf(fd,'/>\n');
end
fprintf(fd,'</AOVAR>\n');
fclose(fd);