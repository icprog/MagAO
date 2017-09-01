%BCUs location
BCU_IP_00 = '192.168.0.68';
BCU_IP_01 = '192.168.0.65';
BCU_IP_10 = '192.168.0.62';
BCU_IP_11 = '192.168.0.71';
BCU_IP_20 = '192.168.0.66';
BCU_IP_21 = '192.168.0.67';
AOConnect(BCU_IP_00,BCU_IP_01,BCU_IP_10,BCU_IP_11,BCU_IP_20,BCU_IP_21);

% DSP
firstDSP=0;
lastDSP=27;

for i=1:length(AO),
    BCUStatus(i)=aoGetBCUStatus(i);
    DSPStatus(:,i)=aoGetDSPFullStatus(firstDSP,lastDSP,i);    
    SigGenStatus(i)=aoGetDSPFullStatus(252,252,i);    
end;
    
fprintf(1,'Crate          #0.0          #0.1          #1.0          #1.1          #2.0          #2.1\n');
fprintf(1,'IP        ');
for i=1:6,
    fprintf(1,' %s ',BCUStatus(i).ipaddress);
end;
fprintf('\n');

for j=1:14,
    fprintf(1,'DSP#%02d ser',j);
    for i=1:6,
        fprintf(1,' %s ',DSPStatus(j,i).serial_number);
    end;
    fprintf('\n');
end

fprintf(1,'SigGen    ');
for i=1:6,
    fprintf(1,' %s ',SigGenStatus(i).serial_number);
end;
fprintf('\n');
