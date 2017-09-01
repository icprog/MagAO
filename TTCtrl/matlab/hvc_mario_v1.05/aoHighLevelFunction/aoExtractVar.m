function aoExtractVar

% aoExtractVar
% this function creates a variable for each aoVariable in caller workspace
% this is useful since you can use the tab for autocompletion

% Author(s): D. Pescoller
%
% Copyright 2007-2011 Microgate s.r.l.
% $Revision 0.1 $ $Date: 19/01/2007

AO_VARIABLE_DEFINE();


aoVariables=evalin('base','aoVariables');
for i=1:length(aoVariables)
    var=aoVariables(i);
    if strcmp(var.name(1:4),'_dm2')
       try
       assignin('caller',var.name(2:end),var.name);
       end
    elseif strcmp(var.name(1:4),'_wfp')
       try
       assignin('caller',var.name(2:end),var.name);
       end
    else
       try
       assignin('caller',var.name(1:end),var.name);
       end
    end
end