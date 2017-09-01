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
    if isfield(test,'vContattoCappella')
       test=rmfield(test,'vContattoCappella');
    end;
    if isfield(test,'transferFunctionCoil')
       test=rmfield(test,'transferFunctionCoil');
    end;
    if isfield(test,'dinamico')
       test=rmfield(test,'dinamico');
    end;
    if isfield(test,'powerSupply')
       test=rmfield(test,'powerSupply');
    end;

    testArray(i)=test;
end;


%extract all fieldnames
field=fieldnames(testArray);
for i=1:length(field)
   eval([cell2mat(field(i)),'=[testArray.',cell2mat(field(i)),'];']);
end;


%in macchina
% precisioneMeno=[testArray.precisioneMeno];
% precisionePiu=[testArray.precisionePiu];

% [sortTime,sortTimeInd]=sort([testArray.starttime])
% [sortNumber,sortNumberInd]=sort([testArray.serialNumber])
% primaProva=testArray([sortTimeInd(1:end/2)])
% secondaProva=testArray([sortTimeInd(end/2+1:end)])
% primaForza=[primaProva.forzaCoil]
% secondaForza=[secondaProva.forzaCoil]
% 

if PLOT_ON
    %plot risultati
    figure
    plot(serialNumber,vR1,'.')
    title('vR1')
    figure
    plot(serialNumber,vR2,'.')
    title('vR2')
    figure
    plot(serialNumber,iR1,'.')
    title('iR1')
    figure
    plot(serialNumber,iR2,'.')
    title('vR2')
    figure
    plot(serialNumber,vContattoCappella,'.')
    title('vContattoCappella')
    figure
    plot(serialNumber,temperaturaAmbiente,'.')
    title('temperaturaAmbiente')

    %ADC
    figure
    plot(serialNumber,[precisioneMeno.gain],'.')
    title('gain ADC Meno')
    figure
    plot(serialNumber,[precisioneMeno.offset],'.')
    title('offset ADC Meno')
    figure
    plot(serialNumber,[precisioneMeno.errRMS],'.')
    title('errRMS ADC Meno')
    figure
    plot(serialNumber,[precisionePiu.gain],'.')
    title('gain ADC Piu')
    figure
    plot(serialNumber,[precisioneMeno.offset],'.')
    title('offset ADC Meno')
    figure
    plot(serialNumber,[precisioneMeno.errRMS],'.')
    title('errRMS ADC Meno')



    %Coil
    figure
    plot(serialNumber,[forzaCoil.fBiasMagnete],'.')
    title('forzaCoil fBiasMagnete')
    figure
    plot(serialNumber,[forzaCoil.gain],'.')
    title('forzaCoil gain')
    figure
    plot(serialNumber,[forzaCoil.offset],'.')
    title('forzaCoil offset')
    figure
    plot(serialNumber,[forzaCoil.errRMS],'.')
    title('forzaCoil errRMS')

    plot([primaForza.gain]-[secondaForza.gain])
    plot([primaProva.serialNumber]-[secondaProva.serialNumber])
end;


for i=1:length(testArray)-1,
    deltaT(i)=temperaturaAmbiente(i+1)-temperaturaAmbiente(i);
    deltaTime(i)=starttime(i+1)-starttime(i);
end;

if exist('precisionePiu')
disp('');
clear ppmADC;
% for j=[1:length(precisionePiu(1).out)];
for j=1;
    disp('===========');
    disp(['CAPSENSE Sig_gain=',num2str(precisionePiu(1).sigGain(j))]);
    disp('===========');
    fprintf(1,'SN   deltaTime    deltaT    ppm        ppm/°K\n');      
    for i=1:length(testArray)-1,
        ppm(i)=(([precisionePiu(i+1).out(j)]-[precisioneMeno(i+1).out(j)])./([precisionePiu(i).out(j)]-[precisioneMeno(i).out(j)])-1)*1e6;
        ppm_C(i)=ppm(i)/deltaT(i);
%         vContattoCap(i)=vContattoCappella(i+1)-vContattoCappella(i);
%         fprintf(1,'%3i  %s  %8.3f  %10.1f  %10.3f  %10.3f\n',serialNumber(i+1),datestr(deltaTime(i),'HH:MM:SS'),deltaT(i),ppm(i),ppm_C(i),vContattoCap(i));      
        fprintf(1,'%3i  %s  %8.3f  %10.1f  %10.3f\n',serialNumber(i+1),datestr(deltaTime(i),'HH:MM:SS'),deltaT(i),ppm(i),ppm_C(i));      
    end;
    ppmADC(:,j)=ppm;
end;
end;

if exist('forzaCoil')
disp('');
clear ppmCoil;
% for j=[1:length(forzaCoil(1).out)];
for j=1;
    disp('===========');
    disp(['FORZA COIL I_in=',num2str(forzaCoil(1).in(j))]);
    disp('===========');
    fprintf(1,'SN   deltaTime    deltaT    ppm        ppm/°K\n');      
    for i=1:length(testArray)-1,
        ppm(i)=((forzaCoil(i+1).out(j)./forzaCoil(i).out(j))-1)*1e6;
        ppm_C(i)=ppm(i)/deltaT(i);
        fprintf(1,'%3i  %s  %8.3f  %10.1f  %10.3f\n',serialNumber(i+1),datestr(deltaTime(i),'HH:MM:SS'),deltaT(i),ppm(i),ppm_C(i));      
    end;
    ppmCoil(:,j)=ppm;
end;
end;


if exist('forzaCoil')
disp('');
clear ppmCoil;
    disp('=================');
    disp(['FORZA BIAS COIL']);
    disp('=================');
    fprintf(1,'SN   deltaTime    deltaT    ppm        ppm/°K\n');      
    for i=1:length(testArray)-1,
        ppm(i)=((forzaCoil(i+1).fBias./forzaCoil(i).fBias)-1)*1e6;
        ppm_fBias(i)=ppm(i)/deltaT(i);
        fprintf(1,'%3i  %s  %8.3f  %10.1f  %10.3f\n',serialNumber(i+1),datestr(deltaTime(i),'HH:MM:SS'),deltaT(i),ppm(i),ppm_fBias(i));      
    end;
end;


if exist('forzaCoil')
disp('');
clear ppmCoil;
    disp('====================');
    disp(['FORZA BIAS MAGNETE']);
    disp('====================');
    fprintf(1,'SN   deltaTime    deltaT    %%        %%/°K\n');      
    for i=1:length(testArray)-1,
        ppm(i)=((forzaCoil(i+1).fBiasMagnete./forzaCoil(i).fBiasMagnete)-1)*1e2;
        ppm_fBiasMagnete(i)=ppm(i)/deltaT(i);
        fprintf(1,'%3i  %s  %8.3f  %10.1f  %10.3f\n',serialNumber(i+1),datestr(deltaTime(i),'HH:MM:SS'),deltaT(i),ppm(i),ppm_fBiasMagnete(i));      
    end;
end;



% for i=1:length(testArray)-1,
%     ppm(i)=(([precisionePiu(i+1).out(1)]-[precisioneMeno(i+1).out(1)])./([precisionePiu(i).out(1)]-[precisioneMeno(i).out(1)])-1)*1e6;
%     deltaT(i)=testArray(i+1).temperaturaAmbiente-testArray(i).temperaturaAmbiente;
%     deltaTime(i)=testArray(i+1).starttime-testArray(i).starttime;
%     ppm_C(i)=ppm(i)/deltaT(i);
%     fprintf(1,'%s    %4.3f      %4.1f  %4.3f\n',datestr(deltaTime(i),'HH:MM:SS'),deltaT(i),ppm(i),ppm_C(i));      
% end;
% 
