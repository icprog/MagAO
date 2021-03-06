%BCUs location
% BCU_IP_00 = '192.168.1.70';
% BCU_IP_01 = '192.168.1.71';
% BCU_IP_10 = '192.168.1.72';
% BCU_IP_11 = '192.168.1.73';
% BCU_IP_20 = '192.168.1.74';
% BCU_IP_21 = '192.168.1.75';
% AOConnect(BCU_IP_00,BCU_IP_01,BCU_IP_10,BCU_IP_11,BCU_IP_20,BCU_IP_21);

clear aaa;
for crateN=1:6,
    aaa(crateN)=aoGetBCUStatus(crateN);
    power_total(crateN)=aaa(crateN).voltage_vccl*aaa(crateN).current_vccl + ...
                        aaa(crateN).voltage_vcca*aaa(crateN).current_vcca + ...
                        aaa(crateN).voltage_vssa*aaa(crateN).current_vssa + ...
                        aaa(crateN).voltage_vccp*aaa(crateN).current_vccp + ...
                        aaa(crateN).voltage_vssp*aaa(crateN).current_vssp;
end;
power_total_sum=sum(power_total);


fprintf(1,'\nCrate    #0.0    #0.1    #1.0    #1.1    #2.0    #2.1\n\n');
fprintf(1,'in0   '); for i=1:6, fprintf(1,' %6.2f ',aaa(i).in0_temp); end; fprintf('�C\n');
fprintf(1,'in1   '); for i=1:6, fprintf(1,' %6.2f ',aaa(i).in1_temp); end; fprintf('�C\n');
fprintf(1,'out0  '); for i=1:6, fprintf(1,' %6.2f ',aaa(i).out0_temp); end; fprintf('�C\n');
fprintf(1,'out1  '); for i=1:6, fprintf(1,' %6.2f ',aaa(i).out1_temp); end; fprintf('�C\n');
fprintf(1,'hum   '); for i=1:6, fprintf(1,' %6.2f ',aaa(i).ext_humidity); end; fprintf('%%RH\n');
fprintf(1,'press '); for i=1:6, fprintf(1,' %6.2f ',aaa(i).pressure); end; fprintf('bar\n');
fprintf(1,'power '); for i=1:6, fprintf(1,' %6.2f ',power_total(i)); end; fprintf('W\n');
fprintf(1,'Total power %6.2f W\n',power_total_sum);
clear aaa;
