clear
PLOT_ON=0;
% create testarray
d = dir('*.mat');
str = {d.name};

[s,v] = listdlg('PromptString','Select a file:',...
                      'SelectionMode','multiple',...
                      'ListSize',[300 300],...
                      'InitialValue',[],...
                      'ListString',str);
if(v==0)
    error('Operation cancelled');
end;


for i=1:length(s)
    load(d(s(i)).name);
    testArray(i)=test;
end;


%extract all fieldnames
field=fieldnames(testArray);
for i=1:length(field)
   eval([cell2mat(field(i)),'=[testArray.',cell2mat(field(i)),'];']);
end;


disp('assorbimento')
disp('assorbimento.I_VCC')
maxVal=max([assorbimento.I_VCC])
minVal=min([assorbimento.I_VCC])
mean([maxVal,minVal])
(maxVal-minVal)*2
disp('assorbimento.I_VCC_A')
maxVal=max([assorbimento.I_VCC_A])
minVal=min([assorbimento.I_VCC_A])
mean([maxVal,minVal])
(maxVal-minVal)*2
disp('assorbimento.I_VSS_A')
maxVal=max([assorbimento.I_VSS_A])
minVal=min([assorbimento.I_VSS_A])
mean([maxVal,minVal])
(maxVal-minVal)*2
disp('assorbimento.I_VCC_P')
maxVal=max([assorbimento.I_VCC_P])
minVal=min([assorbimento.I_VCC_P])
mean([maxVal,minVal])
(maxVal-minVal)*2
disp('assorbimento.I_VSS_P')
maxVal=max([assorbimento.I_VSS_P])
minVal=min([assorbimento.I_VSS_P])
mean([maxVal,minVal])
(maxVal-minVal)*2
disp('assorbimento.I_VCC_P_AB')
maxVal=max([assorbimento.I_VCC_P_AB])
minVal=min([assorbimento.I_VCC_P_AB])
mean([maxVal,minVal])
(maxVal-minVal)*2
disp('assorbimento.I_VSS_P_AB')
maxVal=max([assorbimento.I_VSS_P_AB])
minVal=min([assorbimento.I_VSS_P_AB])
mean([maxVal,minVal])
(maxVal-minVal)*2


disp('precisioneAdc gain')
maxVal=max([precisioneAdc.gainPiuMeno])
minVal=min([precisioneAdc.gainPiuMeno])
mean([maxVal,minVal])
(maxVal-minVal)*2
disp('precisioneAdc offset')
maxVal=max([precisioneAdc.offsetPiuMeno])
minVal=min([precisioneAdc.offsetPiuMeno])
mean([maxVal,minVal])
(maxVal-minVal)*2

disp('precisioneADCSPI gain')
maxVal=max([precisioneADCSPI.gain])
minVal=min([precisioneADCSPI.gain])
mean([maxVal,minVal])
(maxVal-minVal)*2
disp('precisioneADCSPI offset')
maxVal=max([precisioneADCSPI.offset])
minVal=min([precisioneADCSPI.offset])
mean([maxVal,minVal])
(maxVal-minVal)*2

disp('precisioneCoil gain')
maxVal=max([precisioneCoil.gain])
minVal=min([precisioneCoil.gain])
mean([maxVal,minVal])
(maxVal-minVal)*2
disp('precisioneCoil offset')
maxVal=max([precisioneCoil.offset])
minVal=min([precisioneCoil.offset])
mean([maxVal,minVal])
(maxVal-minVal)*2

disp('maxDAC')
maxVal=max([maxDAC])
minVal=min([maxDAC])
mean([maxVal,minVal])
(maxVal-minVal)*2
disp('minDAC')
maxVal=max([minDAC])
minVal=min([minDAC])
mean([maxVal,minVal])
(maxVal-minVal)*2

disp('temperature sensors')
maxVal=max([dspStatus.temp]')
minVal=min([dspStatus.temp]')
mean([maxVal,minVal])
(maxVal-minVal)*2

disp('VCC_OP')
maxVal=max([VCC_OP])
minVal=min([VCC_OP])
mean([maxVal,minVal])
(maxVal-minVal)*2

disp('VSS_OP')
maxVal=max([VSS_OP])
minVal=min([VSS_OP])
mean([maxVal,minVal])
(maxVal-minVal)*2

disp('testBitADC')
max(max([testBitADC.bitCount]))
min(min([testBitADC.bitCount]))

disp('RMS rumoreDAC')
maxVal=max([rumoreDAC.rmsValue])
minVal=min([rumoreDAC.rmsValue])
mean([maxVal,minVal])
(maxVal-minVal)*2


if(PLOT_ON)
   figure(1);
   hold on;
    for i=[1:length(dinamicoADC)]
        
%        figure(i)
       transferFun=computeTransferFunction(dinamicoADC(i).result,dinamicoADC(i).identSetup,'fig',1);
       pause;
   hold on;

    end;
end;